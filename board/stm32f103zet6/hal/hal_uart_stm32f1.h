/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef __HAL_UART_STM32F1_H
#define __HAL_UART_STM32F1_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "aos/kernel.h"
#include "hal/hal.h"
#include "board.h"

typedef struct{    
    uint16_t       irqNumber;
    uint16_t       pPri;
    uint16_t       sPri;
    uint32_t       overSampling;
}uartAttribute;


typedef struct{
    PORT_UART_TYPE uartFuncP;
    void*          uartPhyP; 
    uartAttribute  attr;
}UART_MAPPING;

//Mapping Table is defined in soc_init.c
extern UART_MAPPING UART_MAPPING_TABLE[PORT_UART_SIZE];



#define MAX_BUF_UART_BYTES  1024


#ifdef __cplusplus
}
#endif

#endif /* __HAL_UART_STM32F1_H */
