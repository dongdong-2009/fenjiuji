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
#include "bsp_uart.h"

#include "task_ethernet.h"
#include "task_wifi.h"
#include "task_rtu.h"
#include "task_lcd.h"
#include "task_shell.h"
#include "upgrade.h"

#include "print.h"


#define CONFIG_MAIN_PRINT


#ifdef CONFIG_MAIN_PRINT
    #define print(fmt,args...)  debug(fmt, ##args)
#else
    #define print(fmt,args...)
#endif


/******************************************************************************
    功能说明：打印程序信息
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void show_msg(void)
{
    print("\r\n\r\n");
    print("\r\n\r\n");
    print("\r\n**********************************************************************");
    print("\r\n*                                                                    *");
    print("\r\n*                          YiFuSi                                    *");
    print("\r\n*                                                                    *");
    print("\r\n*               FenJiuJi Main Ctrl Unit App (Version 1.0.0)          *");
    print("\r\n*                                                                    *");
    print("\r\n*                                         2016-11-20                 *");   
    print("\r\n*                                                                    *");
    print("\r\n**********************************************************************");
    print("\r\n");
    print("Programming Application runing!\r\n");
}



/******************************************************************************
    功能说明：任务之间通信方式创建
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int task_event_create(void)
{
    wifi_queue_creat();
    return 0;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_upgrade(void *pvParameters)
{
     
    while(1)
    {
        //ymodem_receive_file(file_name, YMODEM_PORT_COM);
	print("task_upgrade run!\r\n");
        vTaskDelay(10000); 
    }        
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
     
    /* 任务之间通信方式创建 */
    task_event_create();
        
    /* 创建以太网模块管理任务 */
    xTaskCreate(task_ethernet, "eth", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    
    /* 创建wifi模块管理任务 */
    xTaskCreate(task_wifi, "wifi", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
 
    
    /* 创建rtu任务 */
    xTaskCreate(task_rtu, "rtu", configMINIMAL_STACK_SIZE, NULL, 4, NULL);    

    
    /* 创建lcd任务 */
    xTaskCreate(task_lcd, "lcd", configMINIMAL_STACK_SIZE, NULL, 4, NULL);        

     
    /* 创建升级任务 */
    xTaskCreate(task_upgrade, "upgrade", configMINIMAL_STACK_SIZE, NULL, 4, NULL);        
   
    
    /* 创建shell任务 */
    xTaskCreate(task_shell, "shell", configMINIMAL_STACK_SIZE, NULL, 6, NULL);  
    
    /* 任务调度 */
    vTaskStartScheduler();
}

