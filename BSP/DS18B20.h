#ifndef __DS18B20_H
#define __DS18B20_H

#include "main.h"

sbit DQ = P1^1;                     	//DS18B20的数据口位P1.1


void DS18B20_StartCovert();
uint8_t DS18B20_GetTemp(int16_t *temp);  //0->ok  1->不存在 2->crc错误






#endif
