
/*
 * boot
 * likejshy@126.com
 * 2016-01-21
 */
 
 
#include <string.h>
#include "bsp.h"
#include "bsp_uart.h"
#include "print.h"
#include "store_file.h"
#include "flash_if.h"

#ifdef MAKE_BOOT

#define CONFIG_BOOT_PRINT

#ifdef CONFIG_BOOT_PRINT
#define print(fmt,args...)  debug(fmt, ##args)
#else
#define print(fmt,args...)
#endif

#define APPLICATION_ADDRESS     (uint32_t)0x08008000 
#define USER_FLASH_BANK1_START_ADDRESS  0x08000000
#define USER_FLASH_BANK1_END_ADDRESS    FLASH_BANK1_END
extern uint32_t ulPortSetInterruptMask( void );
typedef  void (*pFunction)(void);


pFunction JumpToApplication;
uint32_t JumpAddress;
int file_id;
char buff[1024];


void show_boot_msg(void)
{
	print("\r\n\r\n");
	print("\r\n\r\n");
	print("\r\n**********************************************************************");
	print("\r\n*                                                                    *");
	print("\r\n*                          YiFuSi                                    *");
	print("\r\n*                                                                    *");
	print("\r\n*               FenJiuJi Main Ctrl Unit BOOT (Version 1.0.0)         *");
	print("\r\n*                                                                    *");
	print("\r\n*                                         2016-11-20                 *");
	print("\r\n*                                                                    *");
	print("\r\n**********************************************************************");
	print("\r\n");
}


int upgrade_system_check(char *file_name, unsigned long *file_size)
{		
	int ret;
	
	ret = store_file_open(&file_id, file_name, file_size);
	if (ret > 0)
		return 1;	
	return 0;
}



int upgrade_system(char *file_name, unsigned long file_size)
{		
	unsigned long flashdestination = APPLICATION_ADDRESS;
	unsigned long write_size = 0;
	int ret;
	int len;
		
	FLASH_If_Init();	
	FLASH_If_WriteProtectionConfig(FLASHIF_WRP_DISABLE);	
	FLASH_If_Erase(APPLICATION_ADDRESS);
	
	while (write_size < file_size) {
		memset(buff, 0, 1024);		
		len = store_file_read(file_id, write_size, buff, 1024);
		if (len != 1024) 		
			return -1;
		
		ret = FLASH_If_Write(flashdestination, (uint32_t*)buff, len / 4);
                if (ret != 0)
			return -1;
		
		flashdestination += 1024;
   		write_size += 1024;
	}
	
	store_file_clear(file_id);
	
	return 0;
}





int main(void)
{
	char *file_name = "ProjectApp.bin";
	unsigned long file_size = 0;
	int ret;
	
	bsp_init();
	show_boot_msg();

	while (1) {
		print("Boot runing!\r\n");		
		ret = upgrade_system_check(file_name, &file_size);
		if (ret == 1) {
			print("upgrade_system!\r\n");
			ret = upgrade_system(file_name, file_size);
			if (ret < 0)
				print("upgrade_system error[%d]!\r\n", ret);
			
			print("upgrade_system OK[%d]!\r\n", ret);
		}
		
		HAL_Delay(1000);
		print("5...");
		HAL_Delay(1000);
		print("4...");
		HAL_Delay(1000);
		print("3...");		
		HAL_Delay(1000);
		print("2...");
		HAL_Delay(1000);
		print("1...\r\n");
		print("goto APP!\r\n");
		
		ulPortSetInterruptMask();
		
    		if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000) {
      			JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
      			JumpToApplication = (pFunction) JumpAddress;
      			__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
      			JumpToApplication();
    		}				
	}
}


#endif /* MAKE_BOOT */

 