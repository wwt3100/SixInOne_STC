#ifndef __HMI_PROCESS_H
#define __HMI_PROCESS_H

//抽象"场景"
enum HMI_Scene
{
    eScene_Startup=0,
    eScene_StartPage,
    eScene_Wait,
    eScene_Module_650,
    eScene_Module_633,
    eScene_Module_UVA1,
    eScene_Module_IU,
    eScene_Module_308Wait,
    eScene_Module_308,
    eScene_Module_308test,      //308红斑测试
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


//串口屏页面         英文页面+100
enum HMI_Page
{
    ePage_Startup       =0,
    ePage_Main          =1,
    ePage_Module650     =2,
    ePage_Module633     =3,
    ePage_ModuleUVA1    =4,
    ePage_Loading308    =5,
    ePage_ModuleIU      =6,
    ePage_Module308     =7,
    ePage_Module308Test =9,
    ePage_PasswordInput =17,
    ePage_Error_NoModule=35,
    ePage_Error_Password=36,
    ePage_Error_Black   =37,
    ePage_Debug=62,
    ePage_LogoEnd=204,
};

//串口屏按键码组
enum HMI_KeyCodeGroup
{
    eKeyCode_Group0=0,
    eKeyCode_Group1,
    eKeyCode_Group2,
    eKeyCode_Group3,
    eKeyCode_Group8=8,
    eKeyCode_Group10=0x10,
};

//串口屏按键
enum
{
    eKeyCodeG0_SetLangEN   =2,
    eKeyCodeG0_SetLangCN,
    eKeyCodeG1_TimeAdd     =1,
    eKeyCodeG1_TimeDec     =2,
    eKeyCodeG1_SysInfo     =3,
    eKeyCodeG1_StartPause  =4,
    eKeyCodeG1_Stop        =5,
    eKeyCodeG1_SwitchMode  =6,
};


void HMI_Process(void);


void HMI_Scene_Recovery();
void HMI_Goto_Error();
void HMI_Exit_Error();















#endif



