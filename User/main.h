/*
    注1: 变量一般使用帕斯卡命名,全局变量一般用g开头,局部变量使用下划线命名或m_开头
    注2: define的值一般为全大写
    注3: 除bit变量外,其他全局变量使用结构体打包(bit变量可用位域代替,但代码效率不如直接bit高)
    注4: 串口接收buffer因执行效率考虑放idata段,串口发送用阻塞式
    注5: 治疗头数据比较大且并不会几个同时用,故使用union减少空间占用
    注6: 串口屏视图抽象成Scene(场景),一个Scene对应1个或多个page(页面)
    注7: 往串口屏写字符前需手动更新字符背景,否则会造成字符重叠,若重新刷新page则不需更新背景
    注8: 程序中Module(组件/模块)对应的含义为治疗头
    注9: 需求上的4个智能模式+专家模式,抽象成5个Group(组),数据皆可修改,只是入口不一样
    注10:Group里的步骤抽象成Step(工步),详见struct LightStep
    注11:每个Module都有自己的调试权限,通过位选择,暂只有4种,故只用1byte
*/
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

#define LANG_ZH     (0)       //中文
#define LANG_EN     (1)       //英文

#define ENABLE        (1)
#define DISABLE       (0)

#define POWER_ON      (0)
#define POWER_OFF     (1)

#define M_Type_650      (1)
#define M_Type_633      (3)
#define M_Type_633_1    (0x33)
#define M_Type_IU       (0x10)  /* 正确性未知 */

#define M_Type_308      (0x0B)
#define M_Type_UVA1     (4)
#define M_Type_Wira     (0x43)  /*DermaII*/
#define M_Type_4in1     (0x41)  /*DermaI*/

#define Error_NoModule          (1)
#define Error_PasswordError     (2)
#define Error_TempSenerError    (3)

#define OPEN_DBG_Calib              (0x80)
#define OPEN_DBG_ClearUsedtime      (0x40)
#define OPEN_DBG_Config             (0x20)
#define OPEN_DBG_ROOT               (0x10)

#define MODULE308_MAX_WORKTIME      (140)   /*最大工作时间*/

#define STR_NEW4IN1_L1WL "590nm",5
#define STR_NEW4IN1_L2WL "830nm",5
#define STR_NEW4IN1_L3WL "415nm",5
#define STR_NEW4IN1_L4WL "633nm",5

#define STR_NEWWIRA_L1WL "633nm",5
#define STR_NEWWIRA_L2WL "810nm",5
#define STR_NEWWIRA_L3WL "940nm",5
#define STR_NEWWIRA_L4WL "830nm",5

#define STEP_MODE_Serial        (0)     /*顺序*/
#define STEP_MODE_Parallel      (1)     /*同步*/


#if 1
#define MIN2S 
#else
#define MIN2S *60
#endif

typedef struct Golbal_ComInfo{  
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
}_Golbal_ComInfo;

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
        }Routine;
        struct New4in1{
            uint8_t ConfigSel;           //设置选择
            uint8_t ConfigSelLight;      //选择   bit7:用于强制刷新 
            uint8_t LightGroup;          //选择出光模式 0专家 1-4智能
            uint8_t LastSelGroup;        //上次选择的模式 1-4
            uint8_t LocStep;        //当前工步
            uint8_t EditMode;       //编辑模式
            struct LightStep{
                uint8_t StepMode:1;     //0->顺序    1->同步
                uint8_t StepNum:7;      //工步数,最多4步 对于同步模式就是开启光的数量
                uint8_t Data[12];       //工步数据,[3*n+0]光,[3*n+1]能量,[3*n+2]时间(同步模式时间相同)
            }LightStep[5],TempStep;              //工步0为 专家模式
                                                 //设置的工步先保存到TempStep中,当保存或启动时再存入LightStep再保存
//            uint8_t Str_LightWavelength[4][7]; //4种光的波长 协议为2byte,转字符串为了加速显示//暂未用
            uint8_t PowerMax[4];        //最大能量
            uint8_t PowerMix[4];        //最小能量
            uint8_t PowerLevel[4];      //设置用的能量
            uint8_t WorkTime[5];        //设置用的时间,单位min             [0]同步模式时间 [1-4]顺序模式4光
            uint16_t RemainTime;     //剩余时间,单位s
        }New4in1;
        struct mini308{
            uint8_t WorkMode;       // 0->正常模式 1->红斑测试
            uint8_t TestSelTime;    //红斑模式选择的时间
            uint8_t TestWorkTime;   //红斑测试的工作时间
            uint8_t WorkTime;      //工作时间最长140s 预留2byte
            uint8_t OnceWorkTime;  //单次工作时间,用于保存
            uint8_t RemainTime;    //剩余时间
            uint16_t TotalTime;     //总红斑测试时间
            uint16_t Freq;      //工作频率
            uint16_t Duty;      //占空比
        }mini308;
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

#ifdef _DEBUG
void LOG_E(void*str,...);
#else
#define LOG_E(...)
#endif

void Beep(uint8_t time);
void BeepEx(uint8_t count,uint8_t time);    //count 响的次数 time 50ms整数时间
 
void ADC_Init(void);
//最多写256byte
void Save_Config();
void Save_ModuleSomething();
void SPI_Send(uint16_t dat);    //控制48V电源

extern _Golbal_ComInfo idata gCom;
extern _Golbal_Config  idata gConfig;
extern _Golbal_Info    xdata gInfo;
extern _ModuleSave xdata gModuleSave;

extern uint8_t idata uart1_buff[18];
extern uint8_t idata uart2_buff[20];

extern bit SystemTime100ms;
extern bit SystemTime1s;
extern bit SystemTime1s_1;
extern bit Heardbeat1s;

extern bit Resend_getUsedtime;
extern bit Resend_getCalibData;

extern bit Dbg_Flag_DAC5V;
extern bit Dbg_Flag_MainPower;
extern bit Dbg_Admin;

//串口相关位
extern bit Uart1_Busy;
extern bit Uart2_Busy;
extern bit Uart1_ReviceFrame;
extern bit Uart2_ReviceFrame;

//标志位
extern bit Fire_Flag;
extern bit Fire_MaxOut;  //用于慢启动 633 UVA1
extern bit Pause_Flag;   //308暂停用
extern bit ADConvertDone;


#endif


