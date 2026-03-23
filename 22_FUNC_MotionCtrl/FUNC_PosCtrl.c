/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_PosCtrl.c
 创建人：高小峰                
 修改人：李浩               创建日期：11.09.23 
 描述： 1. 
        2.
        3.
 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_PosCtrl.h"          
#include "FUNC_Pulse.h"
#include "FUNC_FunCode.h"
#include "FUNC_PosRegulator.h"
#include "FUNC_PosCmdFilter.h"
#include "FUNC_EleGear.h"
#include "FUNC_MultiBlockPos.h"
#include "FUNC_XIntPosition.h"
#include "FUNC_Curve.h"
#include "FUNC_GainRegIndicatrix.h"
#include "FUNC_Home.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ServoError.h" //G2_LH_12.31 主要是提供PstErrMsg()函数
#include "FUNC_FullCloseLoop.h"
#include "FUNC_Filter.h"
#include "CANopen_OD.h"
#include "CANopen_Home.h"
#include "CANopen_Pub.h"
#include "CANopen_PP.h"
#include "CANopen_DeviceCtrl.h"
#include "ECT_Pub.h"
#include "ECT_PP.h"
#include "ECT_CSP.h"
#include "FUNC_ManageFunCode.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//#define   POSSTEPNUM   4    //每一次位置环给的位置量，对应速度为POSSTEP_*6

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_INNER_GLOBAL_POSCONTROL    STR_InnerGvarPosCtrl = {0};   //FUNC_MotionCtrl模块内全局结构体变量

STR_LOCAL_POSCONTROL           STR_PosCtrl = {0};    //FUNC_PosCtrl.c文件内结构体变量

STR_ELECTRONIC_GEAR            STR_EleGear;       //定义电子齿轮结构体变量；

STR_POS_AVERAGE_FILTER         STR_PosAverageFilter;

STR_POS_LOWPASS_FILTER         STR_PosLowPassFilter = POS_LOWPASS_FILTER_Defaults;  //定义位置环调用的一阶低通滤波器

STR_POSNOTCH                   STR_PosNotch = {0};      //位置指令陷波器

//位置指令转化成速度值滤波器
STR_BILINEAR_LOWPASS_FILTER    PosRefToSpdFilter = BILINEAR_LOWPASS_FILTER_Defaults ;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitPosCtrl(void);   
void PosCtrlStopUpdata(void);
void PosCtrlUpdata(void);
void PosCtrl_MainLoop(void);
void ClrPosReg(void);
void PosControl(void); 
void PosShow(void);


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

Static_Inline void PosCmdSource(void);
Static_Inline int16 PosStepCalc(void);
Static_Inline void PosArriveAndNear(void);
Static_Inline void PosMonitor(void);
Static_Inline void PosRefToSpdValue(void);
Static_Inline void PulsOutSpdMonitor(void);
void ECTPosShow(void);


/* Extern Functions ---------------------------------------------------------*/
/* 该文件外部部调用的函数的声明 */ 
extern void PowerOffSave_MainLoop(void); //多圈绝对位置模式更新
extern void AbsMod2CalcPosUpLmt(void);

/*******************************************************************************
  函数名: PulsOutSpdMonitor
  输入:   电机当前转速，分频分辨率
  输出:  分频速率
  子函数:          
  描述: 分频输出过速。分频速率F= spdfd * Fdivencoder/60
  如果F大于硬件最大频率则报错，硬件最大频率2000K
********************************************************************************/ 
Static_Inline void PulsOutSpdMonitor(void)
{
	int64 OutPulsFreq;
	int32 OutPulsDelta;
    
    //角度辨识不报警
    if((AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)||(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus==NRD))return;

#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
	//1000K*60*10000(0.0001rpm)=600000000000
	if(FunCodeUnion.code.PL_DivSourceSel==0)
	{
		OutPulsFreq = (int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb * (int32)FunCodeUnion.code.PL_PosCodeDiv_L;
		
		if(ABS(OutPulsFreq) >= (int64)600000000000)
		{
			PostErrMsg(PULSOUTOSPD);
		}
	}
	else if(FunCodeUnion.code.PL_DivSourceSel==1)//要兼容脉冲指令同步输出
	{
		OutPulsDelta = STR_InnerGvarPosCtrl.DeltaPulse * (int32)STR_FUNC_Gvar.System.PosFreq;
		
        if(FunCodeUnion.code.PL_PosPulsInMode == 2)
        {
            if(ABS(OutPulsDelta) >= (int32)4000000)
    		{
    			PostErrMsg(PULSOUTOSPD);
    		}
        }
        else if(ABS(OutPulsDelta) >= (int32)1000000)
		{
			PostErrMsg(PULSOUTOSPD);
		}
	}
	else //无分频输出不保护
	{
	}


#else
	//2000K*60*10000(0.0001rpm)=1200000000000
	if(FunCodeUnion.code.PL_DivSourceSel==0)
	{
		OutPulsFreq = (int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb * (int32)FunCodeUnion.code.PL_PosCodeDiv_L;
		
		if(ABS(OutPulsFreq) >= (int64)1200000000000)
		{
			PostErrMsg(PULSOUTOSPD);
		}
	}
	else if(FunCodeUnion.code.PL_DivSourceSel==1)
	{
		OutPulsDelta = STR_InnerGvarPosCtrl.DeltaPulse * (int32)STR_FUNC_Gvar.System.PosFreq;
		
        if(FunCodeUnion.code.PL_PosPulsInMode==2)
        {
            if(ABS(OutPulsDelta) >= (int32)8000000)
    		{
    			PostErrMsg(PULSOUTOSPD);
    		}
        }
        else if(ABS(OutPulsDelta) >= (int32)2000000)
		{
			PostErrMsg(PULSOUTOSPD);
		}
	}
	else //无分频输出不保护
	{
	}
#endif    
}

/*******************************************************************************
  函数名: void PosCtrlStopUpdata() 
  输入:             
  输出:    
  子函数:                                       
  描述:  位置控制初始化 
********************************************************************************/ 
void InitPosCtrl() 
{
    int32 temp11;
    /*增量式位置指令限制幅值初始化，按硬件最大输入位置指令脉冲频率 1Mpps 来计算的*/
    //STR_InnerGvarPosCtrl.PosRefIncrLmt = (int32)1000000 / (int32)STR_FUNC_Gvar.System.PosFreq;

    /* 电子齿轮初始化 */
    EleGearInit(&STR_EleGear);  //电子齿轮初始化

    // 初始化位置偏差过大判断阈值，用H0A14功能码进行单位换算
    STR_PosCtrl.PerrFaultValue = ( ( (int32)FunCodeUnion.code.ER_PerrFaultVluHigh << 16 ) + (int32)FunCodeUnion.code.ER_PerrFaultVluLow);

    /*位置达到及接近初始化，用位置指令单位进行判断*/	
    STR_InnerGvarPosCtrl.Parrive = (int32)FunCodeUnion.code.PL_PosReachValue;
    if(FunCodeUnion.code.PL_PosNearValue < FunCodeUnion.code.PL_PosReachValue)  //对参数进行关联限制
    {
        STR_InnerGvarPosCtrl.Pnear = STR_InnerGvarPosCtrl.Parrive; // 如果定位接近参数小于定位到达参数
    }
    else
    {
        STR_InnerGvarPosCtrl.Pnear = (int32)FunCodeUnion.code.PL_PosNearValue; 
    }
    
    /*如果位置平滑滤波时间参数发生改变*/
    STR_PosAverageFilter.FilterTime = (((Uint32)FunCodeUnion.code.PL_PosAverge_tme * 100) << 10) / STR_FUNC_Gvar.System.PosPrd_Q10; //平均滤波时间,功能码单位0.1ms，变量单位1ms
    STR_PosAverageFilter.FltTmeDiv = (1 << 15) / STR_PosAverageFilter.FilterTime; 
    STR_PosAverageFilter.FilterBuffCnt = 0;    //位置指令平均值滤波缓冲数组计数器1
    STR_PosAverageFilter.UnfullBuffCnt = 0;    //位置指令平均值滤波缓冲数组为填满时计数器2
    STR_PosAverageFilter.CmdBuffSum = 0;       //位置指令平均值滤波缓冲数组总和
    STR_PosAverageFilter.Remainder = 0;        //位置指令余数清零   //servo off时，清除余数避免没有指令输入时的错误输出
    STR_PosAverageFilter.CmdRemainderSum = 0;  //位置指令余数总和清零

    /* 位置环低通滤波初始化 */
    STR_PosLowPassFilter.Ts = (int32)( STR_FUNC_Gvar.System.PosPrd_Q10  >> 10 );     //位置环采样周期，单位us，
    STR_PosLowPassFilter.Tc = 100L * ((int32)FunCodeUnion.code.PL_PulseInFilterTime);   //初始化位置环调用的低通滤波器滤波时间时间H0504，单位us。
    InitPosLowPassFilter(&STR_PosLowPassFilter);     //初始化中计算双线性变化低通滤波器系数

    /* 通讯位置指令在每次调度中的最大限制值 */
    //STR_PosCtrl.CommPosStepMax = (int32)( ((Uint64)AuxFunCodeUnion.code.CC_CommPosLmtSpd * UNI_FUNC_MTRToFUNC_InitList.List.EncRev) /
    //                                      ((Uint64)60* STR_FUNC_Gvar.System.PosFreq) );

    /* 插补计算中用到的两个系数, 优化插补过程的计算量, 可放在仅上电执行一次的地方 */
    STR_InnerGvarPosCtrl.Ms2PtCoefQ13 = ((Uint32)STR_FUNC_Gvar.System.PosFreq << 13) / 1000L;

    if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30)        //直线光栅尺
    {
        STR_InnerGvarPosCtrl.RPM2PPPtCoefQ16 = ((Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev << 16)
                                             / ((Uint32)STR_FUNC_Gvar.System.PosFreq);

        STR_PosCtrl.PosRefToSpdCoeff_Q20 = ((Uint64)FunCodeUnion.code.MT_EncoderPensH << 20) / 100;  //1ms计算一次
    }
    else
    {
		temp11 = A_SHIFT16_PLUS_B(FunCodeUnion.code.FC_ExCoderPulse_H,FunCodeUnion.code.FC_ExCoderPulse_L);

   		STR_InnerGvarPosCtrl.ExRPM2PPPtCoefQ16 = ((Uint64)temp11 << 16)
                                             / ((Uint64)60L*STR_FUNC_Gvar.System.PosFreq);

    	STR_InnerGvarPosCtrl.RPM2PPPtCoefQ16 = ((Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev << 16)
                                                 / ((Uint64)60* STR_FUNC_Gvar.System.PosFreq);
    	STR_PosCtrl.PosRefToSpdCoeff_Q20 = (int64)(((int64)60000L << 20)                      //1ms计算一次
                                           / (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev);
	}

    //1ms 进行一次H0B11的计算与显示，而计时时间则是在位置环进行计数的
    STR_PosCtrl.SpdRef_PulseTime = (1 * (STR_FUNC_Gvar.System.PosFreq / 1000));
    /* 上电初始化一次 */
    //620P中中断定长的中断处理由FPGA完成，因此无需启用ST_ARM底层的硬件配置，但需要告诉FPGA启用中断定长功能
    if(FunCodeUnion.code.OEM_LocalModeEn ==1)
    {
        XintPosDrvConfig();   // 中断定长FPGA中断触发底层配置
    }

    HomingInitOnce();     // 原点回归 仅上电初始化一次的参数

    MultiPosReset();      // 多段位置插补复位
    MultiPosInit();	      // 初始化 多段位置参数
    HomingStopUpdate();   // 停机更新回原点的参数
    HomingReset();        // 原点回归插补复位
    XintPosReset();       // 中断定长插补复位
    MultiPosRunUpdate();  // 运行时更新多段位置参数  
	CurveReset()       ;  // 定位试运行插补复位
	CurveInit()        ;  // 定位试运行初始化
	CurveDataUp()      ;  // 运行时更行JOG模式参数
	PerformanceIndexInit();//增益调整初始化

    //全闭环初始化
    FullCloseLoopInit(UNI_FUNC_MTRToFUNC_InitList.List.EncRev);
    
     //位置到达输出信号Coin锁存
	if(FunCodeUnion.code.PL_CoinHoldTime==0)
	{
	    temp11=0;
	}
	else
	{
	    temp11=	(int32)FunCodeUnion.code.PL_CoinHoldTime*STR_FUNC_Gvar.System.PosFreq/1000;
		if(temp11==0)
		{
		    temp11=1;											//避免计算导致的截断误差，与功能码设置混淆
		}
	}
	STR_InnerGvarPosCtrl.CoinHoldTime = temp11;

    //位置指令转化成速度值滤波器参数配置
    PosRefToSpdFilter.Ts = 1000;       
    PosRefToSpdFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDoFilt;     
    PosRefToSpdFilter.InitLowPassFilter(&PosRefToSpdFilter);
    
    #if ECT_ENABLE_SWITCH
	    //Canopen初始化
    InitCanopenPPMode();
    InitECTCSP();
	ECTCSPMaxSpeedUpdate();//CSP最大速度限制，运行更新
    CanopenHomingModeFuncVarStop();
    CanopenHomingReset();
    #endif
	
	CanopenPPModeStopUpdata();

}


/*******************************************************************************
  函数名: void PosCtrlStopUpdata() 
  输入:             
  输出:    
  子函数:                                       
  描述:  复位上电及伺服OFF时，更新位置控制中的相关变量  
********************************************************************************/ 
void PosCtrlStopUpdata()
{
	int32  temp11;
	
    /* 位置环低通滤波时间发生变化时，应该对滤波器进行清零复位 */
    if( ( STR_PosLowPassFilter.Tc != (int32)FunCodeUnion.code.PL_PulseInFilterTime * 100L) ||
        ( STR_PosLowPassFilter.Ts != (int32)(STR_FUNC_Gvar.System.PosPrd_Q10 >> 10) )
      )
    {
        STR_PosLowPassFilter.Ts = (int32)(STR_FUNC_Gvar.System.PosPrd_Q10 >> 10);     //位置环采样周期，单位us，
        STR_PosLowPassFilter.Tc = ((int32)FunCodeUnion.code.PL_PulseInFilterTime) * 100L;   //初始化位置环调用的低通滤波器滤波时间时间H0504，单位us。
        InitPosLowPassFilter(&STR_PosLowPassFilter);                          //初始化中计算双线性变化低通滤波器系数
    }

    //1ms 进行一次H0B11的计算与显示，而计时时间则是在位置环进行计数的
    STR_PosCtrl.SpdRef_PulseTime = (1 * (STR_FUNC_Gvar.System.PosFreq / 1000));

    if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30)        //直线光栅尺,注意是10ms计算一次
    {
        STR_InnerGvarPosCtrl.RPM2PPPtCoefQ16 = ((Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev << 16)
                                             / ((Uint64)STR_FUNC_Gvar.System.PosFreq);

        STR_PosCtrl.PosRefToSpdCoeff_Q20 = ((Uint64)FunCodeUnion.code.MT_EncoderPensH << 20) / 100;  //1ms计算一次
    }
    else
    {
		temp11 = A_SHIFT16_PLUS_B(FunCodeUnion.code.FC_ExCoderPulse_H,FunCodeUnion.code.FC_ExCoderPulse_L);

   		STR_InnerGvarPosCtrl.ExRPM2PPPtCoefQ16 = ((Uint64)temp11 << 16)
                                             / ((Uint64)60L*STR_FUNC_Gvar.System.PosFreq);

    	STR_InnerGvarPosCtrl.RPM2PPPtCoefQ16 = ((Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev << 16)
                                                 / ((Uint64)60* STR_FUNC_Gvar.System.PosFreq);
    	STR_PosCtrl.PosRefToSpdCoeff_Q20 = (int64)(((int64)60000L << 20)                      //1ms计算一次
                                           / (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev);
	}

    /*增量式位置指令限制幅值初始化，按硬件最大输入位置指令脉冲频率 1Mpps 来计算的*/
    //STR_InnerGvarPosCtrl.PosRefIncrLmt = 1000000 / STR_FUNC_Gvar.System.PosFreq;

    MultiPosReset();      // 多段位置复位
    MultiPosInit();	      // 初始化 多段位置参数
    HomingStopUpdate();   // 停机更新回原点的参数
    HomingReset();        // 原点回归插补复位
    XintPosReset();       // 中断定长插补复位
	CurveReset()  ;       // 定位试运行插补复位
	CurveStopDataUp();    // JOG运动与定位试运行复位
	IndicatrixStopUpData(); //增益调整性能指标计算清零
	MaxPulsFreqSet();     //位置脉冲最大输入频率设定
    //全闭环初始化
    FullCloseLoopInit(UNI_FUNC_MTRToFUNC_InitList.List.EncRev);

	PosNotchFilterClr(&STR_PosNotch);    //位置指令陷波器清除

    #if ECT_ENABLE_SWITCH
    //Canopen停机更新
    CanopenPPModeStopUpdata();
    CanopenHomingModeFuncVarStop();
    CanopenHomingReset();
    ECTCSPUpdata();
    #endif
    
    AbsMod2CalcPosUpLmt();
}

/*******************************************************************************
  函数名: void PosCtrlRunUpdata(void) 
  输入:             
  输出:    
  子函数:         
  描述:  伺服On 电机运行过程中，更新位置控制中的相关变量  
********************************************************************************/ 
void PosCtrlUpdata()
{
    static Uint16 LowOscFreqLast = 0;	      //上周期的低频值
	static Uint16 LowOscFiltLast = 0;		  //上周期的低频滤波值
	static Uint16 LowOscRatioLast = 0;
    int32 temp11;

	//允许运行中设定低频振动频率和滤波
	if(((LowOscFreqLast != FunCodeUnion.code.AT_LowOscFreqA) || (LowOscFiltLast != FunCodeUnion.code.AT_LowOscFiltA) || (LowOscRatioLast != FunCodeUnion.code.AT_LowFreqRatio)) 
	  && (STR_PosNotch.PosDelta == 0))
	{
		LowOscFreqLast = FunCodeUnion.code.AT_LowOscFreqA;
		LowOscFiltLast = FunCodeUnion.code.AT_LowOscFiltA;
		LowOscRatioLast= FunCodeUnion.code.AT_LowFreqRatio;
		
		PosNotchInit(&STR_PosNotch);   //位置指令陷波器初始化	    
	}

    // 初始化位置偏差过大判断阈值，用H0A14功能码进行单位换算
    STR_PosCtrl.PerrFaultValue = ( ( (int32)FunCodeUnion.code.ER_PerrFaultVluHigh << 16 ) + (int32)FunCodeUnion.code.ER_PerrFaultVluLow);
    
    //把判定振动的阈值转换成跟编码器分辨率有关
    STR_InnerGvarPosCtrl.LowOscPulsCnt = ((Uint32)FunCodeUnion.code.ER_LowOscPuls 
                                         * UNI_FUNC_MTRToFUNC_InitList.List.EncRev) / 10000;
                                             
    
     //位置到达输出信号Coin锁存
	if(FunCodeUnion.code.PL_CoinHoldTime==0)
	{
	    temp11=0;
	}
	else
	{
	    temp11=	(int32)FunCodeUnion.code.PL_CoinHoldTime*STR_FUNC_Gvar.System.PosFreq/1000;
		if(temp11==0)
		{
		    temp11=1;											//避免计算导致的截断误差，与功能码设置混淆
		}
	}
	STR_InnerGvarPosCtrl.CoinHoldTime = temp11;
    /*位置达到及接近初始化，用位置指令单位进行判断*/	
    STR_InnerGvarPosCtrl.Parrive = (int32)FunCodeUnion.code.PL_PosReachValue;
    if(FunCodeUnion.code.PL_PosNearValue < FunCodeUnion.code.PL_PosReachValue)  //对参数进行关联限制
    {
        STR_InnerGvarPosCtrl.Pnear = STR_InnerGvarPosCtrl.Parrive; // 如果定位接近参数小于定位到达参数
    }
    else
    {
        STR_InnerGvarPosCtrl.Pnear = (int32)FunCodeUnion.code.PL_PosNearValue; 
    }

    EleGearRatioErrCheck();  //电子齿轮初始化
    
    /*如果位置平滑滤波时间参数发生改变*/  
    if( STR_PosAverageFilter.FilterTime != (((Uint32)FunCodeUnion.code.PL_PosAverge_tme * 100) << 10) / STR_FUNC_Gvar.System.PosPrd_Q10   )  
    {
        STR_PosAverageFilter.FilterTime = (((Uint32)FunCodeUnion.code.PL_PosAverge_tme * 100) << 10) / STR_FUNC_Gvar.System.PosPrd_Q10; //平均滤波时间,功能码单位0.1ms，变量单位1ms
        STR_PosAverageFilter.FltTmeDiv = (1 << 15) / STR_PosAverageFilter.FilterTime; 
        STR_PosAverageFilter.FilterBuffCnt = 0;    //位置指令平均值滤波缓冲数组计数器1
        STR_PosAverageFilter.UnfullBuffCnt = 0;    //位置指令平均值滤波缓冲数组为填满时计数器2
        STR_PosAverageFilter.CmdBuffSum = 0;       //位置指令平均值滤波缓冲数组总和
        STR_PosAverageFilter.Remainder = 0;        //位置指令余数清零   //servo off时，清除余数避免没有指令输入时的错误输出
        STR_PosAverageFilter.CmdRemainderSum = 0;  //位置指令余数总和清零
    }
    
    //位置指令转化成速度值滤波器参数配置
    PosRefToSpdFilter.Ts = 1000;       
    PosRefToSpdFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDoFilt;     
    PosRefToSpdFilter.InitLowPassFilter(&PosRefToSpdFilter); 

    #if ECT_ENABLE_SWITCH
    //Canopen模式运行更新
    CanopenPPModeUpdate();
    CanopenHomeRunUpdata();	
    #endif
}
/*******************************************************************************
  函数名: void PosCtrl_MainLoop(void) 
  输入:             
  输出:    
  子函数:         
  描述:  主循环(1ms)中更行，伺服On 电机运行过程中，更新位置控制中的相关变量  
********************************************************************************/ 
void PosCtrl_MainLoop(void)
{
//    int32 Temp =0; 

    CurveDataUp()  ;  // 运行时更行JOG运动与定位试运行参数
    
    IndicatrixDataUp(); //增益调整数据更新

	PulsOutSpdMonitor(); //分频输出故障监控

    HomParaUpdateRealTim(); //回零参数更新

    MultiPosRunUpdate();  // 运行时更新多段位置参数  

    PowerOffSave_MainLoop(); //掉电保存更新

	ECTCSPMaxSpeedUpdate();//CSP最大速度限制，运行更新
  
}



/*******************************************************************************
  函数名: 
  输入:  
  输出:	       
  子函数:         
  描述: 
********************************************************************************/ 
void ClrPosReg()
{
    ResetPosSmoothFilter(&STR_PosAverageFilter);      //复位平均滤波器，相关变量清零

    ResetPosLowPassFilter(&STR_PosLowPassFilter);     //复位低通滤波器，相关变量清零

	PosNotchFilterClr(&STR_PosNotch);                 //位置指令陷波器复位

    STR_EleGear.GearRemainder = 0;                    //电子齿轮余数清零

    ClrPosErrAndRegulator();                          //位置随动误差及调节器及速度前馈环节清零

    STR_PosCtrl.PosErrCnt = 0;                        //偏差计数器(位置偏差量)清零

    STR_FUNC_Gvar.PosCtrl.DovarReg_Coin = 0;              //位置到达信号清零
    STR_FUNC_Gvar.PosCtrl.DovarReg_Near = 0;              //位置接近清零
	
	ClearFullLoopVar();                               //全闭环参数清零
    
	STR_FUNC_Gvar.PosCtrl.HomStats = 0;
   
	#if ECT_ENABLE_SWITCH
	CanopenClrPosReg();     //Canopen参数清零
	STR_ECTCSPVar.OTLatchPosCmd = 0;
    #endif
}


/*******************************************************************************
  函数名:  PosControl()
  输入:                                      
  输出:    
  子函数: PosPulseCal()；PosCmdSource()；EleGearGetValue(); EleGearRatioCalc(); PosSmoothFilter();
          PosErrCalcAndClr(); PosArriveAndNear()；PosMonitor()；PosShow()；      
  描述:   位置环控制部分的执行调度，包括位置脉冲计算，位置指令来源，电子齿轮，
          平滑滤波，位置偏差计算及清零,位置到达等环节的执行时序
********************************************************************************/ 
void PosControl()
{
    int32   PosRefTemp = 0;

    //接口      
    STR_InnerGvarPosCtrl.PulsePosRef = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PulsePosRef;          //绝对脉冲指令更新
    STR_InnerGvarPosCtrl.PulsePosRef2 = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PulsePosRef2;          //绝对脉冲指令更新 高速
    #if ECT_ENABLE_SWITCH
    STR_InnerGvarPosCtrl.PulsePosRef3 = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.ExPosFdbAbsValue;
    #endif

	//full closed loop handle   
    FullCloseLoopModeSet(&STR_FUNC_Gvar.PosCtrl.PosFdb,&STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7,UNI_FUNC_MTRToFUNC_FastList_16kHz.List.ExPosFdbAbsValue,STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag);
	//end full closed loop


    PosCmdSource();  //位置指令来源选择   多段位置时平均为931/72=12.93us

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD)   //位置模式下才进行位置控制相关处理244/72us=3.38us
    {
        // 位置环平均值平滑滤波作用 
        //滤波时间为零时, 回原点时, 中断定长时,多段位置连续运行方式且是绝对指令时, 不进行平均值平滑滤波 
        if( (STR_PosAverageFilter.FilterTime == 0) ||
            (STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork == 1)  ||
            (STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork == 1) || 
            ( (2 == FunCodeUnion.code.PL_PosSurcASel) && 
              (1 == FunCodeUnion.code.MP_PosRefType) && 
              (3 == FunCodeUnion.code.MP_PosRunMode) )||
            (FunCodeUnion.code.BP_ModeSelet == 9)    //EtherCAT模式
          )      
        {
            ResetPosSmoothFilter(&STR_PosAverageFilter);      //复位平均滤波器，相关变量清零
            STR_PosCtrl.PosFiltCmd = STR_PosCtrl.PosMultipleCmd;
        }
        else 
        {
            STR_PosAverageFilter.Input = STR_PosCtrl.PosMultipleCmd;
            PosSmoothFilter(&STR_PosAverageFilter);   //位置指令平均值滤波,128ms平均值滤波
            STR_PosCtrl.PosFiltCmd = STR_PosAverageFilter.Output;
        }

        // 位置环低通滤波作用
        //滤波时间为零时, 回原点时,多段位置连续运行方式且是绝对指令时, 不进行滤波 
        if( (STR_PosLowPassFilter.Tc == 0) ||
            (STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork == 1) ||
            ( (2 == FunCodeUnion.code.PL_PosSurcASel) && 
              (1 == FunCodeUnion.code.MP_PosRefType) && 
              (3 == FunCodeUnion.code.MP_PosRunMode) )||
            (FunCodeUnion.code.BP_ModeSelet == 9)    //EtherCAT模式
          )
        {
            ResetPosLowPassFilter(&STR_PosLowPassFilter);     //复位低通滤波器，相关变量清零
            PosRefTemp = STR_PosCtrl.PosFiltCmd;
        }
        else
        {
            STR_PosLowPassFilter.Input = STR_PosCtrl.PosFiltCmd;
            PosCmdLowPassFilter(&STR_PosLowPassFilter);
            PosRefTemp = STR_PosLowPassFilter.Output; 
        }

        //位置指令陷波器计算
		//陷波器频率为100HZ, 回原点时, 中断定长时,多段位置连续运行方式且是绝对指令时, 不进行平均值平滑滤波 
        if( (FunCodeUnion.code.AT_LowOscFreqA == 1000) ||
            (STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork == 1)  ||
            (STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork == 1) ||
			(STR_CanSyscontrol.Mode == ECTPOSMOD)|| 
			(STR_FUNC_Gvar.Monitor2Flag.bit.LagFilterClr == 1) ||
            ( (2 == FunCodeUnion.code.PL_PosSurcASel) && 
              (1 == FunCodeUnion.code.MP_PosRefType) && 
              (3 == FunCodeUnion.code.MP_PosRunMode) )
          )      
        {
            PosNotchFilterClr(&STR_PosNotch);      //复位位置指令陷波器
            PosRefTemp = PosRefTemp;	          //
        }
        else 
        {
            STR_PosNotch.Input[0] = PosRefTemp;
            PosNotchFilter(&STR_PosNotch);         //位置指令陷波器计算
            PosRefTemp = STR_PosNotch.Output[0];
        }

        STR_FUNC_Gvar.PosCtrl.PosRef = PosRefTemp;

        STR_FUNC_Gvar.PosCtrl.PosRefLatch = STR_FUNC_Gvar.PosCtrl.PosRef;

        if (STR_FUNC_Gvar.PosCtrl.PosRefLatch > 0) 
        {    
            STR_InnerGvarPosCtrl.MutexBit.bit.PosRefDir = 0;   //正转标志 
        }
        else if(STR_FUNC_Gvar.PosCtrl.PosRefLatch < 0)
        {
            STR_InnerGvarPosCtrl.MutexBit.bit.PosRefDir = 1;   //反转标志 
        }

        
		if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus!=RUN)
        {
            STR_ECTCSPVar.OTLatchPosCmd = 0;
        }

        //STR_ECTCSPVar.OTDeal = 0;
        if(  (STR_FUNC_Gvar.PosCtrl.HomStats == 0) 
          && (   (STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos == 1) 
              || ( (STR_FUNC_Gvar.DivarRegLw.bit.Pot == 1) && (STR_FUNC_Gvar.PosCtrl.PosRef > 0) ) 
              || ( (STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) && (STR_FUNC_Gvar.PosCtrl.PosRef < 0) ) ) )         
                      
        {
            //位置模式下刚发生超程，零速停机阶段或自由停机方式(PWM没有关闭前)位置指令清零，或高级模式下位置锁定有效，防止报位置偏差过大故障
            STR_ECTCSPVar.OTLatchPosCmd += STR_FUNC_Gvar.PosCtrl.PosRef;
            STR_FUNC_Gvar.PosCtrl.PosRef = 0;
            //STR_ECTCSPVar.OTDeal = 1;
        }
        else if( (STR_FUNC_Gvar.PosCtrl.HomStats == 1) 
                && ((STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) && (STR_FUNC_Gvar.PosCtrl.PosRef < 0)))         
                      
        {
            //位置模式下刚发生超程，零速停机阶段或自由停机方式(PWM没有关闭前)位置指令清零，或高级模式下位置锁定有效，防止报位置偏差过大故障
            STR_FUNC_Gvar.PosCtrl.PosRef = 0;
        }
        else if( (STR_FUNC_Gvar.PosCtrl.HomStats == 2) 
                && ((STR_FUNC_Gvar.DivarRegLw.bit.Pot == 1) && (STR_FUNC_Gvar.PosCtrl.PosRef > 0)))         
                      
        {
            //位置模式下刚发生超程，零速停机阶段或自由停机方式(PWM没有关闭前)位置指令清零，或高级模式下位置锁定有效，防止报位置偏差过大故障
            STR_FUNC_Gvar.PosCtrl.PosRef = 0;
        }
        else if((STR_FUNC_Gvar.MonitorFlag.bit.ToqStop==1)||(STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus==DISPWM))        
        {  
            //急停转矩停机时，位置指令清零
            STR_FUNC_Gvar.PosCtrl.PosRef = 0;
        }
        else if((STR_FUNC_Gvar.DivarRegHi.bit.EmergencyStop == 1))        
        {  
            //DI紧急停车时 位置指令清零
            STR_FUNC_Gvar.PosCtrl.PosRef = 0;
        }
		else
		{
	        
			if((FunCodeUnion.code.CM_ECATHost==2)&&(STR_CanSyscontrol.Mode==ECTCSPMOD))
			{
				if(STR_ECTCSPVar.OTLatchPosCmd>0)
		        {
		            if(STR_FUNC_Gvar.PosCtrl.PosRef<0)
		            {
		                if((STR_ECTCSPVar.OTLatchPosCmd +STR_FUNC_Gvar.PosCtrl.PosRef)>0)
		                {
		                    STR_ECTCSPVar.OTLatchPosCmd += STR_FUNC_Gvar.PosCtrl.PosRef;
		                    STR_FUNC_Gvar.PosCtrl.PosRef = 0;
		                }
		                else
		                {
		                    STR_FUNC_Gvar.PosCtrl.PosRef +=STR_ECTCSPVar.OTLatchPosCmd; 
		                    STR_ECTCSPVar.OTLatchPosCmd = 0;
		                }
		            }
		            else
		            {}
		        }
		        else if(STR_ECTCSPVar.OTLatchPosCmd<0)
		        {
		            if(STR_FUNC_Gvar.PosCtrl.PosRef>0)
		            {
		                if((STR_ECTCSPVar.OTLatchPosCmd +STR_FUNC_Gvar.PosCtrl.PosRef)<0)
		                {
		                    STR_ECTCSPVar.OTLatchPosCmd += STR_FUNC_Gvar.PosCtrl.PosRef;
		                    STR_FUNC_Gvar.PosCtrl.PosRef = 0;
		                }
		                else
		                {
		                    STR_FUNC_Gvar.PosCtrl.PosRef +=STR_ECTCSPVar.OTLatchPosCmd; 
		                    STR_ECTCSPVar.OTLatchPosCmd = 0;
		                }
		            }
		            else
		            {}
		        }
		        else
		        {}
			}
			else
			{
                STR_ECTCSPVar.OTLatchPosCmd = 0;
            }
		
		}

    }
    else
    {
        STR_ECTCSPVar.OTLatchPosCmd = 0;
        STR_FUNC_Gvar.PosCtrl.PosRef = 0;
        STR_FUNC_Gvar.PosCtrl.PosRefLatch = 0;
        ResetPosSmoothFilter(&STR_PosAverageFilter);      //复位平均滤波器，相关变量清零
        ResetPosLowPassFilter(&STR_PosLowPassFilter);     //复位低通滤波器，相关变量清零
		PosNotchFilterClr(&STR_PosNotch);                //复位位置指令陷波器
    }

    if((STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD) || (STR_FUNC_Gvar.MonitorFlag.bit.OTClamp == 1))  //位置模式下才进行位置控制相关处理
    {
        PosErrCalcAndClr();    //位置偏差计算及清零 

        PosArriveAndNear();    //位置到达及位置接近输出

        PosMonitor();          //位置状态监控

        FullCloseLoopErrorCheck(STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag);

        PosRefToSpdValue();    //位置指令转化成速度值

	    PerformanceIndexCal();//增益调整相关指标计算
    }
} 

/*******************************************************************************
  函数名:  void PosCmdSource()
  输入:    STR_PosCtrl.DeltaPulse (增量式位置脉冲指令)以及其相应子函数位置指令输入
  输出:    STR_PosCtrl.PosCmd (位置指令选择输出)
  子函数:  POS_Xint1_Cal()中断定长，PosStepCalc()步进量，通讯来源，
           MultiPosHandle()多段位置，FixLengthChaBu()线性插补                 
  描述:   位置指令来源选择 
********************************************************************************/ 
Static_Inline void PosCmdSource()
{
    int32  PosCmdSource = 0;         //位置来源选择好的位置指令值
    int32  PosDeltaPulse = 0;
    int32  PosCmdThis = 0;           // 用于记录本次位置指令值(齿轮比之前)
    
	Uint16 HRatioSel = 0;           //手轮倍率选择
    static Uint16 HRatio = 1;	    //手轮倍率

    PosDeltaPulse = PosPulseCalc();   //位置脉冲指令的计算
    
    //if(STR_FUNC_Gvar.SoftIntDivarRegHi.bit.PulseInhibit == 1) PosDeltaPulse = 0;

    AuxFunCodeUnion.code.CR_DeltaPulse = PosDeltaPulse;   //H3003 通讯读取输入脉冲指令采样值

    if (STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork == 1)   // 回原点使能时每个插补周期的指令
    {
        // 获取回原点时的位置指令
        PosCmdSource = HomingPosCmd();
        PosCmdThis = PosCmdSource;

        //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新处理
        EleGearGetValue(&STR_EleGear, PosCmdSource);
        STR_PosCtrl.IsMultiPosCycleType = 0;
        //进行电子齿轮计算输出放大了的位置指令
        STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear, PosCmdSource);
    }
	else if (1 == STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork) //JOG运动与定位试运行
	{
	    if(0 == STR_Curve.PulseSendSel)
		{
		    PosCmdSource     = JogCmdGenerator();
			STR_Curve.JogOut = 0;
		}
		else
		{
			PosCmdSource     = CurveHandle();
			STR_Curve.JogOut = 0;
		}
        PosCmdThis = PosCmdSource;
		STR_PosCtrl.IsMultiPosCycleType = 0;
			
		STR_PosCtrl.PosMultipleCmd = PosCmdSource;
    }
#if ECT_ENABLE_SWITCH
    else if((FunCodeUnion.code.BP_ModeSelet == 9)/*&&(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==1)*/)
    {
        if(DeviceControlVar.OEMStatus == STATUS_QUICKSTOPACTIVE)
        {
            CanopenPPReset();
			CanopenPPPosBuffReset();
			CSPClear(); // by huangxin201801_1 急停时csp复位
			CSPPointCal(); // by huangxin201801_2 急停时对齐目标位置，解决急停恢复时反向运动bug
			if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == VALID)
            {
                if((STR_ServoMonitor.StopCtrlVar.CanQuickStopMode==4)
                    ||(STR_ServoMonitor.StopCtrlVar.CanQuickStopMode==5))
                {
                    PosCmdSource = CanopenPosModeSlopeStop();
                }
                else
                {
                    DeviceControlVar.PosSlopeStopActiveFlag = 1;
                    PosCmdSource = 0;
                }
				

            }
            /*  by huangxin201711_18 DeviceControlVar.PosSlopeStopActiveFlag = 0执行不了，导致第二次急停时出错，换个位置
			else
            {
                DeviceControlVar.PosSlopeStopActiveFlag = 0;
            }
			*/
            
            if((STR_CanSyscontrol.Mode==ECTHOMMOD))STR_CanopenHome.HomingStatus.bit.HomingEn = 0;            
            //PP模式可接收位置指令位、回零完成、IP激活位
            ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;

			if(STR_FUNC_Gvar.SpdCtrl.DovarReg_Zero)
			{
				CanopenClrPosReg();     //Canopen参数清零
			}
        }
        else
        {
			DeviceControlVar.PosSlopeStopActiveFlag = 0; //by huangxin201711_18 DeviceControlVar.PosSlopeStopActiveFlag = 0执行不了，导致第二次急停时出错，换个位置
			switch(STR_CanSyscontrol.Mode)
        	{
        		case ECTPOSMOD:
        			PosCmdSource = CanopenPPModePosRefCal();
        			break;
                    
        		case ECTCSPMOD:		
        			PosCmdSource = ECTCSPPosCmd();
                    break;
                    
        		case ECTHOMMOD:
                    PosCmdSource = CanopenHomingPosCmd();
        			break;
                    
        		default:
                    PosCmdSource = 0;
        			break;
        	}
		}
        PosCmdThis = PosCmdSource;
        STR_PosCtrl.PosMultipleCmd = PosCmdThis;//暂时先这么写
    }
#endif
    else if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        if (STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork == 1)  // 运行中断定长时每个插补周期的指令
        {
            // 获取中断定长时的位置指令
            PosCmdSource = XintPosCmd();
            PosCmdThis = PosCmdSource;

            //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新处理
            EleGearGetValue(&STR_EleGear, PosCmdSource);
			STR_PosCtrl.IsMultiPosCycleType = 0;
            //进行电子齿轮计算输出放大了的位置指令
            STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear, PosCmdSource);
        }
        else if(STR_FUNC_Gvar.DivarRegLw.bit.HxEn == 1)    //Di使能手轮输入,则此时指令来源选择为手轮,其他来源忽略
        {
            HRatioSel = (STR_FUNC_Gvar.DivarRegLw.bit.Hx1 + (STR_FUNC_Gvar.DivarRegLw.bit.Hx2<<1)); //HRatioSel代表手轮的倍率 可以为10倍，100倍 
            switch(HRatioSel)               //根据X1,X2的输入选择倍率
            {
                case 1:
                    HRatio = 10;
                    break;
                case 2:
                    HRatio = 100;
                    break;
                default: 
                    HRatio = 1;
                    break;
            } 

            PosCmdSource = ((int32)HRatio * PosDeltaPulse);

            if (STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel == 1) //根据Di信号确定位置指令方向
            {
                PosCmdSource = - PosCmdSource;
            }

            PosCmdThis = PosCmdSource;

            EleGearGetValue(&STR_EleGear,PosCmdSource);   //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新处理
			STR_PosCtrl.IsMultiPosCycleType = 0;
            STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear,PosCmdSource);   //进行电子齿轮计算输出放大了的位置指令
        }
        else
        {
            switch (FunCodeUnion.code.PL_PosSurcASel) //位置指令来源选择
            {
                case 0:
                    PosCmdSource = PosDeltaPulse;  //位置脉冲增量式来源

                    if (STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel == 1)  //根据Di信号确定位置指令方向
                    {
                        PosCmdSource = - PosCmdSource;
                    }

                    PosCmdThis = PosCmdSource;

                    EleGearGetValue(&STR_EleGear,PosCmdSource);   //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新

                    STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear,PosCmdSource);   //进行电子齿轮计算输出放大了的位置指令
					STR_PosCtrl.IsMultiPosCycleType = 0;
                    break;

                case 1:
                    PosCmdSource = PosStepCalc();     //步进形式的位置指令来源 

                    if (STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel == 1)  //根据Di信号确定位置指令方向
                    {
                        PosCmdSource = - PosCmdSource;
                    }

                    PosCmdThis = PosCmdSource;

                    EleGearGetValue(&STR_EleGear,PosCmdSource);   //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新处理

                    STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear,PosCmdSource);   //进行电子齿轮计算输出放大了的位置指令
					STR_PosCtrl.IsMultiPosCycleType = 0;
                    break;

                case 2:
                    // 获取多段位置时的位置指令
                    PosCmdSource = MultiPosHandle();
                    PosCmdThis = PosCmdSource;
            
                    //获取电子齿轮分子分母值,其中包含电子齿轮比切换和更新处理
                    EleGearGetValue(&STR_EleGear, PosCmdSource);
            
                    //进行电子齿轮计算输出放大了的位置指令
                    STR_PosCtrl.PosMultipleCmd = EleGearRatioCalc(&STR_EleGear, PosCmdSource);
					STR_PosCtrl.IsMultiPosCycleType = 1;
                   break;

                default : break;
            }
        }
    }
    
    if ((STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock == 1)&&(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD))
    {
        STR_PosCtrl.PosMultipleCmd = 0; // 在伺服On，抱闸器电源上电后延时300ms内位置指令清零，之后位置指令不清零。
        PosCmdThis = 0;
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)    //伺服运行时，增量式输入脉冲指令累加和
    {
//        Temp = ((int64)STR_PosCtrl.PosMultipleCmd * STR_InnerGvarPosCtrl.Denominator ) + STR_InnerGvarPosCtrl.PulseCalcRemainder;
//
//        STR_InnerGvarPosCtrl.PulseCalcRemainder = Temp % STR_InnerGvarPosCtrl.Numerator;
//
//        Temp = (Temp / STR_InnerGvarPosCtrl.Numerator);
//
//        STR_InnerGvarPosCtrl.InputPulseCnt += (int32)Temp;

        // 累加本次位置指令值(齿轮比之前的值)
        STR_InnerGvarPosCtrl.InputPulseCnt += PosCmdThis;
    }
	STR_InnerGvarPosCtrl.InputPulsCntRT += PosCmdThis;

    if(FunCodeUnion.code.BP_ModeSelet==9)
    {
        STR_FUNC_Gvar.OscTarget.InputPulseCnt = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,STR_InnerGvarPosCtrl.InputPulseCnt);
        STR_FUNC_Gvar.OscTarget.InputPulsCntRT = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,STR_InnerGvarPosCtrl.InputPulsCntRT);
    }
    else
    {
        STR_FUNC_Gvar.OscTarget.InputPulseCnt = STR_InnerGvarPosCtrl.InputPulseCnt;
        STR_FUNC_Gvar.OscTarget.InputPulsCntRT = STR_InnerGvarPosCtrl.InputPulsCntRT;
    }

}


/*******************************************************************************
  函数名:  int16 PosStepCal()
  输入:    FunCodeUnion.code.PL_PosStep
           STR_FUNC_Gvar.DivarRegLw.bit.PosStep
  输出:    PosStepOutput
  子函数:  无       
  描述:    步进量形式给位置指令,且每次位置量以POSSTEPNUM给定，直到总数PosStepGiven给完
********************************************************************************/ 
Static_Inline int16 PosStepCalc() 
{
    static int32 PosStepGiven;   //定义步进形式的位置指令给定值变量
    int32 PosStepOutput;         //定义步进形式的位置指令输出变量
    int32 temp11;
	static int32 PosStepOutputRem=0;
    static int16 WorkStateFlag=0;//运行状态标志
	static int32 PosStepnum=0;
    static int32 PosStepnumLmt1=0;
    static int32 PosStepnumLmt2=0;

    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)  //只有在Servo运行状态下才执行步进形式的位置指定给定
    {
        if((WorkStateFlag==0)&&(STR_FUNC_Gvar.DivarRegLw.bit.PosStep == 1))  //Di使能步进给定 
        {
            PosStepGiven = ((int32)((int16)FunCodeUnion.code.PL_PosStep)<<12); //从功能码 H0505，获取位置步进量给定值

            if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)   //直线光栅尺处理
            {
                //计算步进频率，按照24mm/s
                PosStepnumLmt1 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev*24)/((int64)STR_FUNC_Gvar.System.PosFreq);
				PosStepnumLmt1 = (PosStepnumLmt1<<12);

                //计算电子齿轮比放大之后的步进最大能达到1500mm/s对应的电子齿轮比之前的每周期脉冲个数
                PosStepnumLmt2 = (((int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev*4096*STR_InnerGvarPosCtrl.Denominator) /
                                     ((int64)STR_FUNC_Gvar.System.PosFreq * STR_InnerGvarPosCtrl.Numerator));            
				PosStepnumLmt2 = (PosStepnumLmt2*1500);
            }
            else
            {
                //计算步进频率，按照24rpm
                PosStepnumLmt1 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev*(24L<<12))/((int64)60* STR_FUNC_Gvar.System.PosFreq);

                //计算电子齿轮比放大之后的步进最大能达到1500rpm转速时对应的电子齿轮比之前的每周期脉冲个数
                PosStepnumLmt2 = (((int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev*40960*STR_InnerGvarPosCtrl.Denominator) /
                                     ((int64)6 * STR_FUNC_Gvar.System.PosFreq * STR_InnerGvarPosCtrl.Numerator));            
				PosStepnumLmt2 = (PosStepnumLmt2*15);
            }

            if(PosStepnumLmt2 < PosStepnumLmt1)
            {
                PosStepnum = PosStepnumLmt2;
            }
            else
            {
                PosStepnum = PosStepnumLmt1;
            }

            if(0 == PosStepnum) PosStepnum = 1;

            WorkStateFlag = 1;   //职位步进量给定运行状态标志位，使之从无效状态进入运行状态
        }
        if (WorkStateFlag==1)   //位置步进给定运行处理，在运行过程中，没有运行结束则不允许打断
        {
            if (PosStepGiven >= 0)
            {
                if (PosStepGiven <= PosStepnum)
                {
                    PosStepOutput = PosStepGiven;
                    WorkStateFlag = 2;  //步进给定运行结束，进入结束状态
                }
                else
                {
                    PosStepOutput = PosStepnum;  //步进给定运行过程中 
                    PosStepGiven = PosStepGiven - PosStepnum;  //总的步进给定量减去每次位置环输入的步进量，之后为剩余步进量
                }
            }
            else
            {
                if (PosStepGiven >= -PosStepnum)
                {
                    PosStepOutput = PosStepGiven;
                    WorkStateFlag = 2;  //步进给定运行结束，进入结束状态
                }
                else
                {
                    PosStepOutput = -PosStepnum; //步进给定运行过程中 
                    PosStepGiven = PosStepGiven + PosStepnum;  //总的步进给定量减去每次位置环输入的步进量，之后为剩余步进量
                }
            }
        }
        else  // 在非运行状态下，位置步进量输出为零
        {
            PosStepOutput = 0; 
        }
        if((WorkStateFlag==2)&&(STR_FUNC_Gvar.DivarRegLw.bit.PosStep == 0))  
        {
           WorkStateFlag = 0;   //从结束状态进入无效状态
        }
    }
    else  //在非伺服运行状态下，清除当前位置步进运行状态
    {
        if(WorkStateFlag==1)    
        {
            WorkStateFlag = 0; //位置步进给定运行，置为无效状态
        }
        PosStepOutput = 0;     //清除当前步进输出
        PosStepGiven  = 0;     //清除当前步进量指令
    }
    temp11 = PosStepOutputRem + PosStepOutput;
	PosStepOutput = temp11>>12;
	PosStepOutputRem = temp11 - (PosStepOutput<<12); 

    return PosStepOutput;
} 
/*******************************************************************************
  函数名:  PosArriveAndNear()
  输入:    STR_FUNC_Gvar.PosCtrl.PosAmplifErr
           FunCodeUnion.code.PL_ReachConditio (H0520位置定位完成条件选择)
                     
  参数：   STR_InnerGvarPosCtrl.Parrive  位置到达阈值
           STR_InnerGvarPosCtrl.Pnear    位置接近阈值
               
  输出:    STR_FUNC_Gvar.PosCtrl.DovarReg_Coin  位置达到DO输出
           STR_FUNC_Gvar.PosCtrl.DovarReg_Near  位置接近DO输出

  子函数:  无 
        
  描述:    根据位置到达及位置接近DO输出条件，输出相应DO信号
********************************************************************************/ 
Static_Inline void PosArriveAndNear()
{
	static Uint8 CoinLatch=0;
	static int32 HoldTemp=0;
    int8  CoinTemp = 0;
    int8  NearTemp = 0;
    int32 PosAmplifErrTemp;
	int32 TempCoinHoldTime;
    //全闭环模式下，若用内外位置反馈，外部位置随动偏差不为STR_FUNC_Gvar.PosCtrl.PosAmplifErr，若不用外部位置反馈，
    //外部位置随动偏差即为STR_FUNC_Gvar.PosCtrl.PosAmplifErr
//    if((STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag == 1) && (FunCodeUnion.code.FC_ExInErrFilterTime != 0))
	if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) && ((STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag == 1) || (FunCodeUnion.code.FC_FeedbackMode == 1)))

    {
        PosAmplifErrTemp = STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr;
    }
    else
    {
        PosAmplifErrTemp = STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
    }

    /*位置达到及接近处理判断*/
    /*402添加*/
    #if ECT_ENABLE_SWITCH
    if(FunCodeUnion.code.BP_ModeSelet == 9)
    {
        CanopenPosArrive(PosAmplifErrTemp);
    }
	#endif
    else
    {
        switch (FunCodeUnion.code.PL_ReachConditio) 
        {
            case 0:
                /*位置达到判断*/
                CoinTemp = (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Parrive) ? 1 : 0; 
                /*位置接近判断*/    
                NearTemp = (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Pnear) ? 1 : 0;  
                break;

            case 2:
                /*位置达到判断*/
                CoinTemp = ( (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Parrive)&&(STR_PosCtrl.PosMultipleCmd == 0) ) ? 1 : 0;  
                /*位置接近判断*/    
                NearTemp = ( (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Pnear)&&(STR_PosCtrl.PosMultipleCmd == 0) ) ? 1 : 0;  
                break;

            case 1:
                /*位置达到判断*/
                CoinTemp = ( (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Parrive)&&(STR_FUNC_Gvar.PosCtrl.PosRef == 0) ) ? 1 : 0;  
                /*位置接近判断*/    
                NearTemp = ((ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Pnear)&&(STR_FUNC_Gvar.PosCtrl.PosRef == 0) ) ? 1 : 0;                                                                                                                                                  
            break;

			case 3:							    
	            /*位置达到判断*/
				TempCoinHoldTime = STR_InnerGvarPosCtrl.CoinHoldTime;
				if(( STR_PosCtrl.PosMultipleCmd != 0 ))		 	
				{	
					HoldTemp = TempCoinHoldTime;	   //新命令到达，复位锁存相关参数
					CoinLatch = 0;
				}	
				else if(ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Parrive)			//锁存条件满足
		 		{
					if(TempCoinHoldTime==0)							//H0543功能码设置非零
					{					 						
						CoinLatch= 2;		
					}
					else
					{
					    CoinLatch=1;
						if(HoldTemp==0)	   //更新锁存时间计数器	
						{
							HoldTemp = TempCoinHoldTime;	
						}	
					}	
				}																				
			
				if(CoinLatch==1)		  //位置到达输出信号锁存有效
				{				
					HoldTemp--;	
					if (HoldTemp <= 0) 
					{
					 	HoldTemp=0;
						CoinLatch = 0;
					}
					else 
					{
						CoinTemp  = 1;
					}
				}
					
				if(CoinLatch==0)	  //锁存无效，Coin信号按指定条件变化
				{	
					CoinTemp = ( (ABS(PosAmplifErrTemp) <= STR_InnerGvarPosCtrl.Parrive)&&(STR_PosCtrl.PosMultipleCmd == 0) ) ? 1 : 0; 
				}
				else if(CoinLatch == 2)
				{
					CoinTemp = 1;									//H0543=0，一直锁存位置到达信号到新指令到达
				}						
                break;

            default : break;
        }
        STR_FUNC_Gvar.PosCtrl.DovarReg_Coin  = CoinTemp;

        STR_FUNC_Gvar.PosCtrl.DovarReg_Near  = NearTemp;

    	if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)  //Servo_On 不使能时，不输出相应的DO信号 
        {
            STR_FUNC_Gvar.PosCtrl.DovarReg_Coin = 0; //DO输出无效          
            STR_FUNC_Gvar.PosCtrl.DovarReg_Near = 0; //DO输出无效
        }
    	STR_InnerGvarPosCtrl.MutexBit.bit.ExChangeEn = CoinTemp;

    }
}

/*******************************************************************************
  函数名:  PosMonitor()
  输入:    STR_PosCtrl.PosErrCnt                        
  输出:    PERRWARN; PSTNOERR  (警告或故障)
  子函数:  PostErrMsg() (警告或故障处理函数)      
  描述:    位置偏差监控函数
********************************************************************************/ 
Static_Inline void PosMonitor()
{
	int64 Temp=0;
    
    /*位置偏差计数器,1个位置指令单位*/
    STR_PosCtrl.PosErrCnt = (int32)STR_FUNC_Gvar.PosCtrl.PosAmplifErr;    //位置偏差过大判断中直接根据位置随动误差，与电子齿轮比无关！  
    /*标准402添加*/

    #if ECT_ENABLE_SWITCH
    if(FunCodeUnion.code.BP_ModeSelet==9)
    {
        CanopenPosMonitor(STR_PosCtrl.PosErrCnt);
    }
    #endif
    else
    {
        if ((STR_PosCtrl.PosErrCnt >= STR_PosCtrl.PerrFaultValue )
           ||(STR_PosCtrl.PosErrCnt + STR_PosCtrl.PerrFaultValue <=0 ))  
        {
            PostErrMsg(PSTNOERR);    //偏差位置过大故障
        }
    	Temp = (int64) ((int64)STR_PosCtrl.PosErrCnt * (int64)STR_InnerGvarPosCtrl.Denominator );  
    	STR_FUNC_Gvar.OscTarget.PosAmpErr = Temp / STR_InnerGvarPosCtrl.Numerator; 
    }
	
}

/*******************************************************************************
  函数名:  PosShow()
  输入:                              
  输出:     
  子函数:         
  描述:    位置偏差监控函数
********************************************************************************/
void PosShow(void)
{
    int64  Temp = 0;  
    Uint16 PosRefToSpd = 0;
    
    //H0B09 机械角度(从原点开始的脉冲)显示
    AuxFunCodeUnion.code.DP_MechaAngle = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MechaAngle;  

    //H0B10 旋转角度(电气角度)显示
    AuxFunCodeUnion.code.DP_EleTheta = (Uint16)(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.Etheta * 3600L / 32768L); 

    //H0B17 反馈脉冲计数器赋值
    AuxFunCodeUnion.code.DP_FbPuleCnt_Lo = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos);            
    AuxFunCodeUnion.code.DP_FbPuleCnt_Hi = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos >> 16);


    //H0B-07
    Temp = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Denominator;
    //Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator>>1);   //by huangxin201803 _4 修改-1右移bug
	Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator/2);   //by huangxin201803 _4 修改-1右移bug
    Temp = Temp/(int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator;
    AuxFunCodeUnion.code.DP_AbsPosCnt_Lo = (Uint16)Temp;
    AuxFunCodeUnion.code.DP_AbsPosCnt_Hi = (Uint16)(Temp >> 16); 

    

    //H0B58  机械绝对位置（32位十进制显示）
    AuxFunCodeUnion.code.DP_MechanicalAbsPosL = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos);            
    AuxFunCodeUnion.code.DP_MechanicalAbsPosH = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos >> 16);    
    AuxFunCodeUnion.code.DP_MechanicalAbsPosH32L = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos >> 32); 
    AuxFunCodeUnion.code.DP_MechanicalAbsPosH32H = (Uint16)(STR_InnerGvarPosCtrl.CurrentAbsPos >> 48); 

    //H0B64 实时输入指令脉冲计数器显示和SON无关(指令单位)
	AuxFunCodeUnion.code.DP_InputPulseCntRTL = (Uint16)(STR_FUNC_Gvar.OscTarget.InputPulsCntRT & 0x0000FFFF);
	AuxFunCodeUnion.code.DP_InputPulseCntRTH = (Uint16)(STR_FUNC_Gvar.OscTarget.InputPulsCntRT >> 16);

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD)
    {
		//H0B13 输入指令脉冲计数器显示(指令单位)
        AuxFunCodeUnion.code.DP_InPulseCnt_Lo = (Uint16)((int32)ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue & 0x0000FFFF);
        AuxFunCodeUnion.code.DP_InPulseCnt_Hi = (Uint16)((int32)ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue >> 16);        

        //H0B15 偏差计数器(位置偏差量)显示(编码器单位)
        AuxFunCodeUnion.code.DP_PosErrCnt_Lo = (Uint16)(STR_PosCtrl.PosErrCnt & 0x0000FFFF);
        AuxFunCodeUnion.code.DP_PosErrCnt_Hi = (Uint16)(STR_PosCtrl.PosErrCnt >> 16);  	 

        //H0B53  随动偏差低位(指令单位)
		AuxFunCodeUnion.code.DP_PosErrCntUutL =(Uint16) STR_FUNC_Gvar.OscTarget.PosAmpErr;
        AuxFunCodeUnion.code.DP_PosErrCntUutH = (Uint16)(STR_FUNC_Gvar.OscTarget.PosAmpErr >> 16);
    }
    else
    {
		//H0B13 输入指令脉冲计数器显示(指令单位)
        AuxFunCodeUnion.code.DP_InPulseCnt_Lo = 0;
        AuxFunCodeUnion.code.DP_InPulseCnt_Hi = 0;        

        //H0B15 偏差计数器(位置偏差量)(编码器单位)
        AuxFunCodeUnion.code.DP_PosErrCnt_Lo = 0;
        AuxFunCodeUnion.code.DP_PosErrCnt_Hi = 0;

        //H0B53 随动偏差计数器显示指令单位(指令单位)
		AuxFunCodeUnion.code.DP_PosErrCntUutL = 0;
        AuxFunCodeUnion.code.DP_PosErrCntUutH = 0;
    }

	AuxFunCodeUnion.code.DP_ABSEncSFdbL = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SingleAbsPosFdb & 0xFFFF; 
    AuxFunCodeUnion.code.DP_ABSEncSFdbH = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SingleAbsPosFdb >> 16;

    //旋转负载单圈位置 （编码器单位）
    AuxFunCodeUnion.code.DP_MechSingleAbsPosH32H = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >> 48); 
    AuxFunCodeUnion.code.DP_MechSingleAbsPosH32L = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >> 32); 
    AuxFunCodeUnion.code.DP_MechSingleAbsPosH = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >> 16); 
    AuxFunCodeUnion.code.DP_MechSingleAbsPosL = (Uint16)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;

    //H0B-85
    Temp = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Denominator;
    //Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator>>1);   //by huangxin201803 _4 修改-1右移bug
    Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator/2);   //by huangxin201803 _4 修改-1右移bug
	Temp = Temp/(int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator;
    AuxFunCodeUnion.code.DP_MechSAbsPos_DemandUintL = (Uint16)Temp;
    AuxFunCodeUnion.code.DP_MechSAbsPos_DemandUintH = (Uint16)(Temp >> 16);             


	FullCloseParaDisp();  //全闭环参数显示

    //1ms对应的频率为 1000/1 = 1000Hz
    //位置指令对应的速度 单位1rpm 
    //直线电机 位置指令对应的速度 单位mm/s 
    PosRefToSpd = (Uint16)(((int64)STR_PosCtrl.PosRefPer1ms * (int64)STR_PosCtrl.PosRefToSpdCoeff_Q20) >> 20);

    //位置指令转化成速度值滤波器
    PosRefToSpdFilter.Input = PosRefToSpd;
    PosRefToSpdFilter.LowPassFilter(&PosRefToSpdFilter);
    AuxFunCodeUnion.code.DP_SpdRef_Puse = PosRefToSpdFilter.Output;

    if(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==1)return;

    ECTPosShow();     
}
/*******************************************************************************
  函数名:  PosShow()
  输入:                              
  输出:     
  子函数:         
  描述:    位置数据更新--中断更新
********************************************************************************/
void ECTInteruptPosUpdate(void)
{
    if(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==0)return;
    ECTPosShow();

}
/*******************************************************************************
  函数名:  PosShow()
  输入:                              
  输出:     
  子函数:         
  描述:   位置数据更新--普通更新
********************************************************************************/
void ECTPosShow(void)
{
    int64  Temp = 0; 

    if(FunCodeUnion.code.BP_ModeSelet == 9)
    {
        if(FunCodeUnion.code.FC_FeedbackMode == 1)
		{
			Temp = (int64)STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos; 
		}
		else
		{
            //负载机械位置
            if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
            {
    			Temp = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
            }
            else
            {
    			Temp = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt;
            }
		}
        
		//更新6063 绝对位置反馈 (编码器单位)
        ObjectDictionaryStandard.PosCtrlFunc1.PositionActualValueInc = (int32)Temp;

        //更新6064 绝对位置反馈 (指令单位)
        Temp = Temp * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Denominator;
        //Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator>>1); //by huangxin201803 _4 修改-1右移bug
		Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator/2); //by huangxin201803 _4 修改-1右移bug
        Temp = Temp/(int64)STR_Pos_Factor_Inverse.PosFactor6093_Numerator;
        ObjectDictionaryStandard.PosCtrlFunc1.PositionActualValue = (int32)Temp;

		STR_InnerGvarPosCtrl.PosActualValInUser = Temp;
			

        if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD)
        {
            //更新60FC 输入位置指令计数值(编码器单位)
            ObjectDictionaryStandard.PosCtrlFunc2.PositionDemandValueInc = (int32)STR_InnerGvarPosCtrl.InputPulseCnt;
            //6062--- 输入指令脉冲计数器显示(指令单位)
            ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue    = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,STR_InnerGvarPosCtrl.InputPulseCnt);

	        if(FunCodeUnion.code.FC_FeedbackMode == 1)
			{
				//更新60F4--位置偏差(指令单位)
	            ObjectDictionaryStandard.PosCtrlFunc2.FollowingErrorActualValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr);    			
			}
			else
			{
				//更新60F4--位置偏差(指令单位)
	            ObjectDictionaryStandard.PosCtrlFunc2.FollowingErrorActualValue = STR_FUNC_Gvar.OscTarget.PosAmpErr;    			
			}

        }
        else
        {
            ObjectDictionaryStandard.PosCtrlFunc2.PositionDemandValueInc    = 0;
            ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue       = 0; 
            ObjectDictionaryStandard.PosCtrlFunc2.FollowingErrorActualValue = 0;
        }
    }
    else
    {
        if(FunCodeUnion.code.FC_FeedbackMode == 1)
		{
			Temp = (int64)STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos; 
		}
		else
		{
	        Temp = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt;
		}
        
		//更新6063 绝对位置反馈 (编码器单位)
        ObjectDictionaryStandard.PosCtrlFunc1.PositionActualValueInc = (int32)Temp;

        //更新6064 绝对位置反馈 (指令单位)
        Temp = Temp * (int64)STR_InnerGvarPosCtrl.Denominator;
        //Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_InnerGvarPosCtrl.Numerator>>1);  //by huangxin201803 _4 修改-1右移bug
        Temp = Temp + ((int64)Sign_NP(Temp) * (int64)STR_InnerGvarPosCtrl.Numerator/2);  //by huangxin201803 _4 修改-1右移bug        
		Temp = Temp / (int64)STR_InnerGvarPosCtrl.Numerator; 
        ObjectDictionaryStandard.PosCtrlFunc1.PositionActualValue = (int32)Temp;
		STR_InnerGvarPosCtrl.PosActualValInUser = Temp;
        
        if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD)
        {
            Temp = STR_InnerGvarPosCtrl.InputPulseCnt;

            //6062--- 输入指令脉冲计数器显示(指令单位)
            ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue  = (int32)Temp; 

            //更新60FC 输入位置指令计数值(编码器单位)
            Temp = Temp * (int64)STR_InnerGvarPosCtrl.Numerator;
            Temp = Temp / (int64)STR_InnerGvarPosCtrl.Denominator; 
            ObjectDictionaryStandard.PosCtrlFunc2.PositionDemandValueInc = (int32)Temp;

	        if(FunCodeUnion.code.FC_FeedbackMode == 1)
			{
                Temp = STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr;
                Temp = Temp * (int64)STR_InnerGvarPosCtrl.Denominator;;
                Temp = Temp / (int64)STR_InnerGvarPosCtrl.Numerator; 
			}
			else
			{
	             Temp= STR_FUNC_Gvar.OscTarget.PosAmpErr;    			
			}
			//更新60F4--位置偏差(指令单位)
            ObjectDictionaryStandard.PosCtrlFunc2.FollowingErrorActualValue = (int32)Temp;
        }
        else
        {
            ObjectDictionaryStandard.PosCtrlFunc2.PositionDemandValueInc    = 0;
            ObjectDictionaryStandard.PosCtrlFunc1.PositionDemandValue       = 0; 
            ObjectDictionaryStandard.PosCtrlFunc2.FollowingErrorActualValue = 0;
            
        }
    }
}
/*******************************************************************************
  函数名:  Static_Inline void PosRefToSpdValue()
  输入:    
  输出:    
  子函数:  
  描述:    位置指令转化成速度值(H0B11) 每隔10ms进行一次H0B11的计算与更新
********************************************************************************/ 
Static_Inline void PosRefToSpdValue()
{
    static int16  TempCnt = 0;
    static int32  PosRefAccumulate = 0;         //输入位置指令累加值
    static int32  PosRefAccumulateLatch = 0;    //输入位置指令累加锁存值

    if((STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD) &&
       ((FunCodeUnion.code.PL_PosSurcASel==0)||(FunCodeUnion.code.PL_PosSurcASel==2))) 
    {

        //用于H0B11(位置指令转化成速度)显示使用
        PosRefAccumulate += STR_FUNC_Gvar.PosCtrl.PosRef;

        TempCnt ++;

        if (TempCnt >= STR_PosCtrl.SpdRef_PulseTime)
        {
            STR_PosCtrl.PosRefPer1ms = PosRefAccumulate - PosRefAccumulateLatch;

            TempCnt = 0;
            PosRefAccumulateLatch = PosRefAccumulate;
        }
    }
    else
    {
        STR_PosCtrl.PosRefPer1ms = 0;    

        TempCnt = 0;
        PosRefAccumulate = 0;
        PosRefAccumulateLatch = PosRefAccumulate;
    }
}


/********************************* END OF FILE *********************************/
