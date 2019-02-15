#include "main.h"
#include "WorkProcess.h"
#include "HMIProcess.h"
#include "Module_COMM.h"
#include "HMI_COMM.h"
#include <stdarg.h>


#include "STC_EEPROM.h"

bit Uart1_Busy=0;
bit Uart2_Busy=0;

bit Uart1_buf_sel=0;
bit Uart2_buf_sel=0;
bit Uart1_ReviceFrame;
bit Uart2_ReviceFrame;


bit HMI_Msg_Flag=0;

bit SystemTime100ms=0;
bit SystemTime1s=1;
bit Heardbeat1s=1;

uint8_t idata uart1_buff[18];
uint8_t idata uart2_buff[18];

_Golbal_comInfo idata gComInfo={0};
_Golbal_Config  idata gConfig={0};
_Golbal_Info    xdata gModuleInfo={0};

uint8_t idata BeepTime=0;
void BeepEx(uint8_t time)
{
    BeepTime=time;
    BEEP_IO=0;
    CR=1;
    //LOG_E("BeepOn");
}

void IOPort_Init(void)
{
/* 7      6         5         4      3     2     1     0     Reset Value
   -   LVD_P4.6  ALE_P4.5  NA_P4.4   -     -     -     -	    x000,xxxx	*/
	P4SW |=	0x70;	 		//P4.6,P4.5,P4.4配置为I/O口使用

    P1M1 |= 0x00; // 0000 0000
    P1M0 |= 0x20; // 0010 0000

	P3M1 |=	0x00;			// 0000 0000  
	P3M0 |=	0x40;			// 0100 0000 

	P0M1 |=	0x08;	 		// 0000 1000
	P0M0 |=	0x27;	 		// 0010 0111

	P2M1 |=	0x00;			// 0000 0000  
	P2M0 |=	0x01;			// 0000 0001 
}

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
    PS = 1;			//串口中断优先级低位
    IPH &= ~0x10;	//串口中断优先级高位
    ES = 1;
}

void Uart2_Init(void)
{
	S2CON = 0x50;		//8位 可变波特率
	AUXR &= 0xF7;		//波特率不倍速
	AUXR |= 0x04;  		//独立波特率发生器时钟为Fosc,BRTx12=1,1T工作模式
//	BRT = 0xFD;	        //设置波特率为115200
	BRT = 0xDC;			//设置波特率为9600
	AUXR |= 0x10;  		//启动独立波特率发生器
	IE2  |= 0x01;		//允许串口2中断
	AUXR &= ~0x02;		//内部RAM使能
}

void PCA_Init(void)
{
//	AUXR1 |= 0X40;
	CMOD |= 0x80;	  	//PCA在空闲模式下不计数；PCA=fosc/12计数模式；
	CR = 0;		  		//停止PCA计数；
	CF = 0;		  		//清PCA溢出中断请求标志位；
	CCF0 = 0;		  	//清模块0中断请求标志位；
	CL = 0;
	CH = 0;
	CCAP0H = 0xB4;  	//0xB403为50MS，0x4801h为20MS，0x23FAh为10MS，0x11FDH为5MS,0X08FE为2.5ms，0x0399h为1MS；
	CCAP0L = 0x03;  	//0x01CCH为500us,0x005cH为100US；0x002e为50US； 0x00c8为200个计数脉冲
	CCAPM0 = 0x49;  	//设置PCA模块0为16位软件定时器；ECCF0=1允许PCA模块0中断； //11.0592Mhz是0x07
				  		//当[CH，CL]==[CCAP0H，CCAP0L]时，产生中断请求，CCF0=1，请求中断
//	CR = 1;				//启动PCA计数	
}

void Timer0Init(void)		//50毫秒@11.0592MHz自动重载
{
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x4C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
    ET0 = 1;        //enable timer0 interrupt
    TR0 = 1;		//定时器0开始计时
}

void Init()
{
    KEY_LED_IO=ENABLE;
    FAN_IO=DISABLE;
    IOPort_Init();
    Uart1_Init();
    Uart2_Init();
    PCA_Init();
    Timer0Init();
    EA=1;

    gComInfo.COMMProtocol_Head=0xAA;    //默认先试常规治疗头协议
    gComInfo.COMMProtocol_Tail1=0xC3;
    gComInfo.COMMProtocol_Tail2=0x3C;
}

void LOG_E(void*str,...)
{
    uint8_t i=0;
    char xdata buf[32]={0};
    va_list ap;
    va_start(ap, str);
    vsprintf(buf, (char const*)str, ap);
    va_end(ap);
    while (Uart2_Busy); //防止正常数据没发完
    AUXR1 |= 0x10;  //串口IO切到P4
    while (buf[i])
    {
        while(Uart2_Busy);
        Uart2_Busy=1;
        S2BUF=buf[i];
        i++;
    }
    while(Uart2_Busy);  //等待最后一个byte发送完成
    AUXR1 &= 0xEF;  //串口IO复原
}

void Save_Config()
{
    uint8_t c;
    Sector_Erase(0);    //必须擦扇区,否则写不进
    
    IAP_CONTR = 0x83;         //打开 IAP 功能, 设置Flash 操作等待时间
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM 字节编程命令
    IAP_ADDRH = 0;    //设置目标单元地址的高8 位地址
    IAP_ADDRL = 0;
    EA=0;
    for(IAP_ADDRL=0;IAP_ADDRL<sizeof(_Golbal_Config);IAP_ADDRL++)
    {
        c=*(((char *)&gConfig)+IAP_ADDRL);
        IAP_DATA = c ;                  //要编程的数据先送进IAP_DATA 寄存器
        IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
        IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
        _nop_();
        _nop_();
    }
    EA=1;
    IAP_Disable();
    //LOG_E("SysLang:%02X",(uint16_t)Byte_Read(0));
}

int main()
{
    gConfig.LANG=Byte_Read(0);
    if (gConfig.LANG>1)
    {
        gConfig.LANG=0;
    }
    Init();
    //LOG_E("System Boot \t SysLang:%02X",(uint16_t)Byte_Read(0));//,0x30+gConfig.LANG);
    BeepEx(0);  //响50ms
    for(;;)
    {
        Module_COMM();
        HMI_COMM();
        
        Work_Process();
        HMI_Process();
        
        if (Heardbeat1s==1) //调试用
        {
            Heardbeat1s=0;
            KEY_LED_IO=DISABLE;
            //CR=1;
        }
    }
}

