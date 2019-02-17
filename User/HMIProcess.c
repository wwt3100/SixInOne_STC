#include "HMIProcess.h"
#include "WorkProcess.h"
#include "main.h"
#include "HMI_COMM.h"

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
                    gComInfo.HMI_Scene++;
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
                    gComInfo.HMI_Scene++;
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
            break;
        case eScene_Module_308:
            break;
        case eScene_Module_IU:
            
            break;
        case eScene_Module_650:
        case eScene_Module_633:
            if (gComInfo.WorkStat==eWS_Working)
            {
                if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg1==0x01)
                {
                    if (gComInfo.HMIArg2==0x04)     //暂停
                    {
                        BeepEx(0);
                    }
                    else if(gComInfo.HMIArg2==0x05) //停止
                    {
                        BeepEx(0);
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
                                if (gModuleInfo.RoutineModule.WorkTime<99)
                                {
                                    gModuleInfo.RoutineModule.WorkTime++;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=1;
                                }
                                HMI_Show_Worktime1();
                                break;
                            case 0x02:      //时间 减
                                if (gModuleInfo.RoutineModule.WorkTime>1)
                                {
                                    gModuleInfo.RoutineModule.WorkTime--;
                                }
                                else
                                {
                                    gModuleInfo.RoutineModule.WorkTime=99;
                                }
                                HMI_Show_Worktime1();
                                break;
                            case 0x03:      //系统信息按钮(进入密码页)
                                
                                break;
                            case 0x04:      //开始 
                                gModuleInfo.RoutineModule.RemainTime=gModuleInfo.RoutineModule.WorkTime*60;
                                break;
                            case 0x05:      //停止
                                
                                break;
                            case 0x06:      //模式切换
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
                            if (gModuleInfo.RoutineModule.WorkTime<99)
                            {
                                gModuleInfo.RoutineModule.WorkTime++;
                            }
                            else
                            {
                                gModuleInfo.RoutineModule.WorkTime=1;
                            }
                            HMI_Show_Worktime1();
                            BeepEx(0);
                        }
                        else if(gComInfo.HMIArg2 == 0x02)
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
        case eScene_Module_UVA1:
            
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            
            break;
        case eScene_Error:
            //只处理密码错误按键事件
            if (gComInfo.ErrorCode==Error_PasswordError && 
                gComInfo.HMIMsg ==eMsg_keyUp && 
                gComInfo.HMIArg1== 2 &&
                gComInfo.HMIArg2== 1 )
            {
                gComInfo.ErrorCode=0;
                HMI_Exit_Error();
            }
           break;
        default:
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

}

