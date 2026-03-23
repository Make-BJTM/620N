/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_FricIdentify.c
 创建人：  姚虹          创建日期：12.9.17      
 修改人：                
 描述： 1. 完成库仑摩擦力的辨识
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
#include "FUNC_FunCode.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FricIdentify.h"
#include "FUNC_OperEeprom.h"
#include "FUNC_ServoMonitor.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

#define FRICLENGTH	 5    //用于辨识所用的多段速度的段数


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
 
STR_FRIC  STR_Fric;     //结构体定义

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
const int16 SpdArray[FRICLENGTH] = {200, 350, 500, 650, 800};    //用于测试的速度值
int16  IqArray[FRICLENGTH];		     //不同速度对应的转矩值

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

void Fric_Iden(void);
void Fric_CoffCal(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

Static_Inline void Fric_ParamInit(void);
Static_Inline void Fric_ParamRevert(void);	 
Static_Inline int16 Fric_Monit(void);


//功能：更新功能码参数，以便能进行多段速度运行
Static_Inline void Fric_ParamInit(void)
{


//首先保存需要更新的功能码参数
	STR_Fric.BP_ModeSelet       = FunCodeUnion.code.BP_ModeSelet;
	STR_Fric.SL_SpdBCommandFrom = FunCodeUnion.code.SL_SpdBCommandFrom;
	STR_Fric.SL_SpdABSwitchFlag = FunCodeUnion.code.SL_SpdABSwitchFlag;

    STR_Fric.MTS_RefRunMode   = FunCodeUnion.code.MTS_RefRunMode;
	STR_Fric.MTS_ExeSects     = FunCodeUnion.code.MTS_ExeSects; 
	STR_Fric.MTS_RunTimeUnit  = FunCodeUnion.code.MTS_RunTimeUnit;
	STR_Fric.MTS_RiseTime1    = FunCodeUnion.code.MTS_RiseTime1;
	STR_Fric.MTS_FallTime1    = FunCodeUnion.code.MTS_FallTime1; 
	STR_Fric.MTS_SEC1_Ref     = FunCodeUnion.code.MTS_SEC1_Ref;
	STR_Fric.MTS_SEC1_RunTime = FunCodeUnion.code.MTS_SEC1_RunTime;
	STR_Fric.MTS_SEC1_RiFaSel = FunCodeUnion.code.MTS_SEC1_RiFaSel;
	STR_Fric.MTS_SEC2_Ref     = FunCodeUnion.code.MTS_SEC2_Ref;
	STR_Fric.MTS_SEC2_RunTime = FunCodeUnion.code.MTS_SEC2_RunTime;
	STR_Fric.MTS_SEC2_RiFaSel = FunCodeUnion.code.MTS_SEC2_RiFaSel;
	STR_Fric.MTS_SEC3_Ref     = FunCodeUnion.code.MTS_SEC3_Ref;
	STR_Fric.MTS_SEC3_RunTime = FunCodeUnion.code.MTS_SEC3_RunTime;
	STR_Fric.MTS_SEC3_RiFaSel = FunCodeUnion.code.MTS_SEC3_RiFaSel;
	STR_Fric.MTS_SEC4_Ref     = FunCodeUnion.code.MTS_SEC4_Ref;
	STR_Fric.MTS_SEC4_RunTime = FunCodeUnion.code.MTS_SEC4_RunTime;
	STR_Fric.MTS_SEC4_RiFaSel = FunCodeUnion.code.MTS_SEC4_RiFaSel;
	STR_Fric.MTS_SEC5_Ref     = FunCodeUnion.code.MTS_SEC5_Ref;
	STR_Fric.MTS_SEC5_RunTime = FunCodeUnion.code.MTS_SEC5_RunTime;
	STR_Fric.MTS_SEC5_RiFaSel = FunCodeUnion.code.MTS_SEC5_RiFaSel;

//设定相应的功能码
	FunCodeUnion.code.BP_ModeSelet     = 0;    //速度模式
    FunCodeUnion.code.SL_SpdBCommandFrom = 5;   //B来源选择为多段速度来源
    FunCodeUnion.code.SL_SpdABSwitchFlag = 1;   //选择速度来源为B
	FunCodeUnion.code.MTS_RefRunMode   = 0;
	FunCodeUnion.code.MTS_ExeSects     = 5;
	FunCodeUnion.code.MTS_RunTimeUnit  = 0;
	FunCodeUnion.code.MTS_RiseTime1    = 50;
	FunCodeUnion.code.MTS_FallTime1    = 50; 
	FunCodeUnion.code.MTS_SEC1_Ref     = SpdArray[0];
	FunCodeUnion.code.MTS_SEC1_RunTime = 10;
	FunCodeUnion.code.MTS_SEC1_RiFaSel = 1;
	FunCodeUnion.code.MTS_SEC2_Ref     = SpdArray[1];		//取反
	FunCodeUnion.code.MTS_SEC2_RunTime = 10;
	FunCodeUnion.code.MTS_SEC2_RiFaSel = 1;
	FunCodeUnion.code.MTS_SEC3_Ref     = SpdArray[2];
	FunCodeUnion.code.MTS_SEC3_RunTime = 10;
	FunCodeUnion.code.MTS_SEC3_RiFaSel = 1;
	FunCodeUnion.code.MTS_SEC4_Ref     = SpdArray[3];	   //取反
	FunCodeUnion.code.MTS_SEC4_RunTime = 10;
	FunCodeUnion.code.MTS_SEC4_RiFaSel = 1;
	FunCodeUnion.code.MTS_SEC5_Ref     = SpdArray[4];
	FunCodeUnion.code.MTS_SEC5_RunTime = 10;
	FunCodeUnion.code.MTS_SEC5_RiFaSel = 1;
}


//功能码参数还原
Static_Inline void Fric_ParamRevert(void)
{
	FunCodeUnion.code.BP_ModeSelet       = STR_Fric.BP_ModeSelet;
	FunCodeUnion.code.SL_SpdBCommandFrom = STR_Fric.SL_SpdBCommandFrom;
	FunCodeUnion.code.SL_SpdABSwitchFlag = STR_Fric.SL_SpdABSwitchFlag;	
		
	FunCodeUnion.code.MTS_RefRunMode   = STR_Fric.MTS_RefRunMode;   
	FunCodeUnion.code.MTS_ExeSects     = STR_Fric.MTS_ExeSects;     
	FunCodeUnion.code.MTS_RunTimeUnit  = STR_Fric.MTS_RunTimeUnit;  
	FunCodeUnion.code.MTS_RiseTime1    = STR_Fric.MTS_RiseTime1;    
	FunCodeUnion.code.MTS_FallTime1    = STR_Fric.MTS_FallTime1;    
	FunCodeUnion.code.MTS_SEC1_Ref     = STR_Fric.MTS_SEC1_Ref;
	FunCodeUnion.code.MTS_SEC1_RunTime = STR_Fric.MTS_SEC1_RunTime; 
	FunCodeUnion.code.MTS_SEC1_RiFaSel = STR_Fric.MTS_SEC1_RiFaSel; 
	FunCodeUnion.code.MTS_SEC2_Ref     = STR_Fric.MTS_SEC2_Ref;
	FunCodeUnion.code.MTS_SEC2_RunTime = STR_Fric.MTS_SEC2_RunTime; 
	FunCodeUnion.code.MTS_SEC2_RiFaSel = STR_Fric.MTS_SEC2_RiFaSel; 
	FunCodeUnion.code.MTS_SEC3_Ref     = STR_Fric.MTS_SEC3_Ref;
	FunCodeUnion.code.MTS_SEC3_RunTime = STR_Fric.MTS_SEC3_RunTime; 
	FunCodeUnion.code.MTS_SEC3_RiFaSel = STR_Fric.MTS_SEC3_RiFaSel; 
	FunCodeUnion.code.MTS_SEC4_Ref     = STR_Fric.MTS_SEC4_Ref;
	FunCodeUnion.code.MTS_SEC4_RunTime = STR_Fric.MTS_SEC4_RunTime; 
	FunCodeUnion.code.MTS_SEC4_RiFaSel = STR_Fric.MTS_SEC4_RiFaSel; 
	FunCodeUnion.code.MTS_SEC5_Ref     = STR_Fric.MTS_SEC5_Ref;
	FunCodeUnion.code.MTS_SEC5_RunTime = STR_Fric.MTS_SEC5_RunTime; 
	FunCodeUnion.code.MTS_SEC5_RiFaSel = STR_Fric.MTS_SEC5_RiFaSel; 
}


void Fric_Iden(void)
{
	static int16 Step = 0;			 //辨识步骤
	static int16 TimedelayCnt = 0;	 //延时计数器
    /*
	if(AuxFunCodeUnion.code.FA_FricIdentify == 0)
	{
	    return;
	}
	*/
	if(Step == 0)
	{
	    Fric_ParamInit();   //获取初始参数

		Step = 1;          //进入下一步
	}
	else if(Step == 1)
	{
	    TimedelayCnt++;
								 
		if(TimedelayCnt > 4000)		//等待多段速度初始化完成
		{
		    Step = 2;				//进入下一步
			TimedelayCnt = 0;
		}
	}
	else if(Step == 2)
	{  
		STR_FUNC_Gvar.FricIdentify.FricIdenCmd = 1;	 //内部使能,同时多段速度开始运行

	    if(1 == Fric_Monit())       //判定是否已经得到了5段速度的转矩平均值
		{
			Step = 3;

            STR_FUNC_Gvar.FricIdentify.FricIdenCmd = 0;   //内部关闭使能
		}
	}
	else if(Step == 3)
	{
	 	if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)
        {
            Fric_ParamRevert();				  //参数还原
		
		    Step = 0;
			
		    AuxFunCodeUnion.code.FA_FricIdentify = 0;     //不再辨识
      
		    STR_FUNC_Gvar.FricIdentify.EndIdenFlag = 1; 	   //置结束标志位为1
        }
	}
}


//用于得到不同速度运行时的转矩平均值
Static_Inline int16 Fric_Monit(void)
{
	static int16 DelayCnt = 0;     //延时计数器
	static int32 IqSum = 0;        //用于计算的转矩和
	static int16 SampCnt = 0;     //采集的数据个数
	static int16 SampFlag = 0;    //1～允许采样，0～不允许采样
	static Uint16 MultiNum = 0;   //本周期多段速度段数
	static Uint16 MultiNumPre = 0;   //前一周期多段速段数
	static Uint16 MultiFlag = 0;	 //某段被选择标志位，0～该段速度不辨识，1～该段速度辨识

	MultiNum = STR_FUNC_Gvar.FricIdentify.MultiBlockNum;
	
	if(MultiNum == 0)
	{
		if(MultiNumPre != 0)
		{
		  	MultiNumPre = 0;
		    return 1;			   //结束辨识  
		}
		else
		{
	        MultiFlag = 1;		   //允许进行辨识
		}
	}
	else if(MultiNum != MultiNumPre)
	{
	    MultiFlag = 1;
		MultiNumPre = MultiNum;	    
	}

	if(MultiFlag == 1)
	{
		if(SampFlag == 0)
		{
    		DelayCnt++;

			if(DelayCnt > 1000)	 //做一定延时后开始采样，确保伺服使能和速度稳定
    	    {	    
    			SampFlag = 1;	 //开始采样
				DelayCnt = 0;
    		}
		}
		else
		{
			IqSum += UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef;    //加入转矩指令，已经是百分比
			SampCnt++;

			if(SampCnt == 1024)
			{
				IqArray[MultiNum] = ABS(IqSum >> 10);     //对力矩取绝对值
				SampCnt = 0;
				IqSum = 0;
				SampFlag = 0;	  //采样标志位置0；
				MultiFlag = 0;    //该段速度不再辨识

			}
		}
	}

	return 0;
}

//利用最小二乘法计算得到的库仑摩擦模型系数
void Fric_CoffCal(void)
{
	int32 Temp1 = 0; 
	int32 Temp2 = 0; 
	int32 Temp3 = 0; 
	int32 Temp4 = 0;
	int32 Num1, Num2;	   //最终计算曲线所用到的分子
	int32 Den;		       //最终计算曲线所用到的分母
	int16 i;

	for(i=0; i<FRICLENGTH; i++)
	{
	    Temp1 += SpdArray[i];
		Temp2 += IqArray[i];
		Temp3 += (int32)SpdArray[i] * IqArray[i];
		Temp4 += (int32)SpdArray[i] * SpdArray[i];
	}

	Den  = (FRICLENGTH * Temp4) - (Temp1 * Temp1);
	Num1 = (FRICLENGTH * Temp3) - (Temp1 * Temp2);
	Num2 = (Temp4 * Temp2) - (Temp1 * Temp3);

	STR_Fric.Kslope_Q10 = (Num1 << 10) / Den;        //根据最小二乘计算斜率和偏置
	STR_Fric.Fbias      = Num2 / Den;	

/*
	//根据斜率求取相应的库仑力，百分比形式	
	FunCodeUnion.code.AT_PlusCoulomb  = ((STR_Fric.Kslope_Q10 * FunCodeUnion.code.AT_ToqCompSpd) >> 10) + STR_Fric.Fbias;                                   

	FunCodeUnion.code.AT_MinusCoulomb = FunCodeUnion.code.AT_PlusCoulomb;

	FunCodeUnion.code.AT_ToqPlusComp  = FunCodeUnion.code.AT_PlusCoulomb;
	FunCodeUnion.code.AT_ToqMinusComp = FunCodeUnion.code.AT_PlusCoulomb;

	SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_PlusCoulomb));	    //将库仑力保存
	SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_MinusCoulomb)); 
	SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_ToqPlusComp));	    //将最大静摩擦力保存
	SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_ToqMinusComp));
*/	                               
}
