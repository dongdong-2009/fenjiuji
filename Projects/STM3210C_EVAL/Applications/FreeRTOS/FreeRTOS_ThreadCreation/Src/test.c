/******************************************************************************
    版权所有：依福斯电子
    版 本 号: 1.0
    文 件 名: main.c
    生成日期: 2016.09.25
    作    者：like
    功能说明：主函数入口
    其他说明：
    修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>

#include "bsp.h"
#include "task_ethernet.h"
#include "task_wifi.h"
#include "task_modbus.h"



/******************************************************************************
    功能说明：主函数
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int main(void)
{   
    /* 低层硬件初始化 */
    bsp_init();
  
    /* 创建以太网模块管理任务 */
	xTaskCreate(task_ethernet, "eth", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    
    /* 创建wifi模块管理任务 */
    xTaskCreate(task_wifi, "wifi", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    
    /* 创建modbus任务 */
    xTaskCreate(task_modbus, "modbus", configMINIMAL_STACK_SIZE, NULL, 2, NULL);    
    
    /* Start scheduler */
    vTaskStartScheduler();

    /* We should never get here as control is now taken by the scheduler */
    for (;;);
}

