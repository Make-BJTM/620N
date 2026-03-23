/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_LineInterplt.c
 创建人：肖明海                 创建日期：2010.01.06
 修订人：何云壮                 修订日期：2012.02.03 
 描述： 
     1.直线插补功能的实现
	 2.插补周期等于位置环调度周期
	 3.计算过程中采用Q16格式, 原来是采用Q6格式的, 后来因为既要简化部分计算
	   又要提高精度, 遂采用了Q16格式

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_PosCtrl.h"
#include "FUNC_LineInterplt.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
// 这个指针数组指向目前用到的三个结构体, 用于在不同内部位置指令模式之间切换时复位
// 之前使用的插补结构目前约定:
// pIntplt[0]指向多段位置插补结构
// pIntplt[1]指向中断定长插补结构
// pIntplt[2]指向原点回归插补结构
// pIntplt[3]指向定位试运行插补结构
STR_LINE_INTERPLT_ATTRIB* pIntplt[4] = {0, 0, 0, 0};

// 重新规划减速段时用到以下三个变量
static int64 diff = 0;     // 总差值
static int64 diff2 = 0;    // 总差值的绝对值
static int32 compsnt = 0;  // 分散到每次插补时的补偿值
	
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   
void LineIntpltReset(STR_LINE_INTERPLT_ATTRIB *pAtb);
int32 LineIntplt(STR_LINE_INTERPLT_ATTRIB *pAtb);
void LineIntpltInit(int32 StartSpeed, int32 RunSpeed, int32 StopSpeed,
                    Uint32 UpTime, Uint32 DownTime, int64 LineDist,
					STR_LINE_INTERPLT_ATTRIB *pAtb);					
void RecordIntpltStruct(STR_LINE_INTERPLT_ATTRIB *pAtb, Uint16 index);
void ResetIntpltDownLength(STR_LINE_INTERPLT_ATTRIB *pAtb);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline int64 AccDistCalc(int32 StartPulseQ16, int32 StopPulseQ16, int32 PulseRev);


/*******************************************************************************
  函数名: void LineIntplt() 
  输  入: pAtb-指向直线插补寄存器的指针          
  输  出: 当前插补周期的插补脉冲数   
  子函数:                                       
  描  述: 计算每个插补周期的插补脉冲值  
********************************************************************************/ 
int32 LineIntplt(STR_LINE_INTERPLT_ATTRIB *pAtb)
{
	int64 tmpCnt = 0, sum = 0, sm1 = 0, df1 = 0, twice = 0, appPls = 0;
	
	if ((1 == pAtb->IntpltStatus) && (pAtb->LineRemainLengthQ16 > 0))
	{   // 只有在插补状态标志等于1 并且 插补剩余长度大于0时才进行插补
		//加速,当前剩余长度大于恒速长度＋减速长度
		if (pAtb->LineRemainLengthQ16 > (pAtb->AvergeLengthQ16 + pAtb->DownLengthQ16))   
		{
			pAtb->PlanIntpltValQ16 +=  pAtb->UpPulseRevQ16; 
			if(pAtb->UpPulseRevQ16 > 0)
			{
				if(pAtb->PlanIntpltValQ16 > pAtb->AvergePulseQ16)
				{   // 加速段每插补周期的最大脉冲数不大于恒速运行时的脉冲数
					pAtb->PlanIntpltValQ16 = pAtb->AvergePulseQ16;
				} 
			}
			else
			{
				if(pAtb->PlanIntpltValQ16 < pAtb->AvergePulseQ16)
				{   // 加速段每插补周期的最大脉冲数不大于恒速运行时的脉冲数
					pAtb->PlanIntpltValQ16 = pAtb->AvergePulseQ16;
				} 
			}
		}
		else if(pAtb->LineRemainLengthQ16 <= pAtb->DownLengthQ16)
		{   //减速,当前剩余长度不大于减速长度
			if (1 == pAtb->PlanDecAgain)
			{   // 根据剩余的脉冲数重新规划减速段
				pAtb->DownLengthQ16 = pAtb->LineRemainLengthQ16;				
				sm1 = pAtb->StopPulseQ16 + pAtb->PlanIntpltValQ16;
				df1 = pAtb->StopPulseQ16 - pAtb->PlanIntpltValQ16;
				twice = pAtb->DownLengthQ16 << 1;
				
				// 差值及每次的补偿值存储变量清零
				diff = 0;
		        diff2 = 0;
		        compsnt = 0;
				
                // 计算减速时间(插补次数)
				tmpCnt =  twice / sm1;
				
				// 减速段插补次数大于4 才处理, tmpCntd等于1时会导致指令总数不对
				if (tmpCnt > 4)
				{
					// 计算减速时每插补周期脉冲递减量(减速度)
					// 以防止出现除数为0的情况
					twice += (twice == sm1)? 1 : 0;
					pAtb->DownPulseRevQ16 = (sm1 * df1) / (twice - sm1);
					
					// 根据规划的插补次数和每插补周期脉冲递减量及起停速度计算总脉冲数
					sum = tmpCnt * (pAtb->PlanIntpltValQ16 + (pAtb->DownPulseRevQ16
															* (tmpCnt - 1) >> 1));	
			        // 计算实际总脉冲数与规划所得总脉冲数的差值
					diff = pAtb->DownLengthQ16 - sum;
					
					// 差值大于零说明可能会拖尾, 小于0则可能会跌落
					if (diff > 0)
				    {
						diff2 = diff;
					}
					else
					{
						diff2 = (0-diff);
					}
					
					// 将差值分 tmpCnt/2 次补偿到减速过程, 以使规划的递减过程得以实现
					// 以防止出现除数为0的情况
					tmpCnt += (0 == tmpCnt)? 1 : 0;
					compsnt = (diff2<<1) / tmpCnt;
				}
							
				pAtb->PlanDecAgain = 0;
			}
			else
			{
			    pAtb->PlanIntpltValQ16 += pAtb->DownPulseRevQ16; 
			}
			
			// 补偿差值, 若拖尾则将多余脉冲在减速的前半段发出去
			// 若跌落, 则将使其不跌落时所差的脉冲数在减速的前半段中少发一点补回来
			appPls = 0;
			if (diff2 > 0)
			{
				if (diff2 > compsnt)
				{
					appPls = compsnt;
				}
				else
				{
					appPls = diff2;
				}
				diff2 -= compsnt;
				appPls = (diff > 0)? appPls : (0-appPls);
			}
			 
			if(pAtb->DownPulseRevQ16 > 0 )
			{
			    if(pAtb->PlanIntpltValQ16 > pAtb->StopPulseQ16)
			    {   // 减速段每插补周期的最小脉冲数不小于预期的停止脉冲数
			        pAtb->PlanIntpltValQ16 = pAtb->StopPulseQ16;
			    }
			}
			else
			{
			    if(pAtb->PlanIntpltValQ16 < pAtb->StopPulseQ16)
		        {   // 减速段每插补周期的最小脉冲数不小于预期的停止脉冲数
			        pAtb->PlanIntpltValQ16 = pAtb->StopPulseQ16;
			    }
			}
		}
		else //恒速
		{
			pAtb->PlanIntpltValQ16 = pAtb->AvergePulseQ16;
		}
		
		//插补结束判断
		if(pAtb->LineRemainLengthQ16 <= pAtb->PlanIntpltValQ16)
		{
			//针对多段位置模式3的特殊处理，希望有更好的方式，也就是不在此模块添加
			// 这里是为了实现多段位置顺序模式下段与段切换不出现位置指令为0的插补周期
			if(pAtb->MbpContinue)
			{   
			    // 当前段最后一个插补周期仍然用匀速段的插补脉冲数
				// 但实际上当前段剩下了p->LineRemainLengthQ6个插补脉冲
				// 因此当前段可能多或者少插补了p->extraDist个脉冲,
				// 这部分脉冲将留在下一段中补偿
				pAtb->PlanIntpltValQ16 = pAtb->AvergePulseQ16;
				pAtb->MbpExtraPlsQ16 = (pAtb->AvergePulseQ16
				                        - (int32)pAtb->LineRemainLengthQ16)>>LINEAMPBIT;
				pAtb->MbpContinue = 0;
			}
			else 
			{
				pAtb->MbpExtraPlsQ16 = 0;
				pAtb->PlanIntpltValQ16 = pAtb->LineRemainLengthQ16;
			}
			
			pAtb->IntpltStatus = 0;//插补结束
		}
		
		// 计算实际插补脉冲数	
		pAtb->RealIntpltVal = (pAtb->PlanIntpltValQ16 + appPls
		                           + pAtb->PlanValueRemainQ16)>>LINEAMPBIT;	
		// 计算右移LINEAMPBIT位产生的余数
		pAtb->PlanValueRemainQ16 = pAtb->PlanIntpltValQ16 + appPls + pAtb->PlanValueRemainQ16
		                           - (pAtb->RealIntpltVal<<LINEAMPBIT);
		// 计算剩余脉冲数
		pAtb->LineRemainLengthQ16 -= (pAtb->PlanIntpltValQ16 + appPls);
	}
    else
	{
	    pAtb->IntpltStatus = 0;//插补结束
        return 0;	
	}

	// 根据记录的方向标志, 恢复插补脉冲的方向
	if(pAtb->LineDir == 1)
		return  pAtb->RealIntpltVal;
	else
		return (0 - pAtb->RealIntpltVal);
}


/*******************************************************************************
  函数名: LineIntpltReset
  输  入:  STR_LineReg 
  输  出:    
  子函数:                                       
  描  述: 插补状态复位 
********************************************************************************/
void LineIntpltReset(STR_LINE_INTERPLT_ATTRIB *pAtb)
{
	pAtb->IntpltStatus = 0;
	pAtb->LineRemainLengthQ16 = 0;
	pAtb->MbpExtraPlsQ16 = 0;
	pAtb->MbpContinue = 0;
}


/*******************************************************************************
  函数名: LineIntpltInit
  输  入: StartSpeed - 起步速度, RunSpeed - 运行速度, StopSpeed - 停止速度
		  UpTime - 加速时间,     DownTime - 减速时间, LineDist - 总位移长度
		  参数 UpTime, DownTime 都以插补周期为单位
  输  出:    
  子函数: AccDistCalc()计算加(减)速段的长度
  描  述: 定长直线插补初始化 
********************************************************************************/ 
void LineIntpltInit(int32 StartSpeed, int32 RunSpeed, int32 StopSpeed,
                    Uint32 UpTime, Uint32 DownTime, int64 LineDist,
					STR_LINE_INTERPLT_ATTRIB *pAtb)
{
    Uint16 i = 0;                // 循环计数变量
	int64 MaxPulse = 0;          // 电机最大速度对应的每个插补周期脉冲数
	int32 DecMinDist = 0;        // 计算开始速度大于终止速度时计算最小减速距离
    int64 mulF = 0, divF = 0;    // 齿轮比分子分母
	Uint64 temp11 = 0;
	pAtb->IntpltStatus = 0;  // 插补状态置0, 表示当前并未插补
	
	if(RunSpeed > FunCodeUnion.code.MT_MaxSpd)
	{   // 检查所选运行速度是否大于伺服系统当前最大速度
		RunSpeed = 	FunCodeUnion.code.MT_MaxSpd;
	}
	
    mulF = (int64)STR_InnerGvarPosCtrl.Numerator;
    divF = (int64)STR_InnerGvarPosCtrl.Denominator;

    // 齿轮比改成实时作用, 即齿轮比将在插补完成之后作用又为了用户设置的速度不因齿
    // 轮比而变化需要将插补时各个速度乘上齿轮比的倒数
	// 计算匀速段每个插补周期的脉冲个数 Q16	
    if((STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag == 1)
    	||(FunCodeUnion.code.FC_FeedbackMode == 1))
    {
    	temp11 = STR_InnerGvarPosCtrl.ExRPM2PPPtCoefQ16;
    }
    else
    {
    	temp11 = STR_InnerGvarPosCtrl.RPM2PPPtCoefQ16;
    }
	pAtb->AvergePulseQ16 = (int64)RunSpeed * temp11
                            * divF / mulF;
	 
	// 计算起动速度对应的每个插补周期脉冲个数 Q16
	pAtb->StartPulseQ16 = (int64)StartSpeed * temp11
                            * divF / mulF;
					 
	// 计算停止速度对应的每个插补周期脉冲个数 Q16
	pAtb->StopPulseQ16 = (int64)StopSpeed * temp11
                            * divF / mulF;
					
	// 计算电机最大速度对应的每个插补周期脉冲个数 Q16
	MaxPulse = (int64)SPDUPDOWM_MAXREF * temp11
                            * divF / mulF;
	
	// 根据加速时间 UpTime 计算加速段每个插补周期的脉冲增量
	if (UpTime) // UpTime的单位已经是插补周期
	{
		// 以电机能达到的最大速度而不是用户设置的运行速度来计算每个插补周期的脉冲增量
		// 因此所得结果是这个加速时间下能达到的每个插补周期的最大脉冲增量
		if (pAtb->StartPulseQ16 < pAtb->AvergePulseQ16) 
		{   // 目标速度比起步速度大, 递增量为正
			pAtb->UpPulseRevQ16 = (int32)(MaxPulse / UpTime);
			if (pAtb->UpPulseRevQ16==0)
			{
				pAtb->UpPulseRevQ16 = 1;
			}
		}
		else if (pAtb->StartPulseQ16 > pAtb->AvergePulseQ16)
		{   // 目标速度比起步速度小, 递增量为负数
			pAtb->UpPulseRevQ16 = 0 - (int32)(MaxPulse / UpTime);		   
			if (pAtb->UpPulseRevQ16==0)
			{
				pAtb->UpPulseRevQ16 = -1;
			}
		}
        else
        {
            pAtb->UpPulseRevQ16 = 0;
        }
	}
	else
	{
		pAtb->UpPulseRevQ16 = 0;
	}

	// 根据减速时间 DownTime 计算减速段每个插补周期的脉冲减量, 与加速段的计算方法相同
	if (DownTime) // DownTime的单位已经是插补周期
	{
		if (pAtb->AvergePulseQ16 < pAtb->StopPulseQ16) 
		{   // 目标速度比起步速度大, 递减量为正
			pAtb->DownPulseRevQ16 = (int32)(MaxPulse / DownTime);
			if (pAtb->DownPulseRevQ16==0)
			{
				pAtb->DownPulseRevQ16 = 1;
			}
		}
		else if (pAtb->AvergePulseQ16 > pAtb->StopPulseQ16)
		{   // 目标速度比起步速度小, 递减量为正
			pAtb->DownPulseRevQ16 = 0- (int32)(MaxPulse / DownTime);
			if (pAtb->DownPulseRevQ16==0)
			{
				pAtb->DownPulseRevQ16 = -1;
			}
		}
        else
        {
            pAtb->DownPulseRevQ16 = 0;
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
	pAtb->UpLengthQ16=AccDistCalc(pAtb->StartPulseQ16, pAtb->AvergePulseQ16, pAtb->UpPulseRevQ16);

	// 计算减速段长度
	pAtb->DownLengthQ16= AccDistCalc(pAtb->AvergePulseQ16, pAtb->StopPulseQ16, pAtb->DownPulseRevQ16);

	if((pAtb->UpLengthQ16 + pAtb->DownLengthQ16) > pAtb->LineLengthQ16)
	{   
	    if(pAtb->DownLengthQ16==0)
		{
		    pAtb->UpLengthQ16 = pAtb->LineLengthQ16;
		}
		else
		{
		    // 加速段和减速段长度之和大于总长度, 此时先计算减速段最小长度
			if(pAtb->StartPulseQ16 > pAtb->StopPulseQ16)
			{
			    DecMinDist = AccDistCalc(pAtb->StartPulseQ16, pAtb->StopPulseQ16, pAtb->DownPulseRevQ16);
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
  函数名: void RecordIntpltStruct()
  输  入: pAtb - 指向插补结构的指针
          index - 存储下标
          pIntplt[0]指向多段位置插补结构
          pIntplt[1]指向中断定长插补结构
          pIntplt[2]指向原点回归插补结构
          pIntplt[3]指向定位试运行插补结构
  输  出:   
  子函数:                                       
  描  述: 记录插补结构变量的地址
********************************************************************************/ 				
void RecordIntpltStruct(STR_LINE_INTERPLT_ATTRIB *pAtb, Uint16 index)
{
    Uint16 u16Size = 0;
    u16Size = sizeof(pIntplt) / sizeof(pIntplt[0]);
    if (index > (u16Size - 1))
	{
	    index = u16Size - 1;
	}
	
    if (0 == pIntplt[index])
	{
	    pIntplt[index] = pAtb;
	}
}

/*******************************************************************************
  函数名: void ResetIntpltDownLength()
  输  入: LineLengthQ16 - 总脉冲数 
          LineRemainLengthQ16  - 剩余脉冲数
  输  出: 重置减速段的长度(脉冲数)  
  子函数:                                       
  描  述: 提前结束插补过程时重置减速段长度
********************************************************************************/ 
void ResetIntpltDownLength(STR_LINE_INTERPLT_ATTRIB *pAtb)
{
    if (pAtb->LineRemainLengthQ16 > (pAtb->AvergeLengthQ16 + pAtb->DownLengthQ16))
	{   // 当前正处于加速段, 以已经走过的长度作为减速段长度
	    pAtb->DownLengthQ16 = pAtb->LineLengthQ16 - pAtb->LineRemainLengthQ16;
	}
	else if(pAtb->LineRemainLengthQ16 <= pAtb->DownLengthQ16)
	{   // 当前正处于减速段, 以剩余的作为减速段长度
	    pAtb->DownLengthQ16 = pAtb->LineRemainLengthQ16;
	}
	else
	{   // 当前正处于匀速段, 仍为预先规划的减速段长度
	    // pAtb->DownLengthQ16;
	}
}


/*******************************************************************************
  函数名: void AccDistCalc()
  输  入: StartPulseQ16 - 起点处每个插补周期的插补脉冲数 
          StopPulseQ16  - 终点处每个插补周期的插补脉冲数
		  PulseRev   - 每个插补周期脉冲递增(减)量(加/减速度)
  输  出: 加(减)速段的长度(以脉冲为单位)   
  子函数:                                       
  描  述: 根据起点处每周期插补脉冲数StartPulse, 终点处每周期插补脉冲数EndPulse
          以及每个插补周期脉冲递增量, 计算加(减)速段的长度
		  如果增(减)量 PulseRev等于0, 则认为加(减)速段距离也为0
********************************************************************************/ 
Static_Inline int64 AccDistCalc(int32 StartPulseQ16, int32 StopPulseQ16, int32 PulseRev)
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

/********************************* END OF FILE *********************************/
