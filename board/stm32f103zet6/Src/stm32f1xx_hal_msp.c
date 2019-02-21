/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : stm32f1xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

 HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
		__HAL_RCC_AFIO_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  if(huart->Instance==USART3)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_10;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_11;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);

  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  if(huart->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_ENABLE();

    HAL_GPIO_DeInit(GPIOB,GPIO_PIN_10|GPIO_PIN_11);
  }

}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{	
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_FSMC_CLK_ENABLE();			//Ê¹ÄÜFSMCÊ±ÖÓ
	__HAL_RCC_GPIOD_CLK_ENABLE();			//Ê¹ÄÜGPIODÊ±ÖÓ
	__HAL_RCC_GPIOE_CLK_ENABLE();			//Ê¹ÄÜGPIOEÊ±ÖÓ
	__HAL_RCC_GPIOG_CLK_ENABLE();			//Ê¹ÄÜGPIOGÊ±ÖÓ
	
	//³õÊ¼»¯PD0,1,4,5,8,9,10,14,15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|\
					 GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP; 		//ÍÆÍì¸´ÓÃ
	GPIO_Initure.Pull=GPIO_PULLUP;			//ÉÏÀ­
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;		//¸ßËÙ
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);     //³õÊ¼»¯
	
	//³õÊ¼»¯PE7,8,9,10,11,12,13,14,15
	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
                     GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	//³õÊ¼»¯PG0,12
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);
}
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef GPIO_Initure;
  if(htim->Instance == TIM2){
    __HAL_RCC_TIM2_CLK_ENABLE();			//Ê¹ÄÜ¶¨Ê±Æ÷3
		//__HAL_AFIO_REMAP_TIM2_PARTIAL();		//TIM3Í¨µÀÒý½Å²¿·ÖÖØÓ³ÉäÊ¹ÄÜ
		__HAL_RCC_GPIOA_CLK_ENABLE();			//¿ªÆôGPIOBÊ±ÖÓ
		
		GPIO_Initure.Pin=GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;           	//PB5
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//¸´ÓÃÍÆÍìÊä³ö
		GPIO_Initure.Pull=GPIO_PULLUP;          //ÉÏÀ­
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //¸ßËÙ
		HAL_GPIO_Init(GPIOA,&GPIO_Initure); 	

  }else if(htim->Instance==TIM3){
		__HAL_RCC_TIM3_CLK_ENABLE();			//Ê¹ÄÜ¶¨Ê±Æ÷3
		__HAL_AFIO_REMAP_TIM3_PARTIAL();		//TIM3部分映射
		__HAL_RCC_GPIOB_CLK_ENABLE();			//¿ªÆôGPIOBÊ±ÖÓ
    __HAL_RCC_GPIOA_CLK_ENABLE();			//¿ªÆôGPIOBÊ±ÖÓ
		
		GPIO_Initure.Pin=GPIO_PIN_6 | GPIO_PIN_7;           	//PB5
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//¸´ÓÃÍÆÍìÊä³ö
		GPIO_Initure.Pull=GPIO_PULLUP;          //ÉÏÀ­
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //¸ßËÙ
		HAL_GPIO_Init(GPIOA,&GPIO_Initure); 	
    GPIO_Initure.Pin=GPIO_PIN_5 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	}else if(htim->Instance==TIM4){
    __HAL_RCC_TIM4_CLK_ENABLE();			//Ê¹ÄÜ¶¨Ê±Æ÷3
	//	__HAL_AFIO_REMAP_TIM4_PARTIAL();		//TIM3Í¨µÀÒý½Å²¿·ÖÖØÓ³ÉäÊ¹ÄÜ
		__HAL_RCC_GPIOB_CLK_ENABLE();			//¿ªÆôGPIOBÊ±ÖÓ
		
		GPIO_Initure.Pin=GPIO_PIN_6 |GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;           	//PB5
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//¸´ÓÃÍÆÍìÊä³ö
		GPIO_Initure.Pull=GPIO_PULLUP;          //ÉÏÀ­
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //¸ßËÙ
		HAL_GPIO_Init(GPIOB,&GPIO_Initure); 	

  }

}
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
