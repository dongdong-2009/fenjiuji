/******************************************************************************
       文 件 名: rtu.c
       生成日期: 2016-12-19
       作    者：
       功能说明：
       其他说明：
       修改记录：
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "bsp_uart.h"


#define RTU_JIUTOU_NUM  2
#define RTU_TIAOYA_NUM  1

#define RTU_JIUTOU_REG_DAT_NUM 20
#define RTU_JIUTOU_REG_CTL_NUM 20
#define RTU_JIUTOU_REG_ARG_NUM 20

#define RTU_TIAOYA_REG_DAT_NUM 20
#define RTU_TIAOYA_REG_CTL_NUM 20
#define RTU_TIAOYA_REG_ARG_NUM 20



struct rtu_str
{
    unsigned short reg_jt_dat[RTU_JIUTOU_NUM][RTU_JIUTOU_REG_DAT_NUM];
    unsigned short reg_jt_ctl[RTU_JIUTOU_NUM][RTU_JIUTOU_REG_CTL_NUM];
    unsigned short reg_jt_arg[RTU_JIUTOU_NUM][RTU_JIUTOU_REG_ARG_NUM];

    unsigned short jt_ctl_cmd[RTU_JIUTOU_NUM][RTU_JIUTOU_REG_CTL_NUM];
    char jt_ctl_cmd_idx[RTU_JIUTOU_NUM];

    unsigned short jt_arg_cmd[RTU_JIUTOU_NUM][RTU_JIUTOU_REG_CTL_NUM];
    char jt_arg_cmd_idx[RTU_JIUTOU_NUM];


    unsigned short reg_ty_dat[RTU_TIAOYA_NUM][RTU_TIAOYA_REG_DAT_NUM];
    unsigned short reg_ty_ctl[RTU_TIAOYA_NUM][RTU_TIAOYA_REG_CTL_NUM];
    unsigned short reg_ty_arg[RTU_TIAOYA_NUM][RTU_TIAOYA_REG_ARG_NUM];

    unsigned short ty_ctl_cmd[RTU_TIAOYA_NUM][RTU_TIAOYA_REG_CTL_NUM];
    char ty_ctl_cmd_idx[RTU_TIAOYA_NUM];

    unsigned short ty_arg_cmd[RTU_TIAOYA_NUM][RTU_TIAOYA_REG_ARG_NUM];
    char ty_arg_cmd_idx[RTU_TIAOYA_NUM];
};


struct rtu_str rtu;


static const unsigned char aucCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const unsigned char aucCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};


/******************************************************************************
    功能说明：crc16
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
unsigned short usMBCRC16(unsigned char *pucFrame, unsigned short usLen)
{
    unsigned char   ucCRCHi = 0xFF;
    unsigned char   ucCRCLo = 0xFF;
    int             iIndex;

    while( usLen-- )
    {
        iIndex = ucCRCLo ^ *( pucFrame++ );
        ucCRCLo = ( unsigned char )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }
    return ( unsigned short )( ucCRCHi << 8 | ucCRCLo );
}



/******************************************************************************
    功能说明：帧链路解析
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_frame_unpack(char rtu_addr, char *rxbuf, char rxlen)
{
    unsigned short crc;

    if (rxbuf[0] != rtu_addr)
    {
        return -1;
    }

    crc = usMBCRC16((unsigned char *)rxbuf, rxlen - 2);
    if (crc == (rxbuf[rxlen - 1] * 0x100 + rxbuf[rxlen -2]))
    {
        return 0;
    }

    return -1;
}


/******************************************************************************
    功能说明：
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_frame_receive(char *rxbuf,  char size, int ovt)
{
    int len = 0;
    int rxlen = 0;
    int time = ovt / 50;

    while (--time > 0)
    {
        /* 从串口接收数据 */
        len = bsp_uart_receive(UART_2, rxbuf + rxlen, size - rxlen);
        if (len > 0)
        { 
            rxlen += len;
            time = ovt / 50;
        }

        vTaskDelay(50);
    }

    return rxlen;
}


/******************************************************************************
    功能说明：
    输入参数：
    输出参数：
    返 回 值：
*******************************************************************************/
static int modbus_frame_send(char *buff, char len)
{
    int ret;

    if (buff == NULL)
    {
        return -1;
    }

    /* 最长不能大于128 */
    if (len > 128)
    {
        return -1;
    }

    /* 每一次发送命令就对串口进行初始化，防止串口发出错 */
    uart_init(UART_2, 115200);

    /* 将数据从串口发出去 */
    ret = bsp_uart_send(UART_2, buff, len);
    if (ret < 0)
    {
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
static int modbus_frame_0x03_pack(char *txbuf, char rtu_addr, unsigned short reg_addr,
                                  char reg_num)
{
    char i;
    char index = 0;
    unsigned short crc;

    txbuf[index++] = rtu_addr;
    txbuf[index++] = 0x03;

    txbuf[index++] = (char)(reg_addr >> 8);
    txbuf[index++] = (char)(reg_addr);
  
    txbuf[index++] = 0;
    txbuf[index++] = reg_num;
        
    crc = usMBCRC16((unsigned char *)txbuf, index);
    txbuf[index++] = (char)crc;
    txbuf[index++] = (char)(crc >> 8);
    
    return index;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_frame_0x03_unpack(unsigned short *reg,  char rtu_addr,
                                char *rxbuf, char rxlen)
{
    char i, num = 0, len = 0;
    int ret;

    if (rxbuf[1] != 0x03)
    {
        return -1;
    }

    len = rxbuf[2];

    for (i = 0; i < len; i+=2)
    {
        reg[num++] = rxbuf[3 + i] * 0x100 + rxbuf[3 + i + 1];
    }

    return num;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_ctl_0x05_frame_pack(char *txbuf, char rtu_addr,
                                       unsigned short reg_addr,
                                       unsigned short reg_data)
{
    char index = 0;
    unsigned short crc;

    txbuf[index++] = rtu_addr;
    txbuf[index++] = 0x05;

    txbuf[index++] = (char)((reg_addr) >> 8);
    txbuf[index++] = (char)(reg_addr);

    txbuf[index++] = (char)((reg_data) >> 8);
    txbuf[index++] = (char)(reg_data);

    crc = usMBCRC16((unsigned char *)txbuf, index);
    txbuf[index++] = (char)(crc >> 8);
    txbuf[index++] = (char)crc;

    return index;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_arg_0x06_frame_pack(char *txbuf, char rtu_addr,
                                       unsigned short reg_addr,
                                       unsigned short reg_data)
{
    char index = 0;
    unsigned short crc;

    txbuf[index++] = rtu_addr;
    txbuf[index++] = 0x06;

    txbuf[index++] = (char)((reg_addr) >> 8);
    txbuf[index++] = (char)(reg_addr);

    txbuf[index++] = (char)((reg_data) >> 8);
    txbuf[index++] = (char)(reg_data);

    crc = usMBCRC16((unsigned char *)txbuf, index);
    txbuf[index++] = (char)(crc >> 8);
    txbuf[index++] = (char)crc;

    return index;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_ctl_0x05_frame_unpack(unsigned short reg_addr,
                                        unsigned short reg_data,
                                        char rtu_addr, char *rxbuf, char rxlen)
{
    int ret;

    ret = modbus_frame_unpack(rtu_addr, rxbuf, rxlen);
    if (ret == 0)
    {
        return -1;
    }

    if (rxbuf[1] != 0x05)
    {
        return -1;
    }

    /* 设置的命令应该与读到的一致 */
    if ((reg_addr == (rxbuf[2] * 0x100 + rxbuf[3]))
        && (reg_data == (rxbuf[4] * 0x100 + rxbuf[5])))
    {
        return 0;
    }

    return -1;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int modbus_arg_0x06_frame_unpack(unsigned short reg_addr,
                                        unsigned short reg_data,
                                        char rtu_addr, char *rxbuf, char rxlen)
{
    int ret;

    ret = modbus_frame_unpack(rtu_addr, rxbuf, rxlen);
    if (ret == 0)
    {
        return -1;
    }

    if (rxbuf[1] != 0x06)
    {
        return -1;
    }

    /* 设置的命令应该与读到的一致 */
    if ((reg_addr == (rxbuf[2] * 0x100 + rxbuf[3]))
        && (reg_data == (rxbuf[4] * 0x100 + rxbuf[5])))
    {
        return 0;
    }

    return -1;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int rtu_tiaoya_dat(char rtu_addr)
{
    char buff[64] = {0};
    char len;
    int ret;

    len = modbus_frame_0x03_pack(buff, rtu_addr, 0x00, 7);
    if (len > 0)
    {
        ret = modbus_frame_send(buff, len);
        if (ret < 0)
        {
            return -1;
        }
    }

    memset(buff, 0, 64);

    len = modbus_frame_receive(buff,  64, 150);
    if (len > 0)
    {
        ret = modbus_frame_unpack(rtu_addr, buff, len);
        if (ret < 0)
        {
            return -1;
        }

        ret = modbus_frame_0x03_unpack(rtu.reg_ty_dat[0], rtu_addr, buff, len);
        if (ret > 0)
        {
            return 0;
        }
    }

    return -1;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int rtu_jiutou_dat(char rtu_addr)
{
    char buff[64] = {0};
    char len;
    int ret;

    len = modbus_frame_0x03_pack(buff, rtu_addr, 0x00, 19);
    if (len > 0)
    {
        ret = modbus_frame_send(buff, len);
        if (ret < 0)
        {
            return -1;
        }
    }

    memset(buff, 0, 64);

    len = modbus_frame_receive(buff,  64, 150);
    if (len > 0)
    {
        ret = modbus_frame_unpack(rtu_addr, buff, len);
        if (ret < 0)
        {
            return -1;
        }

        ret = modbus_frame_0x03_unpack(rtu.reg_jt_dat[rtu_addr - 1], rtu_addr, buff, len);
        if (ret > 0)
        {
            return 0;
        }
    }

    return -1;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int rtu_jiutou_ctl(char rtu_addr)
{
    int i;
    char buff[64] = {0};
    char len;
    int ret;

    for (i = 0; i < rtu.jt_ctl_cmd_idx[rtu_addr - 1]; i++)
    {
        len = modbus_ctl_0x05_frame_pack(buff,
                                   rtu_addr,
                                   rtu.jt_ctl_cmd[rtu_addr - 1][i],
                                   rtu.reg_jt_ctl[rtu_addr - 1][i]);
        if (len > 0)
        {
            ret = modbus_frame_send(buff, len);
            if (ret < 0)
            {
                return -1;
            }
        }

        memset(buff, 0, 64);

        len = modbus_frame_receive(buff,  64, 150);
        if (len > 0)
        {
            ret = modbus_frame_unpack(rtu_addr, buff, len);
            if (ret < 0)
            {
                return -1;
            }

            ret = modbus_ctl_0x05_frame_unpack(rtu.jt_ctl_cmd[rtu_addr - 1][i],
                                               rtu.reg_jt_ctl[rtu_addr - 1][i],
                                               rtu_addr,
                                               buff,
                                               len);
            if (ret < 0)
            {
                return -1;
            }
        }
    }

    rtu.jt_ctl_cmd_idx[rtu_addr - 1] = 0;
    memset(rtu.jt_ctl_cmd[rtu_addr - 1], 0, RTU_JIUTOU_REG_CTL_NUM);
    return 0;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int rtu_jiutou_arg(char rtu_addr)
{
    int i;
    char buff[64] = {0};
    char len;
    int ret;

    for (i = 0; i < rtu.jt_arg_cmd_idx[rtu_addr - 1]; i++)
    {
        len = modbus_arg_0x06_frame_pack(buff,
                                   rtu_addr,
                                   rtu.jt_arg_cmd[rtu_addr - 1][i],
                                   rtu.reg_jt_arg[rtu_addr - 1][i]);
        if (len > 0)
        {
            ret = modbus_frame_send(buff, len);
            if (ret < 0)
            {
                return -1;
            }
        }

        memset(buff, 0, 64);

        len = modbus_frame_receive(buff,  64, 150);
        if (len > 0)
        {
            ret = modbus_frame_unpack(rtu_addr, buff, len);
            if (ret < 0)
            {
                return -1;
            }

            ret = modbus_arg_0x06_frame_unpack(rtu.jt_arg_cmd[rtu_addr - 1][i],
                                               rtu.reg_jt_arg[rtu_addr - 1][i],
                                               rtu_addr,
                                               buff,
                                               len);
            if (ret < 0)
            {
                return -1;
            }
        }
    }

    rtu.jt_arg_cmd_idx[rtu_addr - 1] = 0;
    memset(rtu.jt_arg_cmd[rtu_addr - 1], 0, RTU_JIUTOU_REG_ARG_NUM);
    return 0;
}





/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void rtu_init(void)
{   
    /* 通信口初始化 */
    uart_init(UART_2, 115200);
    memset((char *)&rtu, 0, sizeof(rtu));
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_rtu(void *pvParameters)
{   
    rtu_init();
    
    while (1)
    {
        rtu_tiaoya_dat(0x0A);
        //rtu_tiaoya_ctl(0x0A);
        //rtu_tiaoya_arg(0x0A);

        rtu_jiutou_dat(0x01);        
        rtu_jiutou_ctl(0x01);
        rtu_jiutou_arg(0x01);

        rtu_jiutou_dat(0x02);
        rtu_jiutou_ctl(0x02);
        rtu_jiutou_arg(0x02);
        
        vTaskDelay(500);
    }
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int rtu_jiutou_ctl_set(char rtu_addr, unsigned char jt_ctl_cmd,
                        unsigned short reg_jt_ctl)
{
    char index;

    index = rtu.jt_ctl_cmd_idx[rtu_addr - 1];

    if (index < RTU_JIUTOU_REG_CTL_NUM)
    {
        rtu.reg_jt_ctl[rtu_addr - 1][index++] = reg_jt_ctl;
        rtu.jt_ctl_cmd_idx[rtu_addr - 1] = index;
        return 0;
    }

    return -1;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int rtu_jiutou_arg_set(char rtu_addr, unsigned char jt_arg_cmd,
                        unsigned short reg_jt_arg)
{
    char index;

    index = rtu.jt_arg_cmd_idx[rtu_addr - 1];

    if (index < RTU_JIUTOU_REG_ARG_NUM)
    {
        rtu.reg_jt_arg[rtu_addr - 1][index++] = reg_jt_arg;
        rtu.jt_arg_cmd_idx[rtu_addr - 1] = index;
        return 0;
    }

    return -1;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int rtu_jiutou_dat_get(char rtu_addr, unsigned char jt_dat_index,
                       unsigned short *reg_jt_dat)
{
    if (jt_dat_index < RTU_JIUTOU_REG_DAT_NUM)
    {
        *reg_jt_dat = rtu.reg_jt_dat[rtu_addr - 1][jt_dat_index];
        return 0;
    }

    return -1;
}




