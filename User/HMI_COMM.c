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
        uint8_t xdata pbuf[18];
        //uint8_t data_size;
        uint8_t HMI_Cmd;
        memcpy(pbuf,uart1_buff,18);
        //data_size=pbuf[0]<=32?pbuf[0]:32;   //做判断防止堆栈溢出
        HMI_Cmd=pbuf[1];
        Uart1_ReviceFrame=0;
        switch (HMI_Cmd)
        {
            case 0:
                gComInfo.HMIMsg=eMsg_HMI_Shakehand;
                gComInfo.HMIArg1=pbuf[12];
                //LOG_E("HMI Shakehand : %u\r\n",pbuf[12]);
                break;
            case 0x79:  //按下
            {
                if (LongPushTime>=5)
                {
                    gComInfo.HMIMsg=eMsg_KeyLongPush;
                }
                else
                {
                    gComInfo.HMIMsg=eMsg_KeyDown;
                    LongPushTime++;
                }
                gComInfo.HMIArg1=pbuf[2];
                gComInfo.HMIArg2=pbuf[3];
                //LOG_E("D\r\n");
            }
                break;
            case 0x78:  //按键抬起
            {
                gComInfo.HMIMsg=eMsg_keyUp;
                gComInfo.HMIArg1=pbuf[2];
                gComInfo.HMIArg2=pbuf[3];
                LongPushTime=0;
                //LOG_E("up\r\n");
            }
                break;
            default:
                break;
        }
    }
}


