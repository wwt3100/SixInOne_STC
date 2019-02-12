#ifndef __DEFINE_H
#define __DEFINE_H


typedef unsigned char   uint8_t;	
typedef unsigned short  uint16_t;	
typedef unsigned long   uint32_t;	

typedef char   int8_t;	
typedef short  int16_t;	
typedef long   int32_t;	


#define RELOAD_COUNT 0xFD    				//11.0592MHz Crystial,1T,SMOD = 0,115200bps
#define S2RI 0x01		 //S2CON.0
#define S2TI 0x02		 //S2CON.1
#define S2RB8 0x04		 //S2CON.2
#define S2TB8 0x08		 //S2CON.3


/* Define SPI*/
#define SPIF 0x80	//SPSTAT.7
#define WCOL 0X40	//SPSTAT.6
#define SSIG 0X80	//SPCTL.7
#define SPEN 0X40	//SPCTL.6
#define DORD 0X20	//SPCTL.5
#define MSTR 0X10	//SPCTL.4
#define CPOL 0X08	//SPCTL.3
#define CPHA 0X04	//SPCTL.2
#define SPDHH 0X00	//CPU_CLK/4

/*Define ADC operation const for ADC_CONTR*/
#define ADC_POWER   0x80            //ADC power control bit
#define ADC_FLAG    0x10            //ADC complete flag
#define ADC_START   0x08            //ADC start control bit
#define ADC_CHANNEL 0x04			//ADC select channel P1.4,0-P1.0,1-P1.1,...5-P1.5,6-P1.6,7-P1.7
#define ADC_SPEEDLL 0x00            //540 clocks
#define ADC_SPEEDL  0x20            //360 clocks
#define ADC_SPEEDH  0x40            //180 clocks
#define ADC_SPEEDHH 0x60            //90 clocks








/*  GPIO  */
sbit BEEP_IO        = P3^6;
sbit FAN_IO         = P1^6;
sbit KEY_LED_IO     = P3^7;

sbit PowerCtr_Main       = P3^3;
sbit PowerCtr_Light1     = P2^1;    //¿ØÖÆ48V
sbit PowerCtr_Light2     = P2^2;
sbit PowerCtr_Module12v  = P3^4;










#endif
