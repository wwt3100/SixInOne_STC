#ifndef __HMI_PROCESS_H
#define __HMI_PROCESS_H


enum{
    eScene_Startup=0,
    eScene_StartPage,
    eScene_Module_650,
    eScene_Module_633,
    eScene_Module_UVA1,
    eScene_Module_IU,
    eScene_Module_308,
    eScene_Module_Wira,
    eScene_Module_4in1,
    eScene_Info,
    eScene_Debug,
    eScene_Error,
};





void HMI_Process(void);

















#endif



