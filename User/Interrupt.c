#include "main.h"
#include "WorkProcess.h"
#include "HMIProcess.h"

//HMI 通信
void UART1_INT(void) interrupt 4 using 1
{
    static uint8_t head=0;
    static uint8_t data_size;
    uint8_t rx_data;
	if(TI==1)   //发送完1byte
	{
		TI=0;
		Uart1_Busy=0;	//TIFLG作为其他程序的查询标记；
	}
	if(RI==1)  //接收到1byte
	{	
        RI=0;
        rx_data=SBUF;
        if (head!=0xAA)
        {
            head=rx_data;
        }
        else
        {
            data_size++;
            uart1_buff[data_size]=rx_data;
            if(data_size>5 &&
            uart1_buff[data_size]==0x3C && 
            uart1_buff[data_size-1]==0xC3 && 
            uart1_buff[data_size-2]==0x33 && 
            uart1_buff[data_size-3]==0xCC)      //0xCC33C33C
            {
                uart1_buff[0]=(data_size);
                Uart1_ReviceFrame=1;
                head=0;
                data_size=0;
            }
        }
	}
}

//治疗头 通信

void UART2_INT(void) interrupt 8 using 2
{
    static uint8_t head=0,tail=0;
    static uint8_t data_size;
    uint8_t rx_data;

	if(S2CON & S2TI)
	{
		S2CON &= ~S2TI;		//Clear transmit interrupt flag
		Uart2_Busy = 0;
	}
	if(S2CON & S2RI)
	{
		S2CON &= ~S2RI;		//Clear receive interrupt flag
		rx_data = S2BUF;
		if (head!=gComInfo.COMMProtocol_Head)
        {
            head=rx_data;
        }
        else
        {
            data_size++;
            uart2_buff[data_size]=rx_data;
            if (tail==0)
            {
                if (rx_data==gComInfo.COMMProtocol_Tail1)
                {
                    tail++;
                }
            }
            else if(tail==1)
            {
                if (rx_data == gComInfo.COMMProtocol_Tail2)
                {
                    tail++;
                }
                else if(rx_data == gComInfo.COMMProtocol_Tail1)
                {
                    ; //Do nothing
                }
                else
                {
                    tail=0;
                }
            }
            else
            {
                uart2_buff[0]=(data_size);
                Uart2_ReviceFrame=1;
                head=0;
                tail=0;
                data_size=0;
            }
        }
	}
}

/*******************************************************/
/*               Timer0中断                              */
/*			     定时中断一次	                               */
/*******************************************************/

extern uint8_t idata BeepTime;

void Timer0_isr() interrupt 1 using 3
{
    TL0 = 0x00;		//设置定时初值    50ms
    TH0 = 0x4C;		//设置定时初值
    gComInfo.TimerCounter++;
    if (Fire_Flag==1 && gComInfo.WorkStat==eWS_Working && gModuleInfo.RoutineModule.LightMode==1 && gComInfo.TimerCounter%5==0)   //4Hz 闪烁模式
    {
        switch (gComInfo.HMI_Scene)
        {
            case eScene_Module_650:
                PowerCtr_Module12v=~PowerCtr_Module12v;
                break;
            case eScene_Module_633:
                PowerCtr_Light1=~PowerCtr_Light1;
                break;
            default:
                break;
        }
    }
    if(gComInfo.TimerCounter % 2 == 0)
	{
		SystemTime100ms = 1;
	}
	if(gComInfo.TimerCounter >= 20)
	{
		gComInfo.TimerCounter = 0;
		SystemTime1s = 1;		//治疗时间1s倒计时标志
        Heardbeat1s=1;
	}
}

/*******************************************************/
/*               PCA中断                                 */
/*			     定时中断一次	                               */
/*******************************************************/

void Timer_PCA(void) interrupt 7 using 3		//PCA中断函数 蜂鸣器定时,50ms为单位
{
    CL=0;
    CH=0;
    CCF0=0;
    if (BeepTime==0)
    {
        BEEP_IO=1;  //关蜂鸣器
        CR=0;
    }
    else
    {
        --BeepTime;
    }
    KEY_LED_IO=ENABLE;
}

void ADC_ISR(void) interrupt 5 
{
    uint16_t ADC_Value=0;
    ADC_CONTR &= ~ADC_FLAG;         						//Clear ADC interrupt flag
    ADC_Value=(ADC_RES<<2)|ADC_RESL;
    gComInfo.FeedbackVolt=(ADC_Value *1000L *5L) >>10;
    ADConvertDone=1;
}


