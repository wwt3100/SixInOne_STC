#ifndef __WORK_PROCESS_H
#define __WORK_PROCESS_H

#include "main.h"

enum{
    eWS_Idle=0,             //
    eWS_CheckModule,
    eWS_CheckModuleStep1,   //使用协议检测治疗头
    eWS_CheckTempSensor,    //根据类型检查温度传感器是否存在
    eWS_CheckModuleStep2,   //检测308 IU 治疗头
    eWS_Standby,
    eWS_Working,
};



void Work_Process(void);



void WP_Start(void);
void WP_Stop(uint8_t stop_type);    //0->Pause  1->Stop
















#endif



