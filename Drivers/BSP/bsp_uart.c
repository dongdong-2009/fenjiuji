/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: bsp_uart.c
      生成日期: 2016.11.01
      作    者：like
      功能说明：串口模块
      其他说明：1. 串口1用于调试，isp烧录程序
                2. 串口2用于485通信
                3. 串口3用于esp8266模块
                4. 串口4用于组态屏
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> 

#include "stm32f1xx_hal.h"
#include "bsp_uart.h"

/* 串口1调试信息发送缓存 */
#define RT_CONSOLEBUF_SIZE  128

/* 调试口使能标志 */
char debug_en = 1;

/* shell功能打印口使能标志 */
char print_en = 0;


/* 串口操作句柄 */
UART_HandleTypeDef Uart1Handle;
UART_HandleTypeDef Uart2Handle;
UART_HandleTypeDef Uart3Handle;
UART_HandleTypeDef Uart4Handle;

/* 串口中断发送完成标志 */
__IO ITStatus Uart1Ready = RESET;
__IO ITStatus Uart2Ready = RESET;
__IO ITStatus Uart3Ready = RESET;
__IO ITStatus Uart4Ready = RESET;

/* 串口中断接收缓存区 */
char uart1_rx_buff[UART1_RX_BUFF_SIZE];
char uart2_rx_buff[UART2_RX_BUFF_SIZE];
char uart3_rx_buff[UART3_RX_BUFF_SIZE];
char uart4_rx_buff[UART4_RX_BUFF_SIZE];



/******************************************************************************
    功能说明：R485使能
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void r485_en_gpio_init(void)
{   
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* PA1 --> R485_EN */
    __HAL_RCC_GPIOA_CLK_ENABLE();  
    
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 低电平是接受状态 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); 
}



/******************************************************************************
    功能说明：串口初始化
    输入参数：uart_no 串口号 bound 波特率
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
int uart_init(char uart_no, uint32_t bound)
{  
    /* 串口1初始化 */
    if (uart_no == UART_1)
    {    
        /* 串口配置 */
        Uart1Handle.Instance        = USART1;
        Uart1Handle.Init.BaudRate   = bound;
        Uart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart1Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart1Handle.Init.Parity     = UART_PARITY_NONE;
        Uart1Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart1Handle.Init.Mode       = UART_MODE_TX_RX;        
        HAL_UART_DeInit(&Uart1Handle);
        HAL_UART_Init(&Uart1Handle); 
        
        /*  发送完成标志 */
        Uart1Ready = RESET; 
        
        /* 清空串口接收缓存 */
        memset(uart1_rx_buff, 0, UART1_RX_BUFF_SIZE); 
        
        /* 开启串口接收 */
        if (HAL_UART_Receive_IT(&Uart1Handle, 
                                (uint8_t *)uart1_rx_buff, 
                                UART1_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }         
    }
    
    /* 串口2初始化 */
    if (uart_no == UART_2)
    {    
        /* 串口配置 */
        Uart2Handle.Instance        = USART2;
        Uart2Handle.Init.BaudRate   = bound;
        Uart2Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart2Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart2Handle.Init.Parity     = UART_PARITY_NONE;
        Uart2Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart2Handle.Init.Mode       = UART_MODE_TX_RX;       
        HAL_UART_DeInit(&Uart2Handle);
        HAL_UART_Init(&Uart2Handle);
        
        /* 串口2用作485通信，初始化485使能引脚 */
        r485_en_gpio_init();
        
        /*  发送完成标志 */
        Uart2Ready = RESET; 
        
        /* 清空串口接收缓存 */
        memset(uart2_rx_buff, 0, UART2_RX_BUFF_SIZE); 
        
        /* 开启串口接收 */
         if (HAL_UART_Receive_IT(&Uart2Handle, 
                                (uint8_t *)uart2_rx_buff, 
                                 UART2_RX_BUFF_SIZE) != HAL_OK)
         {
            return -1;
         }                
    } 
    
    /* 串口3初始化 */
    if (uart_no == UART_3)
    {    
        /* 串口配置 */
        Uart3Handle.Instance        = USART3;
        Uart3Handle.Init.BaudRate   = bound;
        Uart3Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart3Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart3Handle.Init.Parity     = UART_PARITY_NONE;
        Uart3Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart3Handle.Init.Mode       = UART_MODE_TX_RX;        
        HAL_UART_DeInit(&Uart3Handle);
        HAL_UART_Init(&Uart3Handle);
        
        /*  发送完成标志 */
        Uart3Ready = RESET; 
        
        /* 清空串口接收缓存 */
        memset(uart3_rx_buff, 0, UART3_RX_BUFF_SIZE); 
        
        /* 开启串口接收 */
        if (HAL_UART_Receive_IT(&Uart3Handle, 
                                (uint8_t *)uart3_rx_buff, 
                                UART3_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }               
    }     

    /* 串口4初始化 */
    if (uart_no == UART_4)
    {    
        /* 串口配置 */
        Uart4Handle.Instance        = UART4;
        Uart4Handle.Init.BaudRate   = bound;
        Uart4Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart4Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart4Handle.Init.Parity     = UART_PARITY_NONE;
        Uart4Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart4Handle.Init.Mode       = UART_MODE_TX_RX;       
        HAL_UART_DeInit(&Uart4Handle);
        HAL_UART_Init(&Uart4Handle);
        
        /*  发送完成标志 */
        Uart4Ready = RESET;
        
        /* 清空串口接收缓存 */
        memset(uart4_rx_buff, 0, UART4_RX_BUFF_SIZE); 
        
        /* 开启串口接收 */
        if (HAL_UART_Receive_IT(&Uart4Handle, 
                                (uint8_t *)uart4_rx_buff, 
                                UART4_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }         
    } 

    return 0;
}



/******************************************************************************
    功能说明：串口数据发送
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/ 
int bsp_uart_send(char uart_no, char *buff, int len)
{
    /* 串口1发送 */
    if (uart_no == UART_1)
    {    
        /* 采用中断发送 */
        if (HAL_UART_Transmit_IT(&Uart1Handle, (uint8_t *)buff, len) != HAL_OK)
        {
            return -1;
        } 
        
        /* 等待发送完成 */
        while (Uart1Ready != SET)
        {
            ;
        }
        
        /* 发送完成标志清除 */
        Uart1Ready = RESET;       
    }
    
    /* 串口2发送 */
    if (uart_no == UART_2)
    {    
        /* 高电平是485发送状态 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); 
        
        /* 采用中断发送 */
        if (HAL_UART_Transmit_IT(&Uart2Handle,  (uint8_t *)buff, len) != HAL_OK)
        {
            return -1;
        }
        
        /* 等待发送完成 */
        while (Uart2Ready != SET)
        {
            ;
        }
        
        /* 发送完成标志清除 */
        Uart2Ready = RESET;         
        
        /* 低电平是485接受状态 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); 
        
    }
    
    /* 串口3发送 */
    if (uart_no == UART_3)
    {    
        /* 采用中断发送 */
        if (HAL_UART_Transmit_IT(&Uart3Handle, (uint8_t *)buff, len) != HAL_OK)
        {
            return -1;
        }
        
        /* 等待发送完成 */
        while (Uart3Ready != SET)
        {
            ;
        }
        
        /* 发送完成标志清除 */
        Uart3Ready = RESET;                 
    }
    
    /* 串口4发送 */
    if (uart_no == UART_4)
    {    
        /* 采用中断发送 */
        if (HAL_UART_Transmit_IT(&Uart4Handle, (uint8_t *)buff,  len) != HAL_OK)
        {
            return -1;
        }
        
        /* 等待发送完成 */
        while (Uart4Ready != SET)
        {
            ;
        }
        
        /* 发送完成标志清除 */
        Uart4Ready = RESET;                 
    }
    
    return 0;    
}



/******************************************************************************
    功能说明：串口数据接收
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int bsp_uart_receive(char uart_no, char *buff, int size)
{
    int len = 0;
    int rxlen = 0;
    UART_HandleTypeDef *pUartHandle;
    int uart_rx_buff_size = 0;
    char *uart_rx_buff = 0;
    IRQn_Type IRQn;
    
    /* 串口1接收 */
    if (uart_no ==UART_1)
    {
        uart_rx_buff_size = UART1_RX_BUFF_SIZE;
        pUartHandle = &Uart1Handle;
        uart_rx_buff = uart1_rx_buff;
        IRQn = USART1_IRQn;
    }
    
    /* 串口2接收 */
    else if (uart_no ==UART_2)
    {
        uart_rx_buff_size = UART2_RX_BUFF_SIZE;
        pUartHandle = &Uart2Handle;
        uart_rx_buff = uart2_rx_buff;
        IRQn = USART2_IRQn;
    }    
    
    /* 串口3接收 */
    else if (uart_no ==UART_3)
    {
        uart_rx_buff_size = UART3_RX_BUFF_SIZE;
        pUartHandle = &Uart3Handle;
        uart_rx_buff = uart3_rx_buff;
        IRQn = USART3_IRQn;
    }        
    
    /* 串口4接收 */
    else /* if (uart_no ==UART_4) */
    {
        uart_rx_buff_size = UART4_RX_BUFF_SIZE;
        pUartHandle = &Uart4Handle;
        uart_rx_buff = uart4_rx_buff;
        IRQn = UART4_IRQn;
    } 
   
    if (buff == NULL)
    {
        return -1;
    }
    
    /* 当前收到的数据长度 */
    rxlen = uart_rx_buff_size - pUartHandle->RxXferCount; 
    
    if (rxlen > 0)
    {              
        if (((uart_rx_buff[rxlen - 1] != 0xFF) || (uart_rx_buff[rxlen - 2] != 0xFF)
            || (uart_rx_buff[rxlen - 3] != 0xFF)) && (uart_no ==UART_4))
        {
            return 0;
        }          
      
        /* 取走部分数据 */
        if (rxlen > size)
        {
            //HAL_NVIC_EnableIRQ(IRQn);
            
            /* 将数据取走 */
            len = size;       
            memcpy(buff, uart_rx_buff, len);                    
            
            /* 将剩余的数据搬到最前面 */
            rxlen -= len;
            memcpy(uart_rx_buff, uart_rx_buff + len, rxlen);
            pUartHandle->RxXferCount = uart_rx_buff_size - rxlen;
            if (uart_no ==UART_4)
               uart_init(UART_4, 9600);
           //HAL_NVIC_EnableIRQ(IRQn);

        }
        
        /* 取全部数据 */
        else
        {
            //HAL_NVIC_EnableIRQ(IRQn);
            len = rxlen;       
            memcpy(buff, uart_rx_buff, rxlen);
            rxlen = 0;
            pUartHandle->RxXferCount = uart_rx_buff_size;
           if (uart_no ==UART_4)
               uart_init(UART_4, 9600);
           // HAL_NVIC_EnableIRQ(IRQn);
        }
    }
  
    return len;
}



/******************************************************************************
    功能说明：调试口发送字符
    输入参数：buff 发送缓存
    输出参数：无
    返 回 值：无
*******************************************************************************/
int bsp_uart_send_str(char *buff)
{
    int len;
    
    len = strlen(buff);
    if (len <= RT_CONSOLEBUF_SIZE)
    {
        bsp_uart_send(UART_1, buff, len);
    }
    
    return 0;
}


/******************************************************************************
    功能说明：shell打印
    输入参数：fmt 打印数据
    输出参数：无
    返 回 值：无
*******************************************************************************/
void kprintf(const char *fmt, ...)
{
    va_list args;
    unsigned long length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
    
    if (!print_en)
    {
        return;
    }
    
    va_start(args, fmt);
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;

    bsp_uart_send_str(rt_log_buf);

    va_end(args);
}


/******************************************************************************
    功能说明：调试打印口初始化
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void debug_init(void)
{
    uart_init(UART_1, 115200);  
}


/******************************************************************************
    功能说明：调试打印
    输入参数：fmt 打印数据
    输出参数：无
    返 回 值：无
*******************************************************************************/
void debug(const char *fmt, ...)
{
    va_list args;
    unsigned long length;
    char buff[RT_CONSOLEBUF_SIZE] = {0};
    
    if (!debug_en)
    {
        return;
    }
    
    va_start(args, fmt);
    length = vsnprintf(buff, sizeof(buff) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;

    bsp_uart_send_str(buff);

    va_end(args);
}

