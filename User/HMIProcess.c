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
                            HMI_Cut_Pic(0x9C,60,221,186,508,315,258,272);
                        }
                        else
                        {
                            HMI_Cut_Pic(0x9C,60,221,339,508,470,258,272);
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
            break;
        
        case eScene_Module_633:
            
            break;
        case eScene_Module_UVA1:
            
            break;
        case eScene_Module_Wira:
        case eScene_Module_4in1:
            
            break;
        case eScene_Error:
           {}
           break;
        default:
           break;
    }

}

