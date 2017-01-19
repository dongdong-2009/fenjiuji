
/*
 * bsp adc temperature sample
 * likejshy@126.com
 * 2017-01-06
 */

#include <FreeRTOS.h>
#include <task.h>
#include "stm32f1xx_hal.h"

ADC_HandleTypeDef  AdcHandle;
DMA_HandleTypeDef  DmaHandle;

#define ADCCONVERTEDVALUES_BUFFER_SIZE ((uint32_t)    3)

__IO uint16_t	aADCxConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE];
uint8_t		ubSequenceCompleted = RESET;


int ADC_Config(void)
{
	ADC_ChannelConfTypeDef	 sConfig;

	/* ADC的输入时钟不得超过14MHz，它是由PCLK2经分频产生 */
	AdcHandle.Instance = ADC3; /* 12 bit AD */
	AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT; /* 数据右对齐 0x0000 ~ 0x0FFF */
	AdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
	AdcHandle.Init.ContinuousConvMode    = DISABLE;
	AdcHandle.Init.NbrOfConversion	     = 3;
	AdcHandle.Init.DiscontinuousConvMode = ENABLE;
	AdcHandle.Init.NbrOfDiscConversion   = 1;
	AdcHandle.Init.ExternalTrigConv	     = ADC_SOFTWARE_START;
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
		return -1;

	sConfig.Channel	     = ADC_CHANNEL_4;
	sConfig.Rank	     = ADC_REGULAR_RANK_4;
	sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
		return -1;

	if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
		return -1;

	if (HAL_ADC_Start_DMA(&AdcHandle,
			      (uint32_t *)aADCxConvertedValues,
			      ADCCONVERTEDVALUES_BUFFER_SIZE
			     ) != HAL_OK) {
		return -1;
	}

	HAL_ADC_Start(&AdcHandle);
	HAL_ADC_PollForConversion(&AdcHandle, 1);

	while (ubSequenceCompleted != SET) {
		;
	}

	ubSequenceCompleted = RESET;
	return 0;
}


void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	GPIO_InitTypeDef	  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInit;

	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_ADC3_CLK_ENABLE();

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	__HAL_RCC_DMA1_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/* 只有ADC1和ADC3拥有DMA功能 */
	DmaHandle.Instance		   = DMA1_Channel1;
	DmaHandle.Init.Direction	   = DMA_PERIPH_TO_MEMORY;
	DmaHandle.Init.PeriphInc	   = DMA_PINC_DISABLE;
	DmaHandle.Init.MemInc		   = DMA_MINC_ENABLE;
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	DmaHandle.Init.MemDataAlignment	   = DMA_MDATAALIGN_HALFWORD;
	DmaHandle.Init.Mode		   = DMA_CIRCULAR;
	DmaHandle.Init.Priority		   = DMA_PRIORITY_HIGH;

	HAL_DMA_DeInit(&DmaHandle);
	HAL_DMA_Init(&DmaHandle);

	__HAL_LINKDMA(hadc, DMA_Handle, DmaHandle);

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	HAL_NVIC_SetPriority(ADC3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC3_IRQn);
}



void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
	__HAL_RCC_ADC3_FORCE_RESET();
	__HAL_RCC_ADC3_RELEASE_RESET();

	HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6);
	if(hadc->DMA_Handle != NULL)
		return;

	HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
	HAL_NVIC_DisableIRQ(ADC3_IRQn);
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
	ubSequenceCompleted = SET;
}


