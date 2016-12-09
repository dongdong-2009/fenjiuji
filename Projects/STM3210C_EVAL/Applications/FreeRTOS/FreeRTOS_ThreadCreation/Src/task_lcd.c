/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_lcd.c
      生成日期: 2016.09.26
      作    者：like
      功能说明：LCD任务模块
      其他说明： 
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "bsp_uart.h"
#include "string.h"

#define LCD_RX_BUFF_SIZE 64
#define LCD_TX_BUFF_SIZE 64
#define LCD_BOUND  9600L

struct lcd_str
{
    char rxbuf[LCD_RX_BUFF_SIZE];
    char txbuf[LCD_TX_BUFF_SIZE];
    int rxlen;
    int txlen;
    char show;
    
    char psw[6];
    
};


static struct lcd_str lcd;


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void lcd_init(void)
{
    /* 通信口初始化 */
    uart_init(UART_2, LCD_BOUND);
    
    /* 通信口数据清空 */
    uart_clear(UART_2);   
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_read(char *rxbuf, int size)
{
    int len;
    int time = 2;
    int rxlen = 0;
        
    while (time > 0)
    {   
        /* 从串口接收数据 */
        len = bsp_uart_receive(UART_2, rxbuf + rxlen, size - rxlen); 
        if (len > 0)
        {
            rxlen += len;
            time = 2;
        }
                    
        if (rxlen > 0)
        {
            time--;
        }
        
        vTaskDelay(150);
    }    
    
    lcd_init();
    
    return rxlen;
}


/******************************************************************************
    功能说明：发送AT命令
    输入参数：buff AT命令缓存
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/ 
int lcd_send(char *buff)
{
    int len;
    int ret;
    
    if (buff == NULL)
    {
        return -1;
    }
    
    /* AT命令最长不能大于128 */
    len = strlen(buff);
    if (len > 128)
    {
        return -1;
    }
    
    /* 每一次发送命令就对串口进行初始化，防止串口发出错 */
    uart_init(UART_2, LCD_BOUND);    
    uart_clear(UART_2);
    
    /* 将数据从串口发出去 */
    /* 发送数据到串口*/
    ret = bsp_uart_send(UART_2, buff, len);
    if (ret < 0)
    {
        return -1;
    } 
      
    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_show_reset(void)
{
    int len;
       
    len = lcd_read(lcd.rxbuf + lcd.rxlen, LCD_RX_BUFF_SIZE - lcd.rxlen);
    if (len > 0)
    {
        lcd.rxlen += len;
        strcpy(lcd.txbuf, "code1return.val=1");
        lcd.txlen = strlen(lcd.txbuf);
        lcd.txbuf[lcd.txlen++] = 0xFF;
        lcd.txbuf[lcd.txlen++] = 0xFF;
        lcd.txbuf[lcd.txlen++] = 0xFF;
        lcd_send(lcd.txbuf);
    }
    
    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_show(void)
{
    lcd_show_reset();
    return 0;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_lcd(void *pvParameters)
{     
    lcd_init();
  
    for( ;; )
	{                         
        lcd_show();       
        vTaskDelay(1);
	}           
}

