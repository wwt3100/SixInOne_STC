#ifndef _STC_EEPROM_h_
#define _STC_EEPROM_h_

#define DATA_FLASH_START_ADDRESS 0x00  		//STC5Axx 系列 EEPROM 扇区0起始地址
#define DATA2_FLASH_START_ADDRESS 0x200  	//STC5Axx 系列 EEPROM 扇区1起始地址

/**
  STC_EEPROM 0x200, store data: 
  first 7 bytes, IR treat count and time,  allocated 8 bytes
  followed by 4bytes FRAC used time, allocated 8 bytes (first 4bytes stored used count, last 4 bytes stored total times)
  followed by 4bytes TC used time, allocated 8 bytes
*/
#define STC_EEP_IR_DATA         8
#define STC_EEP_FRAC_DATA       8
#define STC_EEP_TC_DATA         8

extern unsigned char Byte_Read(unsigned int add);      			//读一字节
extern void Byte_Program(unsigned int add, unsigned char ch);   //字节编程，调用前需打开IAP 功能
extern void Sector_Erase(unsigned int add);            			//擦除扇区

extern void IAP_Disable();

#endif