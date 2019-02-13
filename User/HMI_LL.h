#ifndef __HMI_LL_H
#define __HMI_LL_H

#include "main.h"


void LL_HMI_Send(const void* str,uint8_t str_len);

//
void HMI_Shake_Hand(void);
void HMI_Show_Logo(void);
void HMI_Goto_LocPage(uint8_t pic);










#endif



