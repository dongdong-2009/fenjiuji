/******************************************************************************
    版权所有：依福斯电子
    版 本 号: 1.0
    文 件 名: store_file.c
    生成日期: 2016.11.09
    作    者：李科
    功能说明：文件存储
    其他说明：
    修改记录：
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "ff_gen_drv.h"
#include "sd_diskio.h"


FATFS SDFatFs;
FIL MyFile;    
char SDPath[4];


/******************************************************************************
    功能说明：
    输入参数：
    输出参数：
    返 回 值：
*******************************************************************************/
int file_sys_test(void)
{
    FRESULT res;                                          
    uint32_t byteswritten, bytesread;   
    uint8_t wtext[] = "This is STM32 working with FatFs"; 
    uint8_t rtext[100];
        
    if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
    {
        if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK)
        {
            return -1;
        }
        else
        {
            if(f_mkfs((TCHAR const*)SDPath, 0, 0) != FR_OK)
            {
                return -1;
            }
            else
            {       
                if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
                {    
                    return -1;
                }
                else
                {      
                    res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
                    if (f_close(&MyFile) != FR_OK )
                    {
                        return -1;
                    }
          
                    if((byteswritten == 0) || (res != FR_OK))
                    {           
                        return -1;
                    }
                    else
                    {             
                        if(f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
                        {
                            return -1;
                        }
                        else
                        {          
                            res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
              
                            if((bytesread == 0) || (res != FR_OK))
                            {                           
                                return -1;
                            }
                            else
                            {            
                                f_close(&MyFile);
                
                                if((bytesread != byteswritten))
                                {                
                                    return -1;
                                }
                                else
                                {
                                    //BSP_LED_On(LED_GREEN);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
  
    FATFS_UnLinkDriver(SDPath);
    return 0;
}