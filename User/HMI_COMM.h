#ifndef __HMI_COMM_H
#define __HMI_COMM_H

#include "HMI_LL.h"

enum{
    eMsg_NULL=0,
    eMsg_HMI_Shakehand,
    eMsg_KeyDown,
    eMsg_KeyLongPush,
    eMsg_keyUp,
};


void HMI_COMM(void);


//User functions
void HMI_Show_ErrorStr(void);


void HMI_Show_WorkMode(void);

void HMI_Show_ModuleName(const char* str);  //显示治疗模块名称
void HMI_Show_Worktime1(void);       //650 633 IU 使用
void HMI_Show_Worktime2(void);       //UVA1 使用同时显示能量
void HMI_Show_Worktime3(void);       //308 使用

void HMI_Show_Power();       //显示光功率 650 633 
void HMI_Show_RemainTime(); //显示剩余时间
















#endif



