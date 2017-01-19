/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: bsp.c
      生成日期: 2016.09.25
      作    者：like
      功能说明：板级支持包
      其他说明：
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>


#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "w5500.h"
#include "w25q64.h"
#include "bsp_uart.h"
#include "cmsis_os.h"
#include "bsp_adc.h"

static WWDG_HandleTypeDef  WwdgHandle;
static char watch_dog_flag;

int bsp_watchdog_config(void)
{
        WwdgHandle.Instance = WWDG;

        WwdgHandle.Init.Prescaler = WWDG_PRESCALER_8;
        WwdgHandle.Init.Window    = 80;
        WwdgHandle.Init.Counter   = 127;

        if (HAL_WWDG_Init(&WwdgHandle) != HAL_OK) {
                return -1;
        }

        if (HAL_WWDG_Start(&WwdgHandle) != HAL_OK) {
                return -1;
        }

        watch_dog_flag = 1;

        return 0;
}



int bsp_watchdog(void)
{
        if (watch_dog_flag == 1) {
                if (HAL_WWDG_Refresh(&WwdgHandle, 127) != HAL_OK)
                        return -1;
        }

        return 0;
}



void bsp_system_reboot(void)
{
        watch_dog_flag = 0;
        while(1);
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void SystemClock_Config(void)
{
        //uint32_t SysClock, HCLK , PCLK1, PCLK2;

        RCC_ClkInitTypeDef clkinitstruct = {0};
        RCC_OscInitTypeDef oscinitstruct = {0};

        oscinitstruct.OscillatorType        = RCC_OSCILLATORTYPE_HSE;
        oscinitstruct.HSEState              = RCC_HSE_ON;
        oscinitstruct.HSEPredivValue        = RCC_HSE_PREDIV_DIV1;
        oscinitstruct.PLL.PLLState          = RCC_PLL_ON;
        oscinitstruct.PLL.PLLSource         = RCC_PLLSOURCE_HSE;
        oscinitstruct.PLL.PLLMUL            = RCC_PLL_MUL9;

        if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK) {
                /* Initialization Error */
                while(1);
        }

        clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
        clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
        clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
        clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
        if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK) {
                /* Initialization Error */
                while(1);
        }

        //SysClock = HAL_RCC_GetSysClockFreq();72
        //HCLK = HAL_RCC_GetHCLKFreq();72
        //PCLK1 = HAL_RCC_GetPCLK1Freq();36
        //PCLK2 = HAL_RCC_GetPCLK2Freq();72
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void bsp_init(void)
{
        HAL_Init();

        SystemClock_Config();

        //bsp_watchdog_config();

        debug_init();

        SPI_Flash_Init();

        ADC_Config();
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


