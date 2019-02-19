#include "WorkProcess.h"
#include "HMIProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

void Work_Process()
{
    static bit comm_sended=0;

    int16_t temp=0;
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
                    if(++gComInfo.Count>5)
                    {
                        gComInfo.Count=0;
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
                gComInfo.Count=0;
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
                switch (gComInfo.HMI_Scene)
                {
                    case eScene_Module_650:
                    case eScene_Module_633:
                    case eScene_Module_UVA1:
                    {
                        uint8_t ret=DS18B20_GetTemp(&temp);
                        if(ret==1)  //DS18B20不在
                        {
                            gComInfo.TempCount+=2;
                        }
                        else if(ret==2)
                        {
                            gComInfo.TempCount++;
                        }
                        else
                        {
                            gComInfo.TempCount=0;
                            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 13, 415, 13+81, 415+36); //恢复温度背景
                            LL_HMI_Send("\x98\x00\x37\x01\x9F\x21\x81\x03\x00\x1F\x00\x1F",12);
                            if (temp<0)
                            {
                                temp=0;
                            }
                            else
                            {
                                temp=(temp+5)/10;     //四舍五入取整
                            }
                            if (temp/10==0)
                            {
                                while (Uart1_Busy);     //s十位
                                Uart1_Busy=1;
                                SBUF=' ';
                            }
                            else
                            {
                                while (Uart1_Busy);     //s十位
                                Uart1_Busy=1;
                                SBUF=temp/10+'0';
                            }
                            while (Uart1_Busy);     //温度个位
                            Uart1_Busy=1;
                            SBUF=temp%10+'0';
                            LL_HMI_SendEnd();
                        }
                    }
                        break;
                    default:
                        break;
                }
                if (Fire_Flag==0)      //吹到低于45度再关风扇
                {
                    if(gComInfo.Count++>10 && temp<4500)    //关闭之后继续吹30秒再关
                    {
                        FAN_IO=DISABLE;
                    }
                }
                if (gComInfo.TempCount>10)   //温度传感器错误
                {
                    gComInfo.ErrorCode=Error_TempSenerError;
                    HMI_Goto_Error();
                }
            }
            break;
        default:
            break;
    }
}

void WP_Start()
{
    FAN_IO=ENABLE;
    switch (gComInfo.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
        case eScene_Module_UVA1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 16, 618, 411, 618+139, 411+74);     //按钮切暂停
            HMI_Show_RemainTime();
            break;
        default:
            break;
    }
    switch (gComInfo.HMI_Scene)     //工作部分
    {
        case eScene_Module_650:
            PowerCtr_Module12v=POWER_ON;
            break;
        case eScene_Module_633:
            SPI_Send(30310);
            PowerCtr_Light1=POWER_ON; 
            PowerCtr_Main=POWER_ON;   
            break;
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
    switch (gComInfo.HMI_Scene)     //工作部分
    {
        case eScene_Module_650:
            PowerCtr_Module12v=POWER_OFF;
            break;
        case eScene_Module_633:
            
            PowerCtr_Light1=POWER_OFF;  //off
            PowerCtr_Main=POWER_OFF;    //off
            break;
        case eScene_Module_IU:
        case eScene_Module_UVA1:
            
            break;
        default:
            break;
    }
    switch (gComInfo.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
        case eScene_Module_UVA1:
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
            break;
        default:
            break;
    }
    
    //FAN_IO=DISABLE;   //根据温度情况自动关闭
}

