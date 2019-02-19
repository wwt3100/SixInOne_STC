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
    for (i=0; i<8; i++)             //8位计数器
    {
        DQ = 0;                     //拉低总线
        DelayXus(1);                //至少维持了1us,表示写时序(包括写0时序或写1时序)开始
        dat >>= 1;                  //从字节的最低位开始传输，必须在拉低总线后的15us内,
        DQ = CY;					//因为15us后DS18B20会对总线采样
        DelayXus(60);               //必须让写时序持续至少60us
        DQ = 1;                     //写完后,必须释放总线
        DelayXus(1);                //恢复延时
    }
}
uint8_t DS18B20_Read_Byte()
{
    uint8_t i;
    uint8_t dat = 0;
    for (i=0; i<8; i++)             //8位计数器
    {
        dat >>= 1;                  //让从总线上读到的位数据，依次从高位移动到低位
        DQ = 0;                     //将总线拉低，要在1us之后释放总线，单片机要在此下降沿后的15us内读数据才会有效。
        DelayXus(1);                //至少延时1us等待，表示读时序开始
        DQ = 1;                     //释放总线，此后DS18B20会控制总线,把数据传输到总线上
        DelayXus(5);                //延时5us,此处参照推荐的读时序图，尽量把控制器采样时间放到读时序后的15us内的最后部分
        if (DQ)                     //数据位为‘1’
        {
            dat |= 0x80;            //读取数据
        }
        DelayXus(60);               //等待时间片结束
    }
    return dat;
}
uint8_t DS18B20_Reset()         //判断是否存在
{
    uint8_t i = 5;
	CY = 1;
	while(CY && (i--))				//检测5次，不设置死循环
    {
		DQ = 0;                     //送出低电平复位信号
	    DelayXus(240);              //延时至少480us
	    DelayXus(240);				
	    DQ = 1;                     //释放数据线
	    DelayXus(60);               //等待60us
	    CY = DQ;                    //检测存在脉冲
	    DelayXus(240);              //等待设备释放数据线
	    DelayXus(180);
	}
	if(CY)
	{
		return 0;		//DS18B20不存在
	}
	else
	{
		return 1;		//DS18B20存在
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
                tt=0;					//温度为负  
            }
            else 
            {   tt=1;    }			//温度为正	  	  
            *temp=TH; 					//获得高八位
            *temp<<=8;    
            *temp+=TL;					//获得低八位
            *temp=((float)(*temp))*0.625;		//转换     
            if(tt == 0)
            {
                 *temp=-(*temp); 		//负温度值
            }
            return 0;
        }
        else
        {
            return 2;   //CRC错误
        }
    }
    else
    {
        return 1;   //DS18B20不存在
    }
}
