/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_ethernet.c
      生成日期: 2016.09.25
      作    者：like
      功能说明：以太网通信模块
      其他说明：
      修改记录：
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "w5500.h"
#include "store_param.h"
#include "task_ethernet.h"
#include "bsp_uart.h"



const char server_ip[4] = {192, 168, 1, 101};
const char server_port[2] = {0x1F, 0x51}; /* 8017*/
const char client_port[2] = {0x1F,0x41}; /* 8001*/

const char terminal_ip[4] = {192, 168, 1, 100};
const char mac[6] = {0x22, 0x22, 0x11, 0x11, 0x11, 0x2};
const char submask[4] = {255, 255, 255, 0};
const char gateway[4] = {192, 168, 1, 1};


struct ethernet {
	char server_ip[4];
	char server_port[2];
	char client_port[2];

	char terminal_ip[4];
	char mac[6];
	char submask[4];
	char gateway[4];

	char rxbuf[1024];
	int rxlen;
};

struct ethernet net;



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int ethernet_param_init(struct ethernet *arg)
{
	int ret;
	char len;

	len = store_param_read("server_ip", arg->server_ip);
	if (len == 0) {
		ret = store_param_save("server_ip", (char *)server_ip, 4);
		if (ret < 0) {
			kprintf("server_ip read error\r\n");
			return -1;
		}
	}

	len = store_param_read("server_port", arg->server_port);
	if (len == 0) {
		ret = store_param_save("server_port", (char *)server_port, 2);
		if (ret < 0) {
			return -1;
		}
	}

	len = store_param_read("client_port", arg->client_port);
	if (len == 0) {
		ret = store_param_save("client_port", (char *)client_port, 2);
		if (ret < 0) {
			return -1;
		}
	}

	len = store_param_read("terminal_ip", arg->terminal_ip);
	if (len == 0) {
		ret = store_param_save("terminal_ip", (char *)terminal_ip, 4);
		if (ret < 0) {
			return -1;
		}
	}

	len = store_param_read("mac", arg->mac);
	if (len == 0) {
		ret = store_param_save("mac", (char *)mac, 6);
		if (ret < 0) {
			return -1;
		}
	}

	len = store_param_read("submask", arg->submask);
	if (len == 0) {
		ret = store_param_save("submask", (char *)submask, 4);
		if (ret < 0) {
			return -1;
		}
	}

	len = store_param_read("gateway", arg->gateway);
	if (len == 0) {
		ret = store_param_save("gateway", (char *)gateway, 4);
		if (ret < 0) {
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
int ethernet_param_get(struct ethernet *arg)
{
	char len;

	len = store_param_read("server_ip", arg->server_ip);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("server_port", arg->server_port);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("client_port", arg->client_port);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("terminal_ip", arg->terminal_ip);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("mac", arg->mac);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("submask", arg->submask);
	if (len == 0) {
		return -1;
	}

	len = store_param_read("gateway", arg->gateway);
	if (len == 0) {
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
int ethernet_tcp_client_test(struct ethernet *arg)
{
	int sockfd;
	int ret;
	int len;

	ret = ethernet_param_get(arg);

	w5500_init();

	w5500_set_gateway(arg->gateway);

	w5500_set_submask(arg->submask);

	w5500_set_mac(arg->mac);

	w5500_set_terminal_ip(arg->terminal_ip);

	sockfd = w5500_socket(0, "TCP");
	if (sockfd < 0) {
		return -1;
	}

	ret = w5500_connect(sockfd, arg->server_ip, arg->server_port, arg->client_port);
	if (ret < 0) {
		return -1;
	}

	while (1) {
		ret = w5500_write(sockfd, "socket 0 tx: hello word!!\r\n", 27);
		if (ret < 0) {
			return -1;
		}

		len = w5500_read(sockfd, arg->rxbuf + arg->rxlen, 1024 - arg->rxlen);
		if (len < 0) {
			return -1;
		}

		if (len > 0) {
			arg->rxlen += len;
		}

		if (arg->rxlen > 0) {
			ret = w5500_write(sockfd, arg->rxbuf, arg->rxlen);
			if (ret < 0) {
				return -1;
			}

			memset(arg->rxbuf, 0, 1024);
			arg->rxlen = 0;
		}

		vTaskDelay(1000);
	}
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_ethernet(void *pvParameters)
{
	ethernet_param_init(&net);

	for( ;; ) {
		ethernet_tcp_client_test(&net);
		vTaskDelay(1000);
	}
}