#include "Module_COMM.h"
#include "HMI_COMM.h"
#include "WorkProcess.h"
#include "HMIProcess.h"



void LL_Module_Send(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Head;
    for (i = 0; i < str_len; i++)
    {
        while (Uart2_Busy);
        Uart2_Busy=1;
        S2BUF=*ptr;
        ptr++;
    }
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Tail1;
    while (Uart2_Busy);
    Uart2_Busy=1;
    S2BUF=gComInfo.COMMProtocol_Tail2;

}


void Module_COMM()
{
    if (Uart2_ReviceFrame==0)
    {
        return;
    }
    else
    {
        uint8_t xdata pbuf[18];
        uint8_t data_size;
        memcpy(pbuf,uart2_buff,18);
        data_size=pbuf[0];
        Uart2_ReviceFrame=0;
        if (gComInfo.COMMProtocol_Head==0xAA)
        {
            if (pbuf[1]==0x21)
            {
                uint8_t cmd=pbuf[2];
                switch (cmd)
                {
                    case 0x01:      //握手回复治疗头类型
                        gComInfo.ModuleType=pbuf[3];
                        LOG_E("Module Type: %02X",(uint16_t)gComInfo.ModuleType);
                        SystemTime1s=0;
                        gInfo.DebugOpen=OPEN_DBG_Calib|OPEN_DBG_ClearUsedtime|OPEN_DBG_Config;  //给治疗头授权
                        switch (gComInfo.ModuleType)
                        {
                            case M_Type_650:
                                gComInfo.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;  //默认10分钟
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=1800;   //治疗头没回复能量大小
                                gComInfo.HMI_Scene=eScene_Module_650;
                                HMI_Goto_LocPage(2);
                                HMI_Show_ModuleName("Derma-650");
                                HMI_Show_WorkMode();
                                HMI_Show_Worktime1();
                                ModuleRoutine_GetUsedTime();
                                HMI_Show_Power();
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gComInfo.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=150;   //633治疗头没回复能量大小
                                gComInfo.HMI_Scene=eScene_Module_633;
                                HMI_Goto_LocPage(3);
                                HMI_Show_ModuleName("Derma-633");
                                HMI_Show_WorkMode();
                                HMI_Show_Worktime1();
                                ModuleRoutine_GetUsedTime();
                                HMI_Show_Power();
                                break;
                          //case M_Type_IU:     //IU是另外协议,在此不会收到
                            case M_Type_UVA1:
                                gComInfo.WorkStat=eWS_Standby;
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;
                                gComInfo.HMI_Scene=eScene_Module_UVA1;
                                HMI_Goto_LocPage(4);
                                HMI_Show_ModuleName("Derma-UVA1");
                                HMI_Show_Worktime2();
                                ModuleRoutine_GetUsedTime();
                                break;
                            case M_Type_Wira:
                                gComInfo.WorkStat=eWS_Standby;
                                break;
                            case M_Type_4in1:
                                gComInfo.WorkStat=eWS_Standby;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0x03:      //光1光功率
                        LOG_E("Get PowerLevel %u",(uint16_t)pbuf[3]);
                        switch (gComInfo.ModuleType)
                        {
                            case M_Type_650:    //1800mw
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=pbuf[3]*10;
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=pbuf[3];
                                break;
                            default:
                                break;
                        }
                        HMI_Show_Power();
                        break;
                    case 0x04:      //光2光功率
                        break;
                    default:
                        break;
                }
                
            }
            else if(pbuf[1]==0x22)      //光1使用时间
            {
                
            }
            else if(pbuf[1]==0x20)
            {
                switch (gComInfo.HMI_Scene)
                {
                    case eScene_Module_UVA1:
                        if (pbuf[2]==0)     //读到数据且正确
                        {
                            int8_t temp=pbuf[3];
                            gComInfo.TempCount=0;
                            HMI_Cut_Pic(0x71,gConfig.LANG*100 + 2, 13, 415, 13+81, 415+36); //恢复温度背景
                            LL_HMI_Send("\x98\x00\x37\x01\x9F\x21\x81\x03\x00\x1F\x00\x1F",12);
                            if (temp<0)     //>128
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
                        else
                        {
                            ;//do nothing
                        }
                        break;
                }
            }
            else
            {
                ;   //do nothing
            }
        }
        else    //gComInfo.COMMProtocol_Head=='@'
        {
            
        }
    }
}


void ModuleRoutine_Shakehand()
{
    LL_Module_Send("\x39\x21\x00",3);
}

void Module308_Shakehand()
{
    LL_Module_Send("1*11&5&9",8);
}

void ModuleRoutine_GetUsedTime()
{
    LL_Module_Send("\x39\x25\x01", 3);
}

void Module_GetTemp()
{
    LL_Module_Send("\x39\x20\x01", 3);
}

void Module_Send_PWM(uint8_t light_sel,uint8_t duty)
{
    uint8_t xdata cmd[]={"\x39\x31\x0\x0"};
    cmd[2]=light_sel;
    cmd[3]=duty;
    LL_Module_Send(cmd,4);
}
