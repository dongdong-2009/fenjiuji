
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


/*
 * ADC3 not have Vrefint
 */
int ADC_Config(void)
{
	ADC_ChannelConfTypeDef	 sConfig;

	AdcHandle.Instance = ADC3; 
	AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.ScanConvMode = ADC_SCAN_DISABLE;
	AdcHandle.Init.ContinuousConvMode    = DISABLE;
	AdcHandle.Init.NbrOfConversion	     =1;
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	AdcHandle.Init.NbrOfDiscConversion   = 1;
	AdcHandle.Init.ExternalTrigConv	     = ADC_SOFTWARE_START;
	
	if (HAL_ADC_DeInit(&AdcHandle) != HAL_OK)
		return -1;	
	
	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
		return -1;

	sConfig.Channel	     = ADC_CHANNEL_4;
	sConfig.Rank	     = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
		return -1;

	if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
		return -1;

	if (HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)aADCxConvertedValues, 1) 
		!= HAL_OK) {
		return -1;
	}

	HAL_ADC_Start(&AdcHandle);
	HAL_ADC_PollForConversion(&AdcHandle, 100);

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

	__HAL_RCC_DMA2_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	DmaHandle.Instance		   = DMA2_Channel5;
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

	HAL_NVIC_SetPriority(DMA2_Channel4_5_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);

	HAL_NVIC_SetPriority(ADC3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC3_IRQn);
}



void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
	__HAL_RCC_ADC3_FORCE_RESET();
	__HAL_RCC_ADC3_RELEASE_RESET();

	HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6);
	if (hadc->DMA_Handle != NULL)
		return;

	HAL_NVIC_DisableIRQ(DMA2_Channel4_5_IRQn);
	HAL_NVIC_DisableIRQ(ADC3_IRQn);
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
	ubSequenceCompleted = SET;
}



int bsp_temperature_read(int *temperature)
{
	if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)
		return -1;

	if (HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)aADCxConvertedValues, 1) 
		!= HAL_OK) {
		return -1;
	}

	HAL_ADC_Start(&AdcHandle);
	HAL_ADC_PollForConversion(&AdcHandle, 100);

	while (ubSequenceCompleted != SET) {
		;
	}

	ubSequenceCompleted = RESET;
	*temperature = (int)aADCxConvertedValues[0];
	return 0;		
}



