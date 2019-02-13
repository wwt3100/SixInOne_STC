#include "HMI_LL.h"


void LL_HMI_Send(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=0xAA;
    for (i = 0; i < str_len; i++)
    {
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=*ptr;
        ptr++;
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=0xCC;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=0x33;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=0xC3;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=0x3C;
}

void HMI_Shake_Hand(void) 
{
    LL_HMI_Send("\0",1);
}

void HMI_Show_Logo(void) 
{
    LL_HMI_Send("\x9A\0",2);
}
void HMI_Goto_LocPage(uint8_t pic)
{
    char xdata cmd[]={"\x70\x00"};
    cmd[1]=pic;
    LL_HMI_Send(cmd,2);
}