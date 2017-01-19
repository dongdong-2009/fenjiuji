
/*
 * temperature ctrl
 * likejshy@126.com
 * 2017-01-06
 */

/*
 * 温度控制流程
 * ============
 *
 * 1. 开机后实时读取温度传感器值，获取当前环境温度
 *
 * 2. 当温度高于设定值时，且持续时间大于定值，启动制冷过程
 *    1) 确认关闭加热器
 *    2) 开启压缩机，开启压缩机风扇1，开启对流风扇2
 *    3) 温度达到正常范围就关闭压缩机，关闭压缩机风扇1, 关闭对流风扇2
 *    4）记录压缩机关闭的时间值，用于确保上一次关闭时间与下一次开启的时间间隔不少于
 *	 2分钟
 *
 * 3. 温度低于设定值时，且持续时间大于定值，启动加热过程
 *    1) 确认关闭压缩机，确认关闭压缩机风扇1
 *    2) 开启加热器，开启对流风扇2
 *    3) 温度达到正常范围就关闭加热器，关闭流风扇2
 *
 * 4. 以上过程LCD对温度进行实时更新
 */

#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "bsp_uart.h"
#include "rtc.h"

#define CONFIG_TEMP_CTL_DEBUG

#define PORT_CMD_RTU   0
#define PORT_CMD_LCD   1
#define LCD_SOW_TEMP   2
#define OFF 0
#define ON 1

#ifdef CONFIG_TEMP_CTL_DEBUG
    #define print(fmt, args...) debug(fmt, ##args)
#else
    #define print(fmt, args...)
#endif


struct port_cmd_lcd
{
	char cmd;
	union
	{
		int temp;
	} sub;
};


struct port_cmd_rtu
{
	char cmd;
	union
	{
		int temp;
	} sub;
};



static int temp_ctl_port_lcd_show_temp(struct port_cmd_lcd *arg)
{
	extern int temperature1;
	
	temperature1 = arg->sub.temp;
	
	return 0;
}



static int temp_ctl_port_lcd(struct port_cmd_lcd *arg)
{
	int ret;
	
	switch (arg->cmd) {
	case LCD_SOW_TEMP:
		ret = temp_ctl_port_lcd_show_temp(arg);
		break;	
	default:
		break;
	}
        
        return ret;
}



static int temp_ctl_port_rtu(struct port_cmd_rtu *arg)
{
	int ret;
	
	switch (arg->cmd) {
//	case LCD_SOW_TEMP:
//		ret = temp_ctl_port_lcd(arg);
//		break;	
	default:
		break;
	}
        
        return ret;
}


static int temp_ctl_port(char cmd, unsigned long arg)
{
	int ret;
	
	switch (cmd) {
	case PORT_CMD_LCD:
		ret = temp_ctl_port_lcd((struct port_cmd_lcd*)arg);
		break;
	case PORT_CMD_RTU:
		ret = temp_ctl_port_rtu((struct port_cmd_rtu*)arg);
		break;		
	default:
		break;
	}
        
        return ret;
}



static int temperature_real_time_read(int *temperature)
{	
	struct tm calendar;
	
	rtc_read(&calendar);
	
	*temperature = calendar.tm_sec;
	return 0;
}


static int temperature_arg_read(int *temp_min_val, int *temp_min_time,
				int *temp_max_val, int *temp_max_time,
                                int *temp_normal_time_arg)
{
	return 0;
}


int  temperature_compressor(char mode)
{
	return 0;
}


int temperature_compressor_fan(char mode)
{
	return 0;
}

int temperature_heater(char mode)
{
	return 0;
}

int temperature_convect_fan(int mode)
{
	return 0;
}


int temperature_monitor(int monitor_time_sec, int temp_min_val_arg,
			int temp_min_time_arg, int temp_max_val_arg,
			int temp_max_time_arg, int temp_normal_time_arg)
{
	int temperature = 0;
	int temp_min_time = 0;
	int temp_max_time = 0;
        int temp_normal_time = 0;
	struct port_cmd_lcd arg;
        int ret;

	while (monitor_time_sec--) {
		ret = temperature_real_time_read(&temperature);
		if (ret < 0) {
			print("temperature_real_time_read, error[%d]\r\n", ret);
			return -1;
		}

		arg.cmd = LCD_SOW_TEMP;
		arg.sub.temp = temperature;
		ret = temp_ctl_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			print("temp_ctl_port error[%d]\r\n", ret);
			return -1;
		}		
			
		if (temperature < temp_min_val_arg) {
			if (temp_max_time > 0)
				temp_max_time = 0;

			temp_min_time++;
			if (temp_min_time > temp_min_time_arg) {
				print("temperature_monitor LOW!\r\n");
				return 2;
			}

		} else if (temperature > temp_max_val_arg) {
			if (temp_min_time > 0)
				temp_min_time = 0;

			temp_max_time++;
			if (temp_max_time > temp_max_time_arg) {
				print("temperature_monitor HIGHT!\r\n");
				return 3;
			}
		} else {
			if (temp_max_time > 0)
				temp_max_time = 0;
			if (temp_min_time > 0)
				temp_min_time = 0;

			temp_normal_time++;
			if (temp_normal_time > temp_normal_time_arg) {
				return 1;
			}

		}

		vTaskDelay(1000);
	}

	print("temperature_monitor over time!\r\n");
	return 0;
}


int temperature_ctl(void)
{
	int temp_min_val_arg;
	int temp_min_time_arg;
	int temp_max_val_arg;
	int temp_max_time_arg;
        int temp_normal_time_arg;
	int ret;

	ret = temperature_arg_read(&temp_min_val_arg, &temp_min_time_arg,
				   &temp_max_val_arg, &temp_max_time_arg,
                                   &temp_normal_time_arg);
	if (ret < 0) {
		print("temperature_arg_read, error[%d]\r\n", ret);
		return -1;
	}

	ret = temperature_monitor(60, temp_min_val_arg, temp_min_time_arg,
				 temp_max_val_arg, temp_max_time_arg,
				 temp_normal_time_arg);
	if (ret < 0) {
		print("temperature_monitor, error[%d]\r\n", ret);
		return -1;
	}

	/* low temperature ctl */
	if (ret == 2) {

		temperature_compressor(OFF);
		temperature_compressor_fan(OFF);
		temperature_heater(ON);
		temperature_convect_fan(ON);

		ret = temperature_monitor(60, temp_min_val_arg, 
					  temp_min_time_arg,
				 	   temp_max_val_arg, temp_max_time_arg,
				 	    temp_normal_time_arg);

		temperature_compressor(OFF);
		temperature_compressor_fan(OFF);
		temperature_heater(OFF);
		temperature_convect_fan(OFF);

		if (ret != 1) {
			print("temperature monitor, error[%d]\r\n", ret);
			return -1;
		}
	}

	/* high temperature ctl */
	else if (ret == 3) {

		temperature_compressor(ON);
		temperature_compressor_fan(ON);
		temperature_heater(OFF);
		temperature_convect_fan(ON);

		ret = temperature_monitor(60, temp_min_val_arg,
					  temp_min_time_arg,
				 	temp_max_val_arg, temp_max_time_arg,
				 	temp_normal_time_arg);


		if (ret != 1) {
			print("temperature monitor, error[%d]\r\n", ret);
			return -1;
		}
	}

	return 0;
}





 /******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_temp_ctrl(void *pvParameters)
{
     
    while(1)
    {
	temperature_ctl();
	print("task_temp_ctrl run!\r\n");
        vTaskDelay(10000); 
    }        
}



