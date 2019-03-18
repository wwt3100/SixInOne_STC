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
                gCom.HMIMsg=eMsg_HMI_Shakehand;
                gCom.HMIArg1=pbuf[12];
                //LOG_E("HMI Shakehand : %u\r\n",pbuf[12]);
                break;
            case 0x79:  //按下
            {
                if (LongPushTime>=5)
                {
                    gCom.HMIMsg=eMsg_KeyLongPush;
                }
                else
                {
                    gCom.HMIMsg=eMsg_KeyDown;
                    LongPushTime++;
                }
                gCom.HMIArg1=pbuf[2];
                gCom.HMIArg2=pbuf[3];
                //LOG_E("D\r\n");
            }
                break;
            case 0x78:  //按键抬起
            {
                if (LongPushTime<5)     //如果长按,则不发抬起消息
                {
                    gCom.HMIMsg=eMsg_keyUp;
                    gCom.HMIArg1=pbuf[2];
                    gCom.HMIArg2=pbuf[3];
                }
                LongPushTime=0;
                //LOG_E("up\r\n");
            }
                break;
            case 0xE4:  //校准屏幕返回消息
                gCom.HMIMsg=eMSg_CailDone;
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
    if (gConfig.LANG==LANG_ZH)    //CHN
    {
        char xdata str[]={"错误   "};
        str[5]=gCom.ErrorCode/10+'0';
        str[6]=gCom.ErrorCode%10+'0';
        LL_HMI_Send_Pure(str, 7);
    }
    else                    //EN
    {
        char xdata str[]={"Error   "};
        str[6]=gCom.ErrorCode/10+'0';
        str[7]=gCom.ErrorCode%10+'0';
        LL_HMI_Send_Pure(str, 8);
    }
    LL_HMI_SendEnd();
    
}

//只用于常规治疗头模块
void HMI_Show_ModuleName(const char* str)
{
    char xdata cmd[3]={0x98,0x00,0x8C};//0x00,0xC8,0x21,0x81,0x01,0xFF,0xFF,0x00,0x1F};
    uint8_t str_len=strlen(str);
    if (gConfig.LANG==LANG_ZH)    //CHN
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
    if(gInfo.ModuleInfo.Routine.LightMode==0)
    {
        if (gConfig.LANG==LANG_ZH)
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
    if(gInfo.ModuleInfo.Routine.WorkTime/10==0)
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
        SBUF=gInfo.ModuleInfo.Routine.WorkTime/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.ModuleInfo.Routine.WorkTime%10+'0';
//    while (Uart1_Busy);
//    Uart1_Busy=1;
//    SBUF=' ';           //多显示一个空格
    LL_HMI_SendEnd();
}

void HMI_Show_Worktime2()         //UVA1 使用 同时显示能量
{
    uint8_t power=gInfo.ModuleInfo.Routine.WorkTime*5,t;
    HMI_Cut_Pic(0x71,gConfig.LANG*100 + 4, 556, 145, 556+215, 145+264);     //治疗时间背景还原
    if(gInfo.ModuleInfo.Routine.WorkTime/10==0)
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
        SBUF=gInfo.ModuleInfo.Routine.WorkTime/10+'0';
    }
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.ModuleInfo.Routine.WorkTime%10+'0';
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

void HMI_Show_Power()       //显示光功率 650 633
{
    uint16_t pl=gInfo.ModuleInfo.Routine.PowerLevel;
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
    min=gInfo.ModuleInfo.Routine.RemainTime/60;
    s=gInfo.ModuleInfo.Routine.RemainTime%60;
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

void HMI_Show_Temp(int16_t temp)
{
    switch (gCom.HMI_Scene)     //显示部分
    {
        case eScene_Module_650:
        case eScene_Module_633:
        case eScene_Module_IU:
        case eScene_Module_UVA1:
            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 13, 415, 13+81, 415+36); //恢复温度背景
            LL_HMI_Send("\x98\x00\x37\x01\x9F\x21\x81\x03\x00\x1F\x00\x1F",12);
            break;
        case eScene_Debug:
            HMI_Cut_Pic(0x71,62, 517, 22, 517+88, 22+51); //恢复温度背景
            LL_HMI_Send("\x98\x02\x20\x00\x20\x21\x81\x03\x00\x1F\x00\x1F",12);
            break;
        case eScene_Module_4in1:
        case eScene_Module_Wira:
            HMI_Cut_Pic(0x71,45, 229, 504, 229+166, 504+75); //恢复温度背景
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(248, 528);
            LL_HMI_Send_Pure("\x6\x80\x05\x00\xF8\x00\x1F",7); //红字
            LL_HMI_Send_Pure("Temp:",5);
            break;
        default:
            break;
    }
    if (temp<0)
    {
        temp=0;
    }
    else
    {
        temp=(temp+5)/10;     //四舍五入取整
        if (temp>99)
        {
            temp=99;        //最高显示99度
        }
    }
    if (temp/10==0)
    {
        while (Uart1_Busy);     //十位
        Uart1_Busy=1;
        SBUF=' ';
    }
    else
    {
        while (Uart1_Busy);     //十位
        Uart1_Busy=1;
        SBUF=temp/10+'0';
    }
    while (Uart1_Busy);     //温度个位
    Uart1_Busy=1;
    SBUF=temp%10+'0';
    LL_HMI_SendEnd();
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
        #if defined(_DEBUG) && 0   /*调试模式显示字符*/
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

void HMI_DGB_Show_DAval()
{
    HMI_Cut_Pic(0x71,62, 109, 396, 109+106, 396+61);  //DA背景恢复
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(134, 411);
    LL_HMI_Send_Pure("\x6\x80\x05\x00\x1F\x00\x1F",7);
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.Debug.dac/10+'0';
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF='.';
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF=gInfo.Debug.dac%10+'0';
    LL_HMI_SendEnd();
    if (Fire_Flag)      //运行时实时可以改电压
    {
        uint16_t dac=(float)gInfo.Debug.dac/0.01220703125;
        SPI_Send(dac|0x7000);
    }
}

void HMI_Show_IU_Usedtime()
{
    uint8_t code cmd[]="\x21\x81\x01\xFF\xFF\x00\x1F";
    if (gConfig.LANG==LANG_ZH)
    {
        HMI_Cut_Pic(0x71, 2, 133, 219, 133+99, 219+44);     //字符背景还原
    }
    else
    {
        HMI_Cut_Pic(0x71,102, 125, 220, 125+100, 220+42);     //字符背景还原
    }
    
    LL_HMI_Send("\x98",1);      //时间
    if (gConfig.LANG==LANG_ZH)
    {
        LL_HMI_SendXY(0x8C, 0xDE);
    }
    else
    {
        LL_HMI_SendXY(0x74, 0xDE);
    }
    LL_HMI_Send_Pure(cmd,7);
    LL_HMI_Send_Pure(gModuleSave.UsedTime,5);
    LL_HMI_Send_Pure("h",1);
    LL_HMI_Send_Pure(gModuleSave.UsedTime+5,2);
    LL_HMI_Send_Pure("min",3);
    LL_HMI_SendEnd();
    
    LL_HMI_Send("\x98",1);  //次数
    if (gConfig.LANG==LANG_ZH)
    {
        LL_HMI_SendXY(0x8C, 0xF3);
    }
    else
    {
        LL_HMI_SendXY(0x7E, 0xF1);
    }
    LL_HMI_Send_Pure(cmd,7);
    LL_HMI_Send_Pure(gModuleSave.UsedCount,5);
    LL_HMI_SendEnd();

}

void HMI_Show_IU_Temp()         //根据陈工程序逻辑改写,显示假的温度
{
    uint16_t fv;
    if (gCom.FeedbackVolt<1000)
    {
        fv=gCom.FeedbackVolt%1000;
        if(fv>900)
        {
            HMI_Show_Temp(320);
        }
        else if(fv>800)
        {
            HMI_Show_Temp(310);
        }
        else if(fv>700)
        {
            HMI_Show_Temp(300);
        }
        else if(fv>500)
        {
            HMI_Show_Temp(290);
        } 
        else if(fv>200)
        {
            HMI_Show_Temp(280);
        }
        else if(fv>150)
        {
            HMI_Show_Temp(270);
        }
        else if(fv>100)
        {
            HMI_Show_Temp(260);
        }
        else
        {
            HMI_Show_Temp(250);
        }
    }
    else
    {
        fv=gCom.FeedbackVolt%1000;
        if(fv>500)
        {
            HMI_Show_Temp(420);
        }
        else if(fv>400)
        {
            HMI_Show_Temp(400);
        }
        else if(fv>300)
        {
            HMI_Show_Temp(380);
        }
        else if(fv>200)
        {
            HMI_Show_Temp(360);
        } 
        else if(fv>100)
        {
            HMI_Show_Temp(340);
        }
        else
        {
            HMI_Show_Temp(320);
        }
    }
}

void HMI_Show_308WorkTime()         //连同能量一起显示
{
    uint8_t page,t;
    uint16_t power=gInfo.ModuleInfo.mini308.WorkTime*3;
    uint8_t xdata time_str[3]={0x20};
    page=ePage_Module308+gConfig.LANG*100;
    HMI_Cut_Pic(0x71,page, 296, 208, 296+191, 208+150); //背景恢复
    
    //显示时间
    LL_HMI_Send("\x98",1);
    t=gInfo.ModuleInfo.mini308.WorkTime/100;
    if (t==0)
    {
        t=gInfo.ModuleInfo.mini308.WorkTime%100;
        if (t/10==0)    //1位
        {
            LL_HMI_SendXY(370,239);
            time_str[1]='0'+t%10;
        }
        else            //2位
        {
            LL_HMI_SendXY(370-8,239);
            time_str[1]='0'+t/10;
            time_str[2]='0'+t%10;
        }
    }
    else    //3位
    {
        LL_HMI_SendXY(370,239);
        time_str[0]='0'+t;
        t=gInfo.ModuleInfo.mini308.WorkTime%100;
        time_str[1]='0'+t/10;
        time_str[2]='0'+t%10;
    }
    LL_HMI_Send_Pure("\x05\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(time_str,3);
    LL_HMI_SendEnd();
    
    //显示能量
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(362,295);
    LL_HMI_Send_Pure("\x05\x80\x05\x01\xAF\x0\x1f",7);
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF='0'+power/100;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF='.';
    power=power%100;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF='0'+power/10;
    while (Uart1_Busy);
    Uart1_Busy=1;
    SBUF='0'+power%10;
    LL_HMI_SendEnd();
}

void HMI_Show_308RemainTime()
{
    uint8_t t;
    uint16_t power;
    uint8_t xdata time_str[4]={0x20};
    uint8_t xdata power_str[]={"0.00J/cm\xB2"};  //0.00J/cm2 ±2 Ⅱ
    if (gCom.HMI_Scene==eScene_Module_308)
    {
        uint16_t p;
        HMI_Cut_Pic(0x71,ePage_Module308, 162, 409, 162+464, 407+33);     //进度条背景恢复
        p=464/gInfo.ModuleInfo.mini308.WorkTime*(gInfo.ModuleInfo.mini308.WorkTime-gInfo.ModuleInfo.mini308.RemainTime);
        HMI_Cut_Pic(0x71,ePage_Module308+1, 162, 409, 162+(p), 407+33);
        power=(gInfo.ModuleInfo.mini308.WorkTime-gInfo.ModuleInfo.mini308.RemainTime)*3;
        //进度条显示剩余时间
        LL_HMI_Send("\x98",1);
        t=gInfo.ModuleInfo.mini308.RemainTime/100;
        if (t==0)
        {
            t=gInfo.ModuleInfo.mini308.RemainTime%100;
            if (t/10==0)    //1位
            {
                time_str[1]='0'+t%10;
            }
            else            //2位
            {
                time_str[1]='0'+t/10;
                time_str[2]='0'+t%10;
            }
        }
        else    //3位
        {
            
            time_str[0]='0'+t;
            t=gInfo.ModuleInfo.mini308.RemainTime%100;
            time_str[1]='0'+t/10;
            time_str[2]='0'+t%10;
        }
        time_str[3]='s';
        LL_HMI_SendXY(553,412);
        LL_HMI_Send_Pure("\x3\x80\x03\x01\xAF\x0\x1f",7);
        LL_HMI_Send_Pure(time_str,4);
        LL_HMI_SendEnd();
        
        //进度条显示能量
        power_str[0]=power/100+'0';
        power=power%100;
        power_str[2]=power/10+'0';
        power_str[3]=power%10+'0';
        LL_HMI_Send("\x98",1);
        LL_HMI_SendXY(179,412);
        LL_HMI_Send_Pure("\x3\x80\x03\x01\xAF\x0\x1f",7);
        LL_HMI_Send_Pure(power_str,9);
        LL_HMI_SendEnd();
    }
    else    //红斑测试
    {
        uint16_t p;
        HMI_Cut_Pic(0x71,ePage_Module308Test, 178, 399, 178+440, 399+39);     //进度条背景恢复
        p=440/gInfo.ModuleInfo.mini308.TestWorkTime*(gInfo.ModuleInfo.mini308.TestWorkTime-gInfo.ModuleInfo.mini308.RemainTime);
        HMI_Cut_Pic(0x71,ePage_Module308Test+1, 178, 399, 178+p, 399+39); 
    }
}

void HMI_308Test_SelTime(uint8_t seltime)
{
    uint8_t xdata power_str[4]="0.00";
    uint8_t power=seltime*3;
    switch (gInfo.ModuleInfo.mini308.TestSelTime)
    {
        case 3:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 82, 304, 82+108, 304+69);
            break;
        case 6:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 194, 304, 194+108, 304+69);
            break;
        case 9:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 293, 304, 293+108, 304+69);
            break;
        case 12:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 399, 304, 399+99, 304+69);
            break;
        case 15:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 504, 304, 504+105, 304+69);
            break;
        case 18:
            HMI_Cut_Pic(0x71,ePage_Module308Test, 611, 304, 611+99, 304+69);
            break;
        default:
            break;
    }
    gInfo.ModuleInfo.mini308.TestSelTime=seltime;
    switch (gInfo.ModuleInfo.mini308.TestSelTime)
    {
        case 3:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 82, 304, 82+108, 304+69);
            break;
        case 6:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 194, 304, 194+108, 304+69);
            break;
        case 9:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 293, 304, 293+108, 304+69);
            break;
        case 12:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 399, 304, 399+99, 304+69);
            break;
        case 15:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 504, 304, 504+105, 304+69);
            break;
        case 18:
            HMI_Cut_Pic(0x71,ePage_Module308Test+1, 611, 304, 611+99, 304+69);
            break;
        default:
            break;
    }

    //显示当前计量
    power_str[0]=power/100+'0';
    power=power%100;
    power_str[2]=power/10+'0';
    power_str[3]=power%10+'0';
    HMI_Cut_Pic(0x71,ePage_Module308Test+gConfig.LANG*100, 329, 179, 329+183, 179+51);
    LL_HMI_Send("\x98",1);
    if (gConfig.LANG==LANG_ZH)
    {
        LL_HMI_SendXY(397,191);
    }
    else
    {
        LL_HMI_SendXY(408,191);
    }
    LL_HMI_Send_Pure("\x21\x81\x02\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(power_str,4);
    LL_HMI_SendEnd();
}

void HMI_308Test_AllPower()
{
    uint8_t xdata power_str[4]="0.00";
    uint8_t power=gInfo.ModuleInfo.mini308.TotalTime*3;
    HMI_Cut_Pic(0x71,ePage_Module308Test+gConfig.LANG*100, 345, 229, 345+168, 229+48);
    power_str[0]=power/100+'0';
    power=power%100;
    power_str[2]=power/10+'0';
    power_str[3]=power%10+'0';
    LL_HMI_Send("\x98",1);
    if (gConfig.LANG==LANG_ZH)
    {
        LL_HMI_SendXY(397,235);
    }
    else
    {
        LL_HMI_SendXY(408,235);
    }
    LL_HMI_Send_Pure("\x21\x81\x02\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(power_str,4);
    LL_HMI_SendEnd();
}

void HMI_308Set_Freq()
{
    uint8_t f;
    uint8_t xdata freq_str[3];
    f=gInfo.ModuleInfo.mini308.Freq/100;
    if (f!=0)
    {
        freq_str[0]=f+'0';
    }
    else
    {
        freq_str[0]=' ';
    }
    f=gInfo.ModuleInfo.mini308.Freq%100;
    freq_str[1]=f/10+'0';
    freq_str[2]=f%10+'0';
    HMI_Cut_Pic(0x71,13, 398, 203, 398+95, 203+52);
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(413,209);
    LL_HMI_Send_Pure("\x05\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(freq_str,3);
    LL_HMI_SendEnd();
}

void HMI_308Set_Duty()
{
    uint8_t f;
    uint8_t xdata duty_str[2];
    f=gInfo.ModuleInfo.mini308.Duty;
    duty_str[0]=f/10+'0';
    duty_str[1]=f%10+'0';
    HMI_Cut_Pic(0x71,13, 411, 336, 411+82, 336+59);
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(420,347);
    LL_HMI_Send_Pure("\x05\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(duty_str,2);
    LL_HMI_SendEnd();
}

void HMI_New_Show_LightName(uint8_t sellight)
{
    uint8_t edit_mode;
    edit_mode=gInfo.ModuleInfo.New4in1.EditMode;
    switch (gCom.HMI_Scene)
    {
        case eScene_Module_4in1:
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x01) != (sellight & 0x01)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x01)? 46:45;
                    HMI_Cut_Pic(0x71,page, 38, 94, 38+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(67-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x01)? 48:47;
                    HMI_Cut_Pic(0x71,page, 47, 178, 47+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(65, 196);
                }
                if(sellight & 0x01)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEW4IN1_L1WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x02) != (sellight & 0x02)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x02)? 46:45;
                    HMI_Cut_Pic(0x71,page, 193, 94, 193+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(221-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x02)? 48:47;
                    HMI_Cut_Pic(0x71,page, 203, 178, 203+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(218, 196);
                }
                if(sellight & 0x02)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEW4IN1_L2WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x04) != (sellight & 0x04)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x04)? 46:45;
                    HMI_Cut_Pic(0x71,page, 348, 94, 348+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(378-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x04)? 48:47;
                    HMI_Cut_Pic(0x71,page, 359, 178, 359+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(374, 196);
                }
                if(sellight & 0x04)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEW4IN1_L3WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x08) != (sellight & 0x08)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x08)? 46:45;
                    HMI_Cut_Pic(0x71,page, 505, 94, 505+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(534-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x08)? 48:47;
                    HMI_Cut_Pic(0x71,page, 516, 178, 516+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(530, 196);
                }
                if(sellight & 0x08)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEW4IN1_L4WL);
                LL_HMI_SendEnd();
            }
            gInfo.ModuleInfo.New4in1.ConfigSelLight=sellight;
            break;
        case eScene_Module_Wira:
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x01) != (sellight & 0x01)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x01)? 46:45;
                    HMI_Cut_Pic(0x71,page, 38, 94, 38+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(67-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x01)? 48:47;
                    HMI_Cut_Pic(0x71,page, 47, 178, 47+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(65, 196);
                }
                if(sellight & 0x01)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEWWIRA_L1WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x02) != (sellight & 0x02)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x02)? 46:45;
                    HMI_Cut_Pic(0x71,page, 193, 94, 193+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(221-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x02)? 48:47;
                    HMI_Cut_Pic(0x71,page, 203, 178, 203+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(218, 196);
                }
                if(sellight & 0x02)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEWWIRA_L2WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x04) != (sellight & 0x04)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x04)? 46:45;
                    HMI_Cut_Pic(0x71,page, 348, 94, 348+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(378-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x04)? 48:47;
                    HMI_Cut_Pic(0x71,page, 359, 178, 359+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(374, 196);
                }
                if(sellight & 0x04)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEWWIRA_L3WL);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1.ConfigSelLight & 0x08) != (sellight & 0x08)) || (sellight & 0x80))
            {
                if (edit_mode==0)
                {
                    uint8_t page = (sellight & 0x08)? 46:45;
                    HMI_Cut_Pic(0x71,page, 505, 94, 505+145, 94+75);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(534-8, 117);
                }
                else
                {
                    uint8_t page = (sellight & 0x08)? 48:47;
                    HMI_Cut_Pic(0x71,page, 516, 178, 516+142, 178+66);      //背景恢复
                    LL_HMI_Send("\x98",1);
                    LL_HMI_SendXY(530, 196);
                }
                if(sellight & 0x08)
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure(STR_NEWWIRA_L4WL);
                LL_HMI_SendEnd();
            }
            gInfo.ModuleInfo.New4in1.ConfigSelLight=sellight&0x7F;
            break;
        default:
            break;
    }
}

void HMI_New_ShowStr(uint8_t sel)
{
    uint8_t page,edit_mode;
    uint8_t p;
    edit_mode=gInfo.ModuleInfo.New4in1.EditMode;
    if (edit_mode==0)
    {
        page=sel==1?46:45;
    }
    else
    {
        page=sel==1?48:47;
    }
    switch (gInfo.ModuleInfo.New4in1.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 35, 189, 35+145, 189+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(80-16, 215);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 47, 259, 47+143, 259+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(68, 279);
            }
            LL_HMI_Send_Pure("\x3\x80\x03\x1C\xFF\x00\x1F",7);     //蓝字
            p=gInfo.ModuleInfo.New4in1.PowerLevel[0]/100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            p=gInfo.ModuleInfo.New4in1.PowerLevel[0]%100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p%10+'0';
            LL_HMI_Send_Pure("mW/cm\xB2",6);
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel2:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 191, 189, 191+145, 189+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(235-16, 215);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 203, 258, 203+142, 258+72);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(219, 279);
            }
            LL_HMI_Send_Pure("\x3\x80\x03\xF9\x04,\x00\x1F",7);    //红字
            p=gInfo.ModuleInfo.New4in1.PowerLevel[1]/100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            p=gInfo.ModuleInfo.New4in1.PowerLevel[1]%100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p%10+'0';
            LL_HMI_Send_Pure("mW/cm\xB2",6);
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel3:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 349, 189, 349+145, 189+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(388-16, 215);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 361, 261, 361+143, 261+68);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(378, 279);
            }
            LL_HMI_Send_Pure("\x3\x80\x03\xD5\x00\x00\x1F",7);     //黄字
            p=gInfo.ModuleInfo.New4in1.PowerLevel[2]/100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            p=gInfo.ModuleInfo.New4in1.PowerLevel[2]%100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p%10+'0';
            LL_HMI_Send_Pure("mW/cm\xB2",6);
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel4:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 514, 189, 514+145, 189+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(548-16, 215);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 516, 259, 516+143, 259+72);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(530, 279);
            }
            LL_HMI_Send_Pure("\x3\x80\x03\xD3\x40\x00\x1F",7);     //橙字
            p=gInfo.ModuleInfo.New4in1.PowerLevel[3]/100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            p=gInfo.ModuleInfo.New4in1.PowerLevel[3]%100;
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=p%10+'0';
            LL_HMI_Send_Pure("mW/cm\xB2",6);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime1:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 34, 280, 34+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(68, 301);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 47, 343, 47+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(65, 366);
            }
            LL_HMI_Send_Pure("\x4\x80\x04\x1C\xFF\x00\x1F",7);     //蓝字
            while (Uart1_Busy);
            Uart1_Busy=1;
            p=gInfo.ModuleInfo.New4in1.WorkTime[1]/10;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1.WorkTime[1]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime2:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 190, 280, 190+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(226, 301);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 205, 343, 205+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(217, 366);
            }
            LL_HMI_Send_Pure("\x4\x80\x04\xF9\x04\x00\x1F",7);    //红字
            while (Uart1_Busy);
            Uart1_Busy=1;
            p=gInfo.ModuleInfo.New4in1.WorkTime[2]/10;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1.WorkTime[2]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime3:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 344, 280, 344+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(380, 301);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 360, 343, 360+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(373, 366);
            }
            LL_HMI_Send_Pure("\x4\x80\x04\xD5\x00\x00\x1F",7);     //黄字
            while (Uart1_Busy);
            Uart1_Busy=1;
            p=gInfo.ModuleInfo.New4in1.WorkTime[3]/10;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1.WorkTime[3]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime4:
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 503, 280, 503+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(538, 301);
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 516, 343, 516+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(531, 366);
            }
            LL_HMI_Send_Pure("\x4\x80\x04\xD3\x40\x00\x1F",7);     //橙字
            while (Uart1_Busy);
            Uart1_Busy=1;
            p=gInfo.ModuleInfo.New4in1.WorkTime[4]/10;
            if (p==0)
            {
                SBUF=' ';
            }
            else
            {
                SBUF=p+'0';
            }
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1.WorkTime[4]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break; 
        case eHMICode_WorktimeParallel:  //同步模式时间
        {
            uint8_t xdata str[]=" 0min";
            p=gInfo.ModuleInfo.New4in1.WorkTime[0]/10;
            if (p!=0)
            {
                str[0]=p+'0';
            }
            str[1]=gInfo.ModuleInfo.New4in1.WorkTime[0]%10+'0';
            if (edit_mode==0)
            {
                HMI_Cut_Pic(0x71,page, 34, 280, 34+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(68, 301);
                LL_HMI_Send_Pure("\x4\x80\x04\x1C\xFF\x00\x1F",7);     //蓝字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 190, 280, 190+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(226, 301);
                LL_HMI_Send_Pure("\x4\x80\x04\xF9\x04\x00\x1F",7);    //红字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 344, 280, 344+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(380, 301);
                LL_HMI_Send_Pure("\x4\x80\x04\xD5\x00\x00\x1F",7);     //黄字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 503, 280, 503+145, 280+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(538, 301);
                LL_HMI_Send_Pure("\x4\x80\x04\xD3\x40\x00\x1F",7);     //橙字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
            }
            else
            {
                HMI_Cut_Pic(0x71,page, 47, 343, 47+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(65, 366);
                LL_HMI_Send_Pure("\x4\x80\x04\x1C\xFF\x00\x1F",7);     //蓝字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 205, 343, 205+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(217, 366);
                LL_HMI_Send_Pure("\x4\x80\x04\xF9\x04\x00\x1F",7);    //红字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 360, 343, 360+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(373, 366);
                LL_HMI_Send_Pure("\x4\x80\x04\xD3\x40\x00\x1F",7);     //橙字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
                HMI_Cut_Pic(0x71,page, 516, 343, 516+140, 343+71);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(531, 366);
                LL_HMI_Send_Pure("\x4\x80\x04\xD3\x40\x00\x1F",7);     //橙字
                LL_HMI_Send_Pure(str,5);
                LL_HMI_SendEnd();
            }
        }
            break;
        default:
            break;
    }
}

void HMI_New_Add()
{
    switch (gInfo.ModuleInfo.New4in1.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[0]>=gInfo.ModuleInfo.New4in1.PowerMax[0])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[0]=gInfo.ModuleInfo.New4in1.PowerMix[0];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[0]++;
            }
            break;
        case eHMICode_PowerLevel2:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[1]>=gInfo.ModuleInfo.New4in1.PowerMax[1])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[1]=gInfo.ModuleInfo.New4in1.PowerMix[1];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[1]++;
            }
            break;
        case eHMICode_PowerLevel3:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[2]>=gInfo.ModuleInfo.New4in1.PowerMax[2])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[2]=gInfo.ModuleInfo.New4in1.PowerMix[2];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[2]++;
            }
            break;
        case eHMICode_PowerLevel4:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[3]>=gInfo.ModuleInfo.New4in1.PowerMax[3])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[3]=gInfo.ModuleInfo.New4in1.PowerMix[3];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[3]++;
            }
            break;
        case eHMICode_Worktime1:
            if (gInfo.ModuleInfo.New4in1.WorkTime[1]>=99)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[1]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[1]++;
            }
            break;
        case eHMICode_Worktime2:
            if (gInfo.ModuleInfo.New4in1.WorkTime[2]>=99)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[2]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[2]++;
            }
            break;
        case eHMICode_Worktime3:
            if (gInfo.ModuleInfo.New4in1.WorkTime[3]>=99)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[3]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[3]++;
            }
            break;
        case eHMICode_Worktime4:
            if (gInfo.ModuleInfo.New4in1.WorkTime[4]>=99)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[4]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[4]++;
            }
            break;
        case eHMICode_WorktimeParallel:
            if (gInfo.ModuleInfo.New4in1.WorkTime[0]>=99)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[0]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[0]++;
            }
            break;
        default:
            return;
    }
    HMI_New_ShowStr(1);
}
void HMI_New_Dec()
{
    switch (gInfo.ModuleInfo.New4in1.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[0]<=gInfo.ModuleInfo.New4in1.PowerMix[0])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[0]=gInfo.ModuleInfo.New4in1.PowerMax[0];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[0]--;
            }
            break;
        case eHMICode_PowerLevel2:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[1]<=gInfo.ModuleInfo.New4in1.PowerMix[1])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[1]=gInfo.ModuleInfo.New4in1.PowerMax[1];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[1]--;
            }
            break;
        case eHMICode_PowerLevel3:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[2]<=gInfo.ModuleInfo.New4in1.PowerMix[2])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[2]=gInfo.ModuleInfo.New4in1.PowerMax[2];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[2]--;
            }
            break;
        case eHMICode_PowerLevel4:
            if (gInfo.ModuleInfo.New4in1.PowerLevel[3]<=gInfo.ModuleInfo.New4in1.PowerMix[3])
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[3]=gInfo.ModuleInfo.New4in1.PowerMax[3];
            }
            else
            {
                gInfo.ModuleInfo.New4in1.PowerLevel[3]--;
            }
            break;
        case eHMICode_Worktime1:
            if (gInfo.ModuleInfo.New4in1.WorkTime[1]<=1)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[1]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[1]--;
            }
            break;
        case eHMICode_Worktime2:
            if (gInfo.ModuleInfo.New4in1.WorkTime[2]<=1)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[2]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[2]--;
            }
            break;
        case eHMICode_Worktime3:
            if (gInfo.ModuleInfo.New4in1.WorkTime[3]<=1)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[3]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[3]--;
            }
            break;
        case eHMICode_Worktime4:
            if (gInfo.ModuleInfo.New4in1.WorkTime[4]<=1)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[4]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[4]--;
            }
            break;
        case eHMICode_WorktimeParallel:
            if (gInfo.ModuleInfo.New4in1.WorkTime[0]<=1)
            {
                gInfo.ModuleInfo.New4in1.WorkTime[0]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1.WorkTime[0]--;
            }
            break;
            break; 
        default:
            return;
    }
    HMI_New_ShowStr(1);
}

void HMI_New_Sel(uint8_t sel)
{
    HMI_New_ShowStr(0);
    gInfo.ModuleInfo.New4in1.ConfigSel=sel;
    HMI_New_ShowStr(1);
}
void HMI_New_ShowLite()         //显示智能编辑模式选项下面的模式
{
    bit step_mode=0;
    if (gInfo.ModuleInfo.New4in1.LightGroup!=0)
    {
        uint8_t i,j;
        uint8_t step_num;
        int8_t ui_offset;
        for (i = 1; i < 5; i++)     //4次
        {
            //LOG_E("SL %d   ",(uint16_t)i);
            step_mode=gInfo.ModuleInfo.New4in1.LightStep[i].StepMode;
            step_num=gInfo.ModuleInfo.New4in1.LightStep[i].StepNum;     //提高效率
            switch (step_num)
            {
                case 1:
                    ui_offset=54;
                    break;
                case 2:
                    ui_offset=32;
                    break;
                case 3:
                    ui_offset=0;
                    break;
                case 4:
                    ui_offset=-10;
                    break;
                default:
                    break;
            }
            if (gInfo.ModuleInfo.New4in1.LightGroup!=(i)) //没选的才显示
            {
                LL_HMI_Send("\x98",1);
                switch (i)      //根据不同工步显示位置
                {
                    case 1:
                        LL_HMI_SendXY(42+(i-1)*188+ui_offset,312);    
                        break;
                    case 2:
                        LL_HMI_SendXY(42+(i-1)*188+ui_offset,312);
                        break;
                    case 3:
                        LL_HMI_SendXY(42+(i-1)*188+ui_offset,312);
                        break;
                    case 4:
                        LL_HMI_SendXY(42+(i-1)*188+ui_offset,312);
                        break;
                    default:
                        LL_HMI_SendXY(0,0);
                        break;
                }
                LL_HMI_Send_Pure("\x3\x80\x03\x34\x5F\x00\x1F",7);     //蓝字
                j=0;    //进while前需清零
                while (j < step_num)   //如果为0,显示不会出错
                {
                    uint8_t light=gInfo.ModuleInfo.New4in1.LightStep[i].Data[j*3];
                    #if 0
//                    if (light&0x01)       //光波长写到治疗头里,未启用
//                    {
//                        if (j>0)
//                        {
//                            LL_HMI_Send_Pure("/",1);
//                        }
//                        LL_HMI_Send_Pure(gInfo.ModuleInfo.New4in1.Str_LightWavelength[0],
//                            strlen(gInfo.ModuleInfo.New4in1.Str_LightWavelength[0]));
//                    }
                    #endif
                    if (gCom.HMI_Scene==eScene_Module_4in1)
                    {
                        if (light&0x01)       //光波长写到治疗头里,未启用
                        {
                            if (j>0)
                            {
                                LL_HMI_Send_Pure("/",1);
                            }
                            LL_HMI_Send_Pure(STR_NEW4IN1_L1WL-2);
                        }
                        if (light&0x02)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEW4IN1_L2WL-2);
                        }
                        if (light&0x04)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEW4IN1_L3WL-2);
                        }
                        if (light&0x08)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEW4IN1_L4WL-2);
                        }
                    }
                    else if (gCom.HMI_Scene==eScene_Module_Wira)
                    {
                        if (light&0x01)       //光波长写到治疗头里,未启用
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEWWIRA_L1WL-2);
                        }
                        if (light&0x02)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEWWIRA_L2WL-2);
                        }
                        if (light&0x04)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEWWIRA_L3WL-2);
                        }
                        if (light&0x08)
                        {
                            if (j>0)
                            {   
                                if (step_mode==STEP_MODE_Serial)
                                {
                                    LL_HMI_Send_Pure("/",1);
                                }
                                else
                                {
                                    LL_HMI_Send_Pure("+",1);
                                }
                            }
                            LL_HMI_Send_Pure(STR_NEWWIRA_L4WL-2);
                        }
                    }
                    else
                    {
                        ;//MUST not be here
                    }
                    j++;
                }
                LL_HMI_SendEnd();
            }
            else
            {
                ;//do nothing 
            }
        }
    }
    else
    {
        ; //专家模式 //do nothing
    }
}
void HMI_New_ShowDetail(uint8_t countdown)       //显示选择后的详细信息 countdown==1 倒计时模式
{
    bit step_mode;
    uint8_t light_group,step_num,light,power;
    uint16_t time;
    uint16_t x;
    uint8_t i;
    uint8_t xdata power_str[]=" 00mW/cm\xB2";
    uint8_t xdata time_str[]=" 0min";
    uint8_t xdata remain_time_str[]=" 0min00s";
    light_group=gInfo.ModuleInfo.New4in1.LightGroup;
    step_mode = gInfo.ModuleInfo.New4in1.LightStep[light_group].StepMode;
    step_num =  gInfo.ModuleInfo.New4in1.LightStep[light_group].StepNum;
    if (step_num==0)
    {
        return;     //如果没有工步直接返回
    }
    switch (light_group)        //同步/顺序模式元素显示
    {
        case 1:   
        case 2:
        case 3:
        case 4:
            LL_HMI_Send("\x71\x3D",2);      //剪切指令
            x=450+90*step_mode+140*gConfig.LANG; //计算剪切元素
            LL_HMI_SendXY(x,0);
            LL_HMI_SendXY(x+47,47);
            LL_HMI_SendXY(94+(light_group-1)*188,275);  //同步顺序模式元素显示位置
            LL_HMI_SendEnd();
            break;
        case 0:
        default:    // 专家模式或未选择模式直接返回
            return;
    }
    HMI_Cut_Pic(0x71,41, 0, 370, 800, 480); //恢复背景

    for (i = 0; i < step_num; i++)
    {
        light=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3];
        LL_HMI_Send("\x98",1);
        switch (i)
        {
            case 0:
                LL_HMI_SendXY(82,399);
                break;
            case 1:
                LL_HMI_SendXY(430,399);
                break;
            case 2:
                LL_HMI_SendXY(82,439);
                break;
            case 3:
                LL_HMI_SendXY(430,439);
                break;
            default:
                LL_HMI_SendXY(0,0); //Should not be here
                break;
        }
        if (countdown==1 && (gInfo.ModuleInfo.New4in1.LocStep == i || step_mode==STEP_MODE_Parallel))
        {
            LL_HMI_Send_Pure("\x3\x80\x03\x0\x0\x00\x1F",7);     //黑字
        }
        else
        {
            LL_HMI_Send_Pure("\x3\x80\x03\x34\x5F\x00\x1F",7);     //蓝字
        }
        if (gCom.HMI_Scene==eScene_Module_4in1)
        {
            if (light&0x01)      
            {
                LL_HMI_Send_Pure(STR_NEW4IN1_L1WL);
            }
            else if (light&0x02)
            {
                LL_HMI_Send_Pure(STR_NEW4IN1_L2WL);
            }
            else if (light&0x04)
            {
                LL_HMI_Send_Pure(STR_NEW4IN1_L3WL);
            }
            else if (light&0x08)
            {
                LL_HMI_Send_Pure(STR_NEW4IN1_L4WL);
            }
        }
        else if (gCom.HMI_Scene==eScene_Module_Wira)
        {
            if (light&0x01)      
            {
                LL_HMI_Send_Pure(STR_NEWWIRA_L1WL);
            }
            else if (light&0x02)
            {
                LL_HMI_Send_Pure(STR_NEWWIRA_L2WL);
            }
            else if (light&0x04)
            {
                LL_HMI_Send_Pure(STR_NEWWIRA_L3WL);
            }
            else if (light&0x08)
            {
                LL_HMI_Send_Pure(STR_NEWWIRA_L4WL);
            }
        }
        if (countdown==1 && (gInfo.ModuleInfo.New4in1.LocStep == i || step_mode==STEP_MODE_Parallel))
        {
            LL_HMI_Send_Pure("    ",2);     //中间空格
        }
        else
        {
            LL_HMI_Send_Pure("    ",3);     //中间空格
        }
        power=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3+1];
        if (power/100>0)
        {
            power_str[0]=power/100+'0';
        }
        power=power%100;
        power_str[1]=power/10+'0';
        power_str[2]=power%10+'0';
        LL_HMI_Send_Pure(power_str,9);
        if (countdown==1 && (gInfo.ModuleInfo.New4in1.LocStep == i || step_mode==STEP_MODE_Parallel))
        {   //" 0min00s";
            LL_HMI_Send_Pure("    ",1);     //中间空格
            time=gInfo.ModuleInfo.New4in1.RemainTime/600;
            if (time!=0)
            {
                remain_time_str[0]=time+'0';
            }
            time=gInfo.ModuleInfo.New4in1.RemainTime%600;
            remain_time_str[1]=time/60+'0';
            time=time%60;
            remain_time_str[5]=time/10+'0';
            remain_time_str[6]=time%10+'0';
            LL_HMI_Send_Pure(remain_time_str,8);
        }
        else
        {
            LL_HMI_Send_Pure("    ",2);     //中间空格
            time=gInfo.ModuleInfo.New4in1.LightStep[light_group].Data[i*3+2];
            if (time/10!=0)
            {
                time_str[0]=time/10+'0';
            }
            time_str[1]=time%10+'0';
            LL_HMI_Send_Pure(time_str,5);
        }
        LL_HMI_SendEnd();
    }
}

void HMI_New_ShowEditMode() //刷新智能编辑模式
{
    uint8_t i=0;
    uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
    uint16_t y;
    gInfo.ModuleInfo.New4in1.LastSelGroup=light_group;
    switch (light_group)    //根据不同选择,显示不同tab
    {
        case 1:
        case 2:
        case 3:
        case 4:
            y=118+(light_group)*97;
            if (light_group==3)     //修正图片偏差
            {
                y+=2;
            }
            else if(light_group==4)
            {
                y++;
            }
            LL_HMI_Send("\x71\x3D",2);      //剪切指令 页面0x3D 61
            LL_HMI_SendXY(0,y);
            LL_HMI_SendXY(800,y+75);
            LL_HMI_SendXY(0,92);  //tab元素显示位置
            LL_HMI_SendEnd();
            break;
        default:
            light_group=1;  //强制选择光1
            //没有break
    }
    HMI_New_Show_LightName(gInfo.ModuleInfo.New4in1.ConfigSelLight|0x80);
    for (i = 0xc; i <= 0x13; i++)
    {
        gInfo.ModuleInfo.New4in1.ConfigSel=i;
        HMI_New_ShowStr(0);
    }
}

void HMI_New_LoadLightStep() //加载工步信息
{
    uint8_t i=0;
    uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
    uint8_t max_time=(gCom.ModuleType==M_Type_Wira)?30:10;
    uint8_t step_num=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepNum;
    uint8_t step_mode=gInfo.ModuleInfo.New4in1.LightStep[light_group].StepMode;
    gInfo.ModuleInfo.New4in1.ConfigSelLight=0;
    if(light_group==0 || gInfo.ModuleInfo.New4in1.EditMode==1)
    {
        memcpy(&gInfo.ModuleInfo.New4in1.TempStep,&gInfo.ModuleInfo.New4in1.LightStep[light_group],13);
        for(i=0;i<4;i++)        //设置默认的光能量
        {
            gInfo.ModuleInfo.New4in1.PowerLevel[i]=gInfo.ModuleInfo.New4in1.PowerMax[i];
        }
        for (i = 0; i <=5; i++) //设置默认的时间
        {
            gInfo.ModuleInfo.New4in1.WorkTime[i]=max_time;
        }
        if (step_mode==STEP_MODE_Serial)
        {
            for (i = 0; i < step_num; i++)
            {
                gInfo.ModuleInfo.New4in1.ConfigSelLight |= gInfo.ModuleInfo.New4in1.TempStep.Data[i*3];
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        gInfo.ModuleInfo.New4in1.PowerLevel[0]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        gInfo.ModuleInfo.New4in1.WorkTime[1]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+2];
                        break;
                    case 0x02:
                        gInfo.ModuleInfo.New4in1.PowerLevel[1]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        gInfo.ModuleInfo.New4in1.WorkTime[2]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+2];
                        break;
                    case 0x04:
                        gInfo.ModuleInfo.New4in1.PowerLevel[2]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        gInfo.ModuleInfo.New4in1.WorkTime[3]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+2];
                        break;
                    case 0x08:
                        gInfo.ModuleInfo.New4in1.PowerLevel[3]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        gInfo.ModuleInfo.New4in1.WorkTime[4]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+2];
                        break;
                }
            }
        }
        else
        {
            gInfo.ModuleInfo.New4in1.WorkTime[0]=gInfo.ModuleInfo.New4in1.TempStep.Data[2];
            for (i = 0; i < step_num; i++)
            {
                gInfo.ModuleInfo.New4in1.ConfigSelLight |= gInfo.ModuleInfo.New4in1.TempStep.Data[i*3];
                
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        gInfo.ModuleInfo.New4in1.PowerLevel[0]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        break;
                    case 0x02:
                        gInfo.ModuleInfo.New4in1.PowerLevel[1]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        break;
                    case 0x04:
                        gInfo.ModuleInfo.New4in1.PowerLevel[2]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        break;
                    case 0x08:
                        gInfo.ModuleInfo.New4in1.PowerLevel[3]=gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1];
                        break;
                }
            }
        }
        
    }
}

void HMI_New_SaveLightStep()        //保存设置的工步
{
    uint8_t light_group=gInfo.ModuleInfo.New4in1.LightGroup;
    //TODO:能量时间更新
    
    memcpy(&gInfo.ModuleInfo.New4in1.LightStep[light_group],&gInfo.ModuleInfo.New4in1.TempStep,13);
}

void HMI_New_ShowList() //刷新列表
{
    uint8_t step_num=gInfo.ModuleInfo.New4in1.TempStep.StepNum;
    uint8_t i;
    if (gInfo.ModuleInfo.New4in1.EditMode==0)
    {
        HMI_Cut_Pic(0x71,45, 48, 378, 48+493, 378+72); //恢复背景
        if (gCom.HMI_Scene==eScene_Module_4in1)
        {
            for (i = 0; i < step_num; i++)
            {
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(52,382);
                        LL_HMI_SendXY(52+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L1WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x02:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(171,382);
                        LL_HMI_SendXY(171+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L2WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x04:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(290,382);
                        LL_HMI_SendXY(290+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L3WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x08:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(409,382);
                        LL_HMI_SendXY(409+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L4WL);
                        LL_HMI_SendEnd();
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            for (i = 0; i < step_num; i++)
            {
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(52,382);
                        LL_HMI_SendXY(52+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L1WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x02:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(171,382);
                        LL_HMI_SendXY(171+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L2WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x04:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(290,382);
                        LL_HMI_SendXY(290+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L3WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x08:
                        LL_HMI_Send("\x71\x2E",2);      //剪切指令 页面0x3D 46
                        LL_HMI_SendXY(409,382);
                        LL_HMI_SendXY(409+113,382+62);
                        LL_HMI_SendXY(52+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(65+i*119, 399);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L4WL);
                        LL_HMI_SendEnd();
                        break;
                    default:
                        break;
                }
            }
        }
        
    }
    else
    {
        HMI_Cut_Pic(0x71,47, 57, 434, 57+477, 434+69); //恢复背景
        if (gCom.HMI_Scene==eScene_Module_4in1)
        {
            for (i = 0; i < step_num; i++)
            {
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(61,438);
                        LL_HMI_SendXY(61+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L1WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x02:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(180,438);
                        LL_HMI_SendXY(180+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L2WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x04:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(299,438);
                        LL_HMI_SendXY(299+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L3WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x08:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(418,438);
                        LL_HMI_SendXY(418+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEW4IN1_L4WL);
                        LL_HMI_SendEnd();
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            for (i = 0; i < step_num; i++)
            {
                switch (gInfo.ModuleInfo.New4in1.TempStep.Data[i*3])
                {
                    case 0x01:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(61,438);
                        LL_HMI_SendXY(61+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L1WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x02:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(180,438);
                        LL_HMI_SendXY(180+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L2WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x04:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(299,438);
                        LL_HMI_SendXY(299+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L3WL);
                        LL_HMI_SendEnd();
                        break;
                    case 0x08:
                        LL_HMI_Send("\x71\x30",2);      //剪切指令 页面0x3D 48
                        LL_HMI_SendXY(418,438);
                        LL_HMI_SendXY(418+113,438+62);
                        LL_HMI_SendXY(61+i*119,382);  //元素显示位置
                        LL_HMI_SendEnd();
                        LL_HMI_Send("\x98",1);
                        LL_HMI_SendXY(70+i*119, 449);
                        LL_HMI_Send_Pure("\x6\x80\x05\xFF\xFF\x00\x1F",7); 
                        LL_HMI_Send_Pure(STR_NEWWIRA_L4WL);
                        LL_HMI_SendEnd();
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void HMI_New_Add_Step(uint8_t l)   //增加工步/光             0-3
{
    uint8_t step_num=gInfo.ModuleInfo.New4in1.TempStep.StepNum;
    bit step_mode=gInfo.ModuleInfo.New4in1.TempStep.StepMode;
    if (step_num <= 4)
    {
        switch (l)
        {
            case 0x01:
                gInfo.ModuleInfo.New4in1.TempStep.StepNum++;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3]=0x01;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+1]=gInfo.ModuleInfo.New4in1.PowerLevel[0];
                if (step_mode==STEP_MODE_Serial)
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[1];
                }
                else
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[0];
                }
                break;
            case 0x02:
                gInfo.ModuleInfo.New4in1.TempStep.StepNum++;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3]=0x02;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+1]=gInfo.ModuleInfo.New4in1.PowerLevel[1];
                if (step_mode==STEP_MODE_Serial)
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[2];
                }
                else
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[0];
                }
                break;
            case 0x04:
                gInfo.ModuleInfo.New4in1.TempStep.StepNum++;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3]=0x04;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+1]=gInfo.ModuleInfo.New4in1.PowerLevel[2];
                if (step_mode==STEP_MODE_Serial)
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[3];
                }
                else
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[0];
                }
                break;
            case 0x08:
                gInfo.ModuleInfo.New4in1.TempStep.StepNum++;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3]=0x08;
                gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+1]=gInfo.ModuleInfo.New4in1.PowerLevel[3];
                if (step_mode==STEP_MODE_Serial)
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[4];
                }
                else
                {
                    gInfo.ModuleInfo.New4in1.TempStep.Data[step_num*3+2]=gInfo.ModuleInfo.New4in1.WorkTime[0];
                }
                break;
             default:
                break;
        }
        HMI_New_ShowList();
    }
    else
    {
        ;//do nothing
    }
}

void HMI_New_Dec_Step(uint8_t l)   //减工步/光           0-3
{
    uint8_t step_num=gInfo.ModuleInfo.New4in1.TempStep.StepNum;
    bit step_mode=gInfo.ModuleInfo.New4in1.TempStep.StepMode;
    uint8_t i=0;
    if (step_num<=4)
    {
        for (i = 0; i < step_num; i++)
        {
            if(gInfo.ModuleInfo.New4in1.TempStep.Data[i*3]==l)
            {
                gInfo.ModuleInfo.New4in1.TempStep.StepNum--;
                gInfo.ModuleInfo.New4in1.TempStep.Data[i*3]=0;
                gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+1]=0;
                gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+2]=0;
                memcpy(&gInfo.ModuleInfo.New4in1.TempStep.Data[i*3],&gInfo.ModuleInfo.New4in1.TempStep.Data[i*3+3],(step_num-i-1)*3);
                break;
            }
        }
        HMI_New_ShowList();
    }
    else
    {
        ;//do nothing
    }
    
}

