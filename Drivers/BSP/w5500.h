/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: w5500.h
      生成日期: 2016.10.05
      作    者：like
      功能说明：以太网通信
      其他说明： 
      修改记录：
*******************************************************************************/
#ifndef _W5500_H
#define _W5500_H


/* 通用寄存器地址 */
#define MR       0x0000
#define VERSIONR 0x0039


/* socket 寄存器地址 */
#define RX_RSR      0x0026  /* 2 byte */
#define RX_RD       0x0028  /* 2 byte */
#define RX_WR       0x002A  /* 2 byte */
#define RXBUF_SIZE  0x001E  /* 1 byte */
#define TXBUF_SIZE  0x001F  /* 1 byte */


extern void w5500_set_gateway(char *gateway);
extern void w5500_set_submask(char *submask);
extern void w5500_set_mac(char *mac);
extern void w5500_set_terminal_ip(char *ip);

extern int w5500_socket(int socket, char *protocol);
extern int w5500_connect(int sockfd, char *ip, char *port, char *client_port);
extern int w5500_write(int sockfd, char *buff, int len);
extern int w5500_socket_close(int sockfd);
extern int w5500_read(int sockfd, char *buff, int size);

extern int w5500_init(void);



#endif /*  _W5500_H */