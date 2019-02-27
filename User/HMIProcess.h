#ifndef __HMI_PROCESS_H
#define __HMI_PROCESS_H

//³éÏó"³¡¾°"
enum{
    eScene_Startup=0,
    eScene_StartPage,
    eScene_Wait,
    eScene_Module_650,
    eScene_Module_633,
    eScene_Module_UVA1,
    eScene_Module_IU,
    eScene_Module_308,
    eScene_Module_WiraAuto,
    eScene_Module_4in1Auto,
    eScene_Module_Wira,
    eScene_Module_4in1,
    eScene_Module_WiraSetup,
    eScene_Module_4in1Setup,
    eScene_Module_WiraUsedtime,
    eScene_Module_4in1Usedtime,
    eScene_Info,
    eScene_Error,
    eScene_Password,
    eScene_Debug,       //433 IU
    eScene_DebugUVA1,   //UVA1
    eScene_DebugNew,    //Wira 4in1
};





void HMI_Process(void);


void HMI_Scene_Recovery();
void HMI_Goto_Error();
void HMI_Exit_Error();















#endif



