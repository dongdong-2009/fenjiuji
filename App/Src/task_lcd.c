/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_lcd.c
      生成日期: 2016.09.26
      作    者：like
      功能说明：LCD任务模块
      其他说明：
      修改记录：
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "bsp_uart.h"
#include "string.h"
#include "stdio.h"
#include "rtc.h"
#include "task_lcd.h"
#include "divid_cup.h"

#define LCD_RX_BUFF_SIZE 128
#define LCD_TX_BUFF_SIZE 64
#define FRAME_DATA_SIZE 16
#define LCD_BOUND  9600L

#define CMD_BUTTON  0x65
#define CMD_SENDME  0x66
#define CMD_STR     0x70
#define CMD_NUM     0x71
#define CMD_ERR     0x1A


typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif



enum page_enum
{
    page_startup = 0,
    page_code1,
    page_home,
    page_information,
    page_overpressure,
    page_lackpressure,
    page_pour,
    page_code2,
    page_light,
    page_code3,
    page_lock,
    page_code4,
    page_sleep_note,
    page_code5,
    page_change2,
    page_change4,
    page_change_menu,
    page_infor_menu,
    page_product,
    page_price1,
    page_price_s,
    page_price_m,
    page_price_l,
    page_wash1,
    page_wash2,
    page_wash3,
    page_capacity,
    page_wash4,
    page_code6,
    page_setting_menu,
    page_operation_menu,
    page_temperature1,
    page_temperature4,
    page_volume_set2,
    page_volume_set4,
    page_volume_setting,
    page_adjust1,
    page_adjust_choose4,
    page_adjust2,
    page_adjust3,
    page_adjust4,
    page_adjust_pour,
    page_adjust5,
    page_versions,
};

struct lcd_str
{
    char rxbuf[LCD_RX_BUFF_SIZE];
    char txbuf[LCD_TX_BUFF_SIZE];
    int rxlen;
    int txlen;
    char page_last;
    char page_present;
    char clear_time;
};

struct Frame_str
{
    unsigned char valid;                //当前指令是否有效
    unsigned char cmd;              //功能码
    unsigned char datalen;
    char databuf[FRAME_DATA_SIZE];      //
};

static struct lcd_str lcd;
static struct Frame_str frame;

int g_wash_num = 0;                 //洗瓶编号
int g_bottling_num = 0;             //装瓶编号
int c_bAuthorityAlarm = 0;          //权限警告

//模拟数据
const char *password1 ="111111";    //开机密码
const char *password2 = "222222";   //操作密码
const char *password3 = "333333";   //锁定与休眠密码
int temperature1 = 15;                                //温度
const char *str_product1 = "111111";
const char *str_priceS1 = "25";
const char *str_priceM1 = "75";
const char *str_priceL1 = "150";
const char *str_restwine1 = "800";
const char *str_product2 = "222222";
const char *str_priceS2 = "26";
const char *str_priceM2 = "76";
const char *str_priceL2 = "156";
const char *str_restwine2 = "900";
const char *str_product3 = "333333";
const char *str_priceS3 = "27";
const char *str_priceM3 = "77";
const char *str_priceL3 = "157";
const char *str_restwine3 = "1000";
const char *str_product4 = "444444";
const char *str_priceS4 = "28";
const char *str_priceM4 = "78";
const char *str_priceL4 = "158";
const char *str_restwine4 = "1100";
char luminance = 60;                                    //灯光亮度
char color = 3;                                         //灯光颜色
char change_bottle;                                     //更新的瓶位

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void at_cmd(char *pCmd)
{
    strcpy(lcd.txbuf, pCmd);
    lcd.txlen = strlen(lcd.txbuf);
    lcd.txbuf[lcd.txlen++] = 0xFF;
    lcd.txbuf[lcd.txlen++] = 0xFF;
    lcd.txbuf[lcd.txlen++] = 0xFF;
    lcd.txbuf[lcd.txlen++] = 0;
    lcd_send(lcd.txbuf);
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int lcd_author_judge(unsigned char *author)
{
  if((page_startup == lcd.page_present)||
     (page_code1 == lcd.page_present)||
     (page_lock == lcd.page_present)||
     (page_code4 == lcd.page_present)||
       (page_sleep_note == lcd.page_present))
  {
    *author = 0;
  }
  else
  {
    *author = 1;
  }
  return 0;
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int set_author_limit(void)
{
    c_bAuthorityAlarm = 1;
    return 0;
}
/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int check_author_limit(void)
{
    if(c_bAuthorityAlarm)
    {
      c_bAuthorityAlarm = 0;
      return 1;
    }
    return 0;
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int jump_pour_page(void)
{
    if((page_startup == lcd.page_present)||
        (page_code1 == lcd.page_present)||
        (page_lock == lcd.page_present)||
        (page_code4 == lcd.page_present)||
        (page_sleep_note == lcd.page_present))
    {
        return 0;
    }
    else
    {
        at_cmd("page pour");
        return 0;
    }
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int jump_page_lackpressure(void)
{
    if((page_startup == lcd.page_present)||
        (page_code1 == lcd.page_present)||
        (page_lock == lcd.page_present)||
        (page_code4 == lcd.page_present)||
        (page_sleep_note == lcd.page_present))
    {
        return 0;
    }
    else
    {
        at_cmd("page lackpressure");
        return 0;
    }
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int jump_page_overpressure(void)
{
    if((page_startup == lcd.page_present)||
        (page_code1 == lcd.page_present)||
        (page_lock == lcd.page_present)||
        (page_code4 == lcd.page_present)||
        (page_sleep_note == lcd.page_present))
    {
        return 0;
    }
    else
    {
        at_cmd("page overpressure");
        return 0;
    }
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void lcd_init(void)
{     
    /* 通信口初始化 */
    uart_init(UART_4, LCD_BOUND);
    
    at_cmd("rest");
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_read(char *rxbuf, int size)
{
    int len;
    int time = 2;
    int rxlen = 0;

    while (time > 0)
    {
        /* 从串口接收数据 */
        len = bsp_uart_receive(UART_4, rxbuf + rxlen, size - rxlen);
        if (len > 0)
        {
            rxlen += len;
            time = 2;
        }

        time--;

        vTaskDelay(50);
    }
    return rxlen;
}


/******************************************************************************
    功能说明：发送AT命令
    输入参数：buff AT命令缓存
    输出参数：无
    返 回 值：0 成功 -1 失败
*******************************************************************************/
int lcd_send(char *buff)
{
    int len;
    int ret;

    if (buff == NULL)
    {
        return -1;
    }

    /* AT命令最长不能大于128 */
    len = strlen(buff);
    if (len > 128)
    {
        return -1;
    }

    /* 每一次发送命令就对串口进行初始化，防止串口发出错 */
    uart_init(UART_4, LCD_BOUND);

    /* 将数据从串口发出去 */
    /* 发送数据到串口*/
    ret = bsp_uart_send(UART_4, buff, len);
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
static void frame_unpack(void)
{
    int i, j, datalen;
    char buf[32];

    for(i=0; i<lcd.rxlen; i++)
    {//先找结束符
        if((lcd.rxbuf[i]==0xff)&&(lcd.rxbuf[i+1]==0xff)&&(lcd.rxbuf[i+2]==0xff))
        {
            memcpy(buf, lcd.rxbuf, i+3);
            datalen = i;

            for(j=0; j<datalen; j++)
            {//查询所有功能码
                if(((buf[j]>=0x65)&&(buf[j]<=0x71))
                    || ((buf[j]>=0x86)&&(buf[j]<=0x89))
                    || (buf[j]==0xFD)  || (buf[j]==0xFE))
                {
                    frame.cmd = buf[j];
                    frame.datalen = datalen-j-1;

                    if((frame.cmd == CMD_NUM)&&(frame.datalen != 4))
                    {//lcd发送数值必为4位
                        break;
                    }

                    frame.valid = TRUE;
                    memcpy(frame.databuf, buf+j+1, frame.datalen);
                    break;
                }
            }
            //去掉解完的数据
            memcpy(lcd.rxbuf, lcd.rxbuf+i+3, LCD_RX_BUFF_SIZE-i-3);
            lcd.rxlen = lcd.rxlen-i-3;

            return;
        }
    }
    //去掉接收到的数据
    if (lcd.rxlen > 0)
    {
        if (lcd.clear_time++ > 50)
        {
            memset(lcd.rxbuf, 0, lcd.rxlen);
            lcd.rxlen = 0;
            lcd.clear_time = 0;
        }
    }
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void frame_str_reset(void)
{
    memset((char*)&frame, 0, sizeof(frame));
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void lcd_receive(void)
{
    int len;


    len = lcd_read(lcd.rxbuf + lcd.rxlen, LCD_RX_BUFF_SIZE - lcd.rxlen);
    if (len > 0)
    {
        lcd.rxlen += len;
    }
    while((lcd.rxlen>=4)&&(frame.valid == FALSE))
    {
        frame_unpack();
       // lcd_init();
    }
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void check_page(void)
{
    at_cmd("sendme");
}

/******************************************************************************
    功能说明：上电初始化界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_startup(void)
{
    struct tm time;
    char temp[5] = {0};
    char buf[32] = {0};

    if(lcd.page_last != lcd.page_present)
    {
	    rtc_read(&time);
	    
	    sprintf(temp, "%04d", time.tm_year);
	    sprintf(buf, "rtc0=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
	       
	    sprintf(temp, "%02d", time.tm_mon);
	    sprintf(buf, "rtc1=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
	    
	    sprintf(temp, "%02d", time.tm_mday);
	    sprintf(buf, "rtc2=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
	    
	    sprintf(temp, "%02d", time.tm_hour);
	    sprintf(buf, "rtc3=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
	    
	    sprintf(temp, "%02d", time.tm_min);
	    sprintf(buf, "rtc4=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
	    
	    sprintf(temp, "%02d", time.tm_sec);
	    sprintf(buf, "rtc5=%s", temp);
	    at_cmd(buf);
	    vTaskDelay(1);
    }
    
}


/******************************************************************************
    功能说明：开机密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code1(void)
{
    char password[7] = {0};
    int len;
    int ret;
	
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        
	len = store_param_read("opt_psw", password);
	if (len == 0) 
	{
		memcpy(password, password1, 6);
		ret = store_param_save("opt_psw", password, 6);
		if (ret < 0)
			return;
	}
		    
	if(strcmp(frame.databuf, password1) == 0)
        {
            at_cmd("code1return.val=1");
        }
        else
        {
            at_cmd("code1return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：主界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_home(void)
{
    static int last_tempe = 0xff;
    char str_tempe[4];
    char buf[32];

    if((last_tempe != temperature1) || (lcd.page_last != lcd.page_present))
    {//温度变化和返回此界面刷新时
        last_tempe = temperature1;
        sprintf(str_tempe, "%d", last_tempe);
        sprintf(buf, "tempe.txt=\"%s\"", str_tempe);
        at_cmd(buf);
    }

//    if(0)
//    {//如果任意出酒键按下
//        at_cmd("page pour");
//    }
}

/******************************************************************************
    功能说明：红酒信息界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_information(void)
{
    char *str_bottle1 = "bottle1";
    char *str_bottle2 = "bottle2";
    char *str_bottle3 = "bottle3";
    char *str_bottle4 = "bottle4";
    char buf[32];

    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle1) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine1);
            at_cmd(buf);
            memset(buf, 0, 32);
        }
        else if(strcmp(frame.databuf, str_bottle2) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine2);
            at_cmd(buf);
            memset(buf, 0, 32);
        }else if(strcmp(frame.databuf, str_bottle3) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine3);
            at_cmd(buf);
            memset(buf, 0, 32);
        }else if(strcmp(frame.databuf, str_bottle4) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine4);
            at_cmd(buf);
            memset(buf, 0, 32);
        }
    }
}
/******************************************************************************
    功能说明：超压界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_overpressure(void)
{
    ;
}

/******************************************************************************
    功能说明：欠压界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_lackpressure(void)
{
    ;
}


/******************************************************************************
    功能说明：出酒界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_pour(void)
{
    char *str_bottle1 = "bottle1";
    char *str_bottle2 = "bottle2";
    char *str_bottle3 = "bottle3";
    char *str_bottle4 = "bottle4";
    char buf[32];
    static int ms_count = 0; 

    //根据按下的出酒按键指示的瓶号,发送对应瓶号的点击指令
    if (lcd.page_last != lcd.page_present)
    {
      	//酒位
        switch(divid_cup_info.place)
        {
	    case 1:
	    	at_cmd("click bt0,0");
	    	break;
	    case 2:
	    	at_cmd("click bt1,0");
	    	break;
	    case 3:
	    	at_cmd("click bt2,0");
	    	break;
	    case 4:
	    	at_cmd("click bt3,0");
	    	break;
	    default:
	      	break;
	}
	// 小中大杯
	switch(divid_cup_info.bebe)
        {
	    case 1:
	    	at_cmd("capacity.val=1");
	    	break;
	    case 2:
	    	at_cmd("capacity.val=2");
	    	break;
	    case 3:
	    	at_cmd("capacity.val=3");
	    	break;
	    default:
	      	break;
	}
	//货号
	sprintf(buf, "product.txt=\"%s\"", divid_cup_info.huohao);
	at_cmd(buf);
	memset(buf, 0, 32);
	//vTaskDelay(1);
	//小杯价格
	sprintf(buf, "priceS.txt=\"%s\"", divid_cup_info.jiage_1);
	at_cmd(buf);
	memset(buf, 0, 32);
	//vTaskDelay(1);
	//中杯价格
	sprintf(buf, "priceM.txt=\"%s\"", divid_cup_info.jiage_2);
	at_cmd(buf);
	memset(buf, 0, 32);
	//vTaskDelay(1);
	//大杯价格
	sprintf(buf, "priceL.txt=\"%s\"", divid_cup_info.jiage_3);
	at_cmd(buf);
	memset(buf, 0, 32);
	//vTaskDelay(1);
    }
	
    // 余酒量500ms刷新
    if(0 == ms_count)
    {
	sprintf(buf, "restwine.txt=\"%s\"", divid_cup_info.bebe_ml);
	at_cmd(buf);
    }
    ms_count++;
    if(5 == ms_count)
    {
      ms_count = 0;
    }
    
    //判断酒量是否不足
    if(divid_cup_info.yujiubuzu_flag)
    {//酒量不足
        at_cmd("empty.val=1");
        //清除酒量不足标志
        //---------
	divid_cup_info.yujiubuzu_flag = 0;
        //---------
    }

    //判断出酒结束
    if(0)
    {
        at_cmd("end.val=1");
    }

}


/******************************************************************************
    功能说明：灯光设置密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code2(void)
{
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, password2) == 0)
        {
            at_cmd("code2return.val=1");
        }
        else
        {
            at_cmd("code2return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：灯光设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_light(void)
{
    const char *str_luminance = "luminance";
    const char *str_color = "color";

    static int last_luminance = 0xff;
    static int last_color = 0xff;
    static int last_status = 0;
    unsigned long *pTemp;


    if((last_luminance != luminance) || (lcd.page_last != lcd.page_present))
    {
        switch(luminance)
        {
            case 0:
                last_luminance = luminance;
                at_cmd("click bt0,0");
                break;
            case 20:
                last_luminance = luminance;
                at_cmd("click bt1,0");
                break;
            case 40:
                last_luminance = luminance;
                at_cmd("click bt2,0");
                break;
            case 60:
                last_luminance = luminance;
                at_cmd("click bt3,0");
                break;
            case 80:
                last_luminance = luminance;
                at_cmd("click bt4,0");
                break;
            case 100:
                last_luminance = luminance;
                at_cmd("click bt5,0");
                break;
            default:
                break;
        }
    }

    if((last_color != color) || (lcd.page_last != lcd.page_present))
    {
        switch(color)
        {
            case 1:
                last_color = color;
                at_cmd("click bt6,0");
                break;
            case 2:
                last_color = color;
                at_cmd("click bt7,0");
                break;
            case 3:
                last_color = color;
                at_cmd("click bt8,0");
                break;
            case 4:
                last_color = color;
                at_cmd("click bt9,0");
                break;
            case 5:
                last_color = color;
                at_cmd("click bt10,0");
                break;
            case 6:
                last_color = color;
                at_cmd("click bt11,0");
                break;
            case 7:
                last_color = color;
                at_cmd("click bt12,0");
                break;
            case 8:
                last_color = color;
                at_cmd("click bt13,0");
                break;
            case 9:
                last_color = color;
                at_cmd("click bt14,0");
                break;
            case 10:
                last_color = color;
                at_cmd("click bt15,0");
                break;
            default:
                break;
        }
        lcd.page_last = lcd.page_present;
    }

    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_luminance) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if(*pTemp <= 100)
        {
            luminance = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_color) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 3))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 10))
        {
            color = (char)*pTemp;

            //存储亮度和颜色数据
            //--------
            if(1)
            {
                at_cmd("saveOK.val=1");
            }
        }
    }
}


/******************************************************************************
    功能说明：休眠与锁定密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code3(void)
{
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, password3) == 0)
        {
            at_cmd("code3return.val=1");
        }
        else
        {
            at_cmd("code3return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：锁屏界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_lock(void)
{
    static int last_tempe = 0xff;
    char str_tempe[4];
    char buf[32];

    if((last_tempe != temperature1) || (lcd.page_last != lcd.page_present))
    {//温度变化和返回此界面刷新时
        last_tempe = temperature1;
        sprintf(str_tempe, "%d", last_tempe);
        sprintf(buf, "tempe.txt=\"%s\"", str_tempe);
        at_cmd(buf);
    }

    if(check_author_limit())
    {
        at_cmd("vis p0,1"); //显示锁屏提示
    }
}


/******************************************************************************
    功能说明：休眠与锁定解除密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code4(void)
{
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, password3) == 0)
        {
            at_cmd("code4return.val=1");
        }
        else
        {
            at_cmd("code4return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：休眠唤醒界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_sleep_note(void)
{
    if(check_author_limit())//如果按下出酒键
    {
        at_cmd("sleep=0");
    }
}



/******************************************************************************
    功能说明：换瓶密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code5(void)
{
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, password2) == 0)
        {
            at_cmd("code5return.val=1");
            vTaskDelay(1);

            //判断当前装置是两瓶型还是四瓶型
            if(1)
            {
                at_cmd("bottleCnt.val=2");
            }
            else
            {
                at_cmd("bottleCnt.val=4");
            }
        }
        else
        {
            at_cmd("code5return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：两瓶型操作瓶位选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_change2(void)
{
    unsigned long *pTemp;

    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 2))
        {
            change_bottle = (char)*pTemp;
        }
    }
}

/******************************************************************************
    功能说明：四瓶型操作瓶位选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_change4(void)
{
    unsigned long *pTemp;

    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            change_bottle = (char)*pTemp;
        }
    }
}

/******************************************************************************
    功能说明：更新酒品菜单界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_change_menu(void)
{
    ;
}

/******************************************************************************
    功能说明：输入信息菜单界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_infor_menu(void)
{
    ;
}

/******************************************************************************
    功能说明：输入货号界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_product(void)
{
    const char *str_bottle = "bottle";
    const char *str_product = "product";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pProduct;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if(*pTemp <= 100)
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_product) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pProduct = frame.databuf;

        //存储货号
        //--------

        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：输入价格主界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_price1(void)
{
    const char *str_bottle = "bottle";

    static int last_status = 0;
    unsigned long *pTemp;
    unsigned char bottle_num;
    char buf[32];


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            //查询此瓶号各杯型价格,并转换为字符串格式
            //------------

            //------------
            //发送价格
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL1);
            at_cmd(buf);
            memset(buf, 0, 32);
        }
    }

//  if(1)//按出酒键1下
//  {
//      at_cmd("page prices");
//  }else if(0)//按出酒键2下
//  {
//      at_cmd("page pricem");
//  }else if(0)//按出酒键3下
//  {
//      at_cmd("page pricel");
//  }
}

/******************************************************************************
    功能说明：小杯输入价格界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_price_s(void)
{
    const char *str_bottle = "bottle";
    const char *str_price = "price";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pPrice;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_price) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pPrice = frame.databuf;

        //存储小杯价格
        //--------

        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：中杯输入价格界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_price_m(void)
{
    const char *str_bottle = "bottle";
    const char *str_price = "price";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pPrice;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_price) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pPrice = frame.databuf;

        //存储中杯价格
        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：大杯输入价格界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_price_l(void)
{
    const char *str_bottle = "bottle";
    const char *str_price = "price";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pPrice;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_price) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pPrice = frame.databuf;

        //存储大杯价格
        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：装瓶清洗第一页
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_wash1(void)
{
    char str_tempe[4];
    unsigned int rest;
    char buf[32];

    if(lcd.page_last != lcd.page_present)
    {//刷新瓶内余量
        //查询瓶内余量
        //-------
        rest = 1;
        //-------
        sprintf(str_tempe, "%d", rest);
        sprintf(buf, "t0.txt=\"%s\"", str_tempe);
        at_cmd(buf);
    }
}

/******************************************************************************
    功能说明：装瓶清洗第二页
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_wash2(void)
{
    const char *str_bottle = "bottle";
    const char *str_wash = "wash";

    static int last_status = 0;
    unsigned long *pTemp;
    static unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
	    bottle_num = 0;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_wash) == 0)
        {
            //清洗命令
            //------
            g_wash_num = bottle_num;
            //-------
        }
    }
}

/******************************************************************************
    功能说明：装瓶清洗第三页
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_wash3(void)
{
    ;
}

/******************************************************************************
    功能说明：容量选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_capacity(void)
{
    const char *str_bottle = "bottle";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pCapacity;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        pCapacity = frame.databuf;

        //存储容量
        //--------

        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：装瓶清洗第四页
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_wash4(void)
{
    const char *str_bottle = "bottle";
    const char *str_bottling = "bottling";

    static int last_status = 0;
    unsigned long *pTemp;
    static unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
	    bottle_num = 0;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_bottling) == 0)
        {
            //装瓶命令
            //------
            g_bottling_num = bottle_num;
            //-------
        }
    }
}


/******************************************************************************
    功能说明：菜单密码界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_code6(void)
{
    char password[7] = {0};
    int len;
    int ret;    
	
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        
	len = store_param_read("men_psw", password);
	if (len == 0) 
	{
		memcpy(password, password2, 6);
		ret = store_param_save("men_psw", password, 6);
		if (ret < 0)
			return;
	}	    
	    	    
	if(strcmp(frame.databuf, password2) == 0)
        {
            at_cmd("code6return.val=1");
        }
        else
        {
            at_cmd("code6return.val=0");
        }
    }
}

/******************************************************************************
    功能说明：设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_setting_menu(void)
{
    ;
}

/******************************************************************************
    功能说明：操作设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_operation_menu(void)
{
    const char *str_tempe = "temperature";
    const char *str_volumeset = "volumeset";

    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_tempe) == 0)
        {
            //判断当前机型是单温区还是多温区机型
            if(0)
            {
                at_cmd("page temperature1");
            }
            else
            {
                at_cmd("page temperature4");
            }
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_volumeset) == 0)
        {
            //判断当前机型是两只装机型还是四只装机型
            if(0)
            {
                at_cmd("page volumeset2");
            }
            else
            {
                at_cmd("page volumeset4");
            }
        }
    }
}

/******************************************************************************
    功能说明：单温区温度设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_temperature1(void)
{
    long *pTemp;
    char temperature;

    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM))
    {
        pTemp = (long*)frame.databuf;
        if((*pTemp >= 5)&&(*pTemp <= 20))
        {
            temperature = (char)*pTemp;

            //存储温度
            //--------

            //---------
            if(1)
            {
                at_cmd("saveOK.val=1");
            }
        }
    }
}

/******************************************************************************
    功能说明：单温区温度设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_temperature4(void)
{
    const char *str_bottle = "bottle";
    const char *str_temperature = "temperature";

    static int last_status = 0;
    long *pTemp;
    unsigned char bottle_num;
    char temperature;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_temperature) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pTemp = (long*)frame.databuf;
        if((*pTemp >= 5)&&(*pTemp <= 20))
        {
            temperature = (char)*pTemp;

            //根据瓶号存储温度
            //--------

            //--------
            if(1)
            {
                at_cmd("saveOK.val=1");
            }
        }
    }
}

/******************************************************************************
    功能说明：两瓶型出酒量设置瓶号选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_volumeset2(void)
{
    ;
}

/******************************************************************************
    功能说明：四瓶型出酒量设置瓶号选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_volumeset4(void)
{
    ;
}

/******************************************************************************
    功能说明：出酒量设置界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_volumesetting(void)
{

    const char *str_bottle = "bottle";
    const char *str_cupS = "cups";
    const char *str_cupM = "cupm";
    const char *str_cupL = "cupl";

    static int last_status = 0;
    unsigned long *pTemp;
    unsigned char bottle_num;
    int volumeS,volumeM,volumeL;

    char press_cnt;
    //计算按键次数
    //-----------
    press_cnt = 1;
    //-----------
    if(1 == press_cnt)
    {
        at_cmd("cup.val=1");
    }
    else if(2 == press_cnt)
    {
        at_cmd("cup.val=2");
    }
    else if(3 == press_cnt)
    {
        at_cmd("cup.val=3");
    }


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_cupS) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pTemp = (unsigned long*)frame.databuf;
        if(*pTemp <= 500)
        {
            volumeS = (int)*pTemp;
            last_status = 4;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 4))
    {
        if(strcmp(frame.databuf, str_cupM) == 0)
        {
            last_status = 5;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 5))
    {
        pTemp = (unsigned long*)frame.databuf;
        if(*pTemp <= 500)
        {
            volumeM = (int)*pTemp;
            last_status = 6;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 6))
    {
        if(strcmp(frame.databuf, str_cupL) == 0)
        {
            last_status = 7;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 7))
    {
        pTemp = (unsigned long*)frame.databuf;
        if(*pTemp <= 500)
        {
            volumeL = (int)*pTemp;
            //根据瓶号存储出酒量
            //--------

            //--------
            if(1)
            {
                at_cmd("saveOK.val=1");
            }
        }
    }
}


/******************************************************************************
    功能说明：出酒量校准界面1
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust1(void)
{
    ;
}

/******************************************************************************
    功能说明：四瓶型出酒量校准瓶位选择界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust_choose4(void)
{
    ;
}

/******************************************************************************
    功能说明：出酒量校准界面2
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust2(void)
{
    const char *str_bottle = "bottle";
    const char *str_pour = "pour";

    static int last_status = 0;
    unsigned long *pTemp;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_pour) == 0)
        {
            //校准时的出酒操作
            //-------

            //-------
        }
    }

}

/******************************************************************************
    功能说明：出酒量校准界面3
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust3(void)
{
    const char *str_bottle = "bottle";
    const char *str_test = "test";

    static int last_status = 0;
    unsigned long *pTemp;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_test) == 0)
        {
            //校准时的测试操作
            //-------

            //-------
        }
    }

}

/******************************************************************************
    功能说明：出酒量校准输入量杯读数界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust4(void)
{
    const char *str_bottle = "bottle";
    const char *str_num = "num";

    static int last_status = 0;
    unsigned long *pTemp;
    char *pNum;
    unsigned char bottle_num;


    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle) == 0)
        {
            last_status = 1;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_NUM)&&(last_status == 1))
    {
        pTemp = (unsigned long*)frame.databuf;
        if((*pTemp >= 1)&&(*pTemp <= 4))
        {
            bottle_num = (char)*pTemp;
            last_status = 2;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 2))
    {
        if(strcmp(frame.databuf, str_num) == 0)
        {
            last_status = 3;
        }
    }
    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR)&&(last_status == 3))
    {
        pNum = frame.databuf;

        //保存读数
        //--------

        //--------
        if(1)
        {
            at_cmd("saveOK.val=1");
        }
    }
}

/******************************************************************************
    功能说明：出酒量校准出酒界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust_pour(void)
{
    char *str_bottle1 = "bottle1";
    char *str_bottle2 = "bottle2";
    char *str_bottle3 = "bottle3";
    char *str_bottle4 = "bottle4";
    char buf[32];

    //根据按下的出酒按键指示的瓶号,发送对应瓶号的点击指令
    if (lcd.page_last != lcd.page_present)
    {
        //判断瓶号
        //---------

        //---------
        if(0)
        {//例:按下2号瓶
            at_cmd("click bt0,0");
        }
        else if(1)
        {
            at_cmd("click bt1,0");
        }
        else if(0)
        {
            at_cmd("click bt2,0");
        }
        else if(0)
        {
            at_cmd("click bt3,0");
        }
    }

    if((frame.valid == TRUE)&&(frame.cmd == CMD_STR))
    {
        if(strcmp(frame.databuf, str_bottle1) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL1);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine1);
            at_cmd(buf);
            memset(buf, 0, 32);
        }
        else if(strcmp(frame.databuf, str_bottle2) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL2);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine2);
            at_cmd(buf);
            memset(buf, 0, 32);
        }else if(strcmp(frame.databuf, str_bottle3) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL3);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine3);
            at_cmd(buf);
            memset(buf, 0, 32);
        }else if(strcmp(frame.databuf, str_bottle4) == 0)
        {
            sprintf(buf, "product.txt=\"%s\"", str_product4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceS.txt=\"%s\"", str_priceS4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceM.txt=\"%s\"", str_priceM4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "priceL.txt=\"%s\"", str_priceL4);
            at_cmd(buf);
            memset(buf, 0, 32);
            vTaskDelay(1);
            sprintf(buf, "restwine.txt=\"%s\"", str_restwine4);
            at_cmd(buf);
            memset(buf, 0, 32);
        }
    }

    //判断酒量是否不足
    if(1)
    {//酒量不足
        at_cmd("empty.val=1");
        //清除酒量不足标志
        //---------

        //---------
    }

    //判断出酒结束
    if(0)
    {
        at_cmd("end.val=1");
    }

}

/******************************************************************************
    功能说明：出酒量校准界面5
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_adjust5(void)
{
    char buf[30];
    const char *pVolume = "75";

    //发送上一界面出酒量的期望值,比如按下了中杯,设定值是75ml
    if (lcd.page_last != lcd.page_present)
    {
        //查询期望值
        //-------

        //-------
        //以字符串格式发送
        sprintf(buf, "t0.txt=\"%s\"", pVolume);
        at_cmd(buf);
    }
}

/******************************************************************************
    功能说明：版本界面
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static void deal_page_versions(void)
{
    char *str_versions1 = "FJJ001";
    char *str_versions2 = "V1.0.0";
    char *str_versions3 = "007";
    char *str_phonenum = "13988888888";
    char buf[32];

    if(lcd.page_last != lcd.page_present)
    {
        sprintf(buf, "t0.txt=\"%s\"", str_versions1);
        at_cmd(buf);
        memset(buf, 0, 32);
        sprintf(buf, "t1.txt=\"%s\"", str_versions2);
        at_cmd(buf);
        memset(buf, 0, 32);
        sprintf(buf, "t2.txt=\"%s\"", str_versions3);
        at_cmd(buf);
        memset(buf, 0, 32);
        sprintf(buf, "t3.txt=\"%s\"", str_phonenum);
        at_cmd(buf);
    }
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_page(void)
{
    //如果返回指令错误
    if((frame.valid == TRUE) && (frame.cmd == CMD_ERR))
    {
        check_page();
        return 1;
    }
    //页面更新
    if((frame.valid == TRUE) && ((frame.cmd == CMD_SENDME)||(frame.cmd == CMD_BUTTON)))
    {
        lcd.page_last = lcd.page_present;
        lcd.page_present = frame.databuf[0];
        frame_str_reset();
        return 1;
    }

    switch(lcd.page_present)
    {
        case page_startup:
            deal_page_startup();
            break;
        case page_code1:
            deal_page_code1();
            break;
        case page_home:
            deal_page_home();
            break;
        case page_information:
            deal_page_information();
            break;
        case page_overpressure:
            deal_page_overpressure();
            break;
        case page_lackpressure:
            deal_page_lackpressure();
            break;
        case page_pour:
            deal_page_pour();
            break;
        case page_code2:
            deal_page_code2();
            break;
        case page_light:
            deal_page_light();
            break;
        case page_code3:
            deal_page_code3();
            break;
        case page_lock:
            deal_page_lock();
            break;
        case page_code4:
            deal_page_code4();
            break;
        case page_sleep_note:
            deal_page_sleep_note();
            break;
        case page_code5:
            deal_page_code5();
            break;
        case page_change2:
            deal_page_change2();
            break;
        case page_change4:
            deal_page_change4();
            break;
        case page_change_menu:
            deal_page_change_menu();
            break;
        case page_infor_menu:
            deal_page_infor_menu();
            break;
        case page_product:
            deal_page_product();
            break;
        case page_price1:
            deal_page_price1();           
            break;
        case page_price_s:
            deal_page_price_s();
            break;
        case page_price_m:
            deal_page_price_m();
            break;
        case    page_price_l:
            deal_page_price_l();
            break;
        case    page_wash1:
            deal_page_wash1();
            break;
        case page_wash2:
            deal_page_wash2();
            break;
        case page_wash3:
            deal_page_wash3();
            break;
        case page_capacity:
            deal_page_capacity();
            break;
        case page_wash4:
            deal_page_wash4();
            break;
        case page_code6:
            deal_page_code6();
            break;
        case page_setting_menu:
            deal_page_setting_menu();
            break;
        case page_operation_menu:
            deal_page_operation_menu();
            break;
        case page_temperature1:
            deal_page_temperature1();
            break;
        case page_temperature4:
            deal_page_temperature4();
            break;
        case page_volume_set2:
            deal_page_volumeset2();
            break;
        case page_volume_set4:
            deal_page_volumeset4();
            break;
        case page_volume_setting:
            deal_page_volumesetting();
            break;
        case page_adjust1:
            deal_page_adjust1();
            break;
        case page_adjust_choose4:
            deal_page_adjust_choose4();
            break;
        case page_adjust2:
            deal_page_adjust2();
            break;
        case page_adjust3:
            deal_page_adjust3();
            break;
        case page_adjust4:
            deal_page_adjust4();
            break;
        case page_adjust_pour:
            deal_page_adjust_pour();
            break;
        case page_adjust5:
            deal_page_adjust5();
            break;
        case page_versions:
            deal_page_versions();
            break;
        default:
            check_page();
            break;

    }

    lcd.page_last = lcd.page_present;
    frame_str_reset();
    return 1;
}

/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int lcd_show(void)
{
    lcd_page();
    lcd_receive();
    return 0;
}




/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void task_lcd(void *pvParameters)
{

    lcd_init();
    lcd.page_last = 0xff;
    lcd.page_present = 0xff;

    for( ;; )
    {
        lcd_show();
        vTaskDelay(1);
    }
}

