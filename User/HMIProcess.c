#include "HMIProcess.h"
#include "WorkProcess.h"
#include "main.h"
#include "HMI_COMM.h"
#include "Module_COMM.h"

uint8_t ErrorBeepTime=0;

uint8_t HMI_Count=0;


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
                if (gComInfo.HMIArg1==ePage_Startup)
                {
                    HMI_Show_Logo();
                    gComInfo.WorkStat=eWS_CheckModule;
                }
                else if(gComInfo.HMIArg1==ePage_LogoEnd)
                {
                    Beep(10);
                    gComInfo.HMI_Scene=eScene_StartPage;
                    HMI_Goto_LocPage(ePage_Main);
                }
                else
                {
                    ;   //wait
                }
              #else                         //调试模式跳过动画
                if (gComInfo.HMIArg1==ePage_Startup)
                {
                    Beep(10);
                    gComInfo.HMI_Scene=eScene_StartPage;
                    HMI_Goto_LocPage(ePage_Main);
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
                        if(gConfig.LANG==LANG_EN)
                        {
                            HMI_Cut_PicEx(0x9C,60,221,186,508,315,258,272);
                        }
                        else
                        {
                            HMI_Cut_PicEx(0x9C,60,221,339,508,470,258,272);
                        }
                        gComInfo.HMI_Scene=eScene_Wait;
                        #if defined(_DEBUG)
                        gComInfo.WorkStat=eWS_CheckModule;
                        #endif
                    }
                    else if(gComInfo.HMIArg2==2)    //选择英文
                    {
                        gConfig.LANG=LANG_EN;
                        HMI_Goto_LocPage(ePage_Main);
                        Save_Config();
                    }
                    else if(gComInfo.HMIArg2==3)    //选择中文
                    {
                        gConfig.LANG=LANG_ZH;
                        HMI_Goto_LocPage(ePage_Main);
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
                Beep(1);
            }
            break;
        case eScene_Wait:
            if (gComInfo.HMIMsg != eMsg_NULL)
            {
                gComInfo.HMIMsg=eMsg_NULL;      //清除按键消息
            }
            if (gComInfo.HMI_LastScene != eScene_Startup)
            {
                HMI_Scene_Recovery();
            }
            break;
        case eScene_Module_308Wait:
            if (SystemTime1s_1==1)
            {
                SystemTime1s_1=0;
                if(HMI_Count++>1)   //延时5秒
                {
                    HMI_Count=0;
                    gComInfo.HMI_LastScene=eScene_Module_308;
                    HMI_Scene_Recovery();
                    //LOG_E("Goto eScene_Module_308");
                }
            }
            break;
        case eScene_Module_308:         //308从场景按键来说无差异,用于区分显示
        case eScene_Module_308test:
            if (gComInfo.WorkStat==eWS_Working)
            {
                if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg1==eKeyCode_Group1)
                {
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x31:      //开始
                            //TODO:提示已经开始
                            break;
                        case 0x32:      //暂停
                            gComInfo.TimerCounter2=gComInfo.TimerCounter;   //保存定时器计数
                            Pause_Flag=1;
                            WP_Stop(0);     //暂停
                            Beep(1);
                            break;
                        case 0x05:      //停止
                            Pause_Flag=0;
                            Beep(1);
                            WP_Stop(1);
                            if (gComInfo.HMI_Scene==eScene_Module_308test)
                            {
                                HMI_Cut_Pic(0x71,ePage_Module308Test, 178, 399, 178+440, 399+39);     //进度条背景恢复
                                gInfo.ModuleInfo.mini308Module.TotalTime=0;
                                HMI_308Test_AllPower();
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    ;//do nothing
                }
            }
            else
            {
                if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg1==eKeyCode_Group1)
                {
                    Beep(1);
                    switch (gComInfo.HMIArg2)
                    {
                        case 0x02:      //加
                            if (gInfo.ModuleInfo.mini308Module.WorkTime<MODULE308_MAX_WORKTIME)
                            {
                                gInfo.ModuleInfo.mini308Module.WorkTime++;
                            }
                            else
                            {
                                gInfo.ModuleInfo.mini308Module.WorkTime=1;
                            }
                            HMI_Show_308WorkTime();
                            Pause_Flag=0;
                            break;
                        case 0x01:      //减
                            if (gInfo.ModuleInfo.mini308Module.WorkTime<=1)
                            {
                                gInfo.ModuleInfo.mini308Module.WorkTime=MODULE308_MAX_WORKTIME;
                            }
                            else
                            {
                                gInfo.ModuleInfo.mini308Module.WorkTime--;
                            }
                            HMI_Show_308WorkTime();
                            Pause_Flag=0;
                            break;
                        case eKeyCodeG1_SysInfo:        //进入密码输入页
                            HMI_Goto_LocPage(ePage_PasswordInput);
                            gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                            gComInfo.HMI_Scene=eScene_Password;
                            Pause_Flag=0;
                            break;
                        case eKeyCodeG1_Stop:
                            if (gComInfo.HMI_Scene==eScene_Module_308test)
                            {
                                HMI_Cut_Pic(0x71,ePage_Module308Test, 178, 399, 178+440, 399+39);     //进度条背景恢复
                                gInfo.ModuleInfo.mini308Module.TotalTime=0;
                                HMI_308Test_AllPower();
                            }
                            else
                            {
                                HMI_Cut_Pic(0x71,ePage_Module308, 162, 409, 162+464, 407+33);     //进度条背景恢复
                            }
                            Pause_Flag=0;
                        case 0x32:              //暂停
                            //TODO: 提示尚未开始
                            //LL_Module_Send("1*7&0&0",7); 
                            break;
                        case 0x31:              //开始
                            if (Pause_Flag==1)
                            {
                                gComInfo.TimerCounter=gComInfo.TimerCounter2;
                                SystemTime1s=0;
                                Pause_Flag=0;
                            }
                            else
                            {
                                if (gComInfo.HMI_Scene==eScene_Module_308)
                                {
                                    gInfo.ModuleInfo.mini308Module.RemainTime=gInfo.ModuleInfo.mini308Module.WorkTime;
                                }
                                else
                                {
                                    gInfo.ModuleInfo.mini308Module.RemainTime=gInfo.ModuleInfo.mini308Module.TestWorkTime;
                                }
                            }
                            WP_Start();
                            break;
                        case 0x33:              //进入红斑测试
                            gComInfo.HMI_LastScene=eScene_Module_308test;
                            HMI_Scene_Recovery();
                            Pause_Flag=0;
                            break;
                        case 0x34:              //退出红斑测试
                            gComInfo.HMI_LastScene=eScene_Module_308;
                            gInfo.ModuleInfo.mini308Module.TestSelTime=3;
                            HMI_Scene_Recovery();
                            Pause_Flag=0;
                            break;
                        case 0x35:              //红斑3s
                            HMI_308Test_SelTime(3);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=3;
                            Pause_Flag=0;
                            break;
                        case 0x36:              //红斑6s
                            HMI_308Test_SelTime(6);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=6;
                            Pause_Flag=0;
                            break;
                        case 0x37:              //红斑9s
                            HMI_308Test_SelTime(9);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=9;
                            Pause_Flag=0;
                            break;
                        case 0x38:              //红斑12s
                            HMI_308Test_SelTime(12);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=12;
                            Pause_Flag=0;
                            break;
                        case 0x39:              //红斑15s
                            HMI_308Test_SelTime(15);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=15;
                            Pause_Flag=0;
                            break;
                        case 0x3A:              //红斑18s
                            HMI_308Test_SelTime(18);
                            gInfo.ModuleInfo.mini308Module.TestWorkTime=18;
                            Pause_Flag=0;
                            break;
                        case 0x3C:              //进入IU界面
                            memset(&gInfo,0,sizeof(_Golbal_Info));  //清治疗头数据
                            gComInfo.WorkStat=eWS_Standby;
                            gComInfo.ModuleType=M_Type_IU;
                            gComInfo.HMI_LastScene=eScene_Module_IU;
                            gInfo.ModuleInfo.RoutineModule.WorkTime=10;
                            HMI_Scene_Recovery();
                            Pause_Flag=0;
                            break;
                        default:
                            break;
                    }
                }
                else if(gComInfo.HMIMsg==eMsg_KeyLongPush && gComInfo.HMIArg1==eKeyCode_Group1)
                {
                    
                    if (gComInfo.HMIArg2 == 0x02)   //加
                    {
                        Beep(1);
                        if (gInfo.ModuleInfo.mini308Module.WorkTime<MODULE308_MAX_WORKTIME)
                        {
                            gInfo.ModuleInfo.mini308Module.WorkTime++;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.WorkTime=1;
                        }
                        HMI_Show_308WorkTime();
                    }
                    else if(gComInfo.HMIArg2 == 0x01)   //减
                    {
                        Beep(1);
                        if (gInfo.ModuleInfo.mini308Module.WorkTime<=1)
                        {
                            gInfo.ModuleInfo.mini308Module.WorkTime=MODULE308_MAX_WORKTIME;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.WorkTime--;
                        }
                        HMI_Show_308WorkTime();
                    }
                    else
                    {
                        ;//do nothing
                    }
                }
                else
                {
                    ;//do nothing
                }
            }
            gComInfo.HMIMsg=eMsg_NULL; 
            break;
        case eScene_Module_IU:
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_UVA1:
            if (gComInfo.WorkStat==eWS_Working)
            {
                if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg1==eKeyCode_Group1)
                {
                    if (gComInfo.HMIArg2==eKeyCodeG1_StartPause)     
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
                        Beep(1);
                        
                    }
                    else if(gComInfo.HMIArg2==eKeyCodeG1_Stop) //停止
                    {
                        Beep(1);
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
                    if (gComInfo.HMIArg1==eKeyCode_Group1)
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
                                HMI_Goto_LocPage(ePage_PasswordInput);
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
                            case 0x3B:      //切换到308模式
                                HMI_Goto_LocPage(ePage_Loading308);    //等待界面
                                gComInfo.WorkStat=eWS_CheckModule308;
                                break;
                            default:
                                break;
                        }
                        Beep(1);
                    }
                }
                else if(gComInfo.HMIMsg==eMsg_KeyLongPush)
                {
                    gComInfo.HMIMsg=eMsg_NULL;
                    if (gComInfo.HMIArg1==eKeyCode_Group1)
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
                            Beep(1);
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
                            Beep(1);
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
                                Beep(1);
                                WP_Stop(0);
                                break;
                            case 0x02:      //停止
                                Beep(1);
                                WP_Stop(1);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            else    //workstat not in eWS_Working
            {
                if (gComInfo.HMIMsg == eMsg_keyUp)
                {
                    if (gComInfo.HMIArg1==0x08)
                    {
                        switch (gComInfo.HMIArg2)       //Beep  比if效率高
                        {
                            case 0x01:      //开始/暂停
                            case 0x02:      
                            case 0x03:
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
                            case 0x14:
                            case 0x15:
                            case 0x16:
                            case 0x17:
                            case 0x18:
                            case 0x19:
                            case 0x1A:
                            case 0x1B:
                            case 0x1C:
                            case 0x1D:
                            case 0x1E:
                            case 0x1F:
                            case 0x20:
                            case 0x21:
                                Beep(1);
                            default:
                                break;
                        }
                        switch (gComInfo.HMIArg2)
                        {
                            case 0x4:       //选择工步

                                break;
                            case 0x5:

                                break;
                            case 0x6:

                                break;
                            case 0x7:

                                break;
                            case 0x8:       //选择光
                                if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x01)
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0xFE);
                                }
                                else
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x01);
                                }
                                break;
                            case 0x9:
                                if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x02)
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0xFD);
                                }
                                else
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x02);
                                }
                                break;
                            case 0xA:
                                if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x04)
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0xFB);
                                }
                                else
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x04);
                                }
                                break;
                            case 0xB:
                                if (gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0x08)
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight&0xF7);
                                }
                                else
                                {
                                    HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x08);
                                }
                                break;
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
                            case 0x14:
                                HMI_New_Add();
                                break;
                            case 0x15:
                                HMI_New_Dec();
                                break;
                            case 0x16:      // 同步/顺序模式切换
                                if (gInfo.ModuleInfo.New4in1Module.LightStep[gInfo.ModuleInfo.New4in1Module.StepSel][0]&0x80)
                                {
                                    
                                }
                                else
                                {
                                    
                                }
                                break;
                            case 0x01:      //开始/暂停
                                WP_Start();
                                break;
                            case 0x02:      //停止
                                WP_Stop(1);
                                break;
                            case 0x03:      //智能专家模式切换
                                break;
                            case 0x1D:      //设置按钮
                                HMI_Goto_LocPage(49);
                                break;
                            case 0x1E:      //智能模式编辑
                                break;
                            case 0x1F:      //工程模式(密码)
                                gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                                gComInfo.HMI_Scene=eScene_Password;
                                HMI_Goto_LocPage(ePage_PasswordInput);
                                break;
                            case 0x20:      //使用时间
                                HMI_Goto_Page(50);
                                break;
                            case 0x21:      //返回
                                switch (gComInfo.HMI_Scene)
                                {
                                    case eScene_Module_WiraUsedtime:
                                    case eScene_Module_4in1Usedtime:
                                    case eScene_Module_WiraSetup:
                                    case eScene_Module_4in1Setup:
                                        HMI_Goto_LocPage(49);
                                        break;
                                    case eScene_Module_WiraAuto:
                                    case eScene_Module_4in1Auto:
                                    case eScene_Module_Wira:
                                    case eScene_Module_4in1:
                                        gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                                    default:
                                        HMI_Scene_Recovery();
                                        break;
                                }
                                break;
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
                            Beep(1);
                            HMI_New_Add();
                        }
                        else if(gComInfo.HMIArg2==0x15)
                        {
                            Beep(1);
                            HMI_New_Dec();
                        }
                        else
                        {
                            ;//do nothing
                        }
                    }
                }
            }
            gComInfo.HMIMsg=eMsg_NULL;
            break;
        case eScene_Error:      //密码错误在密码场景来处理
            if (ErrorBeepTime<3 && SystemTime100ms==1)
            {
                SystemTime100ms=0;
                ErrorBeepTime++;    
                Beep(1);
            }
            break;
        case eScene_Password:
            if (gComInfo.HMIMsg == eMsg_keyUp && gComInfo.HMIArg1==eKeyCode_Group2)
            {
                Beep(1);
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
                                    HMI_Goto_LocPage(ePage_Error_Password);
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
                                    BeepEx(3,2);
                                    LL_Module_Send("\x39\xee\x00",3);
                                    break;
                                //case M_Type_650:  //不需调试界面
                                case M_Type_IU:   
                                    BeepEx(3,2);
                                    memset(&gModuleSave,0x30,12);
                                    Save_ModuleSomething();
                                    break;
                                case M_Type_308:
                                    BeepEx(3,2);
                                    LL_Module_Send("1*14&0&0",8);
                                    break;
                                default:
                                    HMI_Goto_LocPage(ePage_Error_Password);
                                    break;
                            };
                        }
                        else if((gInfo.DebugOpen&OPEN_DBG_Config)        && strcmp(gInfo.Password,"308308")==0)     //设置
                        {
                            switch (gComInfo.ModuleType)    //该处根据治疗头类型来判断应解析执行的指令
                            {
                                case M_Type_633:  
                                case M_Type_633_1:
                                case M_Type_IU:   
                                case M_Type_UVA1: 
                                    //TODO:进入设置
                                    break;
                                case M_Type_Wira: 
                                case M_Type_4in1: 
                                    
                                    break;
                                case M_Type_650:  
                                    break;
                                case M_Type_308:
                                    HMI_Goto_LocPage(11);
                                    gComInfo.HMI_LastScene=eScene_Module_308;
                                    gComInfo.HMI_Scene=eScene_Info;
                                    LL_Module_Send("1*11&5&9",8);       //握手308 获取最新使用时间
                                    LL_HMI_Send("\x98\x01\x23\x01\x7c\x21\x81\x02\x0\x1F\x0\x1FV1.0",16);  //显示308版本V1.0
                                    LL_HMI_SendEnd();
                                    break;
                                default:
                                    HMI_Goto_LocPage(ePage_Error_Password);
                                    break;
                            }
                        }
                        else if((gInfo.DebugOpen&OPEN_DBG_ROOT)         && strcmp(gInfo.Password,"9973654")==0) //Root
                        {
                            Dbg_Admin=1;
                        }
                        else    //密码错误
                        {
                            HMI_Goto_LocPage(ePage_Error_Password);
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
                    Beep(1);
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
                            Beep(1);
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
                    Beep(1);
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
                            Beep(1);
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
                    Beep(1);
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
                            Beep(1);
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
                            Beep(1);
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
        case eScene_Info:
            if ((gComInfo.HMIMsg == eMsg_keyUp || gComInfo.HMIMsg== eMsg_KeyLongPush) && gComInfo.HMIArg1 == eKeyCode_Group3)
            {
                Beep(1);
                gComInfo.HMIMsg=eMsg_NULL;
                switch (gComInfo.HMIArg2)
                {
                    case 0x01:      //save
                    {
                        uint8_t xdata cmd[12]="1*12&000&0\0\0";
                        uint8_t len;
                        if (gInfo.ModuleInfo.mini308Module.Freq/100>0)
                        {
                            uint8_t t=0;
                            cmd[5]=gInfo.ModuleInfo.mini308Module.Freq/100 +'0';
                            t=gInfo.ModuleInfo.mini308Module.Freq%100;
                            cmd[6]=t/10 +'0';
                            cmd[7]=t%10 +'0';
                            cmd[8]='&';
                            cmd[9]=gInfo.ModuleInfo.mini308Module.Duty/10 +'0';
                            cmd[10]=gInfo.ModuleInfo.mini308Module.Duty%10 +'0';
                            len=11;
                        }
                        else
                        {
                            cmd[5]=gInfo.ModuleInfo.mini308Module.Freq/10 +'0';
                            cmd[6]=gInfo.ModuleInfo.mini308Module.Freq%10 +'0';
                            cmd[7]='&';
                            cmd[8]=gInfo.ModuleInfo.mini308Module.Duty/10 +'0';
                            cmd[9]=gInfo.ModuleInfo.mini308Module.Duty%10 +'0';
                            len=10;
                        }
                        LL_Module_Send(cmd,len);
                    }
                        break;
                    case 0x02:      //触摸屏校准
                        LL_HMI_Send("\xE4\x55\xAA\x5A\xA5,",5);
                        LL_HMI_SendEnd();
                        break;
                    case 0x03:      //exit
                        HMI_Scene_Recovery();
                        break;
                    case 0x04:      //设置    //只有308有设置
                        HMI_Goto_LocPage(13);
                        HMI_308Set_Freq();
                        HMI_308Set_Duty();
                        break;
                    case 0x31:      //308专用   freq add
                        if (gInfo.ModuleInfo.mini308Module.Freq>=250)
                        {
                            gInfo.ModuleInfo.mini308Module.Freq=50;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.Freq++;
                        }
                        HMI_308Set_Freq();
                        break;
                    case 0x32:      //308专用 freq dec
                        if (gInfo.ModuleInfo.mini308Module.Freq<=50)
                        {
                            gInfo.ModuleInfo.mini308Module.Freq=250;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.Freq--;
                        }
                        HMI_308Set_Freq();
                        break;
                    case 0x33:      //308专用   duty add
                        if (gInfo.ModuleInfo.mini308Module.Duty>=70)
                        {
                            gInfo.ModuleInfo.mini308Module.Duty=20;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.Duty++;
                        }
                        HMI_308Set_Duty();
                        break;
                    case 0x34:      //308专用 duty dec
                        if (gInfo.ModuleInfo.mini308Module.Duty<=20)
                        {
                            gInfo.ModuleInfo.mini308Module.Duty=70;
                        }
                        else
                        {
                            gInfo.ModuleInfo.mini308Module.Duty--;
                        }
                        HMI_308Set_Duty();
                        break;
                    default:        
                        break;
                }
            }
            if (gComInfo.HMIMsg == eMSg_CailDone)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                Beep(2);
                switch (gComInfo.ModuleType)
                {
                    case M_Type_308:
                        HMI_Goto_LocPage(13);
                        HMI_308Set_Freq();
                        HMI_308Set_Duty();
                        break;
                    default:
                        HMI_Scene_Recovery();
                        break;
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
    uint8_t i=0;
    switch (gComInfo.HMI_LastScene)
    {
        case eScene_Module_650:
            gComInfo.HMI_Scene=eScene_Module_650;
            HMI_Goto_LocPage(ePage_Module650);
            HMI_Show_ModuleName("Derma-650");
            HMI_Show_WorkMode();
            HMI_Show_Worktime1();
            ModuleRoutine_GetUsedTime();
            HMI_Show_Power();
            break;
        case eScene_Module_633:
            gComInfo.HMI_Scene=eScene_Module_633;
            HMI_Goto_LocPage(ePage_Module633);
            HMI_Show_ModuleName("Derma-633");
            HMI_Show_WorkMode();
            HMI_Show_Worktime1();
            ModuleRoutine_GetUsedTime();
            HMI_Show_Power();
            ModuleRoutine_GetCalibData();
            break;
        case eScene_Module_IU:     
            gComInfo.HMI_Scene=eScene_Module_IU;
            HMI_Goto_LocPage(ePage_ModuleIU);
            HMI_Show_ModuleName("Derma-IU");
            HMI_Show_Worktime1();
            HMI_Show_IU_Usedtime();
            HMI_Show_Temp(350);  //固定显示35度
            break;
        case eScene_Module_UVA1:
            gComInfo.HMI_Scene=eScene_Module_UVA1;
            HMI_Goto_LocPage(ePage_ModuleUVA1);
            HMI_Show_ModuleName("Derma-UVA1");
            HMI_Show_Worktime1();
            //ModuleRoutine_GetUsedTime();      //TODO: 暂不读使用时间
            HMI_Show_Power();
            ModuleRoutine_GetCalibData();
            break;
        case eScene_Module_Wira:
            gComInfo.HMI_Scene=eScene_Module_Wira;
            HMI_Goto_LocPage(45);       //TODO: 现测试先进专家模式
            HMI_Cut_PicEx(0x71,61,0,11,399,96,0,0);     //显示名称恢复背景
            HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x80);
            for (i = 0xc; i <= 0x13; i++)
            {
                gInfo.ModuleInfo.New4in1Module.ConfigSel=i;
                HMI_New_ShowStr(0);
            }
            gInfo.ModuleInfo.New4in1Module.ConfigSel=0;
            break;
        case eScene_Module_4in1:
            gComInfo.HMI_Scene=eScene_Module_4in1;
            HMI_Goto_LocPage(45);       //TODO: 现测试先进专家模式
            HMI_Cut_PicEx(0x71,61,0,111,399,196,0,0);     //显示名称恢复背景
            HMI_New_Show_Light(gInfo.ModuleInfo.New4in1Module.ConfigSelLight|0x80);
            for (i = 0xc; i <= 0x13; i++)
            {
                gInfo.ModuleInfo.New4in1Module.ConfigSel=i;
                HMI_New_ShowStr(0);
            }
            gInfo.ModuleInfo.New4in1Module.ConfigSel=0;
            break;
        case eScene_Module_308:
            gComInfo.HMI_Scene=eScene_Module_308;
            HMI_Goto_LocPage(ePage_Module308);
            HMI_Show_308WorkTime();
            break;
        case eScene_Module_308test:
            gComInfo.HMI_Scene=eScene_Module_308test;
            HMI_Goto_LocPage(ePage_Module308Test);
            HMI_308Test_SelTime(3);
            gInfo.ModuleInfo.mini308Module.TestWorkTime=3;
            HMI_308Test_AllPower();
            break;
        case eScene_Error:
            HMI_Goto_Error();
            break;
        default:        //其他没考虑的情况一律返回开机页面
            gComInfo.HMI_Scene=eScene_StartPage;
            HMI_Goto_LocPage(ePage_Main);
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
        BeepEx(5,2);
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
//            PowerCtr_Light2=POWER_OFF;  //off     //TODO: 为了测试,暂不关电
//            PowerCtr_Light1=POWER_OFF;  //off
//            PowerCtr_Main=POWER_OFF;    //off
//            SPI_Send(0x7000);           //DAC 0V 出错断电
            HMI_Goto_Page(37);
            HMI_Show_ErrorStr();
        }
    }
}
void HMI_Exit_Error()
{
    ErrorBeepTime=0;
}


