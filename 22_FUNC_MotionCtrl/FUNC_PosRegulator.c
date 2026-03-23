/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_PosRegulator.c
 创建人：高小峰                
 修改人：李浩               创建日期：11.10.19 
 描述： 1. 位置调节器，包括前馈
        2. 位置调节器初始化
		
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
#include "FUNC_PosRegulator.h"
#include "FUNC_FunCode.h"
#include "FUNC_PosCtrl.h"

#include "FUNC_GPIODriver.h"          //wzg 20120208
#include "FUNC_Filter.h"        //wzg 20120210
#include "FUNC_FullCloseLoop.h"
#include "CANopen_OD.h"
#include "CANopen_Pub.h"
#include "CANopen_PV.h"
#include "FUNC_CSPCmdBuffer.h"
#include "FUNC_OperEeprom.h"
/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
STR_POSERR_CLEAR_BIT             STR_PosErrClr;                   //定义位置偏差清零的位域结构体变量

STR_BILINEAR_LOWPASS_FILTER      SpdFeedForwardFilter = BILINEAR_LOWPASS_FILTER_Defaults ;     //定义位置环速度前馈滤波器结构体变量并初始化

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
int32   PosReguKpCoef_Q15;       //位置调节器单位换算系数
int32   PosReguFbCoef_Q15;       //速度前馈单位换算系数
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitPosRegu(void);
void PosReguStopUpdata(void);
void PosReguUpdata(void);
void PosRegulator(void);
void PosErrCalcAndClr(void);
void ClrPosErrAndRegulator(void);
void LowOscSelfCal(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void SpdFeedForward(void);

/*******************************************************************************
  函数名:
  输入:
  输出:
  子函数:
  描述:
********************************************************************************/
//void InitPosRegu(void)
//{
//    SpdFeedForwardFilter.Ts = STR_FUNC_Gvar.System.PosPrd_Q10 >> 10;       //单位us，在位置环调用周期
//    SpdFeedForwardFilter.Tc = FunCodeUnion.code.GN_SpdFbFltrT * 10L;    //H08_18;速度前馈滤波时间参数
//    SpdFeedForwardFilter.InitLowPassFilter(&SpdFeedForwardFilter);           //速度前馈滤波初始化
//
//    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线光栅尺处理，注意调节器输出是带4个小数位的！
//    {
//        PosReguKpCoef_Q15 = ((Uint32)1000 << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//        PosReguFbCoef_Q15 = (((int64)10L<<15) * STR_FUNC_Gvar.System.SpdFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//    }
//    else
//    {
//        //位置调节器单位转换系数  PosKp单位0.1Hz,位置调节器输出单位(即速度指令单位为)0.0001rpm
//        //计算公式为 Vcoff_Kp = (60/码盘分辨率) * 10000 / 10    其中乘以10000是由速度指令单位0.0001rpm决定，除以10是由位置比例增益0.1Hz决定的！
//        PosReguKpCoef_Q15 = ((int64)60000L << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//
//        //前馈系数计算，该参数融合了前馈增益的千分比参数，缩小了1000倍后的Q15格式 前馈系数需要考虑时间因素和H0819系数;
//        //计算公式为 Vcoff_Fb = (60/码盘分辨率)* PosFreq *10000 / 1000   其中乘以10000是由速度指令单位0.0001rpm决定，除以1000是由H0819 0.1%决定的
//        PosReguFbCoef_Q15 = (((int64)600L << 15) * STR_FUNC_Gvar.System.SpdFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
//    }
//
//    STR_FUNC_Gvar.PosCtrl.KpCoef_Q15 = PosReguKpCoef_Q15;	   //wzg20111029 提供给速度控制零速钳位(零位固定)内建位置环使用
//    //位置环和速度环频率的比值，用于位置环多级采样控制
//    STR_FUNC_Gvar.PosCtrl.PosDivSpd_Q10 = ((Uint32)FunCodeUnion.code.OEM_SpdLoopFreqScal << 10) / FunCodeUnion.code.OEM_PosLoopFreqScal;
//}
void InitPosRegu(void)
{
    SpdFeedForwardFilter.Ts = STR_FUNC_Gvar.System.PosPrd_Q10 >> 10;       //单位us，在位置环调用周期
    SpdFeedForwardFilter.Tc = FunCodeUnion.code.GN_SpdFbFltrT * 10L;    //H08_18;速度前馈滤波时间参数
    SpdFeedForwardFilter.InitLowPassFilter(&SpdFeedForwardFilter);           //速度前馈滤波初始化

    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线光栅尺处理，注意调节器输出是带4个小数位的！
    {
        PosReguKpCoef_Q15 = ((Uint32)1000 << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
        PosReguFbCoef_Q15 = (((int64)10L<<15) * STR_FUNC_Gvar.System.PosFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
    }
    else
    {
        //位置调节器单位转换系数  PosKp单位0.1Hz,位置调节器输出单位(即速度指令单位为)0.0001rpm
        //计算公式为 Vcoff_Kp = (60/码盘分辨率) * 10000 / 10    其中乘以10000是由速度指令单位0.0001rpm决定，除以10是由位置比例增益0.1Hz决定的！
        PosReguKpCoef_Q15 = ((int64)60000L << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;

        //前馈系数计算，该参数融合了前馈增益的千分比参数，缩小了1000倍后的Q15格式 前馈系数需要考虑时间因素和H0819系数;
        //计算公式为 Vcoff_Fb = (60/码盘分辨率)* PosFreq *10000 / 1000   其中乘以10000是由速度指令单位0.0001rpm决定，除以1000是由H0819 0.1%决定的
        PosReguFbCoef_Q15 = (((int64)600L << 15) * STR_FUNC_Gvar.System.PosFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
    }

    STR_FUNC_Gvar.PosCtrl.KpCoef_Q15 = PosReguKpCoef_Q15;	   //wzg20111029 提供给速度控制零速钳位(零位固定)内建位置环使用
}



/*******************************************************************************
  函数名: PosReguStopUpdata()

  输入:   STR_FUNC_Gvar.System.PosFreq (位置环调度频率)
          UNI_FUNC_MTRToFUNC_InitList.List.EncRev (电机编码器分辨率)

  输出:	  PosReguFbCoef_Q15 (速度前馈转换系数)
          PosReguKpCoef (位置调节器单位转换系数)
     
  子函数: PosReguRunUpdata() (位置调节器运行中初始化内容)
           
  描述:   复位重新上电或者伺服OFF时，初始化位置调节器中的前馈和增益转换系数
********************************************************************************/
//void PosReguStopUpdata(void)
//{
//    if(SpdFeedForwardFilter.Tc != 10L * FunCodeUnion.code.GN_SpdFbFltrT)
//    {
//        SpdFeedForwardFilter.Ts = STR_FUNC_Gvar.System.PosPrd_Q10 >> 10;       //单位us，在位置环调用周期
//        SpdFeedForwardFilter.Tc = 10L * FunCodeUnion.code.GN_SpdFbFltrT ;    //H08_18;速度前馈滤波时间参数
//        SpdFeedForwardFilter.InitLowPassFilter(&SpdFeedForwardFilter);           //速度前馈滤波初始化
//    }
//    
//    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线光栅尺处理，注意调节器输出是带4个小数位的！
//    {
//        PosReguKpCoef_Q15 = ((Uint32)1000 << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//        PosReguFbCoef_Q15 = (((int64)10L<<15) * STR_FUNC_Gvar.System.SpdFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//    }
//    else
//    {
//        //位置调节器单位转换系数  PosKp单位0.1Hz,位置调节器输出单位(即速度指令单位为)0.0001rpm
//        //计算公式为 Vcoff_Kp = (60/码盘分辨率) * 10000 / 10    其中乘以10000是由速度指令单位0.0001rpm决定，除以10是由位置比例增益0.1Hz决定的！
//        PosReguKpCoef_Q15 = ((int64)60000L << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
//
//        //前馈系数计算，该参数融合了前馈增益的千分比参数，缩小了1000倍后的Q15格式 前馈系数需要考虑时间因素和H0819系数;
//        //计算公式为 Vcoff_Fb = (60/码盘分辨率)* PosFreq *10000 / 1000   其中乘以10000是由速度指令单位0.0001rpm决定，除以1000是由H0819 0.1%决定的
//        PosReguFbCoef_Q15 = (((int64)600L << 15) * STR_FUNC_Gvar.System.SpdFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
//    }
//
//    STR_FUNC_Gvar.PosCtrl.KpCoef_Q15 = PosReguKpCoef_Q15;	   //wzg20111029 提供给速度控制零速钳位(零位固定)内建位置环使用
//    //位置环和速度环频率的比值，用于位置环多级采样控制
//    STR_FUNC_Gvar.PosCtrl.PosDivSpd_Q10 = ((Uint32)FunCodeUnion.code.OEM_SpdLoopFreqScal << 10) / FunCodeUnion.code.OEM_PosLoopFreqScal;
//}

void PosReguStopUpdata(void)
{
    if(SpdFeedForwardFilter.Tc != 10L * FunCodeUnion.code.GN_SpdFbFltrT)
    {
        SpdFeedForwardFilter.Ts = STR_FUNC_Gvar.System.PosPrd_Q10 >> 10;       //单位us，在位置环调用周期
        SpdFeedForwardFilter.Tc = 10L * FunCodeUnion.code.GN_SpdFbFltrT ;    //H08_18;速度前馈滤波时间参数
        SpdFeedForwardFilter.InitLowPassFilter(&SpdFeedForwardFilter);           //速度前馈滤波初始化
    }
    
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线光栅尺处理，注意调节器输出是带4个小数位的！
    {
        PosReguKpCoef_Q15 = ((Uint32)1000 << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
        PosReguFbCoef_Q15 = (((int64)10L<<15) * STR_FUNC_Gvar.System.PosFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
    }
    else
    {
        //位置调节器单位转换系数  PosKp单位0.1Hz,位置调节器输出单位(即速度指令单位为)0.0001rpm
        //计算公式为 Vcoff_Kp = (60/码盘分辨率) * 10000 / 10    其中乘以10000是由速度指令单位0.0001rpm决定，除以10是由位置比例增益0.1Hz决定的！
        PosReguKpCoef_Q15 = ((int64)60000L << 15) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;

        //前馈系数计算，该参数融合了前馈增益的千分比参数，缩小了1000倍后的Q15格式 前馈系数需要考虑时间因素和H0819系数;
        //计算公式为 Vcoff_Fb = (60/码盘分辨率)* PosFreq *10000 / 1000   其中乘以10000是由速度指令单位0.0001rpm决定，除以1000是由H0819 0.1%决定的
        PosReguFbCoef_Q15 = (((int64)600L << 15) * STR_FUNC_Gvar.System.PosFreq) / UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
    }

    STR_FUNC_Gvar.PosCtrl.KpCoef_Q15 = PosReguKpCoef_Q15;	   //wzg20111029 提供给速度控制零速钳位(零位固定)内建位置环使用
}

/*******************************************************************************
  函数名: void PosReguRunUpdata()

  输入:   位置环调度周期
          H0818 速度前馈滤波时间参数

  输出:	  滤波器采样时间
          滤波器滤波时间
    
  子函数: Filter1_Init()
          
  描述:  伺服On,运行时,立即更新位置调节器中速度前馈滤波器等相关参数
********************************************************************************/
void PosReguUpdata(void)
{
    if(SpdFeedForwardFilter.Tc !=10L * FunCodeUnion.code.GN_SpdFbFltrT)
    {
        SpdFeedForwardFilter.Ts = STR_FUNC_Gvar.System.PosPrd_Q10 >> 10;       //单位us，在位置环调用周期
        SpdFeedForwardFilter.Tc =10L * FunCodeUnion.code.GN_SpdFbFltrT;    //H08_18;速度前馈滤波时间参数
        SpdFeedForwardFilter.InitLowPassFilter(&SpdFeedForwardFilter);           //速度前馈滤波初始化
    }
}




/*******************************************************************************
  函数名:  PosErrCalcAndClr()

  输入:    FunCodeUnion.code.PL_PerrClrMode (H0516)
           STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus (伺服运行状态)
           CLR_IN (位置偏差清零的IO端口信号)
               
  输出:    STR_FUNC_Gvar.PosCtrl.PosAmplifErr
           
  子函数:  GPIO_ReadPLUSCLR(); 
        
  描述:    计算位置偏差(增量)，以提供位置调节器使用，以及判断执行位置偏差清除功能
********************************************************************************/ 
//void PosErrCalcAndClr(void)
//{
//    static int32 DelPos = 0;
//
//    switch (FunCodeUnion.code.PL_PerrClrMode)                     //根据功能码选择位置偏差清零动作(H0516)
//    {
//        case 0: 
//            //0-伺服OFF及3-发生故障时清除位置偏差脉冲,发出清零标志位
//            STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)? 1 : 0 ; 
//            break;
//        
//        case 1: 
//            //发生故障时,清除位置偏差脉冲 ,发出清零标志位
//            STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR)? 1 : 0 ;                                                                                     
//            break;
//		case 2:
//			//通过CLR信号清除偏差
//			STR_PosErrClr.ClrFlag = STR_FUNC_Gvar.DivarRegHi.bit.ClrPosErr;
//
//        default : break;
//    }
//
//    //当在选择执行Case2-5时，在Servo 未准备好时，仍需要清除位置偏差，发出清零标志位
//    STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == NRD)? 1 : STR_PosErrClr.ClrFlag ;
//
//    if(STR_PosErrClr.ClrFlag == 0)   //不清除脉冲差
//    {
//        if(STR_InnerGvarPosCtrl.PosRefNewFlag == 1)   //位置指令更新后计算16K时位置指令
//        {
//            STR_FUNC_Gvar.PosCtrl.PosRmned += STR_InnerGvarPosCtrl.PosRefNew;
//            STR_InnerGvarPosCtrl.PosRefNewFlag = 0;    //清除更新标志位
//            DelPos = (STR_FUNC_Gvar.PosCtrl.PosRmned * STR_FUNC_Gvar.PosCtrl.PosDivSpd_Q10) >> 10;
//        }
//
//        if(ABS(STR_FUNC_Gvar.PosCtrl.PosRmned) >= ABS(DelPos))     //根据余量计算16K周期时的位置指令
//        {
//            STR_FUNC_Gvar.PosCtrl.PosRef_16K = DelPos;
//        }
//        else
//        {
//            STR_FUNC_Gvar.PosCtrl.PosRef_16K = STR_FUNC_Gvar.PosCtrl.PosRmned;
//        }
//
//        STR_FUNC_Gvar.PosCtrl.PosRmned -= STR_FUNC_Gvar.PosCtrl.PosRef_16K;
//        STR_InnerGvarPosCtrl.PosErrLastT = STR_InnerGvarPosCtrl.PosErrLast;  //上上周期的位置偏差
//        //上一次位置偏差记录 
//        STR_InnerGvarPosCtrl.PosErrLast = STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
//        //位置偏差计算，放在伺服使能及位置模式外面，servo_off时位置差也生效，以报警显示等用
//        STR_FUNC_Gvar.PosCtrl.PosAmplifErr = STR_FUNC_Gvar.PosCtrl.PosRef_16K - STR_FUNC_Gvar.PosCtrl.PosFdb + STR_InnerGvarPosCtrl.PosErrLast;
// 
//    
//    }
//    else                             //清除脉冲差
//    {
//        DelPos                           = 0;
//        STR_FUNC_Gvar.PosCtrl.PosRef_16K = 0;
//        STR_InnerGvarPosCtrl.PosErrLastT = 0;
//        STR_InnerGvarPosCtrl.PosErrLast = 0;
//        STR_FUNC_Gvar.PosCtrl.PosRmned  = 0;
//        STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;    
//		ClearFullLoopVar();
//        STR_PosErrClr.ClrFlag = 0;   // 清除位置偏差后将位置偏差清楚标志位置零
//    }
//}

void PosErrCalcAndClr(void)
{

    switch (FunCodeUnion.code.PL_PerrClrMode)                     //根据功能码选择位置偏差清零动作(H0516)
    {
        case 0: 
            //0-伺服OFF及3-发生故障时清除位置偏差脉冲,发出清零标志位
            STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)? 1 : 0 ; 
            break;
        
        case 1: 
            //发生故障时,清除位置偏差脉冲 ,发出清零标志位
            STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR)? 1 : 0 ;                                                                                     
            break;
		case 2:
			//通过CLR信号清除偏差
			STR_PosErrClr.ClrFlag = STR_FUNC_Gvar.SoftIntDivarRegHi.bit.ClrPosErr;
            break;

        default : break;
    }

    //当在选择执行Case2-5时，在Servo 未准备好时，仍需要清除位置偏差，发出清零标志位
    STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == NRD)? 1 : STR_PosErrClr.ClrFlag ;
	STR_PosErrClr.ClrFlag = (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY)? 1 : STR_PosErrClr.ClrFlag ; 

    if(STR_PosErrClr.ClrFlag == 0)   //不清除脉冲差
    {
        STR_InnerGvarPosCtrl.PosErrLastT = STR_InnerGvarPosCtrl.PosErrLast;  //上上周期的位置偏差
        //上一次位置偏差记录 
        STR_InnerGvarPosCtrl.PosErrLast = STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
        //位置偏差计算，放在伺服使能及位置模式外面，servo_off时位置差也生效，以报警显示等用
        STR_FUNC_Gvar.PosCtrl.PosAmplifErr = STR_FUNC_Gvar.PosCtrl.PosRef - STR_FUNC_Gvar.PosCtrl.PosFdb + STR_InnerGvarPosCtrl.PosErrLast;
        
    }
    else                             //清除脉冲差
    {
        STR_InnerGvarPosCtrl.PosErrLastT = 0;
        STR_InnerGvarPosCtrl.PosErrLast = 0;
        STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;    
		ClearFullLoopVar();
        STR_PosErrClr.ClrFlag = 0;   // 清除位置偏差后将位置偏差清楚标志位置零
    }
}

/*******************************************************************************
  函数名: PosRegulator()

  输入:   FunCodeUnion.code.MT_MaxSpd(电机最大转速H00_15)
          STR_FUNC_Gvar.PosCtrl.PosAmplifErr (增量式位置指令偏差)
          STR_InnerGvarPosCtrl.FdFwdOut (位置环速度前馈输出)
  输出:	  STR_FUNC_Gvar.PosCtrl.PosReguOut (位置调节器输出)

  参数：  STR_FUNC_Gvar.GainSW.PosKp (增益切换之后的，位置环Kp)
          PosReguKpCoef_Q15 (位置调节器单位换算系数)

  子函数: SpdFeedForward() (位置环速度前馈)
         
  描述:   位置环调节器P调节加前馈
********************************************************************************/ 

void PosRegulator(void)
{
    int64 Temp = 0; 
    int32 SpdLmt = 0; 
    int64 PosReguOutputValue = 0;
	int32 TempAmplifErr;

    SpdLmt = 10000L * (Uint32)FunCodeUnion.code.MT_MaxSpd;  //对位置调节器输出的速度指令限制值

    /*计算位置调节器输出*/
	//if(FunCodeUnion.code.FC_FeedbackMode == 3)
    //双位置调节功能使能
	if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) && ((STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag == 1) || (FunCodeUnion.code.FC_FeedbackMode == 1)))
    {
		TempAmplifErr = DoublePosFeedbackCal(STR_FUNC_Gvar.PosCtrl.PosRef,&STR_FUNC_Gvar.PosCtrl.PosAmplifErr);		
	}
	else
	{
		TempAmplifErr = STR_FUNC_Gvar.PosCtrl.PosAmplifErr ;//+ STR_FUNC_Gvar.PosCtrl.PosAmplifErrCalibration;
	}
	
	Temp = ((int64)STR_FUNC_Gvar.GainSW.PosKp * STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7 * TempAmplifErr)>>7;
	Temp = (int64)Temp * (int64)PosReguKpCoef_Q15;
	PosReguOutputValue = (Temp>>15); //15+7

    SpdFeedForward();  //调用位置环速度前馈，计算前馈输出

	if((STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag == 1) || (FunCodeUnion.code.FC_FeedbackMode == 1))
	{
		STR_InnerGvarPosCtrl.FdFwdOut = ((int64)STR_InnerGvarPosCtrl.FdFwdOut * STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7)>>7;
	}
	
    STR_FUNC_Gvar.PosCtrl.FdFwdOut = STR_InnerGvarPosCtrl.FdFwdOut;   //位置环速度前馈

    PosReguOutputValue += STR_InnerGvarPosCtrl.FdFwdOut;	 //位置调节器输出在加上位置环速度前馈 

    //位置环输出的速度指令限幅，主要用在位置指令不清除的情况下
    if(PosReguOutputValue > SpdLmt)    //正速度限幅
    {
        PosReguOutputValue = SpdLmt;
    }

    if(PosReguOutputValue < (-SpdLmt))  //负速度限幅		
    {
        PosReguOutputValue = -SpdLmt;
    }
    STR_FUNC_Gvar.PosCtrl.PosReguOut = PosReguOutputValue;
}

//低频振动自动测试
void LowOscSelfCal(void)
{
    static Uint16 OscCnt  = 0;    //检测到4个峰值时位置环周期数
    static Uint16 PeakCnt = 0;    //检测到的峰值个数
    static Uint16 LowFreq[4] = {0};  //低频振动三个频率
    static Uint16 FreqIndex  = 0;  //索引值
    static Uint16 SqcIdenFlag = 0;  //连续辨识标志位，在一次脉冲发送完后，必须等到下次脉冲开始发送时才进行辨识
    Uint16 Temp; 
    
    //功能码的值更新后暂不辨识，直到滤波器初始化完成
    if(STR_InnerGvarPosCtrl.MutexBit.bit.LowOscFlag == 1)
    {
        return;
    }

    if((STR_FUNC_Gvar.PosCtrl.PosRef == 0) && (SqcIdenFlag == 1))   //单次辨识完后，如果脉冲指令为零则不再辨识
    {
        return;
    }
    //位置指令不为0时不进行辨识，但指令不为零时清除不连续辨识标志位
    if(STR_FUNC_Gvar.PosCtrl.PosRef != 0)
    {
        OscCnt      = 0;
        PeakCnt     = 0;
        SqcIdenFlag = 0;
        return;
    }

        //当位置指令为正时，首次偏差需要为负，指令为负时，首次偏差需要为正才开始判定
    if((((PeakCnt == 0) || (PeakCnt == 2)) && (STR_InnerGvarPosCtrl.MutexBit.bit.PosRefDir == 0))
       || (((PeakCnt == 1) || (PeakCnt == 3)) && (STR_InnerGvarPosCtrl.MutexBit.bit.PosRefDir == 1)))
    {
        if((STR_FUNC_Gvar.PosCtrl.PosAmplifErr < 0)     //连续三个周期偏差小于0，且偏差到达最大值时认为到达顶点
           && (STR_InnerGvarPosCtrl.PosErrLast < 0)
           && (STR_InnerGvarPosCtrl.PosErrLastT < 0)
           && (ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < ABS(STR_InnerGvarPosCtrl.PosErrLast))
           && (ABS(STR_InnerGvarPosCtrl.PosErrLast) >= ABS(STR_InnerGvarPosCtrl.PosErrLastT))
           && (ABS(STR_InnerGvarPosCtrl.PosErrLast) >= STR_InnerGvarPosCtrl.LowOscPulsCnt))
        {
            PeakCnt++;
        }
    }
    else     //当位置偏差为正时的判断
    {
        if((STR_FUNC_Gvar.PosCtrl.PosAmplifErr > 0)     //连续三个周期偏差大于0，且偏差到达最大值时认为到达顶点
           && (STR_InnerGvarPosCtrl.PosErrLast > 0)
           && (STR_InnerGvarPosCtrl.PosErrLastT > 0)
           && (STR_FUNC_Gvar.PosCtrl.PosAmplifErr < STR_InnerGvarPosCtrl.PosErrLast)
           && (STR_InnerGvarPosCtrl.PosErrLast >= STR_InnerGvarPosCtrl.PosErrLastT)
           && (STR_InnerGvarPosCtrl.PosErrLast >= STR_InnerGvarPosCtrl.LowOscPulsCnt))
        {
            PeakCnt++;
        }
    }

    if(PeakCnt > 0)     //找到第一个顶点后才开始对时间进行累加
    {
        OscCnt++;
    }


    if(PeakCnt == 4)     //找到了四个顶点，开始计算低频振动频率,注意四个顶点时1.5个振动周期
    {
        LowFreq[FreqIndex] = ((STR_FUNC_Gvar.System.PosFreq * 10 * 3) >> 1) / OscCnt;   //显示带一个小数位

		if((LowFreq[FreqIndex] > 10)&&(LowFreq[FreqIndex] < 1000))	  //检测频率在1.0到100.0之间
        {
			FreqIndex++;
		}
		OscCnt  = 0;     //计数值清零
        PeakCnt = 0;
        SqcIdenFlag = 1;    //本次脉冲发送周期内辨识完成
    }

    if(FreqIndex == 4)   //得到连续四次辨识频率取平均值,每次辨识偏差小于1.5HZ才进行更新
    {
        Temp = (LowFreq[0] + LowFreq[1] + LowFreq[2] + LowFreq[3]) >> 2;     //几次频率的平均值

        if(FunCodeUnion.code.AT_LowOscFreqA != 1000)     //如果当前已经有初始频率，则先调整低频振动滤波参数
        {
            if(FunCodeUnion.code.AT_LowOscFiltA < 10)
            {
                FunCodeUnion.code.AT_LowOscFiltA += 1;      //提高1个等级

                if(FunCodeUnion.code.AT_LowOscFiltA > 10)
                {
                    FunCodeUnion.code.AT_LowOscFiltA = 10;
                }
            }
            else                    //如果滤波常数增加大最大还是能辨识出频率，则开始更新频率
            {
                FunCodeUnion.code.AT_LowOscFreqA = Temp;
                FunCodeUnion.code.AT_LowOscFiltA  = 2;           //滤波还是从第二等级开始
            }
        }
        else
        {
            if((ABS(LowFreq[0] - LowFreq[1]) < 15) && (ABS(LowFreq[1] - LowFreq[2]) < 15)
                && (ABS(LowFreq[2] - LowFreq[3]) < 15))
            {
                FunCodeUnion.code.AT_LowOscFreqA = Temp;
				FunCodeUnion.code.AT_LowOscFiltA  = 2;
            }
        }

		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_LowOscFreqA));     //保存震动频率和宽度
		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_LowOscFiltA));

        FreqIndex = 0;
        STR_InnerGvarPosCtrl.MutexBit.bit.LowOscFlag = 1;       //一次辨识完成
    }

}
/*******************************************************************************
  函数名: SpdFeedForward()

  输入:   FunCodeUnion.code.PL_PosForeBaceSel (H0519,速度前馈选择)
          STR_FUNC_Gvar.PosCtrl.PosRef (滤波后的位置指令输入)
          STR_FUNC_Gvar.AI1/2/3Out   (Ai模拟输入) 
           
  输出:	  STR_InnerGvarPosCtrl.FdFwdOut (速度前馈输出)
   
  参数：  FunCodeUnion.code.GN_SpdFb_Kp (速度前馈增益)
          PosReguFbCoef_Q15 (速度前馈单位换算系数)
             
  子函数: SpdFeedForwardFilter.Filter1() （低通滤波器）
          
  描述:   位置环速度前馈值计算
********************************************************************************/ 

Static_Inline void SpdFeedForward(void)
{
    int64 Temp = 0;

    switch (FunCodeUnion.code.PL_PosForeBaceSel) //位置环速度前馈控制选择H0519，该功能码停机更新 0：没有速度前馈；1：内部速度前馈。2-4：外部速度前馈
    {
        case 0:
            Temp = 0;   //没有前馈
            SpdFeedForwardFilter.Input = (int32)Temp;    //速度前馈输入
            SpdFeedForwardFilter.LowPassFilter(&SpdFeedForwardFilter);  //滤波
            STR_InnerGvarPosCtrl.FdFwdOut = SpdFeedForwardFilter.Output;//滤波结果作为前馈
            break;

        case 1:
            Temp = (int64)PosReguFbCoef_Q15 * (int32)FunCodeUnion.code.GN_SpdFb_Kp;         //速度前馈增益，单位0.1%，在EEPRON中此参数是0~100的值,单位%
            Temp = ((int64)STR_FUNC_Gvar.PosCtrl.PosRef * (int64)Temp)>>15;             //内部速度前馈输出,单位 0.0001rpm 
//            Temp = ((int64)STR_FUNC_Gvar.PosCtrl.PosRef_16K * (int64)Temp)>>15;             //内部速度前馈输出,单位 0.0001rpm 
            SpdFeedForwardFilter.Input = (int32)Temp;    //速度前馈输入
            SpdFeedForwardFilter.LowPassFilter(&SpdFeedForwardFilter);  //滤波
            STR_InnerGvarPosCtrl.FdFwdOut = SpdFeedForwardFilter.Output;//滤波结果作为前馈
            break;

#if ECT_ENABLE_SWITCH
        case 2:
            if(STR_CanSyscontrol.Mode == ECTCSPMOD)
            {
                //采样电压速度增益计算 单位0.0001rpm
                Temp = (int64)UserVelUnit2RpmUnit(&STR_VelEnc_Factor,(int32)STR_CmdBufferVar.CSP_SpdRef);
                //if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x40)== 0x40)Temp = -Temp;
            }
            else
            {
                Temp = 0;
            }
            STR_InnerGvarPosCtrl.FdFwdOut = Temp;
            break;
#else
        case 2:
            //采样电压速度增益计算 单位0.0001rpm
            Temp = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain  >> 15;
            Temp *= STR_FUNC_Gvar.AI.AI1VoltOut;
            Temp = (Temp * (int32)FunCodeUnion.code.GN_SpdFb_Kp)/1000;   //将AI1用作速度前馈输入
            break;
            
        case 3:
            //采样电压速度增益计算 单位0.0001rpm
            Temp = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain  >> 15;
            Temp *= STR_FUNC_Gvar.AI.AI2VoltOut;
            Temp = (Temp * (int32)FunCodeUnion.code.GN_SpdFb_Kp)/1000;   //将AI2用作速度前馈输入
            break;
#endif
//        case 4:
//            Temp = ((STR_FUNC_Gvar.AI.AI3VoltOut * 120000L) * (Uint16)FunCodeUnion.code.AI_SpdGain ) >> 15;
//            Temp = (Temp * (int32)FunCodeUnion.code.GN_SpdFb_Kp)/1000;   //将AI2用作速度前馈输入
//            break;

        default : break;
    }
}

/*******************************************************************************
  函数名: void ClrReguAndFeedForward() 
  输入:                              
  输出:     
  子函数:         
  描述: 位置调节器及位置随动偏差相关清零  
********************************************************************************/
void ClrPosErrAndRegulator(void)
{
    /*位置随动偏差清零*/
//    STR_FUNC_Gvar.PosCtrl.PosRmned     = 0;
    STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;    //当前一次的脉冲偏差，由脉冲差清除功能确定什么时候清除
    STR_InnerGvarPosCtrl.PosErrLast = 0;       //前馈速度值及AO输出
    
    /*位置调节器输出，及速度前馈输出清零*/ 
//    STR_FUNC_Gvar.PosCtrl.PosRef_16K = 0; 
    STR_InnerGvarPosCtrl.FdFwdOut = 0;          //前馈速度值及AO输出   
    STR_FUNC_Gvar.PosCtrl.PosReguOut = 0;       //位置调节器输出的速度指令

    ResetLowPassFilter(&SpdFeedForwardFilter);  //速度前馈滤波器清零

    //速度模式切换到位置模式时的补偿 Spd -> Pos 切换时的补偿
    if( STR_FUNC_Gvar.MonitorFlag.bit.SpdSwitchPosOffset == 1 ) //先运行第一步清零
    {
         //根据当前速度反馈进行补偿，需注意速度指令和反馈都是0.0001rpm，且位置调节器定标系数PosReguKpCoef_Q15
         //的计算中也进行了四位小数的处理。
		STR_FUNC_Gvar.PosCtrl.PosAmplifErr =0;
		
		//         STR_FUNC_Gvar.PosCtrl.PosAmplifErr = ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb << 15) /
//                                              ((int64)PosReguKpCoef_Q15 * (int64)STR_FUNC_Gvar.GainSW.PosKp);
//         STR_FUNC_Gvar.PosCtrl.PosAmplifErr = ((int64)STR_FUNC_Gvar.PosCtrl.PosAmplifErr <<7)/ STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7;
         //进行一次补偿之后，下次不再补偿
         STR_FUNC_Gvar.MonitorFlag.bit.SpdSwitchPosOffset = 0;
    }

    if ( STR_FUNC_Gvar.MonitorFlag.bit.ToqSwitchPosOffset == 1)
    {
        STR_FUNC_Gvar.PosCtrl.PosAmplifErr =0;
        //根据当前转矩反馈进行补偿，先根据转矩反馈算出速度后在计算位置随动误差。
//        STR_FUNC_Gvar.PosCtrl.PosAmplifErr = ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb << 15) /
//                                             ((int64)PosReguKpCoef_Q15 * (int64)STR_FUNC_Gvar.GainSW.PosKp);
        //进行一次补偿之后，下次不再补偿
        STR_FUNC_Gvar.MonitorFlag.bit.ToqSwitchPosOffset = 0;
    }

}
/********************************* END OF FILE *********************************/

