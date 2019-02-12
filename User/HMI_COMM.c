#include "HMI_COMM.h"
#include "HMIProcess.h"
#include "main.h"


void HMI_COMM()
{
    static uint8_t LongPushTime=0;
    uint8_t *pbuf=uart1_buff[!Uart1_buf_sel];
    if ((pbuf[0]&0x80)==0x80)
    {
        return;
    }
    else
    {
        uint8_t data_size=pbuf[0]&0x7f;
        uint8_t HMI_Cmd=pbuf[1];
        switch (HMI_Cmd)
        {
            case 0:
                
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
        memset(pbuf,0,32);
    }
}


