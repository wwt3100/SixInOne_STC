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
              #ifndef _DEBUG                //��������
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
              #else                         //����ģʽ��������
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
                    if (gComInfo.HMIArg2==1)        //��������Բ��ť
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
                    else if(gComInfo.HMIArg2==2)    //ѡ��Ӣ��
                    {
                        gConfig.LANG=1;
                        HMI_Goto_LocPage(1);
                        Save_Config();
                    }
                    else if(gComInfo.HMIArg2==3)    //ѡ������
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
                gComInfo.HMIMsg=eMsg_NULL;      //���������Ϣ
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
                            WP_Stop(0);     //��ͣ
                        }
                        else
                        {
                            WP_Start();     //��ʼ
                        }
                        BeepEx(0);
                        
                    }
                    else if(gComInfo.HMIArg2==0x05) //ֹͣ
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
            else    //��������״̬
            {
                if (gComInfo.HMIMsg==eMsg_keyUp)
                {
                    gComInfo.HMIMsg=eMsg_NULL;
                    if (gComInfo.HMIArg1==0x01)
                    {
                        switch (gComInfo.HMIArg2)
                        {
                            case 0x01:      //ʱ�� ��
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
                            case 0x02:      //ʱ�� ��
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
                            case 0x03:      //ϵͳ��Ϣ��ť(��������ҳ)
                                HMI_Goto_LocPage(17);
                                gComInfo.HMI_LastScene=gComInfo.HMI_Scene;
                                gComInfo.HMI_Scene=eScene_Password;
                                break;
                            case 0x04:      //��ʼ 
                                gModuleInfo.RoutineModule.RemainTime=gModuleInfo.RoutineModule.WorkTime*60;
                                WP_Start();
                                break;
                            case 0x05:      //ֹͣ        //������״̬��������
                                WP_Stop(1);
                                break;
                            case 0x06:      //ģʽ�л�
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
                                    ;   //UVA1��IUû��ģʽ�л�
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
        case eScene_Error:      //������������볡��������
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
                    case 1:         //������ʾ,ȷ�ϰ�ť
                        HMI_Goto_LocPage(17);   //���½�������ҳ
                        break;
                    case 0xF0:      //�˳�
                        HMI_Scene_Recovery();
                        break;
                    case 0xF1:      //�س�
                        
                        break;
                    case 0xF2:      //����ɾ��
                        
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

//HMI������ԭ,���ڳ���/���ú󳡾���ԭ
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
        default:        //����û���ǵ����һ�ɷ��ؿ���ҳ��
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

