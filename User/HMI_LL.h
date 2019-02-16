#ifndef __HMI_LL_H
#define __HMI_LL_H

#include "main.h"


void LL_HMI_Send(const void* str,uint8_t str_len);

//
void HMI_Shake_Hand(void);
void HMI_Show_Logo(void);
void HMI_Goto_Page(uint8_t pic);            //跳转页面
void HMI_Goto_LocPage(uint8_t pic);         //跳转到当前语言的页面


void HMI_Cut_Pic(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye,uint16_t txs,uint16_t tys);



void HMI_Show_ErrorStr(void);



void HMI_Show_ModuleName(const char* str);  //显示治疗模块名称
void HMI_Show_Worktime1(void);       //650 633 IU 使用
void HMI_Show_Worktime2(void);       //UVA1 使用同时显示能量
void HMI_Show_Worktime3(void);       //308 使用




#endif



