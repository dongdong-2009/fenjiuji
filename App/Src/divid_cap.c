
/*
 * divid cup ctrl
 * likejshy@126.com
 * 2017-01-06
 */

/* 
 * 1. 接收"酒头控制器"出酒命令
 * 	1)获取酒位信息，即是哪个酒头需要出酒？
 * 	2)需要出多少酒，大杯？中杯？小杯？
 * 	3)一旦获取到出洒命令，将进入下面出酒流程，如果这个过程中再获取到出酒命令，
 * 	  不作处理，直到出酒流程结束
 * 
 * 2. 出酒条件判断
 * 	1)出酒权限判断，LCD在没有输入密码的锁屏界面，不允许出酒，LCD进入权限
 * 	  不足的告警界面, 即分杯机已锁定
 * 	2)以上条件满足，LCD进入洒量信息界面，提示哪个酒位需要出酒，并显示剩余洒
 * 	  量杯数
 * 	3)酒量判断，当前酒头位置的酒量少于需要出的酒量，不允许出酒，LCD进入酒量
 * 	  不足的告警界面
 * 
 * 3. 执行出酒	
 * 	1)向"压力调节器"发出打开总气阀命令，实时获取"压力调节器"的压力值，
 * 	  并将压力值下发给"酒头控制器"
 * 	2)压力值超出出酒阀值范围，LCD提示压力异常，出酒失败告警界面
 * 	3)压力值正常,先下发出酒杯量(大中小杯)
 *  4)下发出酒命令给"酒头控制器"，并实时获取酒头出酒的洒量，LCD实
 * 	  时更新剩余酒量。
 * 
 * 3.出酒结束
 *      1)实时获取"酒头控制器"出酒完成后(包括吹酒)的状态，如果完成，出酒终止
 * 	2)实时计算本次出酒量，如果超过出酒的需求量，出酒终止
 * 	3)出酒后开始计时，当达到时间上限后，出酒终止
 * 	4)以上情况只要出现一种，都进行出酒终止，并向"压力调节器"发出关闭总气阀
 * 	  命令
 * 	5)退出出酒流程
 * 	
 * 注:
 * 1)如出现告警界面，人为清除后，退出出酒流程,没有人为清除，30秒后自动退出出酒
 *   流程
 * 2)退出出酒流程后，LCD进入出酒待机状态
 */  

 
/*  
 * 装瓶清洗流程
 * ============
 * 1. 从LCD装瓶清洗界面的“清洗”按钮触发清洗功能
 * 
 * 2. 向"压力调节器"发出打开总气阀命令，实时获取"压力调节器"的压力值，
 *    并将压力值下发给"酒头控制器"
 *   
 * 3. 压力值小于出酒阀值，LCD提示压力不足，清洗失败告警界面
 * 
 * 4. 压力值正常,下发出酒命令给"酒头控制器"， 5秒结束清洗
 * 
 * 5. 向"压力调节器"发出关闭总气阀命令，清洗流程结束
 */

 
/* 
 * 装瓶抽真空流程
 * ==============
 * 1. 从LCD装瓶清洗界面的“装瓶”按钮触发装瓶抽真空功能
 * 
 * 2. 查询"压力调节器"是否关闭总气阀，如果没关闭就发命令关闭
 * 
 * 3. 获取瓶位号，并向对应的"酒头控制器"发出打开气体电磁阀命令，关闭分酒电磁阀命令
 * 
 * 3. 向其他瓶位号对应的"酒头控制器"发出关闭气体电磁阀命令，关闭分酒电磁阀命令
 * 
 * 4. 向"压力调节器"发出抽真空命令，本步骤可反复多次进行，直到该瓶位内的空气抽干净
 * 
 * 5. 完成抽真空后，开启气体总电磁阀，向瓶内注入氮气，获取“压力调节器”压力值
 *    当压力值不再升高，氮气注入完成
 *    
 * 7. 向"压力调节器"发命令关闭总气阀
 * 
 * 8. 向"酒头控制器"发命令关闭气体电磁阀
 */
 

#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "bsp_uart.h"
#include "store_param.h"
#include "task_lcd.h"

#define CONFIG_DIVID_CUP_DEBUG

#define BOTTLE_COUNT_MAX      4

#define PORT_CMD_RTU   		0
#define PORT_CMD_LCD   		1
#define RTU_GET_DIVID_ASK 	2
#define RTU_CLEAR_DIVID_ASK 3
#define LCD_GET_AUTHOR		4
#define LCD_SHOW_ALARM_AUTHOR_LIMIT 5
#define LCD_SHOW_BEBE 6
#define LCD_SHOW_ALARM_UNDER_CAPACITY 7
#define RTU_CTL_PRESS_SWITCH_ON	8
#define RTU_GET_PRESS_VAL	9 
#define LCD_SHOW_ALARM_PRESS_UNUSUAL 10
#define RTU_GET_DIVID_CAP_SWITCH_ON 11
#define RTU_GET_DIVID_CAP_PROCESS 12
#define RTU_GET_DIVID_CAP_SWITCH_OFF 13
#define LCD_CLEAR_BOTTLE_ASK 14
#define LCD_INSTALL_BOTTLE_ASK 15
#define RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_ON 16
#define RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_OFF 17
#define RTU_SET_DIVID_CAP_GAS_VALVE_ON 18
#define RTU_SET_DIVID_CAP_BEBE_VALVE_ON 19
#define RTU_SET_DIVID_CAP_BEBE_VALVE_OFF 20
#define RTU_SET_PRESS_CTL_PUMP_GAS 21
#define RTU_SET_DIVID_CAP_GAS_VALVE_OFF 22
#define	LCD_SHOW_GOTO_BEBE_SHOW 24
#define LCD_SHOW_REFRESH_BEBE 25


#ifdef CONFIG_DIVID_CUP_DEBUG
    #define print(fmt, args...) debug(fmt, ##args)
#else
    #define print(fmt, args...)
#endif


struct port_cmd_rtu 
{
	char cmd;	
	union 	
	{
		struct 
		{
			char enable;
			unsigned short bebe;
			unsigned short place;
			char cmd_time;
		} get_divid_cup;
		
		char press_val;
		
		struct 
		{
			char press_val;
			char bebe;
			char place;
		} divid_cap;
		
		struct 
		{
			char res;
			char bebe;
			char place;
		} divid_cap_pro;
		
		struct 
		{
			char place;
		} divid_cap_gas_valve;
		
		struct 
		{
			char place;
		} divid_cap_bebe_valve;
	} sub;	
};


struct port_cmd_lcd
{
	char cmd;		
	union 
	{
		char author_limit;
		
		struct 
		{
			char bebe;
			char place;		
		} show_bebe;
                
                struct 
                {
                        char enable;
                } clear_bottle;      
                
                struct
                {
                	char pump;
			char place;
                } install_bottle;
	} sub;
};


int divid_cup_get_press_val_arg(unsigned short *arg_press_max_val, unsigned short *arg_press_min_val)
{		
	int len;
	int ret;
	
	len = store_param_read("press_max", (char *)arg_press_max_val);
	if (len == 0) 
	{
		*arg_press_max_val = 1023;
		ret = store_param_save("press_max", (char *)arg_press_max_val, 2);
		if (ret < 0)
			return -1;
	}	

	len = store_param_read("press_min", (char *)arg_press_min_val);
	if (len == 0) 
	{
		*arg_press_min_val = 10;
		ret = store_param_save("press_min", (char *)arg_press_min_val, 2);
		if (ret < 0)
			return -1;
	}	
	
	return 0;
}


int divid_cup_get_bebe_capacity(char place)
{
	return 0;
}


int __port_rtu_get_divid_ask(struct port_cmd_rtu *arg)
{	
	arg->sub.get_divid_cup.place = 1;
	rtu_jiutou_dat_get(1, 0x13, &arg->sub.get_divid_cup.bebe);
	if (arg->sub.get_divid_cup.bebe > 0) { //酒头按钮按下
		arg->sub.get_divid_cup.cmd_time++;
		//按下时间大于一定时间就认为是有效的出酒命令
		if (arg->sub.get_divid_cup.cmd_time > 10) { 
			 arg->sub.get_divid_cup.cmd_time = 0;
			 arg->sub.get_divid_cup.enable = 1;
			 return 0;
		}
	}
	
	arg->sub.get_divid_cup.place = 2;
	rtu_jiutou_dat_get(2, 0x13, &arg->sub.get_divid_cup.bebe);
	if (arg->sub.get_divid_cup.bebe > 0) {
		arg->sub.get_divid_cup.cmd_time++;
		if (arg->sub.get_divid_cup.cmd_time > 10) {
			 arg->sub.get_divid_cup.cmd_time = 0;
			 arg->sub.get_divid_cup.enable = 1;
			 return 0;
		}
	}
	
	return 0;
}

int __port_rtu_clear_divid_ask(struct port_cmd_rtu *arg)
{	
    //清除按键状态
    //01/02 05 00 0B 00 00
    rtu_jiutou_ctl_set(2, 0x000B, 0x0000);
		
	return 0;
}

int __port_rtu_ctl_press_switch_on(struct port_cmd_rtu *arg)
{
	//05
	//01
    	//打开压力调节器气体总阀
    	//0A 05 00 01 FF 00
	rtu_tiaoya_ctl_set(0x0A, 0x0001,0xFF00);	
	
	return 0;
}

int __port_rtu_get_press_val(struct port_cmd_rtu *arg)
{
	//0A 03 00 00 00 0x
    rtu_tiaoya_dat_get(0x01, 0x00, &arg->sub.divid_cap.press_val);
	return 0;
}


int __port_rtu_get_divid_cap_switch_on(struct port_cmd_rtu *arg)
{
	
	//出酒指令
    //01/02 05 00 0A FF 00
    rtu_jiutou_ctl_set(2, 0x000A, 0xff00);
	return 0;
}


int __port_rtu_get_divid_cap_process(struct port_cmd_rtu *arg)
{
	 //0F：出酒倒计时高字节
         // 10：出酒倒计时低字节

	return 0;
}


int __port_rtu_get_divid_cap_switch_off(struct port_cmd_rtu *arg)
{
	// not use
	return 0;
}


int  divid_cup_port_rtu(struct port_cmd_rtu *arg)
{
	int ret = -1;
	
	switch (arg->cmd) {
	case RTU_GET_DIVID_ASK:
		ret = __port_rtu_get_divid_ask(arg);
		break;
		
    	case RTU_CLEAR_DIVID_ASK:
		ret = __port_rtu_clear_divid_ask(arg);
		break;
        
	case RTU_CTL_PRESS_SWITCH_ON:
		ret = __port_rtu_ctl_press_switch_on(arg);
		break;
		
	case RTU_GET_PRESS_VAL:
		ret = __port_rtu_get_press_val(arg);
		break;
		
	case RTU_GET_DIVID_CAP_SWITCH_ON:
		ret = __port_rtu_get_divid_cap_switch_on(arg);
		break;
		
	case RTU_GET_DIVID_CAP_PROCESS:
		ret = __port_rtu_get_divid_cap_process(arg);
		break;
		
	case RTU_GET_DIVID_CAP_SWITCH_OFF:
		ret = __port_rtu_get_divid_cap_switch_off(arg);
		break;
	
	case RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_ON:
		ret = __port_rtu_get_divid_cap_switch_off(arg);
		break;	
	
	case RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_OFF:
		ret = __port_rtu_get_divid_cap_switch_off(arg);
		break;	
		
	default:
		break;
	}
	
	return ret;
}


int __port_lcd_clear_bottle_ask(struct port_cmd_lcd *arg)
{
    int ret;
    
    if((g_wash_num < 0)||(g_wash_num > BOTTLE_COUNT_MAX))
    {
      return -1;
    }
    else
    {
      ret = g_wash_num;
      g_wash_num = 0;
    }
    
	return ret;
}


int __port_lcd_install_bottle_ask(struct port_cmd_lcd *arg)
{
	int ret;
    
    if((g_bottling_num < 0)||(g_bottling_num > BOTTLE_COUNT_MAX))
    {
      return -1;
    }
    else
    {
      ret = g_bottling_num;
      g_bottling_num = 0;
    }
    
	return ret;
}

int __port_lcd_show_alarm_author_limit(struct port_cmd_lcd *arg)
{
	return 0;
}


int __port_lcd_show_bebe(struct port_cmd_lcd *arg)
{
	return 0;
}


int __port_lcd_show_alarm_under_capacity(struct port_cmd_lcd *arg)
{
	return 0;
}



int __port_lcd_show_alarm_press_unusual(struct port_cmd_lcd *arg)
{
	return 0;
}


int  divid_cup_port_lcd(struct port_cmd_lcd *arg)
{
	int ret = 0;
	
	switch (arg->cmd) {
	case LCD_CLEAR_BOTTLE_ASK:
		ret = __port_lcd_clear_bottle_ask(arg);
		break;
		
	case LCD_INSTALL_BOTTLE_ASK:
		ret = __port_lcd_install_bottle_ask(arg);
		break;
		
	case LCD_GET_AUTHOR:
		ret = __port_lcd_get_author(arg);
		break;
		
	case LCD_SHOW_ALARM_AUTHOR_LIMIT:
		ret = __port_lcd_show_alarm_author_limit(arg);
		break;
		
	case LCD_SHOW_BEBE:
		ret = __port_lcd_show_bebe(arg);
		break;
		
	case LCD_SHOW_ALARM_UNDER_CAPACITY:
		ret = __port_lcd_show_alarm_under_capacity(arg);
		break;
	
	case LCD_SHOW_ALARM_PRESS_UNUSUAL:
		ret = __port_lcd_show_alarm_press_unusual(arg);
		break;
	
	default:
		break;
	}
	
	return ret;
}


static int divid_cup_port(char cmd, unsigned long arg)
{
	int ret;
	
	switch (cmd) {
		
	case PORT_CMD_RTU:
		ret = divid_cup_port_rtu((struct port_cmd_rtu *)arg);
		if (ret < 0) {
			print("divid_cup_port_rtu, error[%d]\r\n", ret);
			return -1;
		}		
		break;
		
	case PORT_CMD_LCD:
		ret = divid_cup_port_lcd((struct port_cmd_lcd *)arg);
		if (ret < 0) {
			print("divid_cup_port_lcd, error[%d]\r\n", ret);
			return -1;
		}		
		break;
		
	default:		
		return -1;
	}
	
	return 0;
}


int divid_cup_install_bottle_inflat(char place)
{
	int ret;
	char ovt_sec = 10;
	struct port_cmd_rtu arg;
	unsigned short arg_press_max_val;
	unsigned short arg_press_min_val;		
	
	arg.cmd = RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_ON;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_set_press_ctl_main_gas_valve_on, error[%d]\r\n", 
			ret);
		return -1;
	}	
	
	ret = divid_cup_get_press_val_arg(&arg_press_max_val, &arg_press_min_val);
	if (ret < 0) {
		print("divid_cup_get_press_val_arg error[%d]\r\n", ret);
		return -1;
	}		
				
	while (ovt_sec--) {
		char last_press_val = 255;
		char press_val;
		
		arg.cmd = RTU_GET_PRESS_VAL;
		ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
		if (ret < 0) {
			print("rtu_ctl_press_switch_on, error[%d]\r\n", ret);
			return -1;
		}			
		
		press_val = arg.sub.press_val;
		if ((press_val > arg_press_min_val) 
			&& (press_val < arg_press_max_val)) {
			return -1;		
		}		
		
		if (press_val == last_press_val)
			break;
		
		last_press_val = press_val;
	}
	
	arg.cmd = RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_OFF;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_set_press_ctl_main_gas_valve_off, error[%d]\r\n", 
			ret);
		return -1;
	}

	
	
	return 0;		
}


/* pump the air from bottle */
int divid_cup_install_bottle_pump(char place)
{
	int ret;
	struct port_cmd_rtu arg;
		
	arg.cmd = RTU_SET_PRESS_CTL_MAIN_GAS_VALVE_OFF;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_set_press_ctl_main_gas_valve_off, error[%d]\r\n", 
			ret);
		return -1;
	}

	arg.cmd = RTU_SET_DIVID_CAP_GAS_VALVE_ON;
	arg.sub.divid_cap_gas_valve.place = place;
	if (ret < 0) {
		print("rtu_set_divid_cap_air_switch_off, error[%d]\r\n", ret);
		return -1;
	}	
	
	arg.cmd = RTU_SET_DIVID_CAP_BEBE_VALVE_OFF;
	arg.sub.divid_cap_bebe_valve.place = place;
	if (ret < 0) {
		print("rtu_set_divid_cap_bebe_valve_off, error[%d]\r\n", ret);
		return -1;
	}	
	
	arg.cmd = RTU_SET_PRESS_CTL_PUMP_GAS;
	if (ret < 0) {
		print("rtu_set_press_ctl_pump_gas, error[%d]\r\n", ret);
		return -1;
	}	
	
	arg.cmd = RTU_SET_DIVID_CAP_GAS_VALVE_OFF;
	arg.sub.divid_cap_gas_valve.place = place;
	if (ret < 0) {
		print("rtu_set_divid_cap_air_switch_off, error[%d]\r\n", ret);
		return -1;
	}
	
	return 0;
}


int get_bebe_val(unsigned short *bebe, char place, char bebe_cmd)
{
	int len;
	int ret;
	
	if (place == 1) {
		if (bebe_cmd == 1) {	
			len = store_param_read("bebe1_1", (char *)bebe);
			if (len == 0) {
				*bebe = 10;
				ret = store_param_save("bebe1_1", (char *)bebe, 2);
				if (ret < 0)
					return -1;
				
				return 0;
			}
		}
		
		if (bebe_cmd == 2) {	
			len = store_param_read("bebe1_2", (char *)bebe);
			if (len == 0) {
				*bebe = 20;
				ret = store_param_save("bebe1_2", (char *)bebe, 2);
				if (ret < 0)
					return -1;
			}
		}
		
		if (bebe_cmd == 3) {	
			len = store_param_read("bebe1_3", (char *)bebe);
			if (len == 0) {
				*bebe = 30;
				ret = store_param_save("bebe1_3", (char *)bebe, 2);
				if (ret < 0)
					return -1;
			}
		}	
	}
	
	if (place == 2) {
		if (bebe_cmd == 1) {	
			len = store_param_read("bebe2_1", (char *)bebe);
			if (len == 0) {
				*bebe = 10;
				ret = store_param_save("bebe2_1", (char *)bebe, 2);
				if (ret < 0)
					return -1;
			}
		}
		
		if (bebe_cmd == 2) {	
			len = store_param_read("bebe2_2", (char *)bebe);
			if (len == 0) {
				*bebe = 20;
				ret = store_param_save("bebe2_2", (char *)bebe, 2);
				if (ret < 0)
					return -1;
			}
		}
		
		if (bebe_cmd == 3) {	
			len = store_param_read("bebe2_3", (char *)bebe);
			if (len == 0) {
				*bebe = 30;
				ret = store_param_save("bebe2_3", (char *)bebe, 2);
				if (ret < 0)
					return -1;
			}
		}	
	}
	
	return -1;
}




static int divid_cup_cmd_check(unsigned short *bebe, char *place)
{
	struct port_cmd_rtu arg;
	struct port_cmd_lcd lcd_arg;
        int ret;
	char divid_up_cmd_time = 0;
	
	while (1) {
		
		/* 获取酒头的出洒命令 */
		arg.cmd = RTU_GET_DIVID_ASK;
		ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
		if (ret < 0) {
			print("rtu_get_divid_ask, error[%d]\r\n", ret);
			return -1;
		}		
		
		/* 出酒命令有效 */
		if (arg.sub.get_divid_cup.enable) { 
			arg.sub.get_divid_cup.enable = 0;
			
			/* 清除出酒指令 */
			arg.cmd = RTU_CLEAR_DIVID_ASK;
                	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
                	if (ret < 0) {
                    		print("rtu_clear_divid_ask, error[%d]\r\n", ret);
                    		return -1;
                	}
				
			/* 计算出实际的出酒值 */
			get_bebe_val(bebe, arg.sub.get_divid_cup.place, arg.sub.get_divid_cup.bebe);
			return 1;			
		} 
		
				
		/*  clear bottle ask from LCD  */
		arg.cmd = LCD_CLEAR_BOTTLE_ASK;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&lcd_arg);
		if (ret < 0) {
			print("divid_cup_port, error[%d]\r\n", ret);
			return -1;
		}		
		if (lcd_arg.sub.clear_bottle.enable)
			//break;	
		
		/*  install bottle ask from LCD  */
		arg.cmd = LCD_INSTALL_BOTTLE_ASK;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&lcd_arg);
		if (ret < 0) {
			print("divid_cup_port, error[%d]\r\n", ret);
			return -1;
		}		
		
		/* pump air from bottle */
		if (lcd_arg.sub.install_bottle.pump) {
			ret = divid_cup_install_bottle_pump(
				lcd_arg.sub.install_bottle.place);
			if (ret < 0) {
				print("divid_cup_install_bottle, error[%d]\r\n",
					ret);
				return -1;
			}			
		}

		/* inflat nitrogen to bottle */
		if (lcd_arg.sub.install_bottle.pump) {
			ret = divid_cup_install_bottle_inflat(
				lcd_arg.sub.install_bottle.place);
			if (ret < 0) {
				print("divid_cup_install_bottle_inflat,\
					error[%d]\r\n",
					ret);
				//return -1;
			}			
		}	
		
		vTaskDelay(100);
	}
	
	return 0;
}


static int divid_cup_judge(unsigned short bebe, char place)
{
	struct port_cmd_lcd arg;
	int total_capacity;
	int ret;
	
	arg.cmd = LCD_GET_AUTHOR;
	ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
	if (ret < 0) {
		print("divid_cup_port error[%d]\r\n", ret);
		return -1;
	}
	
	if (arg.sub.author_limit) {		
		
		arg.cmd = LCD_SHOW_ALARM_AUTHOR_LIMIT;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			print("LCD_SHOW_ALARM_AUTHOR_LIMIT error[%d]\r\n", ret);
			return -1;
		}
				
		print("divid_cup_judge author limit!\r\n");
		return -1;	
	}
	
	arg.cmd = LCD_SHOW_BEBE;
	arg.sub.show_bebe.bebe = bebe;
	arg.sub.show_bebe.place = place;
	
	ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
	if (ret < 0) {
		print("divid_cup_port PORT_CMD_LCD error[%d]\r\n", ret);
		return -1;
	}	
		
	total_capacity = divid_cup_get_bebe_capacity(place);
	if (total_capacity < 0) {
		print("divid_cup_get_bebe_capacity error[%d]\r\n", total_capacity);
		return -1;
	}		
	
	if (bebe < total_capacity) {
		
		arg.cmd = LCD_SHOW_ALARM_UNDER_CAPACITY;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			print("LCD_SHOW_ALARM_UNDER_CAPACITY error[%d]\r\n", ret);
			return -1;
		}		
		
		print("divid_cup_judge under capacity!\r\n");
		return -1;		
	}	
	
	return 0;
}



static int divid_cup(unsigned short bebe, char place)
{
	struct port_cmd_rtu arg;
	char ovt_100ms = 5;
	unsigned short arg_press_max_val;
	unsigned short arg_press_min_val;
	unsigned short press_val = 0;
	int ret;
		
	arg.cmd = RTU_CTL_PRESS_SWITCH_ON;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_ctl_press_switch_on, error[%d]\r\n", ret);
		return -1;
	}	
	
	//获取压力上下限
	ret = divid_cup_get_press_val_arg(&arg_press_max_val, &arg_press_min_val);
	if (ret < 0) {
		print("divid_cup_get_press_val_arg error[%d]\r\n", ret);
		return -1;
	}		
	
	//查询压力值
	while (ovt_100ms--) {
		arg.cmd = RTU_GET_PRESS_VAL;
		ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
		if (ret < 0) {
			print("rtu_ctl_press_switch_on, error[%d]\r\n", ret);
			return -1;
		}			
		
		press_val = arg.sub.press_val;
		if ((press_val > arg_press_min_val) 
			&& (press_val < arg_press_max_val)) {
			break;		
		}
		
		vTaskDelay(100);
	}
	
	
	if ((press_val <= arg_press_min_val) 
			|| (press_val >= arg_press_max_val)) {

		arg.cmd = LCD_SHOW_ALARM_PRESS_UNUSUAL;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			print("lcd_show_alarm_press_unusual error[%d]\r\n", ret);
			return -1;
		}		
		
		print("divid_cup_judge press_unusual!\r\n");
		//return -1;		
	}
	
	/* 向酒头发送出酒指令 */		
	arg.cmd = RTU_GET_DIVID_CAP_SWITCH_ON;
	arg.sub.divid_cap.bebe = bebe;
	arg.sub.divid_cap.place = place;
	arg.sub.divid_cap.press_val = press_val;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_get_divid_cap_switch_on, error[%d]\r\n", ret);
		return -1;
	}			
		
	return 0;
}


static int divid_cup_end(char bebe, char place)
{
	struct port_cmd_rtu arg;
	unsigned long ovt_100ms = 100;
	char divid_cup_res = 0;
	unsigned short arg_press_max_val;
	unsigned short arg_press_min_val;
	unsigned short press_val = 0;
	int ret;	
	
	ret = divid_cup_get_press_val_arg(&arg_press_max_val, &arg_press_min_val);
	if (ret < 0) {
		print("divid_cup_get_press_val_arg error[%d]\r\n", ret);
		return -1;
	}	
	
	/* 提示LCD进入显示酒量界面 */
	arg.cmd = LCD_SHOW_GOTO_BEBE_SHOW;
	ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
	if (ret < 0) {
		return -1;
	}		
	
	
	while (ovt_100ms--) {
		
		arg.cmd = RTU_GET_PRESS_VAL;
		ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
		if (ret < 0) {
			print("rtu_ctl_press_switch_on, error[%d]\r\n", ret);
			return -1;
		}			
		
		press_val = arg.sub.press_val;
		if ((press_val > arg_press_min_val) 
			&& (press_val < arg_press_max_val)) {
			divid_cup_res = -1;
			break;		
		}		
		
		
		/* 提示LCD刷新显示酒量界面 */
		arg.cmd = LCD_SHOW_REFRESH_BEBE;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			return -1;
		}		
			
		/* 从酒头获取出酒量 */
		arg.cmd = RTU_GET_DIVID_CAP_PROCESS;		
		arg.sub.divid_cap_pro.place = place;
		arg.sub.divid_cap_pro.bebe = 0;	
		arg.sub.divid_cap_pro.res = 0;	
		
		ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
		if (ret < 0) {
			print("rtu_get_divid_cap_switch_on, error[%d]\r\n", ret);
			return -1;
		}
		
		if (arg.sub.divid_cap_pro.res == 1) {
			divid_cup_res = 1;
			break;
		}
		
		if (arg.sub.divid_cap_pro.bebe >= bebe) {
			divid_cup_res = 2;
			break;		
		}
		
		vTaskDelay(100);
	}
	
	/* 发送命令关闭出酒 */
	arg.cmd = RTU_GET_DIVID_CAP_SWITCH_OFF;
	ret = divid_cup_port(PORT_CMD_RTU, (unsigned long)&arg);
	if (ret < 0) {
		print("rtu_get_divid_cap_switch_on, error[%d]\r\n", ret);
		return -1;
	}	
	
	if (divid_cup_res == 1) {
		arg.cmd = LCD_SHOW_ALARM_PRESS_UNUSUAL;
		ret = divid_cup_port(PORT_CMD_LCD, (unsigned long)&arg);
		if (ret < 0) {
			print("lcd_show_alarm_press_unusual error[%d]\r\n", ret);
			return -1;
		}		
		
		print("divid_cup_judge press_unusual!\r\n");	
		
	}
	
	if (divid_cup_res == 0)
		print("divid cup over time!\r\n");			
	
	print("divid cup compelet[%d]!\r\n", divid_cup_res);
	
	return 0;
}


int divid_cup_ctrl(void)
{
	int ret;
	unsigned short bebe;
	char place;
	
	ret = divid_cup_cmd_check(&bebe, &place);
	if (ret < 0) {
		print("divid_cup_cmd_check error[%d]\r\n", ret);
		return -1;
	} 	

	ret = divid_cup_judge(bebe, place);
	if (ret < 0) {
		print("divid_cup_judge error[%d]\r\n", ret);
		return -1;
	}
	
	ret = divid_cup(bebe, place);
	if (ret < 0) {
		print("divid_cup error[%d]\r\n", ret);
		return -1;
	}

	ret = divid_cup_end(bebe, place);
	if (ret < 0) {
		print("divid_cup error[%d]\r\n", ret);
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
void task_divid_cup(void *pvParameters)
{
     
    while(1)
    {
	divid_cup_ctrl();
	print("task_divid_cup run!\r\n");
        vTaskDelay(10000); 
    }        
}


