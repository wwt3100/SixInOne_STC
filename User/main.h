#ifndef __MAIN_H
#define __MAIN_H

#include "STC12C5Axx.h"

#include <intrins.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "crc.h"

#include "STC_EEPROM.h"
#include "DS18B20.h"

#define LANG_ZH     0       //中文
#define LANG_EN     1       //英文

#define ENABLE        1
#define DISABLE       0

#define POWER_ON      0
#define POWER_OFF     1

#define M_Type_650      (1)
#define M_Type_633      (3)
#define M_Type_633_1    (0x33)
#define M_Type_IU       (0x10)  /* 正确性未知 */

#define M_Type_308      (0x0B)
#define M_Type_UVA1     (4)
#define M_Type_Wira     (0x43)
#define M_Type_4in1     (0x41)

#define Error_NoModule          (1)
#define Error_PasswordError     (2)
#define Error_TempSenerError    (3)

#define OPEN_DBG_Calib              (0x80)
#define OPEN_DBG_ClearUsedtime      (0x40)
#define OPEN_DBG_Config             (0x20)
#define OPEN_DBG_ROOT               (0x10)

#define MODULE308_MAX_WORKTIME      (140)   /*最大工作时间*/

typedef struct Golbal_comInfo{
    uint8_t ModuleType;
    uint8_t WorkStat;       //0-未接治疗头         1-停止    2-暂停    3-运行
    
    uint8_t Count;          //计数用
    uint8_t TempCount;      //温度错误计数
    uint8_t ErrorCode;

    uint8_t HMI_Scene;      //HMI场景
    uint8_t HMI_LastScene;  //保存上一个场景

    uint8_t TimerCounter;
    uint8_t TimerCounter2;

    uint8_t HMIMsg;
    uint8_t HMIArg1,HMIArg2;    //主要用作传递串口屏按钮键码

    uint8_t COMMProtocol_Head;
    uint8_t COMMProtocol_Tail1;
    uint8_t COMMProtocol_Tail2;
    
    uint16_t FeedbackVolt;  //电源反馈电压值          电流?
}_Golbal_comInfo;

typedef struct Golbal_Config{
    uint8_t LANG;           //0-中文   1-英文
}_Golbal_Config;

typedef struct Golbal_Info{
    union ModuleInfo{
        struct RoutineModule{
            uint8_t LightMode;          //0->连续   1->脉冲
            uint8_t WorkTime;           //工作时间用分钟表示
            uint8_t Temp;               //治疗头温度,只有正值
            uint16_t PowerLevel;         //光功率大小
            uint16_t RemainTime;         //剩余时间,用于暂停等  

            uint16_t DAC_Val;           //当前DAC
            uint16_t DAC_Cail;          //校准DAC
            
            uint32_t UsedTime;          //已经使用时间
            uint32_t UsedCount;         //已经使用次数
        }RoutineModule;
        struct New4in1Module{
            uint8_t ConfigSel;          //设置选择
            uint8_t ConfigSelLight;     //选择
            uint8_t LightMode;          //选择出光模式 0专家 1-4智能
            uint8_t StepSel;
            uint8_t LightStep [5][27];       //出光步骤
                                            //需要预先读出工作模式,显示到界面上
                                            //[0]bit7模式 bit6-0步数,[3*n+1]光,[3*n+2]能量,[3*n+3]时间
            uint8_t PowerLevel[4];
            uint8_t WorkTime[5];        //设置的时间,单位min             [0]同步模式时间 [1-4]顺序模式4光
            uint16_t RemainTime[4];     //剩余时间,单位s
        }New4in1Module;
        struct mini308Module{
            uint8_t WorkMode;       // 0->正常模式 1->红斑测试
            uint8_t TestSelTime;    //红斑模式选择的时间
            uint8_t TestWorkTime;   //红斑测试的工作时间
            uint8_t WorkTime;      //工作时间最长140s 预留2byte
            uint8_t OnceWorkTime;  //单次工作时间,用于保存
            uint8_t RemainTime;    //剩余时间
            uint16_t TotalTime;     //总红斑测试时间
            uint16_t Freq;      //工作频率
            uint16_t Duty;      //占空比
        }mini308Module;
    }ModuleInfo;
    uint8_t DebugOpen;      //该治疗头能接受的密码,位控制,暂定最多8种
    char    Password[9];    //8字节+结尾0
    uint8_t PasswordLen;
    struct Debug{
        uint8_t dac; //0-50
    }Debug;
}_Golbal_Info;

typedef struct ModuleSave{
    uint8_t UsedTime[7];
    uint8_t UsedCount[5];   
    uint16_t DAC_Cail;      //DAC校准值
    uint8_t crc;
}_ModuleSave;


void Delay10ms();		//@11.0592MHz

void LOG_E(void*str,...);


void BeepEx(uint8_t time);

//最多写256byte
void Save_Config();
void Save_ModuleSomething();
void SPI_Send(uint16_t dat);    //控制48V电源

extern _Golbal_comInfo idata gComInfo;
extern _Golbal_Config  idata gConfig;
extern _Golbal_Info    xdata gInfo;
extern _ModuleSave xdata gModuleSave;


extern bit Uart1_Busy;
extern bit Uart2_Busy;
extern bit Uart1_ReviceFrame;
extern bit Uart2_ReviceFrame;
extern uint8_t idata uart1_buff[18];
extern uint8_t idata uart2_buff[18];

extern bit HMI_Msg_Flag;

extern bit Fire_Flag;
extern bit Pause_Flag;  //308暂停用


extern bit SystemTime100ms;
extern bit SystemTime1s;
extern bit SystemTime1s_1;
extern bit Heardbeat1s;

extern bit ADConvertDone;

extern bit Resend_getUsedtime;
extern bit Resend_getCalibData;

extern bit Dbg_Flag_DAC5V;
extern bit Dbg_Flag_MainPower;

extern bit Dbg_Admin;


#endif

