/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_PosCtrl.h
 创建人：高小峰                
 修订人：李浩                 创建日期：11.09.23 
 描述： 
     1.转矩控制的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_POSCONTROL_H
#define FUNC_POSCONTROL_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

//wzg20111028 #include "FUNC_EleGear.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	




/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 



/*定义FUNC_PosCtrl.c文件内调用的变量的结构体类型*/
typedef struct{
    int64    PosRefToSpdCoeff_Q20;      //位置指令转化成速度值的计算系数

    int32    PosMultipleCmd;           //经过电子齿轮比放大倍数后的位置指令

    int32    PosFiltCmd;               //经过平均值滤波之后，低通滤波之前的位置指令

    int32    PosErrCnt;                //位置偏差计数器

    int32    PerrFaultValue;           //接收功能码设定的位置偏差过大判断的设定值

    //int32    SpdRef_Pulse;             //输出 AO 脉冲指令当前指示速度值

    //int32    CommPosStepMax;           //通讯位置指令在每次调度中的最大限制值

    int32    SpdRef_PulseTime;         //位置指令转化为速度的1ms调度计算时的计数值，其计时是在位置环中进行的

    int32    PosRefPer1ms;              //每1ms接收的位置指令

    int8    IsMultiPosCycleType;        //多段位置循环模式
}STR_LOCAL_POSCONTROL;


typedef struct {
    Uint16 HomeWork:1;         // 1: 正处于回原点过程(不能进行中断定长); 0: 不处于回原点过程
    Uint16 XiPosWork:1;        // 1: 正处于中断定长过程(不能进行回原点); 0: 不处于中断定长过程
    Uint16 PosRefDir:1;        // 位置指令方向  0：正转   1 反转 中断定长使用
    Uint16 Xintcoin:1;         //中断定长完成信号
    Uint16 LowOscFlag:1;       //低频振动辨识完成标志位，0：未辨识，1：辨识完成
    Uint16 AbsMod1CalcPosOffset:2;  //绝对位置模式1 原点回归后计算位置偏置
    Uint16 AbsMod2RestRemSum:1;     //绝对位置模式2 复位余数和
    Uint16 ExChangeEn:1;
	Uint16 CanopenHomeWork:1;
	Uint16 Rsvd:6;            // 备用
}STR_MUTEXBIT;

typedef union {
    volatile Uint16          all;
    volatile STR_MUTEXBIT    bit;
}UNI_MUTEXREG;                        // 位置模式下互斥功能控制标志位

/* 位置控制模块内多个文件间需要交互的变量 */
typedef struct{

    //先定义64位变量
    int64    CurrentAbsPos;            //当前绝对位置，也就是增量式的位置反馈的累加和，即为原变量 gstr_Gvar.CurrentPos
    int64    CurrentAbsPos_ToqInt;     //当前绝对位置 电流环中断更新
    int64    PosActualValInUser;       //用户单位
    //int32    PosFdb_ToqInt;
    int64    ExCurrentAbsPos_ToqInt;   //当前外部编码器绝对位置 电流环中断更新
    int64    AbsMod2MechSingleAbsPos;  //旋转负载单圈位置
    int64    AbsMod2MechSingleAbsPos_ToqInt;//旋转负载单圈位置,电流环中断更新
    Uint64   RPM2PPPtCoefQ16;          //用于将rpm表示的转速换算到以脉冲每位置环周期(PPPt)为单位的系数
    Uint64	 ExRPM2PPPtCoefQ16;		   //用于将rpm表示的转速换算到以脉冲每位置环周期(PPPt)为单位的系数(外环)
	Uint64   AbsMod2PosUpLmt;        //多圈模式2位置上限值

    //定义32位变量
    int32    PulsePosRef;              //绝对位置指令 低速
    int32    PulsePosRef2;             //绝对位置指令 高速
    int32    PulsePosRef3;             //全闭环当做脉冲输入端口时的绝对位置指令 高速1M
    int32    DeltaPulse;               //脉冲增量

    int32    FdFwdOut;                 //位置环速度前馈（在位置控制和位置调节器两个源文件之间调用）
    int32    PosRefIncrLmt;            //增量式位置指令幅值限制值,该变量需要提供给增量式位置脉冲计算函数使用
    int32    PosErrLast;               //上一次位置偏差记录值STR_FUNC_Gvar.PosCtrl.PosAmplifErr
    int32    PosErrLastT;              //上上周期的位置偏差

	int64    InputPulsCntRT;           //实时记录输入脉冲数
    int64    InputPulseCnt;            //SON时输入指令脉冲计数器（32位十进制显示）H0B13 输入指令脉冲计数器
    int32    PulseCalcRemainder;       //InputPulseCnt(输入指令脉冲计数)计算时的余数

    int32    PosFdbAbsValueLast;       //上次的位置反馈绝对值 反转时可能为负值

    int32    Parrive;                  //位置到达阈值
    int32    Pnear;                    //位置接近阈值

    int32    Numerator;                //位置环使用的电子齿轮分子 (最大值为500)  
    int32    Denominator;              //位置环使用的电子齿轮分母 

    Uint32   Ms2PtCoefQ13;             //用于将ms表示的时间换算到位置环周期为单位的系数

    int32    PosLPFilterRemainder;     //位置环低通滤波器计算余数

    //定义16位变量
    Uint16   LowOscPulsCnt;            //根据编码器转换的低频振动脉冲阈值

	int32  CoinHoldTime;
    volatile UNI_MUTEXREG    MutexBit;          // 位置模式下互斥功能控制标志位
}STR_INNER_GLOBAL_POSCONTROL;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_INNER_GLOBAL_POSCONTROL    STR_InnerGvarPosCtrl; //该结构体中有个两变量需要在位置控制中的其他文件内调用
extern STR_LOCAL_POSCONTROL           STR_PosCtrl;    //FUNC_PosCtrl.c文件内结构体变量

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

/*FUNC_PosCtrl.c模块共享全局函数的声明*/
extern void PosControl(void);
extern void InitPosCtrl(void); 
extern void PosCtrlStopUpdata(void);
extern void PosCtrlUpdata(void);
extern void PosCtrl_MainLoop(void);
extern void ClrPosReg(void);
extern void PosShow(void);
extern void ECTInteruptPosUpdate(void);
#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_PosCtrl.h */

/********************************* END OF FILE *********************************/
