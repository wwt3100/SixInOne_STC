#include "HMI_LL.h"

void LL_HMI_SendEnd()
{
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=0xCC;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=0x33;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=0xC3;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=0x3C;
}
void LL_HMI_Send(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=0xAA;
    for (i = 0; i < str_len; i++)
    {
        while (gCom.Uart1_Busy);
        gCom.Uart1_Busy=1;
        SBUF=*ptr;
        ptr++;
    }
}
void LL_HMI_Send_Pure(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    for (i = 0; i < str_len; i++)
    {
        while (gCom.Uart1_Busy);
        gCom.Uart1_Busy=1;
        SBUF=*ptr;
        ptr++;
    }
}
void LL_HMI_SendXY(uint16_t x,uint16_t y)
{
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=x>>8;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=(uint8_t)x;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=y>>8;
    while (gCom.Uart1_Busy);
    gCom.Uart1_Busy=1;
    SBUF=(uint8_t)y;
}


void HMI_Shake_Hand(void) 
{
    LL_HMI_Send("\0",1);
    LL_HMI_SendEnd();
}

void HMI_Show_Logo(void) 
{
    LL_HMI_Send("\x9A\0",2);
    LL_HMI_SendEnd();
}
void HMI_Goto_Page(uint8_t pic)
{
    char xdata cmd[]={"\x70\x00"};
    cmd[1]=pic;
    LL_HMI_Send(cmd,2);
    LL_HMI_SendEnd();
}

//跳到当前语言的指定页面,会立刻刷新屏幕,这样才能载入按钮事件
void HMI_Goto_LocPage(uint8_t pic)
{
    char xdata cmd[]={"\x70\x00"};
    cmd[1]=pic+gConfig.LANG*100;
    LL_HMI_Send(cmd,2);
    LL_HMI_SendEnd();
}
void HMI_Cut_Pic(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye)
{
    uint8_t xdata buf[16]={0};
    uint8_t i=0;
    buf[i++]=cmd;
    buf[i++]=PicID;
    buf[i++]=xs>>8;
    buf[i++]=xs&0x00ff;
    buf[i++]=ys>>8;
    buf[i++]=ys&0x00ff;
    buf[i++]=xe>>8;
    buf[i++]=xe&0x00ff;
    buf[i++]=ye>>8;
    buf[i++]=ye&0x00ff;
    buf[i++]=xs>>8;
    buf[i++]=xs&0x00ff;
    buf[i++]=ys>>8;
    buf[i++]=ys&0x00ff;
    LL_HMI_Send(buf,i);
    LL_HMI_SendEnd();
}

void HMI_Cut_PicEx(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye,uint16_t x,uint16_t y)
{
    uint8_t xdata buf[16]={0};
    uint8_t i=0;
    buf[i++]=cmd;
    buf[i++]=PicID;
    buf[i++]=xs>>8;
    buf[i++]=xs&0x00ff;
    buf[i++]=ys>>8;
    buf[i++]=ys&0x00ff;
    buf[i++]=xe>>8;
    buf[i++]=xe&0x00ff;
    buf[i++]=ye>>8;
    buf[i++]=ye&0x00ff;
    buf[i++]=x>>8;
    buf[i++]=x&0x00ff;
    buf[i++]=y>>8;
    buf[i++]=y&0x00ff;
    LL_HMI_Send(buf,i);
    LL_HMI_SendEnd();
}


