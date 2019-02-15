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
	P4SW |=	0x70;	 		//P4.6,P4.5,P4.4����ΪI/O��ʹ��

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
	PCON &= 0x7F;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�����ʱ��1ģʽλ
	TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
	TL1 = 0xFD;			//�趨��ʱ��ֵ
	TH1 = 0xFD;			//�趨��ʱ����װֵ
//	TL1 = 0xDC;			//�趨��ʱ��ֵ		//������9600
//	TH1 = 0xDC;			//�趨��ʱ����װֵ
	ET1 = 0;			//��ֹ��ʱ��1�ж�
	TR1 = 1;			//������ʱ��1
    PS = 1;			//�����ж����ȼ���λ
    IPH &= ~0x10;	//�����ж����ȼ���λ
    ES = 1;
}

void Uart2_Init(void)
{
	S2CON = 0x50;		//8λ �ɱ䲨����
	AUXR &= 0xF7;		//�����ʲ�����
	AUXR |= 0x04;  		//���������ʷ�����ʱ��ΪFosc,BRTx12=1,1T����ģʽ
//	BRT = 0xFD;	        //���ò�����Ϊ115200
	BRT = 0xDC;			//���ò�����Ϊ9600
	AUXR |= 0x10;  		//�������������ʷ�����
	IE2  |= 0x01;		//������2�ж�
	AUXR &= ~0x02;		//�ڲ�RAMʹ��
}

void PCA_Init(void)
{
//	AUXR1 |= 0X40;
	CMOD |= 0x80;	  	//PCA�ڿ���ģʽ�²�������PCA=fosc/12����ģʽ��
	CR = 0;		  		//ֹͣPCA������
	CF = 0;		  		//��PCA����ж������־λ��
	CCF0 = 0;		  	//��ģ��0�ж������־λ��
	CL = 0;
	CH = 0;
	CCAP0H = 0xB4;  	//0xB403Ϊ50MS��0x4801hΪ20MS��0x23FAhΪ10MS��0x11FDHΪ5MS,0X08FEΪ2.5ms��0x0399hΪ1MS��
	CCAP0L = 0x03;  	//0x01CCHΪ500us,0x005cHΪ100US��0x002eΪ50US�� 0x00c8Ϊ200����������
	CCAPM0 = 0x49;  	//����PCAģ��0Ϊ16λ�����ʱ����ECCF0=1����PCAģ��0�жϣ� //11.0592Mhz��0x07
				  		//��[CH��CL]==[CCAP0H��CCAP0L]ʱ�������ж�����CCF0=1�������ж�
//	CR = 1;				//����PCA����	
}

void Timer0Init(void)		//50����@11.0592MHz�Զ�����
{
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TMOD |= 0x01;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ֵ
	TH0 = 0x4C;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
    ET0 = 1;        //enable timer0 interrupt
    TR0 = 1;		//��ʱ��0��ʼ��ʱ
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

    gComInfo.COMMProtocol_Head=0xAA;    //Ĭ�����Գ�������ͷЭ��
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
    while (Uart2_Busy); //��ֹ��������û����
    AUXR1 |= 0x10;  //����IO�е�P4
    while (buf[i])
    {
        while(Uart2_Busy);
        Uart2_Busy=1;
        S2BUF=buf[i];
        i++;
    }
    while(Uart2_Busy);  //�ȴ����һ��byte�������
    AUXR1 &= 0xEF;  //����IO��ԭ
}

void Save_Config()
{
    uint8_t c;
    Sector_Erase(0);    //���������,����д����
    
    IAP_CONTR = 0x83;         //�� IAP ����, ����Flash �����ȴ�ʱ��
    IAP_CMD = 0x02;                 //IAP/ISP/EEPROM �ֽڱ������
    IAP_ADDRH = 0;    //����Ŀ�굥Ԫ��ַ�ĸ�8 λ��ַ
    IAP_ADDRL = 0;
    EA=0;
    for(IAP_ADDRL=0;IAP_ADDRL<sizeof(_Golbal_Config);IAP_ADDRL++)
    {
        c=*(((char *)&gConfig)+IAP_ADDRL);
        IAP_DATA = c ;                  //Ҫ��̵��������ͽ�IAP_DATA �Ĵ���
        IAP_TRIG = 0x5A;   //���� 5Ah,����A5h ��ISP/IAP �����Ĵ���,ÿ�ζ������
        IAP_TRIG = 0xA5;   //����A5h ��ISP/IAP ����������������
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
    BeepEx(0);  //��50ms
    for(;;)
    {
        Module_COMM();
        HMI_COMM();
        
        Work_Process();
        HMI_Process();
        
        if (Heardbeat1s==1) //������
        {
            Heardbeat1s=0;
            KEY_LED_IO=DISABLE;
            //CR=1;
        }
    }
}

