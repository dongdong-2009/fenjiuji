/******************************************************************************
      ��Ȩ���У�����˹����
      �� �� ��: 1.0
      �� �� ��: bsp_rtc.h
      ��������: 2016.10.05
      ��    �ߣ�like
      ����˵����ʱ������
      ����˵���� 
      �޸ļ�¼��
*******************************************************************************/
#ifndef _ESP_RTC_H
#define _ESP_RTC_H

#ifndef _TIME_T
#define _TIME_T
typedef unsigned long  time_t;
#endif


struct tm 
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	long tm_year;
	int tm_wday;
	int tm_yday;
};

extern int bsp_rtc_init(void);
extern void RTC_CalendarShow(char *showtime, char *showdate);
extern int RTC_CalendarConfig(char year, char month, char date,
                              char hour, char minute, char second);

void time_to_tm(time_t totalsecs, int offset, struct tm *result);
unsigned long mktime(const unsigned int year0, const unsigned int mon0,
                     const unsigned int day, const unsigned int hour,
                     const unsigned int min, const unsigned int sec);


#endif /*  _ESP_RTC_H */