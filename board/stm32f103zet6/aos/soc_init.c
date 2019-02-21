/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include "hal/hal.h"
#include "k_config.h"
#include "soc_init.h"
#include "stm32f1xx.h"
#include "hal_led_stm32f1.h"
#include "stm32f1xx_hal.h"

#if defined(__CC_ARM) && defined(__MICROLIB)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#elif defined(__ICCARM__)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#else
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#endif /* defined (__CC_ARM) && defined(__MICROLIB) */

uart_dev_t uart_0;

UART_MAPPING UART_MAPPING_TABLE[] =
{
    { PORT_UART_STD,     USART1, { USART1_IRQn,  0, 1,UART_OVERSAMPLING_16 } },
    { PORT_UART_AT,      USART3,  { USART3_IRQn , 0, 1,UART_OVERSAMPLING_16 } },
    // { PORT_UART_RS485,   UART7, { UART7_IRQn, 0, 1,UART_OVERSAMPLING_16 } },
    // { PORT_UART_SCANNER, UART4,  { UART4_IRQn,   0, 1,UART_OVERSAMPLING_16 } },
    // { PORT_UART_LORA,    UART5,  { UART5_IRQn,   0, 1,UART_OVERSAMPLING_16 } },
};
static void stduart_init(void);
void SystemClock_Config();
void Stm32_Clock_Init(uint32_t PLL);
void stm32_soc_init(void)
{
  HAL_Init();
  /**Configure the Systick interrupt time 
    */
   Stm32_Clock_Init(RCC_PLL_MUL9);
  //SystemClock_Config();

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/RHINO_CONFIG_TICKS_PER_SECOND);
  stduart_init();
  #ifdef CONFIG_NET_LWIP
    /*ethernet if init*/
    lwip_tcpip_init();
#endif
}
void Stm32_Clock_Init(uint32_t PLL)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStructure; 
    RCC_ClkInitTypeDef RCC_ClkInitStructure;
    
    RCC_OscInitStructure.OscillatorType=RCC_OSCILLATORTYPE_HSE;    	//时钟源为HSE
    RCC_OscInitStructure.HSEState=RCC_HSE_ON;                      	//打开HSE
	  RCC_OscInitStructure.HSEPredivValue=RCC_HSE_PREDIV_DIV1;		//HSE预分频
    RCC_OscInitStructure.PLL.PLLState=RCC_PLL_ON;					//打开PLL
    RCC_OscInitStructure.PLL.PLLSource=RCC_PLLSOURCE_HSE;			//PLL时钟源选择HSE
    RCC_OscInitStructure.PLL.PLLMUL=PLL; 							//主PLL倍频因子
    ret=HAL_RCC_OscConfig(&RCC_OscInitStructure);//初始化
	
    if(ret!=HAL_OK) while(1);
    
    //选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2
    RCC_ClkInitStructure.ClockType=(RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStructure.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;		//设置系统时钟时钟源为PLL
    RCC_ClkInitStructure.AHBCLKDivider=RCC_SYSCLK_DIV1;				//AHB分频系数为1
    RCC_ClkInitStructure.APB1CLKDivider=RCC_HCLK_DIV2; 				//APB1分频系数为2
    RCC_ClkInitStructure.APB2CLKDivider=RCC_HCLK_DIV1; 				//APB2分频系数为1
    ret=HAL_RCC_ClockConfig(&RCC_ClkInitStructure,FLASH_LATENCY_2);	//同时设置FLASH延时周期为2WS，也就是3个CPU周期。
		
    if(ret!=HAL_OK) while(1);
}

static void stduart_init(void)
{
  uart_0.port = 0;
  uart_0.config.baud_rate = 115200;
  uart_0.config.data_width = DATA_WIDTH_8BIT;
  uart_0.config.flow_control = FLOW_CONTROL_DISABLED;
  uart_0.config.mode = MODE_TX_RX;
  uart_0.config.parity = NO_PARITY;
  uart_0.config.stop_bits = STOP_BITS_1;

  hal_uart_init(&uart_0);
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  krhino_intrpt_enter();
  krhino_tick_proc();
  krhino_intrpt_exit();
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
// PUTCHAR_PROTOTYPE
// {
//   if (ch == '\n')
//   {
//     //hal_uart_send(&console_uart, (void *)"\r", 1, 30000);
//     hal_uart_send(&uart_0, (void *)"\r", 1, 30000);
//   }
//   hal_uart_send(&uart_0, &ch, 1, 30000);
//   return ch;
// }

/**
  * @brief  Retargets the C library scanf function to the USART.
  * @param  None
  * @retval None
  */
GETCHAR_PROTOTYPE
{
  /* Place your implementation of fgetc here */
  /* e.g. readwrite a character to the USART2 and Loop until the end of transmission */
  uint8_t ch = EOF;
  int32_t ret = -1;

  uint32_t recv_size;
  ret = hal_uart_recv_II(&uart_0, &ch, 1, &recv_size, HAL_WAIT_FOREVER);

  if (ret == 0)
  {
    return ch;
  }
  else
  {
    return -1;
  }
}
