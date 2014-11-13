
#include "lcd.h"

static void lcd_reset();
static void lcd_display_on(void);
static void lcd_display_off(void);
static void lcd_set_gamma(void);
static void lcd_set_power(void);
static void lcd_port_init(void);
static void LCD_FSMCConfig(void);

static unsigned short lcd_read_gram(unsigned int x,unsigned int y);
static void lcd_data_bus_test(void);
static void lcd_gram_test(void);


SemaphoreHandle_t xLcdMutex;
static unsigned short deviceid;


void lcd_init(void)
{
    printf("LCD init\r\n");

    // init IO
    lcd_port_init();
    LCD_FSMCConfig();

    lcd_reset();

    // check device ID
    deviceid = lcd_getdeviceid();
    printf("Device ID : %04X\r\n", deviceid);

    assert_param(deviceid == LCD_DRIVER_ID);

    // init LCD
    lcd_set_power();
    lcd_set_gamma();

    // run bus tests
    lcd_display_off();
    // lcd_data_bus_test();
    // lcd_gram_test();
    lcd_display_on();

    lcd_set_entry_mode_normal();
    xLcdMutex = xSemaphoreCreateMutex();

    printf("LCD init done\r\n");
}

void lcd_backlight(uint8_t enable)
{
#ifdef LCD_BL_PIN
    if(enable)
        GPIO_SetBits(LCD_BL_PORT, LCD_BL_PIN);
    else
        GPIO_ResetBits(LCD_BL_PORT, LCD_BL_PIN);
#else
    (void)enable;
#endif
}

void lcd_set_power(void)
{
    // general setup
    write_reg(ILI932x_START_OSCILLATOR, ILI932x_START_OSC);
    write_reg(ILI932x_DRIVER_OUTPUT_CONTROL, LCD_Y_SCAN_CONTROL_REG_VALUE);
    write_reg(ILI932x_DRIVIG_WAVE_CONTROL, ILI932x_DRIVIG_WAVE_CTRL_EOR|ILI932x_DRIVIG_WAVE_CTRL_BC);

    write_reg(ILI932x_GATE_SCAN_CONTROL1, LCD_X_SCAN_CONTROL_REG_VALUE|ILI932x_GATE_SCAN_CTRL1_NL5|ILI932x_GATE_SCAN_CTRL1_NL2|ILI932x_GATE_SCAN_CTRL1_NL1|ILI932x_GATE_SCAN_CTRL1_NL0);
    write_reg(ILI932x_GATE_SCAN_CONTROL2, ILI932x_GATE_SCAN_CTRL2_REV);
    write_reg(ILI932x_GATE_SCAN_CONTROL3, 0x0000);

    write_reg(ILI932x_DISP_CONTROL3, 0x0000);
    write_reg(ILI932x_DISP_CONTROL4, 0x0000);
    write_reg(ILI932x_RGB_DISP_IF_CTRL, ILI932x_RGB_IF_CTRL_ENC0|ILI932x_RGB_IF_CTRL_RIM0|ILI932x_RGB_IF_CTRL_RIM1);
    write_reg(ILI932x_FRAME_MARKER_POS, 0x0000);
    write_reg(ILI932x_RGB_DISP_IF_CTRL2, 0x0000);

    write_reg(ILI932x_POWER_CONTROL1, 0x0000);
    write_reg(ILI932x_POWER_CONTROL2, 0x0000);
    write_reg(ILI932x_POWER_CONTROL3, 0x0000);
    write_reg(ILI932x_POWER_CONTROL4, 0x0000);

/**
 * TODO - must find the datasheets for the display modules themselves.
 * the folowing are more to do with the display than the driver device.
 */
#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID
    write_reg(ILI932x_DISP_CONTROL2, ILI932x_DISP_CONT_FP1|ILI932x_DISP_CONT_FP2|ILI932x_DISP_CONT_BP2);
    write_reg(LGDP4532_REGULATOR_CONTROL, LGDP4532_REGULATOR_CTRL_RV0|LGDP4532_REGULATOR_CTRL_RV1);
    delay(200);
    write_reg(ILI932x_POWER_CONTROL1, LGDP4532_POWER_CTRL1_DK|ILI932x_POWER_CTRL1_AP1|ILI932x_POWER_CTRL1_BT1|ILI932x_POWER_CTRL1_BT2|LGDP4532_POWER_CTRL1_SAP0);
    write_reg(ILI932x_POWER_CONTROL2, ILI932x_POWER_CTRL2_DC02);
    write_reg(ILI932x_POWER_CONTROL3, 0x0000);
    write_reg(ILI932x_POWER_CONTROL4, ILI932x_POWER_CTRL4_VDV4|LGDP4532_POWER_CTRL4_VCM6|LGDP4532_POWER_CTRL4_VCM3|LGDP4532_POWER_CTRL4_VCM2|LGDP4532_POWER_CTRL4_VCM0);
    delay(10);
    write_reg(ILI932x_POWER_CONTROL3, ILI932x_POWER_CTRL3_PON);
    delay(10);
    write_reg(ILI932x_POWER_CONTROL1, LGDP4532_POWER_CTRL1_SAP1|ILI932x_POWER_CTRL1_BT1|ILI932x_POWER_CTRL1_BT2|ILI932x_POWER_CTRL1_AP1);
    write_reg(ILI932x_POWER_CONTROL4, ILI932x_POWER_CTRL4_VDV4|LGDP4532_POWER_CTRL4_VCOMG|ILI932x_POWER_CTRL4_VDV2|LGDP4532_POWER_CTRL4_VCM6|LGDP4532_POWER_CTRL4_VCM3|LGDP4532_POWER_CTRL4_VCM2|LGDP4532_POWER_CTRL4_VCM0);
    delay(10);
    write_reg(LGDP4532_EPROM_CONTROL2, LGDP4532_EPROM_CTRL2_VCMSEL1);
    write_reg(ILI932x_PANEL_IF_CONTROL1, ILI932x_PANEL_IF_CTRL1_RTNI1|ILI932x_PANEL_IF_CTRL1_RTNI7|ILI932x_PANEL_IF_CTRL1_DIVI0);
    write_reg(LGDP4532_PANEL_IF_CONTROL3, LGDP4532_PANEL_IF_CTRL3_MCPI0);
#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID
    write_reg(ILI932x_DISP_CONTROL2, ILI932x_DISP_CONT_FP1|ILI932x_DISP_CONT_BP0|ILI932x_DISP_CONT_BP1|ILI932x_DISP_CONT_BP2);
    delay(200);
    write_reg(ILI932x_POWER_CONTROL1, ILI932x_POWER_CTRL1_SAP|ILI932x_POWER_CTRL1_BT1|ILI932x_POWER_CTRL1_APE|ILI932x_POWER_CTRL1_AP0);
    write_reg(ILI932x_POWER_CONTROL2, ILI932x_POWER_CTRL2_VC0|ILI932x_POWER_CTRL2_VC1|ILI932x_POWER_CTRL2_VC2);
    delay(50);
    write_reg(ILI932x_POWER_CONTROL3, ILI932x_POWER_CTRL3_VRH1|ILI932x_POWER_CTRL3_VRH2|ILI932x_POWER_CTRL3_VRH3|ILI932x_POWER_CTRL3_VCIRE|ILI932x_POWER_CTRL3_PON);
    delay(50);
    write_reg(ILI932x_POWER_CONTROL4, ILI932x_POWER_CTRL4_VDV0|ILI932x_POWER_CTRL4_VDV1|ILI932x_POWER_CTRL4_VDV2|ILI932x_POWER_CTRL4_VDV3);
    write_reg(ILI932x_POWER_CONTROL7, ILI932x_POWER_CTRL7_VCM2|ILI932x_POWER_CTRL7_VCM0);
    write_reg(ILI932x_FR_AND_CLR_CTRL, ILI932x_FR_AND_CLR_CTRL_FRS0|ILI932x_FR_AND_CLR_CTRL_FRS1|ILI932x_FR_AND_CLR_CTRL_FRS3);
    write_reg(ILI932x_PANEL_IF_CONTROL1, ILI932x_PANEL_IF_CTRL1_RTNI4);
    write_reg(ILI932x_PANEL_IF_CONTROL2, ILI932x_PANEL_IF_CTRL2_NOWI1|ILI932x_PANEL_IF_CTRL2_NOWI2);
#endif

    // other general setup
    write_reg(ILI932x_PARTIAL_IMG1_POS, 0x0000);
    write_reg(ILI932x_PARTIAL_IMG1_START_ADDR, 0x0000);
    write_reg(ILI932x_PARTIAL_IMG1_END_ADDR, 0x0000);
    write_reg(ILI932x_PARTIAL_IMG2_POS, 0x0000);
    write_reg(ILI932x_PARTIAL_IMG2_START_ADDR, 0x0000);
    write_reg(ILI932x_PARTIAL_IMG2_END_ADDR, 0x0000);
    write_reg(ILI932x_RESIZE_CONTROL, 0x0000);

    delay(50);
}

void LCD_FSMCConfig(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAM_init;
    FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInit;

    /*-- FSMC Configuration ----------------------------------------------*/
    FSMC_NORSRAMTimingInit.FSMC_AddressSetupTime = 2;             /* ��ַ����ʱ��  */
    FSMC_NORSRAMTimingInit.FSMC_AddressHoldTime = 1;              /* ��ַ����ʱ��  */
    FSMC_NORSRAMTimingInit.FSMC_DataSetupTime = 3;                /* ��ݽ���ʱ��  */
    FSMC_NORSRAMTimingInit.FSMC_BusTurnAroundDuration = 0;        /* ���߷�תʱ��  */
    FSMC_NORSRAMTimingInit.FSMC_CLKDivision = 0;                  /* ʱ�ӷ�Ƶ      */
    FSMC_NORSRAMTimingInit.FSMC_DataLatency = 0;                  /* ��ݱ���ʱ��  */
    FSMC_NORSRAMTimingInit.FSMC_AccessMode = FSMC_AccessMode_A;   /* FSMC ����ģʽ */

    /* Color LCD configuration ------------------------------------
       LCD configured as follow:
       - Data/Address MUX = Disable
       - Memory Type = SRAM
       - Data Width = 16bit
       - Write Operation = Enable
       - Extended Mode = Enable
       - Asynchronous Wait = Disable */
    FSMC_NORSRAM_init.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAM_init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAM_init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAM_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAM_init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAM_init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAM_init.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAM_init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAM_init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAM_init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAM_init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAM_init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAM_init.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInit;
    FSMC_NORSRAM_init.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInit;
    FSMC_NORSRAM_init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;

    FSMC_NORSRAMInit(&FSMC_NORSRAM_init);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

void lcd_port_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    uint16_t pins[] = FSMC_PINS;
    GPIO_TypeDef* ports[] = FSMC_PORTS;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
               RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
               RCC_APB2Periph_AFIO, ENABLE);

    // FSMC pins
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    for(uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++)
    {
        GPIO_InitStructure.GPIO_Pin = pins[i];
        GPIO_Init(ports[i], &GPIO_InitStructure);
    }

    // reset pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = LCD_NRST_PIN;
    GPIO_Init(LCD_NRST_PORT, &GPIO_InitStructure);

    // Backlight pin
#ifdef LCD_BL_PIN
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = LCD_BL_PIN;
    GPIO_Init(LCD_BL_PORT, &GPIO_InitStructure);
#endif
}

void lcd_reset()
{
    GPIO_ResetBits(LCD_NRST_PORT, LCD_NRST_PIN);
    delay(300);
    GPIO_SetBits(LCD_NRST_PORT, LCD_NRST_PIN);
    delay(100);
}

void lcd_set_gamma(void)
{
#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID
    write_reg(0x30,0x0000);
    write_reg(0x31,0x0502);
    write_reg(0x32,0x0307);
    write_reg(0x33,0x0305);
    write_reg(0x34,0x0004);
    write_reg(0x35,0x0402);
    write_reg(0x36,0x0707);
    write_reg(0x37,0x0503);
    write_reg(0x38,0x1505);
    write_reg(0x39,0x1505);
#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID
    write_reg(0x30, 0x0001);
    write_reg(0x31, 0x0101);
    write_reg(0x32, 0x0101);
    write_reg(0x35, 0x0202);
    write_reg(0x36, 0x0A00);
    write_reg(0x37, 0x0507);
    write_reg(0x38, 0x0101);
    write_reg(0x39, 0x0101);
    write_reg(0x3C, 0x0202);
    write_reg(0x3D, 0x0000);
#endif
}

void lcd_display_on(void)
{
#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D0);
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_BASEE|ILI932x_DISP_CONT_DTE|ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D1|ILI932x_DISP_CONT_D0);
#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_D0);
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D0);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D1|ILI932x_DISP_CONT_D0);
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_DTE|ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D1|ILI932x_DISP_CONT_D0);
    delay(10);
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_BASEE|ILI932x_DISP_CONT_DTE|ILI932x_DISP_CONT_GON|ILI932x_DISP_CONT_D1|ILI932x_DISP_CONT_D0);
#endif
}

void lcd_display_off(void)
{
    write_reg(ILI932x_DISP_CONTROL1, ILI932x_DISP_CONT_D0);
}

unsigned int lcd_getdeviceid(void)
{
    unsigned int deviceid;
    read_reg(ILI932x_DEVICE_ID, deviceid);
    return deviceid;
}

unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
    unsigned short temp;
    lcd_set_cursor(y, x);
    lcd_rw_gram();
    /* dummy read */
    temp = read_data();
    temp = read_data();
    return temp;
}

void lcd_data_bus_test(void)
{
    unsigned short temp1;
    unsigned short temp2;
    printf("bus test...\r\n");
    write_reg(ILI932x_ENTRY_MODE, ILI932x_EM_BGR | ILI932x_EM_HORIZONTAL_INC | ILI932x_EM_VERTICAL_INC);

    // Write Alternating Bit Values
    lcd_set_cursor(0,0);
    lcd_rw_gram();
    write_data(0x5555);
    write_data(0xAAAA);

    // Read it back and check
    temp1 = lcd_read_gram(0, 0);
    temp2 = lcd_read_gram(1, 0);
    printf("expect %X, read %X\r\n", 0x5555, temp1);
    printf("expect %X, read %X\r\n", 0xaaaa, temp2);
    assert_param((temp1 == 0x5555) && (temp2 == 0xAAAA));
    printf("pass\r\n");
}

void lcd_gram_test(void)
{
    unsigned short temp;
    unsigned int test_x;
    unsigned int test_y;

    printf("GRAM test...\r\n");

    temp=0;

    write_reg(ILI932x_ENTRY_MODE, ILI932x_EM_BGR | ILI932x_EM_HORIZONTAL_INC | ILI932x_EM_VERTICAL_INC);
    lcd_set_cursor(0,0);
    lcd_rw_gram();

    //test 320*240 Memory locations
    for(test_y=0; test_y < LCD_HEIGHT*LCD_WIDTH; test_y++)
    {
        write_data(temp);
        temp++;
    }

    temp=0;
    for(test_y = 0; test_y < LCD_HEIGHT; test_y++)
    {
        for(test_x = 0; test_x < LCD_WIDTH; test_x++)
            assert_param(lcd_read_gram(test_x, test_y) == temp++);
    }
    printf("pass\r\n");
}

