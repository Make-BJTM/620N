/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_InterfaceProcess.h
 创建人:    李浩                  创建日期：2012.03.26
 描述:
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_INTERFACE_PROCESS_H
#define __FUNC_INTERFACE_PROCESS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h" 

#include "MTR_FUNCInterface.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


//FUNC模块传递给MTR模块,供MTR使用的位域结构体类型定义 (传递频率为16kHz)
//该结构体起到缓冲位结构体的作用，即收集需要发送到MTR模块的FUNC模块内的标志位
typedef struct{
    Uint32    RunMod:3;                     //运行模式
    Uint32    ServoRunStatus:2;             //指示伺服运行状态的变量,根据使能状态、故障状态合成的变量
    Uint32    PwmStatus:1;                  //PWM状态标志位
    Uint32    ToqStop:1;                    //急停减速使能标志
    Uint32    ZeroSpdStop:1;                //零速停机使能标志

    Uint32    OTClamp:1;                    //发生超程时零位置停机固定标志位
    Uint32    SpdReguDatClr:1;              //速度环偏差等参数清零标志位
    Uint32    AlmRst:1;                     //故障复位
    Uint32    Pcl:1;                        //正转外部转矩限制
    Uint32    Ncl:1;                        //反转外部转矩限制
    Uint32    UVAdjustRatioEn:1;            //UV相电流平衡校正使能
    Uint32    SpdToqLoopNact:1;             //速度转矩环程序屏蔽标志位
    Uint32    OffLnInertiaModeEn:1;         //离线惯量辨识使能

    Uint32    InnerServoSon:1;              //内部使能标志
    Uint32    ModSwitchPeriod:1;            //模式切换过度阶段标志位
    Uint32    BrkOut:1;                     //泄放输出信号
    Uint32    UdcOk:1;                      //母线电压Ok

    Uint32    OpenFSAEn:1;                  //开环扫频使能信号

    Uint32    Rsvd:11;                      //Rsvd
}
STR_FUNCToMTR_FLAGBUFF_16kHz;



//+++++++++MTRToFUNC_InitList(初始化)共用体类型定义+++++++++++++++++++//
typedef struct{
    int32   IqRate_MT;              //当电机相有效值电流为电机额定电流时对应IqRef的（即IqRate_MT）输入值 
                                    //IqRate_MT变量只能在接口文件中使用，FUNC模块其它文件额定电流对应1000数字量 与MTR模块不同
    Uint32  EncRev;                 //电机分辨率数,FUNC_PosCtrl计算位置增量反馈时使用      
    int32   ToqCoef_Q12;            //转矩转换系数:百分数(0.1%)->内部转矩,FUNC_ToqCtrl使用
    int32   AbsPosDetection;        //绝对位置检测系统模式
}STR_FUNC_MTRTOFUNC_INITLIST;

typedef union{
    Uint32                           all[MTRTOFUNC_INITLIST_NUM];
    STR_FUNC_MTRTOFUNC_INITLIST      List;
}UNI_FUNC_MTRTOFUNC_INITLIST;  


typedef struct{
    Uint32  UVAdjustResult;         //UV相电流平衡校正结果
    int32   IqPosLmt_F;             //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的FUNC模块正向IqRef最后限制值
    int32   IqNegLmt_F;             //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的FUNC模块反向IqRef最后限制值
    int32   Vd;                     //d轴电压调制度
    int32   Vq;                     //q轴电压调制度
}STR_FUNC_MTRTOFUNC_LIST_1kHz;

typedef union{
    Uint32                           all[MTRTOFUNC_LIST_1kHz_NUM];
    STR_FUNC_MTRTOFUNC_LIST_1kHz     List;
}UNI_FUNC_MTRTOFUNC_LIST_1kHz;


//+++++++++++++++++MTRToFUNC_LIST_16kHz共用体类型定义 +++++++++++++++++++++++++++++++//

//FUNC模块接收由MTR模块传送过来的“位域标志位”变量结构体类型定义
typedef struct{ 
    Uint32    DOvarReg_Clt:1;           //bit0 转矩限制的确认信号
    Uint32    DOvarReg_Vlt:1;           //bit1 速度限制的确认信号
    Uint32    DOvarReg_ToqReach:1;      //bit2 转矩到达标志位 
    Uint32    IncEncAngInitEn:1;        //bit3 省线式编码器初始角辨识使能
    Uint32    AbsEncAngInitEn:1;        //bit4 绝对式编码器初始角辨识使能
    Uint32    STOState:1;               //bit5 STO端子状态  0正常 1保护 封锁PWM波
    Uint32    ShortGndDone:1;           //bit6 对地短路检查完标志位 0 未检测完 1检测完
    Uint32    ShortGndRunFlag:1;        //bit7 对地短路检查完标志位 0 未检测完 1检测完
    Uint32    AbsEncCommInit:1;         //bit8 编码器通信初始化完标志位     
    Uint32    VoltVectorLmt:1;          //bit8 电压矢量限制标志位
    Uint32    CurSmpMode:1;             //bit9 电流环采样模式标志位 
    Uint32    Rsvd:21;                  //
}STR_MTRTOFUNC_STATUSFLAG_16kHz;

typedef union{
    volatile Uint32                              all;
    volatile STR_MTRTOFUNC_STATUSFLAG_16kHz      bit;
}UNI_MTRTOFUNC_STATUSFLAG_16kHz;


typedef struct{  
    Uint32  MechaAngle;        //机械角度（从原点开始的脉冲数）由FPGA输出给 H0B09 显示用
    int32   IqRef;              //Q轴电流调节器参考输入
    int32   SpdFdb;             //速度反馈值,FUNC_SpdCtrl使用
    int32   IqFdb;              //Q轴电流调节器反馈
    int32   IdFdb;              //D轴电流调节器反馈
    int32   IuFdb;              //U相电流值
    int32   IvFdb;              //V相电流值
    int32   IwFdb;              //W相电流值

    int32   PosFdbAbsValue;     //当前位置反馈绝对值，在软中断程序位置模块多次读取，需要在软中断中做接口
    int32   PulsePosRef;        //绝对位置指令，在软中断程序位置模块多次读取，需要在软中断中做接口 
    int32   PulsePosRef2;         //位置脉冲指令给定2 高速
             
    int32   Etheta;             //当前电机电角度值(0-32768) 由FPGA输出给 H0B10 显示用

    int32   CH1_Test;           //电机模块示波器测试变量1
    int32   CH2_Test;           //电机模块示波器测试变量2
    int32   CH3_Test;           //电机模块示波器测试变量3
    int32   CH4_Test;           //电机模块示波器测试变量4

    UNI_MTRTOFUNC_STATUSFLAG_16kHz        StatusFlag;      //FUNC模块接收由MTR传递过来的位域状态标志位变量

	int32  ExPosFdbAbsValue;   //全闭环位置反馈

    Uint32  MultiAbsPosFdbL;     //多圈绝对式编码器位置反馈
    Uint32  MultiAbsPosFdbH;     //多圈绝对式编码器位置反馈

    Uint32  SingleAbsPosFdb;     //编码器单圈位置反馈

    Uint32  FpgaPosFdbInit;          //FPGA位置反馈初值

    int32   HighPrecisionAI;       //高精度AI          
}STR_FUNC_MTRTOFUNC_LIST_16kHz;

typedef union{
    Uint32                            all[MTRTOFUNC_LIST_16kHz_NUM];
    STR_FUNC_MTRTOFUNC_LIST_16kHz     List;
}UNI_FUNC_MTRTOFUNC_LIST_16kHz;



//FUNC模块接收由MTR传递过来的扫频状态变量
typedef struct{  
    int32   FSA_SpdReguOutPlusChirp;     //电流环前端信号
    int32   SpdReguOut;        //速度调节器输出信号 
}STR_FUNC_MTRTOFUNC_FSALIST_16kHz;

typedef union{
    Uint32                            all[2];
    STR_FUNC_MTRTOFUNC_FSALIST_16kHz     List;
}UNI_FUNC_MTRTOFUNC_FSALIST_16kHz;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern UNI_FUNC_MTRTOFUNC_INITLIST      UNI_FUNC_MTRToFUNC_InitList;
extern UNI_FUNC_MTRTOFUNC_LIST_1kHz     UNI_FUNC_MTRToFUNC_SlowList_1kHz;
extern UNI_FUNC_MTRTOFUNC_LIST_16kHz    UNI_FUNC_MTRToFUNC_FastList_16kHz;
extern UNI_FUNC_MTRTOFUNC_FSALIST_16kHz    UNI_FUNC_MTRToFUNC_FSAFastList_16kHz;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
extern void FUNC_Interrupt16kHz_InterfaceDeal(void);


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_INTERFACE_PROCESS_H */

/********************************* END OF FILE *********************************/
