#include "Module_COMM.h"
#include "HMI_COMM.h"
#include "WorkProcess.h"
#include "HMIProcess.h"
#include "STC_EEPROM.h"

void LL_Module_Send(const void* str,uint8_t str_len)
{
    uint8_t i;
    const uint8_t *ptr=str;
    while (gCom.Uart2_Busy);
    gCom.Uart2_Busy=1;
    S2BUF=gCom.COMMProtocol_Head;
    for (i = 0; i < str_len; i++)
    {
        while (gCom.Uart2_Busy);
        gCom.Uart2_Busy=1;
        S2BUF=*ptr;
        ptr++;
    }
    while (gCom.Uart2_Busy);
    gCom.Uart2_Busy=1;
    S2BUF=gCom.COMMProtocol_Tail1;
    while (gCom.Uart2_Busy);
    gCom.Uart2_Busy=1;
    S2BUF=gCom.COMMProtocol_Tail2;
    if (gCom.COMMProtocol_Head=='@')    //�����308ͷ,�෢'\n'
    {
        while (gCom.Uart2_Busy);
        gCom.Uart2_Busy=1;
        S2BUF=0x0A;
    }
    while (gCom.Uart2_Busy);     //�ȴ�������
    #if defined(_DEBUG) && 0 /*����������������*/
    LOG_E("%02X ",(uint16_t)gCom.COMMProtocol_Head);
    for (i = 0; i < str_len; i++)
    {
        LOG_E("%02X ",(uint16_t)*((uint8_t*)(str+i)));
    }
    LOG_E("%02X ",(uint16_t)gCom.COMMProtocol_Tail1);
    LOG_E("%02X ",(uint16_t)gCom.COMMProtocol_Tail2);
    if (gCom.COMMProtocol_Head=='@')    //�����308ͷ,�෢'\n'
    {
        LOG_E("%02X ",(uint16_t)'\n');
    }
    #endif
}


void Module_COMM()
{
    if (gCom.Uart2_ReviceFrame==0)
    {
        return;
    }
    else
    {
        uint8_t xdata pbuf[20]={0};
        uint8_t data_size=uart2_buff[0];
        if (data_size>20)
        {
            data_size=20;
        }
        memcpy(pbuf+1,uart2_buff+1,data_size);
        gCom.Uart2_ReviceFrame=0;
        if (gCom.COMMProtocol_Head==0xAA)
        {
            if (pbuf[1]==0x21)
            {
                uint8_t cmd=pbuf[2];
                switch (cmd)
                {
                    case 0x01:      //���ֻظ�����ͷ����
                        gCom.ModuleType=pbuf[3];
                        LOG_E("Module Type: %02X",(uint16_t)gCom.ModuleType);
                        gCom.TimerCounter=0;
                        SystemTime1s=0;
                        gInfo.DebugOpen=OPEN_DBG_Calib|
                                        OPEN_DBG_ClearUsedtime|
                                        OPEN_DBG_Config|
                                        OPEN_DBG_ROOT;  //������ͷ��Ȩ 
                        //TODO: ��ͬ����ͷ��Ȩ��һ��
                        switch (gCom.ModuleType)    //����ͷ���ݳ�ʼ��
                        {
                            case M_Type_650:
                                gCom.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.Routine.WorkTime=10;  //Ĭ��10����
                                gInfo.ModuleInfo.Routine.PowerLevel=1800;   //����ͷû�ظ�������С
                                gCom.HMI_LastScene=eScene_Module_650;
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gCom.WorkStat=eWS_Standby;
                                DS18B20_StartCovert();
                                gInfo.ModuleInfo.Routine.WorkTime=10;
                                gInfo.ModuleInfo.Routine.PowerLevel=150;   //633����ͷû�ظ�������С
                                gCom.HMI_LastScene=eScene_Module_633;
                                break;
                          //case M_Type_IU:     //IU������Э��,�ڴ˲����յ�
                            case M_Type_UVA1:
                                gCom.WorkStat=eWS_Standby;
                                gInfo.ModuleInfo.Routine.WorkTime=10;
                                gCom.HMI_LastScene=eScene_Module_UVA1;
                                break;
                            case M_Type_Wira:
                                gCom.WorkStat=eWS_Standby;
                                gCom.HMI_LastScene=eScene_Module_Wira;
                                gInfo.ModuleInfo.New4in1.PowerLevel[0]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[1]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[2]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[3]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[0]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[1]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[2]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[3]=100;
                                gInfo.ModuleInfo.New4in1.WorkTime[0]=30;
                                gInfo.ModuleInfo.New4in1.WorkTime[1]=30;
                                gInfo.ModuleInfo.New4in1.WorkTime[2]=30;
                                gInfo.ModuleInfo.New4in1.WorkTime[3]=30;
                                gInfo.ModuleInfo.New4in1.WorkTime[4]=30;
                                gInfo.ModuleInfo.New4in1.LightGroup=0xFF;
                                break;
                            case M_Type_4in1:
                                gCom.WorkStat=eWS_Standby;
                                gCom.HMI_LastScene=eScene_Module_4in1;
                                gInfo.ModuleInfo.New4in1.PowerLevel[0]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[1]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[2]=100;
                                gInfo.ModuleInfo.New4in1.PowerLevel[3]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[0]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[1]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[2]=100;
                                gInfo.ModuleInfo.New4in1.PowerMax[3]=100;
                                gInfo.ModuleInfo.New4in1.WorkTime[0]=10;
                                gInfo.ModuleInfo.New4in1.WorkTime[1]=10;
                                gInfo.ModuleInfo.New4in1.WorkTime[2]=10;
                                gInfo.ModuleInfo.New4in1.WorkTime[3]=10;
                                gInfo.ModuleInfo.New4in1.WorkTime[4]=10;
                                gInfo.ModuleInfo.New4in1.LightGroup=0xFF;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0x03:      //��1�⹦��
                        //LOG_E("Get PowerLevel %u",(uint16_t)pbuf[3]);
                        switch (gCom.ModuleType)
                        {
                            case M_Type_650:    //1800mw
                                gInfo.ModuleInfo.Routine.PowerLevel=pbuf[3]*10;
                                break;
                            case M_Type_633:
                            case M_Type_633_1:
                                gInfo.ModuleInfo.Routine.PowerLevel=pbuf[3];
                                break;
                            default:
                                break;
                        }
                        HMI_Show_Power();
                        break;
                    case 0x04:      //��2�⹦��
                        break;
                    case 0x0B:      //DAC Vfb
                        Resend_getCalibData=0;
                        switch (gCom.ModuleType)    //����ͷ���ݳ�ʼ��
                        {
                            case M_Type_633:
                            case M_Type_633_1:
                                gInfo.ModuleInfo.Routine.DAC_Cail=pbuf[3]<<8|pbuf[4];
                                gCom.FeedbackVolt=pbuf[5]<<8|pbuf[6];
                                break;
                            case M_Type_UVA1:
                            case M_Type_Wira:
                            case M_Type_4in1:
                                break;
                            default:
                                break;
                        }
                        if (gCom.HMI_Scene==eScene_Debug)
                        {
                            char xdata str[6]={0};
                            HMI_Cut_Pic(0x71,62, 657, 21, 657+117, 21+53);  //Vfb�����ָ�
                            HMI_Cut_Pic(0x71,62, 109, 396, 109+106, 396+61);  //DA�����ָ�
                            LL_HMI_Send("\x98",1);
                            LL_HMI_SendXY(685, 32);
                            LL_HMI_Send_Pure("\x5\x80\x05\x00\x1F\x00\x1F",7);
                            sprintf(str,"%u",gCom.FeedbackVolt);
                            LL_HMI_Send_Pure(str ,strlen(str));
                            LL_HMI_SendEnd();
                            
                            gInfo.Debug.dac=((float)0.01220703125*(gInfo.ModuleInfo.Routine.DAC_Cail&0xFFF)+0.05);
                            LL_HMI_Send("\x98",1);
                            LL_HMI_SendXY(134, 411);
                            LL_HMI_Send_Pure("\x5\x80\x05\x00\x1F\x00\x1F",7);
                            while (gCom.Uart1_Busy);
                            gCom.Uart1_Busy=1;
                            SBUF=gInfo.Debug.dac/10+'0';
                            while (gCom.Uart1_Busy);
                            gCom.Uart1_Busy=1;
                            SBUF='.';
                            while (gCom.Uart1_Busy);
                            gCom.Uart1_Busy=1;
                            SBUF=gInfo.Debug.dac%10+'0';
                            LL_HMI_SendEnd();
                        }
                        break;
                    default:
                        break;
                }
                
            }
            else if(pbuf[1]==0x22)      //��1ʹ��ʱ��
            {
                //LOG_E("Recv Cmd 0x22 size %u",(uint16_t)data_size);
                Resend_getUsedtime=0;
                if (data_size==13)
                {
                    uint8_t code cmd[]="\x21\x81\x01\xFF\xFF\x00\x1F";
                    if (gConfig.LANG==LANG_ZH)
                    {
                        HMI_Cut_Pic(0x71, 2, 133, 219, 133+99, 219+44);     //�ַ�������ԭ
                    }
                    else
                    {
                        HMI_Cut_Pic(0x71,102, 125, 220, 125+100, 220+42);     //�ַ�������ԭ
                    }
                    
                    LL_HMI_Send("\x98",1);      //ʱ��
                    if (gConfig.LANG==LANG_ZH)
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
                    
                    LL_HMI_Send("\x98",1);  //����
                    if (gConfig.LANG==LANG_ZH)
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
            else if(pbuf[1]==0x20)      //��ȡ�¶�
            {
                switch (gCom.HMI_Scene)
                {
                    case eScene_Module_UVA1:
                        if (pbuf[2]==0)     //������������ȷ
                        {
                            int16_t temp=pbuf[3]*10;
                            gCom.TempCount=0;
                            HMI_Show_Temp(temp);
                        }
                        else
                        {
                            ;//do nothing
                        }
                        break;
                    case eScene_Module_Wira:
                        if (pbuf[2]==0)     //������������ȷ
                        {
                            int16_t temp=pbuf[3]*10;
                            gCom.TempCount=0;
                            if(Dbg_Admin)
                            {
                                HMI_Show_Temp(temp);
                            }
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
        else    //gCom.COMMProtocol_Head=='@'
        {
            //LOG_E("%s",pbuf+1);   //��ӡЭ��
            if (pbuf[1]=='1' && pbuf[2]=='*')   //Э�鹲ͬ����
            {
                if (pbuf[3]=='1')    //1 10 11 12 13 14 15 
                {
                    switch (pbuf[4])
                    {
                        case '&':   //�⴫��������
                            LOG_E("308 Error 1");
                            gCom.ErrorCode=0x31;
                            HMI_Goto_Error();
                            break;
                        case '0':   //�Ѿ�����,��ʼ��ʱ
                            //LOG_E("308 Fire Flag");
                            gCom.WorkStat=eWS_Working;
                            gCom.TimerCounter=0;
                            SystemTime1s=0;
                            HMI_Show_308RemainTime();
                            break;
                        case '1':   //308���� @1*11&9&5*#
                            if (gCom.HMI_Scene!=eScene_Info)    //�����ϵͳ��Ϣ����������
                            {
                                memset(&gInfo,0,sizeof(_Golbal_Info));  //������ͷ����
                                gCom.WorkStat=eWS_Standby;
                                gCom.ModuleType=M_Type_308;
                                gCom.HMI_Scene=eScene_Module_308Wait;       //ֱ���л������ȴ�����
                                gInfo.ModuleInfo.mini308.WorkTime=15;     //Ĭ��30��
                                gInfo.DebugOpen=OPEN_DBG_ClearUsedtime|
                                        OPEN_DBG_Config|
                                        OPEN_DBG_ROOT;  //������ͷ��Ȩ 
                                ADC_Init();//��ʼ��ADC
                            }
                            break;
                        case '2':   //Ƶ��ռ�ձ�           @1*12&Ƶ������&ռ�ձ�����*#
                            gInfo.ModuleInfo.mini308.Freq=atoi(pbuf+6);
                            gInfo.ModuleInfo.mini308.Duty=atoi(pbuf+10);
//                            LOG_E(" Freq:%d,Duty:%d",
//                                (uint16_t)gInfo.ModuleInfo.mini308.Freq,
//                                (uint16_t)gInfo.ModuleInfo.mini308.Duty);
                            break;
                        case '3':   //ʹ��ʱ��:@1*13&Сʱ&����&����*# 1*13&000&00&00(��ʼ״̬Ϊ0)
                            if (gCom.HMI_Scene==eScene_Info)    //ȷ�ϳ���
                            {
                                pbuf[9]=':';
                                pbuf[12]=':';
                                LL_HMI_Send("\x98\x00\x45\x01\x66\x21\x81\x03\x00\x1F\x00\x1F",12);
                                LL_HMI_Send_Pure(pbuf+6,9);
                                LL_HMI_SendEnd();
                            }
                            break;
                        case '4':   //ʣ��ʱ��@1*14&300&00&00*#(��ʼ״̬Ϊ300Сʱ��00���ӣ�00����)
                            if (gCom.HMI_Scene==eScene_Info)    //ȷ�ϳ���
                            {
                                pbuf[9]=':';
                                pbuf[12]=':';
                                LL_HMI_Send("\x98\x02\x49\x01\x66\x21\x81\x03\x00\x1F\x00\x1F",12);
                                LL_HMI_Send_Pure(pbuf+6,9);
                                LL_HMI_SendEnd();
                            }
                            break;
                        case '5':   //@1*15&0&0*# ֹͣ��ʱ      // û��???
                            
                            break;
                        case '6':   //@1*16&1&1*#--IU����ͷ
                        {
                            uint8_t i=0;
                            memset(&gInfo,0,sizeof(_Golbal_Info));  //������ͷ����
                            gCom.WorkStat=eWS_Standby;
                            gCom.ModuleType=M_Type_IU;
                            gCom.HMI_LastScene=eScene_Module_IU;
                            gInfo.ModuleInfo.Routine.WorkTime=10;
                            gInfo.DebugOpen=OPEN_DBG_Calib|
                                        OPEN_DBG_ClearUsedtime|
                                        OPEN_DBG_ROOT;  //������ͷ��Ȩ 
                            for (i = 0; i < sizeof(_ModuleSave); i++)
                            {
                                *((uint8_t*)&gModuleSave+i)=Byte_Read(0x0200+i);
                            }
                            if (calculate_CRC8(&gModuleSave, sizeof(_ModuleSave))!=0)   //CRC����
                            {
                                memset(&gModuleSave,0x30,sizeof(_ModuleSave)-3);
                                gModuleSave.DAC_Cail=0x7000|3850;   //4.7v 48V??
                                Save_ModuleSomething();
                            }
                            HMI_Scene_Recovery();
                        }
                            break;
                        default:
                            break;
                    }
                }
                else    //0 2 3 4 5 7 ����
                {
                    LOG_E("308 Error %c",pbuf[3]);
                    gCom.ErrorCode=pbuf[3];
                    HMI_Goto_Error();
                }
            }
        }
    }
}


void ModuleRoutine_Shakehand()
{
    LL_Module_Send("\x39\x21\x00",3);
}

void Module308_Shakehand()
{
    LL_Module_Send("1*16&1&1",8);
    //LL_Module_Send("1*11&5&9",8);
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
