#include "WorkProcess.h"
#include "HMIProcess.h"
#include "main.h"

uint8_t CheckModule()
{
    return 0;
}

void Work_Process()
{
    switch (gComInfo.WorkStat)
    {
        case eWS_Idle:  //do nothing
            break;
        case eWS_CheckModuleStep1:
        {

        }
            break;
        case eWS_CheckTempSensor:
            
            break;
        case eWS_CheckModuleStep2:
            
            break;
        case eWS_Wait:
            
            break;
        
        
        default:
            break;
    }
}


