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

extern int rtu_jiutou_ctl_set(char rtu_addr, unsigned short jt_ctl_cmd, 
                        unsigned short reg_jt_ctl);
                        
extern int rtu_jiutou_arg_set(char rtu_addr, unsigned short jt_arg_cmd, 
                        unsigned short reg_jt_arg);
                        
extern int rtu_jiutou_dat_get(char rtu_addr, unsigned short jt_dat_index, 
                       unsigned short *reg_jt_dat);                      

extern int rtu_tiaoya_ctl_set(char rtu_addr, unsigned short ty_ctl_cmd,
		       unsigned short reg_ty_ctl);		       
		       
extern int rtu_tiaoya_dat_get(char rtu_addr, unsigned short ty_dat_index,
		       unsigned short *reg_ty_dat);
		       
extern int rtu_tiaoya_arg_set(char rtu_addr, unsigned short ty_arg_cmd,
		       unsigned short reg_ty_arg);		       
		       
extern void task_rtu(void *pvParameters); 

                    
#endif /* _RTU_ */