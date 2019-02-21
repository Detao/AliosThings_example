/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "hal_uart_stm32f1.h"
// #ifdef TINY_ENGINE
// #define AOS_START_STACK 1536 + 1024*14
// #else
#define AOS_START_STACK 1024
// #endif

static ktask_t demo_task_obj;
cpu_stack_t demo_task_buf[AOS_START_STACK];


ktask_t *g_aos_init;

static kinit_t kinit;

extern int application_start(int argc, char **argv);
extern int aos_framework_init(void);
extern void board_init(void);

static void var_init()
{
    kinit.argc = 0;
    kinit.argv = NULL;
    kinit.cli_enable = 1;
}
extern void hw_start_hal(void);

#include "hal/soc/uart.h"
#include "hal/hal_uart_stm32f1.h"
#include "board.h"

// UART_HandleTypeDef UART1_Handler; //UART句柄
// void uart_init()
// {
//     UART1_Handler.Instance=USART1;					    //USART1
// 	UART1_Handler.Init.BaudRate=115200;				    //波特率
// 	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
// 	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
// 	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
// 	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
// 	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
// 	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()会使能UART1
// }
// void test_uart()
// {
//    uart_init();
   
   
// }

static void sys_init(void)
{
  
    stm32_soc_init();
#ifdef BOOTLOADER
    main();
#else
    hw_start_hal();
    board_init();
    var_init();
    aos_kernel_init(&kinit);
#endif
}


static void sys_start(void)
{
    aos_init();
    //krhino_task_dyn_create(&g_aos_init, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, (task_entry_t)sys_init, 1);
    krhino_task_create(&demo_task_obj, "aos-init", 0,AOS_DEFAULT_APP_PRI, 
        0, demo_task_buf, AOS_START_STACK, (task_entry_t)sys_init, 1);
    
    aos_start();
}
// void FSMC_SRAM_Init(void)
// {	
// 	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
// 	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
// 	GPIO_InitTypeDef  GPIO_InitStructure;
 
//  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);
//   	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);
  
// 	GPIO_InitStructure.GPIO_Pin = 0xFF33; 			 	//PORTD复用推挽输出 
//  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出
//  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  	GPIO_Init(GPIOD, &GPIO_InitStructure);

 
// 	GPIO_InitStructure.GPIO_Pin = 0xFF83; 			 	//PORTE复用推挽输出 
//  	GPIO_Init(GPIOE, &GPIO_InitStructure);

//  	GPIO_InitStructure.GPIO_Pin = 0xF03F; 			 	//PORTD复用推挽输出 
//  	GPIO_Init(GPIOF, &GPIO_InitStructure);

// 	GPIO_InitStructure.GPIO_Pin = 0x043F; 			 	//PORTD复用推挽输出 
//  	GPIO_Init(GPIOG, &GPIO_InitStructure);
 
	 			  	  
//  	readWriteTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK 1/36M=27ns
//     readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
//     readWriteTiming.FSMC_DataSetupTime = 0x03;		 //数据保持时间（DATAST）为3个HCLK 4/72M=55ns(对EM的SRAM芯片)	 
//     readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
//     readWriteTiming.FSMC_CLKDivision = 0x00;
//     readWriteTiming.FSMC_DataLatency = 0x00;
//     readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    

 
//     FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;//  这里我们使用NE3 ，也就对应BTCR[4],[5]。
//     FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; 
//     FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
//     FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit  
//     FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
//     FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
// 	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
//     FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
//     FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
//     FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//存储器写使能 
//     FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  
//     FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable; // 读写使用相同的时序
//     FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
//     FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
//     FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &readWriteTiming; //读写同样时序

//     FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

//    	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);  // 使能BANK3										  
											
// }
int main(void)
{
    //FSMC_SRAM_Init();
    aos_heap_set();
    sys_start();
    return 0;
}


