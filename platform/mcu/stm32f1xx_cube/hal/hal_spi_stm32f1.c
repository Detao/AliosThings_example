/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "hal/hal.h"
#include "stm32f1xx_hal.h"
#include "hal_spi_stm32f1.h"
#include "stm32f1xx_hal_spi.h"

#ifdef HAL_SPI_MODULE_ENABLED 
/* Init and deInit function for spi1 */
static int32_t spi1_init(spi_dev_t *spi);
static int32_t spi1_DeInit(void);

/* function used to transform hal para to stm32f1 para */
static int32_t spi_mode_transform(uint32_t mode_hal, uint32_t *mode_stm32f1);
static int32_t spi_freq_transform(uint32_t freq_hal, uint32_t *BaudRatePrescaler_stm32f1_stm32f1);

/* handle for spi */
SPI_HandleTypeDef spi1_handle;
SPI_InitTypeDef  SPI_InitStructure;

int32_t hal_spi_init(spi_dev_t *spi)
{
    int32_t ret = -1;

    if (spi == NULL) {
        return -1;
    }

    /*init spi handle*/
    memset(&spi1_handle, 0, sizeof(spi1_handle));

    switch (spi->port) {
        case PORT_SPI1:
            spi->priv = &spi1_handle;
            ret = spi1_init(spi);
            break;

    /* if ohter spi exist add init code here */

        default:
            break;
    }

    return ret;
}

int32_t hal_spi_send(spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret = -1;

    if((spi != NULL) && (data != NULL)) {
        ret = HAL_SPI_Transmit((SPI_HandleTypeDef *)spi->priv,
             (uint8_t *)data, size, timeout);
    }

    return ret;
}

int32_t hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret = -1;

    if((spi != NULL) && (data != NULL)) {
          ret = HAL_SPI_Receive((SPI_HandleTypeDef *)spi->priv,
               (uint8_t *)data, size, timeout);
    }

    return ret;
}

int32_t hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data,
                          uint16_t size, uint32_t timeout)
{
    int32_t ret = -1;

    if((spi != NULL) && (tx_data != NULL) && (rx_data != NULL)) {
          ret = HAL_SPI_TransmitReceive((SPI_HandleTypeDef *)spi->priv,
                (uint8_t *)tx_data, (uint8_t *)rx_data, size, timeout);
    }

    return ret;
}

int32_t hal_spi_finalize(spi_dev_t *spi)
{
    int32_t ret = -1;

    if (spi == NULL) {
        return -1;
    }

    switch (spi->port) {
        case PORT_SPI1:
            ret = spi1_DeInit();
            break;
    /* if other spi exist add Deinit code here */

        default:
            break;
    }

    return ret;
}
void SPI2_MSP_Init(spi_dev_t *spi)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    if(spi->Instance == SPI2)
    {
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15上拉

    }

}
int32_t spi1_init(spi_dev_t *spi)
{
    int32_t ret = 0;

    spi1_handle.Instance = SPI1;

    ret = spi_mode_transform(spi->config.mode, &spi1_handle.Init.Mode);
    ret = spi_freq_transform(spi->config.freq, &spi1_handle.Init.BaudRatePrescaler);
    if (ret != 0) {
        return -1;
    }

    /* Initialize other parameters in struction SPI_InitTypeDef */

    /* init spi */
    ret = HAL_SPI_Init(&spi1_handle);

    return ret;
}

int32_t spi1_DeInit(void)
{
    int32_t ret = -1;

    /* spi1 deinitialization */
    ret = HAL_SPI_DeInit(&spi1_handle);

    return ret;
}

static int32_t spi_mode_transform(uint32_t mode_hal, uint32_t *mode_stm32f1)
{
    uint32_t mode = 0;
    int32_t	ret = 0;

    if(mode_hal == HAL_SPI_MODE_MASTER)
    {
        mode = SPI_MODE_MASTER;
    }
    else if(mode_hal == HAL_SPI_MODE_MASTER)
    {
        mode = SPI_MODE_MASTER;
    }
    else
    {
        ret = -1;
    }

    if(ret == 0)
    {
        *mode_stm32f1 = mode;
    }

    return ret;
}

static int32_t spi_freq_transform(uint32_t freq_hal, uint32_t *BaudRatePrescaler_stm32f1)
{
    int32_t	ret = 0;

    /* calc BaudRatePrescaler according to freq_hal */

    return ret;
}
#endif
