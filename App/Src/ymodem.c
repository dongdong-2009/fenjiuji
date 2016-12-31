
//ymodem ptl receive file, save the file to exflash
//run in FreeRTOS
//likejshy@126.com
//2016-12-21

#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "ymodem.h"
#include "task_wifi.h"
#include "bsp_uart.h"
#include "store_file.h"
#include "lib.h"


#define YMODEM_ACK 0x46
#define YMODEM_EOT 0x04 //end of transmission


#define CONFIG_YMODEM_DEBUG


#ifdef CONFIG_YMODEM_DEBUG
    #define Print(fmt,args...) debug(fmt, ##args)
#else
    #define Print(fmt,args...)
#endif


//send a char to port
static int ymodem_putchar(char ch, char port)
{
	int ret;

	if (port == YMODEM_PORT_COM) {
		ret = bsp_uart_send(UART_1, &ch, 1);
		if (ret < 0) {
			Print("ymodem bsp_uart_send error[%d]\r\n", ret);
			return -1;
		}
	}

	if (port == YMODEM_PORT_TCP) {
		ret = wifi_send_byte(&ch, 1);
		if (ret < 0) {
			Print("ymodem wifi_send_byte error[%d]\r\n", ret);
		  	return -1;
		}
	}

	return 0;
}



//receive byte from port
static int ymodem_receive_byte(char *rxbuf, int size, char port)
{
	int len;

	if (port == YMODEM_PORT_COM) {
		len = bsp_uart_receive(UART_1, rxbuf, size);
		if (len < 0) {
			Print("ymodem bsp_uart_receive error[%d]\r\n", len);
			return -1;
		}
	}

	if (port == YMODEM_PORT_TCP) {
		len = wifi_receive_byte(rxbuf, size);
		if (len < 0) {
			Print("ymodem wifi_receive_byte error[%d]\r\n", len);
			return -1;
		}
	}

	return len;
}


// unpack the fisrt ymodem frame
static int ymodem_file_info_unpack(char *file_name, unsigned long *file_size,
                                   char *rxbuf, int len)
{
	char file_size_str[8] = {0};
	unsigned short crc = 0;

	// first package_no file data len = 128
	if (rxbuf[0] == 0x01) {
		// pack num 0
		if ((rxbuf[1] == 0x00) && (rxbuf[2] == 0xFF)) {
			crc = Cal_CRC16((const unsigned char*)rxbuf + 3, 128);
			if (crc == (rxbuf[len - 2] * 0x100 + rxbuf[len - 1])) {
				//get file name
				strcpy(file_name, rxbuf + 3);
				len = strlen(file_name);

				//cover the file_size to int
				strcpy(file_size_str, rxbuf + 3 + len + 1);
				sscanf(file_size_str, "%d", file_size);

				return 0;
			}
		}
	}

	return -1;
}


// unpack the ymodem file data frame
static int ymodem_file_data_unpack(int *package_no, char *rxbuf, int len)
{
	unsigned short crc = 0;

	// file data len 1024
	if (rxbuf[0] == 0x02) {
		// package_no must same pro set
		if ((rxbuf[1] == *package_no) && (rxbuf[2] == ~rxbuf[2])) {
			// crc16 from data
			crc = Cal_CRC16((const unsigned char*)rxbuf + 3, 1024);
			if (crc == (rxbuf[len - 2] * 0x100 + rxbuf[len - 1])) {
				memcpy(rxbuf, rxbuf + 3, 1024);
				*package_no++; // set the next package_no

				return 1024;
			}
		}
	}

	return 0;
}




static int ymodem_receive_frame(char *rxbuf, int size, char port, char over_time)
{
	int tick = over_time * 1000 / 50;
	int rxlen = 0, len;
	// if rxlen > 0, over 3 tick not receive data, frame receive finish
	int byte_tick = 3;


	while (tick-- > 0) {
		//recevie byte from port
		len = ymodem_receive_byte(rxbuf + rxlen, size - rxlen, port);
		if (len < 0) {
			Print("ymodem_receive_byte error[%d]\r\n", len);
			return -1;
		}

		if (len > 0) {
			rxlen += len;
			byte_tick = 3; //reset this byte_tick
			tick = over_time * 1000 / 50; // reset tick
		}

		if ((len == 0) && (rxlen > 0)) {
			if (byte_tick-- == 0) 
				return rxlen;
		}

		vTaskDelay(50);
	}

	return 0;
}



//tcp port will receive this frame
static int ymodem_receive_tcp_frame(char port)
{
	const char frame[6] = {0xFF, 0xFD, 0x00, 0xFF, 0xFB, 0x00};
	char rxframe[6] = {0};
	int len;
	char ovt_time = 30;
	int ret;

	while (ovt_time--) {
		ret = ymodem_putchar('C', port);
		if (ret < 0) {
			Print("ymodem_putchar error[%d]\r\n", ret);
			return -1;
		}

		len = ymodem_receive_frame(rxframe, 6, port, 3);
		if (len < 0) {
			Print("ymodem_receive_frame error[%d]\r\n", ret);
			return -1;
		}
				
		if (len == 6) {
			if (memcmp(rxframe, frame, 6) == 0) {
				ret = ymodem_putchar(YMODEM_ACK, port);
				if (ret < 0) {
					Print("ymodem_putchar error[%d]\r\n", ret);
					return -1;
				}
				
				return 1;
			}
		}

		vTaskDelay(2000);
	}

	return 0;
}


// get the download file info of name, size
static int ymodem_receive_file_info(char *file_name, unsigned long *file_size,
                                    char port, char *rxbuf)
{
	int len;
	int ret;

	ymodem_putchar('C', port);

	len = ymodem_receive_frame(rxbuf, 200, port, 3);
	if (len < 0) {
		Print("ymodem_receive_frame error[%d]\r\n", len);
		return -1;
	}	
	
	if (len > 0) {
		ret = ymodem_file_info_unpack(file_name, file_size, rxbuf, len);
		if (ret < 0) {
			Print("ymodem_file_info_unpack error[%d]\r\n", ret);
			return -1;
		}	
			
		if (ret == 0) {
			ret = ymodem_putchar(YMODEM_ACK, port);
			if (ret < 0) {
				Print("ymodem_putchar error[%d]\r\n", ret);
				return -1;
			}
			
			return 0;
		}
	}

	return -2; //receive file info ovt
}


//receive the file body, get the file data from ymodem ptl
static int ymodem_receive_file_data(int *package_no, char *rxbuf, char port)
{
	const char end_frame[5] = {0x18, 0x18, 0x18, 0x18, 0x18};
	int len;
	int ret;

	len = ymodem_receive_frame(rxbuf, 1200, port, 30);
	if (len < 0) {
		Print("ymodem_receive_frame error[%d]\r\n", len);
		return -1;
	}	
	
	if (len >= 5) {
		//HyperTerminal esc transmission
		if (memcmp(rxbuf, end_frame, 5) == 0) {
			ret = ymodem_putchar(YMODEM_ACK, port);
			if (ret < 0) {
				Print("ymodem_putchar error[%d]\r\n", ret);
				return -1;
			}
			
			return -2; //esc transmission
		}

		//unpack the ymodem ptl, get the file data and len
		len = ymodem_file_data_unpack(package_no, rxbuf, len);		
		if (len > 0) {
			ret = ymodem_putchar(YMODEM_ACK, port);
			if (len < 0) {	
				Print("ymodem_putchar error[%d]\r\n", len);
				return -1;
			}
			
			return len;
		}
	}

	return 0; // receive data ovt
}


//use ymodem ptl receive a file from HyperTerminal
int ymodem_receive_file(char *file_name, char port)
{
	unsigned long download_file_size = 0;
	unsigned long receive_file_size = 0;
	char rxbuf[1200] = {0};
	int package_no = 1;
	int len;
	int ret;
	int file_id;

	if (port == YMODEM_PORT_TCP) {
		//if use tcp port, HyperTerminal will send this frame,
		//but com port not send this
		ret = ymodem_receive_tcp_frame(port);
		if (ret < 0) {
			Print("ymodem_receive_tcp_frame error[%d]\r\n", ret);
			return -1;
		}
					
	}

	// get the download file info of name, size
	ret = ymodem_receive_file_info(file_name, &download_file_size, port, rxbuf);
	if (ret < 0) {
		Print("ymodem_receive_file_info error[%d]\r\n", ret);
		return -1;
	}
        
	file_id = store_file_creat(file_name);
	if (file_id < 0) {
		Print("ymodem store_file_create error[%d]\r\n", file_id);
		return -1;
	}	
		
	//deal the receive process of file data
	while (download_file_size > receive_file_size) {
		//get the file data
		len = ymodem_receive_file_data(&package_no, rxbuf, port);
		if (len < 0) {
			Print("ymodem_receive_file_data error[%d]\r\n", len);
			return -1;
		}

		// save the file data to exflash store
		ret = store_file_write(file_id, rxbuf, len);
		if (ret < 0) {
			Print("ymodem store_file_write error[%d]\r\n", ret);
			return -1;
		}

		receive_file_size += len;
	}

	//end of transmission
	ret = ymodem_putchar(YMODEM_EOT, port);
	if (ret < 0) {
		Print("ymodem_putchar error[%d]\r\n", ret);
		return -1;
	}

	ret = store_file_close(file_id);
	if (ret < 0) {
		Print("ymodem store_file_close error[%d]\r\n", ret);
		return -1;
	}
	
	return receive_file_size;
}







