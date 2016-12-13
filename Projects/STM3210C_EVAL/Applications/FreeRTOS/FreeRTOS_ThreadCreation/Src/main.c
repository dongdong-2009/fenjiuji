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
#include "task_lcd.h"
#include "task_shell.h"
#include "bsp_uart.h"


/* 调试开关 */
#define CONFIG_MAIN_DEBUG

/* 调试接口函数 */
#ifdef CONFIG_MAIN_DEBUG
    #define main_print(fmt,args...) debug(fmt, ##args)
#else
    #define main_print(fmt,args...)
#endif


/******************************************************************************
    功能说明：打印程序信息
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void show_msg(void)
{
    main_print("\r\n\r\n");
    main_print("\r\n\r\n");
    main_print("\r\n**********************************************************************");
    main_print("\r\n*                                                                    *");
    main_print("\r\n*                          依福斯电子                                *");
    main_print("\r\n*                                                                    *");
    main_print("\r\n*               分酒机主控单元应用程序  (Version 1.0.0)              *");
    main_print("\r\n*                                                                    *");
    main_print("\r\n*                                         2016-11-20                 *");   
    main_print("\r\n*                                                                    *");
    main_print("\r\n**********************************************************************");
    main_print("\r\n");
    main_print("Programming Application runing!\r\n");
}



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
    
    /* 打印程序运行信息 */
    show_msg();
       
    /* 创建以太网模块管理任务 */
	xTaskCreate(task_ethernet, "eth", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    main_print("create ethernet task......OK\r\n");
    
    /* 创建wifi模块管理任务 */
    xTaskCreate(task_wifi, "wifi", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL);
    main_print("create wifi task......OK\r\n"); 
    
    /* 创建modbus任务 */
    xTaskCreate(task_modbus, "modbus", configMINIMAL_STACK_SIZE, NULL, 2, NULL);    
    main_print("create modbus task......OK\r\n");
    
    /* 创建lcd任务 */
    xTaskCreate(task_lcd, "lcd", configMINIMAL_STACK_SIZE, NULL, 2, NULL);        
    main_print("create LCD task......OK\r\n");
    
    /* 创建shell任务 */
    //xTaskCreate(task_shell, "shell", configMINIMAL_STACK_SIZE, NULL, 2, NULL);  
    //main_print("create shell task......OK\r\n");
    
    /* 任务调度 */
    vTaskStartScheduler();
}

