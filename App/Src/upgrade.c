/*
 * upgrade
 * likejshy@126.com
 * 2016-12-16
 */
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

#include "bsp_uart.h"
#include "ymodem.h"
#include "upgrade.h"
#include "task_wifi.h"


#define CONFIG_UPGRADE_DEBUG

#ifdef CONFIG_UPGRADE_DEBUG
    #define Print(fmt,args...) debug(fmt, ##args)
#else
    #define Print(fmt,args...)
#endif

struct upgrade
{
	char port;
	char upgrade_en;
	char file_name[16];
	unsigned long file_size;
};


static struct upgrade arg;


int upgrade_query_trigger(char port, char *flag)
{	
	int ret;
	
	if (arg.upgrade_en != 1)
		return 0;
						
	if (port == UPGRADE_PORT_WIFI) {
		char wifi_arg;
		
		//ret = wifi_ioctl(WIFI_LINK_STATUS_GET, (unsigned long)&arg);
		if (ret < 0) {
			Print("upgrade upgrade_query_trigger error[%d]\r\n", ret);
			return -1;
		}
		
		if (wifi_arg)
			return 1;
		
	}
	
	return 0;
}


int upgrade_print(char *str)
{	
	int ret;
	char len;
	
	len = strlen(str);
	if (len > 128)
		len = 128;
		
	if (arg.port == UPGRADE_PORT_COM) {
		ret = bsp_uart_send(UART_1, str, len);
		if (ret < 0) {
			Print("upgrade bsp_uart_send error[%d]\r\n", ret);
			return -1;
		}
	}

	if (arg.port == UPGRADE_PORT_WIFI) {
		ret = wifi_send_byte(str, 1);
		if (ret < 0) {
			Print("upgrade wifi_send_byte error[%d]\r\n", ret);
		  	return -1;
		}
	}

	return 0;
}


int upgrade_getchar(char *ch)
{
	return 0;
}



int upgrade_mem1_download_file(char *file_name, unsigned long *file_size)
{
	return 0;	
}


int upgrade_mem2_firmware(char *file_name, unsigned long file_size)
{
	return 0;	
}

int upgrade_mem3_reboot(void)
{
	return 0;	
}


int upgrade_man_machine(struct upgrade *arg)
{
	char ch;
	int ret;
		
	/* print mem for man to selet */
	upgrade_print("\r\n");
	upgrade_print("*******************************\r\n");
	upgrade_print("*   1. download upgrade file   \r\n");
	upgrade_print("*   2. upgrade system          \r\n");
	upgrade_print("*   3. reboot system           \r\n");
	upgrade_print("*******************************\r\n");
	
	ret = upgrade_getchar(&ch);
	if (ret < 0) {
		Print("upgrade upgrade_getchar error[%d]\r\n", ret);
	  	return -1;
	}
		
	
	switch (ch) {
	case 0x31:
		ret = upgrade_mem1_download_file(arg->file_name, &arg->file_size);
		if (ret < 0) {
			Print("upgrade upgrade_mem1_download_file error[%d]\r\n", ret);
	  		return -1;
		}
		break;
	case 0x32:
		ret = upgrade_mem2_firmware(arg->file_name, arg->file_size);
		if (ret < 0) {
			Print("upgrade upgrade_mem2_firmware error[%d]\r\n", ret);
	  		return -1;
		}		
		break;
	case 0x33:
		ret = upgrade_mem3_reboot();
		if (ret < 0) {
			Print("upgrade upgrade_mem3_firmware error[%d]\r\n", ret);
	  		return -1;
		}		
		break;		
	default:
		upgrade_print("selet error!!!\r\n");
		return 0;		
	}
	
	
	return 0;
}



int upgrade(char port)
{
	int ret;
	char trigger_flag = 0;
		
	/* query the trigger of upgrade flag */
	while (!trigger_flag) {
		ret = upgrade_query_trigger(arg.port, &trigger_flag);
		if (ret < 0) {
			Print("upgrade upgrade_query_trigger error[%d]\r\n", ret);
			return -1;
		}
		
		vTaskDelay(1000);
	}
	
	ret = upgrade_man_machine(&arg);
	if (ret < 0) {
		Print("upgrade upgrade_man_machine error[%d]\r\n", ret);
		return -1;
	}
	
	return 0;	
}


int upgrade_trigger(char port)
{
	arg.port = port;
	arg.upgrade_en = 1;
	return 0;
}


