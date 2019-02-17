#include "HMI_LL.h"

void LL_HMI_SendEnd()
{
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
    LL_HMI_SendEnd();
}

void HMI_Shake_Hand(void) 
{
    LL_HMI_Send("\0",1);
}

void HMI_Show_Logo(void) 
{
    LL_HMI_Send("\x9A\0",2);
}
void HMI_Goto_Page(uint8_t pic)
{
    char xdata cmd[]={"\x70\x00"};
    cmd[1]=pic;
    LL_HMI_Send(cmd,2);
}
void HMI_Goto_LocPage(uint8_t pic)
{
    char xdata cmd[]={"\x70\x00"};
    cmd[1]=pic+gConfig.LANG*100;
    LL_HMI_Send(cmd,2);
}
void HMI_Cut_Pic(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye)
{
    uint8_t xdata buf[16]={0};
    uint8_t i=0;
    buf[i++]=cmd;
    buf[i++]=PicID>>8;
    buf[i++]=PicID&0x00ff;
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
}

void HMI_Cut_PicEx(uint8_t cmd,uint8_t PicID,uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye,uint16_t x,uint16_t y)
{
    uint8_t xdata buf[16]={0};
    uint8_t i=0;
    buf[i++]=cmd;
    buf[i++]=PicID>>8;
    buf[i++]=PicID&0x00ff;
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
}

void HMI_Show_ErrorStr()
{
    if (gConfig.LANG==0)    //CHN
    {
        
    }
    else                    //EN
    {
        
    }
}

//只用于常规治疗头模块
void HMI_Show_ModuleName(const char* str)
{
    char xdata cmd[32]={0x98,0x00,0x8C,0x00,0xC8,0x21,0x81,0x01,0xFF,0xFF,0x00,0x1F};
    uint8_t str_len=strlen(str);
    if (gConfig.LANG==0)    //CHN
    {
        cmd[2]=0x8C;
    }
    else                    //EN
    {
        cmd[2]=0x80;
    }
    memcpy(cmd+13,str,str_len);
    LL_HMI_Send(cmd,13+str_len);
}

void HMI_Show_Worktime1()       //650 633 IU 使用
{
    char code cmd[]={0xAA,0x98,0x02,0x62,0x01,0x0E,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
    uint8_t i=0,str_len = 13;
    const uint8_t *ptr=cmd;
    for (i = 0; i < str_len; i++)
    {
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=*ptr;
        ptr++;
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    if(gModuleInfo.RoutineModule.WorkTime/10==0)
    {
        SBUF=' ';
    }
    else
    {
        SBUF=gModuleInfo.RoutineModule.WorkTime/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gModuleInfo.RoutineModule.WorkTime%10+'0';
    LL_HMI_SendEnd();
}

void HMI_Show_Worktime2()   //UVA1 使用同时显示能量
{
}

void HMI_Show_Worktime3()   //308 使用
{
}
