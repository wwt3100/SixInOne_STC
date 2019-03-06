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

enum{
    eHMICode_Light1=0x08,
    eHMICode_Light2,
    eHMICode_Light3,
    eHMICode_Light4,
    eHMICode_PowerLevel1 =0x0C,
    eHMICode_PowerLevel2,
    eHMICode_PowerLevel3,
    eHMICode_PowerLevel4,
    eHMICode_Worktime1   =0x10,
    eHMICode_Worktime2,
    eHMICode_Worktime3,
    eHMICode_Worktime4,
    eHMICode_WorktimeParallel=0x16,     //ͬʱ����
};

void HMI_COMM(void);


//User functions
void HMI_Show_ErrorStr(void);


void HMI_Show_WorkMode(void);

void HMI_Show_ModuleName(const char* str);  //��ʾ����ģ������
void HMI_Show_Worktime1(void);       //650 633 IU ʹ��
void HMI_Show_Worktime2(void);       //UVA1 ʹ��ͬʱ��ʾ����
void HMI_Show_Worktime3(void);       //308 ʹ��

void HMI_Show_Power(void);       //��ʾ�⹦�� 650 633 
void HMI_Show_RemainTime(void); //��ʾʣ��ʱ��

void HMI_Show_Temp(int16_t temp);       //��ʾ�¶�
void HMI_Show_Password(void);       //��������ҳ��ʾ����




//���Խ���
void HMI_DGB_Show_DAval();




//IU
void HMI_Show_IU_Usedtime();


//Wira 4in1
void HMI_New_Add();
void HMI_New_Dec();
void HMI_New_Sel(uint8_t sel);


void HMI_New_ShowStr(uint8_t sel);
void HMI_New_Show_Light(uint8_t sellight);



#endif



