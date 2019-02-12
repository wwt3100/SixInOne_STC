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
    uint8_t WorkStat;       //0-δ������ͷ         1-ֹͣ    2-��ͣ    3-����

    uint8_t HMI_Scene;      //HMI����
    uint8_t HMI_LastScene;  //������һ������

    uint8_t HMIMsg;
    union HMIArg{
        uint8_t  data8[2];
        uint16_t data16;
    }HMIArg;
}_Golbal_comInfo;

typedef struct Golbal_Config{
    uint8_t LANG;           //0-����   1-Ӣ��
}_Golbal_Config;

typedef union Golbal_Info{
    struct NormalModule{
        uint8_t LightMode;          //0->����  1->����
        uint8_t WorkTime;           //����ʱ���÷��ӱ�ʾ
        uint8_t PowerLevel;         //�⹦�ʴ�С
        uint16_t RemainTime;         //ʣ��ʱ��,������ͣ��
        
    }NormalModule;
    struct New4in1Module{
        uint8_t LightMode [4][4];       //LightMode[0]&0x80==0x80 ͬʱ��� ��4bit��ʾ��ѡ��   else ˳�� [1-4]Ϊ����˳��
                                        //��ҪԤ�ȶ�������ģʽ,��ʾ��������
        uint8_t PowerLevel[4];
        uint8_t WorkTime  [4];      //���õ�ʱ��,��λmin
        uint16_t RemainTime[4];     //ʣ��ʱ��,��λs
    }New4in1Module;
}_Golbal_Info;







extern _Golbal_comInfo idata gComInfo;
extern _Golbal_Config  idata gConfig;
extern _Golbal_Info    xdata gModuleInfo;

extern bit Uart1_TXE;
extern bit Uart2_TXE;

extern bit Uart1_buf_sel;
extern bit Uart2_buf_sel;

extern uint8_t xdata uart1_buff[2][32];
extern uint8_t xdata uart2_buff[2][32];

extern bit HMI_Msg_Flag;


#endif

