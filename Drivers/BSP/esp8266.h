/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: esp8266.h
      ��������: 2016.09.25
      ��    �ߣ�like
      ����˵����wifiͨ��
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#ifndef _ESP_8226_H
#define _ESP_8226_H

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef Uart1Handle;
extern UART_HandleTypeDef Uart2Handle;
extern UART_HandleTypeDef Uart3Handle;


extern int esp8266_init(void);
extern int esp8266_join_ap(char *ssid, char *psw);
extern int esp8266_link_server(char *type, char *server_ip, 
                               unsigned short server_port);

extern int ESP8226_set_ap(char *ssid, char *psw);
extern int ESP8226_server_listening(char *link_id, unsigned short time);

extern int esp8266_write(char *rxbuf, int len);
extern int esp8266_read(char *rxbuf, int size);

extern int ESP8226_set_ip(char *ip, char *gateway, char *netmask);
extern int ESP8226_server_config(unsigned short port);

extern int esp8266_mux_receive(char *link_id, char *rxbuf, int size);
extern int esp8266_mux_send(char link_id, char *txbuf, int len);

#endif /*  _ESP_8226_H */
