/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_GlobalVariable.h                                                           
 创建人：朱祥华            创建日期：2011.11.09 
 修改人：XX                修改日期：XX.XX.XX 
 描述： 电机模块所有源文件均需包含此头文件
    1.电机模块全局变量声明
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef MTR_GLOBALVARIABLE_H
#define MTR_GLOBALVARIABLE_H 



#ifdef __cplusplus
 extern "C" {
#endif    



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 



/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define MTR_Valid          1
#define MTR_Invalid        0


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */    



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//MTR全局标志位
typedef struct{
    Uint32  CurSampleRdy:1;           //电机电流检测准备好标志位
    //Uint32  UdcLow_ToqLmtFlg: 1;     //母线电压下降标志位 
    //Uint32  RbOverLoadWarn:1;         //电阻过载警告
    Uint32  InertiaIdyEn:1;           //惯量辨识生效标志位
    //Uint32  BrakeEn:1;                //泄放使能
    //Uint32  InputRdyFlg:2;            //是否缺相  0刚上电输入未准备好  1 输入准备好  2 输入未准备好
    Uint32  RevlDir:1;                // 指令方向选择
    Uint32  HighPrecisionAIEn:2;        //高精度AI使能
    //Uint32  UdcOkFlag:1;         //bit0 母线电压Ok 
    Uint32  Rsvd:27;                  //保留
}STR_MTR_GLOBAL_FLAG_BIT;

typedef union{
    volatile Uint32                      all;
    volatile STR_MTR_GLOBAL_FLAG_BIT     bit;
}UNI_MTR_GLOBAL_FLAG_REG; 

//MTR传递到FUNC模块的标志位
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
}STR_MTR_TOFUNC_FLAG_BIT;

typedef union{
    volatile Uint32                      all;
    volatile STR_MTR_TOFUNC_FLAG_BIT     bit;
}UNI_MTR_TOFUNC_FLAG_REG; 


//④系统调度文件使用（电机模块内全局变量）结构体
typedef struct{
    Uint16  CarFreq;            //输出 载波频率,Hz  FUN->MTR
    Uint16  ToqFreq;            //输出 转矩环的调节频率,单位Hz  FUN->MTR
    Uint16  SpdFreq;            //输出 速度环的调节频率,单位Hz  FUN->MTR
    //Uint16  PosFreq;            //输出 位置环的调节频率,单位Hz  FUN->MTR

    Uint16  SpdCnt;
    //Uint16  PosCnt;

    //Uint32  CarPrd_Q10;         //输出 载波周期的时间,单位us Q10格式    MTR
    Uint32  ToqPrd_Q10;         //输出 转矩环周期时间,单位us Q10格式    MTR
    Uint32  SpdPrd_Q10;         //输出 电流环周期时间,单位us Q10格式    MTR
    //Uint32  PosPrd_Q10;         //输出 位置环周期时间,单位us Q10格式    MTR 
//    Uint32  MonitPrd_Q10;       //单位us 
}STR_MTR_SYSTEM;


//⑤系统调度标志位
typedef struct{
    Uint16  SpdFlg:1;              //bit0 速度环调度标志位
    Uint16  rsvd:15;
}STR_MTR_SCHEDULAR_BIT;

typedef union{
   volatile Uint16                    all;
   volatile STR_MTR_SCHEDULAR_BIT     bit;
}UNI_MTR_SCHEDULAR_FLAG_REG;


//⑥Q轴电流指令获取文件内（电机模块内全局变量）使用变量
typedef struct{
    int32   IqRef;                //Q轴电流调节器前端指令，按照调节器标幺值处理后的值
    int32   IdRef;                //D轴电流调节器前端指令，按照调节器标幺值处理后的值
    int32   ToqRefConst_Q12;      //输出 转矩指令转换系数: 百分数（0.1%）->内部转矩(Iq)指令 MTR->FUN
	int32   ToqDigToPer_Q12;      //转矩指令数字量到百分比转换系数（数字量）->0.1%
    int16   IqRate_MT;            //当电机相有效值电流为电机额定电流时对应IqRef的（即IqRate_MT）输入值
    int32   IqRate_OEM;           //当电机相有效值电流为驱动器额定电流时对应IqRef（即IqRate_OEM）输入值
    int16   IqMax_MT;             //当电机相有效值电流为电机最大电流时对应IqRef的（即IqMax_MT）输入值
    int32   IqMax_OEM;            //当电机相有效值电流为驱动器最大电流时I对应qRef（即IqMax_OEM ）输入值
    int32   IqPosMax;             //转矩内部限制保守值
    int32   IqNegMax;
    int32   IqPosLmt;             //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的正向IqRef最后限制值
    int32   IqNegLmt;             //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的反向IqRef最后限制值
    int32   IqPosLmt_F;           //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的FUNC模块正向IqRef最后限制值
    int32   IqNegLmt_F;           //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的FUNC模块反向IqRef最后限制值
    Uint16  PseudoSpdFlg;         //转矩模式下的速度限制：伪速度调节器调度标志位，与速度环同步
    int32   IqPoint1;             //增益切换所用切换点
    int32   IqPoint2;
    int32   IqPoint3;
    int32   IqPoint4;
    int32   FirSlope_Q10;         //增益切换第一斜率
    int32   SecSlope_Q10;         //增益切换第二斜率
}STR_MTR_GET_IQ_REFER;



//⑦FPGA文件内（电机模块内全局变量）使用变量
typedef struct{
    Uint16  SysStatus;            //FPGA系统运行状态
    Uint16  SysErr;               //FPGA系统故障状态
    Uint32  EncRev;               //编码器分辨率
	Uint32  AngToCnt;    		  //每个电周期对应的脉冲数
//    Uint32  UdcRate;              //母线电压额定值 单位0.1V
    int32   SpdFdb;               //速度反馈 MTR->FUN
	int32   SpdFdbLast;			 //上周期速度反馈
    int32   IqFdb;                //输出 q轴电流反馈值，标幺值
    int32   IdFdb;                //输出 d轴电流反馈值，标幺值
    int32   IuFdb;                //输出 U相反馈电流值
    int32   IvFdb;                //输出 V相反馈电流值
    int32   IwFdb;                //输出 W相反馈电流值,为模拟电路合成相；
    int32   Vd;                   //输出 d轴电压调制度
    int32   Vq;                   //输出 q轴电压调制度
    int32   PosFdbAbsValue;       //当前位置反馈采样（相对于Z的绝对式）
    int32   PulsePosRef;          //位置脉冲指令给定
    int32   PulsePosRef2;         //位置脉冲指令给定2 高速
    Uint32  MechaAngle;           //机械角度（从原点开始的脉冲数）
    int32   Etheta;               //输出 当前电机电角度值(0-32768)
    int32   LnrPosFdb;            //FPGA中读出的全闭环位置反馈
    int32   UVAdjustResult;       //UV相电流平衡校正结果

    Uint16  DeadTmCtrl;           //死区控制寄存器设置
    Uint16  DeadTmComp;           //死区补偿寄存器设置
    Uint16  CurSampCtrl;          //电流采样控制设置
    Uint16  SysCtrl;              //系统控制设置
    int32   PosFdbAbsVal_ZInt;    //Z中断时的绝对位置反馈

    int32   M_SpdFdb;             //M法速度反馈值
	int32   M_SpdFdbFlt;          //M法测速滤波后的值
    union{
       volatile Uint16  all;
       struct{                         
            Uint16  PwmPeriod:15;
            Uint16  PwmMode:1; 
            }bit;
    }PWMPrdMode;

    int32   HighPrecisionAI;       //高精度AI
}STR_MTR_FPGA;

//MTR模块中需要输出给后台示波器显示的结构体变量类型定义
typedef struct{
    int32  CH1_Test;                //电机模块示波器测试变量1
    int32  CH2_Test;                //电机模块示波器测试变量2
    int32  CH3_Test;                //电机模块示波器测试变量3
    int32  CH4_Test;                //电机模块示波器测试变量4
}STR_MTR_OSCILLOSCOPE;

//MTR模块中的速度调节器文件输出变量
typedef struct{
    int32   SpdReguOut;               //速度调节器输出后经前馈和滤波后的输出值
    int32   FSA_SpdReguOut;           //开环频率特性测试虚拟调节器 
    int32   FSA_SpdReguOutPlusChirp;  
}STR_MTR_SPEED_REGULATOR;

//MTR模块中的惯量辨识文件输出变量
typedef struct{
    Uint32   RatioNow;               //速度调节器输出后经前馈和滤波后的输出值 
}STR_MTR_INERTIA_IDENTIFY;

//初始角度辨识结构体对象
typedef struct
{
    int32 AbsZLatchPos;    //Z相锁存的位置绝对信息
    int16 StartFindZ;     //开始找Z命令
    int16 AngInitZFlag;   //找到Z相脉冲标志
}STR_MTR_ANGINT;

//摩擦力辨识结构体
typedef struct
{
    int32 Coff_J;        //用于计算惯量比的系数
}STR_MTR_INFRID;

//绝对式编码器处理文件输出变量
typedef struct
{
    int16   AbsTransferTime;        //绝对式编码器通讯时间
    int16   AbsCommBaud;            //绝对式编码器通讯波特率

    Uint32  MultiAbsPosFdbL;        //多圈绝对式编码器位置反馈
    Uint32  MultiAbsPosFdbH;        //多圈绝对式编码器位置反馈

    Uint32  SingleAbsPosFdb;       //编码器单圈位置反馈 

    Uint32  FpgaPosFdbInit;         //FPGA位置反馈初值

    Uint32  AbsPosDetection;        //绝对位置检测系统模式
}STR_MTR_ABSROM;
/*MTR模块和FUNC模块间的接口变量以及MTR子模块间的接口变量结构体成员定义*/
typedef struct{

    STR_MTR_SYSTEM                  System;        //④系统调度文件使用（电机模块内全局变量）

    STR_MTR_GET_IQ_REFER            GetIqRef;      //⑥Q轴电流指令获取文件内（电机模块内全局变量）使用变量

    STR_MTR_FPGA                    FPGA;

    STR_MTR_SPEED_REGULATOR         SpeedRegulator;   //速度调节器文件输出
    
    STR_MTR_INERTIA_IDENTIFY        InertiaIdy;      //惯量辨识全局标志位

    STR_MTR_ANGINT                  AngInt;         //初始角辨识结构体
    STR_MTR_INFRID                  InFricId;       //在线摩擦辨识结构体

    STR_MTR_ABSROM                  AbsRom;        //绝对式编码器结构体

    STR_MTR_OSCILLOSCOPE            OscTarget;     //电机模块输出给后台示波器显示的变量
   
    UNI_MTR_SCHEDULAR_FLAG_REG      ScheldularFlag;   //调度标志位

    UNI_MTR_GLOBAL_FLAG_REG         GlobalFlag;       //电机模块内部调用的全局标志位

    UNI_MTR_TOFUNC_FLAG_REG         MTRtoFUNCFlag;    //MTR传递到FUNC模块的标志位

    int32  ToFUNCSpdFdb;             //传递到FUNC模块的速度反馈
}STR_MTR_GLOBALVARIABLE;




/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern volatile Uint16 *HostSon;
extern STR_MTR_GLOBALVARIABLE               STR_MTR_Gvar;






#ifdef __cplusplus
}
#endif

#endif /* MTR_GlobalVariable.h */    
/********************************* END OF FILE *********************************/
