/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: uart.c
      生成日期: 2016.09.25
      作    者：like
      功能说明：串口通信
      其他说明： 
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	


#include "stm32f1xx_hal.h"


UART_HandleTypeDef UartHandle;



#define UART1_TXBUF_SIZE 512
#define UART1_RXBUF_SIZE 512




char uart1_txbuf[UART1_TXBUF_SIZE];
int uart1_txlen;
char uart1_rxbuf[UART1_RXBUF_SIZE];
int uart1_rxlen;


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
void uart1_init(uint32_t bound)
{  
    UartHandle.Instance        = USART1;
    UartHandle.Init.BaudRate   = bound;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    
    HAL_UART_DeInit(&UartHandle);
    HAL_UART_Init(&UartHandle);
}




/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
int uart1_send(char *buf, int len)
{     
    if (HAL_UART_Transmit_IT(&UartHandle, (uint8_t*)buf, len)
        != HAL_OK)
    {
        return -1;
    }
    
    return 0;
}





void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    ;//UartReady = SET; 
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
void uart2_init(uint32_t bound)
{  
    
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
void uart3_init(uint32_t bound)
{  
    
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
void uart4_init(uint32_t bound)
{  
    
}




/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
void uart5_init(uint32_t bound)
{  
  
}


