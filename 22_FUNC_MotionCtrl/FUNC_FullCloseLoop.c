                    /******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_MultiBlockPos.c
 创建人：刘伟                   创建日期：2012.9.19
 
 描述： 
     1.全闭环功能实现

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_FullCloseLoop.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_PosCtrl.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
STR_FULLCLOSE_LOOP STR_FullCloseLoop;
static int64 InnerEncoderFeedbackSum = 0; 
static int64 ExEncoderFeedbackSum = 0;
static int32	FC_ExPulseFeedback=0;
static int32	FC_InnerPulseFeedback=0;
static int64 DeltaSum = 0;



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   

void FullCloseLoopErrorCheck(Uint8 ExPosFeedbackFlag); 
void FullCloseLoopModeSet(int32 * pPosFdb, Uint32 *pExPosCoefQ7, int32 ExPosFdbAbsValue,Uint8 ExPosFeedbackFlag) ;
void FullCloseLoopInit(Uint32 EncRev);
void ClearFullLoopVar(void);
int32 DoublePosFeedbackCal(int32 PosRef,int32 *PosAmplifErr);
void FullCloseParaDisp(void);
void FullLoopLowPassFilterInit(void);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void FPGA_InitLnrEncCtrl(void);
/*******************************************************************************
  函数名: void FullCloseParaDisp(void) 
  输  入:        
  输  出:   内部位置累积，外部位置累积，外部位置偏差值
  子函数:                                       
  描  述: 全闭环的初始化函数  
********************************************************************************/

void FullCloseParaDisp(void)
{
	
     //内部编码器反馈显示
	FunCodeUnion.code.FC_InnerPulseFeedback_L = AuxFunCodeUnion.code.DP_FbPuleCnt_Lo;
    FunCodeUnion.code.FC_InnerPulseFeedback_H = AuxFunCodeUnion.code.DP_FbPuleCnt_Hi;
	//外部编码器反馈显示
    FunCodeUnion.code.FC_ExPulseFeedback_L = FC_ExPulseFeedback;
	FunCodeUnion.code.FC_ExPulseFeedback_H = (FC_ExPulseFeedback>>16);

	//外部偏差值反馈显示
	FunCodeUnion.code.FC_ExInPosErrSum_Lo = DeltaSum;//STR_FullCloseLoop.ExPosAmplifErr;
	FunCodeUnion.code.FC_ExInPosErrSum_Hi = (DeltaSum>>16);//STR_FullCloseLoop.ExPosAmplifErr>>16;	       
}


/*******************************************************************************
  函数名: void FullCloseLoopInit(void) 
  输  入:        EncRev, encoder revolution
  输  出:   
  子函数:                                       
  描  述: 全闭环的初始化函数  
********************************************************************************/

void FullCloseLoopInit(Uint32 EncRev)
{
	int64 ExCoder;
	STR_FullCloseLoop.FeedbackMode = FunCodeUnion.code.FC_FeedbackMode; 
	STR_FullCloseLoop.ExCoderDir   = FunCodeUnion.code.FC_ExCoderDir;
    
    //STR_FullCloseLoop.ExCoderLine = A_SHIFT16_PLUS_B(FunCodeUnion.code.FC_ExCoderLine_H,FunCodeUnion.code.FC_ExCoderLine_L);

    ExCoder = A_SHIFT16_PLUS_B(FunCodeUnion.code.FC_ExCoderPulse_H,FunCodeUnion.code.FC_ExCoderPulse_L);

    STR_FullCloseLoop.ExP2InerPosCoffQ7 = (EncRev<<7)/ExCoder;
    
	STR_FullCloseLoop.ExInnerCoderParaQ20 = (ExCoder<<20) / EncRev;

	
	STR_FullCloseLoop.EncRev = EncRev;

    FullLoopLowPassFilterInit();

//	FPGA_InitLnrEncCtrl();
}


/*******************************************************************************
  函数名: void FullCloseLoopModeSet(void) 
  输  入:    pPosFdb,内部位置反馈，pExPosCoefQ7外增益系数   ,单位转换系数
  ExPosFdbAbsValue 外部位置反馈绝对值
  ExPosFeedbackFlag模式2时外部闭环起用标志
  输  出:   
  子函数:                                       
  描  述: 多段位置参数复位  
********************************************************************************/

void FullCloseLoopModeSet(int32 * pPosFdb, Uint32 *pExPosCoefQ7, int32 ExPosFdbAbsValue,Uint8 ExPosFeedbackFlag) 
{
	static  int32 ExPosFdbAbsValueLast =0;
	static  Uint8 ExPosFdbClrFlg =0;
    static  int64 PosFdbRemainderQ20 =0;
			int32 TempPosFdb;
            int64 Temp;

	//缓存内部位置反馈值
	TempPosFdb = *pPosFdb;
	
	//外部位置增量计算
	STR_FullCloseLoop.ExPosFdb = ExPosFdbAbsValue - ExPosFdbAbsValueLast;

	ExPosFdbAbsValueLast = ExPosFdbAbsValue;	 //锁存当前绝对位置反馈值，以便下次计算增量式位置反馈用

	if (ExPosFdbClrFlg == 0)  //上电第一次清除位置反馈增量值
	{
		ExPosFdbClrFlg = 1;

		STR_FullCloseLoop.ExPosFdb = 0; 				
	}


    //反馈反向调整，确保和电机运行一致
    if(STR_FullCloseLoop.ExCoderDir)
    {
       STR_FullCloseLoop.ExPosFdb = -STR_FullCloseLoop.ExPosFdb;
    }

    //缓存内部位置反馈值，用于位置偏差过大判断
    STR_FullCloseLoop.InPosFdb = TempPosFdb;
    //编码器反馈显示
    FC_InnerPulseFeedback += STR_FullCloseLoop.InPosFdb;

    STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos    += STR_FullCloseLoop.ExPosFdb;


	FC_ExPulseFeedback = STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos;
	STR_FUNC_Gvar.PosCtrl.ExPosFdb        = STR_FullCloseLoop.ExPosFdb;

     if((STR_FullCloseLoop.FeedbackMode == 1)||(ExPosFeedbackFlag == 1))
     {
		 *pExPosCoefQ7 = STR_FullCloseLoop.ExP2InerPosCoffQ7;//FunCodeUnion.code.ExKpCoff;
		 //防止默认增益系数为0时，全闭环位置环增益为0， 不起作用
		 if((*pExPosCoefQ7) == 0)
		 {
			 *pExPosCoefQ7 = 128;
		 }
     }
	 else
	 {
	 	 *pExPosCoefQ7 = 128;
	 }

     if((STR_FullCloseLoop.FeedbackMode == 1)
        || (ExPosFeedbackFlag == 1))
        
     {        
        
         //双位置反馈使能
         if(FunCodeUnion.code.FC_ExInErrFilterTime != 0)
         {
    	     Temp = ((STR_FullCloseLoop.ExInnerCoderParaQ20 * (int64)STR_FullCloseLoop.InPosFdb  + PosFdbRemainderQ20)>>20);
             *pPosFdb  = Temp;
             PosFdbRemainderQ20 = (int64)STR_FullCloseLoop.ExInnerCoderParaQ20 * STR_FullCloseLoop.InPosFdb  + PosFdbRemainderQ20 - (Temp<<20);
    	 } 
         else
         {
             *pPosFdb = STR_FullCloseLoop.ExPosFdb;
         }
    
     }
//	 else if(FunCodeUnion.code.FC_FeedbackMode == 3)
	 

     else
     {
         *pPosFdb = STR_FullCloseLoop.InPosFdb;
         *pExPosCoefQ7 = 128;
     }
}


/*******************************************************************************
  函数名: void FullCloseLoopErrorCheck(void) 
  输  入:   内部位置反馈，外部位置反馈  
                   ExPosFeedbackFlag  内外切换模式下外环作用标志
                   RunStats 驱动器的运行状态标志2:0n 1:rdy,3:err
  输  出:   NULL
  子函数:                                       
  描  述:  混合控制偏差过大报警
********************************************************************************/
void FullCloseLoopErrorCheck(Uint8 ExPosFeedbackFlag) 
{

    static int32 CalCycNum = 0;   //计算圈数

    static int32 CalCycPuls = 0;  //计算圈数的脉冲累加值

    STR_FullCloseLoop.MixCtrlMaxPulse = A_SHIFT16_PLUS_B(FunCodeUnion.code.FC_MixCtrlMaxPulse_H,FunCodeUnion.code.FC_MixCtrlMaxPulse_L);

	STR_FullCloseLoop.MixCtrlPulseClr = FunCodeUnion.code.FC_MixCtrlPulseClr;

	if((STR_FullCloseLoop.MixCtrlMaxPulse == 0) 
      || (STR_FullCloseLoop.MixCtrlPulseClr == 0) 
      || ((ExPosFeedbackFlag == 0) && (STR_FullCloseLoop.FeedbackMode != 1))
	  || (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN))
	{
		InnerEncoderFeedbackSum = 0;
		ExEncoderFeedbackSum = 0;

        DeltaSum = 0;
        STR_FUNC_Gvar.PosCtrl.ExInPosErrSum = DeltaSum; //外部偏差值反馈示波器用 
		return;
	}
    
     InnerEncoderFeedbackSum += STR_FullCloseLoop.InPosFdb;
    
	 ExEncoderFeedbackSum += STR_FullCloseLoop.ExPosFdb;

     CalCycPuls += STR_FullCloseLoop.InPosFdb;
  
    //若电机旋转一圈，圈数加一，重新计数
    if(ABS(CalCycPuls)>= STR_FullCloseLoop.EncRev)
     {
         CalCycNum++ ;
         
         CalCycPuls = 0;
     }

    //在设定的圈数内，判断是否偏差过大
	if(CalCycNum < STR_FullCloseLoop.MixCtrlPulseClr)
	{
        DeltaSum = ABS(ExEncoderFeedbackSum - ((InnerEncoderFeedbackSum*STR_FullCloseLoop.ExInnerCoderParaQ20)>>20));
        //内外圈偏差过大或者方向相反长达500ms，报警
        if(DeltaSum > (int64)STR_FullCloseLoop.MixCtrlMaxPulse)
        {
            
    		if((STR_FullCloseLoop.FeedbackMode == 1)
            || (ExPosFeedbackFlag == 1))
            
            {
                PostErrMsg(FULLCLOSEERR);
                ExEncoderFeedbackSum = 0;
                InnerEncoderFeedbackSum = 0;
            }

            else
            {
                ExEncoderFeedbackSum = 0;
                InnerEncoderFeedbackSum = 0;
            }
		}
        
        STR_FUNC_Gvar.PosCtrl.ExInPosErrSum = DeltaSum; //外部偏差值反馈示波器用 	
	}
	else
	{
		 InnerEncoderFeedbackSum = 0;
		 ExEncoderFeedbackSum = 0;

		 CalCycNum = 0;
	} 	
}
/*******************************************************************************
  函数名:  DoublePosFeedbackCal()

  输入:    STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
           STR_FUNC_Gvar.PosCtrl.PosAmplifErr
               
  输出:    外内偏差经低通滤波后和内部偏差之和
           
  子函数:  MTR_NewLowPassFiltCalc(); 
        
  描述:    计算位置偏差(增量)，以提供位置调节器使用，以及判断执行位置偏差清除功能
********************************************************************************/ 

int32 DoublePosFeedbackCal(int32 PosRef,int32 *PosAmplifErr)
{

    //外内随动偏差之差 经 低通滤波器
    int64 Temp;

	STR_FullCloseLoop.ExPosAmplifErrLast = STR_FullCloseLoop.ExPosAmplifErr;
	STR_FullCloseLoop.ExPosAmplifErr  = (PosRef) - STR_FullCloseLoop.ExPosFdb + STR_FullCloseLoop.ExPosAmplifErrLast;
	
    //外部位置随动偏差用于到位判断
    STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr = STR_FullCloseLoop.ExPosAmplifErr;

    STR_FullCloseLoop.DoubleFeedLowPassInput = STR_FullCloseLoop.ExPosAmplifErr - (*PosAmplifErr);

    STR_FullCloseLoop.DoubleFeedLowPassOutputLatch = STR_FullCloseLoop.DoubleFeedLowPassOutput;

    Temp = STR_FullCloseLoop.DoubleFeedLowPassInput + STR_FullCloseLoop.DoubleFeedLowPassRemainder - STR_FullCloseLoop.DoubleFeedLowPassOutputLatch;

    STR_FullCloseLoop.DoubleFeedLowPassOutput = (Temp/STR_FullCloseLoop.DoubleFeedLowPassKa) + STR_FullCloseLoop.DoubleFeedLowPassOutputLatch;

    STR_FullCloseLoop.DoubleFeedLowPassRemainder = Temp - ((STR_FullCloseLoop.DoubleFeedLowPassOutput - STR_FullCloseLoop.DoubleFeedLowPassOutputLatch)*STR_FullCloseLoop.DoubleFeedLowPassKa);                        
             
    Temp = STR_FullCloseLoop.DoubleFeedLowPassOutput+*PosAmplifErr;
    //当混合偏差等于外环随动偏差时，需清零内部随动偏差和低通滤波器     
    if(Temp == STR_FullCloseLoop.ExPosAmplifErr)
    {                                
        *PosAmplifErr = 0;
        STR_FullCloseLoop.DoubleFeedLowPassRemainder = 0;
        STR_FullCloseLoop.DoubleFeedLowPassOutput = STR_FullCloseLoop.ExPosAmplifErr;
    }

    return (Temp);	//+ STR_FUNC_Gvar.PosCtrl.PosAmplifErr
}


void ClearFullLoopVar(void)
{
    STR_FullCloseLoop.DoubleFeedLowPassInput = 0;
    STR_FullCloseLoop.DoubleFeedLowPassOutput = 0;
    STR_FullCloseLoop.DoubleFeedLowPassOutputLatch = 0;
    STR_FullCloseLoop.DoubleFeedLowPassRemainder = 0;

	 STR_FullCloseLoop.ExPosAmplifErr = 0;
     STR_FullCloseLoop.ExPosAmplifErrLast = 0;

}

void FullLoopLowPassFilterInit(void)
{
    /* 位置环低通滤波初始化 */
    STR_FullCloseLoop.DoubleFeedLowPassTs = (int32)( STR_FUNC_Gvar.System.PosPrd_Q10  >> 10 );     //位置环采样周期，单位us，
    STR_FullCloseLoop.DoubleFeedLowPassTc = 100L * ((int32)FunCodeUnion.code.FC_ExInErrFilterTime);   //初始化位置环调用的低通滤波器滤波时间时间H0504，单位us。
    if (STR_FullCloseLoop.DoubleFeedLowPassTc > STR_FullCloseLoop.DoubleFeedLowPassTs)
    {
        STR_FullCloseLoop.DoubleFeedLowPassKa = (STR_FullCloseLoop.DoubleFeedLowPassTc + STR_FullCloseLoop.DoubleFeedLowPassTs)/STR_FullCloseLoop.DoubleFeedLowPassTs;
    }
    else
    {
        STR_FullCloseLoop.DoubleFeedLowPassKa = 1;
    }    
}

/*******************************************************************************
  函数名: void FullCloseParaRst(void) 
  输  入:        
  输  出:
  子函数:                                       
  描  述: 全闭环参数复位  
********************************************************************************/

void FullCloseParaRst(void)
{
	ClearFullLoopVar();
	STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr = 0;
    DeltaSum = 0;
    STR_FUNC_Gvar.PosCtrl.ExInPosErrSum = DeltaSum; //外部偏差值反馈示波器用 
	
	InnerEncoderFeedbackSum	= STR_InnerGvarPosCtrl.CurrentAbsPos;
	FC_InnerPulseFeedback = InnerEncoderFeedbackSum;

	ExEncoderFeedbackSum = (STR_FullCloseLoop.ExInnerCoderParaQ20 * STR_InnerGvarPosCtrl.CurrentAbsPos)>>20;
	FC_ExPulseFeedback = ExEncoderFeedbackSum;
	STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos = FC_ExPulseFeedback;
    STR_InnerGvarPosCtrl.ExCurrentAbsPos_ToqInt = STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos;
}

///*******************************************************************************
//  函数名: int32 BateVibrationFunc(int32 MotorSpeed,int32 MechSpeed)
//
//  输  入:  电机速度，机械速度    
//  输  出:   转矩补偿量
//  子函数:                                       
//  描  述:  抑振控制功能
//********************************************************************************/
//  int32 BateVibrationFunc(int32 MotorSpeed,int32 MechSpeed)
//  {
//      static int32 DelSpeed = 0;
//      static int64 Sum = 0;
//      static Uint16 Count = 0;
//      static Uint16 FirstFlag = 0;
//      static int32 Buffer[8] = {0};
//      //求平均值     
//      DelSpeed = MotorSpeed - (((int64)MechSpeed * STR_FullCloseLoop.MechInnerSpeedCoff)>>12);
//      Sum += DelSpeed;
//      //滤波次数需要功能码？暂定8次
//      if(Count < 8)
//      {
//           Sum -= Buffer[Count];
//           Buffer[Count] = DelSpeed;
//           Count++; 
//      }
//      else
//      {
//           
//           Count = 0;
//           Sum -= Buffer[Count];
//           Buffer[Count] = DelSpeed;
//           Count++;
//           FirstFlag = 1;
//      }
//      //
//      if(FirstFlag)
//      {
//           //FunCodeUnion.code.BateVibKpCoff为*128的值，对外可宣布为100
//           return (int32)((Sum>>3)*FunCodeUnion.code.BateVibKpCoff>>7);
//      }
//
//      else
//      {
//           return 0;
//      }
//
//  }
//
///*******************************************************************************
//  函数名:  M_SpdFdbLnrCal()      ^_^
//  输入:   1.
//          2.
//  输出:   1.外部编码器速度反馈
//  子函数:无         
//  描述:  得到外部编码器位置反馈，用于全闭环系统调节运算
//********************************************************************************/ 
//void M_SpdFdbLnrCal(void)
//{
//    //全闭环当前绝对位置
//    int32 NewPosLnr;
//    static int16 FirstFlag = 0;
//
//    //全闭环相关中间变量
//    static int32 SpdDelPosLnr;
//    static int32 OldPosLnr = 0;        
//
//    NewPosLnr    = STR_InnerGvarPosCtrl.ExPosFdbAbsValue;
//    SpdDelPosLnr = NewPosLnr - OldPosLnr;
//    OldPosLnr    = NewPosLnr;
//
//    //根据外部编码器反馈方向确定速度方向
//    if(STR_FullCloseLoop.ExCoderDir)
//    {
//         SpdDelPosLnr = -SpdDelPosLnr;
//    }
//
//    if(FirstFlag == 0)                  //第一次进入时需要清零
//    {
//         FirstFlag = 1;
//         SpdDelPosLnr = 0;
//    }
//    STR_MTR_Gvar.SpeedRegulator.M_SpdLnrFdb = STR_MTR_Gvar.SpeedRegulator.M_SpdLnrCoff * SpdDelPosLnr;    
//}


/********************************* END OF FILE *********************************/
