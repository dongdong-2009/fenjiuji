/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: stm32f1xx_hal_msp.c
      ��������: 2016.09.25
      ��	   �ߣ�like
      ����˵����
      ����˵����
      �޸ļ�¼��
*******************************************************************************/
#include "stm32f1xx_hal.h"


extern __IO ITStatus Uart1Ready;
extern __IO ITStatus Uart2Ready;
extern __IO ITStatus Uart3Ready;
extern __IO ITStatus Uart4Ready;

/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if (huart->Instance == USART1) {
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* Enable USART1 clock */
		__HAL_RCC_USART1_CLK_ENABLE();

		GPIO_InitStruct.Pin	  = GPIO_PIN_9;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLUP;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* UART1 RX GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode	  = GPIO_MODE_INPUT;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	}

	if (huart->Instance == USART2) {
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/* Enable USART2 clock */
		__HAL_RCC_USART2_CLK_ENABLE();

		GPIO_InitStruct.Pin	  = GPIO_PIN_2;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLUP;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* UART RX GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}


	if (huart->Instance == USART3) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
		//__HAL_RCC_AFIO_CLK_ENABLE();
		/* Enable USART3 clock */
		__HAL_RCC_USART3_CLK_ENABLE();

		GPIO_InitStruct.Pin	  = GPIO_PIN_10;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLUP;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* UART RX GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART3_IRQn, 0, 2);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
	}


	if (huart->Instance == UART4) {
		__HAL_RCC_GPIOC_CLK_ENABLE();

		//__HAL_RCC_AFIO_CLK_ENABLE();

		/* Enable USART3 clock */
		__HAL_RCC_UART4_CLK_ENABLE();

		GPIO_InitStruct.Pin	  = GPIO_PIN_10;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLUP;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;

		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* UART RX GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(UART4_IRQn, 0, 2);
		HAL_NVIC_EnableIRQ(UART4_IRQn);
	}

}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1) {
		__HAL_RCC_USART1_FORCE_RESET();
		__HAL_RCC_USART1_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}

	if (huart->Instance == USART2) {
		__HAL_RCC_USART2_FORCE_RESET();
		__HAL_RCC_USART2_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);

		HAL_NVIC_DisableIRQ(USART2_IRQn);
	}

	if (huart->Instance == USART3) {
		__HAL_RCC_USART3_FORCE_RESET();
		__HAL_RCC_USART3_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);

		HAL_NVIC_DisableIRQ(USART3_IRQn);
	}

	if (huart->Instance == UART4) {
		__HAL_RCC_UART4_FORCE_RESET();
		__HAL_RCC_UART4_RELEASE_RESET();

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10);
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11);

		HAL_NVIC_DisableIRQ(UART4_IRQn);
	}
}



/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *	    you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if (UartHandle->Instance == USART1) {
		Uart1Ready = SET;
	}

	if (UartHandle->Instance == USART2) {
		Uart2Ready = SET;
	}
	if (UartHandle->Instance == USART3) {
		Uart3Ready = SET;
	}
	if (UartHandle->Instance == UART4) {
		Uart4Ready = SET;
	}

}





/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if (hspi->Instance == SPI1) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
		//__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_SPI1_CLK_ENABLE();

		/* PA4 --> CS */
		GPIO_InitStruct.Pin	  = GPIO_PIN_4;
		GPIO_InitStruct.Mode	  = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* SPI SCK GPIO pin configuration  */
		GPIO_InitStruct.Pin	  = GPIO_PIN_5;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLUP;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* SPI MISO GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* SPI MOSI GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_7;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	if (hspi->Instance == SPI2) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_SPI2_CLK_ENABLE();

		/* SPI SCK GPIO pin configuration  */
		GPIO_InitStruct.Pin	  = GPIO_PIN_13;
		GPIO_InitStruct.Mode	  = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull	  = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* SPI MISO GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_14;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* SPI MOSI GPIO pin configuration  */
		GPIO_InitStruct.Pin = GPIO_PIN_15;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}



/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1) {
		/* Configure SPI CS as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

		/* Configure SPI SCK as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

		/* Configure SPI MISO as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);

		/* Configure SPI MOSI as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);
	}


	if(hspi->Instance == SPI2) {
		/* Configure SPI SCK as alternate function  */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);

		/* Configure SPI MISO as alternate function  */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);

		/* Configure SPI MOSI as alternate function  */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
	}
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_4) {
		;
	}
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
	RCC_OscInitTypeDef	  RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();

	__HAL_RCC_BKP_CLK_ENABLE();

	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		return;
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		return;
	}

	__HAL_RCC_RTC_ENABLE();
}




