/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_wifi.c
      生成日期: 2016.09.25
      作	   者：like
      功能说明：wifi通信模块
      其他说明：
      修改记录：
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>

#include "bsp_uart.h"
#include "esp8266.h"
#include "task_wifi.h"
#include "upgrade.h"

/* wifi功能调试开关 */
//#define CONFIG_WIFI_DEBUG

/* wifi接收缓存区大小 */
#define WIFI_RX_BUFF_SIZE 1024
#define WIFI_TX_BUFF_SIZE 1024

/* wifi 调试接口函数 */
#ifdef CONFIG_WIFI_DEBUG
#define wifi_print(fmt,args...) debug(fmt, ##args)
#else
#define wifi_print(fmt,args...)
#endif


/* wifi 通信功能所用到的全局变量结构 */
struct wifi_param {
	char mode;  /* 应用模式，AP, STA*/
	char rxbuf[WIFI_RX_BUFF_SIZE]; /* 数据接收缓存区 */
	int  rxlen;			/* 接收缓存区数据长度 */
	char txbuf[WIFI_TX_BUFF_SIZE];	 /* 数据发送缓存区 */
	int  txlen;			/* 发送缓存区数据长度 */

	char *sta_ssid; /* STA模式下连接路由器的用户名 */
	char *sta_psw;	/* STA模式下连接路由器的密码 */

	char *ap_ssid;	/* AP模式下作为热点的用户名 */
	char *ap_psw;	/* AP模式下作为热点的密码 */

	char server_ip[4]; /* 服务器IP */
	unsigned short server_port; /* 服务器端口 */

	char client_ip[4]; /* 客户端IP */
	unsigned short client_port; /* 客户端端口 */

	char gateway[4]; /* 网关 */
	char netmask[4]; /* 子网掩码 */

	int link_id;
	char link_stat;
};


/* wifi 通信功能所用到的全局变量定义与出厂值设定 */
static struct wifi_param wifi = {
	.sta_ssid = "like",
	.sta_psw = "LIKEzxmliyi475829",

	.ap_ssid = "fenjiuji",
	.ap_psw = "12345678",

	.client_ip = {192, 168, 1, 102},
	.client_port = 8001,
	.gateway = {192, 168, 1, 1},
	.netmask = {255, 255, 255, 0},

	.server_ip = {192, 168, 1, 103},
	.server_port = 8017,
};


QueueHandle_t xQueue_wifi_tx, xQueue_wifi_fd;


/* creat  queue, use to send to other diff task */
int wifi_queue_creat(void)
{
	xQueue_wifi_fd = xQueueCreate(1, sizeof( struct wifi_msg * ));
	if (xQueue_wifi_fd == 0) {
		return -1;
	}

	return 0;
}


/* the data which receiveing from wifi port, post to other task */
static int wifi_msg_post(char *rxbuf, int len)
{
	struct wifi_msg message;
	BaseType_t ret;

	message.rxbuf = rxbuf;
	message.size = len;
	ret = xQueueSend(xQueue_wifi_fd, ( void * ) &message, ( TickType_t ) 3000 );
	if (ret != pdTRUE) 
		return -1;

	return 0;
}


/* pend data from other task, then send this data to wifi port */
static int wifi_msg_pend(char *rxbuf, int size)
{
	struct upgrade_msg *pmessage;

	if( xQueue_upgrade_fd != 0 ) {
		if( xQueueReceive( xQueue_upgrade_fd, &( pmessage ), ( TickType_t ) 10 ) ) {
			if (pmessage->len > size) {
				pmessage->len = size;
			}

			memcpy(rxbuf, pmessage->buff, pmessage->len);
			return pmessage->len;
		}
	}

	return 0;
}


/******************************************************************************
    功能说明：STA模式下作为客户端与服务器连接
    输入参数：param wifi参数
    输出参数：无
    返 回 值：0 成功 -1失败
*******************************************************************************/
static int wifi_sta_tcp_client(struct wifi_param *param)
{
	int len;
	int ret;
	int time; /*没有收到服务器tcp数据计时 */

	/* wifi模块初始化 */
	ret = esp8266_init();
	wifi_print("[wifi]init......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/*将wifi模块设为STA模式，连接路由器 */
	ret = esp8266_join_ap(param->sta_ssid, param->sta_psw);
	wifi_print("[wifi]ap ssid: %d\r\n", param->sta_ssid);
	wifi_print("[wifi]ap psw : %d\r\n", param->sta_psw);
	wifi_print("[wifi]join ap......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* 建立TCP连接 */
	ret = esp8266_link_server("TCP", param->server_ip, param->server_port);
	wifi_print("[wifi]link server ip: %d.%d.%d.%d\r\n",
		   param->server_ip[0], param->server_ip[1],
		   param->server_ip[2], param->server_ip[3]);
	wifi_print("[wifi]link server port: %d\r\n", param->server_port);
	wifi_print("[wifi]link server......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* TCP 通信，这里采用了透传的方式 */
	while (1) {
		/* 从服务器接收TCP数据 */
		len = esp8266_read(wifi.rxbuf + wifi.rxlen,
				   WIFI_RX_BUFF_SIZE - wifi.rxlen);
		if (len > 0) {
			wifi_print("[wifi]tcp read......[%d]\r\n", len);
			wifi.rxlen += len;

			/* 当收到服务器数据后，超时清零 */
			time = 0;
		}

		/* 将接收到的数据发送到服务器 */
		if (wifi.rxlen > 0) {
			/* 发送TCP数据 */
			ret = esp8266_write(wifi.rxbuf, wifi.rxlen);
			wifi_print("[wifi]tcp write......[%d]\r\n", wifi.rxlen);
			if (ret < 0) {
				return -1;
			}

			wifi.rxlen = 0;
		}

		/* 发送TCP测试数据,后面可以删除 */
		ret = esp8266_write("[wifi]rx: hello word!!\r\n", 23);
		wifi_print("[wifi]tcp write......[%d]\r\n", wifi.rxlen);
		if (ret < 0) {
			return -1;
		}

		/*
		 * TCP没有收到服务器数据后，超时计时累加
		 * 超时大于定值就认为TCP链路断开了，需要
		 * 重新发起连接
		 */
		time++;
		if (time > 5 * 60) {
			time = 0;
			wifi_print("[wifi]tcp link ovt!!!\r\n");
			return -1;
		}

		vTaskDelay(1000);
	}
}



/******************************************************************************
    功能说明：wifi模式：wifi作为AP热点, PC或手机用为STA
	      TCP模式： 服务器
    输入参数：param wifi参数
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
static int wifi_ap_tcp_server(struct wifi_param *param)
{
	int ret;
	int len;
	int i;

	/* wifi模块初始化 */
	ret = esp8266_init();
	wifi_print("[wifi]init......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* 创建wifi热点 */
	ret = ESP8226_set_ap(param->ap_ssid, param->ap_psw);
	wifi_print("[wifi]ap ssid: %s\r\n", param->ap_ssid);
	wifi_print("[wifi]ap psw : %s\r\n", param->ap_psw);
	wifi_print("[wifi]set ap......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* 设置wifi热点的ip, 网关, 子网掩码 */
	ret = ESP8226_set_ip(param->client_ip, param->gateway, param->netmask);

	wifi_print("[wifi]set ip: %d.%d.%d.%d\r\n",
		   param->client_ip[0], param->client_ip[1],
		   param->client_ip[2], param->client_ip[3]);

	wifi_print("[wifi]set gateway: %d.%d.%d.%d\r\n",
		   param->gateway[0], param->gateway[1],
		   param->gateway[2], param->gateway[3]);

	wifi_print("[wifi]set netmask: %d.%d.%d.%d\r\n",
		   param->netmask[0], param->netmask[1],
		   param->netmask[2], param->netmask[3]);

	wifi_print("[wifi]config AP ip, gateway, netmask......[%d]\r\n", ret);

	if (ret < 0) {
		return -1;
	}

	/* wifi服务器配置 */
	ret = ESP8226_server_config(param->client_port);
	wifi_print("[wifi]server listening port: %d\r\n", param->client_port);
	wifi_print("[wifi]server config......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* wifi服务器监听端口 */
	ret = ESP8226_server_listening(&param->link_id, 60000);
	wifi_print("[wifi]server listening......[%d]\r\n", ret);
	if (ret < 0) {
		return -1;
	}

	uart_init(UART_1, 115200);

	wifi.link_stat = 1;

	while (1) {
		/* 从服务器接收数据 */
		len = esp8266_mux_receive(&wifi.link_id, wifi.rxbuf + wifi.rxlen,
					  WIFI_RX_BUFF_SIZE - wifi.rxlen);			
		if (len > 0) {
			wifi.rxlen += len;

			ret = wifi_msg_post(wifi.rxbuf,  wifi.rxlen);
			if (ret == 0) {
				memset(wifi.rxbuf, 0, WIFI_RX_BUFF_SIZE);
				wifi.rxlen = 0;
			}

			wifi_print("[wifi]tcp read......[%d]\r\n", wifi.rxlen);
			for (i = 0; i < len; i++) {
				wifi_print("%02X ", wifi.rxbuf[i]);
			}

			wifi_print("\r\n");
		} 

		/* 将接收到的数据发送到服务器 */
		wifi.txlen = wifi_msg_pend(wifi.txbuf, WIFI_TX_BUFF_SIZE);
		if (wifi.txlen > 0) {
			/* 发送TCP数据 */
			ret = esp8266_mux_send(wifi.link_id, wifi.txbuf, wifi.txlen);
			wifi_print("[wifi]tcp write......[%d]\r\n", wifi.txlen);
			if (ret < 0) {
				wifi.link_stat = 0;
				return -1;
			}

			memset(wifi.txbuf, 0, wifi.txlen);
			wifi.txlen = 0;
		}

		vTaskDelay(100);
	}
}



/******************************************************************************
    功能说明：主函数
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_wifi(void *pvParameters)
{
	int ret;

	wifi_print("[wifi]task runing......OK\r\n");
	wifi.mode = 1;
	wifi_print("[wifi]set AP mode!\r\n");

	for( ;; ) {
		switch (wifi.mode) {
		case 0: /* STA 模式下的作为TCP客户端连接服务器, 采用透传的方式来通信 */
			ret = wifi_sta_tcp_client(&wifi);
			if (ret < 0) {
				wifi_print("[wifi]STA tcp client......[%d]\r\n", ret);
			}
			break;
		case 1:
			ret = wifi_ap_tcp_server(&wifi);
			if (ret < 0) {
				wifi_print("[wifi]AP tcp server......[%d]\r\n", ret);
			}

			break;

		default:
			break;
		}

		vTaskDelay(5000);
	}
}


void wifi_link_status_get(char *status)
{
	*status = wifi.link_stat; 
}

