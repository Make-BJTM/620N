/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_DCBusVolt.h

 创建人：王军干              创建日期：2008.11.10
 修改人：朱祥华              修改日期：2011.11.21 
 描述： 
      电机、驱动器过载保护数据表单。
      驱动器过载数据表单分以下四种类型实验测试得到
    Size_E  7.5KW 驱动器       Size_E  5KW 至6KW  驱动器 
    Size - C&D且1kw以上驱动器  小功率Size - A&B且1kw以下驱动器
 修改记录：
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_MONITOR_DCBUSVOLT_H
#define FUNC_MONITOR_DCBUSVOLT_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define PowerOnWaitTime      18L    //等待10ms再检测是否欠电压，以防启机电压还未建立即报欠压
#define DC_MONIT_FREQ        0x01        // 2m延时，判断母线电压的变化率   


//继电器打开接入限流电阻软启动使能SS_Enable     继电器闭合禁止软启动SS_Disable------
#define   SS_Disable  0
#define   SS_Enable   1 


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//----------------------------------------------------------------------------
//AD采样函数
//片内AD采样数据结构体,此结构体内的变量直接可以被算法调用，其量纲符合算法调度
typedef struct{
    Uint32  BrkPrd_Q8;         //泄放开关管调度周期时间，单位ms Q8 

    //软启动监控变量
    Uint16  SSMonitCnt;        //软启动时监控次数
    int16   DeltaUdc;                   //实际母线电压在设定周期的变化率
    int16   UdcOld;         //上次母线电压采样值（监控周期1K，？？确定母线电压的变化率？？）
    Uint16  UdcMax;        //母线电压工作范围最高点
    Uint16  UdcMin;        //母线电压工作范围最低点
    Uint16  SoftStartFlag;    //
    //母线电压监控变量
    int16   UdcLow;           //母线电压欠压点 200
    int16   UdcOver;           //母线电压过压点；400
    int16   UdcLeak;      //母线电压泄放点  370
    Uint16  UnderUdcErrFlg;
//    int16   UdcRate;         //母线电压额定值 单位0.1V
    //过压滤放监控处理变量
    int16   EnableBrkCnt;
    int16   DisableBrkCnt;
    int64   Brk_HeatCapacity_Q8; //制动电阻热容量
    int64   Brk_HeatTotal_Q8;    //制动电阻累积当前热量
    int32   Brk_HeatAddDetal_Q8;    //每个制动管开关周期如开通贡献的热量
    int32   Brk_HeatSubDetal_Q8;    //每个制动管开关周期如关断释放的热量
    int16   MaxBrkT_RatePower;      // 制动电阻热容量计算常数  目前设定为允许额定功率下运行Xms
    Uint8   Brk_Status;

    //是否允许断电泄放标志位
    int16   PowerOffBleedFlg;     
    Uint16  RbTooSmallWarn;       //制动电阻阻值过小警告
    Uint16  UdcLowWarn;         //欠压警告

   //控制电检测
    int32 CtrlUdc;             //控制电母线电压
    int32 CtrlUdcCoeff_Q12;    //控制电母线电压系数
    int32 CutoffLvl;           //掉电阀值
    Uint8 CtrlUdcOk;           //控制电压ok标志位

    Uint8  UdcOkFlag;          //母线电压Ok 
    Uint8  BrkOC;              //Brk过流
}STR_DCBUSVOLTAGEPROCESS;
             

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitDCBusVoltProcess(void);          //母线电压相关初始化程序
extern void DCBusVoltErr_Monitor(void);          //母线电压监控相关报错
extern void DcBusVolt_SoftStart(void);           //软启动监控相关处理
extern void StopUpdateDCBusBrake(void);          //停机更新母线泄放程序，更新功能码之后执行
extern void INT_16K_DCBusBrkProcess(void);        //中断8K执行母线电压泄放开通关断程序
extern void DCBusVoltBrake_Monitor(void);        //1ms监控母线电压泄放,比如制动电阻是否过载
extern void CtrlDCBusVoltBrake_Monitor(void);    //控制电源母线电源监控
extern void LineVolt_Monitor(void);
#if POWERDRIVER_TYPE==POWDRV_IS650
extern void InitBrkCurrentCheck(void);
extern void INT_16KBrkCurrentCheck(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /*FUNC_Monitor_DCBusVolt.h*/    

/********************************* END OF FILE *********************************/
