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
            if(uart1_buff[data_size]  ==0x3C && 
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
		if (head!=gCom.COMMProtocol_Head)
        {
            head=rx_data;
        }
        else
        {
            data_size++;
            uart2_buff[data_size]=rx_data;
            if (rx_data == gCom.COMMProtocol_Tail2 && 
                uart2_buff[data_size-1]==gCom.COMMProtocol_Tail1)
            {
                uart2_buff[0]=(data_size)-2;    //去掉包尾
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

extern uint8_t BeepTime;
extern uint8_t BeepCount;

void Timer0_isr() interrupt 1 using 3
{
    TL0 = 0x00;		//设置定时初值    50ms
    TH0 = 0x4C;		//设置定时初值
    
    if (Fire_Flag==1 && gCom.WorkStat==eWS_Working && gInfo.ModuleInfo.Routine.LightMode==1 && gCom.TimerCounter%10==0)   //2Hz 闪烁模式
    {
        switch (gCom.HMI_Scene)
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
    
    gCom.TimerCounter++;
    if(gCom.TimerCounter % 2 == 0)
	{
		SystemTime100ms = 1;
	}
	if(gCom.TimerCounter >= 20)
	{
		gCom.TimerCounter = 0;
		SystemTime1s = 1;		//治疗时间1s倒计时标志
		SystemTime1s_1=1;
	}
	else if(gCom.TimerCounter == 10)    //与工作秒错开500ms
	{
	    Heardbeat1s=1;
	}
}

/*******************************************************/
/*               PCA中断                                 */
/*			     定时中断一次	                               */
/*******************************************************/
void Timer_PCA(void) interrupt 7 using 3		//PCA中断函数 蜂鸣器定时,50ms为单位
{
    static uint8_t time=0;
    CL=0;
    CH=0;
    CCF0=0;
    KEY_LED_IO=ENABLE;  //For test

    if (time>=BeepTime)
    {
        BEEP_IO=!BEEP_IO;
        time=0;
        if (BeepCount==0)
        {
            BEEP_IO=1;  //关蜂鸣器
            CR=0;
        }
        else
        {
            BeepCount--;
        }
    }
    else
    {
        time++;
    }
}

void ADC_ISR(void) interrupt 5 
{
    uint16_t ADC_Value=0;
    ADC_CONTR &= ~ADC_FLAG;         						//Clear ADC interrupt flag
    ADC_Value=(ADC_RES<<2)|ADC_RESL;
    gCom.FeedbackVolt=(ADC_Value *1000L *5L) >>10;      //数据扩大1000倍
    ADConvertDone=1;
}


