
/*
 * store files in exflash
 * likejshy@126.com
 * 2016-12-16
 */

#include <string.h>
#include "store.h"
#include "lib.h"
#include "rtc.h"


#define STORE_FILE_INDEX_ADDR	0x3FF00
#define STORE_FILE_INDEX_SIZE	10
#define STORE_FILE_NUM		4



#ifdef CONFIG_STORE_FILE_DEBUG
    #define Print(fmt,args...) debug(fmt, ##args)
#else
    #define Print(fmt,args...)
#endif


/* the files index use to read and write file */
struct store_file_index 
{
	int id; 
	char num;
	unsigned short crc16;
};


/* file info */
struct stroe_file_info 
{
	char name[16]; 
	struct tm time;  /* create data and time */
	unsigned long size;
	unsigned short file_crc16; 
	unsigned short crc16; /* this struct data crc */
};


/* store file map at exflash */
struct store_file_map 
{
	unsigned long file_info_addr; 
	unsigned long file_info_size; 

	unsigned long file_data_addr;
	unsigned long file_data_size;
};


/* file store in exflash map */
const struct store_file_map file_map[STORE_FILE_NUM] = 
{
	{0x40000,	128,	0x40100,	512 * 1024},	/* file 1 */
	{0x50000,	128,	0x50100,	512 * 1024},	/* file 2 */
	{0x60000,	128,	0x60100,	512 * 1024},	/* .....  */
	{0x70000,	128,	0x70100,	512 * 1024},
};


static struct store_file_index file_index;
static struct stroe_file_info file_info;


/* create new file, return the file id for store the file info and file data */
int store_file_creat(char *file_name)
{
	char len;
	int ret;

	/* read file index struct */
	ret = storage_read(STORE_FILE_INDEX_ADDR, (char *)&file_index,
	                 STORE_FILE_INDEX_SIZE);
	if (ret < 0) 
		return -1;

	/* file index struct error, reset this data */
	if (file_index.crc16 != usMBCRC16((unsigned char *)&file_index, sizeof(file_index) - 2)) {
		file_index.id = 0;
		file_index.num = 0;
	}

	/* file index struct error, reset this data */
	if ((file_index.id > 0) || (file_index.num > 0)) {
		if ((file_index.id > file_index.num) 
		    || (file_index.num > STORE_FILE_NUM)) {
			file_index.id = 0;
			file_index.num = 0;
		}
	}
	
	/* create new file info */
	memset((char *)&file_info, 0, sizeof(file_info));

	len = strlen(file_name);
	if (len > 16) 
		len = 16;

	memcpy(file_info.name, file_name, len); 
	 	
	ret = rtc_read(&file_info.time); // set file create time
	if (ret < 0) 
		return -1; 
	
	/* just save in bank 1, the other bank not use */
	file_index.id = 0;
	file_index.num = 0;
	
	return file_index.id;
}


/* save file data */
char bakbuff[1024];
int store_file_write(int file_id, char *buff, int len)
{
	unsigned long file_data_addr;
	unsigned long file_data_size;
	int ret;

	file_data_addr = file_map[file_id].file_data_addr;
	file_data_size = file_map[file_id].file_data_size;

	/* write data must read then check the data writen right */
	if (file_info.size + len < file_data_size) {		
		ret = storage_write(file_data_addr + file_info.size, buff, len);
		if (ret < 0) 
			return -1;		
		ret = storage_read(file_data_addr + file_info.size, bakbuff, len);
		if (ret < 0) 
			return -1;		
		if (memcmp(buff, bakbuff, len) != 0)
			return -1;
		
		file_info.size += len;
		return 0;
	}

	return -1;
}


int store_file_close(int file_id)
{
	unsigned long file_info_addr;
	unsigned long file_info_size;
	int ret;

	file_info_addr = file_map[file_id].file_info_addr;
	file_info_size = file_map[file_id].file_info_size;

	file_info.crc16 = usMBCRC16((unsigned char *)&file_info, sizeof(struct stroe_file_info) - 2);
	ret = storage_write(file_info_addr, (char *)&file_info, file_info_size);
	if (ret < 0) 
		return -1;

	/* index next file stroe area for next file save, save file index struct */
	if (file_id == file_index.id) {
		if (++file_index.id >= STORE_FILE_NUM)
			file_index.id = 0;
	
		if (file_index.num < STORE_FILE_NUM) 
			file_index.num++;
	
		file_index.crc16 = usMBCRC16((unsigned char *)&file_index, 
					     sizeof(file_index) - 2);
		
		ret = storage_write(STORE_FILE_INDEX_ADDR, (char *)&file_index,
		                    STORE_FILE_INDEX_SIZE);
		if (ret < 0) 
			return -1;

		return 0;
	}

	return -1;
}


int store_file_open(int *file_id, char *file_name, unsigned long *file_size)
{	
	unsigned long file_info_addr;
	unsigned long file_info_size;
	int ret;
	char i;

	/* read file index struct */
	ret = storage_read(STORE_FILE_INDEX_ADDR, (char *)&file_index,
	                 STORE_FILE_INDEX_SIZE);
	if (ret < 0) {
		Print("store file storage_read error[%d]\r\n", ret);
		return -1;
	}

	/* file index struct error, reset this data */
	if (file_index.crc16 != usMBCRC16((unsigned char *)&file_index, 
					  sizeof(file_index) - 2)) {
		Print("store file file_index crc16 error\r\n");
		return -1;
	}

	/* file index struct error, reset this data */
	if ((file_index.id > file_index.num) || (file_index.num > STORE_FILE_NUM)) {
		Print("store file file_index id num error\r\n");
		return -1;
	}
	
	for (i = 0; i <file_index.num; i++) {
		file_info_addr = file_map[i].file_info_addr;
		file_info_size = file_map[i].file_info_size;
		ret = storage_read(file_info_addr, (char *)&file_info, file_info_size);
		if (ret < 0) {
			Print("store file storage_read error[%d]\r\n", ret);
			return -1;
		}
		
		if (strcmp(file_name, file_info.name) == 0)
		{
			*file_id = i;
			*file_size = file_info.size;
			return 1;
		}				
	}
	
	return 0;
}



int store_file_read(char file_id, unsigned long offset, char *buff, int size)
{
	unsigned long file_data_addr;
	unsigned long file_data_size;
	int len;

	file_data_addr = file_map[file_id].file_data_addr;
	file_data_size = file_map[file_id].file_data_size;

	if (offset +  size < file_data_size) {		
		len = storage_read(file_data_addr + offset, buff, size);
		if (len < 0) {	
			Print("store file storage_read error[%d]\r\n", len);
			return -1;
		}
		
		return len;
	}

	return 0;
}


int store_file_clear(int file_id)
{	
	unsigned long file_info_addr;
	unsigned long file_info_size;
	int ret;
	
	if (file_id < STORE_FILE_NUM) {	
		file_info_addr = file_map[file_id].file_info_addr;
		file_info_size = file_map[file_id].file_info_size;
		
		memset((char *)&file_info, 0, sizeof(file_info));
		
		ret = storage_write(file_info_addr, (char *)&file_info, file_info_size);
		if (ret < 0) {
			Print("store file storage_write error[%d]\r\n", ret);
			return -1;
		}
		
		return 0;
	}
	
	return -1;
}


