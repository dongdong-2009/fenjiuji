/*
 * RTC and Calendar
 * likejshy@126.com
 * 2016-12-28
 */
 
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

extern void RTC_CalendarShow(char *showtime, char *showdate);
extern int rtc_read(struct tm *calendar);
extern int rtc_write(const unsigned int year, const unsigned int mon,
              	const unsigned int day, const unsigned int hour,
              	const unsigned int min, const unsigned int sec);

#endif /*  _ESP_RTC_H */