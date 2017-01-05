/*
 * print for APP
 * likejshy@126.com
 * 2016-12-16
 */

#ifndef _PRINT_H
#define _PRINT_H

#define  APP_PRINT_PORT_COM 0

#define CONFIG_APP_PRINT

#ifdef CONFIG_APP_PRINT
    #define app_print(fmt,args...)  __app_print(fmt, ##args)
#else
    #define app_print(fmt,args...)
#endif


extern void app_print_config(char en, char port);
extern void __app_print(const char *fmt, ...);




#endif 