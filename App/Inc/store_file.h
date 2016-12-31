
//store files in exflash
//likejshy@126.com
//2016-12-16

#ifndef _STORE_FILE_H
#define _STORE_FILE_H


extern int store_file_creat(char *file_name);
extern int store_file_write(char file_id, char *buff, int len);
extern int store_file_close(int file_id);
extern int store_file_read(char file_id, char *buff, int len);

#endif