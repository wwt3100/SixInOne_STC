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
            }
            if (gComInfo.HMIMsg==eMsg_HMI_Shakehand)
            {
                gComInfo.HMIMsg=eMsg_NULL;
                if (gComInfo.HMIArg.data8[0]==0)
                {
                    HMI_Show_Logo();
                }
                else if(gComInfo.HMIArg.data8[0]==204)
                {
                    gComInfo.HMI_Scene++;
                    HMI_Goto_LocPage((gConfig.LANG*100)+1);
                    BeepEx(10);
                }
                else
                {
                    ;   //wait
                }
            }
        }
            break;
        case eScene_StartPage:
            if (gComInfo.HMIMsg==eMsg_keyUp && gComInfo.HMIArg.data16==(uint16_t)1)
            {
                BeepEx(1);
            }
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
            
            break;
        case eScene_Module_4in1:
            
            break;
        case eScene_Error:
           {}
           break;
        default:
           break;
    }

}

