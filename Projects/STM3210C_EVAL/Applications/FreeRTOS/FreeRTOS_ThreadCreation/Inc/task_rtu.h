/******************************************************************************
       �� �� ��: rtu.h
       ��������: 
       ��    �ߣ�
       ����˵����
       ����˵����
       �޸ļ�¼��
*******************************************************************************/
#ifndef _RTU_
#define _RTU_

extern int rtu_jiutou_ctl_set(char rtu_addr, unsigned char jt_ctl_cmd, 
                        unsigned short reg_jt_ctl);
                        
extern int rtu_jiutou_arg_set(char rtu_addr, unsigned char jt_arg_cmd, 
                        unsigned short reg_jt_arg);
                        
extern int rtu_jiutou_dat_get(char rtu_addr, unsigned char jt_dat_index, 
                       unsigned short *reg_jt_dat);                      

extern void task_rtu(void *pvParameters); 

                    
#endif /* _RTU_ */