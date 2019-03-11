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
            case 0xE4:  //校准屏幕返回消息
                gComInfo.HMIMsg=eMSg_CailDone;
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
    if(gInfo.ModuleInfo.RoutineModule.LightMode==0)
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

void HMI_Show_Temp(int16_t temp)
{
    switch (gComInfo.HMI_Scene)     //显示部分
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
            LL_HMI_Send_Pure("\x21\x81\x03\x00\xF8\x00\x1F",7); //红字
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
    LL_HMI_Send_Pure("\x21\x81\x03\x00\x1F\x00\x1F",7);
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
    if (gComInfo.FeedbackVolt<1000)
    {
        fv=gComInfo.FeedbackVolt%1000;
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
        fv=gComInfo.FeedbackVolt%1000;
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
    uint16_t power=gInfo.ModuleInfo.mini308Module.WorkTime*3;
    uint8_t xdata time_str[3]={0x20};
    page=ePage_Module308+gConfig.LANG*100;
    HMI_Cut_Pic(0x71,page, 296, 208, 296+191, 208+150); //背景恢复
    
    //显示时间
    LL_HMI_Send("\x98",1);
    t=gInfo.ModuleInfo.mini308Module.WorkTime/100;
    if (t==0)
    {
        t=gInfo.ModuleInfo.mini308Module.WorkTime%100;
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
        t=gInfo.ModuleInfo.mini308Module.WorkTime%100;
        time_str[1]='0'+t/10;
        time_str[2]='0'+t%10;
    }
    LL_HMI_Send_Pure("\x03\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(time_str,3);
    LL_HMI_SendEnd();
    
    //显示能量
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(362,295);
    LL_HMI_Send_Pure("\x03\x80\x05\x01\xAF\x0\x1f",7);
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
    uint8_t xdata power_str[]={"0.00J/cm2"};
    if (gComInfo.HMI_Scene==eScene_Module_308)
    {
        uint16_t p;
        HMI_Cut_Pic(0x71,ePage_Module308, 162, 409, 162+464, 407+33);     //进度条背景恢复
        p=464/gInfo.ModuleInfo.mini308Module.WorkTime*(gInfo.ModuleInfo.mini308Module.WorkTime-gInfo.ModuleInfo.mini308Module.RemainTime);
        HMI_Cut_Pic(0x71,ePage_Module308+1, 162, 409, 162+(p), 407+33);
        power=(gInfo.ModuleInfo.mini308Module.WorkTime-gInfo.ModuleInfo.mini308Module.RemainTime)*3;
        //进度条显示剩余时间
        LL_HMI_Send("\x98",1);
        t=gInfo.ModuleInfo.mini308Module.RemainTime/100;
        if (t==0)
        {
            t=gInfo.ModuleInfo.mini308Module.RemainTime%100;
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
            t=gInfo.ModuleInfo.mini308Module.RemainTime%100;
            time_str[1]='0'+t/10;
            time_str[2]='0'+t%10;
        }
        time_str[3]='s';
        LL_HMI_SendXY(553,408);
        LL_HMI_Send_Pure("\x21\x81\x02\x01\xAF\x0\x1f",7);
        LL_HMI_Send_Pure(time_str,4);
        LL_HMI_SendEnd();
        
        //进度条显示能量
        power_str[0]=power/100+'0';
        power=power%100;
        power_str[2]=power/10+'0';
        power_str[3]=power%10+'0';
        LL_HMI_Send("\x98",1);
        LL_HMI_SendXY(179,408);
        LL_HMI_Send_Pure("\x21\x81\x02\x01\xAF\x0\x1f",7);
        LL_HMI_Send_Pure(power_str,9);
        LL_HMI_SendEnd();
    }
    else    //红斑测试
    {
        uint16_t p;
        HMI_Cut_Pic(0x71,ePage_Module308Test, 178, 399, 178+440, 399+39);     //进度条背景恢复
        p=440/gInfo.ModuleInfo.mini308Module.TestWorkTime*(gInfo.ModuleInfo.mini308Module.TestWorkTime-gInfo.ModuleInfo.mini308Module.RemainTime);
        HMI_Cut_Pic(0x71,ePage_Module308Test+1, 178, 399, 178+p, 399+39); 
    }
}

void HMI_308Test_SelTime(uint8_t seltime)
{
    uint8_t xdata power_str[4]="0.00";
    uint8_t power=seltime*3;
    switch (gInfo.ModuleInfo.mini308Module.TestSelTime)
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
    gInfo.ModuleInfo.mini308Module.TestSelTime=seltime;
    switch (gInfo.ModuleInfo.mini308Module.TestSelTime)
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
    uint8_t power=gInfo.ModuleInfo.mini308Module.TotalTime*3;
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
    f=gInfo.ModuleInfo.mini308Module.Freq/100;
    if (f!=0)
    {
        freq_str[0]=f+'0';
    }
    else
    {
        freq_str[0]=' ';
    }
    f=gInfo.ModuleInfo.mini308Module.Freq%100;
    freq_str[1]=f/10+'0';
    freq_str[2]=f%10+'0';
    HMI_Cut_Pic(0x71,13, 398, 203, 398+95, 203+52);
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(413,209);
    LL_HMI_Send_Pure("\x03\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(freq_str,3);
    LL_HMI_SendEnd();
}

void HMI_308Set_Duty()
{
    uint8_t f;
    uint8_t xdata duty_str[2];
    f=gInfo.ModuleInfo.mini308Module.Duty;
    duty_str[0]=f/10+'0';
    duty_str[1]=f%10+'0';
    HMI_Cut_Pic(0x71,13, 411, 336, 411+82, 336+59);
    LL_HMI_Send("\x98",1);
    LL_HMI_SendXY(420,347);
    LL_HMI_Send_Pure("\x03\x80\x05\x01\xAF\x0\x1f",7);
    LL_HMI_Send_Pure(duty_str,2);
    LL_HMI_SendEnd();
}

void HMI_New_Show_Light(uint8_t sellight)
{
    switch (gComInfo.HMI_Scene)
    {
        case eScene_Module_4in1:
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x01) != (sellight & 0x01)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x01)? 46:45;
                HMI_Cut_Pic(0x71,page, 38, 94, 38+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(67, 121);
                if(sellight & 0x01)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("590nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x02) != (sellight & 0x02)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x02)? 46:45;
                HMI_Cut_Pic(0x71,page, 193, 94, 193+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(221, 121);
                if(sellight & 0x02)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("830nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x04) != (sellight & 0x04)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x04)? 46:45;
                HMI_Cut_Pic(0x71,page, 348, 94, 348+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(378, 121);
                if(sellight & 0x04)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("415nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x08) != (sellight & 0x08)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x08)? 46:45;
                HMI_Cut_Pic(0x71,page, 505, 94, 505+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(534, 121);
                if(sellight & 0x08)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("633nm",5);
                LL_HMI_SendEnd();
            }
            gInfo.ModuleInfo.New4in1Module.ConfigSelLight=sellight;
            break;
        case eScene_Module_Wira:
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x01) != (sellight & 0x01)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x01)? 46:45;
                HMI_Cut_Pic(0x71,page, 38, 94, 38+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(67, 121);
                if(sellight & 0x01)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("633nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x02) != (sellight & 0x02)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x02)? 46:45;
                HMI_Cut_Pic(0x71,page, 193, 94, 193+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(221, 121);
                if(sellight & 0x02)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("810nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x04) != (sellight & 0x04)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x04)? 46:45;
                HMI_Cut_Pic(0x71,page, 348, 94, 348+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(378, 121);
                if(sellight & 0x04)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("940nm",5);
                LL_HMI_SendEnd();
            }
            if (((gInfo.ModuleInfo.New4in1Module.ConfigSelLight & 0x08) != (sellight & 0x08)) || (sellight & 0x80))
            {
                uint8_t page = (sellight & 0x08)? 46:45;
                HMI_Cut_Pic(0x71,page, 505, 94, 505+145, 94+75);      //背景恢复
                LL_HMI_Send("\x98",1);
                LL_HMI_SendXY(534, 121);
                if(sellight & 0x08)
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\x0\x0\x00\x1F",7);     //黑字
                }
                else
                {
                    LL_HMI_Send_Pure("\x21\x81\x03\xFF\xFF\x00\x1F",7);     //白字
                }
                LL_HMI_Send_Pure("830nm",5);
                LL_HMI_SendEnd();
            }
            gInfo.ModuleInfo.New4in1Module.ConfigSelLight=sellight&0x7F;
            break;
        default:
            break;
    }
}

void HMI_New_ShowStr(uint8_t sel)
{
    uint8_t page=sel==1?46:45;
    switch (gInfo.ModuleInfo.New4in1Module.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            HMI_Cut_Pic(0x71,page, 35, 189, 35+145, 189+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(80, 215);
            LL_HMI_Send_Pure("\x21\x81\x03\x1C\xFF\x00\x1F",7);     //蓝字
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[0]/100==0)
            {
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=' ';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[0]/10+'0';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[0]%10+'0';
            }
            else
            {
                LL_HMI_Send_Pure("100",3);
            }
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel2:
            HMI_Cut_Pic(0x71,page, 191, 189, 191+145, 189+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(235, 215);
            LL_HMI_Send_Pure("\x21\x81\x03\xF9\x04,\x00\x1F",7);    //红字
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[1]/100==0)
            {
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=' ';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[1]/10+'0';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[1]%10+'0';
            }
            else
            {
                LL_HMI_Send_Pure("100",3);
            }
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel3:
            HMI_Cut_Pic(0x71,page, 349, 189, 349+145, 189+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(388, 215);
            LL_HMI_Send_Pure("\x21\x81\x03\xD5\x00\x00\x1F",7);     //黄字
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[2]/100==0)
            {
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=' ';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[2]/10+'0';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[2]%10+'0';
            }
            else
            {
                LL_HMI_Send_Pure("100",3);
            }
            LL_HMI_SendEnd();
            break;
        case eHMICode_PowerLevel4:
            HMI_Cut_Pic(0x71,page, 514, 189, 514+145, 189+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(548, 215);
            LL_HMI_Send_Pure("\x21\x81\x03\xD3\x40\x00\x1F",7);     //橙字
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[3]/100==0)
            {
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=' ';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[3]/10+'0';
                while (Uart1_Busy);
                Uart1_Busy=1;
                SBUF=gInfo.ModuleInfo.New4in1Module.PowerLevel[3]%10+'0';
            }
            else
            {
                LL_HMI_Send_Pure("100",3);
            }
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime1:
            HMI_Cut_Pic(0x71,page, 34, 280, 34+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(68, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\x1C\xFF\x00\x1F",7);     //蓝字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[1]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[1]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime2:
            HMI_Cut_Pic(0x71,page, 190, 280, 190+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(226, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xF9\x04\x00\x1F",7);    //红字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[2]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[2]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime3:
            HMI_Cut_Pic(0x71,page, 344, 280, 344+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(380, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xD5\x00\x00\x1F",7);     //黄字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[3]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[3]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        case eHMICode_Worktime4:
            HMI_Cut_Pic(0x71,page, 503, 280, 503+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(538, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xD3\x40\x00\x1F",7);     //橙字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[4]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[4]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break; 
        case eHMICode_WorktimeParallel:  //同步模式时间
            HMI_Cut_Pic(0x71,page, 34, 280, 34+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(68, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\x1C\xFF\x00\x1F",7);     //蓝字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            HMI_Cut_Pic(0x71,page, 190, 280, 190+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(226, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xF9\x04\x00\x1F",7);    //红字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            HMI_Cut_Pic(0x71,page, 344, 280, 344+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(380, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xD5\x00\x00\x1F",7);     //黄字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            HMI_Cut_Pic(0x71,page, 503, 280, 503+145, 280+75);      //背景恢复
            LL_HMI_Send("\x98",1);
            LL_HMI_SendXY(538, 301);
            LL_HMI_Send_Pure("\x21\x81\x03\xD3\x40\x00\x1F",7);     //橙字
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]/10+'0';
            while (Uart1_Busy);
            Uart1_Busy=1;
            SBUF=gInfo.ModuleInfo.New4in1Module.WorkTime[0]%10+'0';
            LL_HMI_Send_Pure("min",3);
            LL_HMI_SendEnd();
            break;
        default:
            break;
    }
}

void HMI_New_Add()
{
    switch (gInfo.ModuleInfo.New4in1Module.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[0]>99)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[0]=0;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[0]++;
            }
            break;
        case eHMICode_PowerLevel2:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[1]>99)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[1]=0;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[1]++;
            }
            break;
        case eHMICode_PowerLevel3:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[2]>99)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[2]=0;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[2]++;
            }
            break;
        case eHMICode_PowerLevel4:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[3]>99)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[3]=0;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[3]++;
            }
            break;
        case eHMICode_Worktime1:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[1]>=99)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[1]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[1]++;
            }
            break;
        case eHMICode_Worktime2:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[2]>=99)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[2]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[2]++;
            }
            break;
        case eHMICode_Worktime3:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[3]>=99)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[3]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[3]++;
            }
            break;
        case eHMICode_Worktime4:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[4]>=99)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[4]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[4]++;
            }
            break;
        case eHMICode_WorktimeParallel:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[0]>=99)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[0]=1;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[0]++;
            }
            break;
        default:
            return;
    }
    HMI_New_ShowStr(1);
}
void HMI_New_Dec()
{
    switch (gInfo.ModuleInfo.New4in1Module.ConfigSel)
    {
        case eHMICode_PowerLevel1:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[0]==0)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[0]=100;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[0]--;
            }
            break;
        case eHMICode_PowerLevel2:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[1]==0)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[1]=100;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[1]--;
            }
            break;
        case eHMICode_PowerLevel3:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[2]==0)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[2]=100;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[2]--;
            }
            break;
        case eHMICode_PowerLevel4:
            if (gInfo.ModuleInfo.New4in1Module.PowerLevel[3]==0)
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[3]=100;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.PowerLevel[3]--;
            }
            break;
        case eHMICode_Worktime1:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[1]<=1)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[1]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[1]--;
            }
            break;
        case eHMICode_Worktime2:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[2]<=1)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[2]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[2]--;
            }
            break;
        case eHMICode_Worktime3:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[3]<=1)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[3]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[3]--;
            }
            break;
        case eHMICode_Worktime4:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[4]<=1)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[4]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[4]--;
            }
            break;
        case eHMICode_WorktimeParallel:
            if (gInfo.ModuleInfo.New4in1Module.WorkTime[0]<=1)
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[0]=99;
            }
            else
            {
                gInfo.ModuleInfo.New4in1Module.WorkTime[0]--;
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
    gInfo.ModuleInfo.New4in1Module.ConfigSel=sel;
    HMI_New_ShowStr(1);
}

void HMI_New_SwitchMode()
{

}
