#include "DS18B20.h"


void DelayXus(uint8_t n)
{
    while (n--)
    {
        _nop_();
        _nop_();
    }
}
void DS18B20_Write_Byte(uint8_t dat)
{
    uint8_t i;
    for (i=0; i<8; i++)             //8λ������
    {
        DQ = 0;                     //��������
        DelayXus(1);                //����ά����1us,��ʾдʱ��(����д0ʱ���д1ʱ��)��ʼ
        dat >>= 1;                  //���ֽڵ����λ��ʼ���䣬�������������ߺ��15us��,
        DQ = CY;					//��Ϊ15us��DS18B20������߲���
        DelayXus(60);               //������дʱ���������60us
        DQ = 1;                     //д���,�����ͷ�����
        DelayXus(1);                //�ָ���ʱ
    }
}
uint8_t DS18B20_Read_Byte()
{
    uint8_t i;
    uint8_t dat = 0;
    for (i=0; i<8; i++)             //8λ������
    {
        dat >>= 1;                  //�ô������϶�����λ���ݣ����δӸ�λ�ƶ�����λ
        DQ = 0;                     //���������ͣ�Ҫ��1us֮���ͷ����ߣ���Ƭ��Ҫ�ڴ��½��غ��15us�ڶ����ݲŻ���Ч��
        DelayXus(1);                //������ʱ1us�ȴ�����ʾ��ʱ��ʼ
        DQ = 1;                     //�ͷ����ߣ��˺�DS18B20���������,�����ݴ��䵽������
        DelayXus(5);                //��ʱ5us,�˴������Ƽ��Ķ�ʱ��ͼ�������ѿ���������ʱ��ŵ���ʱ����15us�ڵ���󲿷�
        if (DQ)                     //����λΪ��1��
        {
            dat |= 0x80;            //��ȡ����
        }
        DelayXus(60);               //�ȴ�ʱ��Ƭ����
    }
    return dat;
}
uint8_t DS18B20_Reset()         //�ж��Ƿ����
{
    uint8_t i = 5;
	CY = 1;
	while(CY && (i--))				//���5�Σ���������ѭ��
    {
		DQ = 0;                     //�ͳ��͵�ƽ��λ�ź�
	    DelayXus(240);              //��ʱ����480us
	    DelayXus(240);				
	    DQ = 1;                     //�ͷ�������
	    DelayXus(60);               //�ȴ�60us
	    CY = DQ;                    //����������
	    DelayXus(240);              //�ȴ��豸�ͷ�������
	    DelayXus(180);
	}
	if(CY)
	{
		return 0;		//DS18B20������
	}
	else
	{
		return 1;		//DS18B20����
	}
}

void DS18B20_StartCovert()
{
    if (DS18B20_Reset())
    {
        DS18B20_Write_Byte(0xcc);	// skip rom
        DS18B20_Write_Byte(0x44);	// convert
    }
}
uint8_t DS18B20_GetTemp(int16_t *temp)
{
    uint8_t tt;
    uint8_t xdata t_data[9]={0};
    uint8_t i;
    uint8_t TL,TH;
    *temp=0;
    if (DS18B20_Reset())
    {
        DS18B20_Write_Byte(0xcc);	// skip rom
        DS18B20_Write_Byte(0x44);	// convert
        DS18B20_Reset();
        DS18B20_Write_Byte(0xcc);	// skip rom
        DS18B20_Write_Byte(0xbe);	// read temp	    
    	for(i=0;i<9;i++)
        {
            t_data[i]=DS18B20_Read_Byte();
        }  
        if(calculate_CRC8(t_data,9)==0)
        {
            TL=t_data[0];
            TH=t_data[1];
            if(TH>7)
            {
                TH=~TH;
                TL=~TL; 
                tt=0;					//�¶�Ϊ��  
            }
            else 
            {   tt=1;    }			//�¶�Ϊ��	  	  
            *temp=TH; 					//��ø߰�λ
            *temp<<=8;    
            *temp+=TL;					//��õͰ�λ
            *temp=((float)(*temp))*0.625;		//ת��     
            if(tt == 0)
            {
                 *temp=-(*temp); 		//���¶�ֵ
            }
            return 0;
        }
        else
        {
            return 2;   //CRC����
        }
    }
    else
    {
        return 1;   //DS18B20������
    }
}
