#ifndef __WORK_PROCESS_H
#define __WORK_PROCESS_H

enum{
    eWS_Idle=0,             //
    eWS_CheckModuleStep1,   //使用协议检测治疗头
    eWS_CheckTempSensor,    //根据类型检查温度传感器是否存在
    eWS_CheckModuleStep2,   //检测308 IU 治疗头
    eWS_Wait,
    eWS_Working,
};



void Work_Process(void);





















#endif



