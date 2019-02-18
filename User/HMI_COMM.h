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

void HMI_Show_ModuleName(const char* str);  //��ʾ����ģ������
void HMI_Show_Worktime1(void);       //650 633 IU ʹ��
void HMI_Show_Worktime2(void);       //UVA1 ʹ��ͬʱ��ʾ����
void HMI_Show_Worktime3(void);       //308 ʹ��

void HMI_Show_Power();       //��ʾ�⹦�� 650 633 
void HMI_Show_RemainTime(); //��ʾʣ��ʱ��
















#endif



