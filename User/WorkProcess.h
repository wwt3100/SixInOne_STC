#ifndef __WORK_PROCESS_H
#define __WORK_PROCESS_H

#include "main.h"

enum{
    eWS_Idle=0,             //
    eWS_CheckModule,
    eWS_CheckModule308,
    eWS_CheckTempSensor,    //�������ͼ���¶ȴ������Ƿ����
    eWS_Standby,
    eWS_Working,
};



void Work_Process(void);



void WP_Start(void);
void WP_Stop(uint8_t stop_type);    //0->Pause  1->Stop
















#endif



