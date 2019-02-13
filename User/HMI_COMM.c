#include "HMI_COMM.h"
#include "HMIProcess.h"
#include "main.h"

void HMI_COMM()
{
    static uint8_t LongPushTime=0;
    
    if (Uart1_ReviceFrame==0)
    {
        return;
    }
    else
    {
        uint8_t *pbuf=uart1_buff[!Uart1_buf_sel];
        uint8_t data_size=pbuf[0]<=32?pbuf[0]:32;   //×öÅÐ¶Ï·ÀÖ¹¶ÑÕ»Òç³ö
        uint8_t HMI_Cmd=pbuf[1];
        Uart1_ReviceFrame=0;
        switch (HMI_Cmd)
        {
            case 0:
                gComInfo.HMIMsg=eMsg_HMI_Shakehand;
                gComInfo.HMIArg.data8[0]=pbuf[12];
                break;
            case 0x79:
            {
                if (LongPushTime>5)
                {
                    gComInfo.HMIMsg=eMsg_KeyLongPush;
                }
                else
                {
                    gComInfo.HMIMsg=eMsg_KeyDown;
                    LongPushTime++;
                }
                gComInfo.HMIArg.data8[0]=pbuf[2];
                gComInfo.HMIArg.data8[1]=pbuf[3];
            }
                break;
            case 0x78:  //°´¼üÌ§Æð
            {
                gComInfo.HMIMsg=eMsg_keyUp;
                gComInfo.HMIArg.data8[0]=pbuf[2];
                gComInfo.HMIArg.data8[1]=pbuf[3];
                LongPushTime=0;
            }
                break;
            default:
                break;
        
        }
        memset(pbuf,0,data_size);
    }
}

void HMI_SendShakehand()
{

}


