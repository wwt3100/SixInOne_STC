#ifndef __WORK_PROCESS_H
#define __WORK_PROCESS_H

enum{
    eWS_Idle=0,             //
    eWS_CheckModuleStep1,   //ʹ��Э��������ͷ
    eWS_CheckTempSensor,    //�������ͼ���¶ȴ������Ƿ����
    eWS_CheckModuleStep2,   //���308 IU ����ͷ
    eWS_Wait,
    eWS_Working,
};



void Work_Process(void);





















#endif



