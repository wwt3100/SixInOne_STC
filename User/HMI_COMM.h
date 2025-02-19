#ifndef __HMI_COMM_H
#define __HMI_COMM_H

#include "HMI_LL.h"

enum{
    eMsg_NULL=0,
    eMsg_HMI_Shakehand,
    eMsg_KeyDown,
    eMsg_KeyLongPush,
    eMsg_keyUp,
    eMSg_CailDone,
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
    eHMICode_WorktimeParallel=0x16,     //同时出光
};

void HMI_COMM(void);


//User functions
void HMI_Show_ErrorStr(void);


void HMI_Show_WorkMode(void);

void HMI_Show_ModuleName(const char* str);  //显示治疗模块名称
void HMI_Show_Worktime1(void);       //650 633 IU 使用
void HMI_Show_Worktime2(void);       //UVA1 使用同时显示能量

void HMI_Show_Power(void);       //显示光功率 650 633 
void HMI_Show_RemainTime(void); //显示剩余时间

void HMI_Show_Temp(int16_t temp);       //显示温度
void HMI_Show_Password(void);       //密码输入页显示密码




//调试界面
void HMI_DGB_Show_DAval();




//IU
void HMI_Show_IU_Usedtime();
void HMI_Show_IU_Temp();


//308
void HMI_Show_308WorkTime();
void HMI_Show_308RemainTime();

//308 红斑测试
void HMI_308Test_SelTime(uint8_t seltime);
void HMI_308Test_AllPower();

//308设置
void HMI_308Set_Freq();
void HMI_308Set_Duty();



//Wira 4in1
void HMI_New_Add();
void HMI_New_Dec();
void HMI_New_Sel(uint8_t sel);


void HMI_New_ShowStr(uint8_t sel);
void HMI_New_Show_LightName(uint8_t sellight);
void HMI_New_ShowLite();
void HMI_New_ShowDetail(uint8_t countdown);
void HMI_New_ShowEditMode();
void HMI_New_LoadLightStep(); //加载工步信息
void HMI_New_SaveLightStep();
void HMI_New_Add_Step(uint8_t st);
void HMI_New_Dec_Step(uint8_t st);   //减工步/光           0-3
void HMI_New_ShowList();



#endif



