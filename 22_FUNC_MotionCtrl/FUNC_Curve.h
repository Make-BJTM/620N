/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_Curve.h
 创建人：张小华
 修订人：                      修订日期：2012.09.05 
 描述： 
     1.后台模式控制伺服的实现

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/
#ifndef SERVO_CURVE_H
#define SERVO_VURVE_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define   CURVE_STARTSPD  (12)
#define   CURVE_STOPSPD   (12)


#define   STOP            (0)
#define   CCWRUN          (1)
#define   CWRUN           (2)

#define   GUIRECOVER      (0 )
#define   GUICURVEEN      (50)

#define   JOGAMPBIT       (16)
#define   ONCE_Time       (0)
#define   CYCLE           (1)

#define   ACCELERATESPEED (1000)     


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct  _str_CURVEREG{
    int16  RunSpeed        ; //Jog运行速度
    int16  RiseDownTime    ; //加减速时间
    int32  PulseFreqQ16    ; //脉冲频率
    int32  UpPulseQ16      ; //加减速对应频率
    int32  PulseRemQ16     ; //脉冲余数
    Uint16 JogRun          ; //JOG运行状态 0：STOP不运行，1：CCWRUN正方向运行 ，2：CWRUN反方向运行
    int32  PlanIntpltValQ16    ;
    int32  PlanIntpltValQ16Rem ;
	int32  EleGearRem          ; //电子齿轮比计算余数清楚
    int32  JogOut              ; //JOG输出

    int16  PulseSendSel ;    //定位试运行

    int32  CCWCoordinate;	//定位坐标1
    int32  CWCoordinate ;	//定位坐标2
    int32  JogPulseSend ;   //脉冲发送个数
    int32  MoveDist     ;   //旋转距离
    int32  FedToPosRef  ;
    Uint16 PulseSendEn  ;   //坐标设定使能
    Uint16 WaitTime     ;   //等待时间
    Uint16 CWCCWMaxSet  ;	//正转极限位置设定完成
	Uint32 Denominator ;    //后台模式分母
	Uint32 Numerator   ;    //后台模式分子
	Uint32 MaxPulse    ;    //最大脉冲数
      	  
}str_CURVE;

#define CurvePCMD_Defaults {0,0,0,0,0,0,{0},{0},{0},{0}}

typedef struct  _STR_CURVE_POS { 
    // 多段位置功能共用变量
    Uint16 ExeBlockNum;           //选择执行的总段数，最大16
    Uint16 CurrentBlock;          //记录当前段			  
    Uint16 WaitFlag;              //多段位置等待标志			   
    Uint16 RunFlag;               //多段位置运行标志
    Uint16 RunMode;               //多段运行模式
    Uint16 RefleshNum;            //段更新标志
   
    // 各段参数		
    Uint32 RunSpeed[2];          //记录每段恒速
    Uint16 RiseorDownTime[2];    //记录每段的上升或下降时间	   
    int32  PulsNumPerBlock[2];   //记录每段的总脉冲数
    Uint32 WaitTimePerBlock[3];  //记录每段的等待时间  
}STR_CURVE_BLOCK_POS;	   
/*FUNC_GUIControl.c文件内调用的变量的结构体类型*/


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern str_CURVE  STR_Curve;
extern void CurveDataUp(void);
extern void CurveStopDataUp(void);
extern void CurveInit(void);
extern void CurveReset(void);  // 定位试运行插补复位
extern int32 CurveHandle(void);
extern int32 JogCmdGenerator(void); 





#ifdef __cplusplus
}
#endif  /* extern "C" */
#endif


