/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: task_lcd.h
      ��������: 2016.09.25
      ��    �ߣ�like
      ����˵����LCDͨ��ģ��
      ����˵���� 
      �޸ļ�¼��
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