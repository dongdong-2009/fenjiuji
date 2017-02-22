/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: esp8266.c
      生成日期: 2016.09.25
      作	   者：like
      功能说明：wifi通信
      其他说明：
      修改记录：
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "stm32f1xx_hal.h"
#include "bsp_uart.h"


/* 调试开关 */
#define CONFIG_ESP8266_DEBUG

/* 调试接口函数 */
#ifdef CONFIG_ESP8266_DEBUG
#define esp8266_print(fmt,args...) debug(fmt, ##args)
#else
#define esp8266_print(fmt,args...)
#endif


/* esp8266模块延时函数定义 */
#define	 esp8266_delay(x)  vTaskDelay(x);


/******************************************************************************
    功能说明：复位引脚初始化
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int esp8266_reset_gpio_init(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* PB8 --> pow */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStructure.Mode	 = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull	 = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 关电源 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

	/* PB9 --> RESET */
	GPIO_InitStructure.Mode	 = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull	 = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	return 0;
}


/******************************************************************************
    功能说明：硬件复位
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void esp8266_reset(void)
{
	esp8266_reset_gpio_init();

	/* 关电源 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

	esp8266_delay(500);
	/* 开电源 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

	esp8266_delay(500);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	esp8266_delay(2000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int esp8266_send_byte(char *buff, int len)
{
	int ret;

	/* 发送数据到串口*/
	ret = bsp_uart_send(UART_3, buff, len);
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：发送AT命令
    输入参数：buff AT命令缓存
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_at_cmd_send(char *buff)
{
	int len;
	int ret;


	if (buff == NULL) {
		return -1;
	}

	/* AT命令最长不能大于128 */
	len = strlen(buff);
	if (len > 128) {
		return -1;
	}

	/* 每一次发送命令就对串口进行初始化，防止串口发出错 */
	uart_init(UART_3, 115200);

	/* 将数据从串口发出去 */
	ret = esp8266_send_byte(buff, len);
	if (ret < 0) {
		return -1;
	}

	esp8266_print("[esp8266] tx cmd: ");
	esp8266_print(buff);
	return 0;
}



/******************************************************************************
    功能说明：esp8266模块从串口接收数据
    输入参数：buff 接收数据缓存  size 期望接收的长度
    输出参数：无
    返 回 值：实际接收到的数据长度
*******************************************************************************/
int esp8266_receive_byte(char *buff, int size, int ovt)
{
	int len;

	/* 从串口接收数据 */
	//len = bsp_uart_receive(UART_3, buff, size);

	len = uart3_receive_packet(buff, size, ovt);
	
	
	return len;
}



/******************************************************************************
    功能说明：esp8266模块接收AT命令
    输入参数：buff 接收数据缓存  size 期望接收的长度 ovt 超时等待时间
    输出参数：无
    返 回 值：AT命令的长度
*******************************************************************************/
int esp8266_at_cmd_receive(char *buff, int size, int ovt)
{
	int len = 0;
	int time = ovt;

	while (--time > 0) {
		/* 从串口接收数据 */
		len = esp8266_receive_byte(buff, size, 50);	
		if (len > 0) 
			break;

		esp8266_delay(1);
	}

	if (len > 0) {
		esp8266_print("[esp8266] rx cmd: ");
		esp8266_print(buff);

		/* 每一次发送命令就对串口进行初始化，防止串口发出错 */
		uart_init(UART_3, 115200);
	}

	return len;
}



/******************************************************************************
    功能说明：执行AT命令：开关回显
    输入参数：无
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_ate(void)
{
	int ret;
	char rxbuf[16] = {0};
	char len;

	/* 发送AT命令 */
	ret = esp8266_at_cmd_send("ATE0\r\n");
	if (ret < 0) {
		return -1;
	}

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(rxbuf, 16, 1000);
	if (len > 0) {
		if (strstr(rxbuf, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：执行AT命令：查询版本信息
    输入参数：无
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_gmr(void)
{
	int ret;
	char rxbuf[128] = {0};
	char len;

	/* 发送AT命令 */
	ret = esp8266_at_cmd_send("AT+GMR\r\n");
	if (ret < 0) {
		return -1;
	}

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(rxbuf, 128, 1000);
	if (len > 0) {
		if (strstr(rxbuf, "AT version") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：执行AT命令：所有参数恢复出厂
    输入参数：无
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_restore(void)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	ret = esp8266_at_cmd_send("AT+RESTORE\r\n");
	if (ret < 0) {
		return -1;
	}

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：执行AT命令：设置esp8266模式
    输入参数：mode 1 station 模式
		   2 softAP 模式
		   3 softAP + station 模式
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cwmode(char mode)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CWMODE_CUR=%d\r\n", mode);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：执行AT命令：设置esp8266 DHCP模式
    输入参数：
	       <mode>
		0 : 设置ESP8266 softAP
		1 : 设置ESP8266 station
		2 : 设置ESP8266 softAP 和station
		<en>
		0 : 关闭DHCP
		1 : 开启DHCP
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cwdhcp(char mode, char en)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CWDHCP_CUR=%d,%d\r\n", mode, en);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}




/******************************************************************************
    功能说明：执行AT命令：在STA模式下，连接AP热点
    输入参数：ssid  wifi热点的用户名
	      psw   wifi热点的密码
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cwjap(char *ssid, char *psw)
{
	int ret;
	char buff[64] = {0};
	char len = 0, rxlen = 0;
	char ovt = 15;

	/* 发送AT命令 */
	sprintf(buff, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, psw);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	while (ovt--) {
		len = esp8266_at_cmd_receive(buff + rxlen, 64 - rxlen, 500);
		if (len > 0) {
			rxlen += len;

			/* 热点连接成功 */
			if ((strstr(buff, "CONNECTED") != NULL)
			    && (strstr(buff, "GOT IP") != NULL)
			    && (strstr(buff, "OK") != NULL)) {
				return 0;
			}

			else if (strstr(buff, "ERROR\r\n") != NULL) {
				return -1;
			}
		}

		esp8266_delay(1000);
	}

	return -1;
}



/******************************************************************************
    功能说明：执行AT命令：设置ESP8266 softAP 的配置参数
    输入参数：ssid -AP模式下自己创建的wifi热点的用户名
	      psw  -AP模式下自己创建的wifi热点的密码
	      chl  -wifi频道
	      ecn -加密方式 不支持WEP
		  -0 OPEN
		  -2 WPA_PSK
		  -3 WPA2_PSK
		  -4 WPA_WPA2_PSK
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cwsap(char *ssid, char *psw, char chl, char ecn)
{
	int ret;
	char buff[64] = {0};
	char len = 0;

	/* 发送AT命令 */
	sprintf(buff, "AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d\r\n", ssid, psw, chl, ecn);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 1000);
	if (len > 0) {
		if (strstr(buff, "OK") != NULL) {
			return 0;
		}

		else if (strstr(buff, "ERROR\r\n") != NULL) {
			return -1;
		}
	}

	return -1;
}




/******************************************************************************
    功能说明：设置传输模式
	      0 普通传输模式
	      1 透传模式，仅支持TCP 单连接和UDP 固定通信对端的情况
    输入参数：mode 传输模式
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipmode(char mode)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPMODE=%d\r\n", mode);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}


/******************************************************************************
    功能说明：设置多连接
    输入参数：mode 0 单连接模式
		   1 多连接模式
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipmux(char mode)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPMUX=%d\r\n", mode);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：建立TCP server
    输入参数：mode
		0 关闭server
		1 建立server
	      port 端口
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipserver(char mode, unsigned short port)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPSERVER=%d,%d\r\n", mode, port);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}




/******************************************************************************
    功能说明：设置TCP server 超时时间
    输入参数：time 超时时间
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipsto(unsigned short time)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPSTO=%d\r\n", time);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "OK\r\n") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：建立TCP连接或者UDP传输
    输入参数：type 连接类型
	      ip 服务器IP
	      port 服务器端口
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipstart(char *type, char *ip, unsigned short port)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	if ((strcmp(type, "TCP") != 0)
	    && (strcmp(type, "UDP") !=0)) {
		return -1;
	}

	sprintf(buff, "AT+CIPSTART=\"%s\",\"%d.%d.%d.%d\",%d\r\n",
		type,
		ip[0], ip[1], ip[2], ip[3],
		port);

	memset(buff, 0, 64);

	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if ((strstr(buff, "CONNECT") != NULL)
		    && (strstr(buff, "OK") != NULL)) {
			return 0;
		}
	}

	return -1;
}


/******************************************************************************
    功能说明：发送数据, 进入透传模式
    输入参数：无
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipsend(void)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	ret = esp8266_at_cmd_send("AT+CIPSEND\r\n");
	if (ret < 0) {
		return -1;
	}

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, ">") != NULL) {
			return 0;
		}
	}

	return -1;
}


/******************************************************************************
    功能说明：在多链路下，采用命令的模式发送数据
    输入参数：link_id 链路ID, txbuf 发送缓存 len 发送长度
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cip_mux_send(char link_id, char *txbuf, int txlen)
{
	int ret;
	char buff[64] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPSEND=%d,%d\r\n", link_id, txlen);
	vTaskDelay(1000);
	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 64);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len <= 0) 
		return -1;				
	if (strstr(buff, ">") == NULL) 
		return -1;

	esp8266_send_byte(txbuf, txlen);
	memset(buff, 0, 64);
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) 
		return 0;
		
	return -1;
}



/******************************************************************************
    功能说明：在多链路下，采接收网络数据
    输入参数：link_id 链路ID, txbuf 发送缓存 len 发送长度
    输出参数：无
    返 回 值：数据长度
*******************************************************************************/
static int at_exe_cmd_ipd(int *link_id, char *rxbuf, int size)
{
	int len;
	char *head = 0;

	/* 接收AT命令处理 */
	len = esp8266_receive_byte(rxbuf, size,  50);
	if (len > 8) {
		esp8266_print("[esp8266] rx cmd: %s\r\n", rxbuf);
		if (strstr(rxbuf, "CLOSED") != 0) {
			*link_id = -1;
			return 0; /* 链路断开 */
		}

		if (strstr(rxbuf, "CONNECT") != NULL) {
			*link_id = 0;
			return 0; /* 链路连接成功 */
		}

		head = strstr(rxbuf, "+IPD,");
		if (head != 0) {
			sscanf(head, "+IPD,%d,%d:", link_id, &len);
			esp8266_print("[esp8266]link_id,len: %d,%d\r\n", *link_id, len);
			if (len > 0) {
				head = strstr(rxbuf, ":");
				if (head != 0) {
					//memcpy(rxbuf, head + 1, len);

					for (int i = 0; i < len; i++)
						rxbuf[i] = head[1 + i];
					return len;
				}
			}
		}
	}

	return 0;
}



/******************************************************************************
    功能说明：esp8266模块初始化
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int esp8266_init(void)
{
	int ret;

	/* 硬件引脚复位 */
	esp8266_reset();

	/* 通信口初始化 */
	uart_init(UART_3, 115200);

	/* 模块参数恢复出厂 */
	at_exe_cmd_restore();

	esp8266_delay(5000);

	/* 关回显 */
	ret = at_exe_cmd_ate();
	if (ret < 0) {
		return -1;
	}

	/* 查询版本信息 */
	ret = at_exe_cmd_gmr();
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：连接AP的wifi热点
    输入参数：ssid AP用户名
	      psw AP密码
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_join_ap(char *ssid, char *psw)
{
	int ret;

	/* station 模式  */
	ret = at_exe_cmd_cwmode(1);
	if (ret < 0) {
		return -1;
	}

	/* 配置用户名和密码 */
	ret = at_exe_cmd_cwjap(ssid, psw);
	if (ret < 0) {
		return -1;
	}

	return 0;
}


/******************************************************************************
    功能说明：配置AP的wifi热点
    输入参数：ssid AP用户名
	      psw AP密码
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int ESP8226_set_ap(char *ssid, char *psw)
{
	int ret;

	/* AP 模式  */
	ret = at_exe_cmd_cwmode(2);
	if (ret < 0) {
		return -1;
	}

	/* 配置AP的wifi热点 */
	ret = at_exe_cmd_cwsap(ssid, psw, 5, 3);
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：执行AT命令: AT配置AP的网络参数
    输入参数：ip  本身的IP地址
	      gateway 网关
	      netmask 子网掩码
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_exe_cmd_cipap(char *ip, char *gateway, char *netmask)
{
	int ret;
	char buff[80] = {0};
	char len;

	/* 发送AT命令 */
	sprintf(buff, "AT+CIPAP_CUR=\"%d.%d.%d.%d\",\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n",
		ip[0], ip[1], ip[2], ip[3],
		gateway[0], gateway[1], gateway[2], gateway[3],
		netmask[0], netmask[1], netmask[2], netmask[3]);

	ret = esp8266_at_cmd_send(buff);
	if (ret < 0) {
		return -1;
	}

	memset(buff, 0, 80);

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 80, 500);
	if (len > 0) {
		if (strstr(buff, "OK") != NULL) {
			return 0;
		}
	}

	return -1;
}



/******************************************************************************
    功能说明：配置AP的网络参数
    输入参数：ip  本身的IP地址
	      gateway 网关
	      netmask 子网掩码
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int ESP8226_set_ip(char *ip, char *gateway, char *netmask)
{
	int ret;

	/* 配置AP模式下关闭DHCP */
	ret = at_exe_cmd_cwdhcp(0,0);
	if (ret < 0) {
		return -1;
	}

	/* 配置AP的网络参数 */
	ret = at_exe_cmd_cipap(ip, gateway, netmask);
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：服务器参数配置
    输入参数：port 监听端口
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int ESP8226_server_config(unsigned short port)
{
	int ret;

	/* 设置为多连接模式 */
	ret = at_exe_cmd_cipmux(1);
	if (ret < 0) {
		return -1;
	}

	/* 建立TCP server */
	ret = at_exe_cmd_cipserver(1, port);
	if (ret < 0) {
		return -1;
	}

	/* 设置TCP server 超时时间, 必须建立好TCP server才能设置 */
	ret = at_exe_cmd_cipsto(300);
	if (ret < 0) {
		return -1;
	}

	return 0;
}


/******************************************************************************
    功能说明：查询是否有客户端连接
    输入参数：无
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int at_query_cmd_server_connet(void)
{
	char buff[64] = {0};
	char len;

	/* 接收AT命令处理 */
	len = esp8266_at_cmd_receive(buff, 64, 500);
	if (len > 0) {
		if (strstr(buff, "0,CONNECT") != NULL) {
			return 0;
		}

		/* 每一次发送命令就对串口进行初始化，防止串口发出错 */
		uart_init(UART_3, 115200);
	}

	return -1;
}



/******************************************************************************
    功能说明：TCP服务器模式监听
    输入参数：time 监听时间
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int ESP8226_server_listening(char *link_id, unsigned long time)
{
	int ret;

	/* 监听倒计时 */
	while (time--) {
		/* 查询是否有客户端连接 */
		ret = at_query_cmd_server_connet();
		if (ret == 0) {
			return 0;
		}

		esp8266_delay(500);
	}

	return -1;
}


/******************************************************************************
    功能说明：TCP模式连接服务器
    输入参数：type 协议
	      server_ip 服务器IP
	      server_port 服务器端口
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_link_server(char *type, char *server_ip, unsigned short server_port)
{
	int ret;

	/* 建立TCP连接 */
	ret = at_exe_cmd_cipstart(type, server_ip, server_port);
	if (ret < 0) {
		return -1;
	}

	/* 设置为透传模式 */
	ret = at_exe_cmd_cipmode(1);
	if (ret < 0) {
		return -1;
	}

	/* 进入数据传输模式 */
	ret = at_exe_cmd_cipsend();
	if (ret < 0) {
		return -1;
	}

	return 0;
}


/******************************************************************************
    功能说明：采用透传的方式读数据
    输入参数：rxbuf 接收缓存 size 接收最大长度
    输出参数：无
    返 回 值：接收数据长度
*******************************************************************************/
int esp8266_read(char *rxbuf, int size)
{
	int len;

	/* 从串口读取数据 */
	len = esp8266_receive_byte(rxbuf, size, 50);
	if (len > 0) {
		return len;
	}

	return 0;
}


/******************************************************************************
    功能说明：采用透传的方式写数据
    输入参数：rxbuf 发送缓存 size 发送长度
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_write(char *rxbuf, int len)
{
	int ret;

	ret = esp8266_send_byte(rxbuf, len);
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：采用命令的方式写数据
    输入参数：rxbuf 发送缓存 size 发送长度
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_mux_send(char link_id, char *txbuf, int len)
{
	int ret;

	ret = at_exe_cmd_cip_mux_send(link_id, txbuf, len);
	if (ret < 0) {
		return -1;
	}

	return 0;
}



/******************************************************************************
    功能说明：采用命令的方式接收数据
    输入参数：rxbuf 发送缓存 size 发送长度
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int esp8266_mux_receive(int *link_id, char *rxbuf, int size)
{
	int len;

	len = at_exe_cmd_ipd(link_id, rxbuf, size);

	return len;
}

