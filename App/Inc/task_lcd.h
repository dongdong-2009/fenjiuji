/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: task_lcd.h
      生成日期: 2016.09.25
      作    者：like
      功能说明：LCD通信模块
      其他说明： 
      修改记录：
*******************************************************************************/
#ifndef _TASK_LCD_H
#define _TASK_LCD_H

extern int g_wash_num;
extern int g_bottling_num;

extern void task_lcd(void *pvParameters);
extern int lcd_author_judge(unsigned char *author);
extern int set_author_limit(void);
extern int jump_pour_page(void);
extern int jump_page_lackpressure(void);


#endif /*  _TASK_LCD_H */