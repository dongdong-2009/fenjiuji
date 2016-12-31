/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: task_wifi.h
      ��������: 2016.09.25
      ��    �ߣ�like
      ����˵����wifiͨ��ģ��
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#ifndef _TASK_WIFI_H
#define _TASK_WIFI_H


struct wifi_tx_message
{
    int len;
    char **txbuf;
};

struct wifi_rx_message
{
    int size;
    char **rxbuf;
};


extern void task_wifi(void *pvParameters);
extern int wifi_send_byte(char *txbuf, int len);
extern int wifi_receive_byte(char *rxbuf, int size);
extern int wifi_queue_creat(void);

#endif /*  _TASK_WIFI_H */

