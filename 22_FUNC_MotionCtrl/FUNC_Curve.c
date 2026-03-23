/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_Curve.c
 创建人：张小华               创建日期：2012.09.05
 修订人：姚虹                 修改时间: 2014.10.09    
 描述： 
     1.JOG与定位试运行伺服实现

 修改记录：  
     1.根据上位机的Jog与定位试运行操作调整，修改了相应代码

******************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h" 
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_Curve.h"
#include "FUNC_LineInterplt.h"
#include "FUNC_MultiBlockPos.h"
#include "FUNC_ServoMonitor.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define       ENABLE      1
#define       DISABLE     0

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
str_CURVE  STR_Curve;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
STR_CURVE_BLOCK_POS       STR_CurvePCMD   = CurvePCMD_Defaults;
STR_LINE_INTERPLT_ATTRIB  STR_CbcLnIntplt = LineInterpltAttribDflts;
static Uint32 PosJogRPM2PPPtCoefQ16;
Uint16  ContrlMode[]={SPDMOD,POSMOD,TOQMOD};
extern STR_LINE_INTERPLT_ATTRIB*    pIntplt[4];
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   
void CurveInit(void)        ;   //变量初始化函数
void CurveDataUp(void)      ;
void CurveSetServo(void);    //给伺服的配置参数
void CurveStopDataUp(void)  ;
void CurveReset(void)       ;   
int32 JogCmdGenerator(void) ;   //JOG运动脉冲产生
int32 CurveHandle(void)     ;   //定位试运行脉冲产生
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
// JOG速度设定
Static_Inline void GetJOGFreq(void);

// 正反转极限位置设定
Static_Inline void GetCWCCWPosLimit(void);

// 获得定位试运行的位置指令
Static_Inline void GetPositionCmd(void);

Static_Inline void CurLineIntpltInit(int32 StartSpeed, int32 RunSpeed, int32 StopSpeed,
                    Uint32 UpTime, Uint32 DownTime, int64 LineDist,
					STR_LINE_INTERPLT_ATTRIB *pAtb);

//加减速距离计算
Static_Inline int64 CurAccDistCalc(int32 StartPulseQ16, int32 StopPulseQ16, int32 PulseRev);
/*******************************************************************************
  函数名: CurveSetServo() 
  输  入:          
  输  出:   
  子函数: 无                                      
  描  述: 上位机给伺服的配置信息。
********************************************************************************/
void CurveSetServo(void)
{
	//确定是Jog定位还是位置试运行
	STR_Curve.PulseSendSel = AuxFunCodeUnion.code.CurveSel;
	
	//设置极限位置
	if(AuxFunCodeUnion.code.CurvePNlimitSet == 0)
	{
	    STR_Curve.CWCCWMaxSet = DISABLE;
	}
	else
	{
	    STR_Curve.CWCCWMaxSet = ENABLE;
	}    
}



/*******************************************************************************
  函数名: CurveDataUp() 
  输  入:          
  输  出:   
  子函数: GetJOGFreq(),GetCWCCWPosLimit(),GetPositionCmd()                                      
  描  述: 适时更新指令，限制信息。
********************************************************************************/
void CurveDataUp(void)
{       	
    if (1 == STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork)
	{
		//上位机给伺服的配置参数
		CurveSetServo();
	
	    //Jog参数更新
   	    if(DISABLE == STR_Curve.PulseSendSel)
	    {		
		    //根据设置的点动速度和加减速时间信息计算每位置环周期对应的脉冲数
		    GetJOGFreq();	   	
	    }
	    else 
	    {
		    //位置指令获取
		    GetPositionCmd();
	    }

		//给上位机的正反极限位置及当前位置
	    GetCWCCWPosLimit();
	}
	else
	{
	    STR_Curve.JogPulseSend  = 0;     //指示当前位置
		STR_Curve.CCWCoordinate = 0;	 //指示极限位置
		STR_Curve.CWCoordinate  = 0;
	}
}
/*******************************************************************************
  函数名: CurveStopDataUp() 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: ServoOFF清除功能码及变量。
********************************************************************************/
void CurveStopDataUp(void)
{
	AuxFunCodeUnion.code.CurvePNlimitSet = 0 ;//正反转极限位置设定清除
	AuxFunCodeUnion.code.CurveRunMode    = 0 ;//单次运行
	AuxFunCodeUnion.code.CurveRunDirSet  = 0 ;//停止脉冲定位试运行脉冲发送

	STR_Curve.EleGearRem            = 0;
    STR_Curve.PlanIntpltValQ16      = 0;
	STR_Curve.PlanIntpltValQ16Rem   = 0;
	STR_CurvePCMD.RefleshNum    = 0;
}
/*******************************************************************************
  函数名: CurveInit() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: JOG运动与定位试运行变量初始化模块。
********************************************************************************/
void CurveInit(void)
{
    STR_Curve.PlanIntpltValQ16    = 0;
    STR_Curve.PlanIntpltValQ16Rem = 0;
	STR_Curve.EleGearRem          = 0;
	AuxFunCodeUnion.code.CurveSel = 0;

	AuxFunCodeUnion.code.GUIWorkMode  = GUIRECOVER; 
	
	if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)      //直线电机1mm/s在每个位置环周期对应的脉冲数
    {
        PosJogRPM2PPPtCoefQ16 = ((Uint64)1000*100 << 16)
                                        / (STR_FUNC_Gvar.System.PosFreq * FunCodeUnion.code.MT_EncoderPensH);
		STR_Curve.Numerator   = 1;
	    STR_Curve.Denominator = 1;

    }
    else           //1rpm在每个位置环周期对应的圈数(0.0001rev)   	//获取后台齿轮比
    {
        PosJogRPM2PPPtCoefQ16 = (10000L << 16)
		                                 / (60* STR_FUNC_Gvar.System.PosFreq);
		STR_Curve.Numerator   = UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
		STR_Curve.Denominator = 10000;						    
    }
    
    // 记录定位试运行插补结构变量 STR_CbcLnIntplt 的地址
    RecordIntpltStruct(&STR_CbcLnIntplt, 3);    
}


/*******************************************************************************
  函数名: void CurveReset() 
  输  入: STR_CbcLnIntplt         
  输  出:   
  子函数: LineIntpltReset()                                      
  描  述: 定位试运行复位插补结构
********************************************************************************/
void CurveReset(void)
{
    // 直线插补器参数复位	
    if (1 == STR_CbcLnIntplt.IntpltStatus)
	{
        LineIntpltReset(&STR_CbcLnIntplt);
	}
}

/*******************************************************************************
  函数名: GetJOGFreq() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 计算位置环JOG速度的方向，大小，加速度及其对应的每位置环周期脉冲数
********************************************************************************/
Static_Inline void GetJOGFreq(void)
{
	STR_Curve.MaxPulse = (Uint64)ACCELERATESPEED * PosJogRPM2PPPtCoefQ16;

	if(1==AuxFunCodeUnion.code.JogDir)
	{
	    STR_Curve.RunSpeed = (int16)AuxFunCodeUnion.code.JogSpeedRef;
	    STR_Curve.JogRun   = CCWRUN;
	}
	else if(2==AuxFunCodeUnion.code.JogDir)
	{
	    STR_Curve.RunSpeed =-(int16)AuxFunCodeUnion.code.JogSpeedRef;
		STR_Curve.JogRun   = CWRUN;
	}
	else
	{
	    STR_Curve.RunSpeed = 0;
		STR_Curve.JogRun   = STOP;
	}
	
	STR_Curve.RiseDownTime = ((int64)STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * AuxFunCodeUnion.code.JogRiseDownTime)>>13;
	if(STR_Curve.RiseDownTime==0)
  	{
	    STR_Curve.RiseDownTime = 1;
    }

	//计算运行速度、最高速度对应的频率，Jog按键松掉时减速时间
    STR_Curve.PulseFreqQ16 = (int32)STR_Curve.RunSpeed * PosJogRPM2PPPtCoefQ16;

    STR_Curve.UpPulseQ16   = STR_Curve.MaxPulse/STR_Curve.RiseDownTime; 
}

/*******************************************************************************
  函数名: GetCWCCWPosLimit() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 上位机获取正反转极限位置设定值。
********************************************************************************/
Static_Inline void GetCWCCWPosLimit(void)
{	
	//根据上位机信息，清除当前位置坐标
	if(1 == AuxFunCodeUnion.code.CurveClrPulseSend)//发送脉冲清零
    {
	    STR_Curve.JogPulseSend  = 0;	//指示当前位置
		STR_Curve.CCWCoordinate = 0;	 //指示极限位置
		STR_Curve.CWCoordinate  = 0;		    
		AuxFunCodeUnion.code.CurvePNlimitSet   = 0; //正反转极限位置不限制
		AuxFunCodeUnion.code.CurveClrPulseSend = 0; //每次脉冲清除只进行一次
	}	
	
	//设定正反极限位置
	AuxFunCodeUnion.code.CurveCoordCCWHigh  = (Uint16)((STR_Curve.CCWCoordinate>>16)&0xffff);
	AuxFunCodeUnion.code.CurveCoordCCWLow   = (Uint16)(STR_Curve.CCWCoordinate&0xffff);
	AuxFunCodeUnion.code.CurveCoordCWHigh   = (Uint16)((STR_Curve.CWCoordinate>>16)&0xffff);
	AuxFunCodeUnion.code.CurveCoordCWLow    = (Uint16)(STR_Curve.CWCoordinate&0xffff);
	AuxFunCodeUnion.code.CurvePulseSendHigh = (Uint16)((STR_Curve.JogPulseSend>>16)&0xffff);
	AuxFunCodeUnion.code.CurvePulseSendLow  = (Uint16)(STR_Curve.JogPulseSend&0xffff);
}

/*******************************************************************************
  函数名: GetPositionCmd() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 获取定位试运行指令：速度、脉冲数、加速度、等待时间
********************************************************************************/
Static_Inline void GetPositionCmd(void)
{
	if(AuxFunCodeUnion.code.CurveRunDirSet==CCWRUN)
	{
	    STR_Curve.MoveDist = (Uint32)A_SHIFT16_PLUS_B(AuxFunCodeUnion.code.CurveMoveDisSetHigh,AuxFunCodeUnion.code.CurveMoveDisSetLow);
	}
	else if(AuxFunCodeUnion.code.CurveRunDirSet==CWRUN)
	{
	    STR_Curve.MoveDist =-(int32)A_SHIFT16_PLUS_B(AuxFunCodeUnion.code.CurveMoveDisSetHigh,AuxFunCodeUnion.code.CurveMoveDisSetLow);
	}
	else 
	{
		STR_Curve.MoveDist = 0;
	}
	
	//做两段多段运行
	STR_CurvePCMD.RunMode            = AuxFunCodeUnion.code.CurveRunMode;
	STR_CurvePCMD.ExeBlockNum        = 2;
	STR_CurvePCMD.WaitTimePerBlock[0]= 0;
	STR_CurvePCMD.PulsNumPerBlock[0] = STR_Curve.MoveDist;
	STR_CurvePCMD.RunSpeed[0]        = AuxFunCodeUnion.code.CurveMoveSpeedMax;
	STR_CurvePCMD.RiseorDownTime[0]  = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 
	                                     * AuxFunCodeUnion.code.CurveMoveRiseDownTime)>>13;
	STR_CurvePCMD.WaitTimePerBlock[1] = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13
	                                      *AuxFunCodeUnion.code.WaitTime)>>13;

	STR_CurvePCMD.PulsNumPerBlock[1] =-STR_Curve.MoveDist;
	STR_CurvePCMD.RunSpeed[1]        = AuxFunCodeUnion.code.CurveMoveSpeedMax;
	STR_CurvePCMD.RiseorDownTime[1]  =(STR_InnerGvarPosCtrl.Ms2PtCoefQ13 
	                                  * AuxFunCodeUnion.code.CurveMoveRiseDownTime)>>13;
	STR_CurvePCMD.WaitTimePerBlock[2] =(STR_InnerGvarPosCtrl.Ms2PtCoefQ13 
	                                  *AuxFunCodeUnion.code.WaitTime)>>13;

	//先确保位置指令赋值成功，再开始执行多段
  	if(AuxFunCodeUnion.code.CurveRunDirSet==CCWRUN)
	{
		STR_Curve.PulseSendEn   = ENABLE;
	}
	else if(AuxFunCodeUnion.code.CurveRunDirSet==CWRUN)
	{
		STR_Curve.PulseSendEn   = ENABLE;
	}
	else 
	{
	    STR_Curve.PulseSendEn   = DISABLE;
	}
}

/*******************************************************************************
  函数名: CurLineIntpltInit
  输  入: StartSpeed - 起步速度, RunSpeed - 运行速度, StopSpeed - 停止速度
		  UpTime - 加速时间,     DownTime - 减速时间, LineDist - 总位移长度
		  参数 UpTime, DownTime 都以插补周期为单位
  输  出:    
  子函数: CurAccDistCalc()计算加(减)速段的长度
  描  述: 定位试运行中定长直线插补初始化 
********************************************************************************/ 
Static_Inline void CurLineIntpltInit(int32 StartSpeed, int32 RunSpeed, int32 StopSpeed,
                    Uint32 UpTime, Uint32 DownTime, int64 LineDist,
					STR_LINE_INTERPLT_ATTRIB *pAtb)
{
    Uint16 i = 0;                // 循环计数变量
	int64 MaxPulse = 0;          // 电机最大速度对应的每个插补周期脉冲数
	int32 DecMinDist = 0;        // 计算开始速度大于终止速度时计算最小减速距离
	
	pAtb->IntpltStatus = 0;  // 插补状态置0, 表示当前并未插补
	
	if(RunSpeed > FunCodeUnion.code.MT_MaxSpd)
	{   // 检查所选运行速度是否大于伺服系统当前最大速度
		RunSpeed = 	FunCodeUnion.code.MT_MaxSpd;
	}
	
        
    // 齿轮比改成实时作用, 即齿轮比将在插补完成之后作用又为了用户设置的速度不因齿
    // 轮比而变化需要将插补时各个速度乘上齿轮比的倒数
    // 计算匀速段每个插补周期的脉冲个数 Q16	
	    
	pAtb->AvergePulseQ16 = (int64)RunSpeed * PosJogRPM2PPPtCoefQ16;
	 
	// 计算起动速度对应的每个插补周期脉冲个数 Q16
	pAtb->StartPulseQ16 = (int64)StartSpeed * PosJogRPM2PPPtCoefQ16;
					 
    // 计算停止速度对应的每个插补周期脉冲个数 Q16
	pAtb->StopPulseQ16 = (int64)StopSpeed * PosJogRPM2PPPtCoefQ16;
					
	// 计算电机最大速度对应的每个插补周期脉冲个数 Q16
	MaxPulse = (int64)SPDUPDOWM_MAXREF * PosJogRPM2PPPtCoefQ16;

	// 根据加速时间 UpTime 计算加速段每个插补周期的脉冲增量
	if(UpTime) // UpTime的单位已经是插补周期
	{
		// 以电机能达到的最大速度而不是用户设置的运行速度来计算每个插补周期的脉冲增量
		// 因此所得结果是这个加速时间下能达到的每个插补周期的最大脉冲增量
		if(pAtb->StartPulseQ16 < pAtb->AvergePulseQ16) 
		{   // 目标速度比起步速度大, 递增量为正
			pAtb->UpPulseRevQ16 = (int32)(MaxPulse / UpTime);
			if(pAtb->UpPulseRevQ16==0)
			{
				pAtb->UpPulseRevQ16 = 1;
			}
		}
		else
		{   // 目标速度比起步速度小, 递增量为负数
			pAtb->UpPulseRevQ16 = 0 - (int32)(MaxPulse / UpTime);		   
			if(pAtb->UpPulseRevQ16==0)
			{
				pAtb->UpPulseRevQ16 = -1;
			}
		}
	}
	else
	{
		pAtb->UpPulseRevQ16 = 0;
	}

	// 根据减速时间 DownTime 计算减速段每个插补周期的脉冲减量, 与加速段的计算方法相同
	if(DownTime) // DownTime的单位已经是插补周期
	{
		if(pAtb->AvergePulseQ16 < pAtb->StopPulseQ16) 
		{   // 目标速度比起步速度大, 递减量为正
			pAtb->DownPulseRevQ16 = (int32)(MaxPulse / DownTime);
			if(pAtb->DownPulseRevQ16==0)
			{
				pAtb->DownPulseRevQ16 = 1;
			}
		}
		else
		{   // 目标速度比起步速度小, 递减量为正
			pAtb->DownPulseRevQ16 = 0- (int32)(MaxPulse / DownTime);
			if(pAtb->DownPulseRevQ16==0)
			{
				pAtb->DownPulseRevQ16 = -1;
			}
		}
	}
	else
	{
		pAtb->DownPulseRevQ16 = 0;
	}

	// 插补长度用绝对值表示这里保存长度值的方向(正负号), 待每个插补周期的脉冲数
	// 计算出来后, 再将方向补上
	if(LineDist > 0)
	{
		pAtb->LineDir = 1;
		pAtb->LineLengthQ16 = LineDist << LINEAMPBIT;
	}
	else if(LineDist < 0)
	{
		pAtb->LineDir = -1;
		pAtb->LineLengthQ16 = 0 - (LineDist << LINEAMPBIT);
	}
	else
	{
		pAtb->LineLengthQ16 = 0;
	}
	
	//计算加速段长度
	pAtb->UpLengthQ16=CurAccDistCalc(pAtb->StartPulseQ16, pAtb->AvergePulseQ16, pAtb->UpPulseRevQ16);

	// 计算减速段长度
	pAtb->DownLengthQ16= CurAccDistCalc(pAtb->AvergePulseQ16, pAtb->StopPulseQ16, pAtb->DownPulseRevQ16);

	if((pAtb->UpLengthQ16 + pAtb->DownLengthQ16) > pAtb->LineLengthQ16)
	{   
	    // 加速段和减速段长度之和大于总长度, 此时先计算减速段最小长度
		if(pAtb->StartPulseQ16 > pAtb->StopPulseQ16)
		{
		    DecMinDist = CurAccDistCalc(pAtb->StartPulseQ16, pAtb->StopPulseQ16, pAtb->DownPulseRevQ16);
		}
        else
		    DecMinDist = 0;

		if(pAtb->LineLengthQ16 < DecMinDist)
		{   // 如果减速段最小长度仍然大于总长度
		    pAtb->UpLengthQ16 = 0;                  // 将加速段置0
		    pAtb->AvergeLengthQ16 = 0;              // 将匀速段置0
			pAtb->DownLengthQ16 = pAtb->LineLengthQ16; // 在整段一直减速
		}
		else // 总长度不小于最小减速段长度
		{   // 重置加速段减速段长度值, 将匀速段置0
		    pAtb->UpLengthQ16 = (pAtb->LineLengthQ16 - DecMinDist) >> 1;
		    pAtb->DownLengthQ16 = pAtb->LineLengthQ16 - pAtb->UpLengthQ16;
		    pAtb->AvergeLengthQ16 = 0;
		}
	}
	else // 总长度不小于加速段和减速段长度之和
	{   // 此时存在匀速段
		pAtb->AvergeLengthQ16 = pAtb->LineLengthQ16 - pAtb->UpLengthQ16 - pAtb->DownLengthQ16;
	}
	
	pAtb->LineRemainLengthQ16 = pAtb->LineLengthQ16; // 设置剩余插补长度为总长度
	pAtb->PlanIntpltValQ16 = pAtb->StartPulseQ16;    // 起步时每插补周期脉冲个数,Q6
	pAtb->PlanValueRemainQ16 = 0;                 // 当前插补周期脉冲剩余个数
	pAtb->RealIntpltVal = pAtb->StartPulseQ16>>LINEAMPBIT; // 本插补周期脉冲个数

	if(pAtb->LineRemainLengthQ16 > 0)
	{
	    pAtb->IntpltStatus = 1;   // 置插补状态值为1, 起动插补
		pAtb->PlanDecAgain = 1;   // 再规划减速段的标志
	}
	
	// 目前有三处地方调用了插补功能, 当某处抢占使用时, 其余的插补结构需要复位
    // u16Size = sizeof(pIntplt) / sizeof(pIntplt[0]);
	for (i = 0; i < 4; i++)
	{
	    if (0 != pIntplt[i])
		{
	        if (pAtb != pIntplt[i])
			{
				pIntplt[i]->IntpltStatus = 0;
				pIntplt[i]->LineRemainLengthQ16 = 0;
			}
		}
	}
}
/*******************************************************************************
  函数名: void CurAccDistCalc()
  输  入: StartPulseQ16 - 起点处每个插补周期的插补脉冲数 
          StopPulseQ16  - 终点处每个插补周期的插补脉冲数
		  PulseRev   - 每个插补周期脉冲递增(减)量(加/减速度)
  输  出: 加(减)速段的长度(以脉冲为单位)   
  子函数:                                       
  描  述: 根据起点处每周期插补脉冲数StartPulse, 终点处每周期插补脉冲数EndPulse
          以及每个插补周期脉冲递增量, 计算加(减)速段的长度
		  如果增(减)量 PulseRev等于0, 则认为加(减)速段距离也为0
********************************************************************************/ 
Static_Inline int64 CurAccDistCalc(int32 StartPulseQ16, int32 StopPulseQ16, int32 PulseRev)
{
	int64 Dist = 0;
	Uint32 RunCount; 

	// 计算从起点速度到终点速度的插补周期数
	// 注意: 插补开始时起步速度是 StartPulseQ16 + PulseRev
    if(PulseRev)    
    {
		RunCount = (StopPulseQ16 - StartPulseQ16) / PulseRev + 1;
	}
	else
	{
	    RunCount = 0; // 此时认为速度无需改变, 相应加速段距离也为0
	}

    // 计算加(减)速段长度
	Dist = ((int64)RunCount * (StartPulseQ16 + ((int64)PulseRev * (RunCount-1)>>1)));	

    return Dist ;
}
/*******************************************************************************
  函数名: JogCmdGenerator() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: JOG运动，脉冲指令发送。
********************************************************************************/
int32 JogCmdGenerator(void)
{
    int32 temp11;
	int64 temp111; 

	if(CCWRUN == STR_Curve.JogRun)      //正转键按下，正向加速过程
	{
	    if(STR_Curve.PlanIntpltValQ16 <= STR_Curve.PulseFreqQ16)
		{
		    STR_Curve.PlanIntpltValQ16 += STR_Curve.UpPulseQ16;
			if(STR_Curve.PlanIntpltValQ16 >= STR_Curve.PulseFreqQ16) 
			{
			    STR_Curve.PlanIntpltValQ16 = STR_Curve.PulseFreqQ16;    
			} 
		}
		else 
		{
		    STR_Curve.PlanIntpltValQ16 = STR_Curve.PulseFreqQ16; 
		}  
	}
	else if(CWRUN == STR_Curve.JogRun)  //反转键按下，反向加速过程
	{
	    if(STR_Curve.PlanIntpltValQ16 > STR_Curve.PulseFreqQ16)
		{
		    STR_Curve.PlanIntpltValQ16 -= STR_Curve.UpPulseQ16;
			if(STR_Curve.PlanIntpltValQ16 <= STR_Curve.PulseFreqQ16) 
			{
			    STR_Curve.PlanIntpltValQ16 = STR_Curve.PulseFreqQ16;    
			} 
		}
		else 
		{
		    STR_Curve.PlanIntpltValQ16 = STR_Curve.PulseFreqQ16; 
		}  
	} 
	else	                 //无按键按下时需要考虑减速过程
	{
	    if(STR_Curve.PlanIntpltValQ16 > 0)
        {
            if(STR_Curve.PlanIntpltValQ16 > STR_Curve.UpPulseQ16)      //正向减速    
            {
                STR_Curve.PlanIntpltValQ16 -= STR_Curve.UpPulseQ16;
            }
            else
            {
                STR_Curve.PlanIntpltValQ16     = 0;
                STR_Curve.PlanIntpltValQ16Rem  = 0;
            }
        }
        else if(STR_Curve.PlanIntpltValQ16 < 0)                 
        {
            if(STR_Curve.PlanIntpltValQ16 < (-STR_Curve.UpPulseQ16))     //反向减速
            {
                STR_Curve.PlanIntpltValQ16 += STR_Curve.UpPulseQ16;
            }
            else                                               
            {
                STR_Curve.PlanIntpltValQ16     = 0;
                STR_Curve.PlanIntpltValQ16Rem  = 0;
            }            
        }
        else
        {
            STR_Curve.PlanIntpltValQ16     = 0;
            STR_Curve.PlanIntpltValQ16Rem  = 0;            
        }
	}

	temp11                        = STR_Curve.PlanIntpltValQ16 + STR_Curve.PlanIntpltValQ16Rem ;
	STR_Curve.JogOut              = temp11>>JOGAMPBIT                                          ;
	STR_Curve.PlanIntpltValQ16Rem = temp11 - ((int32)STR_Curve.JogOut<<JOGAMPBIT)              ;

	//JOG正反极限位置限制
	if(STR_Curve.CWCCWMaxSet == DISABLE)     //极限位置未确定时可以扩展当前运行位置和极限位置
	{
	    STR_Curve.JogPulseSend += STR_Curve.JogOut; 

		if(STR_Curve.JogPulseSend > STR_Curve.CCWCoordinate)   //当前位置大于正极限时
		{
		    STR_Curve.CCWCoordinate = STR_Curve.JogPulseSend;
		}
		else if(STR_Curve.JogPulseSend < STR_Curve.CWCoordinate)  //当前位置小于负极限时
		{
		    STR_Curve.CWCoordinate = STR_Curve.JogPulseSend;
		}
	}
	else		   //极限位置确定时需要判定当前发送脉冲是否超出了极限位置
	{
	    temp11 =  STR_Curve.JogPulseSend + STR_Curve.JogOut;
	    if(temp11 >= STR_Curve.CCWCoordinate)
	    {
		    STR_Curve.JogOut = STR_Curve.CCWCoordinate - STR_Curve.JogPulseSend;
		    STR_Curve.PlanIntpltValQ16Rem = 0;
	    }
	    else if(temp11 <= STR_Curve.CWCoordinate)
	    {
		    STR_Curve.JogOut = STR_Curve.CWCoordinate - STR_Curve.JogPulseSend;
		    STR_Curve.PlanIntpltValQ16Rem = 0;
	    }
	
	    STR_Curve.JogPulseSend += STR_Curve.JogOut;
	}

	//将计算出的转数转换成编码器脉冲数发送
	temp111              = STR_Curve.JogOut * (int64)STR_Curve.Numerator + STR_Curve.EleGearRem;
	STR_Curve.JogOut     = temp111 / (int64)STR_Curve.Denominator;
	STR_Curve.EleGearRem = temp111 - (int64)STR_Curve.Denominator * STR_Curve.JogOut;
    return (STR_Curve.JogOut);
}

/*******************************************************************************
  函数名: CurveHandle() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 在定位试运行模式，根据设定生成脉冲指令。
********************************************************************************/
int32 CurveHandle(void)
{
    static Uint16 s_OldCurveEnValue = DISABLE;
	static Uint32 s_counter         = 0      ;
	       Uint16 isRealActive      = 0      ;
		   Uint32 downTime=0;   // 从指定速率减速到停止速率0的减速时间
           Uint32 riseTime=0;   // 从0起始速度加速到指定速率的加速时间
           int64  moveDist=0;   // 当前段位置长度(指令脉冲)
           int32  runSpeed=0;   // 匀速时的速度
           int32  startSpeed;   // 起始速度
		   int32  temp11;
		   int64  temp111;


	STR_CURVE_BLOCK_POS      *pCbc   = &STR_CurvePCMD;
	STR_LINE_INTERPLT_ATTRIB *pCIntp = &STR_CbcLnIntplt;

	if ((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == 2)
		&& (STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD))
    {
		if ((s_OldCurveEnValue != STR_Curve.PulseSendEn) && (DISABLE == s_OldCurveEnValue))
		{
		    isRealActive  = 1;   //曲线规划使能启动
			pCbc->RunFlag = 1;   //曲线规划运行标志位置1
			//if(pCbc->RefleshNum >= pCbc->ExeBlockNum)
            //{
                pCbc->RefleshNum = 0;
			//}
			pCbc->WaitFlag = 1;
            //20150119 市场反馈 等待时间设置为上限时 触发后无法立即运行
            //初始值按16K调度计算
			s_counter = 0xffff0000;//10000001L * 16L; //避免10000 S的对比失效
		}
		else if((s_OldCurveEnValue != STR_Curve.PulseSendEn) && (ENABLE == s_OldCurveEnValue))
		{
		    isRealActive  = 2;
		}
		else 
		{
		    isRealActive  = 0;
		}
		s_OldCurveEnValue = STR_Curve.PulseSendEn;
		startSpeed = CURVE_STARTSPD;  // 获取当前段起步速度
		switch (pCbc->RunMode)     //根据运行模式处理切换
	    {
	        case ONCE_Time:        //单次运行
	        {
				// 断使能处理
				if ((isRealActive == 2 ) && (pCIntp->IntpltStatus == 1))
				{   //断使能时正在插补
                    ResetIntpltDownLength(pCIntp);
					pCIntp->LineRemainLengthQ16 = pCIntp->DownLengthQ16;
					break;
				}
	          			
	            if ((0 == pCIntp->IntpltStatus) && (1 == pCbc->RunFlag)
                    && (ENABLE == STR_Curve.PulseSendEn))
	            {   // 前段运行结束须满足如下四个条件:
				    // 插补状态, 多段位置运行标志为TRUE,
					// 多段位置使能信号为有效, 当前位置随动误差小于定位误差
					if (!pCbc->WaitFlag) //等待处理
	                {
	                    pCbc->WaitFlag = 1;  // 多段位置等待标志置1
						s_counter = 0;       //开始计算等待时间
	                }
	                
					if (++s_counter < pCbc->WaitTimePerBlock[pCbc->RefleshNum])
	                {   //等待时间没到
						break;
	                }

	                //结束等待处理
	                pCbc->WaitFlag = 0;
	                // s_counter = 0;
	            						
	                if (pCbc->RefleshNum >= pCbc->ExeBlockNum)
	                {   // 已执行的段数 不小于设置的总执行段数
	                    pCbc->RefleshNum     = 0;       // 已执行的段数置0
	                    pCIntp->IntpltStatus = 0; // 插补状态置0
	                    pCbc->RunFlag        = 0;          // 单循环，运行一遍就停止
						break;
	                }
	                else
	                {
						moveDist = pCbc->PulsNumPerBlock[pCbc->RefleshNum];
						riseTime = pCbc->RiseorDownTime[pCbc->RefleshNum]; // 获取当前段加速时间
						downTime = riseTime;                               // 获取当前段减速时间
						runSpeed = pCbc->RunSpeed[pCbc->RefleshNum];       // 获取当前段匀速运行速度
						
						//启动下一段插补
	                    CurLineIntpltInit(startSpeed, runSpeed, CURVE_STOPSPD, riseTime, downTime,
						               moveDist, pCIntp);
			            pCbc->CurrentBlock =  pCbc->RefleshNum;   
	                }
	                
	                pCbc->RefleshNum++; // 更新段标号
	            }  // end if () 前段运行结束判断
	        } // end case ONCE_TYPE: //单次运行
	        break;
	        
	        case CYCLE: // 循环运行
	        {			 	 
				// 断使能处理
				if ((isRealActive == 2 ) && (pCIntp->IntpltStatus == 1))
				{   //断使能时正在插补
					ResetIntpltDownLength(pCIntp);
					pCIntp->LineRemainLengthQ16 = pCIntp->DownLengthQ16;
					break;
				}

	            if ((0 == pCIntp->IntpltStatus)
				    && (ENABLE == STR_Curve.PulseSendEn))
	            {	//前段结束了
	                //等待处理
	                if (!pCbc->WaitFlag)
	                {
	                    pCbc->WaitFlag = 1;
						s_counter = 0; //等待计时开始
	                }

					if (++s_counter < pCbc->WaitTimePerBlock[pCbc->RefleshNum])
	                {   //等待时间没到
	                    break;
	                }
					
	                //结束等待处理
	                pCbc->WaitFlag = 0;
	                // s_counter = 0;
	             
	                if(pCbc->RefleshNum >= pCbc->ExeBlockNum)
	                {
	                    pCbc->RefleshNum = 0;	 //从头开始                  
	                }
					moveDist = pCbc->PulsNumPerBlock[pCbc->RefleshNum];
				    riseTime = pCbc->RiseorDownTime[pCbc->RefleshNum]; // 获取当前段加速时间
					downTime = riseTime;                               // 获取当前段减速时间
					runSpeed = pCbc->RunSpeed[pCbc->RefleshNum];   // 获取当前段匀速运行速度

					//启动下一段
	                CurLineIntpltInit(startSpeed, runSpeed, CURVE_STOPSPD, riseTime, downTime,
					               moveDist, pCIntp);    
			        pCbc->CurrentBlock =  pCbc->RefleshNum;  //记录当前段             
	                pCbc->RefleshNum++;  // 更新段标号
	            }
	        }
	        break;
	    }
	}
    	
	STR_Curve.JogOut = LineIntplt(pCIntp);
	
	//JOG正反极限位置限制
	temp11 =  STR_Curve.JogPulseSend + STR_Curve.JogOut;
	if((temp11>=STR_Curve.CCWCoordinate)&&(STR_Curve.CWCCWMaxSet == ENABLE))
	{
		STR_Curve.JogOut = STR_Curve.CCWCoordinate - STR_Curve.JogPulseSend;
	}
	else if((temp11 <= STR_Curve.CWCoordinate)&&(STR_Curve.CWCCWMaxSet == ENABLE))
	{
		STR_Curve.JogOut = STR_Curve.CWCoordinate - STR_Curve.JogPulseSend;
	}
	
	STR_Curve.JogPulseSend += STR_Curve.JogOut;

	//齿轮比计算
	temp111              = STR_Curve.JogOut * (int64)STR_Curve.Numerator + STR_Curve.EleGearRem;
	STR_Curve.JogOut     = temp111 / (int64)STR_Curve.Denominator;
	STR_Curve.EleGearRem = temp111 - (int64)STR_Curve.Denominator * STR_Curve.JogOut;

    return (STR_Curve.JogOut);

}









