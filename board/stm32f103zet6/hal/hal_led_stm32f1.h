#ifndef  __HAL_LED_STM32F1_H
#define  __HAL_LED_STM32F1_H
//#include "sys.h"
#define LED0 PBout(5) // PB5
#define LED1 PEout(5) // PE5	

void LED_Init();
#endif