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
    while (Uart2_Busy);     //等待发送完
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
                        gComInfo.TimerCounter=0;
                        SystemTime1s=0;
                        gInfo.DebugOpen=OPEN_DBG_Calib|OPEN_DBG_ClearUsedtime|OPEN_DBG_Config;  //给治疗头授权 
                        //TODO: 不同治疗头授权不一样
                        switch (gComInfo.ModuleType)    //治疗头数据初始化
                        {
                            case M_Type_650:
                                gComInfo.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;  //默认10分钟
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=1800;   //治疗头没回复能量大小
                                gComInfo.HMI_LastScene=eScene_Module_650;
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gComInfo.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;
                                gInfo.ModuleInfo.RoutineModule.PowerLevel=150;   //633治疗头没回复能量大小
                                gComInfo.HMI_LastScene=eScene_Module_633;
                                break;
                          //case M_Type_IU:     //IU是另外协议,在此不会收到
                            case M_Type_UVA1:
                                gComInfo.WorkStat=eWS_Standby;
                                gInfo.ModuleInfo.RoutineModule.WorkTime=10;
                                gComInfo.HMI_LastScene=eScene_Module_UVA1;
                                break;
                            case M_Type_Wira:
                                gComInfo.WorkStat=eWS_Standby;
                                gComInfo.HMI_LastScene=eScene_Module_Wira;
                                break;
                            case M_Type_4in1:
                                gComInfo.WorkStat=eWS_Standby;

                                gComInfo.HMI_LastScene=eScene_Module_4in1;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0x03:      //光1光功率
                        //LOG_E("Get PowerLevel %u",(uint16_t)pbuf[3]);
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
                    case 0x0B:      //DAC Vfb
                        Resend_getCalibData=0;
                        switch (gComInfo.ModuleType)    //治疗头数据初始化
                        {
                            case M_Type_633:
                            case M_Type_633_1:
                                gInfo.ModuleInfo.RoutineModule.DAC_Cail=pbuf[3]<<8|pbuf[4];
                                gComInfo.FeedbackVolt=pbuf[5]<<8|pbuf[6];
                                break;
                            case M_Type_UVA1:
                            case M_Type_Wira:
                            case M_Type_4in1:
                                break;
                            default:
                                break;
                        }
                        if (gComInfo.HMI_Scene==eScene_Debug)
                        {
                            char xdata str[6]={0};
                            HMI_Cut_Pic(0x71,62, 657, 21, 657+117, 21+53);  //Vfb背景恢复
                            HMI_Cut_Pic(0x71,62, 109, 396, 109+106, 396+61);  //DA背景恢复
                            LL_HMI_Send("\x98",1);
                            LL_HMI_SendXY(685, 32);
                            LL_HMI_Send_Pure("\x21\x81\x03\x00\x1F\x00\x1F",7);
                            sprintf(str,"%u",gComInfo.FeedbackVolt);
                            LL_HMI_Send_Pure(str ,strlen(str));
                            LL_HMI_SendEnd();
                            
                            gInfo.Debug.dac=((float)0.01220703125*(gInfo.ModuleInfo.RoutineModule.DAC_Cail&0xFFF)+0.05);
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
                        }
                        break;
                    default:
                        break;
                }
                
            }
            else if(pbuf[1]==0x22)      //光1使用时间
            {
                //LOG_E("Recv Cmd 0x22 size %u",(uint16_t)data_size);
                Resend_getUsedtime=0;
                if (data_size==16)
                {
                    uint8_t code cmd[]="\x21\x81\x01\xFF\xFF\x00\x1F";
                    if (gConfig.LANG==LANG_CH)
                    {
                        HMI_Cut_Pic(0x71, 2, 133, 219, 133+99, 219+44);     //字符背景还原
                    }
                    else
                    {
                        HMI_Cut_Pic(0x71,102, 125, 220, 125+100, 220+42);     //字符背景还原
                    }
                    
                    LL_HMI_Send("\x98",1);      //时间
                    if (gConfig.LANG==LANG_CH)
                    {
                        LL_HMI_SendXY(0x8C, 0xDE);
                    }
                    else
                    {
                        LL_HMI_SendXY(0x74, 0xDE);
                    }
                    LL_HMI_Send_Pure(cmd,7);
                    LL_HMI_Send_Pure(pbuf+2,5);
                    LL_HMI_Send_Pure("h",1);
                    LL_HMI_Send_Pure(pbuf+7,2);
                    LL_HMI_Send_Pure("min",3);
                    LL_HMI_SendEnd();
                    
                    LL_HMI_Send("\x98",1);  //次数
                    if (gConfig.LANG==LANG_CH)
                    {
                        LL_HMI_SendXY(0x8C, 0xF3);
                    }
                    else
                    {
                        LL_HMI_SendXY(0x7E, 0xF1);
                    }
                    LL_HMI_Send_Pure(cmd,7);
                    LL_HMI_Send_Pure(pbuf+9,5);
                    LL_HMI_SendEnd();
                }
                
            }
            else if(pbuf[1]==0x20)      //读取温度
            {
                switch (gComInfo.HMI_Scene)
                {
                    case eScene_Module_UVA1:
                        if (pbuf[2]==0)     //读到数据且正确
                        {
                            int16_t temp=pbuf[3]*10;
                            gComInfo.TempCount=0;
                            HMI_Show_Temp(temp);
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
    //LOG_E("CMD:GetUsedTime");
    LL_Module_Send("\x39\x25\x01", 3);
    Resend_getUsedtime=1;
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

void ModuleRoutine_GetCalibData()
{
    LL_Module_Send("\x39\x40\x00", 3);
    Resend_getCalibData=1;
}
