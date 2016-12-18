/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_modbus.c
      生成日期: 2016.09.26
      作    者：like
      功能说明：modbus任务模块
      其他说明： 
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "store_file.h"
#include "bsp_uart.h"


char aShowTime[50] = {0};
char aShowDate[50] = {0};


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_modbus(void *pvParameters)
{
   // RTC_CalendarConfig(0x16, 0x01, 0x01, 0x00, 0x00, 0x00);

    uart_init(UART_2, 115200);
    for( ;; )
	{                   
        //RTC_CalendarShow(aShowTime, aShowDate);
        
        
        vTaskDelay(1000);
	}       
}

