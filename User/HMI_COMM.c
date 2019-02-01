#include "HMI_COMM.h"
#include "HMIProcess.h"
#include "main.h"


void HMI_COMM()
{
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
            default:
                break;
        
        }
        memset(pbuf,0,32);
    }
}


