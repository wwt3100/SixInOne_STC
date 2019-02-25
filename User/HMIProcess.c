#include "HMIProcess.h"
#include "WorkProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

uint8_t ErrorBeepTime=0;


void HMI_Process()
{
    switch (gComInfo.HMI_Scene)
    {
        case eScene_Startup:
        {
            if (SystemTime1s==1)
            {
                SystemTime1s=0;
                HMI_Shake_Hand();
                //LOG_E("Send Shakehand");
            }
            if (gComInfo.HMIMsg==eMsg_HMI_Shakehand)
            {
                gComInfo.HMIMsg=eMsg_NULL;
              #if !defined(_DEBUG)                //开机动画
                if (gComInfo.HMIArg1==0)
                {
                    HMI_Show_Logo();
                    gComInfo.WorkStat=eWS_CheckModuleStep1;
                }
                else if(gComInfo.HMIArg1==204)
                {
                    BeepEx(10-1);
                    gComInfo.HMI_Scene=eScene_StartPage;
                    HMI_Goto_LocPage(1);
                }
                else
                {
                    ;   //wait
                }
              #else                         //调试模式跳过动画
                if (gComInfo.HMIArg1==0)
                {
                    BeepEx(10-1);
                    gComInfo.HMI_Scene=eScene_StartPage;
                    HMI_Goto_LocPage(1);
                }
              #endif
            }
        }
            break;
        case eScene_StartPage:
            if (gComInfo.HMIMsg==eMsg_keyUp || gComInfo.HMIMsg==eMsg_KeyLongPush)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                if (gComInfo.HMIArg1==0)
                {
                    if (gComInfo.HMIArg2==1)        //开机界面圆按钮
                    {
                        if(gConfig.LANG==1)
                        {
                            HMI_Cut_PicEx(0x9C,60,221,186,508,315,258,272);
                        }
                        else
                        {
                            HMI_Cut_PicEx(0x9C,60,221,339,508,470,258,272);
                        }
                        gComInfo.HMI_Scene=eScene_Wait;
                        #if defined(_DEBUG)
                        gComInfo.WorkStat=eWS_CheckModuleStep1;
                        #endif
                    }
                    else if(gComInfo.HMIArg2==2)    //选择英文
                    {
                        gConfig.LANG=1;
                        HMI_Goto_LocPage(1);
                        Save_Config();
                    }
                    else if(gComInfo.HMIArg2==3)    //选择中文
                    {
                        gConfig.LANG=0;
                        HMI_Goto_LocPage(1);
                        Save_Config();
                    }
                    else
                    {
                        ;  //do nothing
                    }
                }
                else
                {
                    ; //do nothing
                }
                BeepEx(0);
            }
            break;
        case eScene_Wait:
            if (gComInfo.HMIMsg != eMsg_NULL)
            {
                gComInfo.HMIMsg=eMsg_NULL;      //清除按键消息
            }
            if (gComInfo.HMI_LastScene!=0)
            {
                HMI_Scene_Recovery();
            }
            break;
        case eScene_Module_308:
            break;
        case eScene_Module_IU:
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_UVA1:
            if (gComInfo.WorkStat==eWS_Working)
            {
                if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg1==0x01)
                {
                    if (gComInfo.HMIArg2==0x04)     
                    {
                        if(Fire_Flag==1)    
                        {
                            gComInfo.TimerCounter2=gComInfo.TimerCounter;   //保存定时器计数
                            WP_Stop(0);     //暂停
                        }
                        else
                        {
                            SystemTime1s=0;
                            gComInfo.TimerCounter=gComInfo.TimerCounter2;   //恢复计数
                            WP_Start();     //开始
                        }
                        BeepEx(0);
                        
                    }
                    else if(gComInfo.HMIArg2==0x05) //停止
                    {
                        BeepEx(0);
                        WP_Stop(1);
                    }
                    else
                    {
                        ;//do nothing
                    }
                }
                gComInfo.HMIMsg=eMsg_NULL;
            }
            else    //其他工作状态
            {
                if (gComInfo.HMIMsg==eMsg_keyUp)
                {
                    gComInfo.HMIMsg=eMsg_NULL;
                    if (gComInfo.HMIArg1==0x01)
                    {
                        switch (gComInfo.HMIArg2)
                        {
                            case 0x01:      //时间 加
                                if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                                {
                                    if (gInfo.ModuleInfo.RoutineModule.WorkTime<30)  //TODO:改回30
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime++;
                                    }
                                    else
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime=1;
                                    }
                                    HMI_Show_Worktime2();
                                }
                                else
                                {
                                    if (gInfo.ModuleInfo.RoutineModule.WorkTime<99)
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime++;
                                    }
                                    else
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime=1;
                                    }
                                    HMI_Show_Worktime1();
                                }
                                break;
                            case 0x02:      //时间 减
                                if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                                {
                                    if (gInfo.ModuleInfo.RoutineModule.WorkTime>1)
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime--;
                                    }
                                    else
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime=30;  //TODO:改回30
                                    }
                                    HMI_Show_Worktime2();
                                }
                                else
                                {
                                    if (gInfo.ModuleInfo.RoutineModule.WorkTime>1)
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime--;
                                    }
                                    else
                                    {
                                        gInfo.ModuleInfo.RoutineModule.WorkTime=99;
                                    }
                                    HMI_Show_Worktime1();
                                }
                                
                                break;
                            case 0x03:      //系统信息按钮(进入密码页)
                                HMI_Goto_LocPage(17);
                                gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                                gComInfo.HMI_Scene=eScene_Password;
                                break;
                            case 0x04:      //开始 
                                gInfo.ModuleInfo.RoutineModule.RemainTime=gInfo.ModuleInfo.RoutineModule.WorkTime*60;
                                WP_Start();
                                break;
                            case 0x05:      //停止        //非运行状态不做处理
                                WP_Stop(1);
                                break;
                            case 0x06:      //模式切换
                                if (gComInfo.HMI_Scene!=eScene_Module_UVA1 && gComInfo.HMI_Scene!=eScene_Module_IU)
                                {
                                    if(gInfo.ModuleInfo.RoutineModule.LightMode==0)
                                    {
                                        gInfo.ModuleInfo.RoutineModule.LightMode=1;
                                    }
                                    else
                                    {
                                        gInfo.ModuleInfo.RoutineModule.LightMode=0;
                                    }
                                    HMI_Show_WorkMode();
                                }
                                else
                                {
                                    ;   //UVA1和IU没有模式切换
                                }
                                break;
                            default:
                                break;
                        }
                        BeepEx(0);
                    }
                }
                else if(gComInfo.HMIMsg==eMsg_KeyLongPush)
                {
                    gComInfo.HMIMsg=eMsg_NULL;
                    if (gComInfo.HMIArg1==0x01)
                    {
                        if (gComInfo.HMIArg2 == 0x01)
                        {
                            if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                            {
                                if (gInfo.ModuleInfo.RoutineModule.WorkTime<30)
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime++;
                                }
                                else
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime=1;
                                }
                                HMI_Show_Worktime2();
                            }
                            else
                            {
                                if (gInfo.ModuleInfo.RoutineModule.WorkTime<99)
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime++;
                                }
                                else
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime=1;
                                }
                                HMI_Show_Worktime1();
                            }
                            BeepEx(0);
                        }
                        else if(gComInfo.HMIArg2 == 0x02)
                        {
                            if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                            {
                                if (gInfo.ModuleInfo.RoutineModule.WorkTime>1)
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime--;
                                }
                                else
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime=30;
                                }
                                HMI_Show_Worktime2();
                            }
                            else
                            {
                                if (gInfo.ModuleInfo.RoutineModule.WorkTime>1)
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime--;
                                }
                                else
                                {
                                    gInfo.ModuleInfo.RoutineModule.WorkTime=99;
                                }
                                HMI_Show_Worktime1();
                            }
                            BeepEx(0);
                        }
                    }
                }
                else
                {
                    ;//do nothing
                }
            }
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            if (gComInfo.WorkStat == eWS_Working)
            {
                if (gComInfo.HMIMsg == eMsg_keyUp)
                {
                    if (gComInfo.HMIArg1==0x08)
                    {
                        switch (gComInfo.HMIArg2)
                        {
                            case 0x01:      //暂停
                                break;
                            case 0x02:      //停止
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            else
            {
                if (gComInfo.HMIMsg == eMsg_keyUp)
                {
                    if (gComInfo.HMIArg1==0x08)
                    {
                        switch (gComInfo.HMIArg2)
                        {
                            case 0x4:       //选择的按钮
                            case 0x5:
                            case 0x6:
                            case 0x7:
                            case 0x8:
                            case 0x9:
                            case 0xA:
                            case 0xB:
                            case 0xC:
                            case 0xD:
                            case 0xE:
                            case 0xF:
                            case 0x10:
                            case 0x11:
                            case 0x12:
                            case 0x13:
                                HMI_New_Sel(gComInfo.HMIArg2);
                                break;
                            case 0x01:      //开始/暂停
                            default:
                                break;
                        }
                    }
                }
                else if (gComInfo.HMIMsg == eMsg_KeyLongPush)
                {
                    if (gComInfo.HMIArg1==0x08)
                    {
                        if (gComInfo.HMIArg2==0x14)
                        {
                            HMI_New_Add(gInfo.ModuleInfo.New4in1Module.ConfigSel);
                        }
                        else if(gComInfo.HMIArg2==0x15)
                        {
                            HMI_New_Dec(gInfo.ModuleInfo.New4in1Module.ConfigSel);
                        }
                    }
                }
            }
            break;
        case eScene_Error:      //密码错误在密码场景来处理
            if (ErrorBeepTime<3 && SystemTime100ms==1)
            {
                SystemTime100ms=0;
                ErrorBeepTime++;    
                BeepEx(0);
            }
            break;
        case eScene_Password:
            if (gComInfo.HMIMsg == eMsg_keyUp && gComInfo.HMIArg1==2)
            {
                BeepEx(0);
                switch (gComInfo.HMIArg2)
                {
                    case 1:         //错误提示,确认按钮
                        HMI_Goto_LocPage(17);   //重新进入密码页
                        break;
                    case 0xF0:      //退出
                        gInfo.PasswordLen=0;    //清密码
                        HMI_Scene_Recovery();
                        break;
                    case 0xF1:      //回车
                        gInfo.Password[gInfo.PasswordLen]=0;
                        if ((gInfo.DebugOpen&OPEN_DBG_Calib)             && strcmp(gInfo.Password,"666888")==0)     //进入调试模式
                        {
                            switch (gComInfo.ModuleType)    //该处根据治疗头类型来判断应解析执行的指令
                            {
                                case M_Type_UVA1: 
                                    gComInfo.HMI_Scene=eScene_DebugUVA1;
                                    HMI_Goto_Page(62);
                                    break;
                                case M_Type_633:  
                                case M_Type_633_1:
                                case M_Type_IU:   
                                    gComInfo.HMI_Scene=eScene_Debug;
                                    HMI_Goto_Page(62);
                                    break;
                                case M_Type_Wira: 
                                case M_Type_4in1: 
                                    gComInfo.HMI_Scene=eScene_DebugNew;
                                    HMI_Goto_Page(51);
                                    break;
                                //case M_Type_650:  //不需调试界面
                                default:
                                    HMI_Goto_LocPage(PAGE_PASSWORD_ERROR);
                                    break;
                            }
                        }
                        else if((gInfo.DebugOpen&OPEN_DBG_ClearUsedtime) && strcmp(gInfo.Password,"803803")==0) //清治疗头时间
                        {
                            gInfo.PasswordLen=0;    //清密码
                            HMI_Show_Password();
                            switch (gComInfo.ModuleType)    //该处根据治疗头类型来判断应解析执行的指令
                            {
                                case M_Type_UVA1: 
                                case M_Type_633:  
                                case M_Type_633_1:
                                case M_Type_Wira: 
                                case M_Type_4in1: 
                                    LL_Module_Send("\x39\xee\x00",3);
                                    break;
                                //case M_Type_650:  //不需调试界面
                                case M_Type_IU:   
                                case M_Type_308:
                                    LL_Module_Send("1*14&0&0",8);
                                    break;
                                default:
                                    HMI_Goto_LocPage(PAGE_PASSWORD_ERROR);
                                    break;
                            };
                        }
                        else if((gInfo.DebugOpen&OPEN_DBG_Config)        && strcmp(gInfo.Password,"308308")==0) //进入设置
                        {
                            switch (gComInfo.ModuleType)    //该处根据治疗头类型来判断应解析执行的指令
                            {
                                case M_Type_633:  
                                case M_Type_633_1:
                                case M_Type_IU:   
                                case M_Type_UVA1: 
                                    HMI_Goto_Page(62);
                                    break;
                                case M_Type_Wira: 
                                case M_Type_4in1: 
                                    HMI_Goto_Page(51);
                                    break;
                                case M_Type_650:  //不需调试界面
                                    break;
                                default:
                                    HMI_Goto_LocPage(PAGE_PASSWORD_ERROR);
                                    break;
                            }
                        }
                        else    //密码错误
                        {
                            HMI_Goto_LocPage(PAGE_PASSWORD_ERROR);
                        }
                        gInfo.PasswordLen=0;    //清密码
                        break;
                    case 0xF2:      //密码删除
                        if (gInfo.PasswordLen!=0)
                        {
                            gInfo.PasswordLen--;
                            HMI_Show_Password();
                        }
                        else
                        {
                            HMI_Scene_Recovery();   //密码为空,再按则退出
                        }
                        break;
                    case '0':
                    case '1': 
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if (gInfo.PasswordLen<8)
                        {
                            gInfo.Password[gInfo.PasswordLen]=gComInfo.HMIArg2;
                            gInfo.PasswordLen++;
                            HMI_Show_Password();
                        }
                        break;
                    default:
                        break;
                }
            }
            else if(gComInfo.HMIMsg == eMsg_KeyLongPush && gComInfo.HMIArg1==2 && gComInfo.HMIArg2==0xF2)
            {
                if(gInfo.PasswordLen!=0)
                {
                    BeepEx(0);
                    gInfo.PasswordLen=0;    //长按清空
                    HMI_Show_Password();
                }
            }
            else
            {
                ;//do nothing
            }
            gComInfo.HMIMsg = eMsg_NULL;    //统一清触摸指令
            break;
        case eScene_DebugUVA1:
            if (gComInfo.HMIMsg == eMsg_keyUp || gComInfo.HMIMsg == eMsg_KeyLongPush)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                if (gComInfo.HMIArg1 == 0x10)
                {
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x03:      //占空比 add
                        case 0x04:      //占空比 dec
                        case 0x05:      //dac add
                        case 0x06:      //dac dec
                        case 0x07:      //读占空比
                        case 0x08:      //读DA
                        case 0x09:      //输出/停止
                        case 0x0A:      //保存PWM
                        case 0x0B:      //保存DAC
                        case 0x0C:      //电源开关
                        case 0x0D:      //DAC 5V开关
                            BeepEx(0);
                        default:
                            break;
                    }
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x03:      //占空比 add
                        case 0x04:      //占空比 dec
                        case 0x05:      //dac add
                        case 0x06:      //dac dec
                        case 0x07:      //读占空比
                            break;
                        case 0x08:      //读DA
                            ModuleRoutine_GetCalibData();
                            break;
                        case 0x09:      //输出/停止
                        case 0x0A:      //保存PWM
                        case 0x0B:      //保存DAC
                        case 0x0C:      //电源开关
                        case 0x0D:      //DAC 5V开关
                        default:
                            break;
                    }
                }
                else if(gComInfo.HMIArg1 == 0x03 && gComInfo.HMIArg2 == 0x03) //返回/退出
                {
                    BeepEx(0);
                    HMI_Scene_Recovery();
                }
            }
            break;
        case eScene_Debug:  
            if (gComInfo.HMIMsg == eMsg_keyUp)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                if (gComInfo.HMIArg1 == 0x10)
                {
                    switch (gComInfo.HMIArg2)       //蜂鸣器声音
                    {
                        case 0x05:      //dac add
                        case 0x06:      //dac dec
                        case 0x08:      //读DA
                        case 0x09:      //输出/停止
                        case 0x0B:      //保存DAC
                        case 0x0C:      //电源开关
                        case 0x0D:      //DAC 5V开关
                            BeepEx(0);
                        default:
                            break;
                    }
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x05:      //dac add
                            if (gInfo.Debug.dac<50)
                            {
                                gInfo.Debug.dac++;
                            }
                            else
                            {
                                gInfo.Debug.dac=0;
                            }
                            HMI_DGB_Show_DAval();
                            break;
                        case 0x06:      //dac dec
                            if (gInfo.Debug.dac>0)
                            {
                                gInfo.Debug.dac--;
                            }
                            else
                            {
                                gInfo.Debug.dac=50;
                            }
                            HMI_DGB_Show_DAval();
                            break;
                        case 0x08:      //读DA
                            ModuleRoutine_GetCalibData();
                            break;
                        case 0x09:      //输出/停止
                            if (Fire_Flag)
                            {
                                WP_Stop(1);
                            }
                            else
                            {
                                WP_Start();
                            }
                            break;
                        case 0x0B:      //保存DAC
                            break;
                        case 0x0C:      //电源开关
                            if (Dbg_Flag_MainPower)
                            {
                                PowerCtr_Main=POWER_OFF;
                                Dbg_Flag_MainPower=0;
                                HMI_Cut_Pic(0x71,62, 63, 505, 63+144, 505+86);
                            }
                            else
                            {
                                PowerCtr_Main=POWER_ON;
                                Dbg_Flag_MainPower=1;
                                HMI_Cut_Pic(0x71,63, 63, 505, 63+144, 505+86);
                            }
                            break;
                        case 0x0D:      //DAC 5V开关
                            if (Dbg_Flag_DAC5V)
                            {
                                HMI_Cut_Pic(0x71,63, 225, 504, 225+149, 504+69);
                                SPI_Send(0x7000);
                                Dbg_Flag_DAC5V=0;
                            }
                            else
                            {
                                HMI_Cut_Pic(0x71,63, 225, 504, 225+149, 504+69);
                                SPI_Send(0x7FFF);
                                Dbg_Flag_DAC5V=1;
                            }
                        default:
                            break;
                    }
                }
                else if(gComInfo.HMIArg1 == 0x03 && gComInfo.HMIArg2 == 0x03) //返回/退出
                {
                    BeepEx(0);
                    HMI_Scene_Recovery();
                }
            }
            else if(gComInfo.HMIMsg == eMsg_KeyLongPush)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                if (gComInfo.HMIArg1 == 0x10)
                {
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x05:      //dac add
                            BeepEx(0);
                            if (gInfo.Debug.dac<50)
                            {
                                gInfo.Debug.dac++;
                            }
                            else
                            {
                                gInfo.Debug.dac=0;
                            }
                            HMI_DGB_Show_DAval();
                            
                            break;
                        case 0x06:      //dac dec
                            BeepEx(0);
                            if (gInfo.Debug.dac>0)
                            {
                                gInfo.Debug.dac--;
                            }
                            else
                            {
                                gInfo.Debug.dac=50;
                            }
                            HMI_DGB_Show_DAval();
                            
                            break;
                        default:
                            break;
                    }
                }
            }
            break;
        default:
           break;
    }

}

//HMI场景还原,用于出错/设置后场景还原
void HMI_Scene_Recovery()
{
    switch (gComInfo.HMI_LastScene)
    {
        case eScene_Module_650:
            gComInfo.HMI_Scene=eScene_Module_650;
            HMI_Goto_LocPage(2);
            HMI_Show_ModuleName("Derma-650");
            HMI_Show_WorkMode();
            HMI_Show_Worktime1();
            ModuleRoutine_GetUsedTime();
            HMI_Show_Power();
            break;
        case eScene_Module_633:
            gComInfo.HMI_Scene=eScene_Module_633;
            HMI_Goto_LocPage(3);
            HMI_Show_ModuleName("Derma-633");
            HMI_Show_WorkMode();
            HMI_Show_Worktime1();
            ModuleRoutine_GetUsedTime();
            HMI_Show_Power();
            ModuleRoutine_GetCalibData();
            break;
        case eScene_Module_IU:     
            gComInfo.HMI_Scene=eScene_Module_IU;
            HMI_Goto_LocPage(6);
            HMI_Show_ModuleName("Derma-IU");
            HMI_Show_Worktime1();
            break;
        case eScene_Module_UVA1:
            gComInfo.HMI_Scene=eScene_Module_UVA1;
            HMI_Goto_LocPage(4);
            HMI_Show_ModuleName("Derma-UVA1");
            HMI_Show_Worktime1();
            ModuleRoutine_GetUsedTime();
            HMI_Show_Power();
            ModuleRoutine_GetCalibData();
            break;
        case eScene_Module_Wira:
            gComInfo.HMI_Scene=eScene_Module_Wira;
            HMI_Goto_LocPage(45);       //TODO: 现测试先进专家模式
            HMI_Cut_PicEx(0x71,61,0,11,399,96,0,0);     //显示名称
        case eScene_Module_4in1:
            gComInfo.HMI_Scene=eScene_Module_Wira;
            HMI_Goto_LocPage(45);       //TODO: 现测试先进专家模式
            HMI_Cut_PicEx(0x71,61,0,111,399,196,0,0);     //显示名称

        case eScene_Error:
            HMI_Goto_Error();
            break;
        default:        //其他没考虑的情况一律返回开机页面
            gComInfo.HMI_Scene=eScene_StartPage;
            HMI_Goto_LocPage(1);
            break;
    }
}
void HMI_Goto_Error()
{
    if (gComInfo.ErrorCode==0)
    {
        return;
    }
    else
    {
        gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
        gComInfo.HMI_Scene=eScene_Error;
        if (gComInfo.ErrorCode==Error_NoModule)
        {
            gInfo.DebugOpen=0;      //拔头之后清授权
            HMI_Goto_LocPage(35);
        }
        else if(gComInfo.ErrorCode==Error_PasswordError)
        {
            HMI_Goto_LocPage(36);
        }
        else
        {
            gComInfo.WorkStat=eWS_Idle;
            HMI_Goto_Page(37);
            HMI_Show_ErrorStr();
        }
    }
}
void HMI_Exit_Error()
{
    ErrorBeepTime=0;
}


