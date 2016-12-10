/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: bsp.c
      ��������: 2016.09.25
      ��    �ߣ�like
      ����˵�����弶֧�ְ�
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>


#include "stm32f1xx_hal.h"
#include "bsp_rtc.h"
#include "esp8266.h"
#include "w5500.h"
#include "w25q64.h"
#include "bsp_uart.h"

/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void SystemClock_Config(void)
{
//    uint32_t SysClock, HCLK , PCLK1, PCLK2;
  
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};
  
    oscinitstruct.OscillatorType        = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState              = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue        = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState          = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource         = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL            = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }
    
    clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
    if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
    {
        /* Initialization Error */
        while(1); 
    }
    
//    SysClock = HAL_RCC_GetSysClockFreq();
//    HCLK = HAL_RCC_GetHCLKFreq();
//    PCLK1 = HAL_RCC_GetPCLK1Freq();
//    PCLK2 = HAL_RCC_GetPCLK2Freq();
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void bsp_init(void)
{
    HAL_Init();
    
    SystemClock_Config();
     
    debug_init();
    
    SPI_Flash_Init();
    
    bsp_rtc_init(); 
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  osSystickHandler();

  HAL_IncTick();
}


