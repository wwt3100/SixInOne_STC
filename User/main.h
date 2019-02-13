#ifndef __MAIN_H
#define __MAIN_H

#include "STC12C5Axx.h"

#include <intrins.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"


#define LANG_CH     0
#define LANG_EN     1

#define ENABLE        1
#define DISABLE       0

typedef struct Golbal_comInfo{
    uint8_t ModuleType;
    uint8_t WorkStat;       //0-未接治疗头         1-停止    2-暂停    3-运行

    uint8_t HMI_Scene;      //HMI场景
    uint8_t HMI_LastScene;  //保存上一个场景

    uint8_t TimerCounter;

    uint8_t HMIMsg;
    union HMIArg{
        uint8_t  data8[2];
        uint16_t data16;
    }HMIArg;

    uint8_t COMMProtocol_Head;
    uint8_t COMMProtocol_Tail1;
    uint8_t COMMProtocol_Tail2;
}_Golbal_comInfo;

typedef struct Golbal_Config{
    uint8_t LANG;           //0-中文   1-英文
}_Golbal_Config;

typedef union Golbal_Info{
    struct NormalModule{
        uint8_t LightMode;          //0->连续  1->脉冲
        uint8_t WorkTime;           //工作时间用分钟表示
        uint8_t PowerLevel;         //光功率大小
        uint16_t RemainTime;         //剩余时间,用于暂停等
        
    }NormalModule;
    struct New4in1Module{
        uint8_t LightMode [4][4];       //LightMode[0]&0x80==0x80 同时输出 低4bit表示光选择   else 顺序 [1-4]为出光顺序
                                        //需要预先读出工作模式,显示到界面上
        uint8_t PowerLevel[4];
        uint8_t WorkTime  [4];      //设置的时间,单位min
        uint16_t RemainTime[4];     //剩余时间,单位s
    }New4in1Module;
}_Golbal_Info;





void BeepEx(uint8_t time);

extern _Golbal_comInfo idata gComInfo;
extern _Golbal_Config  idata gConfig;
extern _Golbal_Info    xdata gModuleInfo;

extern bit Uart1_Busy;
extern bit Uart2_Busy;

extern bit Uart1_buf_sel;
extern bit Uart1_ReviceFrame;
extern bit Uart2_buf_sel;
extern bit Uart2_ReviceFrame;


extern uint8_t xdata uart1_buff[2][32];
extern uint8_t xdata uart2_buff[2][32];

extern bit HMI_Msg_Flag;

extern bit SystemTime100ms;
extern bit SystemTime1s;


#endif

