/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_InerFricIden.c  
 创建人：   姚虹                创建日期：2014.6.24                     
 修改人：                      
 描述： 
    1. 采集加减速数据并利用最小二乘算法计算惯量比和摩擦力
    2. 
 修改记录：  
    
    1.      
    2. 	   
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "MTR_InerFricIden.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"
#include "FUNC_FunCode.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */
#define MAXLENSAMP  64       //最大允许的数据采样长度
#define MINLENSAMP  4        //采样数据至少达到的长度

#define SAMPFLTCNT       16      //速度和转矩滑动平均滤波次数
#define SHIFTRIGHT       4       //平均滤波的右移位数2^SHIFTRIGHT = SAMPFLTCNT


/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_INFRID     STR_InFricId = {0};

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

int32 SpdSamp[SAMPFLTCNT];       //用于滑动平均的速度采样值和转矩采样值
int16 TeSamp[SAMPFLTCNT];


static int32 SpdAcc= 0;
static int16 TeAcc = 0;
static int32 SpdDec = 0;
static int16 TeDec = 0;

void FricProcess(void);
void FricRest(void);
void FricUpdate(void);

Static_Inline void FricSpdTeFlt(void);
Static_Inline void FricSamp(void);
Static_Inline void FricSamp_Acc(void);	 //加速时数据采集
Static_Inline void FricSamp_Dec(void);	 //减速时数据采集
Static_Inline void FricAndJCal(void);
 
/*******************************************************************************
  函数名:  void FricProcess(void)
  输入:    
  子函数:          
  描述: 惯量及摩擦力辨识过程
********************************************************************************/
void FricProcess(void)
{
    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == 2)
    {
        FricSpdTeFlt();      //速度反馈和转矩指令滑动平均滤波
        
        if(STR_InFricId.StartCalFlag == 0)    //当前处于采样状态
        {     
			FricSamp();				    //采样过程 
        }
        else             //当前处于计算状态
        {
            FricAndJCal();
        }        
    }
}


/*******************************************************************************
  函数名:  void FricUpdate(void)
  输入:    
  子函数:          
  描述: 惯量及摩擦力辨识参数在线更新，设置阈值和采样时间点
        Te = (K * Coff * Dspd) + Td;   K为惯量比，Td为外部负载转矩
	   Coff = (4096 * fs * 2pi * Jm) / (Tn * 10000 * 60)

	   根据这个公式，多次采样Te和Dspd，就能形成多组加速和减速的数据，
	   就可以利用直线逼近方式求出K和Td
********************************************************************************/
void FricUpdate(void)
{
	if(FunCodeUnion.code.InertiaIdyCountModeSel == 0)     //非辨识状态下清除辨识结果
    {
	    AuxFunCodeUnion.code.DP_JRatioOnline = 0;   //辨识结果复位
	    AuxFunCodeUnion.code.DP_TLOnline = 0;        
	}

	if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN) //非伺服运行状态
	{
	    FricRest();
	}    
}

//惯量和摩擦力辨识时需要进行的参数复位
void FricRest(void)
{
    STR_InFricId.NewTe  = 0;
    STR_InFricId.NewSpd = 0;
    STR_InFricId.OldSpd = 0;
    STR_InFricId.j      = 0;
    STR_InFricId.m      = 0;
	STR_InFricId.SmpCnt = 0;
    STR_InFricId.SpdSum = 0;
    STR_InFricId.SampState = 0;
    STR_InFricId.AccPosSum = 0;
    STR_InFricId.TePosSum  = 0;
    STR_InFricId.AccNegSum = 0;
    STR_InFricId.TeNegSum  = 0;
    STR_InFricId.StartCalFlag = 0;
    STR_InFricId.FiltIndex  = 0;
    STR_InFricId.RatioSum   = 0;
    STR_InFricId.i  = 0;
    while(STR_InFricId.i < 8)
    {
        STR_InFricId.FiltRatio[STR_InFricId.i] = 0;
        STR_InFricId.i++;
    }
    STR_InFricId.i = 0;
    while(STR_InFricId.i < SAMPFLTCNT)
    {
        SpdSamp[STR_InFricId.i] = 0;
        TeSamp[STR_InFricId.i]  = 0;
        STR_InFricId.i++;
    }
    STR_InFricId.i = 0;
	AuxFunCodeUnion.code.DP_JRatioOnline = 0;   //辨识结果复位
	AuxFunCodeUnion.code.DP_TLOnline = 0;    
}


//惯量和摩擦力计算过程
Static_Inline void FricAndJCal(void)
{
    static int32 AccPosFlt;
    static int32 AccNegFlt;
    static int32 TePosFlt;
    static int32 TeNegFlt;
    static int16 Step = 0;

    int32 Temp1;
    int64 Temp2;

    switch(Step)   //由于每步中都含有除法，将计算步骤分开
    {
        case 0:            //加速速阶段平均值
            AuxFunCodeUnion.code.H2F_Rsvd43++;	 //惯量辨识次数累加

            AccPosFlt = STR_InFricId.AccPosSum / STR_InFricId.j;
            TePosFlt  = STR_InFricId.TePosSum  / STR_InFricId.j;
			 
            Step      = 1;
        break;

        case 1:            //减速阶段平均值
            AccNegFlt = STR_InFricId.AccNegSum / STR_InFricId.m;
            TeNegFlt  = STR_InFricId.TeNegSum  / STR_InFricId.m;

            //避免加速时的力矩绝对值小于减速时的力矩，导致计算出的惯量比为负，需要清除相关变量后重新采集计算
			if(ABS(TePosFlt) < ABS(TeNegFlt))	  
			{
			     Step = 5;
			}
			else
			{
			     Step = 2;
			}
			
        break;

        case 2:  //求惯量比	(JL+JM)/Jm，对旋转电机，系数Temp2中10^6指惯量的小数，直线电机还包括mm/s转化为m/s的小数
            Temp2 = (int64)(TePosFlt - TeNegFlt) * 100 * 1000000L;
            //根据斜率求取惯量比
            STR_InFricId.JRatio  = (int32)(Temp2 / ((int64)(AccPosFlt - AccNegFlt) * STR_MTR_Gvar.InFricId.Coff_J));
            Step                 = 3;

        break;

        case 3:
            //求取过零点摩擦力，正反方向摩擦力需要单独保存
            Temp1 = (TePosFlt - TeNegFlt) * ABS(AccNegFlt) / (ABS(AccPosFlt) + ABS(AccNegFlt));

            if(Temp1 > 0)
            {
                STR_InFricId.PosFric = Temp1;

            }
            else
            {
                STR_InFricId.NegFric = Temp1;
            }

            AuxFunCodeUnion.code.DP_JRatioOnline = (Uint16)(int16)STR_InFricId.JRatio;
            AuxFunCodeUnion.code.DP_TLOnline = (Uint16)(int16)((Temp1 * STR_MTR_Gvar.GetIqRef.ToqDigToPer_Q12)>>12);

            Step = 4;
        break;

        case 4:   //惯量比及摩擦力更新
            
            if(AuxFunCodeUnion.code.H2F_Rsvd33 == 0)     //非自调整模式下更新惯量比
            {
                STR_InFricId.RatioSum -= STR_InFricId.FiltRatio[STR_InFricId.FiltIndex];
                STR_InFricId.FiltRatio[STR_InFricId.FiltIndex] = STR_InFricId.JRatio;
                STR_InFricId.RatioSum += STR_InFricId.FiltRatio[STR_InFricId.FiltIndex];
                STR_InFricId.FiltIndex++;

                FunCodeUnion.code.GN_Rsvd16 = STR_InFricId.RatioSum>>3;     //8次滑动平均
				if(FunCodeUnion.code.GN_Rsvd16 > 100)
				{
				    FunCodeUnion.code.GN_Rsvd16 -= 100;
				}

                if(STR_InFricId.FiltIndex == 8)
                {
                    STR_InFricId.FiltIndex = 0;
                }
            }   
        	
            //计算重力负载和摩擦力
            if(STR_InFricId.NegFric != 0)
            {
			    //求重力负载，H2F40，换算成电流百分比
		        Temp1 = ABS((STR_InFricId.PosFric + STR_InFricId.NegFric)>>1);			
				AuxFunCodeUnion.code.H2F_Rsvd40 = (Uint32)((STR_MTR_Gvar.GetIqRef.ToqDigToPer_Q12 * Temp1) >> 12);
			    //求正反向摩擦力，H2F41,H2F42，换算成电流百分比
				Temp1 = (STR_InFricId.PosFric - STR_InFricId.NegFric)>>1;
			    AuxFunCodeUnion.code.H2F_Rsvd41 = (Uint32)((STR_MTR_Gvar.GetIqRef.ToqDigToPer_Q12 * Temp1) >> 12);
			    AuxFunCodeUnion.code.H2F_Rsvd42 = AuxFunCodeUnion.code.H2F_Rsvd41;
                
                STR_InFricId.NegFric = 0;    //参数复位                
            }
			Step = 5;

		break;			  

		case 5:		//参数复位准备下一次辨识
            
            STR_InFricId.i = 0;
            STR_InFricId.j = 0;                  //计数值置零
            STR_InFricId.m = 0;
			STR_InFricId.SmpCnt    = 0;
            STR_InFricId.AccPosSum = 0;
            STR_InFricId.TePosSum  = 0;
            STR_InFricId.AccNegSum = 0;
            STR_InFricId.TeNegSum  = 0;
            STR_InFricId.StartCalFlag = 0;       //允许重复采样计算
            STR_InFricId.OldSpd = STR_InFricId.NewSpd; //保存本周期的滤波速度值

            Step = 0;
        break;

        default:
        break;
    }
}

//使能状态下速度反馈及转矩指令滑动平均处理
Static_Inline void FricSpdTeFlt(void)
{
    //速度滑动平均
    STR_InFricId.SpdSum    -= SpdSamp[STR_InFricId.i];
    SpdSamp[STR_InFricId.i] = STR_MTR_Gvar.FPGA.M_SpdFdb;
    STR_InFricId.SpdSum    += SpdSamp[STR_InFricId.i];

    //转矩滑动平均
    STR_InFricId.TeSum     -= TeSamp[STR_InFricId.i];
    TeSamp[STR_InFricId.i]  = STR_MTR_Gvar.GetIqRef.IqRef;
    STR_InFricId.TeSum     += TeSamp[STR_InFricId.i];

    STR_InFricId.i++;
	STR_InFricId.SmpCnt++;        //速度采样次数，用于计算加速度

    STR_InFricId.NewSpd = STR_InFricId.SpdSum>>SHIFTRIGHT;
    STR_InFricId.NewTe  = STR_InFricId.TeSum>>SHIFTRIGHT;
	
	if(STR_InFricId.i == SAMPFLTCNT)
    {
        STR_InFricId.i      = 0; 
    }    
}


//数据采样过程
Static_Inline void FricSamp(void)
{	
	if(STR_InFricId.SampState == 0)     //加速时数据采集
	{
		FricSamp_Acc();
	}
	else							   //减速时数据采集
	{		
		FricSamp_Dec();
	}
}


//加速时数据采集
Static_Inline void FricSamp_Acc(void)
{	
	if((ABS(STR_InFricId.NewSpd) > 200000L) && (ABS(STR_InFricId.NewTe) > 41))	//大于20rpm才开始判定加速过程
	{
	    if((ABS(STR_InFricId.NewSpd) - ABS(STR_InFricId.OldSpd)) > 30000)   //阈值大于3rpm才认为有加速度
		{
		    SpdAcc = (STR_InFricId.NewSpd - STR_InFricId.OldSpd) / STR_InFricId.SmpCnt;
			TeAcc  = STR_InFricId.NewTe;

            STR_InFricId.AccPosSum += SpdAcc;     //将结果进行累加便于取平均
            STR_InFricId.TePosSum  += TeAcc;

            STR_InFricId.j++;     //正向采样累加值加1。
			STR_InFricId.SmpCnt = 0;
			STR_InFricId.OldSpd = STR_InFricId.NewSpd;     //等待下一次加速度计算

            if(STR_InFricId.j == MAXLENSAMP)    //采集加速点数足够时采集减速数据
            {                    
                STR_InFricId.TeNegSum  = 0;       //反向采样的转矩和需要清零
                STR_InFricId.SampState = 1;       //不再允许正向采样，需要采集减速数值
            }				
		}
		else
		{
		    if(STR_InFricId.SmpCnt > 32)   //经过32个周期，速度还没有上去，则认为无加速
			{
			    if(STR_InFricId.j >= MINLENSAMP)	 //判断加速点是否足够
				{                		
					STR_InFricId.TeNegSum  = 0;       //反向采样的转矩和需要清零
            		STR_InFricId.SampState = 1;       //不再允许正向采样，需要采集减速数值					
				}
				else			    //加速点不够时重新采集
				{
					STR_InFricId.j          = 0;
            		STR_InFricId.AccPosSum  = 0;
            		STR_InFricId.TePosSum   = 0;					    
				}

				STR_InFricId.SmpCnt	= 0;
				STR_InFricId.OldSpd = STR_InFricId.NewSpd;
			}
		}
	}
	else	  //速度不够或转矩不够，则重新开始采集
	{
		if(STR_InFricId.j >= MINLENSAMP)	 //判断加速点是否足够
		{                		
			STR_InFricId.TeNegSum  = 0;       //反向采样的转矩和需要清零
        	STR_InFricId.SampState = 1;       //不再允许正向采样，需要采集减速数值					
		}
		else			    //加速点不够时重新采集
		{
			STR_InFricId.j          = 0;
        	STR_InFricId.AccPosSum  = 0;
        	STR_InFricId.TePosSum   = 0;					    
		}

		STR_InFricId.SmpCnt = 0;
		STR_InFricId.OldSpd = STR_InFricId.NewSpd;
	}
}

//减速时数据采集
Static_Inline void FricSamp_Dec(void)
{
    //确定方向，需要注意减速时需要和加速时方向保持一致
    if(((STR_InFricId.NewSpd > 100000L)&&(TeAcc>0))
       ||((STR_InFricId.NewSpd < -100000L)&&(TeAcc<0)))
	{
	    if((ABS(STR_InFricId.NewSpd) - ABS(STR_InFricId.OldSpd)) < -30000L)    //开始进行减速
		{
            SpdDec = (STR_InFricId.NewSpd - STR_InFricId.OldSpd) / STR_InFricId.SmpCnt;
            TeDec  = STR_InFricId.NewTe;

            STR_InFricId.AccNegSum += SpdDec;
            STR_InFricId.TeNegSum  += TeDec;

            STR_InFricId.m++;
			STR_InFricId.SmpCnt = 0;
			STR_InFricId.OldSpd = STR_InFricId.NewSpd;

            if(STR_InFricId.m == MAXLENSAMP)
            {
                STR_InFricId.SampState = 0;            //不再允许减速采样，准备计算
                STR_InFricId.StartCalFlag = 1;           //采样完毕，允许计算
            }		
		}
		else
		{
		    if(STR_InFricId.SmpCnt > 32)   //经过32个周期，速度偏差还不够，则认为无减速
			{
			    if(STR_InFricId.m >= MINLENSAMP)	 //判断减速点是否足够
				{                		
                    STR_InFricId.SampState = 0;
                    STR_InFricId.StartCalFlag = 1;       //采样完毕，允许计算					
				}
				else			    //减速点不够时重新采集
				{
                    STR_InFricId.m = 0;
                    STR_InFricId.AccNegSum  = 0;
                    STR_InFricId.TeNegSum   = 0;					    
				}

				STR_InFricId.SmpCnt	= 0;
				STR_InFricId.OldSpd = STR_InFricId.NewSpd;
			}		
		}
	}
	else		    //速度或转矩不满足要求，则重新开始采集
	{
		if(STR_InFricId.m >= MINLENSAMP)	 //判断减速点是否足够
		{                		
            STR_InFricId.SampState = 0;
            STR_InFricId.StartCalFlag = 1;       //采样完毕，允许计算					
		}
		else			    //减速点不够时重新采集
		{
            STR_InFricId.m = 0;
            STR_InFricId.AccNegSum  = 0;
            STR_InFricId.TeNegSum   = 0;					    
		}
		STR_InFricId.SmpCnt = 0;
		STR_InFricId.OldSpd = STR_InFricId.NewSpd;	
	}
}


/********************************* END OF FILE *********************************/
