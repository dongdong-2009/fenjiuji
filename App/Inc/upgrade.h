/*
 * upgrade
 * likejshy@126.com
 * 2016-12-16
 */
 
#ifndef _TASK_UPGRADE_H
#define _TASK_UPGRADE_H

#include <queue.h>

#define UPGRADE_PORT_COM  0
#define UPGRADE_PORT_WIFI 1


struct upgrade_msg
{
	int len;
	char *buff;
};

extern QueueHandle_t xQueue_upgrade_fd;

extern int upgrade_msg_post(char *txbuf, int len);
extern int upgrade_msg_pend(char *rxbuf, int size);
extern void task_upgrade(void *pvParameters);
extern int upgrade_trigger(char port);


#endif /*  _TASK_UPGRADE_H */



