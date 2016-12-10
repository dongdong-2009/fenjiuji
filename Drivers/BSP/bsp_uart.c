/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: bsp_uart.c
      ��������: 2016.11.01
      ��    �ߣ�like
      ����˵��������ģ��
      ����˵����1. ����1���ڵ��ԣ�isp��¼����
                2. ����2����485ͨ��
                3. ����3����esp8266ģ��
                4. ����4������̬��
      �޸ļ�¼��
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> 

#include "stm32f1xx_hal.h"
#include "bsp_uart.h"

/* ����1������Ϣ���ͻ��� */
#define RT_CONSOLEBUF_SIZE  128

/* ���Կ�ʹ�ܱ�־ */
char debug_en = 1;

/* shell���ܴ�ӡ��ʹ�ܱ�־ */
char print_en = 0;


/* ���ڲ������ */
UART_HandleTypeDef Uart1Handle;
UART_HandleTypeDef Uart2Handle;
UART_HandleTypeDef Uart3Handle;
UART_HandleTypeDef Uart4Handle;

/* �����жϷ�����ɱ�־ */
__IO ITStatus Uart1Ready = RESET;
__IO ITStatus Uart2Ready = RESET;
__IO ITStatus Uart3Ready = RESET;
__IO ITStatus Uart4Ready = RESET;

/* �����жϽ��ջ����� */
char uart1_rx_buff[UART1_RX_BUFF_SIZE];
char uart2_rx_buff[UART2_RX_BUFF_SIZE];
char uart3_rx_buff[UART3_RX_BUFF_SIZE];
char uart4_rx_buff[UART4_RX_BUFF_SIZE];


/******************************************************************************
    ����˵�������ڳ�ʼ��
    ���������uart_no ���ں� bound ������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void uart_init(char uart_no, uint32_t bound)
{  
    /* ����1��ʼ�� */
    if (uart_no == UART_1)
    {    
        Uart1Handle.Instance        = USART1;
        Uart1Handle.Init.BaudRate   = bound;
        Uart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart1Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart1Handle.Init.Parity     = UART_PARITY_NONE;
        Uart1Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart1Handle.Init.Mode       = UART_MODE_TX_RX;
        
        HAL_UART_DeInit(&Uart1Handle);
        HAL_UART_Init(&Uart1Handle);
        
        Uart1Ready = RESET; 
    }
    
    /* ����2��ʼ�� */
    if (uart_no == UART_2)
    {    
        Uart2Handle.Instance        = USART2;
        Uart2Handle.Init.BaudRate   = bound;
        Uart2Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart2Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart2Handle.Init.Parity     = UART_PARITY_NONE;
        Uart2Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart2Handle.Init.Mode       = UART_MODE_TX_RX;
        
        HAL_UART_DeInit(&Uart2Handle);
        HAL_UART_Init(&Uart2Handle);
        
        Uart2Ready = RESET; 
    } 
    
    /* ����3��ʼ�� */
    if (uart_no == UART_3)
    {    
        Uart3Handle.Instance        = USART3;
        Uart3Handle.Init.BaudRate   = bound;
        Uart3Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart3Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart3Handle.Init.Parity     = UART_PARITY_NONE;
        Uart3Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart3Handle.Init.Mode       = UART_MODE_TX_RX;
        
        HAL_UART_DeInit(&Uart3Handle);
        HAL_UART_Init(&Uart3Handle);
        
        Uart3Ready = RESET; 
    }     

    /* ����4��ʼ�� */
    if (uart_no == UART_4)
    {    
        Uart4Handle.Instance        = UART4;
        Uart4Handle.Init.BaudRate   = bound;
        Uart4Handle.Init.WordLength = UART_WORDLENGTH_8B;
        Uart4Handle.Init.StopBits   = UART_STOPBITS_1;
        Uart4Handle.Init.Parity     = UART_PARITY_NONE;
        Uart4Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
        Uart4Handle.Init.Mode       = UART_MODE_TX_RX;
        
        HAL_UART_DeInit(&Uart4Handle);
        HAL_UART_Init(&Uart4Handle);
        
        Uart4Ready = RESET; 
    }     
}


/******************************************************************************
    ����˵�����������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int uart_clear(char uart_no)
{
    if (uart_no == UART_1)
    {
        memset(uart1_rx_buff, 0, UART1_RX_BUFF_SIZE);
        
        if (HAL_UART_Receive_IT(&Uart1Handle, 
                                (uint8_t *)uart1_rx_buff, 
                                UART1_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }
    }
    
    if (uart_no == UART_2)
    {
        memset(uart2_rx_buff, 0, UART2_RX_BUFF_SIZE);
        
        if (HAL_UART_Receive_IT(&Uart2Handle, 
                                (uint8_t *)uart2_rx_buff, 
                                UART2_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }
    }    

    if (uart_no == UART_3)
    {
        memset(uart3_rx_buff, 0, UART3_RX_BUFF_SIZE);
        
        if (HAL_UART_Receive_IT(&Uart3Handle, 
                                (uint8_t *)uart3_rx_buff, 
                                UART3_RX_BUFF_SIZE) != HAL_OK)
        {
            return -1;
        }
    }       

    if (uart_no == UART_4)
    {
        memset(uart4_rx_buff, 0, UART4_RX_BUFF_SIZE);
        
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
    ����˵�����������ݷ���
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int bsp_uart_send(char uart_no, char *buff, int len)
{
    /* ����1���� */
    if (uart_no == UART_1)
    {    
        if (HAL_UART_Transmit_IT(&Uart1Handle, 
                                (uint8_t *)buff, 
                                len) != HAL_OK)
        {
            return -1;
        }
        
        while (Uart1Ready != SET)
        {
            ;
        }
        
        Uart1Ready = RESET;       
    }
    
    /* ����2���� */
    if (uart_no == UART_2)
    {    
        if (HAL_UART_Transmit_IT(&Uart2Handle, 
                                (uint8_t *)buff, 
                                len) != HAL_OK)
        {
            return -1;
        }
        
        while (Uart2Ready != SET)
        {
            ;
        }
        
        Uart2Ready = RESET;         
        
        
    }
    
    /* ����3���� */
    if (uart_no == UART_3)
    {    
        if (HAL_UART_Transmit_IT(&Uart3Handle, 
                                (uint8_t *)buff, 
                                len) != HAL_OK)
        {
            return -1;
        }
        
        while (Uart3Ready != SET)
        {
            ;
        }
        
        Uart3Ready = RESET;                 
    }
    
    /* ����4���� */
    if (uart_no == UART_4)
    {    
        if (HAL_UART_Transmit_IT(&Uart4Handle, 
                                (uint8_t *)buff, 
                                len) != HAL_OK)
        {
            return -1;
        }
        
        while (Uart4Ready != SET)
        {
            ;
        }
        
        Uart4Ready = RESET;                 
    }
    
    return 0;    
}



/******************************************************************************
    ����˵�����������ݽ���
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int bsp_uart_receive(char uart_no, char *buff, int size)
{
    int len = 0;
    int rxlen = 0;
    UART_HandleTypeDef *pUartHandle;
    int uart_rx_buff_size = 0;
    char *uart_rx_buff = 0;
    
    /* ����1���� */
    if (uart_no ==UART_1)
    {
        uart_rx_buff_size = UART1_RX_BUFF_SIZE;
        pUartHandle = &Uart1Handle;
        uart_rx_buff = uart1_rx_buff;
    }
    
    /* ����2���� */
    else if (uart_no ==UART_2)
    {
        uart_rx_buff_size = UART2_RX_BUFF_SIZE;
        pUartHandle = &Uart2Handle;
        uart_rx_buff = uart2_rx_buff;
    }    
    
    /* ����3���� */
    else if (uart_no ==UART_3)
    {
        uart_rx_buff_size = UART3_RX_BUFF_SIZE;
        pUartHandle = &Uart3Handle;
        uart_rx_buff = uart3_rx_buff;
    }        
    
    /* ����4���� */
    else if (uart_no ==UART_4)
    {
        uart_rx_buff_size = UART4_RX_BUFF_SIZE;
        pUartHandle = &Uart4Handle;
        uart_rx_buff = uart4_rx_buff;
    } 
   
    if (buff == NULL)
    {
        return -1;
    }
    
    /* ��ǰ�յ������ݳ��� */
    rxlen = uart_rx_buff_size - pUartHandle->RxXferCount; 
    
    if (rxlen > 0)
    {      
        /* ȡ�߲������� */
        if (rxlen > size)
        {
            /* ������ȡ�� */
            len = size;       
            memcpy(buff, uart_rx_buff, len);                    
            
            /* ��ʣ������ݰᵽ��ǰ�� */
            rxlen -= len;
            memcpy(uart_rx_buff, uart_rx_buff + len, rxlen);
             
            uart_init(uart_no, 115200);
            
            /* �������ջ��� */
            if (HAL_UART_Receive_IT(pUartHandle, 
                            (uint8_t *)uart_rx_buff + rxlen, 
                            uart_rx_buff_size - rxlen) != HAL_OK)
            {
                return -1;
            }
        }
        
        /* ȡȫ������ */
        else
        {
            len = rxlen;       
            memcpy(buff, uart_rx_buff, rxlen);
            rxlen = 0;
            
            uart_init(uart_no, 115200);
            
            /* �������ջ��� */
            if (HAL_UART_Receive_IT(pUartHandle, 
                            (uint8_t *)uart_rx_buff, 
                            uart_rx_buff_size) != HAL_OK)
            {
                return -1;
            }
        }
    }
  
    return len;
}



/******************************************************************************
    ����˵�������Կڷ����ַ�
    ���������buff ���ͻ���
    �����������
    �� �� ֵ����
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
    ����˵����shell��ӡ
    ���������fmt ��ӡ����
    �����������
    �� �� ֵ����
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
    ����˵�������Դ�ӡ�ڳ�ʼ��
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
void debug_init(void)
{
    uart_init(UART_1, 115200);  
    uart_clear(UART_1);
}


/******************************************************************************
    ����˵�������Դ�ӡ
    ���������fmt ��ӡ����
    �����������
    �� �� ֵ����
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

