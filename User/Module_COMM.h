#ifndef __MODULE_COMM_H
#define __MODULE_COMM_H


#include "main.h"



void Module_COMM(void);

void LL_Module_Send(const void* str,uint8_t str_len);   //要确认协议头尾是否正确


//常规(Routine)治疗头 Function 
void ModuleRoutine_Shakehand(void);






//308治疗头(UI/IU)
void Module308_Shakehand(void);







#endif



