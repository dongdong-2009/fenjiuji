/*
 * divid cup ctrl
 * likejshy@126.com
 * 2017-01-06
 */

 
#ifndef _DIVID_CUP_H
#define _DIVID_CUP_H


struct divid_cup_info_str
{
	unsigned char place; /*  ��λ */
	unsigned char bebe; /* С�����б����� */
	unsigned char bebe_ml[16]; //�����
	unsigned char huohao[16];//����
	unsigned char jiage_1[16]; //�۸�
	unsigned char jiage_2[16]; //�۸�
	unsigned char jiage_3[16]; //�۸�
	char yujiubuzu_flag; //��Ʋ����־
	
	unsigned short bebe_mubiao;
	unsigned short total_capacity;
};

extern struct divid_cup_info_str divid_cup_info;


extern void task_divid_cup(void *pvParameters);



#endif /*  _DIVID_CUP_H */