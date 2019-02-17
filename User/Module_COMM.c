#include "Module_COMM.h"
#include "HMI_COMM.h"
#include "WorkProcess.h"
#include "HMIProcess.h"



void LL_Module_Send(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Head;
    for (i = 0; i < str_len; i++)
    {
        while (Uart2_Busy);
        Uart2_Busy=1;
        S2BUF=*ptr;
        ptr++;
    }
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Tail1;
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Tail2;

}


void Module_COMM()
{
    if (Uart2_ReviceFrame==0)
    {
        return;
    }
    else
    {
        uint8_t xdata pbuf[18];
        uint8_t data_size;
        memcpy(pbuf,uart2_buff,18);
        data_size=pbuf[0];
        Uart2_ReviceFrame=0;
        if (gComInfo.COMMProtocol_Head==0xAA)
        {
            if (pbuf[1]==0x21)
            {
                uint8_t cmd=pbuf[2];
                switch (cmd)
                {
                    case 0x01:      //握手回复治疗头类型
                        gComInfo.ModuleType=pbuf[3];
                        gComInfo.WorkStat=eWS_Standby;
                        LOG_E("Module Type: %02X",(uint16_t)gComInfo.ModuleType);
                        
                        switch (gComInfo.ModuleType)
                        {
                            case M_Type_650:
                                gModuleInfo.RoutineModule.WorkTime=10;  //默认10分钟
                                gComInfo.HMI_Scene=eScene_Module_650;
                                HMI_Goto_LocPage(2);
                                HMI_Show_ModuleName("Derma-650");
                                HMI_Show_Worktime1();
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gModuleInfo.RoutineModule.WorkTime=10;
                                gComInfo.HMI_Scene=eScene_Module_633;
                                HMI_Goto_LocPage(3);
                                HMI_Show_ModuleName("Derma-633");
                                HMI_Show_Worktime1();
                                break;
                          //case M_Type_IU:     //IU是另外协议,在此不会收到
                            case M_Type_UVA1:
                                gModuleInfo.RoutineModule.WorkTime=10;
                                gComInfo.HMI_Scene=eScene_Module_UVA1;
                                HMI_Goto_LocPage(4);
                                HMI_Show_ModuleName("Derma-UVA1");
                                HMI_Show_Worktime1();
                                break;
                            case M_Type_Wira:
                                
                                break;
                            case M_Type_4in1:
                                
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0x03:      //光1光功率
                        
                        break;
                    case 0x04:      //光2光功率
                        break;
                    default:
                        break;
                }
                
            }
            else if(pbuf[1]==0x22)      //光1使用时间
            {
                
            }
            else
            {
                ;   //do nothing
            }
        }
        else    //gComInfo.COMMProtocol_Head=='@'
        {
            
        }
    }
}


void ModuleRoutine_Shakehand()
{
    LL_Module_Send("\x39\x21\x00",3);
}

void Module308_Shakehand()
{
    LL_Module_Send("1*11&5&9",8);
}



