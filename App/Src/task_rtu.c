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
#include "lib.h"


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




