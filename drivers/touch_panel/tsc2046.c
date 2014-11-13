
#include "tsc2046.h"


#define TSC2046_START  0x80
#define TSC2046_CH_X  0x50
#define TSC2046_CH_Y  0x10
#define TSC2046_READ_X  (TSC2046_START | TSC2046_CH_X)
#define TSC2046_READ_Y  (TSC2046_START | TSC2046_CH_Y)
#define TSC2046_MAX_X  4096
#define TSC2046_MAX_Y  4096

#if TSC2046_FLIP_Y
#define TSC2046_Y_OFFSET 239
#define TSC2046_PIX_Y  -240
#else
#define TSC2046_Y_OFFSET 0
#define TSC2046_PIX_Y  240
#endif
#if TSC2046_FLIP_X
#define TSC2046_X_OFFSET 319
#define TSC2046_PIX_X  -320
#else
#define TSC2046_X_OFFSET 0
#define TSC2046_PIX_X  320
#endif

#define tsc2046_select()      GPIO_ResetBits(TSC2046_NCS_PORT, TSC2046_NCS_PIN)
#define tsc2046_deselect()    GPIO_SetBits(TSC2046_NCS_PORT, TSC2046_NCS_PIN)
#define tsc2046_irq()         GPIO_ReadInputDataBit(TSC2046_IRQ_PORT, TSC2046_IRQ_PIN)
/**
 * http://e2e.ti.com/support/other_analog/touch/f/750/t/177249.aspx
 * ignore busy signal, is redundant and not documented in:
 * http://www.ti.com/lit/ds/symlink/tsc2046.pdf
 */
#define tsc2046_busy()        GPIO_ReadInputDataBit(TSC2046_BUSY_PORT, TSC2046_BUSY_PIN)


static void tsc2046_write(uint8_t Data);
static uint8_t tsc2046_read(void);


void tsc2046_init()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // configure SPI port
   // enable spi clock
   #if ((TSC2046_SPI == 3)||(TSC2046_SPI == 2))
      RCC_APB1PeriphClockCmd(TSC2046_SPI_CLOCK, ENABLE);
   #elif (TSC2046_SPI == 1)
      RCC_APB2PeriphClockCmd(TSC2046_SPI_CLOCK, ENABLE);
   #endif

    SPI_InitTypeDef spi_init =
    {
        SPI_Direction_2Lines_FullDuplex,
        SPI_Mode_Master,
        SPI_DataSize_8b,
        SPI_CPOL_Low,
        SPI_CPHA_1Edge,
        SPI_NSS_Soft,
        TSC2046_SPI_PRESC,
        SPI_FirstBit_MSB,
        1
    };

    SPI_Init(TSC2046_SPI_PERIPH, &spi_init);
    SPI_Cmd(TSC2046_SPI_PERIPH, ENABLE);

    // configure IO's
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // input, pullup
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_InitStructure.GPIO_Pin = TSC2046_BUSY_PIN;
//    GPIO_Init(TSC2046_BUSY_PORT, &GPIO_InitStructure);

    // input, floating
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = TSC2046_MISO_PIN;
    GPIO_Init(TSC2046_MISO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_IRQ_PIN;
    GPIO_Init(TSC2046_IRQ_PORT, &GPIO_InitStructure);

    // output push pull
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = TSC2046_NCS_PIN;
    GPIO_Init(TSC2046_NCS_PORT, &GPIO_InitStructure);

    // output push pull alternate function
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = TSC2046_SCK_PIN;
    GPIO_Init(TSC2046_SCK_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_MOSI_PIN;
    GPIO_Init(TSC2046_MOSI_PORT, &GPIO_InitStructure);
}

void tsc2046_deinit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    SPI_I2S_DeInit(TSC2046_SPI_PERIPH);

    // configure IO's
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_InitStructure.GPIO_Pin = TSC2046_BUSY_PIN;
//    GPIO_Init(TSC2046_BUSY_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_MISO_PIN;
    GPIO_Init(TSC2046_MISO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_IRQ_PIN;
    GPIO_Init(TSC2046_IRQ_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_NCS_PIN;
    GPIO_Init(TSC2046_NCS_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_SCK_PIN;
    GPIO_Init(TSC2046_SCK_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = TSC2046_MOSI_PIN;
    GPIO_Init(TSC2046_MOSI_PORT, &GPIO_InitStructure);
}


void tsc2046_write(uint8_t Data)
{
  while(SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(TSC2046_SPI_PERIPH, Data);
}

uint8_t tsc2046_read(void)
{
  while(SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(TSC2046_SPI_PERIPH, 0);
  while (SPI_I2S_GetFlagStatus(TSC2046_SPI_PERIPH, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(TSC2046_SPI_PERIPH);
}

int16_t tsc2046_x()
{
    int16_t x;
    if(tsc2046_irq() == SET)
        return -1;

    tsc2046_select();
    tsc2046_write(TSC2046_READ_X);
    x = ((tsc2046_read() & 0x7F) << 8);
    x |= tsc2046_read();
    x >>= 3;
    x = ((x * TSC2046_PIX_X) / TSC2046_MAX_X) + TSC2046_X_OFFSET;
    tsc2046_deselect();

    return x;
}

int16_t tsc2046_y()
{
    int16_t y;    
    if(tsc2046_irq() == SET)
        return -1;

    tsc2046_select();
    tsc2046_write(TSC2046_READ_Y);
    y = ((tsc2046_read() & 0x7F) << 8);
    y |= tsc2046_read();
    y >>= 3;
    y = ((y * TSC2046_PIX_Y) / TSC2046_MAX_Y) + TSC2046_Y_OFFSET;
    tsc2046_deselect();

    return y;
}
