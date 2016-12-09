/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: w5500.c
      ��������: 2016.10.05
      ��    �ߣ�like
      ����˵������̫��ͨ��
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "stm32f1xx_hal.h"
#include "w5500.h"

SPI_HandleTypeDef Spi1Handle;


#define  W5500_CS_LOW   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define  W5500_CS_HIGH  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define  w5500_delay(x) vTaskDelay(x);


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int spi1_init(void)
{      
    Spi1Handle.Instance               = SPI1;
    Spi1Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    Spi1Handle.Init.Direction         = SPI_DIRECTION_2LINES;
    Spi1Handle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    Spi1Handle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    Spi1Handle.Init.DataSize          = SPI_DATASIZE_8BIT;
    Spi1Handle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    Spi1Handle.Init.TIMode            = SPI_TIMODE_DISABLE;
    Spi1Handle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    Spi1Handle.Init.CRCPolynomial     = 7;
    Spi1Handle.Init.NSS               = SPI_NSS_SOFT;
    Spi1Handle.Init.Mode = SPI_MODE_MASTER;
  
    if (HAL_SPI_Init(&Spi1Handle) != HAL_OK)
    {
        return -1;
    }

    __HAL_SPI_ENABLE(&Spi1Handle);  
     
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_gpio_init(void)
{   
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* PC5 --> RESET */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
     __HAL_RCC_GPIOF_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
    
    
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
       
    /* PC4 --> INT */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure PC.4 pin as input floating */
    GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Enable and set EXTI line 0 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);    
   
    return 0;
}


/******************************************************************************
    ����˵����Ӳ����ʼ��
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_reset(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    w5500_delay(200);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
    w5500_delay(1000);
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_send_byte(char data)
{
    if (HAL_SPI_Transmit(&Spi1Handle, (uint8_t *)&data, 1, 100) != HAL_OK)
    {
        return -1;
    }
    
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_receive_byte(char *data)
{
    if (HAL_SPI_Receive(&Spi1Handle, (uint8_t *)data, 1, 100) != HAL_OK)
    {
        return -1;
    }
    
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void comm_register_read(char *data, unsigned short addr, int size)
{      
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));   
    w5500_send_byte((char)(addr));   
    w5500_send_byte(0x00); 
    for (i = 0; i < size; i++)
    {
        w5500_receive_byte(data + i);
    }
    
    W5500_CS_HIGH;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void comm_register_write(unsigned short addr, char *data, int size)
{       
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));
    w5500_send_byte((char)(addr));   
    w5500_send_byte(0x04|0x00);
    
    for (i = 0; i < size; i++)
    {
        w5500_send_byte(data[i]);
    }
    
    W5500_CS_HIGH;
    
    w5500_delay(100);
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void socket_register_read(char socket, char *data, unsigned short addr, int size)
{      
    const char BSB[8] = {0x01, 0x05, 0x09, 0x0D, 0x11, 0x19, 0x1D};
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));   
    w5500_send_byte((char)(addr));   
    w5500_send_byte((BSB[socket] << 3) | 0x00); 
    
    for (i = 0; i < size; i++)
    {
        w5500_receive_byte(data + i);
    }
    
    W5500_CS_HIGH;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int socket_register_write(char socket, unsigned short addr, char *data, int size)
{       
    const char BSB[8] = {0x01, 0x05, 0x09, 0x0D, 0x11, 0x19, 0x1D};
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));
    w5500_send_byte((char)(addr));   
    w5500_send_byte((BSB[socket] << 3) | 0x04 | 0x00);
    
    for (i = 0; i < size; i++)
    {
        w5500_send_byte(data[i]);
    }
    
    W5500_CS_HIGH;
      
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int socket_buff_write(int sockfd, unsigned short addr, char *buff, int size)
{
    char BSB[8] = {0x02, 0x05, 0x09, 0x0D, 0x11, 0x19, 0x1D};
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));
    w5500_send_byte((char)(addr));   
    w5500_send_byte((BSB[sockfd] << 3) | 0x04 | 0x00);
    
    for (i = 0; i < size; i++)
    {
        w5500_send_byte(buff[i]);
    }
    
    W5500_CS_HIGH;
    
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int socket_buff_read(int sockfd, unsigned short addr, char *buff, int size)
{
    char BSB[8] = {0x03, 0x05, 0x09, 0x0D, 0x11, 0x19, 0x1D};
    int i;
    
    W5500_CS_LOW;
    
    w5500_send_byte((char)(addr / 256));
    w5500_send_byte((char)(addr));   
    w5500_send_byte((BSB[sockfd] << 3) | 0x00 | 0x00);
    
    for (i = 0; i < size; i++)
    {
        w5500_receive_byte(buff + i);
    }
    
    W5500_CS_HIGH;
    
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_defaults(void)
{       
    char data;
      
    /* ��ȡ�汾 */
    comm_register_read(&data, VERSIONR, 1);
    if (data != 0x04)
    {
        return -1;
    }
    
    /* ��bit7��1�����ڳ�ʼ��ȫ���Ĵ�������ʼ�����bit7�ֻ��Զ����0 */
    data = 0x80;
    comm_register_write(0x0000, &data, 1);
     
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void socket_buff_size_config(void)
{
    char data;
    
    data = 2;
    socket_register_write(0,  RXBUF_SIZE, &data, 1);
    socket_register_write(0,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(1,  RXBUF_SIZE, &data, 1);
    socket_register_write(1,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(2,  RXBUF_SIZE, &data, 1);
    socket_register_write(2,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(3,  RXBUF_SIZE, &data, 1);
    socket_register_write(3,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(4,  RXBUF_SIZE, &data, 1);
    socket_register_write(4,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(5,  RXBUF_SIZE, &data, 1);
    socket_register_write(5,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(6,  RXBUF_SIZE, &data, 1);
    socket_register_write(6,  TXBUF_SIZE, &data, 1);
    
    data = 2;
    socket_register_write(7,  RXBUF_SIZE, &data, 1);
    socket_register_write(7,  TXBUF_SIZE, &data, 1);   
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_config(void)
{ 
    /* ��оƬ���мĴ����ָ��������� */
    w5500_defaults();
    
    /* ����socket�ķ��ͻ�������ջ��� */
    socket_buff_size_config();
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_set_gateway(char *gateway)
{ 
    /* �趨���� */
    comm_register_write(0x0001, gateway, 4);
}



/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_set_submask(char *submask)
{ 
    /* �趨�������� */
    comm_register_write(0x0005, submask, 4); 
}



/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_set_mac(char *mac)
{ 
    /* оƬ��λ�������MAC����0, ���Ա����趨һ��MAC */   
    comm_register_write(0x0009, mac, 6);
}

                        

/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
void w5500_set_terminal_ip(char *ip)
{ 
    /* �趨����IP */
    comm_register_write(0x000F, ip, 4);
}
                        
                        

/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_socket(int socket, char *protocol)
{
    char data;
    
    /* ��socket n����ΪTCPģʽ */
    data = 0x01;
    socket_register_write(socket, 0x0000, &data, 1);
  
    /* ��socket n , CRд�����Զ����㣬����������ڴ��� */
    data = 0x01;
    socket_register_write(socket, 0x0001, &data, 1);
  
    w5500_delay(200);
    
    /* ��ѯ socket�ǲ���INIT״̬ */
    socket_register_read(socket, &data, 0x0003, 1);   
    if (data == 0x13)
    {
        return socket;
    }
    
    return -1;
}



/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int w5500_connect(int sockfd, char *ip, char *port, char *client_port)
{
    char data;
       
    /* д�������IP��port���ٶ������ǿյ� */
    socket_register_write(sockfd, 0x000C, ip,   4);   
    socket_register_write(sockfd, 0x0010, port, 2);  
    socket_register_write(sockfd, 0x0004, client_port, 2);   
    
    /* connect , CRд�����Զ����㣬����������ڴ��� */
    data = 0x04;
    socket_register_write(sockfd, 0x0001, &data, 1);    
    
    w5500_delay(500);
    
    /* ��ѯ socket�ǲ���connect״̬ */
    socket_register_read(sockfd, &data, 0x0003, 1);   
    if (data == 0x17)
    {
        return 0;
    }    
    
    return -1;
}



/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int w5500_read(int sockfd, char *buff, int size)
{
    char data[2];
    unsigned short len, addr, rlen;
 
    /* ��ѯ socket�ǲ���connect״̬ */
    socket_register_read(sockfd, data, 0x0003, 1);   
    if (data[0] != 0x17)
    {
        return -1;
    }       
        
    socket_register_read(sockfd, data, RX_RSR, 2);
    len = data[0] * 0x100 + data[1];
    if (len > 0)
    {
        if (len >= size)
        {
            len = size;
        }
        
        socket_register_read(sockfd, data, RX_RD, 2);
        addr = data[0] * 0x100 + data[1];   
        if (addr + len <= 0xFFFF)
        {
            socket_buff_read(sockfd, addr, buff, len);
            addr += len;
        }
        else
        {
            rlen = 0xFFFF - addr;
            socket_buff_read(sockfd, addr, buff, rlen); 
            socket_buff_read(sockfd, 0, buff + rlen, len - rlen);
            addr = len - rlen;
        }
        
        data[0] = (char)(addr >> 8);
        data[1] = (char)addr;
        socket_register_write(sockfd, RX_RD, data, 2);
    
        /* recv, CRд�����Զ����㣬����������ڴ��� */
        data[0] = 0x40; 
        socket_register_write(sockfd, 0x0001, data, 1);  
    }
       
    return len;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/
int w5500_write(int sockfd, char *buff, int len)
{
    char data[2];
    unsigned short TX_RD, wlen;
 
    /* ��ѯ socket�ǲ���connect״̬ */
    socket_register_read(sockfd, data, 0x0003, 1);   
    if (data[0] != 0x17)
    {
        return -1;
    }     
    
    /* ��ȡ���ͻ����н�Ҫ���洫�����ݵ��׵�ַ */
    socket_register_read(sockfd, data, 0x0022, 2);
    TX_RD = data[0] * 0x100 + data[1];  
    if (TX_RD + len <= 0xFFFF) 
    {
        socket_buff_write(sockfd, TX_RD, buff, len);
        TX_RD = TX_RD + len;
    }
    else
    {
        wlen = 0xFFFF - TX_RD;
        socket_buff_write(sockfd, TX_RD, buff, wlen);
        socket_buff_write(sockfd, 0, buff + wlen, len - wlen);
        TX_RD = len - wlen;
    }
        
    data[0] = (char)(TX_RD >> 8);
    data[1] = (char)TX_RD;
    socket_register_write(sockfd, 0x0024, data, 2);

    /* send , CRд�����Զ����㣬����������ڴ��� */
    data[0] = 0x20; 
    socket_register_write(sockfd, 0x0001, data, 1);   
    
    return 0;
}


/******************************************************************************
    ����˵������
    �����������
    �����������
    �� �� ֵ����
*******************************************************************************/ 
int w5500_init(void)
{       
    w5500_gpio_init();
    
    spi1_init();
    
    w5500_reset();
    
    w5500_config();
    
    return 0;
}












