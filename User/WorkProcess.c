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
        case eWS_FindModule:
        {
            switch (CheckModule())
            {
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
                default:
                    break;
            }
        }
            break;
        case eWS_Wait:
            
            break;
        
        
        default:
            break;
    }
}


