//ymodem ptl receice file, save the file to exflash
//run in FreeRTOS
//likejshy@126.com
//2016-12-21

#ifndef _YMODEM_H
#define _YMODEM_H


#define YMODEM_PORT_COM 0
#define YMODEM_PORT_TCP 1

extern int ymodem_receive_file(char *file_name, char port);


#endif
 