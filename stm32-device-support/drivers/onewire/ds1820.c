////////////////////////////////////////////////////////////////////////////
// Brad Goold 2012
// DS1820 driver for STM32F103VE using FreeRTOS
// Modified by Mike Stuart, Feb 2014
////////////////////////////////////////////////////////////////////////////

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "ds1820.h"


#define DS1820_PIN                  (1<<DS1820_PIN_NUM)


#if FAMILY == STM32F1

#define DS1820_TIM_PRESCALER    36
// bus / gpio setup
#if DS1820_PIN_NUM < 8
#define DS1820_GPIO_CRx             DS1820_PORT->CRL
#else
#define DS1820_GPIO_CRx             DS1820_PORT->CRH
#endif

#define DS1820_GPIO_CRx_OUT_VALUE   (0x00000007<<((DS1820_PIN_NUM%8)*4))
#define DS1820_GPIO_CRx_IN_VALUE    (0x00000008<<((DS1820_PIN_NUM%8)*4))
#define DS1820_GPIO_CRx_MASK        (~(0x0000000F<<((DS1820_PIN_NUM%8)*4)))

// bus / gpio controls
#define DQ_IN()  {DS1820_GPIO_CRx &= DS1820_GPIO_CRx_MASK;DS1820_GPIO_CRx |= DS1820_GPIO_CRx_IN_VALUE;} // input
#define DQ_OUT() {DS1820_GPIO_CRx &= DS1820_GPIO_CRx_MASK;DS1820_GPIO_CRx |= DS1820_GPIO_CRx_OUT_VALUE;} // normal output, open drain
#define DQ_SET()   GPIO_SetBits(DS1820_PORT, DS1820_PIN)
#define DQ_RESET() GPIO_ResetBits(DS1820_PORT, DS1820_PIN)
#define DQ_READ()  GPIO_ReadInputDataBit(DS1820_PORT, DS1820_PIN)

#elif FAMILY == STM32F4

#define DS1820_TIM_PRESCALER    84
// bus / gpio controls
#define DQ_IN()   ds1820_input() // input
#define DQ_OUT()  ds1820_output() // normal output, open drain
#define DQ_SET()   GPIO_SetBits(DS1820_PORT, DS1820_PIN)
#define DQ_RESET() GPIO_ResetBits(DS1820_PORT, DS1820_PIN)
#define DQ_READ()  GPIO_ReadInputDataBit(DS1820_PORT, DS1820_PIN)

#endif


static inline void ds1820_input();
static inline void ds1820_output();
static void ds1820_timer();
static unsigned char ds1820_reset(void);
static void ds1820_write_bit(uint8_t bit);
static uint8_t ds1820_read_bit(void);
static void ds1820_write_byte(uint8_t byte);
static uint8_t ds1820_read_byte(void);
static void delay_us(uint16_t count);


static inline void ds1820_input()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  DS1820_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#if FAMILY == STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#elif FAMILY == STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#endif

    GPIO_Init(DS1820_PORT, &GPIO_InitStructure);
}

static inline void ds1820_output()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  DS1820_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#if FAMILY == STM32F1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
#elif FAMILY == STM32F4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#endif

    GPIO_Init(DS1820_PORT, &GPIO_InitStructure);
}

static void ds1820_timer_init()
{
    // intialise timer for counting
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

#if ((DS1820_TIMER == 1)||(DS1820_TIMER == 8))
    RCC_APB2PeriphClockCmd(DS1820_TIMER_CLOCK, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = DS1820_TIM_PRESCALER*2;       //1MHz
#else
    RCC_APB1PeriphClockCmd(DS1820_TIMER_CLOCK, ENABLE);
    TIM_TimeBaseStructure.TIM_Prescaler = DS1820_TIM_PRESCALER;       //1MHz
#endif

    TIM_TimeBaseStructure.TIM_Period = 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInit(DS1820_TIMER_PERIPH, &TIM_TimeBaseStructure);
}

void ds1820_init(void)
{
    ds1820_timer_init();
    ds1820_input();
    DQ_IN();
    DQ_SET();
}

uint64_t ds1820_search()
{
    uint8_t devcode[8];
    uint8_t i;

    ds1820_reset();
    ds1820_write_byte(READ_ROM);
    for(i = 0; i < 8; i++){
        devcode[i] = ds1820_read_byte();
    }
    ds1820_reset();

    return *((uint64_t*)devcode);
}

void ds1820_convert(void)
{
    ds1820_reset();
    ds1820_write_byte(SKIP_ROM);
    ds1820_write_byte(CONVERT_TEMP);
    ds1820_reset();
    DQ_IN();
}

float ds1820_read_device(uint64_t devcode)
{
    float retval;
    uint16_t ds1820_temperature1 = 10000;
    int i;
    uint8_t sp1[9]; //temp to hold scratchpad memory

    if(ds1820_reset() != NO_ERROR)
        return 211.00;
    ds1820_reset();
    ds1820_write_byte(MATCH_ROM);

    for (i = 0; i < 8; i++)
        ds1820_write_byte(*(((uint8_t*)&devcode) + i));

    ds1820_write_byte(READ_SCRATCHPAD);

    for (i = 0; i < 9; i++)
        sp1[i] = ds1820_read_byte();

    ds1820_reset();

    ds1820_temperature1 = sp1[1] & 0x0f;
    ds1820_temperature1 <<= 8;
    ds1820_temperature1 |= sp1[0];
    unsigned char remain = sp1[6];
    ds1820_temperature1 >>= 1;
    ds1820_temperature1 = (ds1820_temperature1 * 100) -  25  + (100 * 16 - remain * 100) / (16);
    retval = ((float)ds1820_temperature1/1000);

    return retval;
}

static void delay_us(uint16_t count)
{
    TIM_SetCounter(DS1820_TIMER_PERIPH, count);
    TIM_Cmd(DS1820_TIMER_PERIPH, ENABLE);
    while(TIM_GetCounter(DS1820_TIMER_PERIPH));
    TIM_Cmd(DS1820_TIMER_PERIPH, DISABLE);
}

static unsigned char ds1820_reset(void)
{
    portENTER_CRITICAL();
    DQ_OUT();
    DQ_RESET();
    delay_us(500);
    DQ_IN();
    DQ_SET();

    delay_us(60);
    if (DQ_READ()!= 0)
    {
        portEXIT_CRITICAL();
        printf("ds18b20 reset failed - no device found\n");
        return PRESENCE_ERROR;
    }
    portEXIT_CRITICAL();

    delay_us(240);

    return NO_ERROR;
}

static void ds1820_write_bit(uint8_t bit){

    DQ_OUT();
    portENTER_CRITICAL();
    DQ_RESET(); // pull low for 2us
    delay_us(2);
    if (bit){
        DQ_IN();
    }
    else DQ_RESET();
    delay_us(60);
    portEXIT_CRITICAL();
    DQ_IN(); // return bus
}

static uint8_t ds1820_read_bit(void){
    uint8_t bit;
    delay_us(1);
    DQ_OUT();
    DQ_SET(); // make sure bus is high
    portENTER_CRITICAL();
    DQ_RESET(); // pull low for 2us
    delay_us(1);
    DQ_IN(); // give bus back to DS1820;
    delay_us(5);
    bit = DQ_READ();
    delay_us(56);
    //DQ_OUT();
    //DQ_SET();
    portEXIT_CRITICAL();
    if (bit)
        return 1;
    else return 0;
}

static void ds1820_write_byte(uint8_t byte){

    delay_us(100);
    portENTER_CRITICAL();
    int i;
    for (i = 0; i < 8; i++) {
        if (byte&0x01){
            ds1820_write_bit(1);
            // printf("1 written\r\n");
        }
        else {
            ds1820_write_bit(0);
            //printf("0 written\r\n");
        }
        byte = byte>>1;
    }
    //delay_us(100);
   portEXIT_CRITICAL();
}

static uint8_t ds1820_read_byte(void){
    int i;
    uint8_t bit, byte = 0;
    portENTER_CRITICAL();
    delay_us(1);
    for (i = 0; i < 8; i++) {
        byte = byte >> 1;
        bit = ds1820_read_bit();
        if (bit==0) {
            byte = byte & 0x7F;
        }
        else byte = byte | 0x80;


    }
     portEXIT_CRITICAL();
    return byte;
}


// TODO, clean up the good parts of the below

// uint8_t rom[8]; //temporary to store rom codes
// float temps[4]; // holds the converted temperatures from the devices
// char * b[5]; // holds the 64 bit addresses of the temp sensors

// ////////////////////////////////////////////////////////////////////////////
// // Interfacing Function
// ////////////////////////////////////////////////////////////////////////////
// void vTaskDS1820Convert( void *pvParameters ){
//     char buf[30];
//     int i = 0;


//     // initialise the bus
//     ds1820_init();
//     if (ds1820_reset() ==PRESENCE_ERROR)
//     {
//         sprintf(buf, "NO SENSOR DETECTED\r\n");
//         vTaskDelete(NULL); // if this task fails... delete it
//     }

//     // Allocate memory for sensors
//     b[HLT] = (char *) malloc (sizeof(rom)+1);
//     b[MASH] = (char *) malloc (sizeof(rom)+1);
//     b[CABINET] = (char *) malloc (sizeof(rom)+1);
//     b[AMBIENT] = (char *) malloc (sizeof(rom)+1);
//     b[SPARE] = (char *) malloc (sizeof(rom)+1);

//     // Copy default values
//     memcpy(b[HLT], HLT_TEMP_SENSOR, sizeof(HLT_TEMP_SENSOR)+1);
//     memcpy(b[MASH], MASH_TEMP_SENSOR, sizeof(MASH_TEMP_SENSOR)+1);
//     memcpy(b[CABINET], CABINET_TEMP_SENSOR, sizeof(CABINET_TEMP_SENSOR)+1);
//     memcpy(b[AMBIENT], AMBIENT_TEMP_SENSOR, sizeof(AMBIENT_TEMP_SENSOR)+1);



//     for (;;)
//     {
//         ds1820_convert();

//         vTaskDelay(2000); // wait for conversion

//         // save values in array for use by application
//         for (i = 0 ; i < 4; i++)
//             temps[i] = ds1820_read_device(b[i]);


//       // Uncomment below to send temps to the console
//         /*
//         sprintf(buf, "HLT Temp = %.2fDeg-C\r\n", temps[HLT]);
//         xQueueSendToBack(xConsoleQueue, &buf, 100);
//         sprintf(buf, "Mash Temp = %.2fDeg-C\r\n", temps[MASH]);
//         xQueueSendToBack(xConsoleQueue, &buf, 100);
//         sprintf(buf, "Cabinet Temp = %.2fDeg-C\r\n", temps[CABINET]);
//         xQueueSendToBack(xConsoleQueue, &buf, 100);
//         sprintf(buf, "Ambient Temp = %.2fDeg-C\r\n", temps[AMBIENT]);
//         xQueueSendToBack(xConsoleQueue, &buf, 100);
//         */

//         taskYIELD();
//     }

// }
// ////////////////////////////////////////////////////////////////////////////

// float ds1820_get_temp(unsigned char sensor){

//     return temps[sensor];
// }
// ////////////////////////////////////////////////////////////////////////////

// void ds1820_search_key(uint16_t x, uint16_t y){
//     uint16_t window = 255;
//     char code[30];
//     char lcd_string[30];
//     char console[100];
//     float sensor_temp = 0.00;
//     static uint16_t last_window = 0;


// /*
//     // window locations
//     if (touchIsInWindow(x,y, 0,0, 150,50) == pdTRUE)
//         window = 0;

//     else  if (touchIsInWindow(x,y, 0,50, 150,100) == pdTRUE)
//         window = 1;


//     else if (touchIsInWindow(x,y, 0,140, 120,220) == pdTRUE)
//         window = 2;

//     else  if (touchIsInWindow(x,y, 120,140, 239,220) == pdTRUE)
//         window = 3;


//     else  if (touchIsInWindow(x,y, 0,220, 120,300) == pdTRUE)
//         window = 4;


//     else  if (touchIsInWindow(x,y, 120,220, 239,300) == pdTRUE)
//         window = 5;


//     else  if (touchIsInWindow(x,y, 160, 0, 230,100) == pdTRUE)
//         window = 6;
// */
//     //Back Button
//     if (window == 6)
//     {


//     }

// #if 0  // needs to be rewritten to use lcd_printf
//     //get code button
//     else if (window == 0){

//         ds1820_search();
//         if (rom[0] == 0x10)
//             sprintf(code, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\0",rom[0],
//                     rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
//         else sprintf(code, "NO SENSOR\0");
//         lcd_PutString(2, 101, code, Blue, Black);


//     }
//     //get temp button
//     else if (window == 1){
//         sensor_temp = ds1820_read_device(rom);
//         sprintf(code, "Current Sensor = %.2f\0", sensor_temp);
//         lcd_PutString(2, 117, code, Blue, Black);


//     }
//     //assign current to HLT
//     else if (window == 2){

//         if (rom[0] == 0x10)  {

//             memcpy(b[HLT], rom, sizeof(rom)+1);
//             sprintf(lcd_string, "COPIED\0");
//             sprintf(console, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n Assigned to HLT\r\n",rom[0],
//                     rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
//             xQueueSendToBack(xConsoleQueue, &console, 0);
//         }
//         else sprintf(lcd_string, "NO SENSOR TO COPY\0");
//         lcd_PutString(2, 160, lcd_string, Blue, Black);

//     }
//     //assign current to Mash
//     else if (window == 3){

//         if (rom[0] == 0x10)  {
//             memcpy(b[MASH], rom, sizeof(rom)+1);

//             sprintf(lcd_string, "COPIED\0");
//             sprintf(console, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n Assigned to MASH\r\n",rom[0],
//                     rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
//             xQueueSendToBack(xConsoleQueue, &console, 0);
//         }
//         else sprintf(lcd_string, "NO SENSOR TO COPY\0");
//         lcd_PutString(121, 160, lcd_string, Blue, Black);

//     }
//     //assign current to CABINET
//     else if (window == 4){
//         if (rom[0] == 0x10)  {
//             memcpy(b[CABINET], rom, sizeof(rom)+1);
//             sprintf(lcd_string, "COPIED\0");
//             sprintf(console, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n Assigned to CABINET\r\n",rom[0],rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
//              xQueueSendToBack(xConsoleQueue, &console, 0);
//         }
//         else sprintf(lcd_string, "NO SENSOR TO COPY\0");
//         lcd_PutString(2, 250, lcd_string, Blue, Black);
//     }
//     //assign current to AMBIENT
//     else if (window == 5){
//         if (rom[0] == 0x10)  {
//             memcpy(b[AMBIENT], rom, sizeof(rom)+1);
//             sprintf(lcd_string, "COPIED\0");
//             sprintf(console, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\r\n Assigned to AMBIENT\r\n",rom[0],
//                     rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
//             xQueueSendToBack(xConsoleQueue, &console, 0);
//         }
//         else sprintf(code, "NO SENSOR TO COPY\0");
//         lcd_PutString(121, 250, lcd_string, Blue, Black);

//     }

// #endif
// }
// ////////////////////////////////////////////////////////////////////////////

// void  ds1820_display_temps(void){

//     char lcd_string[20];

//     lcd_clear(Black);
// //    lcd_draw_back_button();
//     lcd_printf(1,1, 15, "TEMPERATURES");

//     lcd_printf(1, 40, 20, "HLT = %.2f\0", ds1820_get_temp(HLT));
//     lcd_printf(1, 56, 20, "Mash = %.2f\0", ds1820_get_temp(MASH));
//     lcd_printf(1, 72, 20, "Cabinet = %.2f\0", ds1820_get_temp(CABINET));
//     lcd_printf(1, 88, 20, "Ambient = %.2f\0", ds1820_get_temp(AMBIENT));
// }
// ////////////////////////////////////////////////////////////////////////////




// ////////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////////

// static float ds1820_read_device(uint8_t * rom_code){
//     float retval;
//     uint16_t ds1820_temperature1 = 10000;
//     int i;
//     uint8_t sp1[9]; //temp to hold scratchpad memory

//     portENTER_CRITICAL();
//     if (ds1820_reset()!= NO_ERROR)
//         return 211.00;
//     ds1820_reset();
//     ds1820_write_byte(MATCH_ROM);

//     for (i = 0; i < 8; i++)
//     {
//         ds1820_write_byte(rom_code[i]);
//     }
//     ds1820_write_byte(READ_SCRATCHPAD);
//     for (i = 0; i < 9; i++){
//         sp1[i] = ds1820_read_byte();
//     }

//     ds1820_reset();
//     ds1820_temperature1 = sp1[1] & 0x0f;
//     ds1820_temperature1 <<= 8;
//     ds1820_temperature1 |= sp1[0];
//     unsigned char remain = sp1[6];
//     ds1820_temperature1 >>= 1;
//     ds1820_temperature1 = (ds1820_temperature1 * 100) -  25  + (100 * 16 - remain * 100) / (16);
//     retval = ((float)ds1820_temperature1/100);
//     portEXIT_CRITICAL();
//     return retval;
// }
// ////////////////////////////////////////////////////////////////////////////

// void ds1820_search_applet(void)
// {
//     lcd_clear(Black);

//     lcd_printf(2, 3,  25, "Get ROM code from");
//     lcd_printf(2, 19, 25, "Current DS1820");
//     lcd_printf(2, 35, 25, "(One at a time!)");
//     lcd_printf(2, 53, 25, "Display the temp");
//     lcd_printf(2, 69, 25, "of current sensor");
//     char * t1 = "HLT\0";
//     char * t2 = "MASH\0";
//     char * t3 = "CABINET\0";
//     char * t4 = "AMBIENT\0";
//     lcd_DrawRect(160, 0, 230, 100, Red);
// //    lcd_PutString(179, 46, "BACK");
// //    lcd_draw_applet_options(t1, t2, t3, t4);

// }
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////


// // this function was used for diag reasons only.
// float ds1820_one_device_get_temp(void){
//     int i;
//     uint8_t sp[9]; //scratchpad
//     int16_t ds1820_temperature = 0;

//     ds1820_reset();
//     ds1820_write_byte(SKIP_ROM);
//     ds1820_write_byte(READ_SCRATCHPAD);
//     for (i = 0; i < 9; i++){
//         sp[i] = ds1820_read_byte();
//     }
//     ds1820_reset();
//     ds1820_temperature = sp[1] & 0x0f;
//     ds1820_temperature <<= 8;
//     ds1820_temperature |= sp[0];
//     unsigned char remain = sp[6];
//     ds1820_temperature >>= 1;
//     ds1820_temperature = (ds1820_temperature * 100) -  25  + (100 * 16 - remain * 100) / (16);

//     return ((float)ds1820_temperature/100);

// }
