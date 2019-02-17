#include "WorkProcess.h"
#include "HMIProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

void Work_Process()
{
    static uint8_t Count=0;
    static bit comm_sended=0;
    switch (gComInfo.WorkStat)
    {
        case eWS_Idle:  //do nothing
            break;
        case eWS_CheckModuleStep1:
            if (SystemTime1s)
            {
                if (comm_sended)
                {
                    comm_sended=0;
                    gComInfo.WorkStat=eWS_CheckModuleStep2; //直接赋值效率应与自加1一致
                    gComInfo.COMMProtocol_Head='@';     //换消息头尾
                    gComInfo.COMMProtocol_Tail1='*';
                    gComInfo.COMMProtocol_Tail2='#';
                }
                else
                {
                    SystemTime1s=0;
                    //LOG_E("ModuleRoutine_Shakehand");
                    ModuleRoutine_Shakehand();
                }
            }
            break;
        case eWS_CheckModuleStep2:
            if (SystemTime1s)
            {
                if (comm_sended)
                {
                    comm_sended=0;
                    gComInfo.WorkStat=eWS_CheckModuleStep1;
                    gComInfo.COMMProtocol_Head=0xAA;
                    gComInfo.COMMProtocol_Tail1=0xC3;
                    gComInfo.COMMProtocol_Tail2=0x3C;
                }
                else
                {
                    SystemTime1s=0;
                    //LOG_E("Module308_Shakehand");
                    Module308_Shakehand();
                    if(++Count>5)
                    {
                        gComInfo.ErrorCode=Error_NoModule;
                        HMI_Goto_Error();
                    }
                }
            }
            break;
        case eWS_Standby:
            
            break;
        case eWS_Working:
            break;
        
        default:
            break;
    }
}


