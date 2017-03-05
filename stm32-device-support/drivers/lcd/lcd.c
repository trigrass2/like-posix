/*
 * Copyright (c) 2015 Michael Stuart.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the like-posix project, <https://github.com/drmetal/like-posix>
 *
 * based heavily on the code by Zizzle at: https://github.com/Zizzle/stm32_freertos_example/tree/master/drivers
 *
 * Author: Michael Stuart <spaceorbot@gmail.com>
 *
 */

#include "lcd.h"
#include "cutensils.h"

#if LCD_ROTATION == ROTATE_0_DEGREES
#pragma message "building LCD rotated by 0 degrees"
#elif LCD_ROTATION == ROTATE_90_DEGREES
#pragma message "building LCD rotated by 90 degrees"
#elif LCD_ROTATION == ROTATE_180_DEGREES
#pragma message "building LCD rotated by 180 degrees"
#elif LCD_ROTATION == ROTATE_270_DEGREES
#pragma message "building LCD rotated by 270 degrees"
#endif

#if LCD_DRIVER_ID == LGDP4532_DEVICE_ID
#pragma message "building for LCD controller LGDP4532"
#elif LCD_DRIVER_ID == ILI9325_DEVICE_ID
#pragma message "building for LCD controller ILI9325"
#endif

static void lcd_reset();
static void lcd_display_on(void);
static void lcd_display_off(void);
static void lcd_set_gamma(void);
static void lcd_set_power(void);
static void LCD_FSMCConfig(void);

static void lcd_gpio_init();
static unsigned short lcd_read_gram(unsigned int x,unsigned int y);
static void lcd_data_bus_test(void);
static void lcd_gram_test(void);


logger_t tftlog;
SemaphoreHandle_t xLcdMutex;
static unsigned short deviceid;


void lcd_init(void)
{
    log_init(&tftlog, "tft");
    log_debug(&tftlog, "initializing");

    // init IO
    LCD_FSMCConfig();
    lcd_gpio_init();

    lcd_reset();

    // check device ID
    deviceid = lcd_getdeviceid();
    log_debug(&tftlog, "device ID: %04X", deviceid);

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

    log_debug(&tftlog, "done...");
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
	SRAM_HandleTypeDef hsram;

#if defined(FMC_NORSRAM_TimingTypeDef)
    FMC_NORSRAM_TimingTypeDef Timing;
#else
    FSMC_NORSRAM_TimingTypeDef Timing;
#endif

    hsram.Instance  = FSMC_NORSRAM_DEVICE;
    hsram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;
    hsram.State = HAL_SRAM_STATE_RESET;

    // clock enable
#if FAMILY==STM32F4
    RCC->AHB3ENR  = ((uint32_t)(0x01<<0));
#elif FAMILY==STM32F1
    RCC->AHBENR  = ((uint32_t)(0x01<<8));
#endif

    /* SRAM device configuration */
    Timing.AddressSetupTime       = 2;
    Timing.AddressHoldTime        = 1;
    Timing.DataSetupTime          = 2;//3;
    Timing.BusTurnAroundDuration  = 1;//0;
    Timing.CLKDivision            = 2;//2;
    Timing.DataLatency            = 2;//2;
    Timing.AccessMode             = FSMC_ACCESS_MODE_A;

    hsram.Init.NSBank             = FSMC_NORSRAM_BANK1;
    hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;

    HAL_SRAM_DeInit(&hsram);
    HAL_StatusTypeDef ret = HAL_SRAM_Init(&hsram, &Timing, &Timing);
    assert_true(ret == HAL_OK);
    HAL_SRAM_WriteOperation_Enable(&hsram);
}

void lcd_gpio_init()
{
    GPIO_InitTypeDef gpio_init;
    uint16_t pins[] = FSMC_PINS;
    GPIO_TypeDef* ports[] = FSMC_PORTS;

    // FSMC pins
    gpio_init.Speed = GPIO_SPEED_HIGH;
    gpio_init.Mode  = GPIO_MODE_AF_PP;
    gpio_init.Pull  = GPIO_NOPULL;
#if FAMILY==STM32F4
    gpio_init.Alternate = GPIO_AF12_FSMC;
#endif

    for(uint8_t i = 0; i < sizeof(pins)/sizeof(pins[0]); i++)
    {
        gpio_init.Pin = pins[i];
        HAL_GPIO_Init(ports[i], &gpio_init);
    }

    gpio_init.Mode  = GPIO_MODE_OUTPUT_PP;

    // reset pin
    gpio_init.Pin = LCD_NRST_PIN;
    HAL_GPIO_Init(LCD_NRST_PORT, &gpio_init);
}

void lcd_reset()
{
    HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, GPIO_PIN_RESET);
    delay(300);
    HAL_GPIO_WritePin(LCD_NRST_PORT, LCD_NRST_PIN, GPIO_PIN_SET);
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
    log_debug(&tftlog, "bus test...");
    write_reg(ILI932x_ENTRY_MODE, ILI932x_EM_BGR | ILI932x_EM_HORIZONTAL_INC | ILI932x_EM_VERTICAL_INC);

    // Write Alternating Bit Values
    lcd_set_cursor(0,0);
    lcd_rw_gram();
    write_data(0x5555);
    write_data(0xAAAA);

    // Read it back and check
    temp1 = lcd_read_gram(0, 0);
    temp2 = lcd_read_gram(1, 0);
    log_debug(&tftlog, "expect %X, read %X", 0x5555, temp1);
    log_debug(&tftlog, "expect %X, read %X", 0xaaaa, temp2);
    assert_param((temp1 == 0x5555) && (temp2 == 0xAAAA));
    log_debug(&tftlog, "pass");
}

void lcd_gram_test(void)
{
    unsigned short temp;
    unsigned int test_x;
    unsigned int test_y;

    log_debug(&tftlog, "GRAM test...");

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
    log_debug(&tftlog, "pass");
}

