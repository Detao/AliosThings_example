/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "hal/hal.h"
#include "stm32f10x.h"
#include "hal_timer_stm32f1.h"



/* Init and deInit function for adc1 */

/* function used to transform hal para to stm32f1 para */
/*int32_t timer_reload_mode_transform(uint8_t reload_mode_hal, uint8_t *reload_mode_stm32f1);*/

/* handle for adc */
// TIM_HandleTypeDef timer7_handle;
// uint32_t uwPrescalerValue = 0;
// hal_timer_cb_t pFun_timer7 = NULL;
// void * arg_timer7 = NULL;
extern uint16_t USART3_RX_STA;
void TIM7_IRQHandler(void)
{ 	
     krhino_intrpt_enter();
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		USART3_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);  //清除TIM7更新中断标志    
		TIM_Cmd(TIM7, DISABLE);  //关闭TIM7 
	}	 
    krhino_intrpt_exit();   
}

// int32_t hal_timer_init(timer_dev_t *tim)
// {
//     int32_t ret = -1;

//     if (tim == NULL) {
//        return -1;
//     }

//     /*init adc handle*/
//     memset(&timer7_handle, 0, sizeof(timer7_handle));

//     switch (tim->port) {
//         case PORT_TIMER7:
//       	    tim->priv = &timer7_handle;
//             ret = timer7_init(tim);
//             break;

//     /* if other timer exist add init code here */

//         default:
//             break;
//     }

//     return ret;

// }

int32_t timer7_init()
{
    int32_t ret = 0;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    // pFun_timer3 = tim->config.cb;
    // arg_timer3 = tim->config.arg;	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
    /* Set TIMx instance */

  	/* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
    TIM_TimeBaseStructure.TIM_Period            = 1000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler        = 7200-1;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    /*
    ret = timer_reload_mode_transform(tim->config.reload_mode, 
	                             (uint8_t *)&timer3_handle.Init.AutoReloadPreload);*/
	
   TIM_TimeBaseInit(TIM7,&TIM_TimeBaseStructure);
    
    // if (ret == 0 && (tim->config.reload_mode == TIMER_RELOAD_MANU)) {
    //     ret = HAL_TIM_OnePulse_Init(&timer3_handle, TIM_OPMODE_SINGLE);
    // }
    TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE); //使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM7,ENABLE);//开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	return ret;
}

