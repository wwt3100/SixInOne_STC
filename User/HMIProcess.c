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
              #ifndef _DEBUG                //开机动画
                if (gComInfo.HMIArg1==0)
                {
                    HMI_Show_Logo();
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
                        gComInfo.WorkStat=eWS_CheckModuleStep1;
                        gComInfo.HMI_Scene=eScene_Wait;
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
                            gModuleInfo.RoutineModule.RemainTime--;
                            WP_Stop(0);     //暂停
                        }
                        else
                        {
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
                                    if (gModuleInfo.RoutineModule.WorkTime<30)
                                    {
                                        gModuleInfo.RoutineModule.WorkTime++;
                                    }
                                    else
                                    {
                                        gModuleInfo.RoutineModule.WorkTime=1;
                                    }
                                    HMI_Show_Worktime2();
                                }
                                else
                                {
                                    if (gModuleInfo.RoutineModule.WorkTime<99)
                                    {
                                        gModuleInfo.RoutineModule.WorkTime++;
                                    }
                                    else
                                    {
                                        gModuleInfo.RoutineModule.WorkTime=1;
                                    }
                                    HMI_Show_Worktime1();
                                }
                                break;
                            case 0x02:      //时间 减
                                if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                                {
                                    if (gModuleInfo.RoutineModule.WorkTime>1)
                                    {
                                        gModuleInfo.RoutineModule.WorkTime--;
                                    }
                                    else
                                    {
                                        gModuleInfo.RoutineModule.WorkTime=30;
                                    }
                                }
                                else
                                {
                                    if (gModuleInfo.RoutineModule.WorkTime>1)
                                    {
                                        gModuleInfo.RoutineModule.WorkTime--;
                                    }
                                    else
                                    {
                                        gModuleInfo.RoutineModule.WorkTime=99;
                                    }
                                }
                                HMI_Show_Worktime1();
                                break;
                            case 0x03:      //系统信息按钮(进入密码页)
                                HMI_Goto_LocPage(17);
                                gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                                gComInfo.HMI_Scene=eScene_Password;
                                break;
                            case 0x04:      //开始 
                                gModuleInfo.RoutineModule.RemainTime=gModuleInfo.RoutineModule.WorkTime*60;
                                WP_Start();
                                break;
                            case 0x05:      //停止        //非运行状态不做处理
                                WP_Stop(1);
                                break;
                            case 0x06:      //模式切换
                                if (gComInfo.HMI_Scene!=eScene_Module_UVA1 && gComInfo.HMI_Scene!=eScene_Module_IU)
                                {
                                    if(gModuleInfo.RoutineModule.LightMode==0)
                                    {
                                        gModuleInfo.RoutineModule.LightMode=1;
                                    }
                                    else
                                    {
                                        gModuleInfo.RoutineModule.LightMode=0;
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
                                if (gModuleInfo.RoutineModule.WorkTime<30)
                                {
                                    gModuleInfo.RoutineModule.WorkTime++;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=1;
                                }
                                HMI_Show_Worktime2();
                            }
                            else
                            {
                                if (gModuleInfo.RoutineModule.WorkTime<99)
                                {
                                    gModuleInfo.RoutineModule.WorkTime++;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=1;
                                }
                                HMI_Show_Worktime1();
                            }
                            BeepEx(0);
                        }
                        else if(gComInfo.HMIArg2 == 0x02)
                        {
                            if (gComInfo.HMI_Scene==eScene_Module_UVA1)
                            {
                                if (gModuleInfo.RoutineModule.WorkTime>1)
                                {
                                    gModuleInfo.RoutineModule.WorkTime--;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=30;
                                }
                                HMI_Show_Worktime2();
                            }
                            else
                            {
                                if (gModuleInfo.RoutineModule.WorkTime>1)
                                {
                                    gModuleInfo.RoutineModule.WorkTime--;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=99;
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
                gComInfo.HMIMsg = eMsg_NULL;
                BeepEx(0);
                switch (gComInfo.HMIArg2)
                {
                    case 1:         //错误提示,确认按钮
                        HMI_Goto_LocPage(17);   //重新进入密码页
                        break;
                    case 0xF0:      //退出
                        HMI_Scene_Recovery();
                        break;
                    case 0xF1:      //回车
                        
                        break;
                    case 0xF2:      //密码删除
                        
                        break;
                    case '0':
                        
                        break;
                    default:
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
            HMI_Goto_LocPage(35);
        }
        else if(gComInfo.ErrorCode==Error_PasswordError)
        {
            HMI_Goto_LocPage(36);
        }
        else
        {
            HMI_Goto_Page(37);
            HMI_Show_ErrorStr();
        }
    }
}
void HMI_Exit_Error()
{
    ErrorBeepTime=0;
}

