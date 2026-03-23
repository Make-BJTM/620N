/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:                                                           
 创建人:    王治国              创建日期: 2011.10.31
 描述:
    1. 辅助功能码机构体定义
    2. 辅助功能码不存储在Eeprom中
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_AUXFUNCODE_H
#define __FUNC_AUXFUNCODE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/
//每组功能码长度
#define H0BLEN            100
#define H0DLEN            25
#define H2FLEN            70
#define H30LEN            10
#define H31LEN            30
#define H32LEN            24

#define AUXFUNGROUP_STARTINDEX      0x2F
#define AUXFUNGROUP_ENDINDEX        0x32

#define AUXFUNCODELEN   (H0BLEN + H0DLEN + H2FLEN + H30LEN + H31LEN + H32LEN)

//面板显示每组功能码长度
#define      H0B_PANELDISPLEN       99
#define      H0D_PANELDISPLEN       23
#define      H2F_PANELDISPLEN       70
#define      H30_PANELDISPLEN       10
#define      H31_PANELDISPLEN       30
#define      H32_PANELDISPLEN       24

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 

typedef struct{
    Uint16 H0B[H0BLEN];
    Uint16 H0D[H0DLEN];
    Uint16 H2F[H2FLEN];
    Uint16 H30[H30LEN];
    Uint16 H31[H31LEN];
    Uint16 H32[H32LEN];
} AUXFUNCODEGROUP;

typedef struct{
//--------------H0B组显示  H0BLEN = 43 ------------------------------------------------------- 
    Uint16 DP_MotorSpd;          //H0B_00  电机转速              
    Uint16 DP_SpdRef;            //H0B_01  速度指令
    Uint16 DP_ToqRef;            //H0B_02  内部转矩指令（相对于  转矩的值，归一化处理）
    Uint16 DP_DIState;           //H0B_03  输入信号监视DI
    Uint16 H0B_04_Rsvd;          //H0B_04  预留
    Uint16 DP_DOState;           //H0B_05  输出信号监视DO
    Uint16 H0B_06_Rsvd;          //H0B_06  预留
    Uint16 DP_AbsPosCnt_Lo;      //H0B_07  绝对位置计数器（32位十进制显示）低位
    Uint16 DP_AbsPosCnt_Hi;      //H0B_08  绝对位置计数器（32位十进制显示）高位
    Uint16 DP_MechaAngle;        //H0B_09  机械角度
    Uint16 DP_EleTheta;          //H0B_10  电气角度
    Uint16 DP_SpdRef_Puse;       //H0B_11  输入指令脉冲速度（仅脉冲位置控制有效）
    Uint16 DP_AvrLoad;           //H0B_12  平均负载率%
    Uint16 DP_InPulseCnt_Lo;     //H0B_13  输入指令脉冲计数器（32位十进制显示）低位
    Uint16 DP_InPulseCnt_Hi;     //H0B_14  输入指令脉冲计数器（32位十进制显示）高位
    Uint16 DP_PosErrCnt_Lo;      //H0B_15  偏差计数器（位置偏差量）低位
    Uint16 DP_PosErrCnt_Hi;      //H0B_16  偏差计数器（位置偏差量）高位  偏差计数器32bit(用于17bit或20bit编码器)
    Uint16 DP_FbPuleCnt_Lo;      //H0B_17  反馈脉冲计数器（32位十进制显示）低位
    Uint16 DP_FbPuleCnt_Hi;      //H0B_18  反馈脉冲计数器（32位十进制显示）高位
    Uint16 DP_ServoTime_Lo;      //H0B_19  总运行时间（32位10进制显示） 低位
    Uint16 DP_ServoTime_Hi;      //H0B_20  总运行时间（32位10进制显示） 高位
    Uint16 DP_Ai1SampVolt;       //H0B_21  AI1采样电压 采样值(有效值)
    Uint16 DP_Ai2SampVolt;       //H0B_22  AI2采样电压 采样值(有效值)
    Uint16 DP_Ai3SampVolt;       //H0B_23  AI3采样电压 采样值(有效值)
    Uint16 DP_USampCur;          //H0B_24  U电流采样值(有效值)
    Uint16 DP_IdenTheta;         //H0B_25  电压注入辨识角度
    Uint16 DP_Udc;               //H0B_26  母线电压
    Uint16 DP_DriverTemp;        //H0B_27  驱动器温度
    Uint16 DP_FpgaAbsRomErr;     //H0B_28  FPGA给出绝对编码器故障信息
    Uint16 DP_FpgaSysState;      //H0B_29  FPGA给出的系统状态信息
    Uint16 DP_FpgaSysErr;        //H0B_30  FPGA给出的系统故障信息
    Uint16 DP_EncStBit1;         //H0B_31  20位编码器状态位1
    Uint16 DP_EncRsvd;           //H0B_32  编码器状态保留
    Uint16 DP_DefotNo;           //H0B_33  故障记录的显示  0 --> 当前故障
    Uint16 DP_ErrCode;           //H0B_34  故障码
    Uint16 DP_DefotTime_Lo;      //H0B_35  所故障时间戳  低位
    Uint16 DP_DefotTime_Hi;      //H0B_36  所选故障时间戳  高位
    Uint16 DP_Spd_Defot;         //H0B_36  所选故障时转速
    Uint16 DP_CurtU_Defot;       //H0B_38  所选故障时U相电流
    Uint16 DP_CurtV_Defot;       //H0B_39  所选故障时V相电流
    Uint16 DP_Udc_Defot;         //H0B_40  所选故障时母线电压
    Uint16 DP_DI_Defot;          //H0B_41  所选故障时输入端子状态
    Uint16 DP_DO_Defot;          //H0B_42  所选故障时输出端子状态
    Uint16 DP_ParaErrGroup;      //H0B_43  参数异常的功能码组号
    Uint16 DP_ParaErrOffset;     //H0B_44  参数异常的功能码组内偏置
    Uint16 DP_InnerErrCode;      //H0B_45  内部故障码
    Uint16 DP_FpgaAbsRomErr_Defot;  //H0B_46  所选故障时FPGA给出绝对编码器故障信息
    Uint16 DP_FpgaSysState_Defot;   //H0B_47  所选故障时FPGA给出的系统状态信息
    Uint16 DP_FpgaSysErr_Defot;  //H0B_48  所选故障时FPGA给出的系统故障信息
    Uint16 DP_EncStBit1_Defot;   //H0B_49  所选故障时20位编码器状态位1
    Uint16 DP_EncRsvd_Defot;     //H0B_50  所选故障时编码器状态保留
    Uint16 DP_InnerErrCode_Defot;   //H0B_51  所选故障时内部故障码
    Uint16 DP_FpgaTmFlt_Defot;   //H0B_52  所选故障时故障时FPGA超时故障标准位
    Uint16 DP_PosErrCntUutL;     //H0B_53  随动偏差低位(指令单位)
    Uint16 DP_PosErrCntUutH;     //H0B_54  随动偏差高位(指令单位)
    Uint16 DP_MotorSpdL;         //H0B_55  电机转速0.1rpm
    Uint16 DP_MotorSpdH;         //H0B_56  电机转速0.1rpm
    Uint16 DP_CtrlUdc;           //H0B_57  控制电母线电压
    Uint16 DP_MechanicalAbsPosL;    //H0B_58  机械绝对位置 低32位（编码器单位）
    Uint16 DP_MechanicalAbsPosH;    //H0B_59  机械绝对位置 低32位（编码器单位）
    Uint16 DP_MechanicalAbsPosH32L; //H0B_60  机械绝对位置 高32位（编码器单位）
    Uint16 DP_MechanicalAbsPosH32H; //H0B_61  机械绝对位置 高32位（编码器单位）
    Uint16 DP_LineVolt;            //H0B_62   驱动器输出线电压有效值
    Uint16 DP_Rsvd63;            //H0B_63  
    Uint16 DP_InputPulseCntRTL;  //H0B_64  实时脉冲输入低16位
    Uint16 DP_InputPulseCntRTH;  //H0B_65  实时脉冲输入高16位
    Uint16 DP_Rsvd66;            //H0B_66
    Uint16 DP_Rsvd67;            //H0B_67 
    Uint16 DP_FpgaTmFlt;         //H0B_68 FPGA超时故障标准位
    Uint16 DP_ABSEncErr;         //H0B_69 海德汉编码器故障
    Uint16 DP_AbsExtData;        //H0B_70 串行编码器扩展数据/多圈数据
    Uint16 DP_ABSEncSFdbL;       //H0B_71 串行编码器反馈单圈位置
    Uint16 DP_ABSEncSFdbH;       //H0B_72 串行编码器反馈单圈位置
    Uint16 DP_Rsvd73;            //H0B_73
    Uint16 DP_Rsvd74;            //H0B_74
    Uint16 DP_JRatioOnline;      //H0B_75 在线惯量辨识时惯量比
    Uint16 DP_TLOnline;          //H0B_76 在线惯量辨识时外部负载转矩
    Uint16 DP_EncPosL;           //H0B_77 编码器位置L
    Uint16 DP_EncPosH;           //H0B_78 编码器位置H
    Uint16 DP_EncPosH32L;        //H0B_79 编码器位置高32位 L   
    Uint16 DP_EncPosH32H;        //H0B_80 编码器位置高32位 H
    Uint16 DP_MechSingleAbsPosL;    //H0B_81  旋转负载单圈位置 低32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH;    //H0B_82  旋转负载单圈位置 低32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH32L; //H0B_83  旋转负载单圈位置 高32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH32H; //H0B_84  旋转负载单圈位置 高32位（编码器单位）
    Uint16 DP_MechSAbsPos_DemandUintL;    //H0B_85  旋转负载单圈位置（指令单位）
    Uint16 DP_MechSAbsPos_DemandUintH;    //H0B_86  旋转负载单圈位置（指令单位）
    Uint16 DP_Rsvd87;            //H0B_87
    Uint16 DP_Rsvd88;            //H0B_88
    Uint16 DP_Rsvd89;            //H0B_89
    Uint16 DP_Rsvd90;            //H0B_90
    Uint16 DP_Rsvd91;            //H0B_91
    Uint16 DP_Rsvd92;            //H0B_92
    Uint16 DP_Rsvd93;            //H0B_93
    Uint16 DP_MainLoop_PSTMax;   //H0B_94 4秒内主循环调度时间最大值        内部参数后期考虑删除
    Uint16 DP_MainLoop_PRTMax;   //H0B_95 4秒内主循环运行时间最大值        内部参数后期考虑删除
    Uint16 ToqInt_PRTime;        //H0B_96 4秒内电流环中断运行时间最大值    内部参数后期考虑删除
    Uint16 PosInt_PRTime;        //H0B_97 4秒内位置环中断运行时间最大值    内部参数后期考虑删除
    Uint16 DP_ZPosErrMax;        //H0B_98 增量编码器Z位置偏差最值          内部参数后期考虑删除
    Uint16 DP_ZPosErr;           //H0B_99 增量编码器Z位置偏差              内部参数后期考虑删除

//--------------辅助功能  H0DLEN = 25 ------------------------------------------------------- 
    Uint16 FA_SoftRst;                  //H0D_00 软件复位操作
    Uint16 FA_DefaultRst;               //H0D_01 故障复位操作
    Uint16 FA_OffLnInrtMod;             //H0D_02 惯量辨识使能
    Uint16 OEM_ResetABSTheta;           //H0D_03 绝对编码器初始角辨识
    Uint16 MT_OperAbsROM;               //H0D_04 编码器ROM区读写
    Uint16 FA_EmergencyStop;            //H0D_05 紧急停车(仿紧急停止按钮功能)
    Uint16 FA_TorqPiTune;               //H0D_06 转矩PI自调谐
    Uint16 FA_FricIdentify;             //H0D_07 库仑摩擦力辨识
    Uint16 FA_Rsvd08;                   //H0D_08 
    Uint16 FA_AutoTune;                 //H0D_09 一键式自调整使能开关
    Uint16 FA_AIAdjustMode;             //H0D_10 AI自动零点偏移调整
    Uint16 FA_Jog;                      //H0D_11 JOG使能
    Uint16 FA_UVAdjustRatio;            //H0D_12 UV相电流平衡校正
    Uint16 FA_RiseTime1;                //H0D_13 转矩PI自调谐,阶跃响应上升时间   单位：1个电流环周期
    Uint16 FA_StableError;              //H0D_14 转矩PI自调谐,阶跃响应稳态误差  0.1%
    Uint16 FA_OverShot;                 //H0D_15 转矩PI自调谐,阶跃响应超调量  0.1%
    Uint16 FA_RiseTime2;                //H0D_16 转矩PI自调谐,阶跃响应上升时间   单位：1个电流环周期
    Uint16 FA_DiDoSimulateEnable;       //H0D_17 DIDO仿真模式下强行输出开关
    Uint16 FA_DiSimulateSet;            //H0D_18 DI仿真模式下强行输出给定
    Uint16 FA_DoSimulateSet;            //H0D_19 Do仿真模式下强行输出给定
    Uint16 FA_AbsEncRst;                //H0D_20 绝对编码器复位操作
    Uint16 FA_NK_HDH_AbsEncRdErrStatus;      //H0D_21 读尼康海德汉编码器错误状态位
    Uint16 FA_DisTest;                  //H0D_22 显示测试
    Uint16 FA_Rsvd23;                   //H0D_23
    Uint16 FA_Rsvd24;                   //H0D_24

//--------------易用性参数组   H2FLEN = 70 -------------------------------------------------------
    Uint16 GUIWorkMode;                 // H2F_00 后台工作模式  00:正常模式，51：后台工作模式
    Uint16 GUIModeSelet;                // H2F_01 后台模式选择
    Uint16 GUICmdSource;                // H2F_02 后台指令来源  (删掉)
    Uint16 H2F_Rsvd03;                  // H2F_03 保留
    Uint16 H2F_Rsvd04;                  // H2F_04 保留
    Uint16 H2F_AutoTuneStats;           // H2F_05 自调整状态
    Uint16 H2F_Rsvd06;                  // H2F_06 保留
    Uint16  H2F_Rsvd07;                 // H2F_07
    Uint16 CurveServoON;                // H2F_08 内部使能
    Uint16 JogSpeedRef;                 // H2F_09 JOG运行速度
    Uint16 JogRiseDownTime;             // H2F_10 JOG运行加减速
    Uint16 JogDir;                      // H2F_11 JOG定位运行方向
    Uint16 CurveSel;                    // H2F_12 定位试运行选择 0：JOG，1：定位试运行
    Uint16 CurvePNlimitSet;             // H2F_13 0:不设定，1：极限位置设定
    Uint16 CurveCoordCCWLow;            // H2F_14 定位坐标1低位
    Uint16 CurveCoordCCWHigh;           // H2F_15 定位坐标2高位
    Uint16 CurveCoordCWLow;             // H2F_16 定位坐标1低位
    Uint16 CurveCoordCWHigh;            // H2F_17 定位坐标2高位
    Uint16 CurvePulseSendLow;	        // H2F_18 已发送脉冲数低位
    Uint16 CurvePulseSendHigh;		    // H2F_19 已发送脉冲数高位
    Uint16 CurveMoveDisSetLow;          // H2F_20 运行距离设定低位
    Uint16 CurveMoveDisSetHigh;         // H2F_21 运行距离设定高位
    Uint16 CurveRunMode;                // H2F_22 运行模式 0:单次，1：连续
    Uint16 CurveRunDirSet;              // H2F_23 运行方向设定 0：不运行，1：正转运行，2：反转运行 3：暂停
    Uint16 WaitTime;                    // H2F_24 等待时间
    Uint16 CurvePertSpeedMax;           // H2F_25 允许运行最大速度
    Uint16 CurvePertLoadRatio;          // H2F_26 允许负载率
    Uint16 CurvePertCycle;              // H2F_27 允许最大圈数
    Uint16 CurveWarne;                  // H2F_28 警告 
    Uint16 CurveClrPulseSend;           // H2F_29 清除发送脉冲
    Uint16 CurveMoveSpeedMax;           // H2F_30 定位运行最高速度
    Uint16 CurveMoveRiseDownTime;       // H2F_31 定位运行加减速
    Uint16 H2F_LctCondition;            // H2F_32 0~没有约束，1~不允许超调，2~不允许定位范围抖动，3~超调或抖动都不允许
    Uint16 H2F_Rsvd33;                  // H2F_33
    Uint16 GR_DataReady;                // H2F_34 数据已准备好次数
    Uint16 H2F_Rsvd35;                  // H2F_35 
    Uint16 GR_PosLagTime;               // H2F_36 定位时间
    Uint16 GR_oscillateLevel;           // H2F_37 振动等级
    Uint16 GR_LoadRatioAv;              // H2F_38 平均负载率
    Uint16 GR_OvershootPulse;           // H2F_39 过冲脉冲数
    Uint16  H2F_Rsvd40;                 // H2F_40 
    Uint16  H2F_Rsvd41;                 // H2F_41
    Uint16  H2F_Rsvd42;                 // H2F_42
    Uint16  H2F_Rsvd43;                 // H2F_43
    Uint16  ResZeroIndexEn;             // H2F_44 //寻找Z信号速度模式使能
    Uint16  H2F_Rsvd45;                 // H2F_45 
    Uint16  H2F_Rsvd46;                 // H2F_46
    Uint16  H2F_Rsvd47;                 // H2F_47
    Uint16  RatioNow;                   // H2F_48 惯量辨识结果
    Uint16  OnLineInertiaRatio;         // H2F_49 离线惯量辨识结果
    Uint16  ServoRunState;              // H2F_50 伺服运行状态 (删掉 )
    Uint16  FS_SpdLoopFreq;             // H2F_51 速度环调度频率, 将是频率特性分析速度反馈的采样频率
    Uint16  FS_SpdCmdAmpltd;            // H2F_52 速度指令振幅值
    Uint16  FS_SpdCmdOffset;            // H2F_53 速度指令偏置值
    Uint16  FS_AnalyzeStart;            // H2F_54 开始发送频率特性速度指令
    Uint16  FS_EnableSend;              // H2F_55 第3位为1表示速度反馈数据无效, 低两位为11表示允许发送速度反馈数据 	
    Uint16  FS_Mode;                     // H2F_56 扫频模式选择
    Uint16  FS_IqTurbAmpltd;            // H2F_57 开环扫频电流激励幅值
    Uint16  FS_SpdRefAmpltd;            // H2F_58 开环扫频速度指令幅值
    Uint16  FS_TorqSat;                 // H2F_59 报警标识
    Uint16  H2F_FuncSwitch;             // H2F_60 新旧功能切换选择 bit0:1~新扫频指令，0~旧扫频指令 
                                        //        bit1:1~后台故障显示读H2F61  0~后台故障显示读H0B30 
    Uint16  H2F_HighLvlErrCode;         // H2F_61 最高级故障
    Uint16  H2F_FSAState;               // H2F_62 开环扫频状态标志
    Uint16  H2F_Rsvd63;                 // H2F_63
    Uint16  H2F_Rsvd64;                 // H2F_64
    Uint16  H2F_Rsvd65;                 // H2F_65
    Uint16  H2F_Rsvd66;                 // H2F_66
    Uint16  H2F_Rsvd67;                 // H2F_67
    Uint16  H2F_Rsvd68;                 // H2F_68
    Uint16  H2F_Rsvd69;                 // H2F_69

//--------------H30 Comm Read Information   H30LEN = 10 -------------------------------------------------------
    Uint16 CR_SVST;              //H30_00 通信读取伺服状态
    Uint16 CR_FunOut;            //H30_01 通信读取FunOut Bit0-Bit15
    Uint16 CR_FunOutH;           //H30_02 通信读取FunOut Bit16-Bit31
    Uint16 CR_DeltaPulse;        //H30_03 通信读取输入脉冲指令采样值
    Uint16 H3004_Rsvd;           //H30_04 预留
    Uint16 H3005_Rsvd;           //H30_05 预留
    Uint16 H3006_Rsvd;           //H30_06 预留
    Uint16 H3007_Rsvd;           //H30_07 预留
    Uint16 H3008_Rsvd;           //H30_08 预留
    Uint16 H3009_Rsvd;           //H30_09 预留

//--------------H31 通信实现运动控制   H31LEN = 30 -------------------------------------------------------
     Uint16 CC_VDILevel;         //H31_00  VDI虚拟电平
     Uint16 H3101_Rsvd;          //H31_01  预留
     Uint16 H3102_Rsvd;          //H31_02  预留
     Uint16 H3103_Rsvd;          //H31_03  预留
     Uint16 CC_CommCtrlDO;       //H31_04  通信给定DO输出状态
     Uint16 H3105_Rsvd;          //H31_05  通信给定AO1输出电压
     Uint16 H3106_Rsvd;          //H31_06  通信给定AO2输出电压
     Uint16 H3107_Rsvd;          //H31_07  预留
     Uint16 H3108_Rsvd;          //H31_08  预留
     Uint16 CC_CommSendSpeedL;   //H31_09  通信给定速度L
     Uint16 CC_CommSendSpeedH;   //H31_10  通信给定速度H
     Uint16 CC_CommSendTorqL;    //H31_11  转矩指令L
     Uint16 CC_CommSendTorqH;    //H31_12  转矩指令H
     Uint16 CC_MoniterRT;        //H31_13，监测CAN发送位置处理实时
     Uint16 CC_MoniterRT2;       //H31_14，监测CAN发送位置次数
     Uint16 H3115_Rsvd;          //H31_15  预留
     Uint16 H3116_Rsvd;          //H31_16  预留
     Uint16 H3117_Rsvd;          //H31_17  预留
     Uint16 H3118_Rsvd;          //H31_18  预留
     Uint16 H3119_Rsvd;          //H31_19  预留
     Uint16 H3120_Rsvd;          //H31_20  预留
     Uint16 H3121_Rsvd;          //H31_21  预留
     Uint16 H3122_Rsvd;          //H31_22  预留
     Uint16 H3123_Rsvd;          //H31_23  预留
     Uint16 H3124_Rsvd;          //H31_24  预留
     Uint16 H3125_Rsvd;          //H31_25  预留
     Uint16 H3126_Rsvd;          //H31_26  预留
     Uint16 H3127_Rsvd;          //H31_27  预留
     Uint16 H3128_Rsvd;          //H31_28  预留
     Uint16 H3129_Rsvd;          //H31_29  预留

//--------------H32 示波器   H31LEN = 24 -------------------------------------------------------
     Uint16 OS_UartStatus;          //H32_00  串口状态
     Uint16 OS_SeriesTxCtrl;        //H32_01  连续发送数据控制
     Uint16 OS_SectionCtrlA;        //H32_02  示波器缓冲区0-15扇区读写控制 
     Uint16 OS_SectionCtrlB;        //H32_03  示波器缓冲区16-31扇区读写控制
     Uint16 OS_SectionCtrlC;        //H32_04  示波器缓冲区32-47扇区读写控制
     Uint16 OS_SectionCtrlD;        //H32_05  示波器缓冲区48-63扇区读写控制
     Uint16 H3206_Rsvd;             //H32_06  预留
     Uint16 OS_Part;                //H32_07  多次采样当前段数
     Uint16 OS_SampleLen;           //H32_08  后台扫频采样数据长度
     Uint16 OS_SampleClk;           //H32_09  示波器采样时钟
     Uint16 OS_Mode;                //H32_10  示波器模式
     Uint16 OS_SampleClkPrescaler;  //H32_11  示波器采样时钟分频系数
     Uint16 OS_CHSel12;             //H32_12  示波器1/2通道变量选择
     Uint16 OS_CHSel34;             //H32_13  示波器3/4通道变量选择
     Uint16 OS_TrigEdge_A;          //H32_14  示波器触发沿设置A
     Uint16 OS_TrigLevel_A;         //H32_15  示波器触发水平A
     Uint16 OS_TrigEdge_B;          //H32_16  示波器触发沿设置B
     Uint16 OS_TrigLevel_B;         //H32_17  示波器触发水平B
     Uint16 OS_TrigSetup;           //H32_18  示波器触发设定
     Uint16 OS_TrigPosSetup;        //H32_19  示波器触发位置设定
     Uint16 OS_SampleEnable;        //H32_20  示波器采样使能
     Uint16 OS_SampleStatus;        //H32_21  示波器采样状态
     Uint16 OS_TrigPosOfBuff;       //H32_22  示波器缓冲区的触发位置
     Uint16 OS_CommDicnctStop;      //H32_23  Can通信中断，紧急停机 ，等效于H0D05
} AUXFUNCODE;

typedef union{
    Uint16 all[AUXFUNCODELEN];
    AUXFUNCODE          code;
    AUXFUNCODEGROUP     group;
}AUXFUNCODETABLE;

//电子标签结构体 Electronic Label
typedef struct{
     Uint16 JumpFlag;           //标签00 内部使用 
     Uint16 DelayCnt;           //标签01 内部使用 
     Uint16 Label02;            //标签02
     Uint16 Label03;            //标签03
     Uint16 ServoStatus;        //伺服状态
     Uint16 Label05;            //标签05
     Uint16 Label06;            //标签06
     Uint16 Label07;            //标签07
     Uint16 Label08;            //标签08
     Uint16 Label09;            //标签09
     Uint16 Label10;            //标签10
     Uint16 Label11;            //标签11
     Uint16 Label12;            //标签12
     Uint16 Label13;            //标签13
     Uint16 Label14;            //标签14
     Uint16 Label15;            //标签15
     Uint16 Manufacturer;       //厂商
     Uint16 ProductLine;        //产品线
     Uint16 Model;              //产品型号
     Uint16 ProductVer;         //产品版本
     Uint16 CanVer;             //通信版本
     Uint16 BootVer;            //BOOT版本
     Uint16 ChipModel;          //芯片型号
     Uint16 NonstandardVer;     //非标号
     Uint16 Label24;            //标签24
     Uint16 Label25;            //标签25
     Uint16 Label26;            //标签26
     Uint16 Label27;            //标签27
     Uint16 Label28;            //标签28
     Uint16 Label29;            //标签29
     Uint16 Label30;            //标签30
     Uint16 Label31;            //标签31
}STR_ELECTRONIC_LABEL;

typedef union{
    Uint16                  all[32];
    STR_ELECTRONIC_LABEL    Label;
}UNI_ELECTRONIC_LABEL; 

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern AUXFUNCODETABLE  AuxFunCodeUnion;
extern UNI_ELECTRONIC_LABEL  UNI_EleLabel;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_COMMINTERFACE_H */

/********************************* END OF FILE *********************************/
