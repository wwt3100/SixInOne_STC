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
                        //HMI_Goto_Error();
                        LOG_E("Error_NoModule");
                        if (gComInfo.HMI_Scene==eScene_Wait)
                        {
                            HMI_Scene_Recovery();
                        }
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
                        if (gInfo.ModuleInfo.RoutineModule.RemainTime>0)
                        {
                            gInfo.ModuleInfo.RoutineModule.RemainTime--;
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
            if (SystemTime100ms==1 && Fire_Flag==1)
            {
                //TODO: 慢启动
            }
        case eWS_Standby:
            if(SystemTime1s==1)
            {
                SystemTime1s=0;
                switch (gComInfo.HMI_Scene)
                {
                    case eScene_Module_650:
                    case eScene_Module_633:
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
                            HMI_Show_Temp(temp);
                        }
                        
                    }
                        break;
                    case eScene_Module_UVA1:
                        gComInfo.TempCount++;
                        Module_GetTemp();
                        break;
                    case eScene_Debug:
                    {
                        if(DS18B20_GetTemp(&temp)==0)
                        {
                            HMI_Show_Temp(temp);
                        }
                    }
                        break;
                    default:
                        break;
                }
                if (Fire_Flag==0)      //吹到低于45度再关风扇
                {
                    if(gComInfo.Count++>10 && temp<4500)    //关闭之后继续吹10秒再关
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
            if (SystemTime100ms==1)     //重发
            {
                SystemTime100ms=0;
                if (Resend_getUsedtime)
                {
                    ModuleRoutine_GetUsedTime();
                }
                if (Resend_getCalibData)
                {
                    ModuleRoutine_GetCalibData();
                }
                if (ADConvertDone)          //读回Vfb
                {
                    ADConvertDone=0;
                }
            }
            break;
        default:
            break;
    }
}

void WP_Start()
{
    switch (gComInfo.HMI_Scene)     //准备
    {
        case eScene_Module_633:
        //case eScene_Module_UVA1:      //TODO: 暂不判断UVA1校准值
            if (gInfo.ModuleInfo.RoutineModule.DAC_Cail==0)  //没有DA值直接返回
            {
                return ;
            }
            break;
        default:
            break;
    }
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
        case eScene_Debug:
            HMI_Cut_Pic(0x71,63, 655, 247, 655+123, 247+78);     //按钮切暂停
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 46, 652, 504, 652+112, 504+72);     //按钮切暂停
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
            //SPI_Send(30310);        //2V
            SPI_Send(gInfo.ModuleInfo.RoutineModule.DAC_Cail);
            //SPI_Send(0x700|3440);   //4.2
            PowerCtr_Light1=POWER_ON; 
            PowerCtr_Main=POWER_ON;   
            break;
        case eScene_Module_UVA1:
            //SPI_Send(0x7FFF);        //5V
            SPI_Send(0x7E66);        //4.5V
            PowerCtr_Main=POWER_ON;
            Delay10ms();
            Module_Send_PWM(1,80);
            break;
        case eScene_Module_IU:
            break;
        case eScene_Module_Wira:
            SPI_Send(0x7E66);        //4.5V
            PowerCtr_Main=POWER_ON;
            PowerCtr_Light1=POWER_ON; 
            Delay10ms();
            LL_Module_Send("\x39\x31\xff\x63",4);
            break;
        case eScene_Module_4in1:
            break;
        case eScene_Debug:
            switch (gComInfo.ModuleType)
            {
                case M_Type_633:
                case M_Type_633_1:
                {
                    uint16_t dac=(float)gInfo.Debug.dac/0.01220703125;
                    SPI_Send(dac|0x7000);
                    PowerCtr_Light1=POWER_ON;  
                    //LOG_E("DAC val:%X",dac|0x7000);
                }
                    break;
                default:
                    break;
            }
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
            SPI_Send(0x7000);           //DAC 0V
            break;
        case eScene_Module_UVA1:
            Module_Send_PWM(1,0);
            PowerCtr_Main=POWER_OFF;    //off
            SPI_Send(0x7000);           //DAC 0V
            break;
        case eScene_Module_IU:
            break;
        case eScene_Module_Wira:
            LL_Module_Send("\x39\x31\xff\x0",4);
            PowerCtr_Light1=POWER_OFF;  //off
            PowerCtr_Main=POWER_OFF;    //off
            SPI_Send(0x7000);           //DAC 0V
            break;
        case eScene_Module_4in1:
            break;
        case eScene_Debug:
            switch (gComInfo.ModuleType)
            {
                case M_Type_633:
                case M_Type_633_1:
                    SPI_Send(0x7000);           //DAC 0V
                    PowerCtr_Light1=POWER_OFF; 
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    if (stop_type==1)       //停止保存,暂停不保存
    {
        switch (gComInfo.HMI_Scene)     //保存使用时间
        {
            case eScene_Module_650:
            case eScene_Module_633:    
            case eScene_Module_UVA1:            
                {
                    uint8_t xdata cmd[]={"\x39\x26\x00"};
                    int8_t usedtime=gInfo.ModuleInfo.RoutineModule.WorkTime-
                        (gInfo.ModuleInfo.RoutineModule.RemainTime)/60;  
                    if (((gInfo.ModuleInfo.RoutineModule.RemainTime)%60)>45)    //15秒以内不计分钟
                    {
                        usedtime--;
                    }
                    if(usedtime>0)
                    {
                        //LOG_E("Save usedtime :%umin",(uint16_t)usedtime);
                        Resend_getUsedtime=1;
                        cmd[2]=usedtime;
                        LL_Module_Send(cmd,3);
                    }
                    //15秒以内不计时
                    
                }
                break;
            case eScene_Module_IU: 
                break;
            case eScene_Module_308:
            default:
                break;
        }
    }
    switch (gComInfo.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 618, 411, 618+139, 411+74);      //按钮切开始
            if (stop_type==1)
            {
                HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 556, 145, 556+215, 145+264);     //切回时间显示
                HMI_Show_Worktime1();
                gComInfo.WorkStat=eWS_Standby;
            }
            else    //暂停
            {
                ;// do nothing
            }
            break;
        case eScene_Module_UVA1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 618, 411, 618+139, 411+74);      //按钮切开始
            if (stop_type==1)
            {
                HMI_Cut_Pic(0x71,gConfig.LANG*100 + 4, 556, 145, 556+215, 145+264);     //切回时间显示
                HMI_Show_Worktime2();
                gComInfo.WorkStat=eWS_Standby;
            }
            else    //暂停
            {
                ;// do nothing
            }
            break;
        case eScene_Debug:
            HMI_Cut_Pic(0x71,62, 655, 247, 655+123, 247+78);     //按钮切暂停
            gComInfo.WorkStat=eWS_Standby;
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 45, 652, 504, 652+112, 504+72);     //按钮切暂停
            gComInfo.WorkStat=eWS_Standby;
            FAN_IO=DISABLE;
            break;
        default:
            break;
    }
    
    
    //FAN_IO=DISABLE;   //根据温度情况自动关闭
}

