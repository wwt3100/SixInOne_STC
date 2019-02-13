#include "main.h"

//HMI ͨ��
void UART1_INT(void) interrupt 4 using 1
{
    static uint8_t head=0,tail=0;
    static uint8_t data_size;
    uint8_t rx_data;
	if(TI==1)   //������1byte
	{
		TI=0;
		Uart1_Busy=0;	//TIFLG��Ϊ��������Ĳ�ѯ��ǣ�
	}
	if(RI==1)  //���յ�1byte
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
            uart1_buff[Uart1_buf_sel][data_size]=rx_data;
            if(tail<4)
            {
                switch (tail)
                {
                    case 0:
                        if(rx_data==0xCC)
                        {
                            tail++;
                        }
                        break;
                    case 1:
                        if(rx_data==0x33)
                        {
                            tail++;
                        }
                        else
                        {
                            tail=0;
                        }
                        break;
                    case 2:
                        if(rx_data==0xC3)
                        {
                            tail++;
                        }
                        else
                        {
                            tail=0;
                        }
                        break;
                    case 3:
                        if(rx_data==0x3C)
                        {
                            tail++;
                        }
                        else
                        {
                            tail=0;
                        }
                        break;
                    default:
                        tail=0;     //Shoud not be here
                        break;
                }
            }
            else
            {
                uart1_buff[Uart1_buf_sel][0]=(data_size);
                Uart1_buf_sel=!Uart1_buf_sel;
                Uart1_ReviceFrame=1;
                head=0;
                tail=0;
                data_size=0;
            }
        }
	}
}

//����ͷ ͨ��

void UART2_INT(void) interrupt 8 using 2
{
    static uint8_t head=0,tail=0;
    static uint8_t data_size;
    uint8_t rx_data;

	if(S2CON & S2TI)
	{
		S2CON &= ~S2TI;		//Clear transmit interrupt flag
		Uart2_Busy = 1;
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
            uart2_buff[Uart2_buf_sel][data_size]=rx_data;
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
                else
                {
                    tail=0;
                }
            }
            else
            {
                uart2_buff[Uart2_buf_sel][0]=(data_size);
                Uart2_buf_sel=!Uart2_buf_sel;
                Uart2_ReviceFrame=1;
                head=0;
                tail=0;
                data_size=0;
            }
        }
	}
}

/*******************************************************/
/*               Timer0�ж�              			   */
/*			     ��ʱ�ж�һ��	                   */
/*******************************************************/

extern uint8_t idata BeepTime;

void Timer0_isr() interrupt 1 using 3
{
    TL0 = 0x00;		//���ö�ʱ��ֵ    50ms
    TH0 = 0x4C;		//���ö�ʱ��ֵ
    gComInfo.TimerCounter++;
    if (gComInfo.WorkStat==3 && gComInfo.TimerCounter%5==0)
    {

    }
    if(gComInfo.TimerCounter % 2 == 0)
	{
		SystemTime100ms = 1;
	}
	if(gComInfo.TimerCounter >= 20)
	{
		gComInfo.TimerCounter = 0;
		SystemTime1s = 1;		//����ʱ��1s����ʱ��־
	}
}

/*******************************************************/
/*               PCA�ж�              			   */
/*			     ��ʱ�ж�һ��	                   */
/*******************************************************/

void Timer_PCA(void) interrupt 7 using 3		//PCA�жϺ��� ��������ʱ,50msΪ��λ
{
    CL=0;
    CH=0;
    CCF0=0;
    --BeepTime;
    if (BeepTime==0)
    {
        BEEP_IO=1;  //�ط�����
        CR=0;
    }
}

