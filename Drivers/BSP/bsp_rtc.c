/******************************************************************************
      版权所有：依福斯电子
      版 本 号: 1.0
      文 件 名: bsp_rtc.c
      生成日期: 2016.10.05
      作    者：like
      功能说明：时钟驱动
      其他说明：
      修改记录：
*******************************************************************************/
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "stm32f1xx_hal.h"
#include "bsp_rtc.h"


#define SECS_PER_HOUR	(60 * 60)
#define SECS_PER_DAY	(SECS_PER_HOUR * 24)


static const unsigned short __mon_yday[2][13] = {
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};


RTC_HandleTypeDef RtcHandle;



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
int bsp_rtc_init(void)
{
    unsigned long sec;
  
    RtcHandle.Instance = RTC; 
    RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;

    if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
    {
        return -1;
    }
      
    if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x1231)
    {
        sec = mktime(2000, 1, 1, 0, 0, 0);
        
        __HAL_RTC_WRITEPROTECTION_DISABLE(&RtcHandle);
        RtcHandle.Instance->CNTH = (sec >> 16);
        RtcHandle.Instance->CNTL = sec & 0xFFFF;
        __HAL_RTC_WRITEPROTECTION_ENABLE(&RtcHandle); 
           
        HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x1231);
    } 
    
    return 0;
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void RTC_CalendarShow(char *showtime, char *showdate)
{
    unsigned long totalsecs;
    struct tm calendar;
            
    totalsecs = RtcHandle.Instance->CNTH * 0x10000 + RtcHandle.Instance->CNTL ;
    
    time_to_tm(totalsecs, 0, &calendar);
    calendar.tm_mon += 1;
    calendar.tm_year += 1900;
    
    sprintf((char *)showtime, "%2d:%2d:%2d", calendar.tm_hour, calendar.tm_min, calendar.tm_sec);
    sprintf((char *)showdate, "%2d-%2d-%4d", calendar.tm_mon, calendar.tm_mday, calendar.tm_year);
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
unsigned long mktime(const unsigned int year0, const unsigned int mon0,
                     const unsigned int day, const unsigned int hour,
                     const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	if (0 >= (int) (mon -= 2)) {
		mon += 12;
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour 
	  )*60 + min 
	)*60 + sec;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static int __isleap(long year)
{
	return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static long math_div(long a, long b)
{
	return a / b - (a % b < 0);
}



/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
static long leaps_between(long y1, long y2)
{
	long leaps1 = math_div(y1 - 1, 4) - math_div(y1 - 1, 100)
		+ math_div(y1 - 1, 400);
	long leaps2 = math_div(y2 - 1, 4) - math_div(y2 - 1, 100)
		+ math_div(y2 - 1, 400);
	return leaps2 - leaps1;
}


/******************************************************************************
    功能说明：无
    输入参数：无
    输出参数：无
    返 回 值：无
*******************************************************************************/
void time_to_tm(time_t totalsecs, int offset, struct tm *result)
{
	long days, rem, y;
	const unsigned short *ip;

	days = totalsecs / SECS_PER_DAY;
	rem = totalsecs % SECS_PER_DAY;
	rem += offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}

	result->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	result->tm_min = rem / 60;
	result->tm_sec = rem % 60;

	result->tm_wday = (4 + days) % 7;
	if (result->tm_wday < 0)
		result->tm_wday += 7;

	y = 1970;

	while (days < 0 || days >= (__isleap(y) ? 366 : 365)) {
		long yg = y + math_div(days, 365);

		days -= (yg - y) * 365 + leaps_between(y, yg);
		y = yg;
	}

	result->tm_year = y - 1900;

	result->tm_yday = days;

	ip = __mon_yday[__isleap(y)];
	for (y = 11; days < ip[y]; y--)
		continue;
	days -= ip[y];

	result->tm_mon = y;
	result->tm_mday = days + 1;
}














