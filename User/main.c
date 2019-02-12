#include "main.h"
#include "WorkProcess.h"
#include "HMIProcess.h"
#include "Module_COMM.h"
#include "HMI_COMM.h"

#include "STC_EEPROM.h"

bit Uart1_TXE=1;
bit Uart2_TXE=1;

bit Uart1_buf_sel=0;
bit Uart2_buf_sel=0;

bit HMI_Msg_Flag=0;

uint8_t xdata uart1_buff[2][32];
uint8_t xdata uart2_buff[2][32];

_Golbal_comInfo idata gComInfo={0};
_Golbal_Config  idata gConfig={0};
_Golbal_Info    xdata gModuleInfo={0};


void Uart1_Init(void)		//115200bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//清除定时器1模式位
	TMOD |= 0x20;		//设定定时器1为8位自动重装方式
	TL1 = 0xFD;			//设定定时初值
	TH1 = 0xFD;			//设定定时器重装值
//	TL1 = 0xDC;			//设定定时初值		//波特率9600
//	TH1 = 0xDC;			//设定定时器重装值
	ET1 = 0;			//禁止定时器1中断
	TR1 = 1;			//启动定时器1
}

void Uart2_Init(void)
{
	S2CON = 0x50;		//8位可变波特率
	AUXR &= 0xF7;		//波特率不倍速
	AUXR |= 0x04;  		//独立波特率发生器时钟为Fosc,BRTx12=1,1T工作模式
//	BRT = 0xFD;	        //设置波特率为115200
	BRT = 0xDC;			//设置波特率为9600
	AUXR |= 0x10;  		//启动独立波特率发生器
	IE2  |= 0x01;		//允许串口2中断
	AUXR &= ~0x02;		//内部RAM使能
}


void Init()
{
    KEY_LED_IO=ENABLE;
    FAN_IO=DISABLE;
    Uart1_Init();
    Uart2_Init();
}


int main()
{
    gConfig.LANG=Byte_Read(0);
    Init();
    for(;;)
    {
        Work_Process();
        Module_COMM();
        HMI_COMM();
        HMI_Process();
    }
}

