/*
 * print for APP
 * likejshy@126.com
 * 2016-12-16
 */

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> 

#include "stm32f1xx_hal.h"
#include "bsp_uart.h"
#include "print.h"


#define APP_PRINT_BUFF_SIZE 128


struct app_print
{
	char en;
	char port;
};

static struct app_print arg;



void app_print_config(char en, char port)
{
	arg.en = en;
	arg.port = port;
}


static int app_print_send_str(char *str)
{
	int len;
	int ret;
	
	len = strlen(str);
	if (len > APP_PRINT_BUFF_SIZE)
		len = APP_PRINT_BUFF_SIZE;
	
	if (arg.port == APP_PRINT_PORT_COM) {
		ret = bsp_uart_send(UART_1, str, len);
		if (ret < 0) {
			return -1;
		}
	}
	
	return 0;
}


void __app_print(const char *fmt, ...)
{
    	va_list args;
    	unsigned long length;
    	char rt_log_buf[APP_PRINT_BUFF_SIZE];
    	
    	if (!arg.en)
    	    return;
    	
    	va_start(args, fmt);
    	length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    	if (length > APP_PRINT_BUFF_SIZE - 1)
    	    length = APP_PRINT_BUFF_SIZE - 1;
	
    	app_print_send_str(rt_log_buf);
	
    	va_end(args);
}




