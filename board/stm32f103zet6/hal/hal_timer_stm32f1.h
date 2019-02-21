/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef __HAL_TIMER_STM32F1_H
#define __HAL_TIMER_STM32F1_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "aos/kernel.h"

#define PORT_TIMER1 1
#define PORT_TIMER2 2
#define PORT_TIMER3 3
#define PORT_TIMER4 4
#define PORT_TIMER7 7
typedef struct TIM_HandleTypeDef
{
    TIM_TypeDef*     Instance;
	TIM_TimeBaseInitTypeDef  Init;
}TIM_HandleTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* __HAL_TIMER_STM32F1_H */
