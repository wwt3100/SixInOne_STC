#ifndef __HMI_COMM_H
#define __HMI_COMM_H

#include "HMI_LL.h"

enum{
    eMsg_NULL=0,
    eMsg_HMI_Shakehand,
    eMsg_KeyDown,
    eMsg_KeyLongPush,
    eMsg_keyUp,
};


void HMI_COMM(void);


//User functions
void HMI_SendShakehand();
















#endif



