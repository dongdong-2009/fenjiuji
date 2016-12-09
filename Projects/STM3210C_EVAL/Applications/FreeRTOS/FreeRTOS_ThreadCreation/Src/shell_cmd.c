/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: shell_cmd.c
      生成日期: 2016.11.04
      作    者：like
      功能说明：shell 调试命令
      其他说明： 
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <bsp_uart.h>



long hello(void)
{
    kprintf("Hello word!\n");
    return 0;
}








