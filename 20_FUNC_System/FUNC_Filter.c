/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_Filter.c  
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
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
#include "PUB_Table.h"
#include "FUNC_Filter.h"
#include "FUNC_FunCode.h" 
#include "FUNC_PosCtrl.h"
#include "FUNC_GlobalVariable.h"


/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//陷波器宽度等级，共分10级，Q10格式
const Uint16 PosWidthCoff_Q10[] = {512,604,727,860,1024,1218,1443,1720,2048,2048,2048};  //取值范围为[0.5 2];
                                

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void LowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p);
void InitLowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p);

void MidFilter(STR_MidFilter *p);
void AvgFilter(STR_MidFilter *p);

void PosNotchInit(STR_POSNOTCH *p);
void PosNotchFilter(STR_POSNOTCH *p);
void PosNotchFilterClr(STR_POSNOTCH *p);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


//位置指令陷波器函数初始化，双线性变换法进行离散
/*
// biquad滤波器s域传递函数
// s^2 + 2sigma1*omiga1*s + omiga1^2    omiga2^2
//-----------------------------------* -----
// s^2 + 2sigma2*omiga2*s + omiga2^2    omiga1^2

//其中分子omiga1用于确定中心频率，sigma1/sigma2用于确认陷波器深度，即中心频率点输出输入的比值
//sigma2用于确定分母阻尼比，即宽度，omiga2^2/omiga1^2用于确认中心频率以上部分的比值

// 双线性变换公式为
//  s = (2/Ts)*(1-Z^-1)/(1+Z^-1);
//  omiga = (2/Ts)*tan(f*Ts/2);
// 变换后Z域传递函数为

//    num = (a + b*Z^-1 + c*Z^-2)* Coff;
//    den = d + e*Z^-1 + f*Z^-2;

// a = 1 + 2*sigma1*tan(f1*pi/fs) + tan(f1*pi/fs)^2;
// b = -2 + 2*tan(f1*pi/fs)^2;
// c = 1 - 2*sigma1*tan(f1*pi/fs) + tan(f1*pi/fs)^2;
// d = 1 + 2*sigma2*tan(f2*pi/fs) + tan(f2*pi/fs)^2;
// e = -2 + 2*tan(f2*pi/fs)^2;
// f = 1 - 2*sigma2*tan(f2*pi/fs) + tan(f2*pi/fs)^2;

// coff  = tan(f2*pi/fs)^2 / tan(f1*pi/fs)^2;

考虑f1=[1 100], f2=(f1 2f1];正切值最大为tan(200pi/fs)，最小值为tan(pi/fs)，因此需要保证fs>400才不会计算溢出

*/
void PosNotchInit(STR_POSNOTCH *p)
{
    Uint32 NumThetaIndex_Q12;     //分子部分正切索引值
	Uint32 DenThetaIndex_Q12;	  //分母部分正切索引值
	Uint32 MaxFreq;              //避免正切值溢出的最大设置
	int32  SinValue;			  //正弦值
	int32  CosValue;     	      //余弦值
	int32  NumTanValue;			  //分子正切值
	int32  DenTanValue;
	int32  Coff_Q15;            //分子分母放大倍数，Q15格式

	STR_FUNC_Gvar.Monitor2Flag.bit.LagFilterClr = 1;	  //位置陷波器清除
	
	p->SampFreq   = STR_FUNC_Gvar.System.PosFreq;      //采样频率为位置环频率
	MaxFreq       = (p->SampFreq * 10 * 3)>>3;    //允许设置的最大频率，不能大于3*fs/8，避免正切值溢出
    
	p->NotchFreq  = FunCodeUnion.code.AT_LowOscFreqA;  //中心频率设置为低频抑制频率，带一个小数点
	//避免正切值计算时溢出
	if(p->NotchFreq > MaxFreq)
	{
	    p->NotchFreq = MaxFreq;
	}		
		
	//保证分母频率大于分子频率
	if(FunCodeUnion.code.AT_LowFreqRatio > 10)
	{
	    p->DenNotchFreq = p->NotchFreq * FunCodeUnion.code.AT_LowFreqRatio/10;    //分母频率，带一个小数点

		//避免正切值计算时溢出
		if(p->DenNotchFreq > MaxFreq)
		{
		    p->DenNotchFreq = p->NotchFreq * 12/10;
		}
	}
	else
	{
	    p->DenNotchFreq = p->NotchFreq * 12/10;    //分母频率，带一个小数点
	}


   	p->DenSigma     = PosWidthCoff_Q10[FunCodeUnion.code.AT_LowOscFiltA]>>1;     //Q10格式, 2*DenSigma*NotchFreq表示宽度
	p->NumSigma     = p->DenSigma>>7;        //Q10格式  输入输出比NumSigma/DenSigma为1/128，表示深度

	NumThetaIndex_Q12 = (p->NotchFreq<<20)/(2 * p->SampFreq * 10);    //获取分子索引值，0~256对应到0~2pi，注意频率带一个小数点
	DenThetaIndex_Q12 = (p->DenNotchFreq<<20)/(2 * p->SampFreq * 10); //获取分母索引值

	SinValue = SinCal(NumThetaIndex_Q12);    //Q15格式的数
	CosValue = CosCal(NumThetaIndex_Q12);    //Q15格式的数
	NumTanValue = ((int64)SinValue<<20) / CosValue;  //分子正切值，Q20格式的数，由于正切值在4K调度频率下，最大为0.0027，需要取Q20格式的值


	SinValue = SinCal(DenThetaIndex_Q12);
	CosValue = CosCal(DenThetaIndex_Q12);
	DenTanValue = ((int64)SinValue<<20) / CosValue;   //分母正切值，Q20格式的数

	Coff_Q15 = (((int64)DenTanValue<<15) * DenTanValue) / ((int64)NumTanValue * NumTanValue);      //系数为Q15格式的数，tan(f2*pi/fs)^2 / tan(f1*pi/fs)^2;

	//计算除Coff外的各系数，Q23格式
	p->NumCoeff[0] = (1L<<23) + (((int64)NumTanValue * p->NumSigma * 2)>>7) +  (((int64)NumTanValue * NumTanValue) >> 17);
    p->NumCoeff[1] = (-2L<<23) + (((int64)NumTanValue * NumTanValue * 2) >> 17);
    p->NumCoeff[2] = (1L<<23) - (((int64)NumTanValue * p->NumSigma * 2)>>7) +  (((int64)NumTanValue * NumTanValue) >> 17);

	p->DenCoeff[0] = (1L<<23) + (((int64)DenTanValue * p->DenSigma * 2)>>7) +  (((int64)DenTanValue * DenTanValue) >> 17);
    p->DenCoeff[1] = (-2L<<23) + (((int64)DenTanValue * DenTanValue) * 2 >> 17);
	p->DenCoeff[2] = (1L<<23) - (((int64)DenTanValue * p->DenSigma * 2)>>7) +  (((int64)DenTanValue * DenTanValue) >> 17);
	
	//加入Coff后，还是使用Q23格式
	p->NumCoeff[0] = ((int64)p->NumCoeff[0] * Coff_Q15) >> 15;
	p->NumCoeff[1] = ((int64)p->NumCoeff[1] * Coff_Q15) >> 15;
	p->NumCoeff[2] = ((int64)p->NumCoeff[2] * Coff_Q15) >> 15;

    //将DenCoeff[0]定义为2^23,以便于运算。
	p->NumCoeff[0] = ((int64)p->NumCoeff[0]<<23) / p->DenCoeff[0];
	p->NumCoeff[1] = ((int64)p->NumCoeff[1]<<23) / p->DenCoeff[0];
	p->NumCoeff[2] = ((int64)p->NumCoeff[2]<<23) / p->DenCoeff[0];
	p->DenCoeff[1] = ((int64)p->DenCoeff[1]<<23) / p->DenCoeff[0];
	
	p->DenCoeff[0] = (1L<<23);		    //Den[0]设置成幂级数，方便计算
	p->DenCoeff[2] = p->NumCoeff[2] + p->NumCoeff[1] + p->NumCoeff[0] - p->DenCoeff[0] - p->DenCoeff[1];  //保证分子分母累加和相等。 

	STR_InnerGvarPosCtrl.MutexBit.bit.LowOscFlag = 0;   //完成初始化后允许进行下一次辨识
	STR_FUNC_Gvar.Monitor2Flag.bit.LagFilterClr = 0;	 //位置陷波不再清除
}


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.   位置指令陷波器过程计算
    2.
********************************************************************************/
void PosNotchFilter(STR_POSNOTCH *p)
{
    int64 Temp;          //中间变量
    int32 PosInput;      //本周期位置指令输入   
    
    PosInput = p->Input[0];    //当前周期输入
    
    //计算本周期输出
    Temp = ((int64)p->NumCoeff[0] * PosInput) + ((int64)p->NumCoeff[1] * p->Input[1]) + ((int64)p->NumCoeff[2] * p->Input[2]);
    Temp = Temp - ((int64)p->DenCoeff[1] * p->Output[1]) - (((int64)p->DenCoeff[1] * p->PosRmn[0])>>23);     //考虑上周期计算的余量
    Temp = Temp - ((int64)p->DenCoeff[2] * p->Output[2]) - (((int64)p->DenCoeff[2] * p->PosRmn[1])>>23);
    
    if(Temp >= 0)
    {
        p->Output[0] = (Temp + (1L<<22))>>23;   //本周期输出
    }
    else
    {
        p->Output[0] = -(((-Temp) + (1L<<22))>>23);
    }
    
    p->PosRmn[1] = p->PosRmn[0];
    p->PosRmn[0] = Temp - ((int64)p->Output[0]<<23);     //计算本周期残留   
    
    //输入输出值更新
    p->Input[2]  = p->Input[1];
    p->Input[1]  = PosInput;
    p->Output[2] = p->Output[1];
    p->Output[1] = p->Output[0];
    
    //本周期输入为0时，需要判定是否结束陷波器计算
    if(p->Input[0] == 0)
    {
        if(ABS(p->Output[0]) > ABS(p->PosDelta))   //当本周期输出比残留脉冲还要大时，清除剩余脉冲
        {
            p->Output[0] = p->PosDelta;
            p->Input[1]  = 0;
            p->Input[2]  = 0;
            p->Output[1] = 0;
            p->Output[2] = 0;
            p->PosRmn[0] = 0;
            p->PosRmn[1] = 0;
        }
    }
    
    //计算输入输出脉冲的残留量
    p->PosSumIn  += p->Input[0];
    p->PosSumOut += p->Output[0];
    
    p->PosDelta = p->PosSumIn - p->PosSumOut;

}



/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.   位置指令陷波器清零操作
    2.
********************************************************************************/
void PosNotchFilterClr(STR_POSNOTCH *p)
{
    p->Input[1]  = 0;
    p->Input[2]  = 0;
    p->Output[1] = 0;
    p->Output[2] = 0;
    p->PosRmn[0] = 0;
    p->PosRmn[1] = 0;
    p->PosSumIn  = 0;  
    p->PosSumOut = 0; 
    p->PosDelta  = 0;
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.  
    2.
********************************************************************************/
void InitLowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p)
{
    if(p->Tc > p->Ts)
    {
        //ka=Ts/(2*Tc+Ts)
        p->Ka_Q20 = ((int32)p->Ts << 20) / ((p->Tc << 1) + p->Ts);
        if(p->Ka_Q20 < 400)      // Tc < 1600 * Ts
        {
            p->Ka_Q20 = 400;
        }
        //kb=1-2*ka
        p->Kb_Q20 = (1L<<20) - (p->Ka_Q20 << 1) ;
    }
    else
    {
        p->Ka_Q20 = 0;
        p->Kb_Q20 = 0;
    }
}

/*******************************************************************************
  函数名: void LowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p)
  输入:   无 
  输出:   无 
  描述:   采用双线性变换的低通的滤波器
    1.  Y/X=1/(1+Tc*s)      s=2*(z-1)/(Ts*(z+1))
        Y/X=Ts*(z+1)/(Ts*(z+1)+2*Tc*(z-1))
        Y/X=Ts*(z+1)/(z*(2*Tc+Ts)+(-2*Tc+Ts))
        Y*z*(2*Tc+Ts)+Y*(-2*Tc+Ts)=Ts*(z+1)*X
        Y*z*(2*Tc+Ts)=Y*(2*Tc-Ts)+X*z*Ts+X*Ts
        Y*z=Y*(2*Tc-Ts)/(2*Tc+Ts)+(X*z*Ts+X*Ts)/(2*Tc+Ts)
        Y*z=Y-Y*2*Ts/(2*Tc+Ts)+(X*z*Ts+X*Ts)/(2*Tc+Ts)
        y(n)=y(n-1)*kb+(x(n)+x(n-1))*ka
        ka=Ts/(2*Tc+Ts)
        kb=1-2*ka
********************************************************************************/
void LowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p)
{
    int64  Input_Q9 = 0;
    int64  Output_Q9 = 0;

    if((p->Ka_Q20 == 0) && (p->Kb_Q20 == 0))
    {
        p->Output = p->Input;
        return;
    }
    //y(n)=y(n-1)*kb+(x(n)+x(n-1))*ka
    Input_Q9 = (int64)p->Input << 9;

    Output_Q9 = ((int64)p->Kb_Q20 * p->OutputLatch_Q9 + (int64)p->Ka_Q20 * (p->InputLatch_Q9 + Input_Q9)) >> 20;

    p->InputLatch_Q9  = Input_Q9;
    p->OutputLatch_Q9 = Output_Q9;
    p->Output  = p->OutputLatch_Q9 >> 9;  
}


//==========================================================================
// 函数名：  MidFilter()
// 输  入：  InPut;
// 输  出:   OutPut;
// 功能描述: 中值滤波
//==========================================================================
void MidFilter(STR_MidFilter *p)
{              
    static int8 count = 1;
    int32  BubbleSortArray[3] = {0,0,0};
    int32  Temp = 0;

    //取依次输入的三个数
    p->SeriData[0]= p->SeriData[1];
    p->SeriData[1]= p->SeriData[2];
    p->SeriData[2]= p->InPut;

    if(count<3)   //前两个数以输入为输出
    {
        p->OutPut = p->InPut;
        count++;
    }
    else          //从第三个数开始取中值
    {
        BubbleSortArray[0] = p->SeriData[0];
        BubbleSortArray[1] = p->SeriData[1];
        BubbleSortArray[2] = p->SeriData[2];

        if(BubbleSortArray[0] < BubbleSortArray[1])
        {
            Temp = BubbleSortArray[0];
            BubbleSortArray[0] = BubbleSortArray[1];
            BubbleSortArray[1] = Temp;
        }

        if(BubbleSortArray[0] < BubbleSortArray[2])
        {
            Temp = BubbleSortArray[0];
            BubbleSortArray[0] = BubbleSortArray[2];
            BubbleSortArray[2] = Temp;
        }

        if(BubbleSortArray[1] > BubbleSortArray[2])
        {
            p->OutPut = BubbleSortArray[1];
        }
        else
        {
            p->OutPut = BubbleSortArray[2];
        }
    }
}

//==========================================================================
// 函数名：  AvgFilter()
// 输  入：  InPut;
// 输  出:   OutPut;
// 功能描述: 中值滤波
//==========================================================================
void AvgFilter(STR_MidFilter *p)
{
	static 	int32 inputlast = 0;
	static int8 count = 1;
	int64 temp;
	if (count < 2)
	{
		p->OutPut = p->InPut;
		count ++;
	}
	else
	{
		temp = (int64)(p->InPut + inputlast);
		p->OutPut = (int32)(temp / 2);
	}
	inputlast = p->InPut;
}

/********************************* END OF FILE *********************************/
