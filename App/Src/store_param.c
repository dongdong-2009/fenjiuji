/******************************************************************************
    版权所有：依福斯电子
    版 本 号: 1.0
    文 件 名: store.c
    生成日期: 2016.10.12
    作    者：李科
    功能说明：数据存储
    其他说明：
    修改记录：
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "store.h"
#include "lib.h"

/* 存储器大小 8M */
#define STA_ADDR_STORE   0x000000L
#define END_ADDR_STORE   0x800000L



/* 参数保存地址 0x400 = 1K, 这里分配10K */
#define STA_ADDR_STORE_PARAM   0x100000L
#define END_ADDR_STORE_PARAM   0x103FFFL
#define PARAM_MAX_NUM          100
#define PARAM_MAX_SIZE         64


/* 参数信息 */
struct stroe_param_map
{
    char *name;
    unsigned long addr;
    char size;
};

#pragma pack(1)
struct stroe_param
{
    char len;
    unsigned short cs;
    char data[PARAM_MAX_SIZE];    
};
#pragma pack()


/* 参数存储映射表 */
const struct stroe_param_map param_map[PARAM_MAX_NUM] = 
{
    {"server_ip",    0x000002L,  16},
    {"server_port",  0x000012L,  16},
    {"client_port",  0x000022L,  16},
    {"terminal_ip",  0x000032L,  16},
    {"mac",          0x000042L,  16},
    {"submask",      0x000052L,  16},    
    {"gateway",      0x000062L,  16},
};



/******************************************************************************
    功能说明：
    输入参数：
    输出参数：
    返 回 值：
*******************************************************************************/
int store_param_save(char *name, char *data, char len)
{
    struct stroe_param param = {0}; 
    unsigned long addr;
    char size;
    int ret; 
    int i;
    
    if (len > PARAM_MAX_SIZE)
    {
        return -1;
    }
    
    for (i = 0; i < PARAM_MAX_NUM; i++)
    {
        if (strcmp(name, param_map[i].name) == 0)
        {
            size = param_map[i].size;
            addr = param_map[i].addr;           
            if (len > size)
            {
                return -1;
            } 
            
            param.len = len;
            memcpy(param.data, data, len);
            param.cs = usMBCRC16((unsigned char *)param.data, len);
            
            len += 3; 
            ret = storage_write(addr, (char *)&param, len);
            if (ret < 0)
            {
                return -1;
            }
            
            return 0;
        }
    }
    
    return -1;
}



/******************************************************************************
    功能说明：
    输入参数：
    输出参数：
    返 回 值：
*******************************************************************************/
int store_param_read(char *name, char *data)
{
    struct stroe_param param = {0}; 
    unsigned long addr;
    unsigned short cs;
    char size;
    int len = 0; 
    int i;
    
    for (i = 0; i < PARAM_MAX_NUM; i++)
    {
        if (strcmp(name, param_map[i].name) == 0)
        {
            size = param_map[i].size;
            addr = param_map[i].addr;
            
            len = storage_read(addr, (char *)&param, size);
            if ((len > 0) && (len <= size))
            {
                cs = usMBCRC16((unsigned char *)param.data, param.len); 
                if ((cs == param.cs) && (cs != 0))               
                {           
                    len = param.len;
                    memcpy(data, param.data,len); 
                    return len;
                }
            }
            
            break;
        }
    }
    
    return 0;
}


