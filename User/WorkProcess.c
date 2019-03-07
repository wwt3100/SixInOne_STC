#include "WorkProcess.h"
#include "HMIProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

void Work_Process()
{
    int16_t temp=0;
    switch (gComInfo.WorkStat)
    {
        case eWS_Idle:  //do nothing
            break;
        case eWS_CheckModule:
            if (SystemTime1s)
            {
                SystemTime1s=0;
                if (DQ==0)      //如果温度脚是低电平则是接的IU/308头
                {
                    SPI_Send(0x7E66);        //4.5V
                    PowerCtr_Main=POWER_ON;     //开总电
                    PowerCtr_Light2=POWER_ON;   //开308电源才能通信
                    Delay10ms();
                    gComInfo.COMMProtocol_Head='@';     //换消息头尾
                    gComInfo.COMMProtocol_Tail1='*';
                    gComInfo.COMMProtocol_Tail2='#';
                    LOG_E("Module308_Shakehand");
                    Module308_Shakehand();
                    gComInfo.Count++;
                }
                else    //其他功能头
                {
                    SPI_Send(0x7000);        //0V
                    PowerCtr_Main=POWER_OFF;     //关总电
                    PowerCtr_Light2=POWER_OFF;   //关mos
                    gComInfo.COMMProtocol_Head=0xAA;
                    gComInfo.COMMProtocol_Tail1=0xC3;
                    gComInfo.COMMProtocol_Tail2=0x3C;
                    LOG_E("ModuleRoutine_Shakehand");
                    ModuleRoutine_Shakehand();
                    gComInfo.Count+=2;
                }
                
                if(gComInfo.Count>10)
                {
                    gComInfo.Count=0;
                    gComInfo.ErrorCode=Error_NoModule;
                    gComInfo.HMI_LastScene=eScene_Error;
                    //HMI_Goto_Error();
                    LOG_E("Error_NoModule");
                    if (gComInfo.HMI_Scene==eScene_Wait)
                    {
                        HMI_Scene_Recovery();
                    }
                }
            }
            break;
        case eWS_CheckModule308:
            if (SystemTime1s)
            {
                SystemTime1s=0;
                LL_Module_Send("1*11&5&9",8);       //握手308
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
                    case eScene_Module_308:
                    case eScene_Module_308test:
//                        LL_Module_Send("1*15&0&0",8);
//                        LOG_E("Send 308 HeartBeat");
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
                    case eScene_Module_Wira:
                        //gComInfo.TempCount++;     //TODO:暂不开报警
                        Module_GetTemp();
                        break;
                    case eScene_Module_308:
                    case eScene_Module_308test:
                        LL_Module_Send("1*15&0&0",8);
                        LOG_E("Send 308 HeartBeat");
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
            PowerCtr_Light1=POWER_ON;
            break;
        case eScene_Module_Wira:
        {
            LL_Module_Send("\x39\x31\xff\x0",4);    //先关闭
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x01)
            {
                uint8_t xdata m_cmd[]={"\x39\x31\x1\x5"};
                m_cmd[3]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,4);     //打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x02)
            {
                uint8_t xdata m_cmd[]={"\x39\x31\x2\x5"};
                m_cmd[3]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,4);     //打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x04)
            {
                uint8_t xdata m_cmd[]={"\x39\x31\x3\x5"};
                m_cmd[3]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,4);     //打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x08)
            {
                uint8_t xdata m_cmd[]={"\x39\x31\x4\x5"};
                m_cmd[3]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,4);     //打开
            }
            PowerCtr_Main=POWER_ON;
            PowerCtr_Light1=POWER_ON; 
            Delay10ms();
            SPI_Send(0x7FFF);        //5V
        }
            break;
        case eScene_Module_4in1:
        {
            uint8_t xdata m_cmd[]={"\x39\x31\xff\x1\x5"};
            LL_Module_Send("\x39\x31\xff\xff\x0",5);    //先关闭
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x01)
            {
                m_cmd[3]=1;
                m_cmd[4]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,5);     //再打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x02)
            {
                m_cmd[3]=2;
                m_cmd[4]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,5);     //再打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x04)
            {
                m_cmd[3]=3;
                m_cmd[4]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,5);     //再打开
            }
            if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x08)
            {
                m_cmd[3]=4;
                m_cmd[4]=gInfo.ModuleInfo.New4in1Module.PowerLevel[0];
                LL_Module_Send(m_cmd,5);     //再打开
            }
            
            Delay10ms();
            SPI_Send(0x7FFF);        //5V
            PowerCtr_Main=POWER_ON;
            PowerCtr_Light1=POWER_ON; 
        }
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
            PowerCtr_Light1=POWER_OFF;
            break;
        case eScene_Module_Wira:
            LL_Module_Send("\x39\x31\xff\x0",4);
            PowerCtr_Light1=POWER_OFF;  //off
            PowerCtr_Main=POWER_OFF;    //off
            SPI_Send(0x7000);           //DAC 0V
            break;
        case eScene_Module_4in1:
            LL_Module_Send("\x39\x31\xff\xff\x0",5);
            PowerCtr_Light1=POWER_OFF;  //off
            PowerCtr_Main=POWER_OFF;    //off
            SPI_Send(0x7000);           //DAC 0V
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
                    if (((gInfo.ModuleInfo.RoutineModule.RemainTime)/60)==0 && 
                        ((gInfo.ModuleInfo.RoutineModule.RemainTime)%60)>45)    //15秒以内不计分钟
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
                {
                    int8_t usedtime=gInfo.ModuleInfo.RoutineModule.WorkTime-
                        (gInfo.ModuleInfo.RoutineModule.RemainTime)/60;  
                    if (((gInfo.ModuleInfo.RoutineModule.RemainTime)/60)==0 && 
                        ((gInfo.ModuleInfo.RoutineModule.RemainTime)%60)>45)    //15秒以内不计分钟
                    {
                        usedtime--;
                    }
                    if(usedtime>0)
                    {
                        uint8_t i;
                        for (i = 4; i > 0; i--)
                        {
                            gModuleSave.UsedCount[i]++;
                            if (gModuleSave.UsedCount[i]=='9'+1)
                            {
                                gModuleSave.UsedCount[i]='0';
                            }
                            else
                            {
                                break;
                            }
                        }
                        i=(usedtime%60);
                        gModuleSave.UsedTime[6]+=i%10;
                        if (gModuleSave.UsedTime[6]>'9')
                        {
                            gModuleSave.UsedTime[6]-=10;
                            gModuleSave.UsedTime[5]++;
                            if (gModuleSave.UsedTime[5]>'6')
                            {
                                gModuleSave.UsedTime[5]-=6;
                                gModuleSave.UsedTime[4]++;
                            }
                        }
                        gModuleSave.UsedTime[5]+=i/10;
                        if (gModuleSave.UsedTime[5]>'6')
                        {
                            gModuleSave.UsedTime[5]-=6;
                            gModuleSave.UsedTime[4]++;
                        }
                        gModuleSave.UsedTime[4]+=(usedtime/60);
                        for (i = 4; i > 0; i--)
                        {
                            if (gModuleSave.UsedTime[i]>'9')
                            {
                                gModuleSave.UsedTime[i]-=10;
                                gModuleSave.UsedTime[i-1]++;
                            }
                            else
                            {
                                break;
                            }
                        }
                        Save_ModuleSomething();
                        HMI_Show_IU_Usedtime();
                    }
                }
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

