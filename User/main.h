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

#define LANG_ZH     0       //����
#define LANG_EN     1       //Ӣ��

#define ENABLE        1
#define DISABLE       0

#define POWER_ON      0
#define POWER_OFF     1

#define M_Type_650      (1)
#define M_Type_633      (3)
#define M_Type_633_1    (0x33)
#define M_Type_IU       (0x10)  /* ��ȷ��δ֪ */

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

#define MODULE308_MAX_WORKTIME      (140)   /*�����ʱ��*/

typedef struct Golbal_comInfo{
    uint8_t ModuleType;
    uint8_t WorkStat;       //0-δ������ͷ         1-ֹͣ    2-��ͣ    3-����
    
    uint8_t Count;          //������
    uint8_t TempCount;      //�¶ȴ������
    uint8_t ErrorCode;

    uint8_t HMI_Scene;      //HMI����
    uint8_t HMI_LastScene;  //������һ������

    uint8_t TimerCounter;
    uint8_t TimerCounter2;

    uint8_t HMIMsg;
    uint8_t HMIArg1,HMIArg2;    //��Ҫ�������ݴ�������ť����

    uint8_t COMMProtocol_Head;
    uint8_t COMMProtocol_Tail1;
    uint8_t COMMProtocol_Tail2;
    
    uint16_t FeedbackVolt;  //��Դ������ѹֵ          ����?
}_Golbal_comInfo;

typedef struct Golbal_Config{
    uint8_t LANG;           //0-����   1-Ӣ��
}_Golbal_Config;

typedef struct Golbal_Info{
    union ModuleInfo{
        struct RoutineModule{
            uint8_t LightMode;          //0->����   1->����
            uint8_t WorkTime;           //����ʱ���÷��ӱ�ʾ
            uint8_t Temp;               //����ͷ�¶�,ֻ����ֵ
            uint16_t PowerLevel;         //�⹦�ʴ�С
            uint16_t RemainTime;         //ʣ��ʱ��,������ͣ��  

            uint16_t DAC_Val;           //��ǰDAC
            uint16_t DAC_Cail;          //У׼DAC
            
            uint32_t UsedTime;          //�Ѿ�ʹ��ʱ��
            uint32_t UsedCount;         //�Ѿ�ʹ�ô���
        }RoutineModule;
        struct New4in1Module{
            uint8_t ConfigSel;          //����ѡ��
            uint8_t ConfigSelLight;     //ѡ��
            uint8_t LightMode;          //ѡ�����ģʽ 0ר�� 1-4����
            uint8_t StepSel;
            uint8_t LightStep [5][27];       //���ⲽ��
                                            //��ҪԤ�ȶ�������ģʽ,��ʾ��������
                                            //[0]bit7ģʽ bit6-0����,[3*n+1]��,[3*n+2]����,[3*n+3]ʱ��
            uint8_t PowerLevel[4];
            uint8_t WorkTime[5];        //���õ�ʱ��,��λmin             [0]ͬ��ģʽʱ�� [1-4]˳��ģʽ4��
            uint16_t RemainTime[4];     //ʣ��ʱ��,��λs
        }New4in1Module;
        struct mini308Module{
            uint8_t WorkMode;       // 0->����ģʽ 1->��߲���
            uint8_t TestSelTime;    //���ģʽѡ���ʱ��
            uint8_t TestWorkTime;   //��߲��ԵĹ���ʱ��
            uint8_t WorkTime;      //����ʱ���140s Ԥ��2byte
            uint8_t OnceWorkTime;  //���ι���ʱ��,���ڱ���
            uint8_t RemainTime;    //ʣ��ʱ��
            uint16_t TotalTime;     //�ܺ�߲���ʱ��
            uint16_t Freq;      //����Ƶ��
            uint16_t Duty;      //ռ�ձ�
        }mini308Module;
    }ModuleInfo;
    uint8_t DebugOpen;      //������ͷ�ܽ��ܵ�����,λ����,�ݶ����8��
    char    Password[9];    //8�ֽ�+��β0
    uint8_t PasswordLen;
    struct Debug{
        uint8_t dac; //0-50
    }Debug;
}_Golbal_Info;

typedef struct ModuleSave{
    uint8_t UsedTime[7];
    uint8_t UsedCount[5];   
    uint16_t DAC_Cail;      //DACУ׼ֵ
    uint8_t crc;
}_ModuleSave;


void Delay10ms();		//@11.0592MHz

void LOG_E(void*str,...);


void BeepEx(uint8_t time);

//���д256byte
void Save_Config();
void Save_ModuleSomething();
void SPI_Send(uint16_t dat);    //����48V��Դ

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
extern bit Pause_Flag;  //308��ͣ��


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

