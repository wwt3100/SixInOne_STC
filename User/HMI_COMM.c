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
                if (LongPushTime<5)     //如果长按,则不发抬起消息
                {
                    gComInfo.HMIMsg=eMsg_keyUp;
                    gComInfo.HMIArg1=pbuf[2];
                    gComInfo.HMIArg2=pbuf[3];
                }
                LongPushTime=0;
                //LOG_E("up\r\n");
            }
                break;
            default:
                break;
        }
    }
}

void HMI_Show_ErrorStr()
{
    char code cmd[]={0x98,0x1,0x4b,0x01,0xe,0x23,0x81,0x03,0x00,0x1F,0x00,0x1F};
    LL_HMI_Send(cmd, 12);
    if (gConfig.LANG==0)    //CHN
    {
        char xdata str[]={"错误   "};
        str[5]=gComInfo.ErrorCode/10+'0';
        str[6]=gComInfo.ErrorCode%10+'0';
        LL_HMI_Send_Pure(str, 7);
    }
    else                    //EN
    {
        char xdata str[]={"Error   "};
        str[6]=gComInfo.ErrorCode/10+'0';
        str[7]=gComInfo.ErrorCode%10+'0';
        LL_HMI_Send_Pure(str, 8);
    }
    LL_HMI_SendEnd();
    
}

//只用于常规治疗头模块
void HMI_Show_ModuleName(const char* str)
{
    char xdata cmd[3]={0x98,0x00,0x8C};//0x00,0xC8,0x21,0x81,0x01,0xFF,0xFF,0x00,0x1F};
    uint8_t str_len=strlen(str);
    if (gConfig.LANG==0)    //CHN
    {
        cmd[2]=148;
    }
    else                    //EN
    {
        cmd[2]=138;
    }
    LL_HMI_Send(cmd,3);
    LL_HMI_Send_Pure("\x00\xC8\x21\x81\x01\xFF\xFF\x00\x1F", 9);
    LL_HMI_Send_Pure(str, str_len);
    LL_HMI_SendEnd();
}

//显示工作模式(连续/脉冲),部分功能头没有此功能
void HMI_Show_WorkMode()
{
    if(gInfo.ModuleInfo.RoutineModule.LightMode==0)
    {
        if (gConfig.LANG==0)
        {
            HMI_Cut_Pic(0x71,2,385,485,385+173,485+67);
        }
        else
        {
            HMI_Cut_Pic(0x71,102,325,489,325+231,489+59);
        }
    }
    else
    {
        if (gConfig.LANG==0)
        {
            HMI_Cut_Pic(0x71,16,385,485,385+173,485+67);
        }
        else
        {
            HMI_Cut_Pic(0x71,116,325,489,325+231,489+59);
        }
    }
}

void HMI_Show_Worktime1()       //650 633 IU 使用
{
    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 556, 145, 556+215, 145+264);     //治疗时间背景还原
    if(gInfo.ModuleInfo.RoutineModule.WorkTime/10==0)
    {
        char code cmd[]={0x98,0x02,0x5A,0x01,0x0F,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
        LL_HMI_Send(cmd,12);
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=' ';
    }
    else
    {
        char code cmd[]={0x98,0x02,0x62,0x01,0x0F,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
        LL_HMI_Send(cmd,12);
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=gInfo.ModuleInfo.RoutineModule.WorkTime/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.ModuleInfo.RoutineModule.WorkTime%10+'0';
//    while (Uart1_Busy);
//    Uart1_Busy=1;
//    SBUF=' ';           //多显示一个空格
    LL_HMI_SendEnd();
}

void HMI_Show_Worktime2()         //UVA1 使用 同时显示能量
{
    uint8_t power=gInfo.ModuleInfo.RoutineModule.WorkTime*5,t;
    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 4, 556, 145, 556+215, 145+264);     //治疗时间背景还原
    if(gInfo.ModuleInfo.RoutineModule.WorkTime/10==0)
    {
        char code cmd[]={0x98,0x02,0x8C,0x01,0x19,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
        LL_HMI_Send(cmd,12);
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=' ';
    }
    else
    {
        char code cmd[]={0x98,0x02,0x94,0x01,0x19,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
        LL_HMI_Send(cmd,12);
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=gInfo.ModuleInfo.RoutineModule.WorkTime/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.ModuleInfo.RoutineModule.WorkTime%10+'0';
    LL_HMI_SendEnd();
    
    t=power%100;
    if (power/100==0)
    {
        
        if (t/10==0)
        {
            char code cmd[]={0x98,0x02,0x9c,0x00,0xf1,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
            LL_HMI_Send(cmd,12);
        }
        else
        {
            char code cmd[]={0x98,0x02,0x94,0x00,0xf1,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
            LL_HMI_Send(cmd,12);
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=t/10+'0';
        }
    }
    else
    {
        char code cmd[]={0x98,0x02,0x8B,0x00,0xf1,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
        LL_HMI_Send(cmd,12);
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=power/100+'0';
        while (Uart1_Busy);
        Uart1_Busy=1;
        SBUF=t/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=t%10+'0';
    LL_HMI_SendEnd();
}

void HMI_Show_Worktime3()   //308 使用 同时显示能量
{
}

void HMI_Show_Power()       //显示光功率 650 633
{
    uint16_t pl=gInfo.ModuleInfo.RoutineModule.PowerLevel;
    uint8_t code cmd[]={0x98,0x00,0x3C,0x01,0x5E,0x21,0x81,0x03,0x00,0x1F,0x00,0x1F};
    LL_HMI_Send(cmd,12);
    while (Uart1_Busy);     //千位
    Uart1_Busy=1;
    if (pl/1000==0)
    {
        SBUF=' ';
    }
    else
    {
        SBUF=pl/1000+'0';
    }
    pl=pl%1000;
    while (Uart1_Busy);     //百位
    Uart1_Busy=1;
    if (pl/100==0)
    {
        SBUF=' ';
    }
    else
    {
        SBUF=pl/100+'0';
    }
    pl=pl%100;
    while (Uart1_Busy);     //十位
    Uart1_Busy=1;
    SBUF=pl/10+'0';
    while (Uart1_Busy);     //个位
    Uart1_Busy=1;
    SBUF=pl%10+'0';
    LL_HMI_SendEnd();
}

void HMI_Show_RemainTime()
{
    uint8_t min=0,s=0;
    min=gInfo.ModuleInfo.RoutineModule.RemainTime/60;
    s=gInfo.ModuleInfo.RoutineModule.RemainTime%60;
    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 16, 556, 145, 556+215, 145+264);     //切到剩余时间
    
    LL_HMI_Send("\x98\x02\x4E\x01\x0B\x21\x81\x05\x00\x1F\x00\x1F",12);
    while (Uart1_Busy);     //min十位
    Uart1_Busy=1;
    if (min/10==0)
    {
        SBUF=' ';
    }
    else
    {
        SBUF=min/10+'0';
    }
    while (Uart1_Busy);     //min个位
    Uart1_Busy=1;
    SBUF=min%10+'0';
    LL_HMI_SendEnd();
    
    LL_HMI_Send("\x98\x02\xA6\x01\x0B\x21\x81\x05\x00\x1F\x00\x1F",12);
    while (Uart1_Busy);     //s十位
    Uart1_Busy=1;
    SBUF=s/10+'0';
    while (Uart1_Busy);     //s个位
    Uart1_Busy=1;
    SBUF=s%10+'0';
    LL_HMI_SendEnd();
}

void HMI_Show_Temp()
{
    switch (gComInfo.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
        case eScene_Module_UVA1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 16, 618, 411, 618+139, 411+74);     //按钮切暂停
            HMI_Show_RemainTime();
            break;
        default:
            break;
    }
}


void HMI_Show_Password()
{
    uint8_t i=0;
    uint8_t xdata cmd[]={"\x98\x0\x0"};//0\x6A"};
    uint16_t x=400;
    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 17, 168, 145, 168+463, 152+72);     //字符背景还原
    if (gInfo.PasswordLen>0)
    {
        x = x - 16*gInfo.PasswordLen;
        cmd[1]=x>>8;
        cmd[2]=(uint8_t)x;
        LL_HMI_Send(cmd,3);
        LL_HMI_Send_Pure("\x0\x9B\x0\x81\x7\x0\x1f\x0\x1f",9);
        //#undef  _DEBUG
        #ifdef _DEBUG   //调试模式显示字符
        gInfo.Password[gInfo.PasswordLen]=0;
        LL_HMI_Send_Pure(gInfo.Password,gInfo.PasswordLen);
        #else           //非调试显示*
        for (i = 0; i < gInfo.PasswordLen; i++)
        {
            LL_HMI_Send_Pure("*", 1);
        }
        #endif
        LL_HMI_SendEnd();
    }
}

