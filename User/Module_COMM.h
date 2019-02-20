#ifndef __MODULE_COMM_H
#define __MODULE_COMM_H


#include "main.h"



void Module_COMM(void);

void LL_Module_Send(const void* str,uint8_t str_len);   //要确认协议头尾是否正确


//常规(Routine)治疗头 Function 
void ModuleRoutine_Shakehand(void);
void ModuleRoutine_GetUsedTime();






//308治疗头(UI/IU)
void Module308_Shakehand(void);




//四合一 威伐光 UVA1
void Module_GetTemp();
void Module_Send_PWM(uint8_t light_sel,uint8_t duty);



#endif



