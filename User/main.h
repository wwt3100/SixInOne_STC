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

#define LANG_CH     0
#define LANG_EN     1

#define ENABLE        1
#define DISABLE       0

#define POWER_ON      0
#define POWER_OFF     1

#define M_Type_650      (1)
#define M_Type_633      (3)
#define M_Type_633_1    (0x33)
#define M_Type_IU       (0x10)  /* ��ȷ��δ֪ */
#define M_Type_UVA1     (4)
#define M_Type_Wira     (0x43)
#define M_Type_4in1     (0x41)

#define Error_NoModule          (1)
#define Error_PasswordError     (2)
#define Error_TempSenerError    (3)


typedef struct Golbal_comInfo{
    uint8_t ModuleType;
    uint8_t WorkStat;       //0-δ������ͷ         1-ֹͣ    2-��ͣ    3-����
    
    uint8_t Count;          //������
    uint8_t TempCount;      //�¶ȴ������
    uint8_t ErrorCode;

    uint8_t HMI_Scene;      //HMI����
    uint8_t HMI_LastScene;  //������һ������

    uint8_t TimerCounter;

    uint8_t HMIMsg;
    uint8_t HMIArg1,HMIArg2;

    uint8_t COMMProtocol_Head;
    uint8_t COMMProtocol_Tail1;
    uint8_t COMMProtocol_Tail2;
}_Golbal_comInfo;

typedef struct Golbal_Config{
    uint8_t LANG;           //0-����   1-Ӣ��
}_Golbal_Config;

typedef union Golbal_Info{
    struct RoutineModule{
        uint8_t LightMode;          //0->����  1->����
        uint8_t WorkTime;           //����ʱ���÷��ӱ�ʾ
        uint8_t Temp;               //����ͷ�¶�,ֻ����ֵ
        uint16_t PowerLevel;         //�⹦�ʴ�С
        uint16_t RemainTime;         //ʣ��ʱ��,������ͣ��
        uint32_t UsedTime;          //�Ѿ�ʹ��ʱ��
        uint32_t UsedCount;         //�Ѿ�ʹ�ô���
    }RoutineModule;
    struct New4in1Module{
        uint8_t LightMode [4][4];       //LightMode[0]&0x80==0x80 ͬʱ��� ��4bit��ʾ��ѡ��   else ˳�� [1-4]Ϊ����˳��
                                        //��ҪԤ�ȶ�������ģʽ,��ʾ��������
        uint8_t PowerLevel[4];
        uint8_t WorkTime  [4];      //���õ�ʱ��,��λmin
        uint16_t RemainTime[4];     //ʣ��ʱ��,��λs
    }New4in1Module;
}_Golbal_Info;



void LOG_E(void*str,...);


void BeepEx(uint8_t time);

//���д256byte
void Save_Config();
void SPI_Send(uint16_t dat);    //����48V��Դ

extern _Golbal_comInfo idata gComInfo;
extern _Golbal_Config  idata gConfig;
extern _Golbal_Info    xdata gModuleInfo;

extern bit Uart1_Busy;
extern bit Uart2_Busy;

extern bit Uart1_buf_sel;
extern bit Uart1_ReviceFrame;
extern bit Uart2_buf_sel;
extern bit Uart2_ReviceFrame;


extern uint8_t idata uart1_buff[18];
extern uint8_t idata uart2_buff[18];

extern bit HMI_Msg_Flag;

extern bit Fire_Flag;

extern bit SystemTime100ms;
extern bit SystemTime1s;
extern bit Heardbeat1s;

#endif

