#ifndef __MODULE_COMM_H
#define __MODULE_COMM_H


#include "main.h"



void Module_COMM(void);

void LL_Module_Send(const void* str,uint8_t str_len);   //Ҫȷ��Э��ͷβ�Ƿ���ȷ


//����(Routine)����ͷ Function 
void ModuleRoutine_Shakehand(void);
void ModuleRoutine_GetUsedTime();






//308����ͷ(UI/IU)
void Module308_Shakehand(void);




//�ĺ�һ ������ UVA1
void Module_GetTemp();
void Module_Send_PWM(uint8_t light_sel,uint8_t duty);



#endif



