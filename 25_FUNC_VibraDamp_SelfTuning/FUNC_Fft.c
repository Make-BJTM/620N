/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Fft.c                                                                
 创建人：马世贤            创建日期：2012.09.09 
  
 修改人：姚虹 			   2014.08.15
 描述： 
    1.自适应滤波器共振频率检测，FFT运算。
 修改记录：
    1.将FFT计算改为按频率抽取，并在电流环中断中拆开计算,拆分
	  的步骤最大计算时间为3us。
	2.旋转因子使用查表方式，不再根据正弦表来计算 
    3.修改了自适应处理过程
********************************************************************************/
#include "PUB_GlobalPrototypes.h"      //全局变量外部声明 
#include "PUB_Table.h"	 
#include "PUB_Library_Function.h"
#include "FUNC_FunCode.h"
#include "FUNC_Fft.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_OperEeprom.h" 
 

//主调度函数处理步骤宏定义
#define  FFT_PROCESS_STEP0  0
#define  FFT_PROCESS_STEP1  1
#define  FFT_PROCESS_STEP2  2
#define  FFT_PROCESS_STEP3  3
#define  FFT_PROCESS_STEP4  4
#define  FFT_PROCESS_STEP5	5

//FFT计算步骤宏定义
#define FFTCAL_STEP0    0 
#define FFTCAL_STEP1    1 
#define FFTCAL_STEP2    2 
#define FFTCAL_STEP3    3
#define FFTCAL_STEP4    4
#define FFTCAL_STEP5    5
#define FFTCAL_STEP6    6
#define FFTCAL_STEP7    7
	

//FFT采样步骤宏定义
#define  FFT_SAMPLE_STEP0 0
#define  FFT_SAMPLE_STEP1 1


//共振阈值，当FFT频域值大于此设置时，认为发生了共振，2%的额定转矩,考虑有20%的能量损失，定义
//为：2%*4096*80%
//#define  RESONANTLMT     64  

//用于比较频率变化的系数，分别对应1.05,1.1
#define  FIRSTPROP_Q10  1075
#define  SECONDPROP_Q10 1126


//1024点FFT计算时旋转因子定义，使用Q14格式的数，Real为实部，Image为虚部
//Tw = cos(2pi*n/N)+j(-sin(2pi*n/N));
const int16 Tw_Real[512] = 
{
16384, 16383, 16382, 16381, 16379, 16376, 16372, 16368, 16364, 16359, 16353, 16346, 16339, 16331, 16323, 16314,
16305, 16294, 16284, 16272, 16260, 16248, 16234, 16221, 16206, 16191, 16175, 16159, 16142, 16125, 16107, 16088,
16069, 16049, 16028, 16007, 15985, 15963, 15940, 15917, 15892, 15868, 15842, 15817, 15790, 15763, 15735, 15707,
15678, 15649, 15618, 15588, 15557, 15525, 15492, 15459, 15426, 15392, 15357, 15322, 15286, 15249, 15212, 15175,
15136, 15098, 15058, 15018, 14978, 14937, 14895, 14853, 14810, 14767, 14723, 14679, 14634, 14589, 14543, 14496,
14449, 14401, 14353, 14304, 14255, 14205, 14155, 14104, 14053, 14001, 13948, 13895, 13842, 13788, 13733, 13678,
13622, 13566, 13510, 13452, 13395, 13337, 13278, 13219, 13159, 13099, 13038, 12977, 12916, 12854, 12791, 12728,
12665, 12600, 12536, 12471, 12406, 12340, 12273, 12207, 12139, 12072, 12003, 11935, 11866, 11796, 11726, 11656,
11585, 11513, 11442, 11370, 11297, 11224, 11150, 11077, 11002, 10928, 10853, 10777, 10701, 10625, 10548, 10471,
10393, 10315, 10237, 10159, 10079, 10000, 9920,  9840,  9759,  9679,  9597,  9516,  9434,  9351,  9268,  9185,
9102,  9018,  8934,  8850,  8765,  8680,  8594,  8509,  8423,  8336,  8249,  8162,  8075,  7988,  7900,  7811,
7723,  7634,  7545,  7456,  7366,  7276,  7186,  7095,  7005,  6914,  6822,  6731,  6639,  6547,  6455,  6362,
6269,  6176,  6083,  5990,  5896,  5802,  5708,  5614,  5519,  5424,  5329,  5234,  5139,  5043,  4948,  4852,
4756,  4659,  4563,  4466,  4369,  4272,  4175,  4078,  3980,  3883,  3785,  3687,  3589,  3491,  3393,  3294,
3196,  3097,  2998,  2900,  2801,  2701,  2602,  2503,  2404,  2304,  2204,  2105,  2005,  1905,  1805,  1705,
1605,  1505,  1405,  1305,  1205,  1105,  1004,  904,   803,   703,	  603,   502,	402,   301,	  201,   100,
0,	   -100,  -201,	 -301,	-402,  -502,  -603,  -703,	-803,  -904,  -1004, -1105, -1205, -1305, -1405, -1505,
-1605, -1705, -1805, -1905, -2005, -2105, -2204, -2304, -2404, -2503, -2602, -2701, -2801, -2900, -2998, -3097,
-3196, -3294, -3393, -3491, -3589, -3687, -3785, -3883, -3980, -4078, -4175, -4272, -4369, -4466, -4563, -4659,
-4756, -4852, -4948, -5043, -5139, -5234, -5329, -5424, -5519, -5614, -5708, -5802, -5896, -5990, -6083, -6176,
-6269, -6362, -6455, -6547, -6639, -6731, -6822, -6914, -7005, -7095, -7186, -7276, -7366, -7456, -7545, -7634,
-7723, -7811, -7900, -7988, -8075, -8162, -8249, -8336, -8423, -8509, -8594, -8680, -8765, -8850, -8934, -9018,
-9102, -9185, -9268, -9351, -9434, -9516, -9597, -9679, -9759, -9840, -9920, -10000,-10079,-10159,-10237,-10315,
-10393,-10471,-10548,-10625,-10701,-10777,-10853,-10928,-11002,-11077,-11150,-11224,-11297,-11370,-11442,-11513,
-11585,-11656,-11726,-11796,-11866,-11935,-12003,-12072,-12139,-12207,-12273,-12340,-12406,-12471,-12536,-12600,
-12665,-12728,-12791,-12854,-12916,-12977,-13038,-13099,-13159,-13219,-13278,-13337,-13395,-13452,-13510,-13566,
-13622,-13678,-13733,-13788,-13842,-13895,-13948,-14001,-14053,-14104,-14155,-14205,-14255,-14304,-14353,-14401,
-14449,-14496,-14543,-14589,-14634,-14679,-14723,-14767,-14810,-14853,-14895,-14937,-14978,-15018,-15058,-15098,
-15136,-15175,-15212,-15249,-15286,-15322,-15357,-15392,-15426,-15459,-15492,-15525,-15557,-15588,-15618,-15649,
-15678,-15707,-15735,-15763,-15790,-15817,-15842,-15868,-15892,-15917,-15940,-15963,-15985,-16007,-16028,-16049,
-16069,-16088,-16107,-16125,-16142,-16159,-16175,-16191,-16206,-16221,-16234,-16248,-16260,-16272,-16284,-16294,
-16305,-16314,-16323,-16331,-16339,-16346,-16353,-16359,-16364,-16368,-16372,-16376,-16379,-16381,-16382,-16383
};

const int16 Tw_Imag[512] = 
{
0,	   -100,  -201,  -301,	-402,  -502,  -603,  -703,	-803,  -904,  -1004, -1105,	-1205, -1305, -1405, -1505 ,
-1605, -1705, -1805, -1905, -2005, -2105, -2204, -2304, -2404, -2503, -2602, -2701, -2801, -2900, -2998, -3097 ,
-3196, -3294, -3393, -3491, -3589, -3687, -3785, -3883, -3980, -4078, -4175, -4272, -4369, -4466, -4563, -4659 ,
-4756, -4852, -4948, -5043, -5139, -5234, -5329, -5424, -5519, -5614, -5708, -5802, -5896, -5990, -6083, -6176 ,
-6269, -6362, -6455, -6547, -6639, -6731, -6822, -6914, -7005, -7095, -7186, -7276, -7366, -7456, -7545, -7634 ,
-7723, -7811, -7900, -7988, -8075, -8162, -8249, -8336, -8423, -8509, -8594, -8680, -8765, -8850, -8934, -9018 ,
-9102, -9185, -9268, -9351, -9434, -9516, -9597, -9679, -9759, -9840, -9920, -10000,-10079,-10159,-10237,-10315,
-10393,-10471,-10548,-10625,-10701,-10777,-10853,-10928,-11002,-11077,-11150,-11224,-11297,-11370,-11442,-11513,
-11585,-11656,-11726,-11796,-11866,-11935,-12003,-12072,-12139,-12207,-12273,-12340,-12406,-12471,-12536,-12600,
-12665,-12728,-12791,-12854,-12916,-12977,-13038,-13099,-13159,-13219,-13278,-13337,-13395,-13452,-13510,-13566,
-13622,-13678,-13733,-13788,-13842,-13895,-13948,-14001,-14053,-14104,-14155,-14205,-14255,-14304,-14353,-14401,
-14449,-14496,-14543,-14589,-14634,-14679,-14723,-14767,-14810,-14853,-14895,-14937,-14978,-15018,-15058,-15098,
-15136,-15175,-15212,-15249,-15286,-15322,-15357,-15392,-15426,-15459,-15492,-15525,-15557,-15588,-15618,-15649,
-15678,-15707,-15735,-15763,-15790,-15817,-15842,-15868,-15892,-15917,-15940,-15963,-15985,-16007,-16028,-16049,
-16069,-16088,-16107,-16125,-16142,-16159,-16175,-16191,-16206,-16221,-16234,-16248,-16260,-16272,-16284,-16294,
-16305,-16314,-16323,-16331,-16339,-16346,-16353,-16359,-16364,-16368,-16372,-16376,-16379,-16381,-16382,-16383,
-16384,-16383,-16382,-16381,-16379,-16376,-16372,-16368,-16364,-16359,-16353,-16346,-16339,-16331,-16323,-16314,
-16305,-16294,-16284,-16272,-16260,-16248,-16234,-16221,-16206,-16191,-16175,-16159,-16142,-16125,-16107,-16088,
-16069,-16049,-16028,-16007,-15985,-15963,-15940,-15917,-15892,-15868,-15842,-15817,-15790,-15763,-15735,-15707,
-15678,-15649,-15618,-15588,-15557,-15525,-15492,-15459,-15426,-15392,-15357,-15322,-15286,-15249,-15212,-15175,
-15136,-15098,-15058,-15018,-14978,-14937,-14895,-14853,-14810,-14767,-14723,-14679,-14634,-14589,-14543,-14496,
-14449,-14401,-14353,-14304,-14255,-14205,-14155,-14104,-14053,-14001,-13948,-13895,-13842,-13788,-13733,-13678,
-13622,-13566,-13510,-13452,-13395,-13337,-13278,-13219,-13159,-13099,-13038,-12977,-12916,-12854,-12791,-12728,
-12665,-12600,-12536,-12471,-12406,-12340,-12273,-12207,-12139,-12072,-12003,-11935,-11866,-11796,-11726,-11656,
-11585,-11513,-11442,-11370,-11297,-11224,-11150,-11077,-11002,-10928,-10853,-10777,-10701,-10625,-10548,-10471,
-10393,-10315,-10237,-10159,-10079,-10000,-9920, -9840, -9759, -9679, -9597, -9516, -9434, -9351, -9268, -9185, 
-9102, -9018, -8934, -8850, -8765, -8680, -8594, -8509, -8423, -8336, -8249, -8162, -8075, -7988, -7900, -7811, 
-7723, -7634, -7545, -7456, -7366, -7276, -7186, -7095, -7005, -6914, -6822, -6731, -6639, -6547, -6455, -6362, 
-6269, -6176, -6083, -5990, -5896, -5802, -5708, -5614, -5519, -5424, -5329, -5234, -5139, -5043, -4948, -4852, 
-4756, -4659, -4563, -4466, -4369, -4272, -4175, -4078, -3980, -3883, -3785, -3687, -3589, -3491, -3393, -3294, 
-3196, -3097, -2998, -2900, -2801, -2701, -2602, -2503, -2404, -2304, -2204, -2105, -2005, -1905, -1805, -1705, 
-1605, -1505, -1405, -1305, -1205, -1105, -1004, -904,  -803,  -703,  -603,  -502,  -402,  -301,  -201,  -100
};


STR_FFTFUNC  FFTFunc; 

STR_COMPLX   FftArray[1024];     //用于FFT计算的1024点数据

static Uint16 AdaptiveSampleStart=0;
static Uint16 AdaptiveSampleEnd=0;

static Uint16 AdaptiveQuitFlag = 0;    //自适应滤波器退出标志位,1~退出
static Uint16 AdaptiveQuitCnt  = 0;	   //自适应滤波器退出时计数器，连续20次计算无共振点时，退出

static Uint16 FftProcessStep = FFT_PROCESS_STEP0;
static Uint16 FftSampleStep = FFT_SAMPLE_STEP0;
static Uint16 FftCalStep = FFTCAL_STEP0;

void FftInit(void);	//fft初始化函数
void FftProcess(void); //fft主调度函数
void FftSample(int32 TorqCmdTemp);//FFT采样函数

void FftUpdate(void);       //FFT中需要在线更新的参数
void FftStopUpdate(void);

Uint16 FftCal(void);

/*******************************************************************************
  函数名:  FftInit(void)
  输入:   1.FFT计算涉及的几个主要变量

  输出:   1. 无
  子函数: 无    
  描述: FFT计算涉及的几个主要变量初始化，包括采样频率Fs,FFT运算点数N，阶数M等。 
********************************************************************************/
void FftInit(void)
{
   FFTFunc.Fs = STR_FUNC_Gvar.System.ToqFreq>>1;	//采样频率为电流环频率一半，其辨识范围为Fs/2。
   FFTFunc.N  = NN;		  //采集点数
   FFTFunc.M  = MM;			  //FFT运算层级，2^M=N
   FFTFunc.OldNtchUpdtFlag = 0;

   FFTFunc.CntPerSec    = 0;    //用于自适应滤波器开启后自动退出的设置
   FFTFunc.QuitDelaySec = 0;

   FFTFunc.Mag[1] = UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT * 3;    //设置初始用于比较的幅值为3倍额定转矩

   FunCodeUnion.code.AT_NotchFiltFreqDisp = 0;

   FftProcessStep =	FFT_PROCESS_STEP0; 
   FftCalStep     = FFTCAL_STEP0;

   STR_FUNC_Gvar.Fft.FftCalCnt 			   = 0;
   STR_FUNC_Gvar.System.AdaptiveFilterMode = FunCodeUnion.code.AT_AdaptiveFilterMode;     
}


/*******************************************************************************
  函数名:  FftUpdate(void)
  输入:   1.无

  输出:   1. 无
  子函数: 无    
  描述: FFT计算需要运行更新的参数 
********************************************************************************/
void FftUpdate(void)
{
       //获取设定的震动阈值对应的数字量，考虑有1/8的能量损失，因而乘7/8；
    STR_FUNC_Gvar.Fft.OscillateLevel = ((int32)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 
                                        * FunCodeUnion.code.AT_VibThrshld * 7) >> 15;
}

/*******************************************************************************
  函数名:  FftStopUpdate(void)
  输入:   1.无

  输出:   1. 无
  子函数: 无    
  描述: FFT计算需要在停机更新的参数 
********************************************************************************/
void FftStopUpdate(void)
{
    STR_FUNC_Gvar.Fft.FftCalCnt = 0;
	STR_FUNC_Gvar.Fft.OscillateValue = 0;
}


//二进制码位倒读,只考虑输入Index最大值为1024的情形
Uint16 BinReverse(Uint16 Index)
{	
	union
	{
	    Uint16 all;

		struct
	    {
	        Uint16 bit0:1;
		    Uint16 bit1:1;
		    Uint16 bit2:1;
		    Uint16 bit3:1;
		    Uint16 bit4:1;
		    Uint16 bit5:1;
		    Uint16 bit6:1;
		    Uint16 bit7:1;
		    Uint16 bit8:1;
			Uint16 bit9:1;
		    Uint16 bit10:6;
	    }Str_Num;		
	}Union_Input,Union_Output;

	Union_Input.all  = Index;	//获取初值
	Union_Output.all = 0;

	//二进制数值按位交换,即码位倒读
	Union_Output.Str_Num.bit0 = Union_Input.Str_Num.bit9;
	Union_Output.Str_Num.bit1 = Union_Input.Str_Num.bit8;
	Union_Output.Str_Num.bit2 = Union_Input.Str_Num.bit7;
	Union_Output.Str_Num.bit3 = Union_Input.Str_Num.bit6; 
	Union_Output.Str_Num.bit4 = Union_Input.Str_Num.bit5;
	Union_Output.Str_Num.bit5 = Union_Input.Str_Num.bit4;
	Union_Output.Str_Num.bit6 = Union_Input.Str_Num.bit3;
	Union_Output.Str_Num.bit7 = Union_Input.Str_Num.bit2;
	Union_Output.Str_Num.bit8 = Union_Input.Str_Num.bit1;
	Union_Output.Str_Num.bit9 = Union_Input.Str_Num.bit0;

	return(Union_Output.all);
}

//将循环计算拆分成每步计算，以便能在中断中运行,1024点计算
//循环总数为(N/2)*logN=512*10=5120次
Uint16 FftCal(void)
{
    static int32 n = 0;	   //每个颗粒的输入样本索引
	static int32 m = MM;	   //当前运算的层级,2点DFT运算时为第一级
	static int32 k = 0;            //每层的颗粒索引
	static int32 GrnNum;		//每个层级颗粒个数
	static int32 InputNum;     //每个颗粒的样本输入个数
	static int32 FnshNum;      //每层中完成计算的输入样本数目	
	static int32 Step;         //每个颗粒蝶形运算时的跳转数，即N/2;
	static int32 Index;        //旋转因子索引
	static int32 Comp = 0;	  //保存的比较值
	static int32 MaxAmp;	  //保存的最大幅值
	static int16 i = 1;       //找最大幅值点时的索引,从第1个点开始，不从0开始是因为不用比较基波的值
	static int16 MaxIndex = 0;	  //最大幅值点对应的索引值

	    
	int32 Amp;		      //幅值	
	int32 Temp1,Temp2;
	int16 Index1, Index2;     //蝶形运算时计算输出的索引变量
	Uint16 Flag = 0;          //返回的标志位
	
	STR_COMPLX  TT1 = {0};		     //辅助变量
	STR_COMPLX  Tw  = {0};            //蝶形运算时的旋转因子

	switch(FftCalStep)
	{
	    case FFTCAL_STEP0:			   //初始值赋值处理
		    m = MM;			  //蝶形运算层级循环，2点DFT运算为第一级
			k = 0;			  //蝶形运算的颗粒循环
			n = 0;			  //蝶形运算的输入样本循环
			
		   	InputNum  = NN;	  //初始颗粒样本输入个数为N个
			
			FftCalStep = FFTCAL_STEP1;			
			
		break; 

		case FFTCAL_STEP1:		//层级循环
		    if(m > 0)
			{				
				GrnNum  = NN>>m;		   //确定该层级蝶形运算颗粒个数
		        Step    = InputNum>>1;	   //每个颗粒蝶形运算时的跳转数N/2;
		        FnshNum = 0;               //将每层已完成计算的输入样本个数置0；
		
				FftCalStep = FFTCAL_STEP2;	//进入下一步								
			}
			else
			{
			    i    = 1;					//不用比较基波，从1开始
				Comp = 0;					//设置用于比较的初始幅值
				FftCalStep = FFTCAL_STEP4;	//结束FFT运算,开始比较找最大幅值点
			}		
		break;

		case FFTCAL_STEP2:		//蝶形运算颗粒循环
			if(k < GrnNum)
			{
			    FftCalStep = FFTCAL_STEP3;
			}
			else
			{
			    k = 0;			//计算完一层，需要计算下一层
				m--;
				InputNum   = Step;        //确定下一层每个颗粒的样本输入个数
				FftCalStep = FFTCAL_STEP1;
			}
		break;

		case FFTCAL_STEP3:		//蝶形运算的输入样本循环
				
			//通过查表计算旋转因子，也可通过正弦表计算，但要消耗计算时间
			Index   = GrnNum * n;      //得到旋转因子索引
			Tw.Real = Tw_Real[Index];	  //获取旋转因子
			Tw.Imag = Tw_Imag[Index];

			//求取蝶形运算的结果
			Index1 = n+FnshNum;				 //两点交叉相乘时第一个输出索引
			Index2 = n+FnshNum+Step;		 //两点交叉相乘时第二个输出索引
									 
			TT1.Real = FftArray[Index1].Real + FftArray[Index2].Real;
			TT1.Imag = FftArray[Index1].Imag + FftArray[Index2].Imag;
			Temp1    = FftArray[Index1].Real - FftArray[Index2].Real;
			Temp2    = FftArray[Index1].Imag - FftArray[Index2].Imag;

			FftArray[Index1].Real = TT1.Real;
			FftArray[Index1].Imag = TT1.Imag;
			FftArray[Index2].Real = (((int64)Temp1*Tw.Real)>>14) - (((int64)Temp2*Tw.Imag)>>14);	    //旋转因子为Q14格式的数
			FftArray[Index2].Imag = (((int64)Temp1*Tw.Imag)>>14) + (((int64)Temp2*Tw.Real)>>14);

			n++;
			
			if(n >= Step)				 //计算完一个颗粒，准备计算下一个颗粒
			{
			    n = 0;			 
			    k++;
			    FnshNum	+= InputNum;	//已完成颗粒的输入样本数累加
			    FftCalStep = FFTCAL_STEP2;
			}							    

		break;

		case FFTCAL_STEP4:		    //找到计算结果中幅值最大的点,幅值大小需要减小一定比例以避免计算溢出
		    if(i < (NN-1))
			{
			    Amp = ((int64)FftArray[i].Real*FftArray[i].Real + (int64)FftArray[i].Imag*FftArray[i].Imag) >> 16;
				
				if(Amp > Comp)		      //获取幅值最大的索引
		        {
		            Comp     = Amp;
			        MaxIndex = i;
					MaxAmp   = Amp;
		        }
		        i++;
			}
			else
			{
			    FftCalStep = FFTCAL_STEP5;	 //码序倒读
			}
		break;

		case FFTCAL_STEP5:					//码序倒读，取得最大幅值对应的索引值
		    i = BinReverse(MaxIndex);

			FftCalStep = FFTCAL_STEP6;
		break;
		
		case FFTCAL_STEP6:	      //计算最大幅值对应的频率点,使用四舍五入提高精度
		    Temp1 = (i*FFTFunc.Fs +  (NN>>1))>>MM;	
					
			if(Temp1 > (FFTFunc.Fs>>1))	    //考虑频谱对称的特性，最大频率点必须小于Fs/2;
			{
			    FFTFunc.Freq = FFTFunc.Fs - Temp1;
			}
			else
			{
			    FFTFunc.Freq = Temp1;
			}

			FftCalStep = FFTCAL_STEP7;
		break;

	    case FFTCAL_STEP7:	      //计算最大幅值对应的能量值:qsqrt(Amp)*2/N;
			
			FFTFunc.Mag[0]   = ((qsqrt32(MaxAmp))<<8) >> (MM-1);

			FftCalStep       = FFTCAL_STEP0;  				      //返回第一步，并返回计算成功标志
		    
			Flag = 1;		     //完成计算
		break;

		default:
		break;
	} 
	
	return Flag;   
}

 
/*******************************************************************************
  函数名:  FftProcess(void)
  输入:   1.FunCodeUnion.code.TL_AdaptiveFilterMode ，自适应滤波器工作模式
          2.

  输出:   1. FunCodeUnion.code.TL_NotchFiltFreqC   陷波器C的中心频率
		     FunCodeUnion.code.TL_NotchFiltBandWidthC 陷波器C的陷波带宽
		     FunCodeUnion.code.TL_NotchFiltFreqD   陷波器D的中心频率
			 FunCodeUnion.code.TL_NotchFiltBandWidthD 陷波器D的陷波带宽
  子函数: fftCal     
  描述: 自适应滤波器共振频率检测调度函数，实现自适应滤波器5种工作模式的切换，调用FFT计算函数检测共振频率
        ，陷波器共振频率及陷波带宽设定。
		在主循环中调用，调用周期为1ms。 
********************************************************************************/	
void FftProcess(void)
{    
	static Uint32 FftSmpCnt = 0;			 //用于确定采样频率
	static int32  DelayCnt = 0;
	static Uint32 WidthValue = 0;	        //陷波器宽度设置
	static Uint32 DepthValue = 1;           //陷波器深度设置
	static int32 NotchFilterDepth = 0;		//共振点对应的深度值

	static Uint32 UpdateCnt = 0;    //自适应滤波器模式改变后需要设置延时才能正常采样运行，保证陷波器更新完

    int32 DeltaFreqC;      //陷波器C中心频率变化量百分比    
    int32 DeltaFreqD;      //陷波器D中心频率变化量百分比
	int32 BigFreq;		   //用于比较测试频率及已设置频率
	int32 SmllFreq; 	
	int16 TempValue;		//辅助计算变量
	int16 NotchUpdateFlag = 0;		//用于确认更新哪个陷波器，1~陷波器C，2~陷波器D	 
		
	//当设置值发生改变时，重新初始化
	if(STR_FUNC_Gvar.System.AdaptiveFilterMode !=  FunCodeUnion.code.AT_AdaptiveFilterMode)
	{
	    FftInit();									//初始化后从第一步开始

		if(FunCodeUnion.code.AT_AdaptiveFilterMode == 2)   //当模式切换为两个共振点抑制时,清除相应陷波器
		{
			FunCodeUnion.code.AT_NotchFiltFreqC = 4000;
			FunCodeUnion.code.AT_NotchFiltBandWidthC =	2;
            FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = 0;
		    FunCodeUnion.code.AT_NotchFiltFreqD = 4000;
			FunCodeUnion.code.AT_NotchFiltBandWidthD =	2;
            FunCodeUnion.code.AT_NotchFiltAttenuatLvlD = 0;
		}
		else if(FunCodeUnion.code.AT_AdaptiveFilterMode == 1)   //一个共振点时，只清除一个陷波器
		{
			FunCodeUnion.code.AT_NotchFiltFreqC = 4000;
			FunCodeUnion.code.AT_NotchFiltBandWidthC =	2;
            FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = 0;
		}

		UpdateCnt = 0;
	}

	//避免自适应滤波器模式改变后，为避免陷波器复位导致的抖动，延时1s后采样
	if(UpdateCnt < STR_FUNC_Gvar.System.SpdFreq)
	{
	    UpdateCnt++;
		return;
	}

	//当开启自适应后，经过半个小时，无论辨识结果如何，都需要退出
	if((FunCodeUnion.code.AT_AdaptiveFilterMode == 1) || (FunCodeUnion.code.AT_AdaptiveFilterMode == 2))
	{
		FFTFunc.CntPerSec++;

		if(FFTFunc.CntPerSec == STR_FUNC_Gvar.System.SpdFreq)   //累加次数等于速度环周期时认为经过1s
		{
		    FFTFunc.QuitDelaySec++;		    //秒计数加1
			FFTFunc.CntPerSec  = 0;
		}

		if(FFTFunc.QuitDelaySec == 1800)    //经过1800s后自适应退出
		{
		    FunCodeUnion.code.AT_AdaptiveFilterMode = 0; 

			SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));	
		}		
	}
	    
	switch(FftProcessStep)
    {
        case FFT_PROCESS_STEP0:						     //根据模式设定来确定下一步动作
		    if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 0)
			{
			    //不处理，直接结束
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 1)	  //只检测一个共振点时
			{	   //判断设置陷波器后振动是否消失，如果是则需要退出
			    if((AdaptiveQuitFlag == 1) && (FunCodeUnion.code.AT_NotchFiltFreqC != 4000)
                   &&(AuxFunCodeUnion.code.H2F_Rsvd33 == 0))    //判断是否结束自适应，需要在非自调整状态下
				{		   
					AdaptiveQuitFlag = 0;
					AdaptiveQuitCnt  = 0;
					FunCodeUnion.code.AT_AdaptiveFilterMode = 0;
					SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));				    
				}
				else
				{
				    FftProcessStep      = FFT_PROCESS_STEP1;
				    AdaptiveSampleStart = 1;
				    AdaptiveSampleEnd   = 0;				    
				}
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 2)	 //检测两个共振点时
			{			 //判断设置陷波器后振动是否变大，如果是则需要退出
			    if((AdaptiveQuitFlag == 1) && (FunCodeUnion.code.AT_NotchFiltFreqC != 4000)
				   && (FunCodeUnion.code.AT_NotchFiltFreqD != 4000))    //判断是否退出
				{				    
					AdaptiveQuitFlag = 0;
					AdaptiveQuitCnt  = 0;
					FunCodeUnion.code.AT_AdaptiveFilterMode = 0;
					SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));				    
				}
				else
				{
				    FftProcessStep      = FFT_PROCESS_STEP1;
				    AdaptiveSampleStart = 1;
				    AdaptiveSampleEnd   = 0;				    
				}			    
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 3) //只显示共振点时
			{				 
				FftProcessStep      = FFT_PROCESS_STEP1;
				AdaptiveSampleStart = 1;
				AdaptiveSampleEnd   = 0;
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 4) //清除自适应滤波器的值。
			{
				FunCodeUnion.code.AT_NotchFiltFreqC = 4000;
				FunCodeUnion.code.AT_NotchFiltBandWidthC =	2;
                FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = 0;
				FunCodeUnion.code.AT_NotchFiltFreqD = 4000;
				FunCodeUnion.code.AT_NotchFiltBandWidthD =	2;
                FunCodeUnion.code.AT_NotchFiltAttenuatLvlD = 0;
				FunCodeUnion.code.AT_AdaptiveFilterMode = 0;
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlC));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqD));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthD));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlD));
			}
            break;      
        case FFT_PROCESS_STEP1:	  //启动采样并等待采样结束			
			if((AdaptiveSampleStart==1) && (AdaptiveSampleEnd == 0))    //允许采样
			{
			    if(DelayCnt < (STR_FUNC_Gvar.System.ToqFreq >> 2))  //允许采样后要设置一个延时，避免采到刚设置陷波器后面的值
				{
				     DelayCnt++;
				}
				else
				{
					if(FftSmpCnt<STR_FUNC_Gvar.System.Const4KMsk)	 //确定采样频率
	                {
	                    FftSmpCnt++;
	                }
	                else
	                {
	                    FftSmpCnt=0;
	
	                    FftSample(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef);	   //采集转矩指令
	                }
				}				
			}
           	else if((AdaptiveSampleStart == 0) && (AdaptiveSampleEnd == 1))  //采样结束，准备进入下一步  
			{
				AdaptiveSampleStart = 0;
				AdaptiveSampleEnd   = 0;
				FftProcessStep      = FFT_PROCESS_STEP2;				 
			}
            else if((AdaptiveSampleStart == 0) && (AdaptiveSampleEnd == 0)) //采样中断使能，跳到第STEP0
            {				 
				FftProcessStep = FFT_PROCESS_STEP0;
            }
            break;
			            
      	case FFT_PROCESS_STEP2:   //FFT运算准备阶段				 

			DelayCnt       = 0;
			FftCalStep     = FFTCAL_STEP0;
			FftProcessStep = FFT_PROCESS_STEP3;

			break;
      	case  FFT_PROCESS_STEP3:  //FFT运算阶段   			     				 
			
			TempValue = FftCal();	    //开始FFT运算

			if(TempValue == 1) //运算成功
			{						   
				DelayCnt = 0;		 //清除计算不成功计数值

				STR_FUNC_Gvar.Fft.FftCalCnt++;         //运算成功次数累加				

				if(FunCodeUnion.code.AT_NotchFiltFreqC != 4000)	 //设置此标志位用于给一键式调整时振动情况判定
				{
				    STR_FUNC_Gvar.Fft.CalAfterNotchFlag = 1;
				}

			    //根据检测频率和振幅判定何时退出
				if(FFTFunc.Mag[0] > FFTFunc.Mag[1])
				{
					if(STR_FUNC_Gvar.System.AdaptiveFilterMode != 3)
					{
						if(FFTFunc.OldNtchUpdtFlag == 1)    //如果之前更新的是陷波器C
						{
						    FunCodeUnion.code.AT_NotchFiltFreqC        = FFTFunc.OldFreq;
						    FunCodeUnion.code.AT_NotchFiltBandWidthC   = FFTFunc.OldBandW;
						    FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = FFTFunc.OldDepth;				      
						}
						else if(FFTFunc.OldNtchUpdtFlag == 2)  //如果之前更新的是陷波器D
						{
						    FunCodeUnion.code.AT_NotchFiltFreqD        = FFTFunc.OldFreq;
						    FunCodeUnion.code.AT_NotchFiltBandWidthD   = FFTFunc.OldBandW;
						    FunCodeUnion.code.AT_NotchFiltAttenuatLvlD = FFTFunc.OldDepth;				    
						}
		
						FunCodeUnion.code.AT_AdaptiveFilterMode = 0;    //退出自适应过程
		                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));
					}
					else
					{
					    FunCodeUnion.code.AT_NotchFiltFreqDisp = FFTFunc.Freq;    //显示检测的共振频率
					}
	
					FftProcessStep = FFT_PROCESS_STEP0;				    
				}
				else if(FFTFunc.Mag[0] >= STR_FUNC_Gvar.Fft.OscillateLevel)
				{
				    if((FFTFunc.Freq >= 100) && (FFTFunc.Freq <= 4000))
					{
	    			    AdaptiveQuitFlag = 0;	     //只要检测到共振点就不再退出
						AdaptiveQuitCnt  = 0;
						FftProcessStep   = FFT_PROCESS_STEP4;		  //满足要求后进入处理程序
	
						FFTFunc.Mag[1]                         = FFTFunc.Mag[0];		    //获取前次辨识结果幅值	
						STR_FUNC_Gvar.Fft.OscillateValue       = FFTFunc.Mag[0];
						FunCodeUnion.code.AT_NotchFiltFreqDisp = FFTFunc.Freq;    //显示检测的共振频率					    
					}
					else
					{
						FunCodeUnion.code.AT_AdaptiveFilterMode = 0;    //退出自适应过程
		                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_AdaptiveFilterMode));
		
						FftProcessStep = FFT_PROCESS_STEP0;					    
					}
				}
				else
				{
				    AdaptiveQuitCnt++;						  //退出次数累加
					STR_FUNC_Gvar.Fft.OscillateValue = 10; 

					if(AdaptiveQuitCnt > 20)		   //连续20次计算都没有合适的共振点，则允许退出
					{
					    AdaptiveQuitFlag = 1; 
					}

					FftProcessStep = FFT_PROCESS_STEP0;       //振幅或频率不合要求则重新开始采集				
				}									 					
			}
			else
			{
			    DelayCnt++;

				//计算时间需要控制在1s内
				if(DelayCnt >= STR_FUNC_Gvar.System.ToqFreq)
				{
			        DelayCnt 	   = 0;
			        FftProcessStep = FFT_PROCESS_STEP0;					    
				}
			}

			break;

      	case  FFT_PROCESS_STEP4:	//FFT运算结果处理阶段——计算深度和宽度

			//设置陷波器C和陷波器D的宽度
			if((FFTFunc.Freq >= 100) && (FFTFunc.Freq <200))		
            {
                WidthValue = 0;    //陷波宽度为0级
				DepthValue = 5;	   //抑制深度为5%
            }
            else if((FFTFunc.Freq >= 200) && (FFTFunc.Freq <300))
            {
                WidthValue = 1;    //陷波宽度为1级
				DepthValue = 2;	   //抑制深度为2%
            }
            else
            {
                WidthValue = 2;    //陷波宽度为2级,71%中心频率
				DepthValue = 1;	   //抑制深度为1%
            }
					 				
		    //将震动抑制到额定转矩的1%以内：(mag/4096)*(depth/100)=2/100;
		    NotchFilterDepth = (DepthValue * UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT) / (FFTFunc.Mag[0]);	

		    if(NotchFilterDepth >= 100)
		    {
		        NotchFilterDepth = 80;
		    }

		    FftProcessStep = FFT_PROCESS_STEP5;			    
		break;

		case  FFT_PROCESS_STEP5:	//FFT运算结果处理阶段——设置陷波器
							
			if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 1)       //如果是模式1 ，设置陷波器C
			{					
                NotchUpdateFlag = 1;                     
		    }
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 2)   //设置两个陷波器
			{
				//以C陷波器为主，D陷波器辅助动作
				//计算两个陷波器频率变化量
				DeltaFreqC = ABS((int16)FunCodeUnion.code.AT_NotchFiltFreqC - FFTFunc.Freq);
				DeltaFreqD = ABS((int16)FunCodeUnion.code.AT_NotchFiltFreqD - FFTFunc.Freq);
       
				if(FunCodeUnion.code.AT_NotchFiltFreqC == 4000)	    //更新陷波器C
				{
				    NotchUpdateFlag = 1;
				}
				else if(FunCodeUnion.code.AT_NotchFiltFreqD == 4000)   //判断更新哪个陷波器
				{
				    if(FFTFunc.Freq < FunCodeUnion.code.AT_NotchFiltFreqC) 
                    {    //频率变化小时更新陷波器C
                        if(((FFTFunc.Freq * SECONDPROP_Q10)>>10) > FunCodeUnion.code.AT_NotchFiltFreqC)
                        {
                            NotchUpdateFlag = 1; 
                        }
                        else
                        {
                            NotchUpdateFlag = 2;
                        }
                    }
                    else
                    {   //频率变化小时更新陷波器C
                        if(((FunCodeUnion.code.AT_NotchFiltFreqC* SECONDPROP_Q10)>>10) > FFTFunc.Freq)
                        {
                            NotchUpdateFlag = 1; 
                        }
                        else
                        {
                            NotchUpdateFlag = 2;
                        }                    
                    }
				}
				else
				{
				    if(DeltaFreqC < DeltaFreqD)   //新共振点离C近
					{
					    NotchUpdateFlag = 1;
					}
					else
					{
					    NotchUpdateFlag = 2;
					}
				}				                     
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 3) //测试共振频率，只显示
			{
				FunCodeUnion.code.AT_NotchFiltFreqDisp= FFTFunc.Freq;
		    }
			
			if(NotchUpdateFlag == 1)     //更新陷波器C
			{
			    //保存陷波器更新前的数值，用于振动变强后的复位
				FFTFunc.OldFreq	 = FunCodeUnion.code.AT_NotchFiltFreqC;
				FFTFunc.OldBandW = FunCodeUnion.code.AT_NotchFiltBandWidthC;
				FFTFunc.OldDepth = FunCodeUnion.code.AT_NotchFiltAttenuatLvlC;
				
				//根据当前频率和计算出的新频率变化比例设置合适的陷波器
			    if((int16)FunCodeUnion.code.AT_NotchFiltFreqC < FFTFunc.Freq)
			    {
			        BigFreq  = FFTFunc.Freq;
				    SmllFreq = (int16)FunCodeUnion.code.AT_NotchFiltFreqC;
			    }
			    else
			    {
			        BigFreq  = (int16)FunCodeUnion.code.AT_NotchFiltFreqC;
				    SmllFreq = FFTFunc.Freq;
			    }
                
				if(((SmllFreq * FIRSTPROP_Q10)>>10) > BigFreq) //频率变化小，只改变深度
			    {
				    //在原有深度基础上进行变化，变更为原深度的0.75倍
				    TempValue = (FunCodeUnion.code.AT_NotchFiltAttenuatLvlC * 3)>>2; 

				    FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = (Uint16)TempValue;
			    }                          								 
			    else if(((SmllFreq * SECONDPROP_Q10)>>10) > BigFreq) //频率变化一般，改变中心频率，深度不变 
			    {
				    FunCodeUnion.code.AT_NotchFiltFreqC = FFTFunc.Freq;
			    }                         						 
			    else     //频率变化较大，跟随频率并复位深度
			    {
				    FunCodeUnion.code.AT_NotchFiltFreqC = FFTFunc.Freq;
				    //陷波器初始值选择
					FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = NotchFilterDepth;								        
			    }				

			    FunCodeUnion.code.AT_NotchFiltBandWidthC = WidthValue;														    
			}		    
			else if(NotchUpdateFlag == 2)     //更新陷波器D
			{
			   //保存陷波器更新前的数值，用于振动变强后的复位
				FFTFunc.OldFreq	 = FunCodeUnion.code.AT_NotchFiltFreqD;
				FFTFunc.OldBandW = FunCodeUnion.code.AT_NotchFiltBandWidthD;
				FFTFunc.OldDepth = FunCodeUnion.code.AT_NotchFiltAttenuatLvlD;				
								
				if((int16)FunCodeUnion.code.AT_NotchFiltFreqD < FFTFunc.Freq)
			    {
			        BigFreq  = FFTFunc.Freq;
				    SmllFreq = (int16)FunCodeUnion.code.AT_NotchFiltFreqD;
			    }
			    else
			    {
			        BigFreq  = (int16)FunCodeUnion.code.AT_NotchFiltFreqD;
				    SmllFreq = FFTFunc.Freq;
			    }
                
				if(((SmllFreq * FIRSTPROP_Q10)>>10) > BigFreq) //频率变化小，只改变深度
			    {
				     //在原有深度基础上进行变化，变更为原深度的0.75倍
				    TempValue = (FunCodeUnion.code.AT_NotchFiltAttenuatLvlD * 3)>>2; 

				    FunCodeUnion.code.AT_NotchFiltAttenuatLvlD = (Uint16)TempValue;
			    }                          								 
			    else if(((SmllFreq * SECONDPROP_Q10)>>10) > BigFreq) //频率变化一般，改变中心频率，深度不变 
			    {
				    FunCodeUnion.code.AT_NotchFiltFreqD = FFTFunc.Freq;
			    }                         						 
			    else     //频率变化较大，跟随频率并复位深度
			    {
				    FunCodeUnion.code.AT_NotchFiltFreqD = FFTFunc.Freq; 
				    //陷波器初始值选择
					FunCodeUnion.code.AT_NotchFiltAttenuatLvlD = NotchFilterDepth;
			    }

			    FunCodeUnion.code.AT_NotchFiltBandWidthD = WidthValue;				    
			}
			
			FFTFunc.OldNtchUpdtFlag = NotchUpdateFlag;      //保存更新标志位，记录本周期更新了哪个陷波器								  
		     
			FftProcessStep= FFT_PROCESS_STEP0;
        break;

		default:
		    FftProcessStep = FFT_PROCESS_STEP0; 		    //直接跳转步骤里面再跳转到第1步
        break;      
    }	
} 

 
	   	
/*******************************************************************************
  函数名:  FftSample(int32 TorqCmdTemp)
  输入:   
          TorqCmdTemp转矩指令。
  输出:   1. FFTDataBuff.FftDataFuShu 采样数据
          2. AdaptiveSampleEnd 采样结束标志
  子函数:无         
  描述: 采样开启后，如果速度指令从0开始，则延时一段时间再开始采样；如果起始值非0，则直接开始采样。
        采样长度为1024点，采样率为电流环频率一半，采集对象为转矩指令。  
********************************************************************************/ 
void FftSample(int32 TorqCmdTemp)
{
	static Uint16 FftSampleCnt=0;    //用于数据采集计数
	int64  Temp;

	switch(FftSampleStep)
	{
	    case FFT_SAMPLE_STEP0: //伺服只要使能就采集
            if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == 2)
            {
                FFTFunc.DataSum = 0;
			    FftSampleCnt    = 0;
				FftSampleStep   = FFT_SAMPLE_STEP1;
            }
		    break;
        case FFT_SAMPLE_STEP1:		      	 //数据采集，必须要保证在使能状态下
            if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == 2)
			{
				if(FftSampleCnt<(FFTFunc.N))    //时域顺序存储，频域码序倒读计算
				{

				    Temp = ((int64)TorqCmdTemp * UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12)>>12; 
					
					FFTFunc.DataSum += Temp;			   //保存累加和
					FftArray[FftSampleCnt].Real = Temp;     //实部为转矩指令					

					//FFTFunc.DataSum += TestTable[FftSampleCnt];			   //保存累加和
					//FftArray[FftSampleCnt].Real = TestTable[FftSampleCnt];     //实部为转矩指令
					FftArray[FftSampleCnt].Imag = 0;			   //虚部为0

				    FftSampleCnt++;					
				}
				else
				{
					AdaptiveSampleStart = 0;
					AdaptiveSampleEnd = 1;
					FftSampleCnt = 0;
					FftSampleStep = FFT_SAMPLE_STEP0;
				}
			}
			else
			{				  				 
				AdaptiveSampleStart = 0;
				AdaptiveSampleEnd   = 0; 
				FftSampleStep = FFT_SAMPLE_STEP0;
		    }
		    break;
        default:
		    break;		 
	}
}
