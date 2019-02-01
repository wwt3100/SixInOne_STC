#include "main.h"

//HMI ͨ��
void UART1_INT(void) interrupt 4
{
    static uint8_t head=0,tail=0;
    static uint8_t data_size;
    uint8_t rx_data;
	if(TI==1)   //������1byte
	{
		TI=0;
		Uart1_TXE=1;	//TIFLG��Ϊ��������Ĳ�ѯ��ǣ�
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
            uart1_buff[Uart1_buf_sel][data_size]=SBUF;
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
                uart1_buff[Uart1_buf_sel][0]=(data_size|0x80);
                Uart1_buf_sel=!Uart1_buf_sel;
                head=0;
                tail=0;
                data_size=0;
            }
        }
	}
}

//����ͷ ͨ��
void UART2_INT(void) interrupt 8
{
	if(S2CON & S2TI)
	{
		S2CON &= ~S2TI;		//Clear transmit interrupt flag
		Uart2_TXE = 1;
	}
	if(S2CON & S2RI)
	{
		S2CON &= ~S2RI;		//Clear receive interrupt flag
		//Message_Data = S2BUF;
		if(gComInfo.ModuleType == 0x55 || gComInfo.ModuleType == 0x0b)  //308����ͷ
		{
	
		}
		else	 //UI,UV1����������ͷ
		{

		}
	}
}
