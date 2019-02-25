#ifndef __HMI_LL_H
#define __HMI_LL_H

#include "main.h"


#define HMI_COLOR_BLACK 0x0
#define HMI_COLOR_WHITE 0xFFFF
#define HMI_COLOR_RED   0xF800
#define HMI_COLOR_BLUE  0x001F
#define HMI_COLOR_GREEN 0x07E0


void LL_HMI_Send(const void* str,uint8_t str_len);
void LL_HMI_Send_Pure(const void* str,uint8_t str_len); //不带0xAA头
void LL_HMI_SendXY(uint16_t x,uint16_t y);
void LL_HMI_SendEnd(void);
//
void HMI_Shake_Hand(void);
void HMI_Show_Logo(void);
void HMI_Goto_Page(uint8_t pic);            //跳转页面
void HMI_Goto_LocPage(uint8_t pic);         //跳转到当前语言的页面

void HMI_Cut_Pic(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye);
void HMI_Cut_PicEx(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye,uint16_t txs,uint16_t tys);








#endif



