
/*
 * daemon.c
 * likejshy@126.com
 * 2016-12-16
 */


#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

#include "bsp.h"

#include "print.h"


#define CONFIG_DAEMN_PRINT

#ifdef CONFIG_DAEMN_PRINT
#define print(fmt,args...) debug(fmt, ##args)
#else
#define print(fmt,args...)
#endif



void task_daemon(void *pvParameters)
{
	while (1) {

		len_run(0);
		vTaskDelay(500);
		len_run(1);
		vTaskDelay(500);
	}
}