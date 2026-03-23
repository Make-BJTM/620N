/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_EleGear.c
 创建人：高小峰                
 修改人：李浩               创建日期：11.10.18 
 描述： 1. 电子齿轮初始化
        2. 电子齿轮计算
		
 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_EleGear.h"
#include "FUNC_FunCode.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_PosCtrl.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
extern STR_LOCAL_POSCONTROL STR_PosCtrl;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
static Uint32 PulsePRev = 0; //电机每旋转一圈的指令脉冲数

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
int32 EleGearRatioCalc(STR_ELECTRONIC_GEAR *p,int32 CommandInput);

void EleGearInit(STR_ELECTRONIC_GEAR *p);

void EleGearRatioErrCheck(void);

void EleGearGetValue(STR_ELECTRONIC_GEAR *p,int32 CommandInput);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

/*******************************************************************************
  函数名: void EleGearStopUpdata(STR_ELECTRONIC_GEAR *p) 
  输入:   电子齿轮功能码 
  输出:   电子齿轮分子分母子以及电子齿轮切换锁存
  子函数: 无       
  描述: 电子齿轮停机初始化，该函数在伺服OFF和上电复位初始化时调用
********************************************************************************/ 
void EleGearInit(STR_ELECTRONIC_GEAR *p)
{
    Uint32 Num1st = 0;          //电子齿轮分子暂存变量
    Uint32 Den1st = 0;          //电子齿轮分母暂存变量

    Uint64 Temp111 = 0;
    Uint64 Temp222 = 0;

    PulsePRev = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PulsePRevHigh,FunCodeUnion.code.PL_PulsePRevLow) ; //电机每旋转一圈的指令脉冲数赋值(H0502~H0503)
    
    if(PulsePRev==0)
	{
		EleGearRatioErrCheck();
            
    	/*根据此时电子切换Di信息，更新当前电子齿轮的分子分母值*/ 
    	if (STR_FUNC_Gvar.DivarRegLw.bit.GearSel == 1)
    	{
        	p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCmxHigh,FunCodeUnion.code.PL_PosSecCmxLow); //第2组电子齿轮分子
       	 	p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCdvHigh,FunCodeUnion.code.PL_PosSecCdvLow); //第2组电子齿轮分母
    	}
    	else
    	{
        	p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCmxHigh,FunCodeUnion.code.PL_PosFirCmxLow); //第1组电子齿轮分子
        	p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCdvHigh,FunCodeUnion.code.PL_PosFirCdvLow); //第1组电子齿轮分母
    	}
    
		p->GroupSelLatch = STR_FUNC_Gvar.DivarRegLw.bit.GearSel;  //锁存电子齿轮组选择值     
    }
	else
	{
		p->Numerator = UNI_FUNC_MTRToFUNC_InitList.List.EncRev; //不响应齿轮比切换,不检查齿轮比范围
        p->Denominator = PulsePRev;

		Num1st = p->Numerator;    
    	Den1st = p->Denominator;
    
    	/*第一组电子齿轮比范围判断，规格范围0.001 * 编码器脉冲数 /10000 <= Numerator/Denominator <=4000 * 编码器脉冲数 /10000 */
    	if (Num1st < Den1st)
    	{
            Temp111 = (Uint64)Den1st * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
            Temp222 = (Uint64)Num1st * (Uint64)1000L *(Uint64)10000L;
            if(Temp111>Temp222)PostErrMsg(GEALSETERR0);
    	}
    	else if (Num1st > Den1st)
    	{
	        Temp111 = (Uint64)Num1st * (Uint64)10L;
	        Temp222 = (Uint64)4L * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * (Uint64)Den1st;
	        if(Temp111>Temp222)PostErrMsg(GEALSETERR0);
    	}		
	}
    
    STR_InnerGvarPosCtrl.Numerator   = p->Numerator;
    STR_InnerGvarPosCtrl.Denominator = p->Denominator;

    STR_FUNC_Gvar.PosCtrl.Denominator = p->Denominator; 
    STR_FUNC_Gvar.PosCtrl.Numerator   = p->Numerator;  
}


/*******************************************************************************
  函数名: void EleGearStopUpdata(STR_ELECTRONIC_GEAR *p) 
  输入:   电子齿轮功能码 
  输出:   电子齿轮分子分母子以及电子齿轮切换锁存
  子函数: 无       
  描述: 电子齿轮停机初始化，该函数在伺服OFF和上电复位初始化时调用
********************************************************************************/ 
void EleGearRatioErrCheck(void)
{
    Uint32 Num1st = 0;          //电子齿轮分子暂存变量
    Uint32 Den1st = 0;          //电子齿轮分母暂存变量

    Uint32 Num2nd = 0;          //电子齿轮分子暂存变量
    Uint32 Den2nd = 0;          //电子齿轮分母暂存变量

    Uint64 Temp111 = 0;
    Uint64 Temp222 = 0;
    
    if(PulsePRev==0)
	{
    	Num1st = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCmxHigh,FunCodeUnion.code.PL_PosFirCmxLow); //第1组电子齿轮分子根据功能码(H0507~H0508)赋值

    	Den1st = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCdvHigh,FunCodeUnion.code.PL_PosFirCdvLow); //第1组电子齿轮分母根据功能码(H0507~H0508)赋值
    
    	/*第一组电子齿轮比范围判断，规格范围0.001 * 编码器脉冲数 /10000 <= Numerator/Denominator <=4000 * 编码器脉冲数 /10000 */
    	if (Num1st < Den1st)
    	{
            Temp111 = (Uint64)Den1st * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
            Temp222 = (Uint64)Num1st * (Uint64)1000L *(Uint64)10000L;
            if(Temp111>Temp222)PostErrMsg(GEALSETERR1);
			
    	}
    	else if (Num1st > Den1st)
    	{
	        Temp111 = (Uint64)Num1st * (Uint64)10L;
	        Temp222 = (Uint64)4L * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * (Uint64)Den1st;
	        if(Temp111>Temp222)PostErrMsg(GEALSETERR1);
    	}
    	
		Num2nd = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCmxHigh,FunCodeUnion.code.PL_PosSecCmxLow); //第2组电子齿轮分子根据功能码(H0509~H010)赋值
       
    	Den2nd = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCdvHigh,FunCodeUnion.code.PL_PosSecCdvLow); //第2组电子齿轮分母根据功能码(H0509~H010)赋值
            
    	/*第二组电子齿轮比范围判断，规格范围0.001 * 编码器脉冲数 /10000<= Numerator/Denominator <=4000 * 编码器脉冲数 /10000 */
   		if(Num2nd < Den2nd)
    	{
            Temp111 = (Uint64)Den2nd * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
            Temp222 = (Uint64)Num2nd * (Uint64)1000L *(Uint64)10000L;
            if(Temp111>Temp222)PostErrMsg(GEALSETERR2);
    	}
    	else if(Num2nd > Den2nd)
    	{
	        Temp111 = (Uint64)Num2nd * (Uint64)10L;
	        Temp222 = (Uint64)4L * (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev * (Uint64)Den2nd;
	        if(Temp111>Temp222)PostErrMsg(GEALSETERR2);
    	}
	}    
}


/*******************************************************************************
  函数名: void EleGearGetValue() 
  输入:   电子齿轮切换DI信号，两组电子齿轮功能码值 
  输出:   当前电子齿轮分子分母数值
  子函数:  无       
  描述:  进行电子齿轮切换动作，并相应的更改当前电子齿轮分子分母值 
********************************************************************************/ 
void EleGearGetValue(STR_ELECTRONIC_GEAR *p,int32 CommandInput)
{
    static Uint16 NoPosRefCnt = 0;    //没有位置指令输入时的计数器
    Uint8 Flag = 0;

    p->InPut = CommandInput;          //接收外面输入的位置指令进行切换判断

    if((FunCodeUnion.code.PL_PosGearChangeEn ==0)&&(PulsePRev==0))   //在两组电子齿轮切换不使能时的处理
    {  
        if( (STR_PosCtrl.PosMultipleCmd == 0) 
        &&  ( ((FunCodeUnion.code.FC_FeedbackMode == 2) && (STR_InnerGvarPosCtrl.MutexBit.bit.ExChangeEn == 1))
           || (FunCodeUnion.code.FC_FeedbackMode != 2)) )
        {
            if(NoPosRefCnt <= 10) 
            {
                NoPosRefCnt++;
                Flag = 0;
            }
            else 
            {
                NoPosRefCnt = 0;
                Flag = 1;
            }
        }
        else
        {
            NoPosRefCnt = 0;
            Flag = 0;
        }               
          
        if(Flag == 1)
        {
            NoPosRefCnt = 0;
            //内外环切换
            if(((FunCodeUnion.code.FC_FeedbackMode == 2) && (STR_FUNC_Gvar.DivarRegLw.bit.GearSel == 1)))
            {
                STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag = 1;
            }
            else
            {
                STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag = 0;
            }
           
            if(STR_FUNC_Gvar.DivarRegLw.bit.GearSel == 1)		 //当前电子齿轮分子分母值更新相应的第二组电子齿轮功能码设定值，并在切换时将电子齿轮计算余数清零
            {
                p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCmxHigh,FunCodeUnion.code.PL_PosSecCmxLow);
                
                p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCdvHigh,FunCodeUnion.code.PL_PosSecCdvLow);

                if ( p->GroupSelLatch != STR_FUNC_Gvar.DivarRegLw.bit.GearSel )
                {
                    p->GearRemainder = 0;
                    p->GroupSelLatch = STR_FUNC_Gvar.DivarRegLw.bit.GearSel;
                }
            }
            else        //当前电子齿轮分子分母值更新相应的第一组电子齿轮功能码设定值，并在切换时将电子齿轮计算余数清零
            {
                p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCmxHigh,FunCodeUnion.code.PL_PosFirCmxLow);
			 	
                p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCdvHigh,FunCodeUnion.code.PL_PosFirCdvLow);

                if ( p->GroupSelLatch != STR_FUNC_Gvar.DivarRegLw.bit.GearSel )
                {
                    p->GearRemainder = 0;
                    p->GroupSelLatch = STR_FUNC_Gvar.DivarRegLw.bit.GearSel;
                }	  
            }
        }
    }
    else if ((FunCodeUnion.code.PL_PosGearChangeEn == 1)&&((PulsePRev==0)))	 //在两组电子齿轮切换使能时的处理
    {   
        //内环电子齿轮切换
        STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag = 0;        
      
        if(STR_FUNC_Gvar.DivarRegLw.bit.GearSel == 1)	    //当前电子齿轮分子分母值更新相应的第二组电子齿轮功能码设定值，并在切换时将电子齿轮计算余数清零
        {
            p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCmxHigh,FunCodeUnion.code.PL_PosSecCmxLow);

            p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosSecCdvHigh,FunCodeUnion.code.PL_PosSecCdvLow);

            if ( p->GroupSelLatch != STR_FUNC_Gvar.DivarRegLw.bit.GearSel )
            {
                p->GearRemainder = 0;
                p->GroupSelLatch = STR_FUNC_Gvar.DivarRegLw.bit.GearSel;
            }
        }
        else    //当前电子齿轮分子分母值更新相应的第一组电子齿轮功能码设定值，并在切换时将电子齿轮计算余数清零
        {
            p->Numerator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCmxHigh,FunCodeUnion.code.PL_PosFirCmxLow);

            p->Denominator = A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_PosFirCdvHigh,FunCodeUnion.code.PL_PosFirCdvLow);

            if ( p->GroupSelLatch != STR_FUNC_Gvar.DivarRegLw.bit.GearSel )
            {
                p->GearRemainder = 0;
                p->GroupSelLatch = STR_FUNC_Gvar.DivarRegLw.bit.GearSel;
            }
        }
	}

    STR_InnerGvarPosCtrl.Numerator   = p->Numerator;
    STR_InnerGvarPosCtrl.Denominator = p->Denominator;

	STR_FUNC_Gvar.PosCtrl.Denominator = STR_InnerGvarPosCtrl.Denominator; 
    STR_FUNC_Gvar.PosCtrl.Numerator   = STR_InnerGvarPosCtrl.Numerator; 
}

/*******************************************************************************
  函数名:  EleGearRatioCalc(STR_ELECTRONIC_GEAR *p)
  输入:    STR_EleGear.Numerator	          //当前电子齿轮分子
           STR_EleGear.Denominator	          //当前电子齿轮分母
		   STR_EleGear.InPut          //位置指令输入
  参数：   STR_EleGear.GearRemainder  //电子齿轮余数    
  输出:    STR_PosCtrl.Ref        
  描述:    对位置指令进行电子齿轮比计算          
********************************************************************************/ 
int32 EleGearRatioCalc(STR_ELECTRONIC_GEAR *p,int32 CommandInput)
{
    int64  Sum = 0; //齿轮计算中间变量 
    int32  Result = 0;     
    int64  Temp = 0;      //必须为64位，要不然容易溢出

    if (p->Denominator == 0)	  //当电子齿轮分母为零时退出该程序，返回的位置指令为零
    {
        return 0;
    }

    p->InPut = CommandInput;    //电子齿轮计算中接收输入指令

    /*计算放大后的位置指令加上上次计算的余数*/	
    Sum = ((int64)p->InPut) * ((int64)p->Numerator); 
    Sum += (int64)p->GearRemainder;  
    
    /*计算乘以电子齿轮比后的位置指令*/  
    Result = (int32) (Sum / (int64)p->Denominator);

    /*计算电子齿轮余数*/         
    Temp = (int64)Result * (int64)p->Denominator; 
    p->GearRemainder = Sum - Temp;     

    return ((int32)Result);    
}

/********************************* END OF FILE *********************************/

