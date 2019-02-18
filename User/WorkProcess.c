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
                    LOG_E("ModuleRoutine_Shakehand");
                    ModuleRoutine_Shakehand();
                    comm_sended=1;
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
                    LOG_E("Module308_Shakehand");
                    Module308_Shakehand();
                    comm_sended=1;
                    if(++Count>5)
                    {
                        Count=0;
                        gComInfo.ErrorCode=Error_NoModule;
                        HMI_Goto_Error();
                        LOG_E("Error_NoModule");
                    }
                }
            }
            break; 
        case eWS_Working:
            if(SystemTime1s==1 && Fire_Flag==1)
            {
                switch (gComInfo.HMI_Scene)
                {
                    case eScene_Module_650:
                    case eScene_Module_633:
                    case eScene_Module_IU:
                    case eScene_Module_UVA1:
                        if (gModuleInfo.RoutineModule.RemainTime>0)
                        {
                            gModuleInfo.RoutineModule.RemainTime--;
                            HMI_Show_RemainTime();
                        }
                        else
                        {
                            WP_Stop(1);     //Timeout Stop
                        }
                        break;
                    default:
                        break;
                }
            }
        case eWS_Standby:
            if(SystemTime1s==1)
            {
                SystemTime1s=0;
                //DOTO:Show Temp
            }
            break;
        default:
            break;
    }
}

void WP_Start()
{
    FAN_IO=ENABLE;
    switch (gComInfo.HMI_Scene)
    {
        case eScene_Module_650:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 16, 618, 411, 618+139, 411+74);     //按钮切暂停
            HMI_Show_RemainTime();
            PowerCtr_Module12v=0;
            break;
        case eScene_Module_633:
        case eScene_Module_IU:
        case eScene_Module_UVA1:
            
            break;
        default:
            break;
    }
    gComInfo.WorkStat=eWS_Working;
    gComInfo.TimerCounter=0;
    Fire_Flag=1;
}

void WP_Stop(uint8_t stop_type)
{
    Fire_Flag=0;
    switch (gComInfo.HMI_Scene)
        {
            case eScene_Module_650:
                PowerCtr_Module12v=1;
                break;
            case eScene_Module_633:
            case eScene_Module_IU:
            case eScene_Module_UVA1:
                
                break;
            default:
                break;
        }

    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 618, 411, 618+139, 411+74);      //按钮切开始
    if (stop_type==1)
    {
        HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 556, 145, 556+215, 145+264);     //切回时间显示
        HMI_Show_Worktime1();
        gComInfo.WorkStat=eWS_Standby;
    }
    else    //暂停
    {
        
    }
    FAN_IO=DISABLE;
}

