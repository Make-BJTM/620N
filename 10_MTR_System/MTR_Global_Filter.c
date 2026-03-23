/*****************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Global_Filter.h  
 创建人：   梅仕峰                 创建日期：2010.08.31                     
 修改人：   王治国、熊飞、朱祥华   修改日期：2012.02.10
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.      
    2. 	   
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "MTR_Global_Filter.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"

#include "PUB_Table.h"
/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */
#define   DEPTHLVL   335544     //Q10格式的数：0.01×32768
/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//陷波器宽度等级，共分20级，和松下保持一致
const Uint16 WidthCoff_Q10[] = {512,604,727,860,1024,1218,1443,1720,2048,2437,
                                2897,3440,4096,4874,5795,6891,8192,9738,11581,13772,16384};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void MTR_NewNotchFilterInit(STR_MTR_NotchFilter *p, Uint16 Type);
void MTR_NotchFilter(STR_MTR_NotchFilter *p);

//姚虹新设计的低通滤波器
void MTR_InitNewLowPassFilt(STR_MTR_NEW_LOWPASS_FILTER *p);
void MTR_NewLowPassFiltCalc(STR_MTR_NEW_LOWPASS_FILTER *p);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 



/*******************************************************************************
  函数名:   void MTR_InitNewLowPassFilt_(str_LpFilt *p)  姚虹设计的新的低通滤波器
  输入:   Fs:采样频率 
  输出:   Fc:截止频率 
  子函数: 无
    1.  
    2.
********************************************************************************/
void MTR_InitNewLowPassFilt(STR_MTR_NEW_LOWPASS_FILTER *p)
{
    Uint32 TanTheta_Q17;
    Uint32 TanTheta;       //角度的索引值
    Uint16 SinTheta;
    Uint32 ThetaTmp;       //用于插值计算时的索引值差值
    int32  SinValueTmp;
    int16  SinValue;
    int32  TanValue;       //计算出的正切值   
    Uint16 CosTheta;
    int32  CosValueTmp;    //插值计算后的余弦值差值
    int16  CosValue;      //计算出的余弦值
    int32  Temp;          //计算中间值

    TanTheta_Q17 = ((int64)p->Fc << 24) / p->Fs;
    TanTheta    = (TanTheta_Q17 >> 17);
    ThetaTmp    = TanTheta_Q17 - (TanTheta << 17);
    SinTheta    = TanTheta;
    CosTheta    = SinTheta + 64;

    if(CosTheta > 255)
    {
        CosTheta -= 256;
    }

    SinValueTmp  = (int32)(SinTable[SinTheta + 1] - SinTable[SinTheta]) * ThetaTmp;
    SinValue     = SinTable[SinTheta] + (int16)(SinValueTmp >> 17);

    CosValueTmp  = (int32)(SinTable[CosTheta + 1] - SinTable[CosTheta]) * ThetaTmp;
    CosValue     = SinTable[CosTheta] + (int16)(CosValueTmp >> 17);

    if(CosValue != 0)
    {
        TanValue = ((int32)SinValue << 15) / CosValue;                 //得到正切值
        Temp     = 32768;
    }
    else             //除数为0时，滤波器不起作用
    {
        TanValue = 0;
        Temp     = -32768;
    }

    Temp  = (((int64)(Temp - TanValue)) << 16) / (Temp + TanValue);

    if((p->Fc<<1) < p->Fs)      //只有在截止频率比采样频率小一半时才起作用
    {
        p->Num[0] = (65536 - Temp) >> 1;          //得到一阶和二阶滤波器系数
        //p->Num[1] = (65536 - Temp) >> 1;
        p->Num[1] = (65536 - Temp) - p->Num[0];
        p->Den[0] = 65536;
        p->Den[1] = -Temp;
    }
    else
    {
        p->Num[0] = 65536;
        p->Num[1] = 0;
        p->Den[0] = 65536;
        p->Den[1] = 0;
    }
}


/*******************************************************************************
  函数名:   void MTR_NewLowPassFilt_(str_LpFilt *p)  姚虹设计的新的低通滤波器
  输入:   P->InPut：当前调度周期的输入值
          P->InPut0：上个调度周期的输入值
          p->Output0:上周期输出值
  输出:   P->OutPut:  当前调度周期的输出值
  子函数: 无
    1.  
    2.
********************************************************************************/
void MTR_NewLowPassFiltCalc(STR_MTR_NEW_LOWPASS_FILTER *p)
{
    int64 Temp;

    if((p->Fc<<1) >= p->Fs)
    {
        p->Output = p->Input;
    }
    else
    {
        //Temp = ((int64)p->Num[0] * p->Input) + ((int64)p->Num[1] * p->InputLast) - ((int64)p->Den[1] * p->OutputLast) ; 
        Temp = ((int64)p->Num[0] * p->Input) + ((int64)p->Num[1] * p->InputLast) - ((int64)p->Den[1] * p->OutputLast) + p->Rem; 

        //负数不能直接移位做除法，一旦不能除尽，则会出现移位后的绝对值比实际大1的情况
        if(Temp < 0)
        {
            p->Output = (int32)(-((-Temp) >> 16));
        }
        else
        {
            p->Output = (int32)(Temp >> 16);
        }
        p->Rem    =  Temp - ((int64)p->Output<<16);

        p->OutputLast = p->Output;           //保存本周期的输出
        p->InputLast  = p->Input;            //保存本周期的输入
    }
}


/*******************************************************************************
  函数名: void NotchFilterInit(STR_NotchFilter *p)
  输入:   SampFreq: 采样频率
          NotchFreq：陷波器中心频率
          DeltaFreq_3dB：陷波器3dB衰减时的阻带频率差值，差值越大，则阻带增益越大！
          Type：  1～陷波器A，2～陷波器B，3～陷波器C   
  输出:   无 
  描述:   计算二阶陷波器的分子和分母系数！
  实现方法: 可以直接使用正弦表来计算正切值，也可以建立一个正切表来进行查表计算！
            这里采用正弦表进行计算，正切表及其查表算法暂时屏蔽！
********************************************************************************/
/*void MTR_NotchFilterInit(STR_MTR_NotchFilter *p, Uint16 Type)
{
	Uint32 TanTheta_Q7;
	Uint16 TanTheta; 	   //角度的索引值
	Uint16 SinTheta;
	Uint16 ThetaTmp;	   //用于插值计算时的索引值差值
	int32  SinValueTmp;
	int16  SinValue;
	int32  TanValue;	   //计算出的正切值
	Uint32 CosTheta_Q7;   
	Uint16 CosTheta;
	int32  CosValueTmp;    //插值计算后的余弦值差值
	int16  CosValue;       //计算出的余弦值
	int32  Temp1; 		   //用于计算的中间变量
	int32  Temp2;
	int32  Temp3;
	int32  Temp4;

	switch(Type)    //根据类型选择是哪个陷波器
	{

	    case MTR_NOTCHFILTERA:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqA;
		    p->DeltaFreq_3dB = FunCodeUnion.code.AT_NotchFiltBandWidthA;
		  break;
	    case MTR_NOTCHFILTERB:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqB;
		    p->DeltaFreq_3dB = FunCodeUnion.code.AT_NotchFiltBandWidthB;
		 break;

	    case MTR_NOTCHFILTERC:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqC;
		    p->DeltaFreq_3dB = FunCodeUnion.code.AT_NotchFiltBandWidthC;
		 break;	
	    case MTR_NOTCHFILTERD:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqD;
		    p->DeltaFreq_3dB = FunCodeUnion.code.AT_NotchFiltBandWidthD;
		 break;

	    case MTR_NOTCHFILTERAA:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.TL_NotchFiltFreqAA;
		    p->DeltaFreq_3dB = FunCodeUnion.code.TL_NotchFiltBandWidthAA;
		  break;
	    case MTR_NOTCHFILTERBB:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.TL_NotchFiltFreqBB;
		    p->DeltaFreq_3dB = FunCodeUnion.code.TL_NotchFiltBandWidthBB;
		 break;
	 default:
	     break;     	     
	}

    // 以下计算Tan数值 
    TanTheta_Q7 = ((Uint32)p->DeltaFreq_3dB << 14) / p->SampFreq;
	TanTheta    = (Uint16)(TanTheta_Q7 >> 7);
	ThetaTmp    = TanTheta_Q7 - (TanTheta <<7);
	SinTheta    = TanTheta;
	CosTheta    = SinTheta + 64;

    if(CosTheta > 255)
	{
		CosTheta -= 256;
	}

    SinValueTmp  = (int32)(SinTable[SinTheta + 1] - SinTable[SinTheta]) * ThetaTmp;
    SinValue     = SinTable[SinTheta] + (int16)(SinValueTmp >> 7);

    CosValueTmp  = (int32)(SinTable[CosTheta + 1] - SinTable[CosTheta]) * ThetaTmp;
    CosValue     = SinTable[CosTheta] + (int16)(CosValueTmp >> 7);

    TanValue     = ((int32)SinValue <<15) / CosValue; 
     
    // 以下计算Cos数值 
	CosTheta_Q7 = ((Uint32)p->NotchFreq << 15) / p->SampFreq;   	//利用插值运算计算余弦值
	CosTheta    = (Uint16)(CosTheta_Q7 >> 7);
	ThetaTmp    = CosTheta_Q7 - (CosTheta << 7);
	CosTheta    = CosTheta + 64;

	if(CosTheta > 255)
	{
	    CosTheta -= 256;
	}

	CosValueTmp = (int32)(SinTable[CosTheta + 1] - SinTable[CosTheta]) * ThetaTmp;
	CosValue    = SinTable[CosTheta] + (int16)(CosValueTmp >> 7);

    // 以下计算系数 
	Temp1 = ((32768L - TanValue) << 15) / (32768L + TanValue);		   //分子分母系数计算
	Temp2 = CosValue;
	Temp3 = (32768L + Temp1) >> 1;
	Temp4 = (2 * Temp2 * Temp3) >> 15; 

    // 陷波器传函分子分母相对应系数赋值 
	p->NumCoeff[0] = Temp3;
	p->NumCoeff[1] = -Temp4;
	p->NumCoeff[2] = Temp3; 
	p->DenCoeff[0] = 32768;
	p->DenCoeff[1] = -Temp4;
	p->DenCoeff[2] = Temp1;

    // 初始设置输入输出参数为零, 避免修改陷波宽度后影响计算值 
    p->NotchInput[0]  = 0;
	p->NotchInput[1]  = 0;
	p->NotchInput[2]  = 0;
	p->NotchOutput[0] = 0;
	p->NotchOutput[1] = 0;
	p->NotchOutput[2] = 0;
}
*/
/*******************************************************************************
  函数名: void NewNotchFilterInit(STR_NotchFilter *p)
  输入:   SampFreq: 采样频率
          NotchFreq：陷波器中心频率
          DeltaFreq_3dB：陷波器3dB衰减时的阻带频率差值，差值越大，则阻带增益越大！
          NotchDepth:陷波器深度
          Type：  1～陷波器A，2～陷波器B，3～陷波器C   
  输出:   无 
  描述:   计算二阶陷波器的分子和分母系数！
  实现方法: 可以直接使用正弦表来计算正切值，也可以建立一个正切表来进行查表计算！
            这里采用正弦表进行计算，正切表及其查表算法暂时屏蔽！
********************************************************************************/
void MTR_NewNotchFilterInit(STR_MTR_NotchFilter *p, Uint16 Type)
{
	Uint32 TanTheta_Q20;
	Uint32 TanTheta_Q8; 	   //角度的索引值
	Uint16 SinTheta;
	Uint32 ThetaTmp;	   //用于插值计算时的索引值差值
	int32  SinValueTmp;
	int16  SinValue;
	int32  TanValue;	   //计算出的正切值
	Uint16 CosTheta;
	int32  CosValueTmp;    //插值计算后的余弦值差值
	int16  CosValue;       //计算出的余弦值
	int32  Temp1; 		   //用于计算的中间变量
	int32  Temp2;
	int32  Temp3;
	int32  Temp4;

	switch(Type)    //根据类型选择是哪个陷波器
	{
	    case MTR_NOTCHFILTERA:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqA;
		    p->DeltaFreq_3dB = ((Uint32)WidthCoff_Q10[FunCodeUnion.code.AT_NotchFiltBandWidthA] * p->NotchFreq)>>10;
            p->NotchDepth = ((Uint32)FunCodeUnion.code.AT_NotchFiltAttenuatLvlA * DEPTHLVL) >> 10;
		  break;
	    case MTR_NOTCHFILTERB:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqB;
		    p->DeltaFreq_3dB = ((Uint32)WidthCoff_Q10[FunCodeUnion.code.AT_NotchFiltBandWidthB] * p->NotchFreq)>>10;
            p->NotchDepth = ((Uint32)FunCodeUnion.code.AT_NotchFiltAttenuatLvlB * DEPTHLVL) >> 10;
		 break;
	    case MTR_NOTCHFILTERC:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqC;
		    p->DeltaFreq_3dB = ((Uint32)WidthCoff_Q10[FunCodeUnion.code.AT_NotchFiltBandWidthC] * p->NotchFreq)>>10;
            p->NotchDepth = ((Uint32)FunCodeUnion.code.AT_NotchFiltAttenuatLvlC * DEPTHLVL) >> 10;
		 break;	
	    case MTR_NOTCHFILTERD:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.AT_NotchFiltFreqD;
		    p->DeltaFreq_3dB = ((Uint32)WidthCoff_Q10[FunCodeUnion.code.AT_NotchFiltBandWidthD] * p->NotchFreq)>>10;
            p->NotchDepth = ((Uint32)FunCodeUnion.code.AT_NotchFiltAttenuatLvlD * DEPTHLVL) >> 10;
		 break;
/*
	    case MTR_NOTCHFILTERAA:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.TL_NotchFiltFreqAA;
		    p->DeltaFreq_3dB = FunCodeUnion.code.TL_NotchFiltBandWidthAA;
            p->NotchDepth = 0;         //老陷波器深度无穷大
		  break;
	    case MTR_NOTCHFILTERBB:
	        p->SampFreq  = STR_MTR_Gvar.System.SpdFreq;
		    p->NotchFreq = FunCodeUnion.code.TL_NotchFiltFreqBB;
		    p->DeltaFreq_3dB = FunCodeUnion.code.TL_NotchFiltBandWidthBB;
            p->NotchDepth = 0;         //老陷波器深度无穷大
		 break;
*/
	 default:
	     break;     	     
	}

    /*计算中心频率处衰减对应的系数*/
    Temp1 = p->NotchDepth;


    /* 以下计算中心频率处的正切值 */
    TanTheta_Q20 = ((Uint32)p->NotchFreq << 19) / p->SampFreq;
	TanTheta_Q8  = (TanTheta_Q20 >> 12);
	ThetaTmp     = TanTheta_Q20 - (TanTheta_Q8 << 12);
	SinTheta     = TanTheta_Q8;
	CosTheta     = SinTheta + 64;

    if(CosTheta > 255)
	{
		CosTheta -= 256;
	}

    SinValueTmp  = (int32)(SinTable[SinTheta + 1] - SinTable[SinTheta]) * ThetaTmp;
    SinValue     = SinTable[SinTheta] + (SinValueTmp >> 12);

    CosValueTmp  = (int32)(SinTable[CosTheta + 1] - SinTable[CosTheta]) * ThetaTmp;
    CosValue     = SinTable[CosTheta] + (CosValueTmp >> 12);

    if(CosValue != 0)
    {
        TanValue = ((int32)SinValue <<15) / CosValue;
    }
    else
    {
        TanValue = 32768;
    }

    Temp2     = TanValue;

    /*以下计算宽度对应的正切值  */  
    TanTheta_Q20 = ((Uint32)p->DeltaFreq_3dB << 19) / p->SampFreq;
	TanTheta_Q8  = (TanTheta_Q20 >> 12);
	ThetaTmp    = TanTheta_Q20 - (TanTheta_Q8 << 12);
	SinTheta    = TanTheta_Q8;
	CosTheta    = SinTheta + 64;

    if(CosTheta > 255)
	{
		CosTheta -= 256;
	}

    SinValueTmp  = (int32)(SinTable[SinTheta + 1] - SinTable[SinTheta]) * ThetaTmp;
    SinValue     = SinTable[SinTheta] + (SinValueTmp >> 12);

    CosValueTmp  = (int32)(SinTable[CosTheta + 1] - SinTable[CosTheta]) * ThetaTmp;
    CosValue     = SinTable[CosTheta] + (CosValueTmp >> 12);

    if(CosValue != 0)
    {
        TanValue = ((int32)SinValue <<15) / CosValue;
    }
    else
    {
        TanValue = 32768;
    }
    
    Temp3 = TanValue;

//    Temp4 = (((int64)Temp2 * Temp2 * 4) - ((int64)Temp3 * Temp3)) >> 17;
    Temp4 = ((int64)Temp2 * Temp2) >> 15;

    /* 陷波器传函分子分母相对应系数赋值 */
	p->NumCoeff[0] = 32768 + (((int64)Temp1 * Temp3)>>15) + Temp4;
	p->NumCoeff[1] = -(2 * 32768) + (2 * Temp4);
	p->NumCoeff[2] = 32768 - (((int64)Temp1 * Temp3)>>15) + Temp4; 
	p->DenCoeff[0] = 32768 + Temp3 + Temp4;
	p->DenCoeff[1] = -(2*32768) + (2 * Temp4);
	p->DenCoeff[2] = 32768 - Temp3 + Temp4;
    p->DenDivCoeff_Q30 = (1L<<30) / p->DenCoeff[0];      //将除法转换为乘法时的倒数
    p->DenDivCoeffRmn_Q30 = (1L<<30) - (p->DenDivCoeff_Q30 * p->DenCoeff[0]);


    /* 初始设置为陷波器当前周期输入, 避免修改陷波宽度后出现跳变 */
    p->NotchInput[0]  = p->NotchInput[0];
	p->NotchInput[1]  = p->NotchInput[0];
	p->NotchInput[2]  = p->NotchInput[0];
	p->NotchOutput[0] = p->NotchInput[0];
	p->NotchOutput[1] = p->NotchInput[0];
	p->NotchOutput[2] = p->NotchInput[0];
}


/*******************************************************************************
  函数名: void NotchFilter(STR_NotchFilter *p)
  输入:   p->NotchIutput[0]:当前调度周期的输入值 
  输出:   p->NotchOutput[0]:当前调度周期的输出值 
  子函数: 无
  描述：  根据传函进行陷波器输出计算
********************************************************************************/
void MTR_NotchFilter(STR_MTR_NotchFilter *p)
{
    int64 Temp;
	int64 Temp1;

    Temp =  (int64)p->NumCoeff[0] * p->NotchInput[0];
    Temp += (int64)p->NumCoeff[1] * p->NotchInput[1];
    Temp += (int64)p->NumCoeff[2] * p->NotchInput[2];

    //减去上周期的输出时应包含对输出截尾的处理
    //Temp -= (p->DenCoeff[1] * p->NotchOutput[1]);
    //Temp -= (p->DenCoeff[2] * p->NotchOutput[2]);
    Temp -= (((int64)p->DenCoeff[1] * p->NotchOutput[1]) + (((int64)p->NotchRmn[0] * p->DenCoeff[1] * p->DenDivCoeff_Q30)>>30));
    Temp -= (((int64)p->DenCoeff[2] * p->NotchOutput[2]) + (((int64)p->NotchRmn[1] * p->DenCoeff[2] * p->DenDivCoeff_Q30)>>30));
    //Temp -= ((p->DenCoeff[1] * p->NotchOutput[1]) + (((int64)p->NotchRmn[0] * p->DenCoeff[1]) / p->DenCoeff[0]));
    //Temp -= ((p->DenCoeff[2] * p->NotchOutput[2]) + (((int64)p->NotchRmn[1] * p->DenCoeff[2]) / p->DenCoeff[0]));

    //p->NotchOutput[0] = (Temp / p->DenCoeff[0]);
    
    //加入将除法变为乘法后产生的截尾
	Temp1 = (Temp * p->DenDivCoeff_Q30) + ((Temp * p->DenDivCoeffRmn_Q30 * p->DenDivCoeff_Q30)>>30);
	if(Temp1 > 0)
	{
		p->NotchOutput[0] = (Temp1 + ((int32)1<<29)) >> 30;
	}
	else
	{
	    p->NotchOutput[0] = -((-Temp1 + ((int32)1<<29)) >> 30);
	}

    //计算本周期输出的截尾
    p->NotchRmn[1]    = p->NotchRmn[0];
    p->NotchRmn[0]    = Temp - ((int64)p->NotchOutput[0] * p->DenCoeff[0]);
    	
    //保存输入输出值
    p->NotchOutput[2] = p->NotchOutput[1];
	p->NotchOutput[1] = p->NotchOutput[0];

	p->NotchInput[2] = p->NotchInput[1];
	p->NotchInput[1] = p->NotchInput[0];
}

/********************************* END OF FILE *********************************/
