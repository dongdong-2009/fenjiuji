
/*
 * store files in exflash
 * likejshy@126.com
 * 2016-12-16
 */

#include <string.h>
#include "store.h"
#include "lib.h"

#define STORE_FILE_INDEX_ADDR	0x00000
#define STORE_FILE_INDEX_SIZE	10
#define STORE_FILE_NUM		4


/* the files index use to read and write file */
struct store_file_index 
{
	char id; 
	char num;
	unsigned short crc16;
};


/* file info */
struct stroe_file_info 
{
	char name[16]; 
	char time[7];  /* create data and time */
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
	{0,	0,	0,	0},	/* file 1 */
	{0,	0,	0,	0},	/* file 2 */
	{0,	0,	0,	0},	/* .....  */
	{0,	0,	0,	0},
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
	if ((file_index.id >= file_index.num) || (file_index.num > STORE_FILE_NUM)) {
		file_index.id = 0;
		file_index.num = 0;
	}

	/* create new file info */
	memset((char *)&file_info, 0, sizeof(file_info));

	len = strlen(file_name);
	if (len > 16) 
		len = 16;

	memcpy(file_info.name, file_name, len); 

	/* 	
	ret = rtc_time_read(file_info.time); // set file create time
	if (ret < 0) 
		return -1; 
	*/

	return file_index.id;
}


/* save file data */
int store_file_write(char file_id, char *buff, int len)
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

	file_info.crc16 = usMBCRC16((unsigned char *)&file_info, sizeof(file_info) - 2);
	ret = storage_write(file_info_addr, (char *)&file_info, file_info_size);
	if (ret < 0) 
		return -1;

	/* index next file stroe area for next file save, save file index struct */
	if (file_id == file_index.id) {
		file_index.id++;
		if (file_index.id >= STORE_FILE_NUM)
			file_index.id = 0;

		file_index.num++;
		if (file_index.num > STORE_FILE_NUM) 
			file_index.id = STORE_FILE_NUM;

		ret = storage_write(STORE_FILE_INDEX_ADDR, (char *)&file_index,
		                 STORE_FILE_INDEX_SIZE);
		if (ret < 0) 
			return -1;

		return 0;
	}

	return -1;
}


int store_file_open(char *file_name)
{
	return 0;
}



int store_file_read(char file_id, char *buff, int len)
{
	return 0;
}


