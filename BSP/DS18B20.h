#ifndef __DS18B20_H
#define __DS18B20_H

#include "main.h"

sbit DQ = P1^1;                     	//DS18B20�����ݿ�λP1.1


void DS18B20_StartCovert();
uint8_t DS18B20_GetTemp(int16_t *temp);  //0->ok  1->������ 2->crc����






#endif
