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
extern int lcd_author_judge(void);

#endif /*  _TASK_LCD_H */