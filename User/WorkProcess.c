#include "WorkProcess.h"
#include "HMIProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

void Work_Process()
{
    int16_t temp=0;
    switch (gCom.WorkStat)
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
                    gCom.COMMProtocol_Head='@';     //换消息头尾
                    gCom.COMMProtocol_Tail1='*';
                    gCom.COMMProtocol_Tail2='#';
                    LOG_E("Module308_Shakehand");
                    Module308_Shakehand();
                    gCom.Count++;
                }
                else    //其他功能头
                {
                    SPI_Send(0x7000);        //0V
                    PowerCtr_Main=POWER_OFF;     //关总电
                    PowerCtr_Light2=POWER_OFF;   //关mos
                    gCom.COMMProtocol_Head=0xAA;
                    gCom.COMMProtocol_Tail1=0xC3;
                    gCom.COMMProtocol_Tail2=0x3C;
                    LOG_E("ModuleRoutine_Shakehand");
                    ModuleRoutine_Shakehand();
                    gCom.Count+=2;
                }
                
                if(gCom.Count>10)
                {
                    gCom.Count=0;
                    gCom.ErrorCode=Error_NoModule;
                    gCom.HMI_LastScene=eScene_Error;
                    //HMI_Goto_Error();
                    LOG_E("Error_NoModule");
                    if (gCom.HMI_Scene==eScene_Wait)
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
                gCom.Count=0;
                switch (gCom.HMI_Scene)
                {
                    case eScene_Module_650:
                    case eScene_Module_633:
                    case eScene_Module_IU:
                    case eScene_Module_UVA1:
                        if (gInfo.ModuleInfo.Routine.RemainTime>1)
                        {
                            gInfo.ModuleInfo.Routine.RemainTime--;
                            HMI_Show_RemainTime();
                        }
                        else
                        {
                            BeepEx(3,3);
                            WP_Stop(1);     //Timeout Stop
                        }
                        break;
                    case eScene_Module_308test:
                        gInfo.ModuleInfo.mini308.TotalTime++;     //红斑测试总时间
                        HMI_308Test_AllPower();
                    case eScene_Module_308:
                        LL_Module_Send("1*15&0&0",8);   //308工作心跳
                        gInfo.ModuleInfo.mini308.OnceWorkTime++;
                        if(gInfo.ModuleInfo.mini308.RemainTime>1)
                        {
                            gInfo.ModuleInfo.mini308.RemainTime--;
                            HMI_Show_308RemainTime();
                        }
                        else
                        {
                            BeepEx(3,3);
                            WP_Stop(1);     //Timeout Stop
                        }
                        break;
                    case eScene_Module_4in1:
                    case eScene_Module_Wira:
                    {
                        uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
                        if(gInfo.ModuleInfo.New4in1.RemainTime>1)
                        {
                            gInfo.ModuleInfo.New4in1.RemainTime--;
                            HMI_New_ShowDetail(1);
                        }
                        else    //timeout
                        {
                            if (gInfo.ModuleInfo.New4in1.LightStep[light_group].StepMode==STEP_MODE_Serial)    //顺序
                            {
                                gInfo.ModuleInfo.New4in1.LocStep++;
                                if (gInfo.ModuleInfo.New4in1.LocStep >= gInfo.ModuleInfo.New4in1.LightStep[light_group].StepNum)
                                {
                                    BeepEx(3,3);
                                    WP_Stop(1);     //Timeout Stop
                                }
                                else
                                {
                                    gInfo.ModuleInfo.New4in1.RemainTime=
                                        gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[gInfo.ModuleInfo.New4in1.LocStep*3+2] MIN2S;
                                    BeepEx(2,2);
                                    WP_Start();
                                }
                            }
                            else    //同步模式只有一步
                            {
                                BeepEx(3,3);
                                WP_Stop(1);     //Timeout Stop
                            }
                            
                        }
                        
                    }
                        break;
                    default:
                        break;
                }
            }
            if (SystemTime100ms==1 && Fire_MaxOut==0)   //慢启动
            {
                switch (gCom.HMI_Scene)
                {
                    case eScene_Module_633:
                        gInfo.ModuleInfo.Routine.DAC_Val+=10;
                        if (gInfo.ModuleInfo.Routine.DAC_Val>gInfo.ModuleInfo.Routine.DAC_Cail)
                        {
                            gInfo.ModuleInfo.Routine.DAC_Val=gInfo.ModuleInfo.Routine.DAC_Cail;
                            Fire_MaxOut=1;
                        }
                        SPI_Send(gInfo.ModuleInfo.Routine.DAC_Val);
                        break;
                    default:
                        break;
                }
            }
        case eWS_Standby:
            if(SystemTime1s==1)
            {
                SystemTime1s=0;
                switch (gCom.HMI_Scene)
                {
                    case eScene_Module_650:
                    case eScene_Module_633:
                    {
                        uint8_t ret=DS18B20_GetTemp(&temp);
                        if(ret==1)  //DS18B20不在
                        {
                            gCom.TempCount+=2;
                        }
                        else if(ret==2)
                        {
                            gCom.TempCount++;
                        }
                        else
                        {
                            gCom.TempCount=0;
                            HMI_Show_Temp(temp);
                        }                        
                    }
                        break;
                    case eScene_Module_UVA1:
                    case eScene_Module_Wira:
                        //gCom.TempCount++;     //TODO:暂不开报警
                        Module_GetTemp();
                        break;
                    case eScene_Module_IU:
                        if (ADConvertDone)          //读回Vfb
                        {
                            ADConvertDone=0;
                            HMI_Show_IU_Temp();
                            ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | 0x01 ;
                        }
                        break;
                    case eScene_Module_308:
                    case eScene_Module_308test:

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
                    if(gCom.Count++>10 && temp<4500)    //关闭之后继续吹10秒再关
                    {
                        FAN_IO=DISABLE;
                    }
                }
                if (gCom.TempCount>10)   //温度传感器错误
                {
                    gCom.ErrorCode=Error_TempSenerError;
                    HMI_Goto_Error();
                }
                
            }
            if (SystemTime100ms==1)     //重发
            {
                SystemTime100ms=0;
                if (Resend_getUsedtime)
                {
                    //LOG_E("ReSend GetUsedTime");
                    ModuleRoutine_GetUsedTime();
                }
                if (Resend_getCalibData)
                {
                    ModuleRoutine_GetCalibData();
                }
            }
            break;
        default:
            break;
    }
}

void WP_Start()
{
    switch (gCom.HMI_Scene)     //准备
    {
        case eScene_Module_633:
        //case eScene_Module_UVA1:      //TODO: 暂不判断UVA1校准值
            if (gInfo.ModuleInfo.Routine.DAC_Cail==0)  //没有DA值直接返回
            {
                return ;
            }
            break;
        default:
            break;
    }
    FAN_IO=ENABLE;
    switch (gCom.HMI_Scene)     //显示部分
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
            HMI_New_ShowDetail(1); //倒计时
            break;
        default:
            break;
    }
    switch (gCom.HMI_Scene)     //工作部分
    {
        case eScene_Module_650:
            PowerCtr_Module12v=POWER_ON;
            break;
        case eScene_Module_633:
            //SPI_Send(30310);        //2V
            gInfo.ModuleInfo.Routine.DAC_Val=0x7000|2457;   //3V
            if (gInfo.ModuleInfo.Routine.LightMode==0)
            {
                SPI_Send(gInfo.ModuleInfo.Routine.DAC_Val);   //从3V开始
                Fire_MaxOut=0;
            }
            else
            {
                SPI_Send(gInfo.ModuleInfo.Routine.DAC_Cail);  //直接输出最大
                Fire_MaxOut=1;
            }
            
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
            uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
            bit step_mode=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepMode;
            uint8_t xdata cmd[]={"\x39\x31\x2\x5"};
            if (step_mode==STEP_MODE_Serial)   //顺序模式
            {
                uint8_t loc_step=gInfo.ModuleInfo.New4in1.LocStep;
                uint8_t light=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[loc_step*3];
                uint8_t power=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[loc_step*3+1];   
                if (power==100)
                {
                    power=99;
                }
                //TODO:根据PowerMaxMin计算0-100%的能量再发送
                LL_Module_Send("\x39\x31\xff\x0",4);    //先关闭
                if (light&0x01)
                {
                    cmd[2]=1;
                    cmd[3]=power;
                    LL_Module_Send(cmd,4);     //再打开
                }
                else if (light&0x02)
                {
                    cmd[2]=2;
                    cmd[3]=power;
                    LL_Module_Send(cmd,4);     //再打开
                }
                else if (light&0x04)
                {
                    cmd[2]=3;
                    cmd[3]=power;
                    LL_Module_Send(cmd,4);  
                }
                else if (light&0x08)
                {
                    cmd[2]=4;
                    cmd[3]=power;
                    LL_Module_Send(cmd,4);  
                }
            }
            else    //同步模式
            {
                uint8_t i;
                uint8_t light_num=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepNum;
                LL_Module_Send("\x39\x31\xff\x0",4);    //先关闭
                for (i = 0; i < light_num; i++)
                {
                    uint8_t light=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3];
                    uint8_t power=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3+1];   
                    //TODO:根据PowerMaxMin计算0-100%的能量再发送
                    if (power==100)
                    {
                        power=99;
                    }
                    if (light&0x01)
                    {
                        cmd[2]=1;
                        cmd[3]=power;
                        LL_Module_Send(cmd,4);     //再打开
                    }
                    else if (light&0x02)
                    {
                        cmd[2]=2;
                        cmd[3]=power;
                        LL_Module_Send(cmd,4);     //再打开
                    }
                    else if (light&0x04)
                    {
                        cmd[2]=3;
                        cmd[3]=power;
                        LL_Module_Send(cmd,4);  
                    }
                    else if (light&0x08)
                    {
                        cmd[2]=4;
                        cmd[3]=power;
                        LL_Module_Send(cmd,4);  
                    }
                }
            }
            
            
            PowerCtr_Main=POWER_ON;
            PowerCtr_Light1=POWER_ON; 
            Delay10ms();
            SPI_Send(0x7FFF);        //5V
        }
            break;
        case eScene_Module_4in1:
        {
            uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
            bit step_mode=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepMode;
            uint8_t xdata cmd[]={"\x39\x31\xff\x1\x5"};
            if (step_mode==STEP_MODE_Serial)   //顺序模式
            {
                uint8_t loc_step=gInfo.ModuleInfo.New4in1.LocStep;
                uint8_t light=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[loc_step*3];
                uint8_t power=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[loc_step*3+1];   
                //TODO:根据PowerMaxMin计算0-100%的能量再发送
                LL_Module_Send("\x39\x31\xff\xff\x0",5);    //先关闭
                if (light&0x01)
                {
                    cmd[3]=1;
                    cmd[4]=power;
                    LL_Module_Send(cmd,5);     //再打开
                }
                else if (light&0x02)
                {
                    cmd[3]=2;
                    cmd[4]=power;
                    LL_Module_Send(cmd,5);     //再打开
                }
                else if (light&0x04)
                {
                    cmd[3]=3;
                    cmd[4]=power;
                    LL_Module_Send(cmd,5);  
                }
                else if (light&0x08)
                {
                    cmd[3]=4;
                    cmd[4]=power;
                    LL_Module_Send(cmd,5);  
                }
            }
            else    //同步模式
            {
                uint8_t i;
                uint8_t light_num=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepNum;
                LL_Module_Send("\x39\x31\xff\xff\x0",5);    //先关闭
                for (i = 0; i < light_num; i++)
                {
                    uint8_t light=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3];
                    uint8_t power=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3+1];   
                    //TODO:根据PowerMaxMin计算0-100%的能量再发送
                    
                    if (light&0x01)
                    {
                        cmd[3]=1;
                        cmd[4]=power;
                        LL_Module_Send(cmd,5);     //再打开
                    }
                    else if (light&0x02)
                    {
                        cmd[3]=2;
                        cmd[4]=power;
                        LL_Module_Send(cmd,5);     //再打开
                    }
                    else if (light&0x04)
                    {
                        cmd[3]=3;
                        cmd[4]=power;
                        LL_Module_Send(cmd,5);  
                    }
                    else if (light&0x08)
                    {
                        cmd[3]=4;
                        cmd[4]=power;
                        LL_Module_Send(cmd,5);  
                    }
                }
            }
            
            Delay10ms();
            SPI_Send(0x7FFF);        //5V
            PowerCtr_Main=POWER_ON;
            PowerCtr_Light1=POWER_ON; 
        }
            break;
        case eScene_Module_308:
        case eScene_Module_308test:
            LL_Module_Send("1*7&1&0",7);    //发送亮灯指令
            break;
        case eScene_Debug:
            switch (gCom.ModuleType)
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
    gCom.WorkStat=eWS_Working;
    gCom.TimerCounter=0;
    Fire_Flag=1;
}

void WP_Stop(uint8_t stop_type)
{
    Fire_Flag=0;
    switch (gCom.HMI_Scene)     //工作部分
    {
        case eScene_Module_650:
            PowerCtr_Module12v=POWER_OFF;
            break;
        case eScene_Module_633:
            PowerCtr_Light1=POWER_OFF;  //off
            PowerCtr_Main=POWER_OFF;    //off
            SPI_Send(0x7000);           //DAC 0V
            Fire_MaxOut=0;
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
        case eScene_Module_308:
        case eScene_Module_308test:
        {
            uint8_t xdata save_worktime[]={"1*13&0&0&00"};
            LL_Module_Send("1*7&0&0",7);    //308停止指令
            save_worktime[7]=gInfo.ModuleInfo.mini308.OnceWorkTime/60+'0';
            gInfo.ModuleInfo.mini308.OnceWorkTime=gInfo.ModuleInfo.mini308.OnceWorkTime%60;
            save_worktime[9]=gInfo.ModuleInfo.mini308.OnceWorkTime/10+'0';
            save_worktime[10]=gInfo.ModuleInfo.mini308.OnceWorkTime%10+'0';
            LL_Module_Send(save_worktime,11);    //保存使用时间
            gCom.WorkStat=eWS_Standby;
            gInfo.ModuleInfo.mini308.OnceWorkTime=0;  //停止时,单次工作时长清零
        }
            break;
        case eScene_Debug:
            switch (gCom.ModuleType)
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
        switch (gCom.HMI_Scene)     //保存使用时间
        {
            case eScene_Module_650:
            case eScene_Module_633:    
            case eScene_Module_UVA1:            
                {
                    uint8_t xdata cmd[]={"\x39\x26\x00"};
                    int8_t usedtime=gInfo.ModuleInfo.Routine.WorkTime-(gInfo.ModuleInfo.Routine.RemainTime)/60;  
                    if (((gInfo.ModuleInfo.Routine.RemainTime)/60)==0 && ((gInfo.ModuleInfo.Routine.RemainTime)%60)>45)    //15秒以内不计分钟
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
                    int8_t usedtime=gInfo.ModuleInfo.Routine.WorkTime-(gInfo.ModuleInfo.Routine.RemainTime)/60;  
                    if (((gInfo.ModuleInfo.Routine.RemainTime)/60)==0 && ((gInfo.ModuleInfo.Routine.RemainTime)%60)>45)    //15秒以内不计分钟
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
    switch (gCom.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 618, 411, 618+139, 411+74);      //按钮切开始
            if (stop_type==1)
            {
                HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 556, 145, 556+215, 145+264);     //切回时间显示
                HMI_Show_Worktime1();
                gCom.WorkStat=eWS_Standby;
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
                gCom.WorkStat=eWS_Standby;
            }
            else    //暂停
            {
                ;// do nothing
            }
            break;
        case eScene_Debug:
            HMI_Cut_Pic(0x71,62, 655, 247, 655+123, 247+78);     //按钮切暂停
            gCom.WorkStat=eWS_Standby;
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 45, 652, 504, 652+112, 504+72);     //按钮切暂停
            if (stop_type==1)
            {
                HMI_New_ShowDetail(0);
            }
            else //暂停
            {
                ;//do nothing
            }
            gCom.WorkStat=eWS_Standby;
            FAN_IO=DISABLE;
            break;
        case eScene_Module_308:
            if (Pause_Flag==1)
            {
                ;//do nothing
            }
            else
            {
                HMI_Cut_Pic(0x71,ePage_Module308, 162, 409, 162+464, 407+33);     //进度条背景恢复
            }
            break;
        case eScene_Module_308test:
            if (Pause_Flag==0)
            {
                HMI_Cut_Pic(0x71,ePage_Module308Test, 178, 399, 178+440, 399+39);     //进度条背景恢复
            }
            HMI_308Test_AllPower();
            break;
        default:
            break;
    }
    
    
    //FAN_IO=DISABLE;   //根据温度情况自动关闭
}

