/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: esp8266.c
      ��������: 2016.09.25
      ��    �ߣ�like
      ����˵����wifiͨ��
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "stm32f1xx_hal.h"
#include "bsp_uart.h"


/* ���Կ��� */
#define CONFIG_ESP8266_DEBUG

/* ���Խӿں��� */
#ifdef CONFIG_ESP8266_DEBUG
    #define esp8266_print(fmt,args...) debug(fmt, ##args)
#else
    #define esp8266_print(fmt,args...)
#endif



/* esp8266ģ����ʱ�������� */
#define  esp8266_delay(x)  vTaskDelay(x);


/******************************************************************************
    ����˵������λ���ų�ʼ��
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int esp8266_reset_gpio_init(void)
{   
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* PB8 --> pow */
    __HAL_RCC_GPIOB_CLK_ENABLE();  
    
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* �ص�Դ */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); 
    
    /* PB9 --> RESET */
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pin = GPIO_PIN_9;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);   
    
    
    return 0;
}


/******************************************************************************
    ����˵����Ӳ����λ
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void esp8266_reset(void)
{  
    esp8266_reset_gpio_init();
    
    /* �ص�Դ */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); 
    
    esp8266_delay(500);   
    /* ����Դ */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);    
    
    esp8266_delay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    esp8266_delay(2000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int esp8266_send_byte(char *buff, int len)
{
    int ret;
    
    /* �������ݵ�����*/
    ret = bsp_uart_send(UART_3, buff, len);
    if (ret < 0)
    {
        return -1;
    }   
    
    return 0;
}
  


/******************************************************************************
    ����˵��������AT����
    ���������buff AT�����
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_at_cmd_send(char *buff)
{
    int len;
    int ret;
    
    if (buff == NULL)
    {
        return -1;
    }
    
    /* AT��������ܴ���128 */
    len = strlen(buff);
    if (len > 128)
    {
        return -1;
    }
    
    /* ÿһ�η�������ͶԴ��ڽ��г�ʼ������ֹ���ڷ����� */
    uart_init(UART_3, 115200);    
    uart_clear(UART_3);
    
    /* �����ݴӴ��ڷ���ȥ */
    ret = esp8266_send_byte(buff, len);
    if (ret < 0)
    {
        return -1;
    }
    
    esp8266_print("[esp8266] tx cmd: ");
    esp8266_print(buff);
    return 0;
}



/******************************************************************************
    ����˵����esp8266ģ��Ӵ��ڽ�������
    ���������buff �������ݻ���  size �������յĳ���
    �����������
    �� �� ֵ��ʵ�ʽ��յ������ݳ���
*******************************************************************************/ 
int esp8266_receive_byte(char *buff, int size)
{
    int len;
    
    /* �Ӵ��ڽ������� */
    len = bsp_uart_receive(UART_3, buff, size);
                 
    return len;
}



/******************************************************************************
    ����˵����esp8266ģ�����AT���� 
    ���������buff �������ݻ���  size �������յĳ��� ovt ��ʱ�ȴ�ʱ��
    �����������
    �� �� ֵ��AT����ĳ���
*******************************************************************************/ 
int esp8266_at_cmd_receive(char *buff, int size, int ovt)
{
    int len = 0;
    int rxlen = 0;
    int time = ovt;
    
    while (--time > 0)
    {   
        /* �Ӵ��ڽ������� */
        len = esp8266_receive_byte(buff + rxlen, size - rxlen);
        if (len > 0)
        {
            rxlen += len;
            time = ovt;
        }
               
        esp8266_delay(1);
    }
    
    if (rxlen > 0)
    {
        esp8266_print("[esp8266] rx cmd: ");
        esp8266_print(buff);
        
        /* ÿһ�η�������ͶԴ��ڽ��г�ʼ������ֹ���ڷ����� */
        uart_init(UART_3, 115200);    
        uart_clear(UART_3);       
    }
   
    return rxlen;
}



/******************************************************************************
    ����˵����ִ��AT������ػ���
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_ate(void)
{
    int ret;
    char rxbuf[16] = {0};
    char len;
    
    /* ����AT���� */
    ret = esp8266_at_cmd_send("ATE0\r\n");
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(rxbuf, 16, 300);
    if (len > 0)
    {
        if (strstr(rxbuf, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵����ִ��AT�����ѯ�汾��Ϣ
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_gmr(void)
{
    int ret;
    char rxbuf[64] = {0};
    char len;
    
    /* ����AT���� */
    ret = esp8266_at_cmd_send("AT+GMR\r\n");
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(rxbuf, 16, 300);
    if (len > 0)
    {
        if (strstr(rxbuf, "AT version") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵����ִ��AT���������
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_rst(void)
{
    int ret;
    char rxbuf[16] = {0};
    char len;
    
    /* ����AT���� */
    ret = esp8266_at_cmd_send("AT+RST\r\n");
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(rxbuf, 16, 3000);
    if (len > 0)
    {
        if (strstr(rxbuf, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}




/******************************************************************************
    ����˵����ִ��AT������в����ָ�����
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_restore(void)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    ret = esp8266_at_cmd_send("AT+RESTORE\r\n");
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵����ִ��AT�������esp8266ģʽ
    ���������mode 1 station ģʽ
                   2 softAP ģʽ
                   3 softAP + station ģʽ
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cwmode(char mode)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CWMODE_CUR=%d\r\n", mode);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵����ִ��AT�������esp8266 DHCPģʽ
    ��������� 
               <mode> 
                0 : ����ESP8266 softAP 
                1 : ����ESP8266 station 
                2 : ����ESP8266 softAP ��station 
                <en> 
                0 : �ر�DHCP 
                1 : ����DHCP
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cwdhcp(char mode, char en)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CWDHCP_CUR=%d,%d\r\n", mode, en);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}




/******************************************************************************
    ����˵����ִ��AT�����STAģʽ�£�����AP�ȵ�
    ���������ssid  wifi�ȵ���û���
              psw   wifi�ȵ������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cwjap(char *ssid, char *psw)
{
    int ret;
    char buff[64] = {0};
    char len = 0, rxlen = 0;
    char ovt = 15;
    
    /* ����AT���� */
    sprintf(buff, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, psw);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
 
    memset(buff, 0, 64);
    
    /* ����AT����� */
    while (ovt--)
    {
        len = esp8266_at_cmd_receive(buff + rxlen, 64 - rxlen, 500);
        if (len > 0)
        {
            rxlen += len;
            
            /* �ȵ����ӳɹ� */
            if ((strstr(buff, "CONNECTED") != NULL)                        
                && (strstr(buff, "GOT IP") != NULL)
                && (strstr(buff, "OK") != NULL)) 
            {
                return 0;
            }
            
            else if (strstr(buff, "ERROR\r\n") != NULL) 
            {
                return -1;
            }           
        }
        
        esp8266_delay(1000);       
    }
    
    return -1;
}



/******************************************************************************
    ����˵����ִ��AT�������ESP8266 softAP �����ò���
    ���������ssid -APģʽ���Լ�������wifi�ȵ���û���
              psw  -APģʽ���Լ�������wifi�ȵ������
              chl  -wifiƵ��
              ecn -���ܷ�ʽ ��֧��WEP
                  -0 OPEN
                  -2 WPA_PSK 
                  -3 WPA2_PSK 
                  -4 WPA_WPA2_PSK
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cwsap(char *ssid, char *psw, char chl, char ecn)
{
    int ret;
    char buff[64] = {0};
    char len = 0, rxlen = 0;
   
    /* ����AT���� */
    sprintf(buff, "AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d\r\n", ssid, psw, chl, ecn);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
 
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff + rxlen, 64 - rxlen, 5000);
    if (len > 0)
    {
        rxlen += len;
        
        if (strstr(buff, "OK") != NULL)
        {
            return 0;
        }
        
        else if (strstr(buff, "ERROR\r\n") != NULL) 
        {
            return -1;
        }           
    }
                 
    return -1;
}




/******************************************************************************
    ����˵�������ô���ģʽ
              0 ��ͨ����ģʽ 
              1 ͸��ģʽ����֧��TCP �����Ӻ�UDP �̶�ͨ�ŶԶ˵����
    ���������mode ����ģʽ
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipmode(char mode)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPMODE=%d\r\n", mode);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}


/******************************************************************************
    ����˵�������ö�����
    ���������mode 0 ������ģʽ
                   1 ������ģʽ
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipmux(char mode)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPMUX=%d\r\n", mode);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵��������TCP server 
    ���������mode
                0 �ر�server
                1 ����server
              port �˿�
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipserver(char mode, unsigned short port)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPSERVER=%d,%d\r\n", mode, port);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}




/******************************************************************************
    ����˵��������TCP server ��ʱʱ��
    ���������time ��ʱʱ��
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipsto(unsigned short time)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPSTO=%d\r\n", time);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}




/******************************************************************************
    ����˵�����ر�TCP / UDP ����
    ���������link_id ��·��
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipclose(char link_id)
{
    int ret;
    char buff[16] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPCLOSE=%d\r\n", link_id);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK\r\n") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵��������TCP���ӻ���UDP����
    ���������type ��������
              ip ������IP
              port �������˿�
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipstart(char *type, char *ip, unsigned short port)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    if ((strcmp(type, "TCP") != 0) 
     && (strcmp(type, "UDP") !=0))
    {   
        return -1;
    }
       
    sprintf(buff, "AT+CIPSTART=\"%s\",\"%d.%d.%d.%d\",%d\r\n", 
            type, 
            ip[0], ip[1], ip[2], ip[3], 
            port);
    
    memset(buff, 0, 64);
    
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 64, 500);
    if (len > 0)
    {
        if ((strstr(buff, "CONNECT") != NULL)
          && (strstr(buff, "OK") != NULL)) 
        {
            return 0;
        }   
    }
    
    return -1;
}


/******************************************************************************
    ����˵������������, ����͸��ģʽ
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipsend(void)
{
    int ret;
    char buff[64] = {0};
    char len;
    
    /* ����AT���� */
    ret = esp8266_at_cmd_send("AT+CIPSEND\r\n");
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 16, 500);
    if (len > 0)
    {
        if (strstr(buff, ">") != NULL)
        {
            return 0;
        }
    }
    
    return -1;
}


/******************************************************************************
    ����˵�����ڶ���·�£����������ģʽ��������
    ���������link_id ��·ID, txbuf ���ͻ��� len ���ͳ���
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/
static int at_exe_cmd_cip_mux_send(char link_id, char *txbuf, int txlen)
{
    int ret;
    char buff[64] = {0};
    char len;
  
    /* ����AT���� */
    sprintf(buff, "AT+CIPSEND=%d,%d\r\n", link_id, txlen);
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 64);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 64, 100);
    if (len > 0)
    {
        if (strstr(buff, ">") != NULL)
        {
            esp8266_send_byte(txbuf, txlen);
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    ����˵�����ڶ���·�£��ɽ�����������
    ���������link_id ��·ID, txbuf ���ͻ��� len ���ͳ���
    �����������
    �� �� ֵ�����ݳ���
*******************************************************************************/
static int at_exe_cmd_ipd(int *link_id, char *rxbuf, int size)
{
    int len;
    char *head = 0;
    
    /* ����AT����� */   
    len = esp8266_receive_byte(rxbuf, size);
    if (len > 8)
    {
        esp8266_print("[esp8266] rx cmd: %s\r\n", rxbuf);       
        if (strstr(rxbuf, "CLOSED") != 0)
        {
            *link_id = -1;
            return 0; /* ��·�Ͽ� */
        }     
        
        if (strstr(rxbuf, "CONNECT") != NULL)
        {
            *link_id = 0;
            return 0; /* ��·���ӳɹ� */
        }       

        head = strstr(rxbuf, "+IPD,");
        if (head != 0)                   
        {   
            sscanf(head, "+IPD,%d,%d:", link_id, &len);
            esp8266_print("[esp8266]link_id,len: %d,%d\r\n", *link_id, len);           
            if (len > 0)
            {
                head = strstr(rxbuf, ":");
                if (head != 0)
                {
                    memcpy(rxbuf, head + 1, len);
                    return len;
                }
            }
        }
    }
      
    return 0;
}



/******************************************************************************
    ����˵����esp8266ģ���ʼ��
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int esp8266_init(void)
{
    int ret;
    
    /* Ӳ�����Ÿ�λ */
    esp8266_reset();
    
    /* ͨ�ſڳ�ʼ�� */
    uart_init(UART_3, 115200);
    
    /* ͨ�ſ�������� */
    uart_clear(UART_3);
    
    /* ģ������ָ����� */
    at_exe_cmd_restore();
    
    esp8266_delay(5000);
    
    /* �ػ��� */
    ret = at_exe_cmd_ate();
    if (ret < 0)
    {
        return -1;
    } 
    
    /* ��ѯ�汾��Ϣ */
    ret = at_exe_cmd_gmr();
    if (ret < 0)
    {
        return -1;
    }     
    
    return 0;
}



/******************************************************************************
    ����˵��������AP��wifi�ȵ�
    ���������ssid AP�û���
              psw AP����
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_join_ap(char *ssid, char *psw)
{
    int ret;
   
     /* station ģʽ  */
    ret = at_exe_cmd_cwmode(1);
    if (ret < 0)
    {
        return -1;
    }      
    
    /* �����û��������� */    
    ret = at_exe_cmd_cwjap(ssid, psw);
    if (ret < 0)
    {
        return -1;
    }
       
    return 0;
}


/******************************************************************************
    ����˵��������AP��wifi�ȵ�
    ���������ssid AP�û���
              psw AP����
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int ESP8226_set_ap(char *ssid, char *psw)
{
    int ret;
   
     /* AP ģʽ  */
    ret = at_exe_cmd_cwmode(2);
    if (ret < 0)
    {
        return -1;
    }  
    
    /* ����AP��wifi�ȵ� */
    ret = at_exe_cmd_cwsap(ssid, psw, 5, 3);
    if (ret < 0)
    {
        return -1;
    } 
    
    return 0;
}



/******************************************************************************
    ����˵����ִ��AT����: AT����AP���������
    ���������ip  �����IP��ַ
              gateway ����
              netmask ��������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_exe_cmd_cipap(char *ip, char *gateway, char *netmask)
{
    int ret;
    char buff[80] = {0};
    char len;
    
    /* ����AT���� */
    sprintf(buff, "AT+CIPAP_CUR=\"%d.%d.%d.%d\",\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n", 
            ip[0], ip[1], ip[2], ip[3], 
            gateway[0], gateway[1], gateway[2], gateway[3],
            netmask[0], netmask[1], netmask[2], netmask[3]);
    
    ret = esp8266_at_cmd_send(buff);
    if (ret < 0)
    {
        return -1;
    }
    
    memset(buff, 0, 80);
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 64, 500);
    if (len > 0)
    {
        if (strstr(buff, "OK") != NULL)
        {
            return 0;
        }   
    }
    
    return -1;
}



/******************************************************************************
    ����˵��������AP���������
    ���������ip  �����IP��ַ
              gateway ����
              netmask ��������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int ESP8226_set_ip(char *ip, char *gateway, char *netmask)
{
    int ret;
 
    /* ����APģʽ�¹ر�DHCP */
    ret = at_exe_cmd_cwdhcp(0,0);
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����AP��������� */
    ret = at_exe_cmd_cipap(ip, gateway, netmask);
    if (ret < 0)
    {
        return -1;
    }  
      
    return 0;
}



/******************************************************************************
    ����˵������������������
    ���������port �����˿�
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int ESP8226_server_config(unsigned short port)
{
    int ret;
   
    /* ����Ϊ������ģʽ */
    ret = at_exe_cmd_cipmux(1);
    if (ret < 0)
    {
        return -1;
    }   
    
    /* ����TCP server */    
    ret = at_exe_cmd_cipserver(1, port);
    if (ret < 0)
    {
        return -1;
    }

    /* ����TCP server ��ʱʱ��, ���뽨����TCP server�������� */
    ret = at_exe_cmd_cipsto(300);
    if (ret < 0)
    {
        return -1;
    } 
    
    return 0;
}


/******************************************************************************
    ����˵������ѯ�Ƿ��пͻ�������
    �����������
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
static int at_query_cmd_server_connet(void)
{
    char buff[64] = {0};
    char len;
    
    /* ����AT����� */
    len = esp8266_at_cmd_receive(buff, 64, 500);
    if (len > 0)
    {
        if (strstr(buff, "0,CONNECT") != NULL)
        {
            return 0;
        }
        
        /* ÿһ�η�������ͶԴ��ڽ��г�ʼ������ֹ���ڷ����� */
        uart_init(UART_3, 115200);    
        uart_clear(UART_3);               
    }
    
    return -1;
}



/******************************************************************************
    ����˵����TCP������ģʽ����
    ���������time ����ʱ��
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int ESP8226_server_listening(char *link_id, unsigned long time)
{
    int ret;
    
    /* ��������ʱ */
    while (time--)
    {           
        /* ��ѯ�Ƿ��пͻ������� */
        ret = at_query_cmd_server_connet();
        if (ret == 0)
        {
            return 0;
        }
        
        esp8266_delay(500);
    }
    
    return -1;
}


/******************************************************************************
    ����˵����TCPģʽ���ӷ�����
    ���������type Э��
              server_ip ������IP
              server_port �������˿�
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_link_server(char *type, char *server_ip, unsigned short server_port)
{
    int ret;
   
    /* ����TCP���� */
    ret = at_exe_cmd_cipstart(type, server_ip, server_port);
    if (ret < 0)
    {
        return -1;
    }
    
    /* ����Ϊ͸��ģʽ */
    ret = at_exe_cmd_cipmode(1);
    if (ret < 0)
    {
        return -1;
    }
   
    /* �������ݴ���ģʽ */
    ret = at_exe_cmd_cipsend();
    if (ret < 0)
    {
        return -1;
    }
    
    return 0;
}


/******************************************************************************
    ����˵��������͸���ķ�ʽ������
    ���������rxbuf ���ջ��� size ������󳤶�
    �����������
    �� �� ֵ���������ݳ���
*******************************************************************************/ 
int esp8266_read(char *rxbuf, int size)
{
    int len;
    
    /* �Ӵ��ڶ�ȡ���� */
    len = esp8266_receive_byte(rxbuf, size);
    if (len > 0)
    {
        return len;
    }
    
    return 0;
}


/******************************************************************************
    ����˵��������͸���ķ�ʽд����
    ���������rxbuf ���ͻ��� size ���ͳ���
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_write(char *rxbuf, int len)
{
    int ret;
    
    ret = esp8266_send_byte(rxbuf, len);
    if (ret < 0)
    {
        return -1;
    }
    
    return 0;
}



/******************************************************************************
    ����˵������������ķ�ʽд����
    ���������rxbuf ���ͻ��� size ���ͳ���
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_mux_send(char link_id, char *txbuf, int len)
{
    int ret;
        
    ret = at_exe_cmd_cip_mux_send(link_id, txbuf, len);
    if (ret < 0)
    {
        return -1;
    }
    
    return 0;
}



/******************************************************************************
    ����˵������������ķ�ʽ��������
    ���������rxbuf ���ͻ��� size ���ͳ���
    �����������
    �� �� ֵ��0 �ɹ� -1 ʧ��
*******************************************************************************/ 
int esp8266_mux_receive(int *link_id, char *rxbuf, int size)
{
    int len;
    
    len = at_exe_cmd_ipd(link_id, rxbuf, size); 
        
    return len;
}

