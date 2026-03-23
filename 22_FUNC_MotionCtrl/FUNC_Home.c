/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_Home.c
 创建人：肖明海                 创建日期：2010.01.26
 修订人：何云壮                 修订日期：2012.02.09 
 描述： 
     1.原点回归功能的实现

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ManageFunCode.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_LineInterplt.h"
#include "FUNC_Home.h"
#include "FUNC_FullCloseLoop.h"
#include "PUB_Main.h"
#include "stm32f4xx.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define HM_STARTSPD   (12)
#define HM_STOPSPD    (12)

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
static STR_HOME_ATTRIB STR_Home = HomeAttribDflts;
static STR_LINE_INTERPLT_ATTRIB  STR_HmLnIntplt	= LineInterpltAttribDflts;
static Uint16 u16IsOverTime = 0;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void HomingInitOnce(void);           // 上电复位原点回归相关参数(上电执行一次)
void HomingStopUpdate(void);         // 停机更新原点回归相关参数
void DoHoming(void);                 // 执行原点回归过程
int32 HomingPosCmd(void);            // 给出回原点时每个插补周期的位置指令
void HomingReset(void);              // 复位插补结构
void ZeroIndexISR(void);			 // Z 脉冲中断服务
void HomParaUpdateRealTim(void);//回零参数实时更新


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void HomingTriggerUpdate(void);
Static_Inline void HomingOverTimeCheck(void);
Static_Inline void HomingHighSpeedSearch(void);
Static_Inline void HomingLowSpeedSearch(void);
Static_Inline void HomingLowSpeedSearchEnd(void);
Static_Inline void HomingZeroNearStatusUpdate(void); 


/*******************************************************************************
  函数名: void HomingInitOnce() 
  输  入:           
  输  出:   
  子函数:  SaveToEepromOne()                                     
  描  述: 上电初始化原点回归相关参数, 仅上电运行一次
********************************************************************************/
void HomingInitOnce(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;


	STR_FUNC_Gvar.PosCtrl.HomStats = 0;

	// 记录原点回归插补结构变量 STR_HmLnIntplt 的地址
	RecordIntpltStruct(&STR_HmLnIntplt, 2);
	
    pHome->Step = 0;                                       // 回零步骤标志清0
	//pHome->Trigger = STR_FUNC_Gvar.DivarRegLw.bit.OrgChuFa;  // 记录当前原点开关信号状态
    //pHome->DITriggerOld = pHome->Trigger;                  // 备份原点开关信号状态
	pHome->Trigger = 0;                                     
    pHome->DITriggerOld = 0;                               
    pHome->ZeroIndex = 0;                                  // Z 信号锁存器清0
	STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0;		   // 告知系统现在不处于回原点过程
    pHome->HomActFlg = 0;//回零未启动
    pHome->HomeEnable = FunCodeUnion.code.PL_OriginResetEn;

    if(pHome->HomeEnable == 3)
    {   //上电伺服ON之后立即回零处理
	    pHome->Trigger = 1;
        pHome->Elector = 0;
    }
    else if(pHome->HomeEnable == 4)
    {   //避免上次立即回零失败后，重新上电伺服ON之后立即开始回零
	    FunCodeUnion.code.PL_OriginResetEn = 0;
		pHome->HomeEnable                  = 0;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_OriginResetEn));
    }

    STR_FUNC_Gvar.PosCtrl.HomStats = 0; //回零结束

}
/*******************************************************************************
  函数名: void HomParaUpdateRealTim() 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: 实时更新原点回归相关参数  
********************************************************************************/
void HomParaUpdateRealTim(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;

    if(pHome->HomActFlg == 0) //参数更新只能在回零未进行时
    {
        pHome->HomeEnable = FunCodeUnion.code.PL_OriginResetEn;
        pHome->HighSpeedSearch = FunCodeUnion.code.PL_OriginHighSpeedSearch; //原点高速搜索速度 H0532
        pHome->LowSpeedSearch = FunCodeUnion.code.PL_OriginLowSpeedSearch;   //原点低速搜索速度 H0533
        pHome->RiseDownTime = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * FunCodeUnion.code.PL_OriginRiseDownTime)
    	                    >> 13;  // 原点加减速时间(已换算到以插补周期为单位)
        pHome->SearchTime = FunCodeUnion.code.PL_OriginSearchTime;           //原点查找时间 H0535

        // 计算找零距离
        if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线电机需要转换成脉冲数
        {
            pHome->Dist = (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * pHome->SearchTime
    	                * pHome->HighSpeedSearch / 1000;    
        }
        else
        {
            pHome->Dist = (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * pHome->SearchTime
    	                * pHome->HighSpeedSearch / 60000;    
        }

            // 复归后坐标设置,偏移距离计算
        if((pHome->HomeEnable != 2)
           && (pHome->HomeEnable != 5)
           && (pHome->OriginSel.bit.IsOffset))
        {   // 不工作在电气归零模式, 且H05.40 == 0 或 1, 此时回原点之后, 原点坐标并不偏移
    	    // 伺服电机在回原点之后自动移动 H05.36 所示距离
            pHome->Offset = 0;
            pHome->MoveDist = (int32)(((Uint32)FunCodeUnion.code.PL_OriginOffsetHigh << 16)
                            + FunCodeUnion.code.PL_OriginOffsetLow);
        }
        else
        {   // 回原点之后, 将原点坐标设置为05.36 所示值
            pHome->Offset = (int32)(((Uint32)FunCodeUnion.code.PL_OriginOffsetHigh << 16)
                             + FunCodeUnion.code.PL_OriginOffsetLow);            

            pHome->MoveDist = 0;
        }
		if(FunCodeUnion.code.FC_FeedbackMode == 1)
		{
			pHome->Offset  = ((int64) pHome->Offset	* (int64)STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7)>>7;
			pHome->MoveDist = ((int64) pHome->MoveDist	* (int64)STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7)>>7;
		}
    }

}

/*******************************************************************************
  函数名: void HomingStopUpdate() 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: 实时更新原点回归相关参数  
********************************************************************************/
void HomingStopUpdate(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;

    STR_FUNC_Gvar.PosCtrl.HomStats = 0; //回零结束

    pHome->Mode = FunCodeUnion.code.PL_OriginResetMode;                  //原点复位模式 H0531
    pHome->HomeEnable = FunCodeUnion.code.PL_OriginResetEn;              //原点回零使能
    pHome->HighSpeedSearch = FunCodeUnion.code.PL_OriginHighSpeedSearch; //原点高速搜索速度 H0532
    pHome->LowSpeedSearch = FunCodeUnion.code.PL_OriginLowSpeedSearch;   //原点低速搜索速度 H0533
    pHome->RiseDownTime = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * FunCodeUnion.code.PL_OriginRiseDownTime)
	                    >> 13;  // 原点加减速时间(已换算到以插补周期为单位)
    pHome->SearchTime = FunCodeUnion.code.PL_OriginSearchTime;           //原点查找时间 H0535
	
    // 计算找零距离
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线电机需要转换成脉冲数
    {
        pHome->Dist = (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * pHome->SearchTime
	                * pHome->HighSpeedSearch / 1000;    
    }
    else
    {
        pHome->Dist = (int64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * pHome->SearchTime
	                * pHome->HighSpeedSearch / 60000;    
    }
    pHome->LenCmpsFlag = 0;
    pHome->DirInvFlag = 0;
    pHome->ZeroPosFlag = 0;
    pHome->OTStatus = 0;
    pHome->ZeroDownEdge = 0;
	pHome->Near = 0;
    pHome->HomActFlg = 0;//回零未启动
    pHome->NearUpEdge = 0;

    //原点偏移方式选择, 功能码 FunCodeUnion.code.PL_OriginOriginSel 的bit0 和 bit1 的意义如下
    //bit0=0 H0536为原点复归后的坐标 
    //bit0=1 H0536为原点复归后机械偏移距离
    //bit1=0 遇到限位开关后，不反向找零
    //bit1=1 遇到限位开关后，反向找零
    pHome->OriginSel.all = FunCodeUnion.code.PL_OriginOriginSel;

    // 复归后坐标设置,偏移距离计算
    if((pHome->HomeEnable != 2)
       && (pHome->HomeEnable != 5)
       && (pHome->OriginSel.bit.IsOffset))
    {   // 不工作在电气归零模式, 且H05.40 == 0 或 1, 此时回原点之后, 原点坐标并不偏移
	    // 伺服电机在回原点之后自动移动 H05.36 所示距离
        pHome->Offset = 0;
        pHome->MoveDist = (int32)(((Uint32)FunCodeUnion.code.PL_OriginOffsetHigh << 16)
                        + FunCodeUnion.code.PL_OriginOffsetLow);
    }
    else
    {   // 回原点之后, 将原点坐标设置为05.36 所示值
        pHome->Offset = (int32)(((Uint32)FunCodeUnion.code.PL_OriginOffsetHigh << 16)
                         + FunCodeUnion.code.PL_OriginOffsetLow);            

        pHome->MoveDist = 0;
    }
	if(FunCodeUnion.code.FC_FeedbackMode == 1)
	{
		pHome->Offset  = ((int64) pHome->Offset	* (int64)STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7)>>7;
		pHome->MoveDist = ((int64) pHome->MoveDist	* (int64)STR_FUNC_Gvar.PosCtrl.ExPosCoefQ7)>>7;
	}
    // 超时警告标志位
    u16IsOverTime = 0;
}


/*******************************************************************************
  函数名: void HomingPosCmd() 
  输  入: STR_HmLnIntplt         
  输  出:   
  子函数: LineIntplt()                                      
  描  述: 给出使用原点复归时每个插补周期的位置指令
********************************************************************************/
int32 HomingPosCmd(void)
{
	return LineIntplt(&STR_HmLnIntplt);  
}


/*******************************************************************************
  函数名: void HomingReset() 
  输  入: STR_HmLnIntplt         
  输  出:   
  子函数: LineIntpltReset()                                      
  描  述: 复位插补结构
********************************************************************************/
void HomingReset(void)
{
    // 直线插补器参数复位
    if (1 == STR_HmLnIntplt.IntpltStatus)
	{
        LineIntpltReset(&STR_HmLnIntplt);
	}
} 

/*******************************************************************************
  函数名: void DoHoming() 
  输  入:           
  输  出:   
  子函数: SaveToEepromOne(), HomingZeroNearStatusUpdate(), PostErrMsg(),
          HomingTriggerUpdate(), LineIntpltInit(), HomingHighSpeedSearch(),
		  HomingHighSpeedSearch(), HomingOverTimeCheck()
  描  述: 执行回归原点流程
********************************************************************************/
void DoHoming(void)
{
    static int8 OriginMoveDistSign = 0;
	int64 OriginMoveDistTmp = 0;
    int64 tempDist = 0;

	STR_HOME_ATTRIB *pHome = &STR_Home;
	STR_LINE_INTERPLT_ATTRIB *pHmIntp = &STR_HmLnIntplt;

	if(pHome->HomeEnable == 7) 
	{ 
        FunCodeUnion.code.PL_MultiAbsPosOffsetL = 0; 
        FunCodeUnion.code.PL_MultiAbsPosOffsetH = 0;
        FunCodeUnion.code.PL_MultiAbsPosOffsetH32L = 0; 
        FunCodeUnion.code.PL_MultiAbsPosOffsetH32H = 0;                        
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetL));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH32H));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH32L));

        FunCodeUnion.code.PL_OriginResetEn = 0; 
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_OriginResetEn));
    }
	else if(pHome->HomeEnable == 6) 
	{   
        //以当前点为原点
	    tempDist = ((int64)(pHome->Offset) << LINEAMPBIT)
					* STR_InnerGvarPosCtrl.Numerator / STR_InnerGvarPosCtrl.Denominator;

        DINT;
        STR_InnerGvarPosCtrl.CurrentAbsPos = (int32)(tempDist >> LINEAMPBIT);
        STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt = STR_InnerGvarPosCtrl.CurrentAbsPos;
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.CurrentAbsPos;
        if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos < 0)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
        }

        if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
        }
        
		STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.CurrentAbsPos;
        if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt < 0)
        {
            if(((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)<=0)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
            }

            if(((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 100)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = -(STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + STR_InnerGvarPosCtrl.AbsMod2PosUpLmt);
            }
        }
        else 
        {
            if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
            }

            if(((int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - (int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt) <= 100)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
            }
        }
        EINT;
		STR_InnerGvarPosCtrl.InputPulseCnt = 0;
        STR_InnerGvarPosCtrl.PulseCalcRemainder = 0;              
		pHome->HomeEnable = 0;

        FunCodeUnion.code.PL_OriginResetEn = 0; 
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_OriginResetEn));

        if((1 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)||(3 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection))
        {
            AbsMod1_MultiTurnOffset();
            STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset = 1;
        }
        else if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
        {
            STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod2RestRemSum = 1;            
        }         

        pHome->Status = 1;//回零成功
		FullCloseParaRst();
		STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0; // 告知系统已结束回零过程
	}

    //回零状态记录 bit0 -普通回零ok  bit1 -电气回零ok bit2-回零超时  
    if(pHome->Status & 0x01)
	{
        STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOk = VALID;  // 成功找到普通原点
	}
	else
	{
        STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOk = INVALID;  // 还没有找到原点
	}

    if(pHome->Status & 0x02)
	{
        STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOkElectric = VALID; // 成功找到电气原点
	}
	else
	{
        STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOkElectric = INVALID; // 还没有找到电气原点
	}

    HomingZeroNearStatusUpdate(); // 更新减速点及原点信号状态

	//原点复归过程中超程
    //遇到行程限位的处理 0-手动反向找零 1-自动找零处理(H05.40_bit1)
    if((STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome) && (pHome->Step == 6))
    {   // 遇到了限位开关
		if ((pHome->OriginSel.bit.IsLimited) || (3 == pHome->HomeEnable))
		{   // 设置了自动反向重新开始找原点
			pHome->Trigger = 1;                         // 此时自动触发

            if (1 == pHmIntp->IntpltStatus)
			{
				pHmIntp->IntpltStatus = 0;              // 立即结束当前的插补, 开始反向
			    STR_FUNC_Gvar.PosCtrl.PosReguOut = 0;   // 位置调节器输出的速度指令

				// 当前一次的脉冲偏差, 由脉冲差清除功能确定什么时候清除
			    STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;
			    STR_InnerGvarPosCtrl.PosErrLast = 0;    //前馈速度值及AO输出		
			    STR_InnerGvarPosCtrl.FdFwdOut = 0;      //前馈速度值及AO输出
			    STR_FUNC_Gvar.PosCtrl.DovarReg_Coin = 0;    //位置到达信号清零
			    STR_FUNC_Gvar.PosCtrl.DovarReg_Near = 0;    //位置接近清零
			}
		}
		STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0; // 告知系统当前不处于回零过程
		pHmIntp->IntpltStatus = 0;   // 准备重新开始找原点
		pHome->DirInvFlag = 1;       // 置反向标志
		pHome->Dist = -pHome->Dist;  // 距离值反向
	    
		//高速寻找时走至开关打开再进入低速
		if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.Pot)
		    && (INVALID == STR_FUNC_Gvar.DivarRegLw.bit.Not))
		{
			pHome->OTStatus |= 1;  // 遇到正限位
		}
		else if ((INVALID == STR_FUNC_Gvar.DivarRegLw.bit.Pot)
		         && (VALID == STR_FUNC_Gvar.DivarRegLw.bit.Not))
		{
			pHome->OTStatus |= 2;  // 遇到负限位
		}
		else if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.Pot)
		         && (VALID == STR_FUNC_Gvar.DivarRegLw.bit.Not))
		{   // 无效的限位信号
	 	    PostErrMsg(ORIGINOVERTIME);//原点回零超时
		}

		if((pHome->OTStatus == 1)||(pHome->OTStatus == 2))
		{
			pHome->LenCmpsFlag = 1;  // 遇到限位时设置长度补偿标志
		}
		else if(pHome->OTStatus == 3)
		{   // 第二次遇到限位
			pHome->LenCmpsFlag = 0;
			pHome->OTStatus = 0;
		}		

		pHome->Step = 0;            // 回零步骤标志清0
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome == 0)
    {   // 限位信号解除
  	    pHome->DirInvFlag = 0;	    // 反向标志清0
    }
    
	switch(pHome->Step)
	{			
	case 0: //判断原点回零是否触发
	    // 多段位置，中断定长运行中不允许原点复归
        if((POSMOD == STR_FUNC_Gvar.MonitorFlag.bit.RunMod)	        // 要求是位置模式
		   && (RUN == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus) // 要求伺服正运行
		   && (INVALID == STR_FUNC_Gvar.DivarRegLw.bit.PosInSen)      // 多段位置没有使能
		   && (0 == STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork)    // 不处于中断定长
		   && (0 == pHmIntp->IntpltStatus)					        // 不处于插补过程
		   && (0 != pHome->HomeEnable)                              // 开启了原点回归
		   && (0 == STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork) )   // 不处于前一个原点回归过程
	    {
            HomingTriggerUpdate(); //  获取触发信号状态

		    if(pHome->Trigger) //有效电平上升沿触发
		    {
		        pHome->HomActFlg = 1;//回零启动
		        
                if((pHome->Mode == 6)
                    ||(pHome->Mode == 8))
                {
                    STR_FUNC_Gvar.PosCtrl.HomStats = 1; //回零启动
                }

                if((pHome->Mode == 7)
                    ||(pHome->Mode == 9))
                {
                    STR_FUNC_Gvar.PosCtrl.HomStats = 2; //回零启动
                }
                
                // 超时报警再次触发后, 清除回零超时警告以及相关标志位
                if (1 == u16IsOverTime)
                {
                    u16IsOverTime = 0;
                    WarnAutoClr(ORIGINOVERTIME);

                    // 距离值可能已经取反了, 这里要恢复过来
                    pHome->Dist = ABS(pHome->Dist);
                    pHome->LenCmpsFlag = 0;
                    pHome->DirInvFlag = 0;
                    pHome->ZeroPosFlag = 0;
                    pHome->OTStatus = 0;
                    pHome->ZeroDownEdge = 0;
                }                

				if(pHome->Elector)     // 电气回零
				{
					pHome->Near = 0;   // 减速点信号清0
					
					// 设置电气回零的运行距离
					OriginMoveDistTmp = pHome->Offset - (int32)( ((int64)STR_InnerGvarPosCtrl.CurrentAbsPos * (int64)STR_InnerGvarPosCtrl.Denominator )
					                    / (int64)STR_InnerGvarPosCtrl.Numerator); 


					// 启动电气回零
					LineIntpltInit(HM_STARTSPD, pHome->HighSpeedSearch, HM_STOPSPD,
					               pHome->RiseDownTime, pHome->RiseDownTime,
					               OriginMoveDistTmp, pHmIntp);
					OriginMoveDistSign = (OriginMoveDistTmp > 0) ? 0 : 1;
					pHome->Step = 1;
				}
				else  // 普通回零
				{
					// 反向找原点
					if(pHome->Mode%2)
					{
						OriginMoveDistTmp = 0-pHome->Dist;
					}
					else // 正向找原点
					{
						OriginMoveDistTmp = pHome->Dist;
					}

                    //将插补函数放到else下面
					if(pHome->Near) //起步时位于原点位置则直接进入低速找零
					{
                        //下面这句话要注释掉，否则编译不通过
//						pHmIntp->IntpltStatus = 0;
						pHome->ZeroPosFlag = 1;
						pHome->Step = 2; 
					}
					if(0 == pHome->Near)
					{
					    // 启动高速搜索原点
    					LineIntpltInit(HM_STARTSPD, pHome->HighSpeedSearch, HM_STOPSPD,
    					               pHome->RiseDownTime, pHome->RiseDownTime,
    								   OriginMoveDistTmp, pHmIntp);
					    OriginMoveDistSign = (OriginMoveDistTmp > 0) ? 0 : 1;
						pHome->Step = 1; 
						pHome->ZeroPosFlag = 0;  			
					}
				}
				
				pHome->TimeCount = 0;           
				pHome->Trigger = 0;
				pHome->Status = 0;
				pHome->ZeroIndex = 0;
				STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 1;  // 回零过程正在执行
		    }
        }
	    break;
	
	case 1://按回零方向高速搜素减速点
	    if(pHome->Elector)
		{
		    //电气回零结束
		    if (pHmIntp->IntpltStatus==0)
		    {
	            pHome->Step = 7;
		        pHome->Status |=0x02;
				STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0;  // 当前不处于回零过程
				
				// 如果使用了Z信号中断, 则完成之后清除
				//if (pHome->Mode > 1)
				//{
				    //ZIntDrvClear();
				//}
		    }
			
			// 检查电气回零是否超时
			HomingOverTimeCheck();
		}
        else
		{
	        // 高速运行时超程
		    if ((STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome) && (0 == pHome->DirInvFlag)
			      &&(STR_FUNC_Gvar.PosCtrl.HomStats==0))
		    {
				//pHome->Step = 6; // 超程则反向重新开始找
                if(  ((1 == STR_FUNC_Gvar.DivarRegLw.bit.Pot) && (0 == OriginMoveDistSign))
                  || ((1 == STR_FUNC_Gvar.DivarRegLw.bit.Not) && (1 == OriginMoveDistSign)) )
                {
                    pHome->Step = 6;   
                }
		    }
               
	        HomingHighSpeedSearch();
		}
	    break;
	
	case 2://检测高速减速是否完成
		if((pHmIntp->IntpltStatus==0) && (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < 1000000L))		
		{
            STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;
            if(pHome->Mode%2)
		    {
			   	OriginMoveDistTmp = pHome->Dist;
		    }
		    else
		    {
			   	OriginMoveDistTmp = 0 - pHome->Dist;
		    }

		    if((pHome->ZeroDownEdge))
		    {
		   	    OriginMoveDistTmp = -OriginMoveDistTmp;
				if(pHome->ZeroDownEdge == 1)
				{
					pHome->ZeroDownEdge = 2;
				}
		    }

			// 启动低速搜索原点
            LineIntpltInit(HM_STARTSPD, pHome->LowSpeedSearch, HM_STOPSPD, pHome->RiseDownTime,
			               pHome->RiseDownTime, OriginMoveDistTmp, pHmIntp);						   
            OriginMoveDistSign = (OriginMoveDistTmp > 0) ? 0 : 1;

	        pHome->Step = 3;
		}

        HomingOverTimeCheck();
	    break;
		
	case 3:
		// 低速找零过程中超程触发
		if ((STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome) && (pHome->DirInvFlag == 0)
		    &&(STR_FUNC_Gvar.PosCtrl.HomStats==0))
		{
			//pHome->Step = 6; // 超程则反向重新开始找
            if(  ((1 == STR_FUNC_Gvar.DivarRegLw.bit.Pot) && (0 == OriginMoveDistSign))
              || ((1 == STR_FUNC_Gvar.DivarRegLw.bit.Not) && (1 == OriginMoveDistSign)) )
            {
                pHome->Step = 6;   
            }
		}
	    HomingLowSpeedSearch();
	    break;
		
	case 4:
        // 原点复归完成后偏移量处理
        if(pHome->MoveDist) 
		{
		    if(pHmIntp->IntpltStatus == 0  && ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr)<4)	
		    {
                // 插补定位完成之后, 再移动偏移量
                LineIntpltInit(HM_STARTSPD, pHome->HighSpeedSearch, HM_STOPSPD, pHome->RiseDownTime,
			                   pHome->RiseDownTime, (int64)(pHome->MoveDist), pHmIntp);
			    pHome->Step = 5;
		    }
		}
		else
		{
            pHome->Step = 7;
            pHome->Status = 1;//回零成功
			STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0;
		}
	    break;
		
	case 5:
	    if((pHmIntp->IntpltStatus==0)
		   && (ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < 4)
		   && (RUN == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus))
		{
            pHome->Step = 7;
            pHome->Status = 1;//回零成功
			STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0; // 告知系统已结束回零过程
        } 
	    break;
		
	case 6:
	    break;

	case 7:
	    if ((4 == pHome->HomeEnable) || (5 == pHome->HomeEnable))
		{   // 伺服ON之后立即回普通原点 或者 立即回电气原点
		    FunCodeUnion.code.PL_OriginResetEn = 0;
			pHome->HomeEnable = 0 ;
	        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_OriginResetEn));		
		}
        
        STR_FUNC_Gvar.PosCtrl.HomStats = 0; //回零结束
        pHome->HomActFlg = 0;//回零未启动

        
		pHome->Step = 0;
	
    default:
	    break;
	}

	pHome->NearBkp = pHome->Near;  // 备份当前减速点信号状态
	pHome->ZeroBkp = pHome->Zero;  // 备份当前原点信号状态

    // 原点复归过程中, 如果伺服停机或者异常则提前结束原点复归工作
	if (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)
	{
        if (pHome->Step > 0)
        {
    	    pHmIntp->IntpltStatus = 0;
    		pHome->Step = 0;		  
    		pHome->DirInvFlag = 0;
    		STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0;  //  当前不处于回零过程
    		
    		// 如果使用了Z信号中断, 则完成之后清除
    		//if (pHome->Mode > 1)
    		//{
    		    //ZIntDrvClear();
    		//}
        }

        // 清除回零超时警告
        WarnAutoClr(ORIGINOVERTIME);
	}
}


/*******************************************************************************
  函数名: void HomingTriggerUpdate() 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: 原点复归使能方式处理
		  0-关闭原点复归功能
		  1-通过DI输入ORGSET信号来使能原点复归功能
		  2-通过DI输入ORGSET信号使能电气回原点功能
	      3-上电后立即启动原点复归
		  4-立即进行原点复归
		  5-启动电气回原点命令
		  6-以当前位置为原点
********************************************************************************/ 
Static_Inline void HomingTriggerUpdate(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;
	
    if(pHome->HomeEnable == 1) 
    {   //IO触发普通回零
	    if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.OrgChuFa) && (INVALID == pHome->DITriggerOld))
	    {
		    pHome->Trigger = 1; // 原点回归触发标志
		    pHome->Elector = 0; // 电气回零标志
	    }
    }
	else if(pHome->HomeEnable == 2) 
    {   //IO触发电气回零
	    if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.OrgChuFa) && (INVALID == pHome->DITriggerOld))
	    {
		    pHome->Trigger = 1;
		    pHome->Elector = 1;
	    }
    }
	else if(pHome->HomeEnable == 4) 
    {   //普通立即回零
	    pHome->Trigger = 1;
	    pHome->Elector = 0;
    }
	else if(pHome->HomeEnable == 5) 
    {   //电气立即回零
	    pHome->Trigger = 1;
		pHome->Elector = 1;
    }

    pHome->DITriggerOld = STR_FUNC_Gvar.DivarRegLw.bit.OrgChuFa; // 备份原点开关当前状态
}


/*******************************************************************************
  函数名: void HomingOverTimeCheck() 
  输  入:           
  输  出:   
  子函数: PostErrMsg()                                      
  描  述: 控制寻找原点的时间
********************************************************************************/
Static_Inline void HomingOverTimeCheck(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;
	
	pHome->TimeCount++;  // 搜索原点过程计时累加

	if(pHome->TimeCount > ((Uint32)pHome->SearchTime))
	{   // 超时则停止原点回归过程, 复位相关参数
	    pHome->Status |= 4;//找零超时报警
        pHome->Step = 0;
        STR_HmLnIntplt.IntpltStatus = 0;
		STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork = 0;  // 告知系统当前不处于回零过程
   	    STR_FUNC_Gvar.PosCtrl.HomStats = 0; 

        if (0 == u16IsOverTime)
        {
            u16IsOverTime = 1;
        }
		
		// 如果使用了Z信号中断, 则完成之后清除
		//if (pHome->Mode > 1)
		//{
		    //ZIntDrvClear();
		//}
	    PostErrMsg(ORIGINOVERTIME);//原点回零超时
        if ((4 == pHome->HomeEnable) || (5 == pHome->HomeEnable))
		{   // 伺服ON之后立即回普通原点 或者 立即回电气原点
		    FunCodeUnion.code.PL_OriginResetEn = 0;
		    pHome->HomeEnable = 0 ;
	        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_OriginResetEn));
	    }
	}
}


/*******************************************************************************
  函数名: void HomingZeroNearStatusUpdate() 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: 更新减速点位置及原点位置设置
		  0-正向回零, 原点和减速点都是原点开关
		  1-反向回零, 原点和减速点都是原点开关
		  2-正向回零, 原点和减速点都是电机Z信号
		  3-反向回零, 原点和减速点都是电机Z信号
		  4-正向回零, 减速点是原点开关, 正向找电机Z信号作为原点位置
		  5-反向回零, 减速点是原点开关, 反向找电机Z信号作为原点位置
		  6-正向回零, 原点和减速点都是POT
		  7-反向回零, 原点和减速点都是NOT
		  8-正向回零, 减速点是POT,正向找电机Z信号作为原点位置
		  9-反向回零, 减速点是NOT, 反向找电机Z信号作为原点位置
********************************************************************************/
Static_Inline void HomingZeroNearStatusUpdate(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;

    switch(pHome->Mode)
    {
        case 0:
        case 1:
            pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.OrgNear;
            pHome->Zero = STR_FUNC_Gvar.DivarRegLw.bit.OrgNear;
            break;
        case 2:
        case 3:
            pHome->Near = pHome->ZeroIndex;
            pHome->Zero = pHome->ZeroIndex;
            break;
        case 4:
        case 5:
           pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.OrgNear; 
           pHome->Zero = pHome->ZeroIndex;
           break;
        case 6:
           pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.Pot;
           pHome->Zero = STR_FUNC_Gvar.DivarRegLw.bit.Pot;
           break;
        case 7:
           pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.Not;
           pHome->Zero = STR_FUNC_Gvar.DivarRegLw.bit.Not;
           break;
        case 8:
           pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.Pot;
           pHome->Zero = pHome->ZeroIndex;
           break;
        case 9:
           pHome->Near = STR_FUNC_Gvar.DivarRegLw.bit.Not;
           pHome->Zero = pHome->ZeroIndex;
           break;
        default:
           pHome->Near = 0;  
           pHome->Zero = 0;
           break;
            
    }

}


/*******************************************************************************
  函数名: void HomingHighSpeedSearch() 
  输  入:           
  输  出:   
  子函数: HomingOverTimeCheck()                                      
  描  述: 回零过程中, 高速搜索减速点
********************************************************************************/
Static_Inline void HomingHighSpeedSearch(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;
	
    if(pHome->Mode<2 || pHome->Mode>3)
	{   //减速点是原点开关
  	    if(pHome->Near && !pHome->NearBkp)
	    {   // 超程后的原点复归处理，使平台找零参考侧一致
			if(pHome->LenCmpsFlag == 0)
			{   //停止匀速运动,开始降速
				ResetIntpltDownLength(&STR_HmLnIntplt);
				STR_HmLnIntplt.LineRemainLengthQ16 = STR_HmLnIntplt.DownLengthQ16;
				if(STR_HmLnIntplt.LineRemainLengthQ16 == 0)
				{
					STR_HmLnIntplt.IntpltStatus=0;		
				}
				pHome->Step = 2;
				pHome->ZeroIndex = 0;  // Z信号锁存清理
			}
	    }
		
	    //LW等待另一侧触发原点开关
	    if((pHome->LenCmpsFlag == 1) && (!pHome->Near) && (pHome->NearBkp))
	    {
	  	   	//停止匀速运动,开始降速
			ResetIntpltDownLength(&STR_HmLnIntplt);
		    STR_HmLnIntplt.LineRemainLengthQ16 = STR_HmLnIntplt.DownLengthQ16;
			if(STR_HmLnIntplt.LineRemainLengthQ16==0)
			{
			    STR_HmLnIntplt.IntpltStatus=0;		
			}
		    pHome->Step = 2;
	        pHome->ZeroIndex = 0;   // Z信号锁存清理
	    }
	}
	else
	{   // 减速点是电机Z信号
	    if(pHome->ZeroIndex)
	    {
		    ResetIntpltDownLength(&STR_HmLnIntplt);
			STR_HmLnIntplt.LineRemainLengthQ16 = STR_HmLnIntplt.DownLengthQ16;
			pHome->Step = 2;
			pHome->ZeroIndex = 0;   // Z信号锁存清理
	    }
	}

    HomingOverTimeCheck(); // 控制搜索原点的时间
}

/*******************************************************************************
  函数名: void HomingLowSpeedSearchEnd() 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: 处理搜索原点结束时的情况
********************************************************************************/
Static_Inline void HomingLowSpeedSearchEnd(void)
{
    int64 tempDist = 0;
    STR_HOME_ATTRIB *pHome = &STR_Home;
	
    //停止匀速运动,开始降速
    STR_InnerGvarPosCtrl.PosErrLast = 0;
    STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0;
    STR_HmLnIntplt.LineRemainLengthQ16 = 0;
    STR_HmLnIntplt.IntpltStatus = 0;
    pHome->Step = 4;//LW10324
    if(pHome->LenCmpsFlag == 1)
    {
   	    pHome->LenCmpsFlag = 0;
        pHome->Dist = -pHome->Dist;
    }

    //绝对位置计数器,1个位置指令单位
	tempDist = ((int64)(pHome->Offset) << LINEAMPBIT)
				* STR_InnerGvarPosCtrl.Numerator / STR_InnerGvarPosCtrl.Denominator;		
 	DINT;
    STR_InnerGvarPosCtrl.CurrentAbsPos = (int32)(tempDist >> LINEAMPBIT);
    STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt = STR_InnerGvarPosCtrl.CurrentAbsPos;
    STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.CurrentAbsPos;
    if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos < 0)
    {
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
    }

    if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
    {
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
    }
    
	STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.CurrentAbsPos;
    if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt < 0)
    {
        if(((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)<=0)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
        }

        if(((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 100)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = -(STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + STR_InnerGvarPosCtrl.AbsMod2PosUpLmt);
        }
    }
    else 
    {
        if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
        }

        if(((int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - (int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt) <= 100)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
        }
    }
    EINT;

    STR_InnerGvarPosCtrl.InputPulseCnt = 0;  // 指令计数器也清除
    STR_InnerGvarPosCtrl.PulseCalcRemainder = 0;

        if((1 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)||(3 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection))
    {
        AbsMod1_MultiTurnOffset();
        STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset = 1;
    }  
    else if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
    {
        STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod2RestRemSum = 1;
    }
	FullCloseParaRst();
	// 如果使用了Z信号中断, 则完成之后清除
	//if (pHome->Mode > 1)
	//{
	    //ZIntDrvClear();
	//}
}

/*******************************************************************************
  函数名: void HomingLowSpeedSearch() 
  输  入:           
  输  出:   
  子函数: HomingLowSpeedSearchEnd(), HomingOverTimeCheck()                                      
  描  述: 低速搜索原点
********************************************************************************/
Static_Inline void HomingLowSpeedSearch(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;
    if((pHome->Mode<2)||(pHome->Mode==6)||(pHome->Mode==7))
	{   // 回零模式0和1, 原点为原点开关
	    if(pHome->LenCmpsFlag == 1)
	    {   //LW10324超程后原点复归低速碰原点开关处理
	  	    switch(pHome->ZeroPosFlag)
		    {
		  	    case 0: // 起始位置不在原点开关处
				    if(pHome->Zero && !pHome->ZeroBkp)
				    {   // 遇到原点开关信号的上升沿, 结束回零过程
			            HomingLowSpeedSearchEnd();
			            pHome->ZeroIndex = 0;      // Z信号锁存清理
				    }
				    break;
					
			    case 1:	// 起始位置在原点开关处
			 	    if(!pHome->Zero && pHome->ZeroBkp)
			        {	// 遇到原点开关信号的下降沿, 跳至回零过程的第二步
					    STR_HmLnIntplt.IntpltStatus = 0;
		                pHome->Step = 2;
					    pHome->ZeroIndex = 0;      // Z信号锁存清理
					    pHome->ZeroPosFlag = 0;
			        }
			        break;
					
			    default :
			        break;
		    }
	    }
	    else
	    {
	  	    if (!pHome->Zero && pHome->ZeroBkp && pHome->ZeroDownEdge == 0)
		    {
			    STR_HmLnIntplt.IntpltStatus = 0;
				pHome->Step = 2;
				pHome->ZeroIndex = 0;    // Z信号锁存清理
				pHome->ZeroDownEdge = 1;
		    }

		    if (pHome->Zero && !pHome->ZeroBkp && pHome->ZeroDownEdge == 2)
		    {
				HomingLowSpeedSearchEnd();
				pHome->ZeroIndex = 0;    // Z信号锁存清理
				pHome->ZeroDownEdge = 0;
		    }
	    }
    }
    else if(pHome->Mode<4)
    {   // 回零模式2和3, 原点为电机Z信号
	    if(pHome->ZeroIndex)
	    {
			HomingLowSpeedSearchEnd();
			pHome->ZeroIndex = 0;
        }
    }
    else if((pHome->Mode < 6)||(pHome->Mode==8)||(pHome->Mode==9))
    {   // 回零模式4和5, 减速点为原点开关, 原点为电机Z信号
	    if(pHome->LenCmpsFlag == 1)
	    {
	  	    switch(pHome->ZeroPosFlag)
		    {
		  	    case 0:
			        if((pHome->Near) && (!pHome->NearBkp))
				    {   // 锁存捕捉到开关的上升沿, 因为回原点流程位于主循环, 可被中断调度抢断
				  		pHome->NearUpEdge = 1;
				    }
				    if((pHome->ZeroIndex) && (pHome->NearUpEdge ==1))
				    {
						pHome->NearUpEdge = 0;
						HomingLowSpeedSearchEnd();
						pHome->ZeroDownEdge = 0;
						pHome->ZeroIndex = 0;
			        }
				    break;
					
			    case 1:
			 	    if((!pHome->Near) && (pHome->NearBkp))
			        {
					    STR_HmLnIntplt.IntpltStatus = 0;
		                pHome->Step = 2;
					    pHome->ZeroPosFlag = 0;
			        }
			        break;
					
			    default :
			        break;
		    }
	    }
	    else
	    {
	  	    if(!pHome->Near && pHome->NearBkp)
		    {
			    STR_HmLnIntplt.IntpltStatus = 0;
			    pHome->Step = 2;
			    pHome->ZeroDownEdge = 1;
		    }

		    if (pHome->Near && !pHome->NearBkp && pHome->ZeroDownEdge == 2)
		    {  // 锁存捕捉到原点开关的上升沿, 因为回原点流程位于主循环, 可被中断调度抢断
		  	    pHome->NearUpEdge = 1;
		    }

		    if((pHome->ZeroIndex) && (pHome->NearUpEdge ==1))
		    {
				pHome->NearUpEdge = 0;
				pHome->ZeroDownEdge = 0;
				HomingLowSpeedSearchEnd();
				pHome->ZeroIndex = 0;
	        }
	    }
	}
	else
	{
	}

    HomingOverTimeCheck(); // 控制搜索原点的时间
}

/*******************************************************************************
  函数名: void ZeroIndexISR() 
  输  入:           
  输  出:   
  子函数: ZeroIndexISR()                                      
  描  述: z信号中断服务程序
********************************************************************************/
void ZeroIndexISR(void)
{
    STR_HOME_ATTRIB *pHome = &STR_Home;
	
    if (1 == pHome->Step)
    {
        pHome->ZeroIndex = 1;
    }

    //不能用step3判断，否则回提前置Z信号标志位为1 liuwei10324
	if (((((pHome->Mode < 4) && (pHome->Mode > 1))) && (3 == pHome->Step))
     	|| (1 == pHome->NearUpEdge))
    {
	  	pHome->ZeroIndex = 1;
		STR_HmLnIntplt.IntpltStatus = 0;
		
		STR_FUNC_Gvar.PosCtrl.PosReguOut = 0;   // 位置调节器输出的速度指令
		STR_FUNC_Gvar.PosCtrl.PosAmplifErr = 0; // 随动偏差
		STR_InnerGvarPosCtrl.PosErrLast = 0;    // 前馈速度值及AO输出		
		STR_InnerGvarPosCtrl.FdFwdOut = 0;      // 前馈速度值及AO输出
		STR_FUNC_Gvar.PosCtrl.DovarReg_Coin = 0;    // 位置到达信号清零
		STR_FUNC_Gvar.PosCtrl.DovarReg_Near = 0;    // 位置接近清零
	}

}

/********************************* END OF FILE *********************************/
