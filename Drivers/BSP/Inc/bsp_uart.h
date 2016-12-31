/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: bsp_uart.h
      ��������: 2016.11.01
      ��    �ߣ�like
      ����˵��������ģ��
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#ifndef _BSP_UART_H
#define _BSP_UART_H

#define UART1_RX_BUFF_SIZE 512
#define UART2_RX_BUFF_SIZE 512
#define UART3_RX_BUFF_SIZE 1200
#define UART4_RX_BUFF_SIZE 512
#define UART5_RX_BUFF_SIZE 512

#define UART_1 0
#define UART_2 1
#define UART_3 2
#define UART_4 3
#define UART_5 4

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef Uart1Handle;
extern UART_HandleTypeDef Uart2Handle;
extern UART_HandleTypeDef Uart3Handle;
extern UART_HandleTypeDef Uart4Handle;
extern UART_HandleTypeDef Uart5Handle;


extern int uart_init(char uart_no, uint32_t bound);
extern int bsp_uart_send(char uart_no, char *txbuf, int len);
extern int bsp_uart_receive(char uart_no, char *buff, int size);
extern int uart_clear(char uart_no);
extern void kprintf(const char *fmt, ...);
extern void debug_init(void);
extern void debug(const char *fmt, ...);



#endif /*  _BSP_UART_H */