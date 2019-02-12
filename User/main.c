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
}

void Uart2_Init(void)
{
	S2CON = 0x50;		//8λ�ɱ䲨����
	AUXR &= 0xF7;		//�����ʲ�����
	AUXR |= 0x04;  		//���������ʷ�����ʱ��ΪFosc,BRTx12=1,1T����ģʽ
//	BRT = 0xFD;	        //���ò�����Ϊ115200
	BRT = 0xDC;			//���ò�����Ϊ9600
	AUXR |= 0x10;  		//�������������ʷ�����
	IE2  |= 0x01;		//������2�ж�
	AUXR &= ~0x02;		//�ڲ�RAMʹ��
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

