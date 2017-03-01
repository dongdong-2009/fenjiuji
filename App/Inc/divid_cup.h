/*
 * divid cup ctrl
 * likejshy@126.com
 * 2017-01-06
 */

 
#ifndef _DIVID_CUP_H
#define _DIVID_CUP_H


struct divid_cup_info_str
{
	unsigned char place; /*  酒位 */
	unsigned char bebe; /* 小杯，中杯，大杯 */
	unsigned char bebe_ml[16]; //余酒量
	unsigned char huohao[16];//货号
	unsigned char jiage_1[16]; //价格
	unsigned char jiage_2[16]; //价格
	unsigned char jiage_3[16]; //价格
	char yujiubuzu_flag; //余酒不足标志
	
	unsigned short bebe_mubiao;
	unsigned short total_capacity;
};

extern struct divid_cup_info_str divid_cup_info;


extern void task_divid_cup(void *pvParameters);



#endif /*  _DIVID_CUP_H */