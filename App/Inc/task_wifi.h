/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_wifi.h
      生成日期: 2016.09.25
      作    者：like
      功能说明：wifi通信模块
      其他说明： 
      修改记录：
*******************************************************************************/
#ifndef _TASK_WIFI_H
#define _TASK_WIFI_H

#include <queue.h>

struct wifi_msg
{
    int size;
    char *rxbuf;
};

extern QueueHandle_t xQueue_wifi_fd;


extern void task_wifi(void *pvParameters);
extern int wifi_send_byte(char *txbuf, int len);
extern int wifi_receive_byte(char *rxbuf, int size);
extern int wifi_queue_creat(void);
extern void wifi_link_status_get(char *status);

#endif /*  _TASK_WIFI_H */

