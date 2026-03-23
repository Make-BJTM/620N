/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_FunCode.h
 创建人：李浩                创建日期：11.10.09 
 描述：
    1.功能码类型的定义
    2.ERRMSGREC类型的定义
            
 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx
*******************************************************************************/

#ifndef FUNC_FUNCODE_H
#define FUNC_FUNCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"  
#include "FUNC_AuxFunCode.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* 功能码结构体长度 
 * 当前所选的Eeprom型号AT24C32C,容量32Kbits(4096*8bytes)
 * 可以存储2048个功能码,由于Eeprom前两页(32bytes)空出,
 * 故功能码结构体总的长度为2032
 * 下面各组功能码的长度之和应等于2040
 */

//厂家密码
#define OEMPASSWORD      1430

//用户可看到的H00 H01长度
#define      H00_PANELDISPLEN_OEM       9
#define      H01_PANELDISPLEN_OEM       3


//各组功能码的长度     各组功能码的长度之和应等于2032
#define     EEPROM_CHECK_NUM        16
#define     H00LEN          60
#define     H01LEN          70
#define     H02LEN          50
#define     H03LEN          100
#define     H04LEN          70
#define     H05LEN          90
#define     H06LEN          50
#define     H07LEN          70
#define     H08LEN          90
#define     H09LEN          60
#define     H0ALEN          40
//H0B组不用存储在Eeprom中,在辅助功能码中定义
#define     H0CLEN          50
//H0D组不用存储在Eeprom中,在辅助功能码中定义
#define     H0ELEN          50
#define     H0FLEN          50
#define     H10LEN          50
#define     H11LEN          100
#define     H12LEN          70
#define     H13LEN          50
#define     H14LEN          50
#define     H15LEN          50
#define     H16LEN          50
#define     H17LEN          70
#define     H18LEN          50
#define     H19LEN          50
#define     H1ALEN          50
#define     H1BLEN          50
#define     H1CLEN          50
#define     H1DLEN          50
#define     HRsvdLEN        102
#define     HEL_LEN         16
#define     ErrSvLEN        176
#define     OffSvLEN        16   //长度不能改变
#define     IAPLEN          16    //长度不能改变
//末尾功能码组序号
#define FUNGROUP_ENDINDEX       0x1D

//功能码结构体中,每组首个功能码的序号
#define      H0000INDEX        EEPROM_CHECK_NUM 
#define      H0100INDEX       (H0000INDEX + H00LEN)
#define      H0200INDEX       (H0100INDEX + H01LEN)
#define      H0300INDEX       (H0200INDEX + H02LEN)
#define      H0400INDEX       (H0300INDEX + H03LEN)
#define      H0500INDEX       (H0400INDEX + H04LEN)
#define      H0600INDEX       (H0500INDEX + H05LEN)
#define      H0700INDEX       (H0600INDEX + H06LEN)
#define      H0800INDEX       (H0700INDEX + H07LEN)
#define      H0900INDEX       (H0800INDEX + H08LEN)
#define      H0A00INDEX       (H0900INDEX + H09LEN)
//H0B组不用存储在Eeprom中,在辅助功能码中定义
#define      H0C00INDEX       (H0A00INDEX + H0ALEN)
//H0D组不用存储在Eeprom中,在辅助功能码中定义
#define      H0E00INDEX       (H0C00INDEX + H0CLEN)
#define      H0F00INDEX       (H0E00INDEX + H0ELEN)
#define      H1000INDEX       (H0F00INDEX + H0FLEN)
#define      H1100INDEX       (H1000INDEX + H10LEN)
#define      H1200INDEX       (H1100INDEX + H11LEN)
#define      H1300INDEX       (H1200INDEX + H12LEN)
#define      H1400INDEX       (H1300INDEX + H13LEN)
#define      H1500INDEX       (H1400INDEX + H14LEN)
#define      H1600INDEX       (H1500INDEX + H15LEN)
#define      H1700INDEX       (H1600INDEX + H16LEN)
#define      H1800INDEX       (H1700INDEX + H17LEN)
#define      H1900INDEX       (H1800INDEX + H18LEN)
#define      H1A00INDEX       (H1900INDEX + H19LEN)
#define      H1B00INDEX       (H1A00INDEX + H1ALEN)
#define      H1C00INDEX       (H1B00INDEX + H1BLEN)
#define      H1D00INDEX       (H1C00INDEX + H1CLEN)
#define      HRsvd00INDEX     (H1D00INDEX + H1DLEN)
#define      HEL00INDEX       (HRsvd00INDEX + HRsvdLEN)
#define      HErrSvINDEX      (HEL00INDEX + HEL_LEN)
#define      HOffSv00INDEX    (HErrSvINDEX + ErrSvLEN)
#define      HIAP00INDEX      (HOffSv00INDEX + OffSvLEN)

//面板显示每组功能码长度
#define      H00_PANELDISPLEN       38
#define      H01_PANELDISPLEN       69
#define      H02_PANELDISPLEN       42
#define      H03_PANELDISPLEN       36
#define      H04_PANELDISPLEN       56
#define      H05_PANELDISPLEN       62
#define      H06_PANELDISPLEN       20
#define      H07_PANELDISPLEN       41
#define      H08_PANELDISPLEN       31
#define      H09_PANELDISPLEN       47
#define      H0A_PANELDISPLEN       39
#define      H0C_PANELDISPLEN       49
#define      H0E_PANELDISPLEN       0
#define      H0F_PANELDISPLEN       36
#define      H10_PANELDISPLEN       0
#define      H11_PANELDISPLEN       92
#define      H12_PANELDISPLEN       68
#define      H13_PANELDISPLEN       0
#define      H14_PANELDISPLEN       0
#define      H15_PANELDISPLEN       0
#define      H16_PANELDISPLEN       0
#define      H17_PANELDISPLEN       65
#define      H18_PANELDISPLEN       0
#define      H19_PANELDISPLEN       0
#define      H1A_PANELDISPLEN       0
#define      H1B_PANELDISPLEN       0
#define      H1C_PANELDISPLEN       48
#define      H1D_PANELDISPLEN       49

//Di功能选择最大值
#define  DI_FUNC_SEL_MAX    39
//Do功能选择最大值
#define  DO_FUNC_SEL_MAX    20    

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */
//获取功能码的序号
#define     GetCodeIndex(code)          (Uint16)((&code) - (&FunCodeUnion.all[0]))


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

/*故障信息记录结构体定义*/
//STR_ERR_MSG_RECORD结构体成员可以再增加4个
typedef struct{
    Uint16   ErrCode;       //故障码
    Uint16   TimeStampL;    //故障时间戳低16位
    Uint16   TimeStampH;    //故障时间戳高16位
    Uint16   Spd;           //故障时的转速
    int16    Iu;            //故障时的u相瞬时电流
    int16    Iv;            //故障时的v相瞬时电流
    Uint16   Udc;           //故障时的母线电压
    Uint16   DiState;       //故障时的DI端子状态
    Uint16   DoState;       //故障时的DO端子状态 
    Uint16   FpgaAbsRomErr;     //故障时FPGA给出绝对编码器故障信息
    Uint16   FpgaSysState;      //故障时FPGA给出的系统状态信息
    Uint16   FpgaSysErr;        //故障时FPGA给出的系统状态信息
    Uint16   EncStBit1;         //故障时20位编码器状态位
    Uint16   EncStBit2;         //故障时20位编码器状态位  
    Uint16   InnerErrCode;      //故障时内部故障码
    Uint16   FpgaTmFlt;         //故障时FPGA超时故障标准位
}STR_ERR_MSG_RECORD;


/*功能码组结构类型定义*/
typedef struct{
    Uint16 EepromCheckWord[EEPROM_CHECK_NUM];
    Uint16 H00[H00LEN];
    Uint16 H01[H01LEN];
    Uint16 H02[H02LEN];
    Uint16 H03[H03LEN];
    Uint16 H04[H04LEN];
    Uint16 H05[H05LEN];
    Uint16 H06[H06LEN];
    Uint16 H07[H07LEN];
    Uint16 H08[H08LEN];
    Uint16 H09[H09LEN];
    Uint16 H0A[H0ALEN];
    Uint16 H0C[H0CLEN];
    Uint16 H0E[H0ELEN];
    Uint16 H0F[H0FLEN]; 
    Uint16 H10[H10LEN];
    Uint16 H11[H11LEN];
    Uint16 H12[H12LEN];
    Uint16 H13[H13LEN];
    Uint16 H14[H14LEN];
    Uint16 H15[H15LEN];
    Uint16 H16[H16LEN];
    Uint16 H17[H17LEN];
    Uint16 H18[H18LEN];
    Uint16 H19[H19LEN];
    Uint16 H1A[H1ALEN];
    Uint16 H1B[H1BLEN];
    Uint16 H1C[H1CLEN];
    Uint16 H1D[H1DLEN];
    Uint16 HRsvd[HRsvdLEN];
    Uint16 HEL[HEL_LEN];           //电子标签产品序列号存储组
    Uint16 HErrSv[ErrSvLEN];       //故障存储组 
    Uint16 HOffSv[OffSvLEN];       //掉电保存组
    Uint16 HIAP[IAPLEN];
} STR_FUNGROUP; 

/*功能码结构类型定义*/
typedef struct{
//0000------------Eeprom校验字   EEPROM_CHECK_NUM = 16 ------------------------------------------------
    Uint16  EepromCheckWord1;       //EEPROM校验字1
    Uint16  EepromCheckWord2;       //EEPROM校验字2
    Uint16  EepromCheckWord3;       //EEPROM校验字3
    Uint16  EepromCheckWord4;       //EEPROM校验字4
    Uint16  EepromCheckRsvd[12];     //保留

//0016--------------伺服电机参数  H00LEN = 60 -------------------------------------------------------
    Uint16 MT_MotorModel;           //H00_00 电机型号 用户用来选择的当前电机参数组
    //H00_01--H00_34是电机关联参数
    Uint16 MT_RsdMotorModel;        //H00_01 内部电机参数数组的唯一标识
    Uint16 MT_NonStandardVerL;      //H00_02 非标号L
    Uint16 MT_NonStandardVerH;      //H00_03 非标号H
    Uint16 MT_ABSEncVer;            //H00_04 编码器版本号
    Uint16 MT_RsdAbsRomMotorModel;  //H00_05 内部总线电机型号
    Uint16 MT_FpgaNonStandardVerL;  //H00_06 FPGA非标号L
    Uint16 MT_FpgaNonStandardVerH;  //H00_07 FPGA非标号H
    Uint16 MT_AbsEncType;           //H00_08 绝对编码器类型  14100-多圈    其它-单圈
    Uint16 MT_RateVolt;             //H00_09 额定电压
    Uint16 MT_RatePower;            //H00_10 额定功率
    Uint16 MT_RateCurrent;          //H00_11 额定电流  / 连续电流
    Uint16 MT_RateToq;              //H00_12 额定转矩  / 连续推力
    Uint16 MT_MaxToqOrCur;          //H00_13 最大转矩或最大电流
    Uint16 MT_RateSpd;              //H00_14 额定转速  / 额定速度
    Uint16 MT_MaxSpd;               //H00_15 最大转速  / 最大速度
    Uint16 MT_Inertia;              //H00_16 转动惯量  / 动子质量
    Uint16 MT_PolePair;             //H00_17 永磁同步电机极对数
    Uint16 MT_StatResist;           //H00_18 定子电阻
    Uint16 MT_StatInductQ;          //H00_19 定子电感Lq
    Uint16 MT_StatInductD;          //H00_20 定子电感Ld
    Uint16 MT_RevEleCoe;            //H00_21 反电势系数
    Uint16 MT_ToqCoe;               //H00_22 转矩系数Kt
    Uint16 MT_EleConst;             //H00_23 电气常数Te
    Uint16 MT_MachConst;            //H00_24 机械常数Tm  / 电机常数
    Uint16 MT_Rsvd0025;             //H00_25
    Uint16 MT_Rsvd0026;             //H00_26
    Uint16 MT_Rsvd0027;             //H00_27
    Uint16 MT_ThetaOffsetL;         //H00_28 绝对式码盘位置偏置L 
    Uint16 MT_ThetaOffsetH;         //H00_29 绝对式码盘位置偏置H
    Uint16 MT_EncoderSel;           //H00_30 编码器选择
    Uint16 MT_EncoderPensL;         //H00_31 编码器线数L   / 直线电机极距  mm
    Uint16 MT_EncoderPensH;         //H00_32 编码器线数H   / 光栅尺分辨率  0.01um
    Uint16 MT_InitTheta;            //H00_33 Z信号对应电角度
    Uint16 MT_UposedgeTheta;        //H00_34 U相上升沿对应电角度
    //H00_01--H00_34是电机关联参数
    Uint16 MT_AbsRomMotorModel;     //H00_35 总线电机型号
    Uint16 MT_Rsvd36;               //H00_36
    Uint16 MT_AbsEncFunBit;         //H00_37 绝对编码器功能设置位
    Uint16 MT_Rsvd38;               //H00_38
    Uint16 MT_Rsvd39;               //H00_39
    Uint16 MT_Rsvd40;               //H00_40
    Uint16 MT_Rsvd41;               //H00_41
    Uint16 MT_Rsvd42;               //H00_42
    Uint16 MT_Rsvd43;               //H00_43
    Uint16 MT_Rsvd44;               //H00_44
    Uint16 MT_Rsvd45;               //H00_45
    Uint16 MT_Rsvd46;               //H00_46
    Uint16 MT_Rsvd47;               //H00_47
    Uint16 MT_Rsvd48;               //H00_48
    Uint16 MT_Rsvd49;               //H00_49
    Uint16 MT_Rsvd50;               //H00_50
    Uint16 MT_Rsvd51;               //H00_51
    Uint16 MT_Rsvd52;               //H00_52
    Uint16 MT_Rsvd53;               //H00_53
    Uint16 MT_Rsvd54;               //H00_54
    Uint16 MT_Rsvd55;               //H00_55
    Uint16 MT_Rsvd56;               //H00_56
    Uint16 MT_Rsvd57;               //H00_57
    Uint16 MT_Rsvd58;               //H00_58
    Uint16 MemCheck00;              //H00_59 组校验字

//0076--------------驱动器参数  H01LEN = 70 -------------------------------------------------------
    Uint16 OEM_SoftVersion;         //H01_00 软件版本号  不跟驱动器关联
    Uint16 OEM_FpgaVersion;         //H01_01 FPGA软件版本号  不跟驱动器关联
    Uint16 OEM_ServoSeri;           //H01_02 伺服驱动系列号
    //H01_03--H01_21是驱动器关联参数
    Uint16 OEM_RsdServoSeri;        //H01_03 保留的用于比较的驱动器型号
    Uint16 OEM_VoltClass;           //H01_04 电压级
    Uint16 OEM_RatePower;           //H01_05 额定功率
    Uint16 OEM_MaxPowerOut;         //H01_06 最大输出功率
    Uint16 OEM_RateCurrent;         //H01_07 驱动器额定输出电流
    Uint16 OEM_MaxCurrentOut;       //H01_08 驱动器最大输出电流
    Uint16 OEM_SdmClkEnbl;          //H01_09 Σ-Δ调制器外部时钟使能
    Uint16 OEM_CarrWaveFreq;        //H01_10 载波频率
    Uint16 OEM_ToqLoopFreqSel;      //H01_11 电流环调制频率选择
    Uint16 OEM_SpdLoopFreqScal;     //H01_12 速度环调度分频系数
    Uint16 OEM_PosLoopFreqScal;     //H01_13 位置环调度分频系数
    Uint16 OEM_PwmDeadT;            //H01_14 死区时间
    Uint16 OEM_UdcOver;             //H01_15 直流母线过压保护点
    Uint16 OEM_UdcLeak;             //H01_16 直流母线电压泄放点
    Uint16 OEM_UdcLow;              //H01_17 直流母线电压欠压点
    Uint16 OEM_OCProtectPoint;      //H01_18 驱动器过流保护点
    Uint16 IS_7860Gain;             //H01_19 7860采样系数
    Uint16 OEM_DeadComp;            //H01_20 死区补偿量 
    Uint16 OEM_Rsvd21;              //H01_21
    //H01_03--H01_21是驱动器关联参数
    Uint16 OEM_KedGain;             //H01_22 D轴反电动势补偿系数
    Uint16 OEM_KeqGain;             //H01_23 Q轴反电动势补偿系数
    Uint16 OEM_CurIdKpSec;          //H01_24 D轴电流环比例增益
    Uint16 OEM_CurIdKiSec;          //H01_25 D轴电流环积分补偿因子
    Uint16 OEM_Decimation;          //H01_26 电流采样Sinc3滤波器数据抽取率
    Uint16 OEM_CurIqKpSec;          //H01_27 Q轴电流环比例增益
    Uint16 OEM_CurIqKiSec;          //H01_28 Q轴电流环积分补偿因子
    Uint16 OEM_Rsvd29;              //H01_29
    Uint16 OEM_UdcGain;             //H01_30 母线电压增益调整
    Uint16 OEM_FocCalTime;          //H01_31 FOC计算时间
    Uint16 OEM_V2UCalCoff;          //H01_32 UV采样相对增益
    Uint16 OEM_HighPrecisionAIEn;   //H01_33 高精度AI使能     手册和后台不公开
    Uint16 ErrMsgSaveDisable;       //H01_34 故障记录存储开关 0 存储 1 不存储    手册和后台不公开
    Uint16 OEM_LocalModeEn;         //H01_35 本地模式使能
    Uint16 OEM_Rsvd36;              //H01_36    
    Uint16 OEM_Rsvd37;              //H01_37    
    Uint16 OEM_Rsvd38;              //H01_38    
    Uint16 OEM_Rsvd39;              //H01_39    
    Uint16 OEM_Rsvd40;              //H01_40    
    Uint16 OEM_Rsvd41;              //H01_41    
    Uint16 OEM_Rsvd42;              //H01_42    
    Uint16 OEM_Rsvd43;              //H01_43    
    Uint16 OEM_SecDecimation;       //H01_44 第二组电流采样Sinc3滤波器数据抽取率
    Uint16 OEM_DutyU;               //H01_45 电压注入辨识电角度时直接给定占空比
    Uint16 OEM_SrchCurFreq;         //H01_46 电压注入辨识电角度时得到的载频设置，不开放
    Uint16 OEM_IqCalTm;             //H01_47 MCU电流指令处理时间
    Uint16 OEM_ADSamDly;            //H01_48 电流检测回路延时
    Uint16 OEM_AbsTransDelay;       //H01_49 总线编码器数据传播延时   0.01us
    Uint16 OEM_DSPVerBD;            //H01_50 DSP软件内部版本号
    Uint16 OEM_FPGAVerBD;           //H01_51 FPGA软件内部版本号
    Uint16 OEM_CapIdKp;              //H01_52 性能优先模式D轴比例增益
    Uint16 OEM_CapIdKi;              //H01_53 性能优先模式D轴积分补偿因子
    Uint16 OEM_CapIqKp;              //H01_54 性能优先模式Q轴比例增益
    Uint16 OEM_CapIqKi;              //H01_55 性能优先模式Q轴积分补偿因子
    Uint16 OEM_CapSecKpCoff;         //H01_56 性能优先模式第二组比例增益系数
    Uint16 OEM_CapThirdKpCoff;       //H01_57 性能优先模式第三组比例增益系数
    Uint16 OEM_FirSwtchIq;           //H01_58 性能优先模式第一增益切换点
    Uint16 OEM_SecSwtchIq;           //H01_59 性能优先模式第二增益切换点
    Uint16 OEM_ThirdSwtchIq;         //H01_60 性能优先模式第三增益切换点
    Uint16 OEM_FourthSwtchIq;        //H01_61 性能优先模式第四增益切换点
    Uint16 OEM_OvrCurUV;            //H01_62 U V相7860检测保护点
    Uint16 OEM_AbsTransCompTime;    //H01_63 总线编码器数据传输补偿时间 
    Uint16 OEM_HDHAbsTstTime;       //H01_64 海德汉编码器恢复时间 0.001us
    Uint16 OEM_HDHEncBaud;          //H01_65 海德汉编码器通讯频率 kHz
    Uint16 OEM_EncCmdTransLineDelay;//H01_66 总线编码器命令字传输线延时  0.01us
    Uint16 OEM_Rsvd67;              //H01_67 
    Uint16 OEM_Rsvd68;              //H01_68 
    Uint16 MemCheck01;              //H01_69 组校验字

//0146--------------基本控制参数  H02LEN = 50 -------------------------------------------------------
    Uint16 BP_ModeSelet;            //H02_00 模式选择
    Uint16 BP_AbsPosDetectionSel;   //H02_01 绝对位置检测系统选择
    Uint16 BP_RevlDir;              //H02_02 指令方向选择
    Uint16 BP_PulsFdbDir;           //H02_03 速度反馈方向选择-1:控制速度反馈乘以负一
    Uint16 BP_MinMeasureSpd;        //H02_04 最小速度设置
    Uint16 BP_StopSoff;             //H02_05 伺服OFF停机方式选择  
    Uint16 BP_StopNo2;              //H02_06 故障停止方式No.2
    Uint16 BP_OPStop;               //H02_07 超程停止方式
    Uint16 BP_StopNo1;              //H02_08 故障时停机方式No.1选择
    Uint16 BP_SonBrkDelay;          //H02_09 上电伺服On时抱闸打开释放延时
    Uint16 BP_Clasp2OffTime;        //H02_10 抱闸指令--伺服OFF延迟时间
    Uint16 BP_ClaspSpdLmt;          //H02_11 抱闸指令输出速度限制值
    Uint16 BP_Off2ClaspTime;        //H02_12 伺服OFF-抱闸指令等待时间
    Uint16 BP_SpdCmp;               //H02_13 转速测量及比较门限，2倍频或4倍频测速
    Uint16 BP_StopModStateCutSpd;   //H02_14 停机方式和停机状态切换速度条件值 (内部功能码，不对外公开)
    Uint16 BP_AlmDispSel;           //H02_15 LED警告显示选择
    Uint16 BP_OTStopAuxFun;         //H02_16 超程停机时锁存辅助处理  (内部功能码，不对外公开)
    Uint16 BP_PowOffZeroSpdStopEn;  //H02_17 掉电零速停机使能
    Uint16 BP_SonFltTime;           //H02_18 伺服on信号滤波时间
    Uint16 BP_SonBrkDelayTime;      //H02_19 伺服On上电抱闸延时打开时间
    Uint16 BP_Rsvd20;               //H02_20
    Uint16 BP_RBMinOhm;             //H02_21 驱动器允许的能耗电阻最小值
    Uint16 BP_RBPSizeI;             //H02_22 内置能耗电阻功率容量
    Uint16 BP_RBOhmI;               //H02_23 内置能耗电阻阻值
    Uint16 BP_RBEfficiency;         //H02_24 电阻散热系数
    Uint16 BP_RBChoice;             //H02_25 能耗电阻设置
    Uint16 BP_RBPSizeO;             //H02_26 外置能耗电阻功率容量
    Uint16 BP_RBOhmO;               //H02_27 外置能耗电阻阻值
    Uint16 BP_Rsvd28;               //H02_28 
    Uint16 BP_UserPass_Rsvd;        //H02_29 保留用户密码
    Uint16 BP_UserPass;             //H02_30 用户密码
    Uint16 BP_InitServo;            //H02_31 系统参数初始化
    Uint16 BP_DefaultDisplayCode;   //H02_32 H0B功能码选择 (用于面板监控模式下显示)
    Uint16 BP_EtherCatVer;          //H02_33 EtherCat软件版本号
    Uint16 BP_CanVer;               //H02_34 CAN软件版本号
    Uint16 BP_PanelDisFreq;         //H02_35 面板数据刷新频率
    Uint16 BP_Rsvd36;               //H02_36
    Uint16 BP_StopModStateCutSpd2;  //H02_37
    Uint16 BP_Rsvd38;               //H02_38
    Uint16 BP_Rsvd39;               //H02_39
    Uint16 MT_EnVisable;            //H02_40 电机组参数可见使能
    Uint16 OEM_OEMPass;             //H02_41 厂家密码
    Uint16 BP_Rsvd42;               //H02_42
    Uint16 BP_Rsvd43;               //H02_43
    Uint16 BP_Rsvd44;               //H02_44
    Uint16 BP_Rsvd45;               //H02_45
    Uint16 BP_Rsvd46;               //H02_46
    Uint16 BP_Rsvd47;               //H02_47
    Uint16 BP_Rsvd48;               //H02_48
    Uint16 MemCheck02;              //H02_49 组校验字

//0196--------------端子输入参数  H03LEN = 100 -------------------------------------------------------
    /* DI */
    Uint16 DIDeftL;                 //H03_00 DI信号未分配默认状态 低16位低字节
    Uint16 DIDeftH;                 //H03_01 DI信号未分配默认状态 低16位高字节
    Uint16 DIFuncSel1;              //H03_02 DI1端子功能选择
    Uint16 DILogicSel1;             //H03_03 DI1端子逻辑选择
    Uint16 DIFuncSel2;              //H03_04 DI2端子功能选择
    Uint16 DILogicSel2;             //H03_05 DI2端子逻辑选择
    Uint16 DIFuncSel3;              //H03_06 DI3端子功能选择
    Uint16 DILogicSel3;             //H03_07 DI3端子逻辑选择
    Uint16 DIFuncSel4;              //H03_08 DI4端子功能选择
    Uint16 DILogicSel4;             //H03_09 DI4端子逻辑选择
    Uint16 DIFuncSel5;              //H03_10 DI5端子功能选择
    Uint16 DILogicSel5;             //H03_11 DI5端子逻辑选择
    Uint16 DIFuncSel6;              //H03_12 DI6端子功能选择
    Uint16 DILogicSel6;             //H03_13 DI6端子逻辑选择
    Uint16 DIFuncSel7;              //H03_14 DI7端子功能选择
    Uint16 DILogicSel7;             //H03_15 DI7端子逻辑选择
    Uint16 DIFuncSel8;              //H03_16 DI8端子功能选择
    Uint16 DILogicSel8;             //H03_17 DI8端子逻辑选择
    Uint16 DIFuncSel9;              //H03_18 DI9端子功能选择
    Uint16 DILogicSel9;             //H03_19 DI9端子逻辑选择
    Uint16 DI_Rsvd20;               //H03_20 DI10端子功能选择
    Uint16 DI_Rsvd21;               //H03_21 DI10端子逻辑选择
    Uint16 DI_Rsvd22;               //H03_22 DI11端子功能选择
    Uint16 DI_Rsvd23;               //H03_23 DI11端子逻辑选择
    Uint16 DI_Rsvd24;               //H03_24 DI12端子功能选择
    Uint16 DI_Rsvd25;               //H03_25 DI12端子逻辑选择
    Uint16 DI_Rsvd26;               //H03_26 DI13端子功能选择
    Uint16 DI_Rsvd27;               //H03_27 DI13端子逻辑选择
    Uint16 DI_Rsvd28;               //H03_28 DI14端子功能选择
    Uint16 DI_Rsvd29;               //H03_29 DI14端子逻辑选择
    Uint16 DI_Rsvd30;               //H03_30 DI15端子功能选择
    Uint16 DI_Rsvd31;               //H03_31 DI15端子逻辑选择
    Uint16 DI_Rsvd32;               //H03_32 DI16端子功能选择
    Uint16 DI_Rsvd33;               //H03_33 DI16端子逻辑选择
    Uint16 DIDeftHL;                //H03_34 DI信号未分配默认状态 高16位低字节
    Uint16 DIDeftHH;                //H03_35 DI信号未分配默认状态 高16位高字节
    Uint16 DI_Rsvd36;               //H03_36
    Uint16 DI_Rsvd37;               //H03_37
    Uint16 DI_Rsvd38;               //H03_38
    Uint16 DI_Rsvd39;               //H03_39
    Uint16 DI_Rsvd40;               //H03_40
    Uint16 DI_Rsvd41;               //H03_41
    Uint16 DI_Rsvd42;               //H03_42
    Uint16 DI_Rsvd43;               //H03_43
    Uint16 DI_Rsvd44;               //H03_44
    Uint16 DI_Rsvd45;               //H03_45
    Uint16 DI_Rsvd46;               //H03_46
    Uint16 DI_Rsvd47;               //H03_47
    Uint16 DI_Rsvd48;               //H03_48
    Uint16 DI_Rsvd49;               //H03_49

    /* AI */
    Uint16 AI1_Offset;              //H03_50 AI1偏置                1mV
    Uint16 AI1_InFltTime;           //H03_51 AI1输入滤波时间        0.01ms
    Uint16 AI1_MidFltrEn;           //H03_52 AI1输入中值滤波使能
    Uint16 AI1_DeadT;               //H03_53 AI1死区                0.1mV
    Uint16 Ai1_ZeroDrift;           //H03_54 AI1零漂                0.1mV
    Uint16 AI2_Offset;              //H03_55 AI2偏置                1mV
    Uint16 AI2_InFltTime;           //H03_56 AI2输入滤波时间        0.01ms
    Uint16 AI2_MidFltrEn;           //H03_57 AI2输入中值滤波使能
    Uint16 AI2_DeadT;               //H03_58 AI2死区                0.1mV
    Uint16 Ai2_ZeroDrift;           //H03_59 AI2零漂                0.1mV
    Uint16 AI3_Offset;              //H03_60 AI3偏置                1mV     (保留)
    Uint16 AI3_InFltTime;           //H03_61 AI3输入滤波时间        0.01ms  (保留)
    Uint16 AI3_MidFltrEn;           //H03_62 AI3输入中值滤波使能            (保留)
    Uint16 AI3_DeadT;               //H03_63 AI3死区                0.1mV   (保留)
    Uint16 Ai3_ZeroDrift;           //H03_64 AI3零漂                0.1mV   (保留)
    Uint16 AI_Rsvd65;               //H03_65
    Uint16 AI_Rsvd66;               //H03_66
    Uint16 AI_Rsvd67;               //H03_67
    Uint16 AI_Rsvd68;               //H03_68 
    Uint16 AI_Rsvd69;               //H03_69 
    Uint16 AI_Rsvd70;               //H03_70 
    Uint16 AI_Rsvd71;               //H03_71
    Uint16 AI_Rsvd72;               //H03_72 
    Uint16 AI_Rsvd73;               //H03_73 
    Uint16 AI_Rsvd74;               //H03_74 
    Uint16 AI_Rsvd75;               //H03_75 
    Uint16 AI_Rsvd76;               //H03_76 
    Uint16 AI_Rsvd77;               //H03_77
    Uint16 AI_Rsvd78;               //H03_78
    Uint16 AI_Rsvd79;               //H03_79
    Uint16 AI_SpdGain;              //H03_80 模拟量10V对应速度1RPM
    Uint16 AI_ToqGain;              //H03_81 模拟量10V对应转矩
    Uint16 AI_Rsvd82;               //H03_82
    Uint16 AI_Rsvd83;               //H03_83
    Uint16 AI_Rsvd84;               //H03_84
    Uint16 AI_Rsvd85;               //H03_85
    Uint16 AI_Rsvd86;               //H03_86
    Uint16 AI_Rsvd87;               //H03_87
    Uint16 AI_Rsvd88;               //H03_88
    Uint16 AI_Rsvd89;               //H03_89
    Uint16 AI_Rsvd90;               //H03_90
    Uint16 AI_Rsvd91;               //H03_91
    Uint16 AI_Rsvd92;               //H03_92
    Uint16 AI_Rsvd93;               //H03_93
    Uint16 AI_Rsvd94;               //H03_94
    Uint16 AI_Rsvd95;               //H03_95
    Uint16 AI_Rsvd96;               //H03_96
    Uint16 AI_Rsvd97;               //H03_97
    Uint16 AI_Rsvd98;               //H03_98
    Uint16 MemCheck03;              //H03_99 组校验字

//0296--------------端子输出参数  H04LEN = 70 -------------------------------------------------------
    /* DO */
    Uint16 DOFuncSel1;                  //H04_00 DO01端子功能选择 
    Uint16 DOLogicSel1;                 //H04_01 DO01端子逻辑选择 
    Uint16 DOFuncSel2;                  //H04_02 DO02端子功能选择 
    Uint16 DOLogicSel2;                 //H04_03 DO02端子逻辑选择 
    Uint16 DOFuncSel3;                  //H04_04 DO03端子功能选择 
    Uint16 DOLogicSel3;                 //H04_05 DO03端子逻辑选择 
    Uint16 DOFuncSel4;                  //H04_06 DO04端子功能选择 
    Uint16 DOLogicSel4;                 //H04_07 DO04端子逻辑选择   
    Uint16 DOFuncSel5;                  //H04_08 DO05端子功能选择 
    Uint16 DOLogicSel5;                 //H04_09 DO05端子逻辑选择 
    Uint16 DO_Rsvd10;                   //H04_10 DO06端子功能选择 
    Uint16 DO_Rsvd11;                   //H04_11 DO06端子逻辑选择 
    Uint16 DO_Rsvd12;                   //H04_12 DO07端子功能选择 
    Uint16 DO_Rsvd13;                   //H04_13 DO07端子逻辑选择 
    Uint16 DO_Rsvd14;                   //H04_14 DO08端子功能选择 
    Uint16 DO_Rsvd15;                   //H04_15 DO08端子逻辑选择 
    Uint16 DO_Rsvd16;                   //H04_16 DO09端子功能选择 
    Uint16 DO_Rsvd17;                   //H04_17 DO09端子逻辑选择 
    Uint16 DO_Rsvd18;                   //H04_18 DO10端子功能选择 
    Uint16 DO_Rsvd19;                   //H04_19 DO10端子逻辑选择 
    Uint16 DO_Rsvd20;                   //H04_20 DO11端子功能选择 
    Uint16 DO_Rsvd21;                   //H04_21 DO11端子逻辑选择 
    Uint16 DO_SourceSel;                //H04_22 DO来源选择
    Uint16 DO_Rsvd23;                   //H04_23
    Uint16 DO_Rsvd24;                   //H04_24
    Uint16 DO_Rsvd25;                   //H04_25
    Uint16 DO_Rsvd26;                   //H04_26
    Uint16 DO_Rsvd27;                   //H04_27
    Uint16 DO_Rsvd28;                   //H04_28
    Uint16 DO_Rsvd29;                   //H04_29
    Uint16 DO_Rsvd30;                   //H04_30
    Uint16 DO_Rsvd31;                   //H04_31
    Uint16 DO_Rsvd32;                   //H04_32
    Uint16 DO_Rsvd33;                   //H04_33
    Uint16 DO_Rsvd34;                   //H04_34
    Uint16 DO_Rsvd35;                   //H04_35
    Uint16 DO_Rsvd36;                   //H04_36
    Uint16 DO_Rsvd37;                   //H04_37
    Uint16 DO_Rsvd38;                   //H04_38
    Uint16 DO_Rsvd39;                   //H04_39
    Uint16 DO_Rsvd40;                   //H04_40
    Uint16 DO_Rsvd41;                   //H04_41
    Uint16 DO_Rsvd42;                   //H04_42
    Uint16 DO_Rsvd43;                   //H04_43
    Uint16 DO_Rsvd44;                   //H04_44
    Uint16 DO_Rsvd45;                   //H04_45
    Uint16 DO_Rsvd46;                   //H04_46
    Uint16 DO_Rsvd47;                   //H04_47
    Uint16 DO_Rsvd48;                   //H04_48
    Uint16 DO_Rsvd49;                   //H04_49 
    /* AO */
    Uint16 AO1SignalSel;                //H04_50 A01信号选择
    Uint16 AO1Offset;                   //H04_51 AO1偏移量电压 mV
    Uint16 AO1Gain;                     //H04_52 AO1倍率   0.01倍
    Uint16 AO2SignalSel;                //H04_53 AO2信号选择
    Uint16 AO2Offset;                   //H04_54 AO2偏移量电压 mV
    Uint16 AO2Gain;                     //H04_55 AO2倍率   0.01倍
    Uint16 AO_Rsvd56;                   //H04_56
    Uint16 AO_Rsvd57;                   //H04_57
    Uint16 AO_Rsvd58;                   //H04_58
    Uint16 AO_Rsvd59;                   //H04_59
    Uint16 AO_Rsvd60;                   //H04_60
    Uint16 AO_Rsvd61;                   //H04_61
    Uint16 AO_Rsvd62;                   //H04_62
    Uint16 AO_Rsvd63;                   //H04_63
    Uint16 AO_Rsvd64;                   //H04_64
    Uint16 AO_Rsvd65;                   //H04_65
    Uint16 AO_Rsvd66;                   //H04_66
    Uint16 AO_Rsvd67;                   //H04_67
    Uint16 AO_Rsvd68;                   //H04_68
    Uint16 MemCheck04;                  //H04_69 组校验字

//0366--------------位置控制参数  H05LEN = 90 -------------------------------------------------------
    Uint16 PL_PosSurcASel;              //H05_00 位置指令A来源
    Uint16 PL_PulsePosCmdSel;           //H05_01 高低速脉冲位置指令选择 0--低速，1--高速
    Uint16 PL_PulsePRevLow;             //H05_02 电机每旋转一圈的指令脉冲数低位
    Uint16 PL_PulsePRevHigh;            //H05_03 电机每旋转一圈的指令脉冲数高位
    Uint16 PL_PulseInFilterTime;        //H05_04 一阶低通滤波时间
    Uint16 PL_PosStep;                  //H05_05 步进位置指令总量
    Uint16 PL_PosAverge_tme;            //H05_06 位置指令移动平均时间
    Uint16 PL_PosFirCmxLow;             //H05_07 电子齿数比1 分子 L
    Uint16 PL_PosFirCmxHigh;            //H05_08 电子齿数比1 分子 H
    Uint16 PL_PosFirCdvLow;             //H05_09 电子齿数比1 分母 L
    Uint16 PL_PosFirCdvHigh;            //H05_10 电子齿数比1 分母 H
    Uint16 PL_PosSecCmxLow;             //H05_11 电子齿数比2 分子 L
    Uint16 PL_PosSecCmxHigh;            //H05_12 电子齿数比2 分子 H
    Uint16 PL_PosSecCdvLow;             //H05_13 电子齿数比2 分母 L
    Uint16 PL_PosSecCdvHigh;            //H05_14 电子齿数比2 分母 H
    Uint16 PL_PosPulsInMode;            //H05_15 指令脉冲形态
    Uint16 PL_PerrClrMode;              //H05_16 清除动作选择
    Uint16 PL_PosCodeDiv_L;             //H05_17 编码器分频脉冲数 L
    Uint16 PL_PosCodeDiv_H;             //H05_18 编码器分频脉冲数 H
    Uint16 PL_PosForeBaceSel;           //H05_19 位置前馈控制选择
    Uint16 PL_ReachConditio;            //H05_20 定位完成信号COIN输出条件
    Uint16 PL_PosReachValue;            //H05_21 定位完成幅度
    Uint16 PL_PosNearValue;             //H05_22 定位完成接近信号NEAR幅度
    Uint16 PL_XintPosSel;               //H05_23 中断位移使能
    Uint16 PL_XintPosRef_Lo;            //H05_24 中断位移长度 L
    Uint16 PL_XintPosRef_Hi;            //H05_25 中断位移长度 H
    Uint16 PL_XintMoveSpd;              //H05_26 移动速度
    Uint16 PL_XintAccTime;              //H05_27 加减速时间
    Uint16 PL_XintSmSpd;                //H05_28 预留 中断定长是否以中断时速度运行
    Uint16 PL_XintFreeEn;               //H05_29 中断锁定信号使能
    Uint16 PL_OriginResetEn;            //H05_30 原点复位使能
    Uint16 PL_OriginResetMode;          //H05_31 原点复位模式
    Uint16 PL_OriginHighSpeedSearch;    //H05_32 原点高速搜索速度
    Uint16 PL_OriginLowSpeedSearch;     //H05_33 原点低速搜索速度
    Uint16 PL_OriginRiseDownTime;       //H05_34 原点加减速时间
    Uint16 PL_OriginSearchTime;         //H05_35 原点查找时间
    Uint16 PL_OriginOffsetLow;          //H05_36 原点机械偏移量 L
    Uint16 PL_OriginOffsetHigh;         //H05_37 原点机械偏移量 H
    Uint16 PL_DivSourceSel;             //H05_38 伺服脉冲输出来源选择
    Uint16 PL_PosGearChangeEn;          //H05_39 电子齿轮切换使能
    Uint16 PL_OriginOriginSel;          //H05_40 bit0-原点是否偏移，bit1-遇限位是否反向找零 
    Uint16 PL_EncDivZPolSet;            //H05_41 Z脉冲输出极性设置，0-正极性设置(Z脉冲为高电平) 1-负极性设置(Z脉冲为低电平)
    Uint16 PL_PosPulsInMode2;           //H05_42 高速脉冲指令脉冲形态 保留给非标使用
    Uint16 PL_PosCmdEdge;               //H05_43 位置脉冲边沿选择：1～上升沿有效，0～下降沿有效
    Uint16 PL_EncMultiTurnOffset;       //H05_44 编码器多圈数据偏置
    Uint16 PL_HDHEncMultiTurnOffset;    //H05_45 海德汉编码器多圈数据偏置
    Uint16 PL_MultiAbsPosOffsetL;       //H05_46 多圈绝对位置偏置L
    Uint16 PL_MultiAbsPosOffsetH;       //H05_47 多圈绝对位置偏置H
    Uint16 PL_MultiAbsPosOffsetH32L;    //H05_48 多圈绝对位置偏置H32L
    Uint16 PL_MultiAbsPosOffsetH32H;    //H05_49 多圈绝对位置偏置H32H
    Uint16 PL_AbsMode2MechGearNum;      //H05_50 多圈绝对位置模式2 机械齿轮比分子
    Uint16 PL_AbsMode2MechGearDen;      //H05_51 多圈绝对位置模式2 机械齿轮比分母
    Uint16 PL_MechAbsPosLimUpL;         //H05_52 多圈绝对位置模式2 机械绝对位置上限值 低32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH;         //H05_53 多圈绝对位置模式2 机械绝对位置上限值 低32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH32L;      //H05_54 多圈绝对位置模式2 机械绝对位置上限值 高32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH32H;      //H05_55 多圈绝对位置模式2 机械绝对位置上限值 高32位（编码器单位）
    Uint16 PL_Rsvd56;                   //H05_56 
    Uint16 PL_Rsvd57;                   //H05_57 
    Uint16 PL_Rsvd58;                   //H05_58 
    Uint16 PL_Rsvd59;                   //H05_59 
    Uint16 PL_CoinHoldTime;             //H05_60 定位完成保持时间
   	Uint16 PL_PosWinUnitSet;            //H05_61 位置到达窗口单位设置 0-编码器单位 1-指令单位
    Uint16 PL_Rsvd62;                   //H05_62 
    Uint16 PL_Rsvd63;                   //H05_63 
    Uint16 PL_Rsvd64;                   //H05_64 
    Uint16 PL_Rsvd65;                   //H05_65 
    Uint16 PL_Rsvd66;                   //H05_66 
    Uint16 PL_Rsvd67;                   //H05_67 
    Uint16 PL_Rsvd68;                   //H05_68 
    Uint16 PL_Rsvd69;                   //H05_69 
    Uint16 PL_Rsvd70;                   //H05_70 
    Uint16 PL_Rsvd71;                   //H05_71 
    Uint16 PL_Rsvd72;                   //H05_72 
    Uint16 PL_Rsvd73;                   //H05_73 
    Uint16 PL_Rsvd74;                   //H05_74 
    Uint16 PL_Rsvd75;                   //H05_75 
    Uint16 PL_Rsvd76;                   //H05_76 
    Uint16 PL_Rsvd77;                   //H05_77 
    Uint16 PL_Rsvd78;                   //H05_78 
    Uint16 PL_Rsvd79;                   //H05_79 
    Uint16 PL_Rsvd80;                   //H05_80
    Uint16 PL_Rsvd81;                   //H05_81
    Uint16 PL_Rsvd82;                   //H05_82
    Uint16 PL_Rsvd83;                   //H05_83
    Uint16 PL_Rsvd84;                   //H05_84
    Uint16 PL_Rsvd85;                   //H05_85
    Uint16 PL_Rsvd86;                   //H05_86
    Uint16 PL_Rsvd87;                   //H05_87
    Uint16 PL_Rsvd88;                   //H05_88
    Uint16 MemCheck05;                  //H05_89 组校验字

//0456--------------速度控制参数  H06LEN = 50 -------------------------------------------------------
    Uint16 SL_SpdACommandFrom;          //H06_00 速度指令A来源
    Uint16 SL_SpdBCommandFrom;          //H06_01 速度指令B来源
    Uint16 SL_SpdABSwitchFlag;          //H06_02 速度指令选择
    Uint16 SL_SpdCMDFromKeyBoard;       //H06_03 速度指令键盘设定值     1rpm
    Uint16 SL_JOGSpdCommand;            //H06_04 点动速度设定值     1rpm
    Uint16 SL_SpdCMDRiseTime;           //H06_05 速度指令加速斜坡时间  1ms
    Uint16 SL_SpdCMDDownTime;           //H06_06 速度指令减速斜坡时间  1ms
    Uint16 SL_SpdMaxLimit;              //H06_07 最大转速限制值        1rpm
    Uint16 SL_SpdPositiveLimit;         //H06_08 速度正向限制          1rpm
    Uint16 SL_SpdNegativeLimit;         //H06_09 速度反向限制          1rpm
    Uint16 SL_Rsvd10;                   //H06_10 速度反馈滤波选择 (删掉)
    Uint16 SL_SpdForwardChooseFlag;     //H06_11 转矩前馈选择
    Uint16 SL_Rsvd12;                   //H06_12
    Uint16 SL_Rsvd13;                   //H06_13 
    Uint16 SL_Rsvd14;                   //H06_14
    Uint16 SL_SpdZeroClampTolerance;    //H06_15 零速钳位/零位固定速度指令阈值    1rpm
    Uint16 SL_SpdZeroTolerance;         //H06_16 电机旋转状态阈值    1rpm
    Uint16 SL_SpdReachTolerance;        //H06_17 速度一致信号宽度    1rpm
    Uint16 SL_SpdArriveTolerance;       //H06_18 速度到达信号阈值    1rpm
    Uint16 SL_ZeroSpdTolerance;         //H06_19 零速输出信号阈值    1rpm
    Uint16 SL_Rsvd20;                   //H06_20
    Uint16 SL_Rsvd21;                   //H06_21
    Uint16 SL_Rsvd22;                   //H06_22
    Uint16 SL_Rsvd23;                   //H06_23
    Uint16 SL_Rsvd24;                   //H06_24
    Uint16 SL_Rsvd25;                   //H06_25
    Uint16 SL_Rsvd26;                   //H06_26
    Uint16 SL_Rsvd27;                   //H06_27
    Uint16 SL_Rsvd28;                   //H06_28
    Uint16 SL_Rsvd29;                   //H06_29
    Uint16 SL_Rsvd30;                   //H06_30
    Uint16 SL_Rsvd31;                   //H06_31
    Uint16 SL_Rsvd32;                   //H06_32
    Uint16 SL_Rsvd33;                   //H06_33
    Uint16 SL_Rsvd34;                   //H06_34
    Uint16 SL_Rsvd35;                   //H06_35
    Uint16 SL_Rsvd36;                   //H06_36
    Uint16 SL_Rsvd37;                   //H06_37
    Uint16 SL_Rsvd38;                   //H06_38
    Uint16 SL_Rsvd39;                   //H06_39
    Uint16 SL_Rsvd40;                   //H06_40
    Uint16 SL_Rsvd41;                   //H06_41
    Uint16 SL_Rsvd42;                   //H06_42
    Uint16 SL_Rsvd43;                   //H06_43
    Uint16 SL_Rsvd44;                   //H06_44
    Uint16 SL_Rsvd45;                   //H06_45
    Uint16 SL_Rsvd46;                   //H06_46
    Uint16 SL_Rsvd47;                   //H06_47
    Uint16 SL_Rsvd48;                   //H06_48
    Uint16 MemCheck06;                  //H06_49 组校验字

//0506--------------转矩控制参数  H07LEN = 70 -------------------------------------------------------
    Uint16 TL_ToqCmdSourceA;          //H07_00 转矩指令A来源
    Uint16 TL_ToqCmdSourceB;          //H07_01 转矩指令B来源
    Uint16 TL_ToqCmdSel;              //H07_02 转矩指令选择
    Uint16 TL_ToqCmdInner;            //H07_03 转矩指令键盘设定
    Uint16 TL_Rsvd04;                 //H07_04 保留
    Uint16 TL_ToqFiltTime;            //H07_05 转矩指令滤波时间1
    Uint16 TL_ToqFiltTime2;           //H07_06 转矩指令滤波时间2
    Uint16 TL_ToqLmtSource;           //H07_07 转矩限制来源
    Uint16 TL_ToqLmtAISel;            //H07_08 T-LMT选择
    Uint16 TL_ToqPlusLmtIn;           //H07_09 正转内部转矩限制值
    Uint16 TL_ToqMinusLmtIn;          //H07_10 反转内部转矩限制值
    Uint16 TL_ToqPlusLmtTrig;         //H07_11 正转侧外部转矩限制值
    Uint16 TL_ToqMinusLmtTrig;        //H07_12 反转侧外部转矩限制值
    Uint16 TL_ToqPlusComp;            //H07_13 正向转矩补偿量
    Uint16 TL_ToqMinusComp;           //H07_14 反向转矩补偿量
    Uint16 TL_EmergentToq;            //H07_15 急停转矩
    Uint16 TL_Rsvd16;                 //H07_16 
    Uint16 TL_SpdLmtSel;              //H07_17 速度限制来源选择
    Uint16 TL_SpdLmtAiSel;            //H07_18 V-LMT选择
    Uint16 TL_SpdLmtIn;               //H07_19 转矩控制时内部速度限制值
    Uint16 TL_SpdLmtNegIn;            //H07_20 转矩控制时内部速度负向限制值
    Uint16 TL_ToqRchStandard;         //H07_21 转矩到达基准值
    Uint16 TL_ToqRchOn;               //H07_22 转矩到达DO信号开启时输出转矩值
    Uint16 TL_ToqRchOff;              //H07_23 转矩到达DO信号关闭时输出转矩值
    Uint16 TL_Rsvd24;                 //H07_24 
    Uint16 TL_Rsvd25;                 //H07_25 
    Uint16 TL_Rsvd26;                 //H07_26 
    Uint16 TL_Rsvd27;                 //H07_27 
    Uint16 TL_Rsvd28;                 //H07_28 
    Uint16 TL_Rsvd29;                 //H07_29 
    Uint16 TL_Rsvd30;                 //H07_30 
    Uint16 TL_Rsvd31;                 //H07_31 
    Uint16 TL_Rsvd32;                 //H07_32 
    Uint16 TL_Rsvd33;                 //H07_33 
    Uint16 TL_Rsvd34;                 //H07_34 
    Uint16 TL_Rsvd35;                 //H07_35 
    Uint16 TL_Rsvd36;                 //H07_36 
    Uint16 TL_Rsvd37;                 //H07_37 
    Uint16 TL_SpdLmtSatura;           //H07_38 
    Uint16 TL_SpdLmtReguSel;          //H07_39 速度限制调节器选择
    Uint16 TL_OVSpdLmtWaitT;          //H07_40 是否超过速度限制判断等待时间
    Uint16 TL_Rsvd41;                 //H07_41 
    Uint16 TL_Rsvd42;                 //H07_42 
    Uint16 TL_Rsvd43;                 //H07_43 
    Uint16 TL_Rsvd44;                 //H07_44 
    Uint16 TL_Rsvd45;                 //H07_45 
    Uint16 TL_Rsvd46;                 //H07_46 
    Uint16 TL_Rsvd47;                 //H07_47 
    Uint16 TL_Rsvd48;                 //H07_48 
    Uint16 TL_Rsvd49;                 //H07_49 
    Uint16 TL_Rsvd50;                 //H07_50 
    Uint16 TL_Rsvd51;                 //H07_51 
    Uint16 TL_Rsvd52;                 //H07_52 
    Uint16 TL_Rsvd53;                 //H07_53 
    Uint16 TL_Rsvd54;                 //H07_54 
    Uint16 TL_Rsvd55;                 //H07_55 
    Uint16 TL_Rsvd56;                 //H07_56 
    Uint16 TL_Rsvd57;                 //H07_57 
    Uint16 TL_Rsvd58;                 //H07_58 
    Uint16 TL_Rsvd59;                 //H07_59 
    Uint16 TL_Rsvd60;                 //H07_60
    Uint16 TL_Rsvd61;                 //H07_61
    Uint16 TL_Rsvd62;                 //H07_62
    Uint16 TL_Rsvd63;                 //H07_63
    Uint16 TL_Rsvd64;                 //H07_64
    Uint16 TL_Rsvd65;                 //H07_65
    Uint16 TL_Rsvd66;                 //H07_66
    Uint16 TL_Rsvd67;                 //H07_67
    Uint16 TL_Rsvd68;                 //H07_68
    Uint16 MemCheck07;                //H07_69 组校验字

//0576--------------增益类参数  H08LEN = 90 -------------------------------------------------------
    Uint16 GN_Spd_Kp;                 //H08_00 速度环增益  0.1Hz
    Uint16 GN_Spd_Ti;                 //H08_01 速度环积分时间常数  0.01ms
    Uint16 GN_Pos_Kp;                 //H08_02 位置环增益  0.1Hz
    Uint16 GN_Spd_Kp2;                //H08_03 第2速度环增益  0.1Hz
    Uint16 GN_Spd_Ti2;                //H08_04 第2速度环积分时间常数  0.01ms
    Uint16 GN_Pos_Kp2;                //H08_05 第2位置环增益  0.1Hz
    Uint16 GN_ServoCtrlMode;          //H08_06 性能模式：0～高速优先，1～性能优先
    Uint16 GN_Rsvd07;                 //H08_07
    Uint16 GN_Gn2ndSetup;             //H08_08 第二增益模式设置
    Uint16 GN_GnSwModeSel;            //H08_09 增益切换条件选择
    Uint16 GN_GnSwDlyTm;              //H08_10 增益切换延时时间 0.1ms
    Uint16 GN_GnSwLvl;                //H08_11 增益切换等级
    Uint16 GN_GnSwHysteresis;         //H08_12 增益切换时滞
    Uint16 GN_PosGnSwTm;              //H08_13 位置增益切换时间 0.1ms
    Uint16 GN_Rsvd14;                 //H08_14 
    Uint16 GN_InertiaRatio;           //H08_15 负载转动惯量比
    Uint16 GN_Rsvd16;                 //H08_16 
    Uint16 GN_Rsvd17;                 //H08_17 保留
    Uint16 GN_SpdFbFltrT;             //H08_18 速度前馈滤波时间参数
    Uint16 GN_SpdFb_Kp;               //H08_19 速度前馈增益
    Uint16 GN_ToqFbFltrT;             //H08_20 转矩前馈滤波时间参数
    Uint16 GN_ToqFb_Kp;               //H08_21 转矩前馈增益
    Uint16 GN_SpdFdbFilt_On;          //H08_22 FPGA速度平均值滤波使能  默认为0
    Uint16 GN_SpdLpFiltFc;            //H08_23 速度反馈低通滤波截止频率
    Uint16 GN_Pdff_Kf;                //H08_24 PDFF控制系数	 Disable时为100%
    Uint16 GN_SpdFbSel;               //H08_25 速度反馈选择
    Uint16 GN_Damping_Kf;             //H08_26 Damping factor控制系数，理论匹配值为25%，Disable时为0，Max为100%。
    Uint16 GN_Rsvd27;                 //H08_27
    Uint16 GN_Rsvd28;                 //H08_28
    Uint16 GN_Rsvd29;                 //H08_29
    Uint16 GN_Rsvd30;                 //H08_30
    Uint16 GN_Rsvd31;                 //H08_31
    Uint16 GN_Rsvd32;                 //H08_32
    Uint16 GN_Rsvd33;                 //H08_33
    Uint16 GN_Rsvd34;                 //H08_34
    Uint16 GN_Rsvd35;                 //H08_35
    Uint16 GN_Rsvd36;                 //H08_36
    Uint16 GN_Rsvd37;                 //H08_37
    Uint16 GN_Rsvd38;                 //H08_38
    Uint16 GN_Rsvd39;                 //H08_39
    Uint16 GN_Rsvd40;                 //H08_40 
    Uint16 GN_Rsvd41;                 //H08_41 
    Uint16 GN_Rsvd42;                 //H08_42 
    Uint16 GN_Rsvd43;                 //H08_43 
    Uint16 GN_Rsvd44;                 //H08_44 
    Uint16 GN_Rsvd45;                 //H08_45 
    Uint16 GN_Rsvd46;                 //H08_46 
    Uint16 GN_Rsvd47;                 //H08_47 
    Uint16 GN_Rsvd48;                 //H08_48 
    Uint16 GN_Rsvd49;                 //H08_49 
    Uint16 GN_Rsvd50;                 //H08_50  
    Uint16 GN_Rsvd51;                 //H08_51  
    Uint16 GN_Rsvd52;                 //H08_52  
    Uint16 GN_Rsvd53;                 //H08_53  
    Uint16 GN_Rsvd54;                 //H08_54  
    Uint16 GN_Rsvd55;                 //H08_55  
    Uint16 GN_Rsvd56;                 //H08_56  
    Uint16 GN_Rsvd57;                 //H08_57  
    Uint16 GN_Rsvd58;                 //H08_58  
    Uint16 GN_Rsvd59;                 //H08_59  
    Uint16 GN_Rsvd60;                 //H08_60 
    Uint16 GN_Rsvd61;                 //H08_61 
    Uint16 GN_Rsvd62;                 //H08_62 
    Uint16 GN_Rsvd63;                 //H08_63 
    Uint16 GN_Rsvd64;                 //H08_64 
    Uint16 GN_Rsvd65;                 //H08_65 
    Uint16 GN_Rsvd66;                 //H08_66 
    Uint16 GN_Rsvd67;                 //H08_67 
    Uint16 GN_Rsvd68;                 //H08_68 
    Uint16 GN_Rsvd69;                 //H08_69 
    Uint16 GN_Rsvd70;                 //H08_70
    Uint16 GN_Rsvd71;                 //H08_71
    Uint16 GN_Rsvd72;                 //H08_72
    Uint16 GN_Rsvd73;                 //H08_73
    Uint16 GN_Rsvd74;                 //H08_74
    Uint16 GN_Rsvd75;                 //H08_75
    Uint16 GN_Rsvd76;                 //H08_76
    Uint16 GN_Rsvd77;                 //H08_77
    Uint16 GN_Rsvd78;                 //H08_78
    Uint16 GN_Rsvd79;                 //H08_79
    Uint16 GN_Rsvd80;                 //H08_80
    Uint16 GN_Rsvd81;                 //H08_81
    Uint16 GN_Rsvd82;                 //H08_82
    Uint16 GN_Rsvd83;                 //H08_83
    Uint16 GN_Rsvd84;                 //H08_84
    Uint16 GN_Rsvd85;                 //H08_85
    Uint16 GN_Rsvd86;                 //H08_86
    Uint16 GN_Rsvd87;                 //H08_87
    Uint16 GN_Rsvd88;                 //H08_88
    Uint16 MemCheck08;                //H08_89 组校验字
              

//0686--------------自整定参数  H09LEN = 60 -------------------------------------------------------
    //英文名称Auto-gain Tuning
    Uint16 AutoTuningModeSel;           //H09_00 自调整模式选择
    Uint16 Rigidity_Level1st;           //H09_01 第1组刚性等级选择
    Uint16 AT_AdaptiveFilterMode;       //H09_02 自适应滤波器模式选择，模仿松下 
    Uint16 InertiaIdyCountModeSel;      //H09_03 在线惯量辨识模式
    Uint16 AT_LowOscMod;                //H09_04 低频抑振模式选择
    Uint16 OffLnInertiaModeSel;         //H09_05 离线惯量辨识模式
    Uint16 InertiaSpdMax;               //H09_06 惯量辨识时到达的最大速度
    Uint16 InertiaAcceTime ;            //H09_07 惯量辨识时加速至最大速度时间
    Uint16 InertiaWaitTime;             //H09_08 每次惯量辨识之后的等待时间
    Uint16 InertiaIdy_Circle;           //H09_09 完整单次惯量辨识转动圈数
    Uint16 AT_RespnLevel;               //H09_10 一键式调整整响应等级
    Uint16 AT_VibThrshld;               //H09_11 一键式调整振动判定阈值
    Uint16 AT_NotchFiltFreqA;           //H09_12 第1组陷波器频率
    Uint16 AT_NotchFiltBandWidthA;      //H09_13 第1组陷波器带宽
    Uint16 AT_NotchFiltAttenuatLvlA;    //H09_14 第1组陷波器衰减等级
    Uint16 AT_NotchFiltFreqB;           //H09_15 第2组陷波器频率
    Uint16 AT_NotchFiltBandWidthB;      //H09_16 第2组陷波器带宽
    Uint16 AT_NotchFiltAttenuatLvlB;    //H09_17 第2组陷波器衰减等级
    Uint16 AT_NotchFiltFreqC;           //H09_18 第3组陷波器频率
    Uint16 AT_NotchFiltBandWidthC;      //H09_19 第3组陷波器带宽
    Uint16 AT_NotchFiltAttenuatLvlC;    //H09_20 第3组陷波器衰减等级
    Uint16 AT_NotchFiltFreqD;           //H09_21 第4组陷波器频率
    Uint16 AT_NotchFiltBandWidthD;      //H09_22 第4组陷波器带宽
    Uint16 AT_NotchFiltAttenuatLvlD;    //H09_23 第4组陷波器衰减等级
    Uint16 AT_NotchFiltFreqDisp;        //H09_24 共振频率辨识结果
    Uint16 AT_LoadMode;                 //H09_25 一键式调整时外部负载运行模式
    Uint16 AT_Rsvd26;                   //H09_26 
    Uint16 AT_Rsvd27;                   //H09_27
    Uint16 AT_Rsvd28;                   //H09_28
    Uint16 AT_Rsvd29;                   //H09_29
    Uint16 AT_ToqDisKp;                 //H09_30 扰动转矩补偿增益
    Uint16 AT_DobFiltTime;              //H09_31 扰动观测器滤波时间
    Uint16 AT_ConstToqComp;              //H09_32 恒定转矩补偿值
    Uint16 AT_ToqPlusComp;               //H09_33 正向摩擦补偿
    Uint16 AT_ToqMinusComp;              //H09_34 反向摩擦补偿
    Uint16 AT_Rsvd35;                    //H09_35 
    Uint16 AT_Rsvd36;                    //H09_36 
    Uint16 AT_Rsvd37;                   //H09_37  
    Uint16 AT_LowOscFreqA;              //H09_38 低频共振频率A
    Uint16 AT_LowOscFiltA;              //H09_39 低频抑制宽度
    Uint16 AT_LowFreqRatio;             //H09_40 低频抖动分母频率和分子频率比
    Uint16 AT_Rsvd41;                   //H09_41
    Uint16 AT_Rsvd42;                   //H09_42
    Uint16 AT_Rsvd43;                   //H09_43
    Uint16 AT_Rsvd44;                   //H09_44
    Uint16 AT_Rsvd45;                   //H09_45
    Uint16 AT_Rsvd46;                   //H09_46
    Uint16 AT_Rsvd47;                   //H09_47
    Uint16 AT_Rsvd48;                   //H09_48
    Uint16 AT_Rsvd49;                   //H09_49
    Uint16 AT_Rsvd50;                   //H09_50
    Uint16 AT_Rsvd51;                   //H09_51
    Uint16 AT_Rsvd52;                   //H09_52
    Uint16 AT_Rsvd53;                   //H09_53
    Uint16 AT_Rsvd54;                   //H09_54
    Uint16 AT_Rsvd55;                   //H09_55
    Uint16 AT_Rsvd56;                   //H09_56
    Uint16 AT_Rsvd57;                   //H09_57
    Uint16 AT_Rsvd58;                   //H09_58
    Uint16 MemCheck09;                  //H09_59        组校验字

//0726--------------故障与保护参数  H0ALEN = 40 -------------------------------------------------------
    /*等做故障处理时要重新整理该组功能码*/
    Uint16 ER_InPLProtectSel;           //H0A00 电源输入缺相保护选择
    Uint16 AbsPosActSet;                //H0A01 软件位置限制设置
    Uint16 ER_Rsvd02;                   //H0A02 模块温度警告点    (删掉)
    Uint16 ER_PowerOffSaveToEeprom;     //H0A03 是否执行掉电保存EEPROM,msx111203
    Uint16 ER_MotOLProtect_Kp;          //H0A04 电机过载保护增益
    Uint16 ER_AngCnt;                   //H0A05 角度辨识时电周期分成的份数——不开放
    Uint16 ER_MotorLoadRate;            //H0A06 电机过载等级
    Uint16 ER_UVWIdenEn;                //H0A07 UVW相序辨识使能
    Uint16 ER_OvrSpdErr;                //H0A08 过速判断阀值
    Uint16 ER_MaxPulsFreq;              //H0A09 位置最大脉冲输入频率
    Uint16 ER_PerrFaultVluLow;          //H0A10 位置偏差过大故障设定值低16位
    Uint16 ER_PerrFaultVluHigh;         //H0A11 位置偏差过大故障设定值高16位
    Uint16 ER_RunAwaySel;               //H0A12 飞车保护选择 1飞车报警  0 屏蔽飞车报警   默认1
    Uint16 ER_AngIntSel;                //H0A13 初始角度辨识方式选择
    Uint16 ER_DutySearchEn;             //H0A14 电压注入辨识时占空比搜索使能
    Uint16 ER_MotRunPuls;               //H0A15 电机转动判定阈值——用于不找Z的角度辨识
    Uint16 ER_LowOscPuls;               //H0A16 伺服低频振动位置偏差判断阈值
    Uint16 ER_BlkOverCurrent;           //H0A17	制动电流百分比
    Uint16 ER_DriverTempErr;            //H0A18 驱动器过温保护点
    Uint16 ER_Di8FiltTm;                //H0A19 DI8滤波时间
    Uint16 ER_Di9FiltTm;                //H0A20 DI9滤波时间
    Uint16 ER_StoFiltTm;                //H0A21 STO信号滤波时间
    Uint16 ER_SdmFiltTm;                //H0A22 Sigma_Delta滤波时间
    Uint16 ER_GkFiltTm;                 //H0A23 TZ信号滤波时间
    Uint16 ER_PosCmdFiltTmLow;          //H0A24 低速输入脉冲滤波时间
    Uint16 ER_SpdDispFilt;              //H0A25 速度显示滤波时间
    Uint16 ER_MTOLClose;                //H0A26 关闭过载警告
    Uint16 ER_SpdDoFilt;                //H0A27 速度Do滤波时间
    Uint16 ER_RotEncFiltTm;             //H0A28 正交编码器滤波时间
    Uint16 ER_LnrEncFiltTm;             //H0A29 直线编码器滤波时间
    Uint16 ER_PosCmdFiltTmHigh;         //H0A30 高速速输入脉冲滤波时间
    Uint16 NKAbsEncRstPwrOn;            //H0A31 尼康编码器上电复位
    Uint16 ER_LockedRotorTimer;         //H0A32 堵转过温保护时间窗口
    Uint16 ER_LockedRotorSel;           //H0A33 堵转过温保护使能
    Uint16 EncFrameMode;                //H0A34 2代编码器帧格式
    Uint16 DisRdEncEepromPwrOn;         //H0A35 上电禁止读取编码器EEPROM
    Uint16 EncMultOvDisable;            //H0A36 编码器多圈溢出故障禁止
	#if DRIVER_TYPE == SERVO_650N
    Uint16 ER_UseInProduce;             //H0A37 生产参数
	#else
	 Uint16 ER_Rsvd37;                   //H0A37
	#endif
    Uint16 ER_Rsvd38;                   //H0A38
    Uint16 MemCheck0A;                  //H0A39 组校验字

//0766--------------(MODBUS CAN)通信参数  H0CLEN = 50 -------------------------------------------------------
    Uint16 CM_AxisAdress;               //H0C_00 轴/驱动器地址
    Uint16 CM_Rsvd01;                   //H0C_01
    Uint16 CM_BodeRate;                 //H0C_02 RS232波特率设置
    Uint16 CM_Parity;                   //H0C_03 奇偶校验设置
    Uint16 CM_StationAdd;               //H0C_04 EtherCAT从站站点正名
    Uint16 CM_StationAlias;             //H0C_05 EtherCAT从站站点别名
    Uint16 CM_Rsvd06;                   //H0C_06
    Uint16 CM_Rsvd07;                   //H0C_07
    Uint16 CM_CanSpdSel;                //H0C_08 CAN通信速率选择
    Uint16 CM_UseVDI;                   //H0C_09 是否使用VDI
    Uint16 CM_VDIDefaultValue;          //H0C_10 上电后VDI默认值
    Uint16 CM_UseVDO;                   //H0C_11 是否使用VDO
    Uint16 CM_VDODefaultValue;          //H0C_12 VDO功能选择为0时默认值
    Uint16 CM_WriteEepromEnable;        //H0C_13 写Eeprom开关
    Uint16 CM_ErrorType;                //H0C_14 区分错误码类型bywshp1013
    Uint16 CM_Rsvd15;                   //H0C_15
    Uint16 CM_Rsvd16;                   //H0C_16
    Uint16 CM_Rsvd17;                   //H0C_17
    Uint16 CM_Rsvd18;                   //H0C_18
    Uint16 CM_Rsvd19;                   //H0C_19
    Uint16 CM_Rsvd20;                   //H0C_20
    Uint16 CM_Rsvd21;                   //H0C_21
    Uint16 CM_Rsvd22;                   //H0C_22
    Uint16 CM_Rsvd23;                   //H0C_23
    Uint16 CM_Rsvd24;                   //H0C_24
    Uint16 CM_SendDelay;                //H0C_25 MODBUS应答发送延时
    Uint16 CM_Modbus32BitsSeq;          //H0C_26 32位高低位次序选择
    Uint16 CM_Rsvd27;                   //H0C_27
    Uint16 CM_Rsvd28;                   //H0C_28
    Uint16 CM_Rsvd29;                   //H0C_29
    Uint16 CM_ModbusErrFrameType;       //H0C_30 错误帧类型  1标准  0兼容以前
    Uint16 ModbusRcvDeal;               //H0C_31 Modbus接收处理 0 使能接收中断 1电流环中断查询
    Uint16 CM_XmlVersion;               //H0C_32 XML版本
    Uint16 CM_AlStatusCode;             //H0C_33 AL状态码
    Uint16 CM_SyncErrCnt0;              //H0C_34 同步丢失次数
    Uint16 CM_SyncErrCnt;               //H0C_35 EtherCAT同步中断丢失允许次数
    Uint16 CM_Port0CRC;                 //H0C_36 端口0  无效帧
    Uint16 CM_Port1CRC;                 //H0C_37 端口1  无效帧
    Uint16 CM_ForwardErr;               //H0C_38 端口0、1 转发错误
    Uint16 CM_HandleErr;                //H0C_39 处理单元和PDI 错误
    Uint16 CM_LinkLost;                 //H0C_40 端口0、1 端口丢失
    Uint16 CM_ECATHost;                 //H0C_41 EtherCAT主站选择 非标功能，为匹配各家的MC模块
    Uint16 CM_SyncDetecMethod;          //H0C_42 同步检测机制
    Uint16 CM_FpgaSyncModeSel;          //H0C_43 FPGA同步模式选择  0-自同步 1-OP模式下 载波与SYNC同步
    Uint16 CM_SyncErrThreshold;         //H0C_44 FPGA同步检测偏差阈值
    Uint16 CM_PosBuffEnale;             //H0C_45 位置缓存选择 0--无缓存  1--1个缓存
    Uint16 CM_CSPCmdIncErrCnt;          //H0C_46 CSP位置指令增量过大阈值
    Uint16 CM_CSPCmdErrCnt;             //H0C_47 CSP位置指令增量过大次数
    Uint16 CM_ESM;                      //H0C_48 EtherCAT状态机
    Uint16 MemCheck0C;                  //H0C_49        组校验字

//0816--------------预留参数组  H0ELEN = 50 ------------------------------------------------------- 
    Uint16 H0E_Rsvd00;                  //H0E_00
    Uint16 H0E_Rsvd01;                  //H0E_01
    Uint16 H0E_Rsvd02;                  //H0E_02
    Uint16 H0E_Rsvd03;                  //H0E_03
    Uint16 H0E_Rsvd04;                  //H0E_04
    Uint16 H0E_Rsvd05;                  //H0E_05
    Uint16 H0E_Rsvd06;                  //H0E_06
    Uint16 H0E_Rsvd07;                  //H0E_07
    Uint16 H0E_Rsvd08;                  //H0E_08
    Uint16 H0E_Rsvd09;                  //H0E_09
    Uint16 H0E_Rsvd10;                  //H0E_10
    Uint16 H0E_Rsvd11;                  //H0E_11
    Uint16 H0E_Rsvd12;                  //H0E_12
    Uint16 H0E_Rsvd13;                  //H0E_13
    Uint16 H0E_Rsvd14;                  //H0E_14
    Uint16 H0E_Rsvd15;                  //H0E_15
    Uint16 H0E_Rsvd16;                  //H0E_16
    Uint16 H0E_Rsvd17;                  //H0E_17
    Uint16 H0E_Rsvd18;                  //H0E_18
    Uint16 H0E_Rsvd19;                  //H0E_19
    Uint16 H0E_Rsvd20;                  //H0E_20
    Uint16 H0E_Rsvd21;                  //H0E_21
    Uint16 H0E_Rsvd22;                  //H0E_22
    Uint16 H0E_Rsvd23;                  //H0E_23
    Uint16 H0E_Rsvd24;                  //H0E_24
    Uint16 H0E_Rsvd25;                  //H0E_25
    Uint16 H0E_Rsvd26;                  //H0E_26
    Uint16 H0E_Rsvd27;                  //H0E_27
    Uint16 H0E_Rsvd28;                  //H0E_28
    Uint16 H0E_Rsvd29;                  //H0E_29
    Uint16 H0E_Rsvd30;                  //H0E_30
    Uint16 H0E_Rsvd31;                  //H0E_31
    Uint16 H0E_Rsvd32;                  //H0E_32
    Uint16 H0E_Rsvd33;                  //H0E_33
    Uint16 H0E_Rsvd34;                  //H0E_34
    Uint16 H0E_Rsvd35;                  //H0E_35
    Uint16 H0E_Rsvd36;                  //H0E_36
    Uint16 H0E_Rsvd37;                  //H0E_37
    Uint16 H0E_Rsvd38;                  //H0E_38
    Uint16 H0E_Rsvd39;                  //H0E_39
    Uint16 H0E_Rsvd40;                  //H0E_40
    Uint16 H0E_Rsvd41;                  //H0E_41
    Uint16 H0E_Rsvd42;                  //H0E_42
    Uint16 H0E_Rsvd43;                  //H0E_43
    Uint16 H0E_Rsvd44;                  //H0E_44
    Uint16 H0E_Rsvd45;                  //H0E_45
    Uint16 H0E_Rsvd46;                  //H0E_46
    Uint16 H0E_Rsvd47;                  //H0E_47
    Uint16 H0E_Rsvd48;                  //H0E_48 
    Uint16 MemCheck0E;                  //H0E_49     组校验字 

//0866--------------预留参数组  H0FLEN = 50 ------------------------------------------------------- 
    Uint16 FC_FeedbackMode;            //H0F_00 编码器反馈模式
    Uint16 FC_ExCoderDir;              //H0F_01 外部编码器使用方式
    Uint16 FC_ExCoderLine_L;           //H0F_02 外部编码器线数
    Uint16 FC_ExCoderLine_H;           //H0F_03外部编码器线数
    Uint16 FC_ExCoderPulse_L;          //H0F_04 电机旋转一圈外部编码器脉冲数
    Uint16 FC_ExCoderPulse_H;          //H0F_05 电机旋转一圈外部编码器脉冲数    Uint16 FC_Rsvd05;                  //H0F_05
    Uint16 FC_ExKpCoff;                //H0F_06,外部增益系数
    Uint16 FC_Rsvd07;                  //H0F_07
    Uint16 FC_MixCtrlMaxPulse_L;       //H0F_08 混合控制偏差最大值
    Uint16 FC_MixCtrlMaxPulse_H;       //H0F_09
    Uint16 FC_MixCtrlPulseClr;         //H0F_10 混合控制偏差清除设定
    Uint16 FC_Rsvd11;                  //H0F_11
    Uint16 FC_Rsvd12;                  //H0F_12
    Uint16 FC_ExInErrFilterTime;       //H0F_13 外内偏差一阶低通滤波时间
    Uint16 FC_Rsvd14;                  //H0F_14
    Uint16 FC_Rsvd15;                  //H0F_15
	Uint16 FC_ExInPosErrSum_Lo;        // H0F.16 全闭环外部位置误差低位(显示用)
	Uint16 FC_ExInPosErrSum_Hi;        // H0F.17 全闭环外部位置误差高位(显示用)
    Uint16 FC_InnerPulseFeedback_L;    //H0F_18  内部编码器反馈值
    Uint16 FC_InnerPulseFeedback_H;    //H0F_19
    Uint16 FC_ExPulseFeedback_L;       //H0F_20 外部编码器反馈值
    Uint16 FC_ExPulseFeedback_H;       //H0F_21
    Uint16 H0F_Rsvd22;                  //H0F_22
    Uint16 H0F_Rsvd23;                  //H0F_23
    Uint16 H0F_Rsvd24;                  //H0F_24
    Uint16 FC_TouchSet;                 //H0F_25 探针扩展设置
    Uint16 H0F_Rsvd26;                  //H0F_26
    Uint16 H0F_Rsvd27;                  //H0F_27
    Uint16 H0F_Rsvd28;                  //H0F_28
    Uint16 H0F_Rsvd29;                  //H0F_29
    Uint16 H0F_Rsvd30;                  //H0F_30
    Uint16 H0F_Rsvd31;                  //H0F_31
    Uint16 H0F_Rsvd32;                  //H0F_32
    Uint16 H0F_Rsvd33;                  //H0F_33
    Uint16 H0F_Rsvd34;                  //H0F_34
    Uint16 H0F_Rsvd35;                  //H0F_35
    Uint16 H0F_Rsvd36;                  //H0F_36
    Uint16 H0F_Rsvd37;                  //H0F_37
    Uint16 H0F_Rsvd38;                  //H0F_38
    Uint16 H0F_Rsvd39;                  //H0F_39
    Uint16 H0F_Rsvd40;                  //H0F_40
    Uint16 H0F_Rsvd41;                  //H0F_41
    Uint16 H0F_Rsvd42;                  //H0F_42
    Uint16 H0F_Rsvd43;                  //H0F_43
    Uint16 H0F_Rsvd44;                  //H0F_44
    Uint16 H0F_Rsvd45;                  //H0F_45
    Uint16 H0F_Rsvd46;                  //H0F_46
    Uint16 H0F_Rsvd47;                  //H0F_47
    Uint16 H0F_Rsvd48;                  //H0F_48 
    Uint16 MemCheck0F;                  //H0F_49     组校验字

//0916--------------预留参数组  H10LEN = 50 ------------------------------------------------------- 
    Uint16 SF_PID1RefSel;               //H10_00 PID1控制器指令源 index: 871
    Uint16 SF_PID1FdbSel;               //H10_01 PIDx反馈源
    Uint16 SF_PID1KeyRef;               //H10_02 PIDx键盘设定值
    Uint16 SF_PID1ActDir;               //H10_03 PIDx作用方向 0-正方向
    Uint16 SF_PID1FdbRang_Lo;           //H10_04 PIDx给定反馈量程 L
    Uint16 SF_PID1FdbRang_Hi;           //H10_05 PIDx给定反馈量程 H
    Uint16 SF_PID1EnSel_Rsvd;           //H10_06 PIDx使能开关  保留
    Uint16 SF_Rsvd07;                   //H10_07
    Uint16 SF_Rsvd08;                   //H10_08
    Uint16 SF_Rsvd09;                   //H10_09
    Uint16 SF_PID1_Kp;                  //H10_10 PIDxProportional Gain 0.1HZ
    Uint16 SF_PID1_Ti;                  //H10_11 PIDx积分时间 ms
    Uint16 SF_PID1_Td;                  //H10_12 PIDx微分时间 ms
    Uint16 SF_PID1_MaxOut;              //H10_13
    Uint16 SF_Rsvd14;                   //H10_14
    Uint16 SF_PID1_InverFc;             //H10_15 PIDx反转截止频率
    Uint16 SF_PID1_ErrLmt;              //H10_16 PIDx偏差极限
    Uint16 SF_PID1_IntegLmt;            //H10_17 PIDx积分极限
    Uint16 SF_PID1_DiffLmt;             //H10_18 PIDx微分极限
    Uint16 SF_Rsvd19;                   //H10_19
    Uint16 SF_PID2RefSel;               //H10_20 PID2控制器指令源 index: 871
    Uint16 SF_PID2FdbSel;               //H10_21 PIDx反馈源
    Uint16 SF_PID2KeyRef;               //H10_22 PIDx键盘设定值
    Uint16 SF_PID2ActDir;               //H10_23 PIDx作用方向 0-正方向
    Uint16 SF_PID2FdbRang_Lo;           //H10_24 PIDx给定反馈量程 L
    Uint16 SF_PID2FdbRang_Hi;           //H10_25 PIDx给定反馈量程 H
    Uint16 SF_PID2EnSel_Rsvd;           //H10_26 PIDx使能开关  保留
    Uint16 SF_Rsvd27;                   //H10_27
    Uint16 SF_Rsvd28;                   //H10_28
    Uint16 SF_Rsvd29;                   //H10_29
    Uint16 SF_PID2_Kp;                  //H10_30 PIDxProportional Gain
    Uint16 SF_PID2_Ti;                  //H10_31 PIDx积分时间
    Uint16 SF_PID2_Td;                  //H10_32 PIDx微分时间
    Uint16 SF_Rsvd33;                   //H10_33
    Uint16 SF_Rsvd34;                   //H10_34
    Uint16 SF_PID2_InverFc;             //H10_35 PIDx反转截止频率
    Uint16 SF_PID2_ErrLmt;              //H10_36 PIDx偏差极限
    Uint16 SF_PID2_IntegLmt;            //H10_37 PIDx积分极限
    Uint16 SF_PID2_DiffLmt;             //H10_38 PIDx微分极限
    Uint16 SF_Rsvd39;                   //H10_39
    Uint16 SF_Rsvd40;                   //H10_40
    Uint16 SF_Rsvd41;                   //H10_41
    Uint16 SF_Rsvd42;                   //H10_42
    Uint16 SF_Rsvd43;                   //H10_43
    Uint16 SF_Rsvd44;                   //H10_44
    Uint16 SF_Rsvd45;                   //H10_45
    Uint16 SF_Rsvd46;                   //H10_46
    Uint16 SF_Rsvd47;                   //H10_47
    Uint16 SF_Rsvd48;                   //H10_48
    Uint16 MemCheck10;                  //H10_49         组校验字

//0966--------------多段位置参数  H11LEN = 100 ------------------------------------------------------- 
    Uint16 MP_PosRunMode;               //H11_00 多段位置运行方式
    Uint16 MP_PosExeSects;              //H11_01 位移执行段数选择 起始点为第一段
    Uint16 MP_RemdPosDealFlg;           //H11_02 余量处理方式
    Uint16 MP_PosTimeUnit;              //H11_03 等待时间单位
    Uint16 MP_PosRefType;               //H11_04 位移指令类型选择
    Uint16 MP_Mod4RecyStartSect;        //H11_05 循环模式起始段选择
    Uint16 MP_Rsvd06;                   //H11_06
    Uint16 MP_Rsvd07;                   //H11_07
    Uint16 MP_Rsvd08;                   //H11_08
    Uint16 MP_Rsvd09;                   //H11_09
    Uint16 MP_Rsvd10;                   //H11_10
    Uint16 MP_Rsvd11;                   //H11_11
    Uint16 MP_SEC1_PosRef_Lo;           //H11_12 第1段移动位移L
    Uint16 MP_SEC1_PosRef_Hi;           //H11_13 第1段移动位移H
    Uint16 MP_SEC1_MoveSpd;             //H11_14 第1段移动速度
    Uint16 MP_SEC1_AccTime;             //H11_15 第1段移动加减速时间
    Uint16 MP_SEC1_WaitTime;            //H11_16 第1段位移完成后等待时间
    Uint16 MP_SEC2_PosRef_Lo;           //H11_17 第2段移动位移L
    Uint16 MP_SEC2_PosRef_Hi;           //H11_18 第2段移动位移H
    Uint16 MP_SEC2_MoveSpd;             //H11_19 第2段移动速度
    Uint16 MP_SEC2_AccTime;             //H11_20 第2段移动加减速时间
    Uint16 MP_SEC2_WaitTime;            //H11_21 第2段位移完成后等待时间
    Uint16 MP_SEC3_PosRef_Lo;           //H11_22 第3段移动位移L
    Uint16 MP_SEC3_PosRef_Hi;           //H11_23 第3段移动位移H
    Uint16 MP_SEC3_MoveSpd;             //H11_24 第3段移动速度
    Uint16 MP_SEC3_AccTime;             //H11_25 第3段移动加减速时间
    Uint16 MP_SEC3_WaitTime;            //H11_26 第3段位移完成后等待时间
    Uint16 MP_SEC4_PosRef_Lo;           //H11_27 第4段移动位移L
    Uint16 MP_SEC4_PosRef_Hi;           //H11_28 第4段移动位移H
    Uint16 MP_SEC4_MoveSpd;             //H11_29 第4段移动速度
    Uint16 MP_SEC4_AccTime;             //H11_30 第4段移动加减速时间
    Uint16 MP_SEC4_WaitTime;            //H11_31 第4段位移完成后等待时间
    Uint16 MP_SEC5_PosRef_Lo;           //H11_32 第5段移动位移L
    Uint16 MP_SEC5_PosRef_Hi;           //H11_33 第5段移动位移H
    Uint16 MP_SEC5_MoveSpd;             //H11_34 第5段移动速度
    Uint16 MP_SEC5_AccTime;             //H11_35 第5段移动加减速时间
    Uint16 MP_SEC5_WaitTime;            //H11_36 第5段位移完成后等待时间
    Uint16 MP_SEC6_PosRef_Lo;           //H11_37 第6段移动位移L
    Uint16 MP_SEC6_PosRef_Hi;           //H11_38 第6段移动位移H
    Uint16 MP_SEC6_MoveSpd;             //H11_39 第6段移动速度
    Uint16 MP_SEC6_AccTime;             //H11_40 第6段移动加减速时间
    Uint16 MP_SEC6_WaitTime;            //H11_41 第6段位移完成后等待时间
    Uint16 MP_SEC7_PosRef_Lo;           //H11_42 第7段移动位移L
    Uint16 MP_SEC7_PosRef_Hi;           //H11_43 第7段移动位移H
    Uint16 MP_SEC7_MoveSpd;             //H11_44 第7段移动速度
    Uint16 MP_SEC7_AccTime;             //H11_45 第7段移动加减速时间
    Uint16 MP_SEC7_WaitTime;            //H11_46 第7段位移完成后等待时间
    Uint16 MP_SEC8_PosRef_Lo;           //H11_47 第8段移动位移L
    Uint16 MP_SEC8_PosRef_Hi;           //H11_48 第8段移动位移H
    Uint16 MP_SEC8_MoveSpd;             //H11_49 第8段移动速度
    Uint16 MP_SEC8_AccTime;             //H11_50 第8段移动加减速时间
    Uint16 MP_SEC8_WaitTime;            //H11_51 第8段位移完成后等待时间
    Uint16 MP_SEC9_PosRef_Lo;           //H11_52 第9段移动位移L
    Uint16 MP_SEC9_PosRef_Hi;           //H11_53 第9段移动位移H
    Uint16 MP_SEC9_MoveSpd;             //H11_54 第9段移动速度
    Uint16 MP_SEC9_AccTime;             //H11_55 第9段移动加减速时间
    Uint16 MP_SEC9_WaitTime;            //H11_56 第9段位移完成后等待时间
    Uint16 MP_SEC10_PosRef_Lo;          //H11_57 第10段移动位移L
    Uint16 MP_SEC10_PosRef_Hi;          //H11_58 第10段移动位移H
    Uint16 MP_SEC10_MoveSpd;            //H11_59 第10段移动速度
    Uint16 MP_SEC10_AccTime;            //H11_60 第10段移动加减速时间
    Uint16 MP_SEC10_WaitTime;           //H11_61 第10段位移完成后等待时间
    Uint16 MP_SEC11_PosRef_Lo;          //H11_62 第11段移动位移L
    Uint16 MP_SEC11_PosRef_Hi;          //H11_63 第11段移动位移H
    Uint16 MP_SEC11_MoveSpd;            //H11_64 第11段移动速度
    Uint16 MP_SEC11_AccTime;            //H11_65 第11段移动加减速时间
    Uint16 MP_SEC11_WaitTime;           //H11_66 第11段位移完成后等待时间
    Uint16 MP_SEC12_PosRef_Lo;          //H11_67 第12段移动位移L
    Uint16 MP_SEC12_PosRef_Hi;          //H11_68 第12段移动位移H
    Uint16 MP_SEC12_MoveSpd;            //H11_69 第12段移动速度
    Uint16 MP_SEC12_AccTime;            //H11_70 第12段移动加减速时间
    Uint16 MP_SEC12_WaitTime;           //H11_71 第12段位移完成后等待时间
    Uint16 MP_SEC13_PosRef_Lo;          //H11_72 第13段移动位移L
    Uint16 MP_SEC13_PosRef_Hi;          //H11_73 第13段移动位移H
    Uint16 MP_SEC13_MoveSpd;            //H11_74 第13段移动速度
    Uint16 MP_SEC13_AccTime;            //H11_75 第13段移动加减速时间
    Uint16 MP_SEC13_WaitTime;           //H11_76 第13段位移完成后等待时间
    Uint16 MP_SEC14_PosRef_Lo;          //H11_77 第14段移动位移L
    Uint16 MP_SEC14_PosRef_Hi;          //H11_78 第14段移动位移H
    Uint16 MP_SEC14_MoveSpd;            //H11_79 第14段移动速度
    Uint16 MP_SEC14_AccTime;            //H11_80 第14段移动加减速时间
    Uint16 MP_SEC14_WaitTime;           //H11_81 第14段位移完成后等待时间
    Uint16 MP_SEC15_PosRef_Lo;          //H11_82 第15段移动位移L
    Uint16 MP_SEC15_PosRef_Hi;          //H11_83 第15段移动位移H
    Uint16 MP_SEC15_MoveSpd;            //H11_84 第15段移动速度
    Uint16 MP_SEC15_AccTime;            //H11_85 第15段移动加减速时间
    Uint16 MP_SEC15_WaitTime;           //H11_86 第15段位移完成后等待时间
    Uint16 MP_SEC16_PosRef_Lo;          //H11_87 第16段移动位移L
    Uint16 MP_SEC16_PosRef_Hi;          //H11_88 第16段移动位移H
    Uint16 MP_SEC16_MoveSpd;            //H11_89 第16段移动速度
    Uint16 MP_SEC16_AccTime;            //H11_90 第16段移动加减速时间
    Uint16 MP_SEC16_WaitTime;           //H11_91 第16段位移完成后等待时间
    Uint16 MP_Rsvd92;                   //H11_92
    Uint16 MP_Rsvd93;                   //H11_93
    Uint16 MP_Rsvd94;                   //H11_94
    Uint16 MP_Rsvd95;                   //H11_95
    Uint16 MP_Rsvd96;                   //H11_96
    Uint16 MP_Rsvd97;                   //H11_97
    Uint16 MP_Rsvd98;                   //H11_98
    Uint16 MemCheck11;                  //H11_99            组校验字


//1066--------------多段指令(速度)参数  H12LEN = 70 ------------------------------------------------------- 
    Uint16 MTS_RefRunMode;              //H12_00 多段速度指令运行方式
    Uint16 MTS_ExeSects;                //H12_01 速度指令终点段数选择
    Uint16 MTS_RunTimeUnit;             //H12_02 运行时间单位选择
    Uint16 MTS_RiseTime1;               //H12_03 加速时间1
    Uint16 MTS_FallTime1;               //H12_04 减速时间1
    Uint16 MTS_RiseTime2;               //H12_05 加速时间2
    Uint16 MTS_FallTime2;               //H12_06 减速时间2
    Uint16 MTS_RiseTime3;               //H12_07 加速时间3
    Uint16 MTS_FallTime3;               //H12_08 减速时间3
    Uint16 MTS_RiseTime4;               //H12_09 加速时间4
    Uint16 MTS_FallTime4;               //H12_10 减速时间4
    Uint16 MTS_Rsvd11;                  //H12_11
    Uint16 MTS_Rsvd12;                  //H12_12
    Uint16 MTS_Rsvd13;                  //H12_13
    Uint16 MTS_Rsvd14;                  //H12_14
    Uint16 MTS_Rsvd15;                  //H12_15
    Uint16 MTS_Rsvd16;                  //H12_16
    Uint16 MTS_Rsvd17;                  //H12_17
    Uint16 MTS_Rsvd18;                  //H12_18
    Uint16 MTS_Rsvd19;                  //H12_19
    Uint16 MTS_SEC1_Ref;                //H12_20 第1段指令
    Uint16 MTS_SEC1_RunTime;            //H12_21 第1段运行时间
    Uint16 MTS_SEC1_RiFaSel;            //H12_22 第1段升降速时间
    Uint16 MTS_SEC2_Ref;                //H12_23 第2段指令
    Uint16 MTS_SEC2_RunTime;            //H12_24 第2段运行时间
    Uint16 MTS_SEC2_RiFaSel;            //H12_25 第2段升降速时间
    Uint16 MTS_SEC3_Ref;                //H12_26 第3段指令
    Uint16 MTS_SEC3_RunTime;            //H12_27 第3段运行时间
    Uint16 MTS_SEC3_RiFaSel;            //H12_28 第3段升降速时间
    Uint16 MTS_SEC4_Ref;                //H12_29 第4段指令
    Uint16 MTS_SEC4_RunTime;            //H12_30 第4段运行时间
    Uint16 MTS_SEC4_RiFaSel;            //H12_31 第4段升降速时间
    Uint16 MTS_SEC5_Ref;                //H12_32 第5段指令
    Uint16 MTS_SEC5_RunTime;            //H12_33 第5段运行时间
    Uint16 MTS_SEC5_RiFaSel;            //H12_34 第5段升降速时间
    Uint16 MTS_SEC6_Ref;                //H12_35 第6段指令
    Uint16 MTS_SEC6_RunTime;            //H12_36 第6段运行时间
    Uint16 MTS_SEC6_RiFaSel;            //H12_37 第6段升降速时间
    Uint16 MTS_SEC7_Ref;                //H12_38 第7段指令
    Uint16 MTS_SEC7_RunTime;            //H12_39 第7段运行时间
    Uint16 MTS_SEC7_RiFaSel;            //H12_40 第7段升降速时间
    Uint16 MTS_SEC8_Ref;                //H12_41 第8段指令
    Uint16 MTS_SEC8_RunTime;            //H12_42 第8段运行时间
    Uint16 MTS_SEC8_RiFaSel;            //H12_43 第8段升降速时间
    Uint16 MTS_SEC9_Ref;                //H12_44 第9段指令
    Uint16 MTS_SEC9_RunTime;            //H12_45 第9段运行时间
    Uint16 MTS_SEC9_RiFaSel;            //H12_46 第9段升降速时间
    Uint16 MTS_SEC10_Ref;               //H12_47 第10段指令
    Uint16 MTS_SEC10_RunTime;           //H12_48 第10段运行时间
    Uint16 MTS_SEC10_RiFaSel;           //H12_49 第10段升降速时间
    Uint16 MTS_SEC11_Ref;               //H12_50 第11段指令
    Uint16 MTS_SEC11_RunTime;           //H12_51 第11段运行时间
    Uint16 MTS_SEC11_RiFaSel;           //H12_52 第11段升降速时间
    Uint16 MTS_SEC12_Ref;               //H12_53 第12段指令
    Uint16 MTS_SEC12_RunTime;           //H12_54 第12段运行时间
    Uint16 MTS_SEC12_RiFaSel;           //H12_55 第12段升降速时间
    Uint16 MTS_SEC13_Ref;               //H12_56 第13段指令
    Uint16 MTS_SEC13_RunTime;           //H12_57 第13段运行时间
    Uint16 MTS_SEC13_RiFaSel;           //H12_58 第13段升降速时间
    Uint16 MTS_SEC14_Ref;               //H12_59 第14段指令
    Uint16 MTS_SEC14_RunTime;           //H12_60 第14段运行时间
    Uint16 MTS_SEC14_RiFaSel;           //H12_61 第14段升降速时间
    Uint16 MTS_SEC15_Ref;               //H12_62 第15段指令
    Uint16 MTS_SEC15_RunTime;           //H12_63 第15段运行时间
    Uint16 MTS_SEC15_RiFaSel;           //H12_64 第15段升降速时间
    Uint16 MTS_SEC16_Ref;               //H12_65 第16段指令
    Uint16 MTS_SEC16_RunTime;           //H12_66 第16段运行时间
    Uint16 MTS_SEC16_RiFaSel;           //H12_67 第16段升降速时间
    Uint16 MTS_Rsvd67;                  //H12_68
    Uint16 MemCheck12;                  //H12_69 组校验字


//1136---------------------------- CAN参数组 H13LEN = 50 ------------------------------------------------------- 
    Uint16 H13_Rsvd00;                  //H13_00
    Uint16 TP_Tp1Type;                  //H13_01 Tpdo1类型
    Uint16 TP_Tp1MapObjCnt;             //H13_02 Tpdo1映射对象计数
    Uint16 TP_Tp1MapObj1Low16;          //H13_03 Tpdo1映射对象1低16位
    Uint16 TP_Tp1MapObj1Hi16;           //H13_04 Tpdo1映射对象1高16位
    Uint16 TP_Tp1MapObj2Low16;          //H13_05 Tpdo1映射对象2低16位
    Uint16 TP_Tp1MapObj2Hi16;           //H13_06 Tpdo1映射对象2低16位
    Uint16 TP_Tp1MapObj3Low16;          //H13_07 Tpdo1映射对象3低16位
    Uint16 TP_Tp1MapObj3Hi16;           //H13_08 Tpdo1映射对象3低16位
    Uint16 TP_Tp1MapObj4Low16;          //H13_09 Tpdo1映射对象4低16位
    Uint16 TP_Tp1MapObj4Hi16;           //H13_10 Tpdo1映射对象4低16位
    Uint16 TP_Tp2Type;                  //H13_11 Tpdo2类型
    Uint16 TP_Tp2MapObjCnt;             //H13_12 Tpdo2映射对象计数
    Uint16 TP_Tp2MapObj1Low16;          //H13_13 Tpdo2映射对象1低16位
    Uint16 TP_Tp2MapObj1Hi16;           //H13_14 Tpdo2映射对象1高16位
    Uint16 TP_Tp2MapObj2Low16;          //H13_15 Tpdo2映射对象2低16位
    Uint16 TP_Tp2MapObj2Hi16;           //H13_16 Tpdo2映射对象2低16位
    Uint16 TP_Tp2MapObj3Low16;          //H13_17 Tpdo2映射对象3低16位
    Uint16 TP_Tp2MapObj3Hi16;           //H13_18 Tpdo2映射对象3低16位
    Uint16 TP_Tp2MapObj4Low16;          //H13_19 Tpdo2映射对象4低16位
    Uint16 TP_Tp2MapObj4Hi16;           //H13_20 Tpdo2映射对象4低16位
    Uint16 TP_Tp3Type;                  //H13_21 Tpdo3类型
    Uint16 TP_Tp3MapObjCnt;             //H13_22 Tpdo3映射对象计数
    Uint16 TP_Tp3MapObj1Low16;          //H13_23 Tpdo3映射对象1低16位
    Uint16 TP_Tp3MapObj1Hi16;           //H13_24 Tpdo3映射对象1高16位
    Uint16 TP_Tp3MapObj2Low16;          //H13_25 Tpdo3映射对象2低16位
    Uint16 TP_Tp3MapObj2Hi16;           //H13_26 Tpdo3映射对象2低16位
    Uint16 TP_Tp3MapObj3Low16;          //H13_27 Tpdo3映射对象3低16位
    Uint16 TP_Tp3MapObj3Hi16;           //H13_28 Tpdo3映射对象3低16位
    Uint16 TP_Tp3MapObj4Low16;          //H13_29 Tpdo3映射对象4低16位
    Uint16 TP_Tp3MapObj4Hi16;           //H13_30 Tpdo3映射对象4低16位
    Uint16 TP_Tp4Type;                  //H13_31 Tpdo4类型
    Uint16 TP_Tp4MapObjCnt;             //H13_32 Tpdo4映射对象计数
    Uint16 TP_Tp4MapObj1Low16;          //H13_33 Tpdo4映射对象1低16位
    Uint16 TP_Tp4MapObj1Hi16;           //H13_34 Tpdo4映射对象1高16位
    Uint16 TP_Tp4MapObj2Low16;          //H13_35 Tpdo4映射对象2低16位
    Uint16 TP_Tp4MapObj2Hi16;           //H13_36 Tpdo4映射对象2低16位
    Uint16 TP_Tp4MapObj3Low16;          //H13_37 Tpdo4映射对象3低16位
    Uint16 TP_Tp4MapObj3Hi16;           //H13_38 Tpdo4映射对象3低16位
    Uint16 TP_Tp4MapObj4Low16;          //H13_39 Tpdo4映射对象4低16位
    Uint16 TP_Tp4MapObj4Hi16;           //H13_40 Tpdo4映射对象4低16位
    Uint16 H13_Rsvd41;                  //H13_41
    Uint16 H13_Rsvd42;                  //H13_42
    Uint16 H13_Rsvd43;                  //H13_43
    Uint16 H13_Rsvd44;                  //H13_44
    Uint16 H13_Rsvd45;                  //H13_45
    Uint16 H13_Rsvd46;                  //H13_46
    Uint16 H13_Rsvd47;                  //H13_47
    Uint16 H13_Rsvd48;                  //H13_48
    Uint16 MemCheck13;                  //H13_49           组校验字

//1186---------------------------- CAN参数组  H14LEN = 50 ------------------------------------------------------- 
    Uint16 H14_Rsvd00;                  //H14_00
    Uint16 RP_Rp1Type;                  //H14_01 Rpdo1类型
    Uint16 RP_Rp1MapObjCnt;             //H14_02 Rpdo1映射对象计数
    Uint16 RP_Rp1MapObj1Low16;          //H14_03 Rpdo1映射对象1低16位
    Uint16 RP_Rp1MapObj1Hi16;           //H14_04 Rpdo1映射对象1高16位
    Uint16 RP_Rp1MapObj2Low16;          //H14_05 Rpdo1映射对象2低16位
    Uint16 RP_Rp1MapObj2Hi16;           //H14_06 Rpdo1映射对象2低16位
    Uint16 RP_Rp1MapObj3Low16;          //H14_07 Rpdo1映射对象3低16位
    Uint16 RP_Rp1MapObj3Hi16;           //H14_08 Rpdo1映射对象3低16位
    Uint16 RP_Rp1MapObj4Low16;          //H14_09 Rpdo1映射对象4低16位
    Uint16 RP_Rp1MapObj4Hi16;           //H14_10 Rpdo1映射对象4低16位
    Uint16 RP_Rp2Type;                  //H14_11 Rpdo2类型
    Uint16 RP_Rp2MapObjCnt;             //H14_12 Rpdo2映射对象计数
    Uint16 RP_Rp2MapObj1Low16;          //H14_13 Rpdo2映射对象1低16位
    Uint16 RP_Rp2MapObj1Hi16;           //H14_14 Rpdo2映射对象1高16位
    Uint16 RP_Rp2MapObj2Low16;          //H14_15 Rpdo2映射对象2低16位
    Uint16 RP_Rp2MapObj2Hi16;           //H14_16 Rpdo2映射对象2低16位
    Uint16 RP_Rp2MapObj3Low16;          //H14_17 Rpdo2映射对象3低16位
    Uint16 RP_Rp2MapObj3Hi16;           //H14_18 Rpdo2映射对象3低16位
    Uint16 RP_Rp2MapObj4Low16;          //H14_19 Rpdo2映射对象4低16位
    Uint16 RP_Rp2MapObj4Hi16;           //H14_20 Rpdo2映射对象4低16位
    Uint16 RP_Rp3Type;                  //H14_21 Rpdo3类型
    Uint16 RP_Rp3MapObjCnt;             //H14_22 Rpdo3映射对象计数
    Uint16 RP_Rp3MapObj1Low16;          //H14_23 Rpdo3映射对象1低16位
    Uint16 RP_Rp3MapObj1Hi16;           //H14_24 Rpdo3映射对象1高16位
    Uint16 RP_Rp3MapObj2Low16;          //H14_25 Rpdo3映射对象2低16位
    Uint16 RP_Rp3MapObj2Hi16;           //H14_26 Rpdo3映射对象2低16位
    Uint16 RP_Rp3MapObj3Low16;          //H14_27 Rpdo3映射对象3低16位
    Uint16 RP_Rp3MapObj3Hi16;           //H14_28 Rpdo3映射对象3低16位
    Uint16 RP_Rp3MapObj4Low16;          //H14_29 Rpdo3映射对象4低16位
    Uint16 RP_Rp3MapObj4Hi16;           //H14_30 Rpdo3映射对象4低16位
    Uint16 RP_Rp4Type;                  //H14_31 Rpdo4类型
    Uint16 RP_Rp4MapObjCnt;             //H14_32 Rpdo4映射对象计数
    Uint16 RP_Rp4MapObj1Low16;          //H14_33 Rpdo4映射对象1低16位
    Uint16 RP_Rp4MapObj1Hi16;           //H14_34 Rpdo4映射对象1高16位
    Uint16 RP_Rp4MapObj2Low16;          //H14_35 Rpdo4映射对象2低16位
    Uint16 RP_Rp4MapObj2Hi16;           //H14_36 Rpdo4映射对象2低16位
    Uint16 RP_Rp4MapObj3Low16;          //H14_37 Rpdo4映射对象3低16位
    Uint16 RP_Rp4MapObj3Hi16;           //H14_38 Rpdo4映射对象3低16位
    Uint16 RP_Rp4MapObj4Low16;          //H14_39 Rpdo4映射对象4低16位
    Uint16 RP_Rp4MapObj4Hi16;           //H14_40 Rpdo4映射对象4低16位
    Uint16 H14_Rsvd41;                  //H14_41
    Uint16 H14_Rsvd42;                  //H14_42
    Uint16 H14_Rsvd43;                  //H14_43
    Uint16 H14_Rsvd44;                  //H14_44
    Uint16 H14_Rsvd45;                  //H14_45
    Uint16 H14_Rsvd46;                  //H14_46
    Uint16 H14_Rsvd47;                  //H14_47
    Uint16 H14_Rsvd48;                  //H14_48
    Uint16 MemCheck14;                  //H14_49           组校验字

//1236--------------预留参数组  H15LEN = 50 ------------------------------------------------------- 
    Uint16 H15_Rsvd00;                  //H15_00
    Uint16 H15_Rsvd01;                  //H15_01
    Uint16 H15_Rsvd02;                  //H15_02
    Uint16 H15_Rsvd03;                  //H15_03
    Uint16 H15_Rsvd04;                  //H15_04
    Uint16 H15_Rsvd05;                  //H15_05
    Uint16 H15_Rsvd06;                  //H15_06
    Uint16 H15_Rsvd07;                  //H15_07
    Uint16 H15_Rsvd08;                  //H15_08
    Uint16 H15_Rsvd09;                  //H15_09
    Uint16 H15_Rsvd10;                  //H15_10
    Uint16 H15_Rsvd11;                  //H15_11
    Uint16 H15_Rsvd12;                  //H15_12
    Uint16 H15_Rsvd13;                  //H15_13
    Uint16 H15_Rsvd14;                  //H15_14
    Uint16 H15_Rsvd15;                  //H15_15
    Uint16 H15_Rsvd16;                  //H15_16
    Uint16 H15_Rsvd17;                  //H15_17
    Uint16 H15_Rsvd18;                  //H15_18
    Uint16 H15_Rsvd19;                  //H15_19
    Uint16 H15_Rsvd20;                  //H15_20
    Uint16 H15_Rsvd21;                  //H15_21
    Uint16 H15_Rsvd22;                  //H15_22
    Uint16 H15_Rsvd23;                  //H15_23
    Uint16 H15_Rsvd24;                  //H15_24
    Uint16 H15_Rsvd25;                  //H15_25
    Uint16 H15_Rsvd26;                  //H15_26
    Uint16 H15_Rsvd27;                  //H15_27
    Uint16 H15_Rsvd28;                  //H15_28
    Uint16 H15_Rsvd29;                  //H15_29
    Uint16 H15_Rsvd30;                  //H15_30
    Uint16 H15_Rsvd31;                  //H15_31
    Uint16 H15_Rsvd32;                  //H15_32
    Uint16 H15_Rsvd33;                  //H15_33
    Uint16 H15_Rsvd34;                  //H15_34
    Uint16 H15_Rsvd35;                  //H15_35
    Uint16 H15_Rsvd36;                  //H15_36
    Uint16 H15_Rsvd37;                  //H15_37
    Uint16 H15_Rsvd38;                  //H15_38
    Uint16 H15_Rsvd39;                  //H15_39
    Uint16 H15_Rsvd40;                  //H15_40
    Uint16 H15_Rsvd41;                  //H15_41
    Uint16 H15_Rsvd42;                  //H15_42
    Uint16 H15_Rsvd43;                  //H15_43
    Uint16 H15_Rsvd44;                  //H15_44
    Uint16 H15_Rsvd45;                  //H15_45
    Uint16 H15_Rsvd46;                  //H15_46
    Uint16 H15_Rsvd47;                  //H15_47
    Uint16 H15_Rsvd48;                  //H15_48
    Uint16 MemCheck15;                  //H15_49           组校验字

//1286--------------(CANLINK)预留参数组  H16LEN = 50 ------------------------------------------------------- 
    Uint16 H16_Rsvd00;                  //H16_00
    Uint16 CG_CfgInfo1_1;               //H16_01 配置信息1的第1个字
    Uint16 CG_CfgInfo1_2;               //H16_02 配置信息1的第2个字
    Uint16 CG_CfgInfo1_3;               //H16_03 配置信息1的第3个字
    Uint16 CG_CfgInfo1_4;               //H16_04 配置信息1的第4个字
    Uint16 CG_CfgInfo2_1;               //H16_05 配置信息2的第1个字
    Uint16 CG_CfgInfo2_2;               //H16_06 配置信息2的第2个字
    Uint16 CG_CfgInfo2_3;               //H16_07 配置信息2的第3个字
    Uint16 CG_CfgInfo2_4;               //H16_08 配置信息2的第4个字
    Uint16 CG_CfgInfo3_1;               //H16_09 配置信息3的第1个字
    Uint16 CG_CfgInfo3_2;               //H16_10 配置信息3的第2个字
    Uint16 CG_CfgInfo3_3;               //H16_11 配置信息3的第3个字
    Uint16 CG_CfgInfo3_4;               //H16_12 配置信息3的第4个字
    Uint16 CG_CfgInfo4_1;               //H16_13 配置信息4的第1个字
    Uint16 CG_CfgInfo4_2;               //H16_14 配置信息4的第2个字
    Uint16 CG_CfgInfo4_3;               //H16_15 配置信息4的第3个字
    Uint16 CG_CfgInfo4_4;               //H16_16 配置信息4的第4个字
    Uint16 CG_CfgInfo5_1;               //H16_17 配置信息5的第1个字
    Uint16 CG_CfgInfo5_2;               //H16_18 配置信息5的第2个字
    Uint16 CG_CfgInfo5_3;               //H16_19 配置信息5的第3个字
    Uint16 CG_CfgInfo5_4;               //H16_20 配置信息5的第4个字
    Uint16 CG_CfgInfo6_1;               //H16_21 配置信息6的第1个字
    Uint16 CG_CfgInfo6_2;               //H16_22 配置信息6的第2个字
    Uint16 CG_CfgInfo6_3;               //H16_23 配置信息6的第3个字
    Uint16 CG_CfgInfo6_4;               //H16_24 配置信息6的第4个字
    Uint16 CG_CfgInfo7_1;               //H16_25 配置信息7的第1个字
    Uint16 CG_CfgInfo7_2;               //H16_26 配置信息7的第2个字
    Uint16 CG_CfgInfo7_3;               //H16_27 配置信息7的第3个字
    Uint16 CG_CfgInfo7_4;               //H16_28 配置信息7的第4个字
    Uint16 CG_CfgInfo8_1;               //H16_29 配置信息8的第1个字
    Uint16 CG_CfgInfo8_2;               //H16_30 配置信息8的第2个字
    Uint16 CG_CfgInfo8_3;               //H16_31 配置信息8的第3个字
    Uint16 CG_CfgInfo8_4;               //H16_32 配置信息8的第4个字
    Uint16 CG_CfgInfo9_1;               //H16_33 配置信息9的第1个字
    Uint16 CG_CfgInfo9_2;               //H16_34 配置信息9的第2个字
    Uint16 CG_CfgInfo9_3;               //H16_35 配置信息9的第3个字
    Uint16 CG_CfgInfo9_4;               //H16_36 配置信息9的第4个字
    Uint16 CG_CfgInfo10_1;              //H16_37 配置信息10的第1个字
    Uint16 CG_CfgInfo10_2;              //H16_38 配置信息10的第2个字
    Uint16 CG_CfgInfo10_3;              //H16_39 配置信息10的第3个字
    Uint16 CG_CfgInfo10_4;              //H16_40 配置信息10的第4个字
    Uint16 CG_DelCanCfgInfo;            //H16_41 删除CAN配置信息
    Uint16 CG_CanCfgNum;                //H16_42 CANLINK配置信息个数
    Uint16 H16_Rsvd43;                  //H16_43
    Uint16 H16_Rsvd44;                  //H16_44
    Uint16 H16_Rsvd45;                  //H16_45
    Uint16 H16_Rsvd46;                  //H16_46
    Uint16 H16_Rsvd47;                  //H16_47
    Uint16 H16_Rsvd48;                  //H16_48
    Uint16 MemCheck16;                  //H16_49               组校验字

//1336--------------VDI/VDO参数  H17LEN = 70 ------------------------------------------------------- 
    Uint16 VI_VDIFuncSel1;              //H17_00 VDI1端子功能选择
    Uint16 VI_VDILogicSel1;             //H17_01 VDI1端子逻辑选择
    Uint16 VI_VDIFuncSel2;              //H17_02 VDI2端子功能选择
    Uint16 VI_VDILogicSel2;             //H17_03 VDI2端子逻辑选择
    Uint16 VI_VDIFuncSel3;              //H17_04 VDI3端子功能选择
    Uint16 VI_VDILogicSel3;             //H17_05 VDI3端子逻辑选择
    Uint16 VI_VDIFuncSel4;              //H17_06 VDI4端子功能选择
    Uint16 VI_VDILogicSel4;             //H17_07 VDI4端子逻辑选择
    Uint16 VI_VDIFuncSel5;              //H17_08 VDI5端子功能选择
    Uint16 VI_VDILogicSel5;             //H17_09 VDI5端子逻辑选择
    Uint16 VI_VDIFuncSel6;              //H17_10 VDI6端子功能选择
    Uint16 VI_VDILogicSel6;             //H17_11 VDI6端子逻辑选择
    Uint16 VI_VDIFuncSel7;              //H17_12 VDI7端子功能选择
    Uint16 VI_VDILogicSel7;             //H17_13 VDI7端子逻辑选择
    Uint16 VI_VDIFuncSel8;              //H17_14 VDI8端子功能选择
    Uint16 VI_VDILogicSel8;             //H17_15 VDI8端子逻辑选择
    Uint16 VI_VDIFuncSel9;              //H17_16 VDI9端子功能选择
    Uint16 VI_VDILogicSel9;             //H17_17 VDI9端子逻辑选择
    Uint16 VI_VDIFuncSel10;             //H17_18 VDI10端子功能选择
    Uint16 VI_VDILogicSel10;            //H17_19 VDI10端子逻辑选择
    Uint16 VI_VDIFuncSel11;             //H17_20 VDI11端子功能选择
    Uint16 VI_VDILogicSel11;            //H17_21 VDI11端子逻辑选择
    Uint16 VI_VDIFuncSel12;             //H17_22 VDI12端子功能选择
    Uint16 VI_VDILogicSel12;            //H17_23 VDI12端子逻辑选择
    Uint16 VI_VDIFuncSel13;             //H17_24 VDI13端子功能选择
    Uint16 VI_VDILogicSel13;            //H17_25 VDI13端子逻辑选择
    Uint16 VI_VDIFuncSel14;             //H17_26 VDI14端子功能选择
    Uint16 VI_VDILogicSel14;            //H17_27 VDI14端子逻辑选择
    Uint16 VI_VDIFuncSel15;             //H17_28 VDI15端子功能选择
    Uint16 VI_VDILogicSel15;            //H17_29 VDI15端子逻辑选择
    Uint16 VI_VDIFuncSel16;             //H17_30 VDI16端子功能选择
    Uint16 VI_VDILogicSel16;            //H17_31 VDI16端子逻辑选择
    Uint16 VI_AllVDOLevel;              //H17_32 VDO虚拟电平
    Uint16 VI_VDOFuncSel1;              //H17_33 VDO1端子功能选择
    Uint16 VI_VDOLogicSel1;             //H17_34 VDO1端子逻辑选择
    Uint16 VI_VDOFuncSel2;              //H17_35 VDO2端子功能选择
    Uint16 VI_VDOLogicSel2;             //H17_36 VDO2端子逻辑选择
    Uint16 VI_VDOFuncSel3;              //H17_37 VDO3端子功能选择
    Uint16 VI_VDOLogicSel3;             //H17_38 VDO3端子逻辑选择
    Uint16 VI_VDOFuncSel4;              //H17_39 VDO4端子功能选择
    Uint16 VI_VDOLogicSel4;             //H17_40 VDO4端子逻辑选择
    Uint16 VI_VDOFuncSel5;              //H17_41 VDO5端子功能选择
    Uint16 VI_VDOLogicSel5;             //H17_42 VDO5端子逻辑选择
    Uint16 VI_VDOFuncSel6;              //H17_43 VDO6端子功能选择
    Uint16 VI_VDOLogicSel6;             //H17_44 VDO6端子逻辑选择
    Uint16 VI_VDOFuncSel7;              //H17_45 VDO7端子功能选择
    Uint16 VI_VDOLogicSel7;             //H17_46 VDO7端子逻辑选择
    Uint16 VI_VDOFuncSel8;              //H17_47 VDO8端子功能选择
    Uint16 VI_VDOLogicSel8;             //H17_48 VDO8端子逻辑选择
    Uint16 VI_VDOFuncSel9;              //H17_49 VDO9端子功能选择
    Uint16 VI_VDOLogicSel9;             //H17_50 VDO9端子逻辑选择
    Uint16 VI_VDOFuncSel10;             //H17_51 VDO10端子功能选择
    Uint16 VI_VDOLogicSel10;            //H17_52 VDO10端子逻辑选择
    Uint16 VI_VDOFuncSel11;             //H17_53 VDO11端子功能选择
    Uint16 VI_VDOLogicSel11;            //H17_54 VDO11端子逻辑选择
    Uint16 VI_VDOFuncSel12;             //H17_55 VDO12端子功能选择
    Uint16 VI_VDOLogicSel12;            //H17_56 VDO12端子逻辑选择
    Uint16 VI_VDOFuncSel13;             //H17_57 VDO13端子功能选择
    Uint16 VI_VDOLogicSel13;            //H17_58 VDO13端子逻辑选择
    Uint16 VI_VDOFuncSel14;             //H17_59 VDO14端子功能选择
    Uint16 VI_VDOLogicSel14;            //H17_60 VDO14端子逻辑选择
    Uint16 VI_VDOFuncSel15;             //H17_61 VDO15端子功能选择
    Uint16 VI_VDOLogicSel15;            //H17_62 VDO15端子逻辑选择
    Uint16 VI_VDOFuncSel16;             //H17_63 VDO16端子功能选择
    Uint16 VI_VDOLogicSel16;            //H17_64 VDO16端子逻辑选择
    Uint16 H17_Rsvd65;                  //H17_65
    Uint16 H17_Rsvd66;                  //H17_66
    Uint16 H17_Rsvd67;                  //H17_67
    Uint16 H17_Rsvd68;                  //H17_68
    Uint16 MemCheck17;                  //H17_69                    组校验字

//1406--------------预留参数组  H18LEN = 50 -------------------------------------------------------
    Uint16 H18_Rsvd00;                  //H18_00
    Uint16 H18_Rsvd01;                  //H18_01
    Uint16 H18_Rsvd02;                  //H18_02
    Uint16 H18_Rsvd03;                  //H18_03
    Uint16 H18_Rsvd04;                  //H18_04
    Uint16 H18_Rsvd05;                  //H18_05
    Uint16 H18_Rsvd06;                  //H18_06
    Uint16 H18_Rsvd07;                  //H18_07
    Uint16 H18_Rsvd08;                  //H18_08
    Uint16 H18_Rsvd09;                  //H18_09
    Uint16 H18_Rsvd10;                  //H18_10
    Uint16 H18_Rsvd11;                  //H18_11
    Uint16 H18_Rsvd12;                  //H18_12
    Uint16 H18_Rsvd13;                  //H18_13
    Uint16 H18_Rsvd14;                  //H18_14
    Uint16 H18_Rsvd15;                  //H18_15
    Uint16 H18_Rsvd16;                  //H18_16
    Uint16 H18_Rsvd17;                  //H18_17
    Uint16 H18_Rsvd18;                  //H18_18
    Uint16 H18_Rsvd19;                  //H18_19
    Uint16 H18_Rsvd20;                  //H18_20
    Uint16 H18_Rsvd21;                  //H18_21
    Uint16 H18_Rsvd22;                  //H18_22
    Uint16 H18_Rsvd23;                  //H18_23
    Uint16 H18_Rsvd24;                  //H18_24
    Uint16 H18_Rsvd25;                  //H18_25
    Uint16 H18_Rsvd26;                  //H18_26
    Uint16 H18_Rsvd27;                  //H18_27
    Uint16 H18_Rsvd28;                  //H18_28
    Uint16 H18_Rsvd29;                  //H18_29
    Uint16 H18_Rsvd30;                  //H18_30
    Uint16 H18_Rsvd31;                  //H18_31
    Uint16 H18_Rsvd32;                  //H18_32
    Uint16 H18_Rsvd33;                  //H18_33
    Uint16 H18_Rsvd34;                  //H18_34
    Uint16 H18_Rsvd35;                  //H18_35
    Uint16 H18_Rsvd36;                  //H18_36
    Uint16 H18_Rsvd37;                  //H18_37
    Uint16 H18_Rsvd38;                  //H18_38
    Uint16 H18_Rsvd39;                  //H18_39
    Uint16 H18_Rsvd40;                  //H18_40
    Uint16 H18_Rsvd41;                  //H18_41
    Uint16 H18_Rsvd42;                  //H18_42
    Uint16 H18_Rsvd43;                  //H18_43
    Uint16 H18_Rsvd44;                  //H18_44
    Uint16 H18_Rsvd45;                  //H18_45
    Uint16 H18_Rsvd46;                  //H18_46
    Uint16 H18_Rsvd47;                  //H18_47
    Uint16 H18_Rsvd48;                  //H18_48
    Uint16 MemCheck18;                  //H18_49           组校验字

//1456--------------预留参数组  H19LEN = 50 -------------------------------------------------------
    Uint16 H19_Rsvd00;                  //H19_00
    Uint16 H19_Rsvd01;                  //H19_01
    Uint16 H19_Rsvd02;                  //H19_02
    Uint16 H19_Rsvd03;                  //H19_03
    Uint16 H19_Rsvd04;                  //H19_04
    Uint16 H19_Rsvd05;                  //H19_05
    Uint16 H19_Rsvd06;                  //H19_06
    Uint16 H19_Rsvd07;                  //H19_07
    Uint16 H19_Rsvd08;                  //H19_08
    Uint16 H19_Rsvd09;                  //H19_09
    Uint16 H19_Rsvd10;                  //H19_10
    Uint16 H19_Rsvd11;                  //H19_11
    Uint16 H19_Rsvd12;                  //H19_12
    Uint16 H19_Rsvd13;                  //H19_13
    Uint16 H19_Rsvd14;                  //H19_14
    Uint16 H19_Rsvd15;                  //H19_15
    Uint16 H19_Rsvd16;                  //H19_16
    Uint16 H19_Rsvd17;                  //H19_17
    Uint16 H19_Rsvd18;                  //H19_18
    Uint16 H19_Rsvd19;                  //H19_19
    Uint16 H19_Rsvd20;                  //H19_20
    Uint16 H19_Rsvd21;                  //H19_21
    Uint16 H19_Rsvd22;                  //H19_22
    Uint16 H19_Rsvd23;                  //H19_23
    Uint16 H19_Rsvd24;                  //H19_24
    Uint16 H19_Rsvd25;                  //H19_25
    Uint16 H19_Rsvd26;                  //H19_26
    Uint16 H19_Rsvd27;                  //H19_27
    Uint16 H19_Rsvd28;                  //H19_28
    Uint16 H19_Rsvd29;                  //H19_29
    Uint16 H19_Rsvd30;                  //H19_30
    Uint16 H19_Rsvd31;                  //H19_31
    Uint16 H19_Rsvd32;                  //H19_32
    Uint16 H19_Rsvd33;                  //H19_33
    Uint16 H19_Rsvd34;                  //H19_34
    Uint16 H19_Rsvd35;                  //H19_35
    Uint16 H19_Rsvd36;                  //H19_36
    Uint16 H19_Rsvd37;                  //H19_37
    Uint16 H19_Rsvd38;                  //H19_38
    Uint16 H19_Rsvd39;                  //H19_39
    Uint16 H19_Rsvd40;                  //H19_40
    Uint16 H19_Rsvd41;                  //H19_41
    Uint16 H19_Rsvd42;                  //H19_42
    Uint16 H19_Rsvd43;                  //H19_43
    Uint16 H19_Rsvd44;                  //H19_44
    Uint16 H19_Rsvd45;                  //H19_45
    Uint16 H19_Rsvd46;                  //H19_46
    Uint16 H19_Rsvd47;                  //H19_47
    Uint16 H19_Rsvd48;                  //H19_48
    Uint16 MemCheck19;                  //H19_49           组校验字

//1506--------------预留参数组  H1ALEN = 50 -------------------------------------------------------
    Uint16 H1A_Rsvd00;                  //H1A_00
    Uint16 H1A_Rsvd01;                  //H1A_01
    Uint16 H1A_Rsvd02;                  //H1A_02
    Uint16 H1A_Rsvd03;                  //H1A_03
    Uint16 H1A_Rsvd04;                  //H1A_04
    Uint16 H1A_Rsvd05;                  //H1A_05
    Uint16 H1A_Rsvd06;                  //H1A_06
    Uint16 H1A_Rsvd07;                  //H1A_07
    Uint16 H1A_Rsvd08;                  //H1A_08
    Uint16 H1A_Rsvd09;                  //H1A_09
    Uint16 H1A_Rsvd10;                  //H1A_10
    Uint16 H1A_Rsvd11;                  //H1A_11
    Uint16 H1A_Rsvd12;                  //H1A_12
    Uint16 H1A_Rsvd13;                  //H1A_13
    Uint16 H1A_Rsvd14;                  //H1A_14
    Uint16 H1A_Rsvd15;                  //H1A_15
    Uint16 H1A_Rsvd16;                  //H1A_16
    Uint16 H1A_Rsvd17;                  //H1A_17
    Uint16 H1A_Rsvd18;                  //H1A_18
    Uint16 H1A_Rsvd19;                  //H1A_19
    Uint16 H1A_Rsvd20;                  //H1A_20
    Uint16 H1A_Rsvd21;                  //H1A_21
    Uint16 H1A_Rsvd22;                  //H1A_22
    Uint16 H1A_Rsvd23;                  //H1A_23
    Uint16 H1A_Rsvd24;                  //H1A_24
    Uint16 H1A_Rsvd25;                  //H1A_25
    Uint16 H1A_Rsvd26;                  //H1A_26
    Uint16 H1A_Rsvd27;                  //H1A_27
    Uint16 H1A_Rsvd28;                  //H1A_28
    Uint16 H1A_Rsvd29;                  //H1A_29
    Uint16 H1A_Rsvd30;                  //H1A_30
    Uint16 H1A_Rsvd31;                  //H1A_31
    Uint16 H1A_Rsvd32;                  //H1A_32
    Uint16 H1A_Rsvd33;                  //H1A_33
    Uint16 H1A_Rsvd34;                  //H1A_34
    Uint16 H1A_Rsvd35;                  //H1A_35
    Uint16 H1A_Rsvd36;                  //H1A_36
    Uint16 H1A_Rsvd37;                  //H1A_37
    Uint16 H1A_Rsvd38;                  //H1A_38
    Uint16 H1A_Rsvd39;                  //H1A_39
    Uint16 H1A_Rsvd40;                  //H1A_40
    Uint16 H1A_Rsvd41;                  //H1A_41
    Uint16 H1A_Rsvd42;                  //H1A_42
    Uint16 H1A_Rsvd43;                  //H1A_43
    Uint16 H1A_Rsvd44;                  //H1A_44
    Uint16 H1A_Rsvd45;                  //H1A_45
    Uint16 H1A_Rsvd46;                  //H1A_46
    Uint16 H1A_Rsvd47;                  //H1A_47
    Uint16 H1A_Rsvd48;                  //H1A_48
    Uint16 MemCheck1A;                  //H1A_49           组校验字

//1556--------------预留参数组  H1BLEN = 50 -------------------------------------------------------
     Uint16 H1B_Rsvd00;                 //H1B_00
    Uint16 H1B_Rsvd01;                  //H1B_01
    Uint16 H1B_Rsvd02;                  //H1B_02
    Uint16 H1B_Rsvd03;                  //H1B_03
    Uint16 H1B_Rsvd04;                  //H1B_04
    Uint16 H1B_Rsvd05;                  //H1B_05
    Uint16 H1B_Rsvd06;                  //H1B_06
    Uint16 H1B_Rsvd07;                  //H1B_07
    Uint16 H1B_Rsvd08;                  //H1B_08
    Uint16 H1B_Rsvd09;                  //H1B_09
    Uint16 H1B_Rsvd10;                  //H1B_10
    Uint16 H1B_Rsvd11;                  //H1B_11
    Uint16 H1B_Rsvd12;                  //H1B_12
    Uint16 H1B_Rsvd13;                  //H1B_13
    Uint16 H1B_Rsvd14;                  //H1B_14
    Uint16 H1B_Rsvd15;                  //H1B_15
    Uint16 H1B_Rsvd16;                  //H1B_16
    Uint16 H1B_Rsvd17;                  //H1B_17
    Uint16 H1B_Rsvd18;                  //H1B_18
    Uint16 H1B_Rsvd19;                  //H1B_19
    Uint16 H1B_Rsvd20;                  //H1B_20
    Uint16 H1B_Rsvd21;                  //H1B_21
    Uint16 H1B_Rsvd22;                  //H1B_22
    Uint16 H1B_Rsvd23;                  //H1B_23
    Uint16 H1B_Rsvd24;                  //H1B_24
    Uint16 H1B_Rsvd25;                  //H1B_25
    Uint16 H1B_Rsvd26;                  //H1B_26
    Uint16 H1B_Rsvd27;                  //H1B_27
    Uint16 H1B_Rsvd28;                  //H1B_28
    Uint16 H1B_Rsvd29;                  //H1B_29
    Uint16 H1B_Rsvd30;                  //H1B_30
    Uint16 H1B_Rsvd31;                  //H1B_31
    Uint16 H1B_Rsvd32;                  //H1B_32
    Uint16 H1B_Rsvd33;                  //H1B_33
    Uint16 H1B_Rsvd34;                  //H1B_34
    Uint16 H1B_Rsvd35;                  //H1B_35
    Uint16 H1B_Rsvd36;                  //H1B_36
    Uint16 H1B_Rsvd37;                  //H1B_37
    Uint16 H1B_Rsvd38;                  //H1B_38
    Uint16 H1B_Rsvd39;                  //H1B_39
    Uint16 H1B_Rsvd40;                  //H1B_40
    Uint16 H1B_Rsvd41;                  //H1B_41
    Uint16 H1B_Rsvd42;                  //H1B_42
    Uint16 H1B_Rsvd43;                  //H1B_43
    Uint16 H1B_Rsvd44;                  //H1B_44
    Uint16 H1B_Rsvd45;                  //H1B_45
    Uint16 H1B_Rsvd46;                  //H1B_46
    Uint16 H1B_Rsvd47;                  //H1B_47
    Uint16 H1B_Rsvd48;                  //H1B_48
    Uint16 MemCheck1B;                  //H1B_49           组校验字

//1606--------------预留参数组  H1CLEN = 50 -------------------------------------------------------
    Uint16 H1C_Rsvd00;                  //H1C_00
    Uint16 H1C_Rsvd01;                  //H1C_01
    Uint16 H1C_Rsvd02;                  //H1C_02
    Uint16 H1C_Rsvd03;                  //H1C_03
    Uint16 H1C_Rsvd04;                  //H1C_04
    Uint16 H1C_Rsvd05;                  //H1C_05
    Uint16 H1C_Rsvd06;                  //H1C_06
    Uint16 H1C_Rsvd07;                  //H1C_07
    Uint16 H1C_Rsvd08;                  //H1C_08
    Uint16 H1C_Rsvd09;                  //H1C_09
    Uint16 H1C_Rsvd10;                  //H1C_10
    Uint16 H1C_Rsvd11;                  //H1C_11
    Uint16 H1C_Rsvd12;                  //H1C_12
    Uint16 H1C_Rsvd13;                  //H1C_13
    Uint16 H1C_Rsvd14;                  //H1C_14
    Uint16 H1C_Rsvd15;                  //H1C_15
    Uint16 H1C_Rsvd16;                  //H1C_16
    Uint16 H1C_Rsvd17;                  //H1C_17
    Uint16 H1C_Rsvd18;                  //H1C_18
    Uint16 H1C_Rsvd19;                  //H1C_19
    Uint16 H1C_Rsvd20;                  //H1C_20
    Uint16 H1C_Rsvd21;                  //H1C_21
    Uint16 H1C_Rsvd22;                  //H1C_22
    Uint16 H1C_Rsvd23;                  //H1C_23
    Uint16 H1C_Rsvd24;                  //H1C_24
    Uint16 H1C_Rsvd25;                  //H1C_25
    Uint16 H1C_Rsvd26;                  //H1C_26
    Uint16 H1C_Rsvd27;                  //H1C_27
    Uint16 H1C_Rsvd28;                  //H1C_28
    Uint16 H1C_Rsvd29;                  //H1C_29
    Uint16 H1C_Rsvd30;                  //H1C_30
    Uint16 H1C_Rsvd31;                  //H1C_31
    Uint16 H1C_Rsvd32;                  //H1C_32
    Uint16 H1C_Rsvd33;                  //H1C_33
    Uint16 H1C_Rsvd34;                  //H1C_34
    Uint16 H1C_Rsvd35;                  //H1C_35
    Uint16 H1C_Rsvd36;                  //H1C_36
    Uint16 H1C_Rsvd37;                  //H1C_37
    Uint16 H1C_Rsvd38;                  //H1C_38
    Uint16 H1C_Rsvd39;                  //H1C_39
    Uint16 H1C_Rsvd40;                  //H1C_40
    Uint16 H1C_Rsvd41;                  //H1C_41
    Uint16 H1C_Rsvd42;                  //H1C_42
    Uint16 H1C_Rsvd43;                  //H1C_43
    Uint16 H1C_Rsvd44;                  //H1C_44
    Uint16 H1C_Rsvd45;                  //H1C_45
    Uint16 H1C_Rsvd46;                  //H1C_46
    Uint16 H1C_Rsvd47;                  //H1C_47
    Uint16 H1C_Rsvd48;                  //H1C_48
    Uint16 MemCheck1C;                  //H1C_49           组校验字

//1656--------------预留参数组  H1DLEN = 50 -------------------------------------------------------
    Uint16 H1D_Rsvd00;                  //H1D_00
    Uint16 H1D_Rsvd01;                  //H1D_01
    Uint16 H1D_Rsvd02;                  //H1D_02
    Uint16 H1D_Rsvd03;                  //H1D_03
    Uint16 H1D_Rsvd04;                  //H1D_04
    Uint16 H1D_Rsvd05;                  //H1D_05
    Uint16 H1D_Rsvd06;                  //H1D_06
    Uint16 H1D_Rsvd07;                  //H1D_07
    Uint16 H1D_Rsvd08;                  //H1D_08
    Uint16 H1D_Rsvd09;                  //H1D_09
    Uint16 H1D_Rsvd10;                  //H1D_10
    Uint16 H1D_Rsvd11;                  //H1D_11
    Uint16 H1D_Rsvd12;                  //H1D_12
    Uint16 H1D_Rsvd13;                  //H1D_13
    Uint16 H1D_Rsvd14;                  //H1D_14
    Uint16 H1D_Rsvd15;                  //H1D_15
    Uint16 H1D_Rsvd16;                  //H1D_16
    Uint16 H1D_Rsvd17;                  //H1D_17
    Uint16 H1D_Rsvd18;                  //H1D_18
    Uint16 H1D_Rsvd19;                  //H1D_19
    Uint16 H1D_Rsvd20;                  //H1D_20
    Uint16 H1D_Rsvd21;                  //H1D_21
    Uint16 H1D_Rsvd22;                  //H1D_22
    Uint16 H1D_Rsvd23;                  //H1D_23
    Uint16 H1D_Rsvd24;                  //H1D_24
    Uint16 H1D_Rsvd25;                  //H1D_25
    Uint16 H1D_Rsvd26;                  //H1D_26
    Uint16 H1D_Rsvd27;                  //H1D_27
    Uint16 H1D_Rsvd28;                  //H1D_28
    Uint16 H1D_Rsvd29;                  //H1D_29
    Uint16 H1D_Rsvd30;                  //H1D_30
    Uint16 H1D_Rsvd31;                  //H1D_31
    Uint16 H1D_Rsvd32;                  //H1D_32
    Uint16 ComMoniPara1IndexLow;        //H1D_33 通信监控参数1 索引-子索引 16进制显示
    Uint16 ComMoniPara1IndexHigh;       //H1D_34 
    Uint16 ComMoniPara2IndexLow;        //H1D_35 通信监控参数2 索引-子索引 16进制显示
    Uint16 ComMoniPara2IndexHigh;       //H1D_36
    Uint16 ComMoniPara3IndexLow;        //H1D_37 通信监控参数3 索引-子索引 16进制显示
    Uint16 ComMoniPara3IndexHigh;       //H1D_38 
    Uint16 ComMoniPara4IndexLow;        //H1D_39 通信监控参数4 索引-子索引 16进制显示
    Uint16 ComMoniPara4IndexHigh;       //H1D_40 
    Uint16 ComMoniPara1Low;             //H1D_41 通信监控参数1数据
    Uint16 ComMoniPara1High;            //H1D_42 
    Uint16 ComMoniPara2Low;             //H1D_43 通信监控参数2数据
    Uint16 ComMoniPara2High;            //H1D_44 
    Uint16 ComMoniPara3Low;             //H1D_45 通信监控参数3数据
    Uint16 ComMoniPara3High;            //H1D_46           
    Uint16 ComMoniPara4Low;             //H1D_47 通信监控参数4数据
    Uint16 ComMoniPara4High;            //H1D_48
    Uint16 MemCheck1D;                  //H1D_49           组校验字

//1706------------保留参数  HRsvdLEN = 102 --------------------------------------------------------
    Uint16  HRsvd_Rsvd[HRsvdLEN];

//1808------------电子标签存储参数  HEL_LEN = 16 ------------------------------------------------------
    Uint16  EL_SN[HEL_LEN];          //32字节产品唯一序列标签（ASCII码）

//1824------------故障存储参数  ErrSvLEN = 176 ------------------------------------------------------
    Uint16  ES_ServoTime_L;          //伺服运行时间,单位0.1s
    Uint16  ES_ServoTime_H;          //伺服运行时间,单位0.1s
    Uint16  ES_ErrMsgTail;           //停机故障序号
    Uint16  ES_Rsvd[13];             //保留
    //故障信息记录结构体数组
    STR_ERR_MSG_RECORD      ErrMsgBuff[10];

//2000------------掉电保存参数  OffSvLEN = 16 --------------------------------------------------------
    Uint16  OS_CurrentPos_L;            //停机位置L
    Uint16  OS_CurrentPos_H;            //停机位置H
    Uint16  OS_Rsvd[5]; 
    
    Uint16  OS_AbsMod2MechSAbsPos_PowOffL;     //多圈模式2掉电时旋转负载单圈位置L
    Uint16  OS_AbsMod2MechSAbsPos_PowOffH;     //多圈模式2掉电时旋转负载单圈位置H
    Uint16  OS_AbsMod2MechSAbsPos_PowOffH32L;  //多圈模式2掉电时旋转负载单圈位置高32位 L  

    Uint16  OS_AbsMod2MechSAbsPos_PowOff222L;     //多圈模式2掉电时编码器位置L
    Uint16  OS_AbsMod2MechSAbsPos_PowOff222H;     //多圈模式2掉电时编码器位置H
    Uint16  OS_AbsMod2MechSAbsPos_PowOff222H32L;  //多圈模式2掉电时编码器位置高32位 L
    
    Uint16  OS_AbsMod2EncPos_PowOffL;     //多圈模式2掉电时编码器位置L
    Uint16  OS_AbsMod2EncPos_PowOffH;     //多圈模式2掉电时编码器位置H
    Uint16  OS_AbsMod2EncPos_PowOffH32L;  //多圈模式2掉电时编码器位置高32位 L
    



    
//2016------------ 在线下载加密参数  IAPLEN = 16 --------------------------------------------------------
    Uint16  IAP_Rsvd[16];

//共计2032
}STR_FUNCODE;

typedef union{
    Uint16 all[2032];
    STR_FUNCODE  code;
    STR_FUNGROUP group;
} UNI_CODETABLE;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern UNI_CODETABLE FunCodeUnion;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* end of FUNC_FunCode.h */

/********************************* END OF FILE *********************************/


