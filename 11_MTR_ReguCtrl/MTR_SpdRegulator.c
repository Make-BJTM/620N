/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: MTR_SpdRegulator.c       //isvo test                                                         
 创建人：高小峰            创建日期：2008.10.31 
 修改人：朱祥华            修改日期：2011.10.09  
 描述： 
    1.电机模块速度调节文件源程序    
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */    
#include "MTR_SpdRegulator.h"   
#include "MTR_GlobalVariable.h"  
#include "MTR_InterfaceProcess.h"   
#include "MTR_Global_Filter.h"
#include "MTR_PDFFCtrl.h"
#include "PUB_Main.h"
#include "FUNC_AuxFunCode.h"
#include "FUNC_CSPCmdBuffer.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  

//姚虹新设计的转矩指令低通滤波器
STR_MTR_NEW_LOWPASS_FILTER    NewToqFdFwdFilter={0};
STR_MTR_NEW_LOWPASS_FILTER    NewToqRefLowpassFilter={0};
STR_MTR_NEW_LOWPASS_FILTER    NewDobLowpassFilter={0};

//wzg 速度环PDFF控制器
STR_PDFFCONTROLLER  Spd_PdffCtrl = PDFF_CONTROLLER_Defaults;   

//wq 开环频率特性分析虚拟控制器
STR_PDFFCONTROLLER  FSA_SpdRegulator = PDFF_CONTROLLER_Defaults; 

//速度调节器结构变量
STR_SPDREGULATOR   STR_SpdRegu = STR_SPDREGULATOR_DEFAULT;

//陷波器定义
STR_MTR_NotchFilter     NotchFilterA;
STR_MTR_NotchFilter     NotchFilterB;
STR_MTR_NotchFilter     NotchFilterC;
STR_MTR_NotchFilter     NotchFilterD;


//陷波器C/D更新标志
static Uint16 NotchFilterAUpdateFlag = 0;
static Uint16 NotchFilterBUpdateFlag = 0;
static Uint16 NotchFilterCUpdateFlag = 0;
static Uint16 NotchFilterDUpdateFlag = 0;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
 
//以下函数在上电启机初始化程序 G_MTR_SysInit_STAR()中调度执行 

void SpdReguStopUpdata(void);      //速度环停机更新 
void SpdReguUpdata(void);      //速度环运行更新生效功能码  
void SpdReguCoefUpdata(void);  //速度环调节器系数和转矩指令滤波系数更新

//以下函数在后台程序 G_MTR_Task_BKINT()（主循环）中调度执行
void SpdReguDatClr(void);     //速度环累积参数清除

//以下函数在时基中断程序（速度位置环调度）G_MTR_Task_TBINT()中调度执行
void Nomal_SpdSchedueMode(void);   //正常的速度环调度模式 

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline int32 ToqFeedForward(void);     //转矩前馈
Static_Inline int32 ToqCompensate(void);      //摩擦力过零补偿  
Static_Inline void ToqDisObserve(int32 IqCmd);  //转矩干扰观测器  

/*******************************************************************************
  函数名:  InitSpdRegulator()     ^_^  若更改此部分量纲请谨记同步更改伪速度调节器定标和惯量辨识处定标
  输入:   1.速度环调节器SpdCoef_Q24变量计算所需H00组电机、H01组驱动器、 
          2.速度环调用周期；
          3.转矩前馈滤波、转矩指令滤波所需参数。
  输出:   1.SpdCoef_Q24；
          2.换算后的速度环比例积分增益；
          3. 转矩前馈滤波所需换算参数
  子函数:无         
  描述: 启机初始化速度调节器用和转矩前馈滤波、转矩指令滤波用的相关功能
        G2新平台将速度环PI增益切换、PI控制方法切换、积分时间转换成积分增益和滤波系数更新等工作均移入FUN处理，
        然后作为快表接口于每个中断传送至MTR更新。MTR模块初始化和主循环中不需要再更新。
        既能保证更新的实时性也能保证速度调节器的纯洁性。
        该函数在上电启机初始化程序 G_MTR_SysInit_STAR()中调度执行    
********************************************************************************/ 
void InitSpdRegulator(void)
{
/*----------------------------------------------------------------------------------------------
  量纲转换系数 SpdCoef = 2*PI * Jmotor * (Jratio+100) *( 2*PI/60) * (1/Kt)* (IqRate_MT/In) * (1/100)
------------------------------------------------------------------------------------------------
  电机惯量     Jmotor -> STR_MTR_Gvar.FunCode.MT_Inertia        单位0.01㎏c㎡  分子乘10^(-6)
  惯量比       Jratio -> STR_MTR_Gvar.FunCode.GN_InertiaRatio   单位0.01       分子乘10^(-2)
  转矩系数     Kt     -> STR_MTR_Gvar.FunCode.MT_ToqCoe         单位0.01N/A    分母乘10^(-2)
  电机额定电流 In     -> STR_MTR_Gvar.FunCode.MT_RateCurrent    单位0.01A      分母乘10^(-2)
  速度内部定标 1/10000  -> 速度反馈内部                         单位0.0001rpm  分子乘10^(-4)
      4096/(SQRT2*In) -> 额定转矩对应数字量为4096,对应电流为SQRT2*IN
------------------------------------------------------------------------------------------------
  因此最后为10^(-8)数量级   最终速度环比例增益单位为0.1Hz   积分增益单位为0.1Hz
------------------------------------------------------------------------------------------------*/
    int64 Temp;

    /*若更改此部分量纲请谨记同步更改伪速度调节器定标和惯量辨识处定标*/
    //速度环定标系数改为Q32格式
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线电机处理
    {
        //速度指令改为四位小数0.0001mm/s
        STR_SpdRegu.SpdScalCoef_Q38 = (((Uint64)2 * PI_Q12 * STR_MTR_Gvar.GetIqRef.IqRate_MT) << 26) /1000000000L;
        STR_SpdRegu.SpdScalCoef_Q38 = STR_SpdRegu.SpdScalCoef_Q38 * FunCodeUnion.code.MT_Inertia;
        STR_SpdRegu.SpdScalCoef_Q38 = STR_SpdRegu.SpdScalCoef_Q38 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);
	//干扰观测器所用系数！    
		STR_SpdRegu.DobCoff_Q32 = ((Uint64)STR_MTR_Gvar.System.SpdFreq * STR_MTR_Gvar.GetIqRef.IqRate_MT) << 32;
		STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / 10000;			               //转速值带4个小数位
		STR_SpdRegu.DobCoff_Q32 = (STR_SpdRegu.DobCoff_Q32 * FunCodeUnion.code.MT_Inertia);	
		STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / 10000;
        STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);
    }
    else
    {
        //速度指令改为四位小数0.0001rpm
        STR_SpdRegu.SpdScalCoef_Q38 = (((Uint64)2 * PI_Q12 * STR_MTR_Gvar.GetIqRef.IqRate_MT) << 30) /100000000L;
        STR_SpdRegu.SpdScalCoef_Q38 = STR_SpdRegu.SpdScalCoef_Q38 * FunCodeUnion.code.MT_Inertia;
        STR_SpdRegu.SpdScalCoef_Q38 = ((STR_SpdRegu.SpdScalCoef_Q38 * 2 * PI_Q12) >> 16) / (60 * 10);  //H0800重新带一个小数点
        STR_SpdRegu.SpdScalCoef_Q38 = STR_SpdRegu.SpdScalCoef_Q38 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);

	//干扰观测器所用系数！    
		STR_SpdRegu.DobCoff_Q32 = ((Uint64)2 * PI_Q12 * STR_MTR_Gvar.System.SpdFreq * STR_MTR_Gvar.GetIqRef.IqRate_MT) << 20;
		STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / (60*10000L);			               //转速值带4个小数位
		STR_SpdRegu.DobCoff_Q32 = (STR_SpdRegu.DobCoff_Q32 * FunCodeUnion.code.MT_Inertia);	
		STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / 10000;
        STR_SpdRegu.DobCoff_Q32 = STR_SpdRegu.DobCoff_Q32 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);
    }

    //初始化计算一次变换系数，之后主循环中继续更新 //惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
    Spd_PdffCtrl.Kb_Scal_Q38 = STR_SpdRegu.SpdScalCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);    //惯量比

    FSA_SpdRegulator.Kb_Scal_Q38 = STR_SpdRegu.SpdScalCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);

    //转矩前馈增益参数转换系数  惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
    STR_SpdRegu.ToqFbScalCoef_Q38 = (Uint64)STR_MTR_Gvar.System.SpdFreq * STR_SpdRegu.SpdScalCoef_Q38 * 10 /1000;   //转矩前馈计算中不能考虑H0800所带的小数点，所以得乘上10，除以1000是由于H0821单位0.1%而引入的！
    STR_SpdRegu.ToqFbScalCoef_Q38 = STR_SpdRegu.ToqFbScalCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);    //惯量比
    STR_SpdRegu.ToqFbScalCoef_Q38 =  (STR_SpdRegu.ToqFbScalCoef_Q38 << 12)  / (2 * PI_Q12);
     
    //转矩前馈滤波初次赋值 之后主循环中继续更新
    NewToqFdFwdFilter.Fs = STR_MTR_Gvar.System.SpdFreq;     
    if(0 == FunCodeUnion.code.GN_ToqFbFltrT)
    {
        NewToqFdFwdFilter.Fc = NewToqFdFwdFilter.Fs;
    }
    else
    {
        NewToqFdFwdFilter.Fc = ((Uint32)100000 << 11) / ((Uint32)FunCodeUnion.code.GN_ToqFbFltrT * PI_Q12);    
    }
    MTR_InitNewLowPassFilt(&NewToqFdFwdFilter);


    //姚虹新设计转矩指令低通滤波器初始化
    NewToqRefLowpassFilter.Fs = STR_MTR_Gvar.System.SpdFreq;     //转矩滤波时间初始化

    if(UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc != 0)
    {
        NewToqRefLowpassFilter.Fc = ((Uint32)1000000 << 11)
                                    / ((Uint32)UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc * PI_Q12);
    }
    else
    {
        NewToqRefLowpassFilter.Fc = NewToqRefLowpassFilter.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
    }

    NewToqRefLowpassFilter.LastFc = NewToqRefLowpassFilter.Fc;
    MTR_InitNewLowPassFilt(&NewToqRefLowpassFilter);

    //陷波器初始化，H09组的前两组陷波器使用新的模型
    MTR_NewNotchFilterInit(&NotchFilterA, MTR_NOTCHFILTERA);
    MTR_NewNotchFilterInit(&NotchFilterB, MTR_NOTCHFILTERB);
    MTR_NewNotchFilterInit(&NotchFilterC, MTR_NOTCHFILTERC);
    MTR_NewNotchFilterInit(&NotchFilterD, MTR_NOTCHFILTERD);

    STR_SpdRegu.SpdRefInLatch = 0;                     //速度指令旧值
	STR_SpdRegu.ToqCompTemp = 0;
    //在线摩擦辨识惯量计算系数初始化,使用Te=J×a+Td来计算，最小二乘法，
    //Coff = (4096×2pi)×(Jm×100/Tn)×(fs/(60×10000));
    //其中10000表示速度反馈小数位数，100表示额定转矩带两个小数位
    //系数没有考虑惯量的小数位是为了使系数为整数
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //直线电机处理
	{
	    Temp = (int64)STR_MTR_Gvar.System.SpdFreq * 100;     //此处的100为额定转矩中的小数位
        Temp = (Temp * FunCodeUnion.code.MT_Inertia) / 10000;   //此处10000为速度中的小数位,16表示速度滤波次数
        Temp = (Temp * STR_MTR_Gvar.GetIqRef.IqRate_MT) / FunCodeUnion.code.MT_RateToq;
    
        STR_MTR_Gvar.InFricId.Coff_J = (int32)(Temp >> 12);	
	}
	else
	{	
	    Temp = (int64)STR_MTR_Gvar.System.SpdFreq * 100 * 2 * PI_Q12;   //此处的100为额定转矩中的小数位
        Temp = (Temp * FunCodeUnion.code.MT_Inertia) / 10000;   //此处10000为速度中的小数位,16表示速度滤波次数
        Temp = (Temp * STR_MTR_Gvar.GetIqRef.IqRate_MT) / (FunCodeUnion.code.MT_RateToq * 60);
    
        STR_MTR_Gvar.InFricId.Coff_J = (int32)(Temp >> 12);
	}
}

/*******************************************************************************
  函数名:  SpdReguStopUpdata()     ^_^
  输入:            
  输出:            
  子函数:无         
  描述: 
         
********************************************************************************/ 
void SpdReguStopUpdata(void)
{ 
    //两组陷波器初始化，H09组的四个陷波器使用最新的陷波器
    MTR_NewNotchFilterInit(&NotchFilterA, MTR_NOTCHFILTERA);
    MTR_NewNotchFilterInit(&NotchFilterB, MTR_NOTCHFILTERB);
    MTR_NewNotchFilterInit(&NotchFilterC, MTR_NOTCHFILTERC);
    MTR_NewNotchFilterInit(&NotchFilterD, MTR_NOTCHFILTERD);
	STR_SpdRegu.ToqCompTemp = 0;       
}

/*******************************************************************************
  函数名:  SpdRunUpdata()     ^_^
  输入:   1.速度环调节器STR_SpdRegu.KSpdCoef_Q24变量计算所需惯量比和速度相关增益功能码；
          2.转矩前馈滤波所需参数。
  输出:   1.速度环增益转换系数STR_SpdRegu.KSpdCoef_Q24；
          2. 转矩前馈滤波所需换算参数
  子函数:无         
  描述: 运行更新速度调节器用和转矩前馈滤波用的相关参数
        速度环PI增益切换、PI控制方法切换、积分时间转换成积分增益和滤波系数更新等工作均移入FUN处理，
        然后作为快表接口于每个中断传送至MTR更新。MTR模块初始化和主循环中不需要再更新。
        既能保证更新的实时性也能保证速度调节器的纯洁性
        该函数在后台程序 G_MTR_Task_BKINT(void)（主循环）中调度执行  
********************************************************************************/ 
void SpdReguUpdata(void)   
{
    static 	Uint16 NotchFiltFreqAOld = 0;
	static 	Uint16 NotchFiltBandWidthAOld = 0;
	static 	Uint16 NotchFiltDepthAOld = 0;
    static 	Uint16 NotchFiltFreqBOld = 0;
	static 	Uint16 NotchFiltBandWidthBOld = 0;
	static 	Uint16 NotchFiltDepthBOld = 0;
    static 	Uint16 NotchFiltFreqCOld = 0;
	static 	Uint16 NotchFiltBandWidthCOld = 0;
	static 	Uint16 NotchFiltDepthCOld = 0;
    static 	Uint16 NotchFiltFreqDOld = 0;
	static 	Uint16 NotchFiltBandWidthDOld = 0;
	static 	Uint16 NotchFiltDepthDOld = 0;
    Uint64 Temp64 = 1L;
	
	
	//更改功能码H0815惯量比后更新速度环转换系数  //惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
    Spd_PdffCtrl.Kb_Scal_Q38 = STR_SpdRegu.SpdScalCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);

    FSA_SpdRegulator.Kb_Scal_Q38 = Spd_PdffCtrl.Kb_Scal_Q38;
    Temp64 = Temp64 << 38;
    Temp64 = Temp64 / (Spd_PdffCtrl.Kb_Scal_Q38 * FunCodeUnion.code.GN_Spd_Kp);
    if (Temp64 > 30000L)
    {
        Temp64 = 30000L;
    }
    AuxFunCodeUnion.code.FS_SpdRefAmpltd = (Uint16)Temp64;

    //转矩前馈增益参数转换系数  惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
    STR_SpdRegu.ToqFbScalCoef_Q38 = (Uint64)STR_MTR_Gvar.System.SpdFreq * STR_SpdRegu.SpdScalCoef_Q38 * 10 /1000;   //转矩前馈计算中不能考虑H0800所带的小数点，所以得乘上10，除以1000是由于H0821单位0.1%而引入的！
    STR_SpdRegu.ToqFbScalCoef_Q38 = STR_SpdRegu.ToqFbScalCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);    //惯量比
    STR_SpdRegu.ToqFbScalCoef_Q38 =  (STR_SpdRegu.ToqFbScalCoef_Q38 << 12)  / (2 * PI_Q12);

    //更新观测器参数
    STR_SpdRegu.DobCoffUpdt_Q32 = STR_SpdRegu.DobCoff_Q32 * (FunCodeUnion.code.GN_InertiaRatio + 100L);

    NewToqFdFwdFilter.Fs = STR_MTR_Gvar.System.SpdFreq;     
    if(0 == FunCodeUnion.code.GN_ToqFbFltrT)
    {
        NewToqFdFwdFilter.Fc = NewToqFdFwdFilter.Fs;
    }
    else
    {
        NewToqFdFwdFilter.Fc = ((Uint32)100000 << 11) / ((Uint32)FunCodeUnion.code.GN_ToqFbFltrT * PI_Q12);    
    }
    MTR_InitNewLowPassFilt(&NewToqFdFwdFilter);

//    //姚虹新设计转矩指令低通滤波器初始化
//    NewToqRefLowpassFilter.Fs = STR_MTR_Gvar.System.SpdFreq;     //转矩滤波时间初始化
//
//    if(UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc != 0)
//    {
//        NewToqRefLowpassFilter.Fc = ((Uint32)1000000 << 11)
//                                    / ((Uint32)UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc * PI_Q12);
//    }
//    else
//    {
//        NewToqRefLowpassFilter.Fc = NewToqRefLowpassFilter.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
//    }
//    
//    if(NewToqRefLowpassFilter.Fc != NewToqRefLowpassFilter.LastFc)
//    {
//        NewToqRefLowpassFilter.LastFc = NewToqRefLowpassFilter.Fc;
//        MTR_InitNewLowPassFilt(&NewToqRefLowpassFilter);
//    }
	
    //干扰观测器滤波器初始化
	NewDobLowpassFilter.Fs = STR_MTR_Gvar.System.SpdFreq;          //干扰观测器滤波器初始化

	if(FunCodeUnion.code.AT_DobFiltTime != 0)
	{  
	    NewDobLowpassFilter.Fc = ((Uint32)100000 << 11) / ((Uint32)FunCodeUnion.code.AT_DobFiltTime * PI_Q12);
	}
	else
	{
	    NewDobLowpassFilter.Fc = NewDobLowpassFilter.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
	}

    if(NewDobLowpassFilter.Fc != NewDobLowpassFilter.LastFc)	  //判定滤波时间是否发生改变
    { 
        NewDobLowpassFilter.LastFc = NewDobLowpassFilter.Fc;
        MTR_InitNewLowPassFilt(&NewDobLowpassFilter);
    }



    
    //手动设置陷波器初始化
	if((NotchFiltFreqAOld != FunCodeUnion.code.AT_NotchFiltFreqA)  \
	  || (NotchFiltBandWidthAOld != FunCodeUnion.code.AT_NotchFiltBandWidthA) \
      ||(NotchFiltDepthAOld != FunCodeUnion.code.AT_NotchFiltAttenuatLvlA))
	{
	    NotchFilterAUpdateFlag=1;
        MTR_NewNotchFilterInit(&NotchFilterA, MTR_NOTCHFILTERA);	 
		NotchFilterAUpdateFlag=0; 
	}

	if((NotchFiltFreqBOld != FunCodeUnion.code.AT_NotchFiltFreqB)  \
	  || (NotchFiltBandWidthBOld != FunCodeUnion.code.AT_NotchFiltBandWidthB) \
      || (NotchFiltDepthBOld != FunCodeUnion.code.AT_NotchFiltAttenuatLvlB))
	{
	    NotchFilterBUpdateFlag=1;
        MTR_NewNotchFilterInit(&NotchFilterB, MTR_NOTCHFILTERB);
	    NotchFilterBUpdateFlag=0; 
	}

    NotchFiltFreqAOld      = FunCodeUnion.code.AT_NotchFiltFreqA;
	NotchFiltBandWidthAOld = FunCodeUnion.code.AT_NotchFiltBandWidthA;
    NotchFiltDepthAOld     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlA;
    NotchFiltFreqBOld      = FunCodeUnion.code.AT_NotchFiltFreqB;
	NotchFiltBandWidthBOld = FunCodeUnion.code.AT_NotchFiltBandWidthB;
    NotchFiltDepthBOld     =FunCodeUnion.code.AT_NotchFiltAttenuatLvlB;
    
    //自适应滤波器C/D参数更新
	if((NotchFiltFreqCOld != FunCodeUnion.code.AT_NotchFiltFreqC)  \
	  || (NotchFiltBandWidthCOld != FunCodeUnion.code.AT_NotchFiltBandWidthC) \
      ||(NotchFiltDepthCOld != FunCodeUnion.code.AT_NotchFiltAttenuatLvlC))
	{
	    NotchFilterCUpdateFlag=1;
        MTR_NewNotchFilterInit(&NotchFilterC, MTR_NOTCHFILTERC);	 
		NotchFilterCUpdateFlag=0; 
	}

	if((NotchFiltFreqDOld != FunCodeUnion.code.AT_NotchFiltFreqD)  \
	  || (NotchFiltBandWidthDOld != FunCodeUnion.code.AT_NotchFiltBandWidthD) \
      || (NotchFiltDepthDOld != FunCodeUnion.code.AT_NotchFiltAttenuatLvlD))
	{
	    NotchFilterDUpdateFlag=1;
        MTR_NewNotchFilterInit(&NotchFilterD, MTR_NOTCHFILTERD);
	    NotchFilterDUpdateFlag=0; 
	}

	NotchFiltFreqCOld      = FunCodeUnion.code.AT_NotchFiltFreqC;
	NotchFiltBandWidthCOld = FunCodeUnion.code.AT_NotchFiltBandWidthC;
	NotchFiltFreqDOld      = FunCodeUnion.code.AT_NotchFiltFreqD;
	NotchFiltBandWidthDOld = FunCodeUnion.code.AT_NotchFiltBandWidthD;
    NotchFiltDepthCOld     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlC;
    NotchFiltDepthDOld     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlD;
}

/*******************************************************************************
  函数名:  SpdReguDatClr()   ^_^
  输入:    
  输出:   速度调节器相关变量（有累积、记忆特性变量） 
  子函数:无         
  描述: 将速度调节器的变量清零，以防调节器停止后再次
        启动时有前期数据的累积、记忆（谨记清零）。 停机清零 
        该函数在后台程序 G_MTR_Task_BKINT(void)（主循环）中调度执行
********************************************************************************/ 
void SpdReguDatClr(void)
{
     if (AuxFunCodeUnion.code.H2F_FSAState == 2)
     {
        Nomal_SpdSchedueMode ();
     }
     else
     {
        ResetPDFFCtrl(&Spd_PdffCtrl);                      //wzg PDFF控制器复位

        //姚虹设计转矩滤波器复位
        MTR_ResetNewLowPassFilter(&NewToqRefLowpassFilter);
    
        MTR_ResetNewLowPassFilter(&NewToqFdFwdFilter);           //wzg 转矩前馈低通滤波器复位
        
        //陷波器清零
        MTR_CLR_AllNotchFilter(&NotchFilterA);
        MTR_CLR_AllNotchFilter(&NotchFilterB);
        MTR_CLR_AllNotchFilter(&NotchFilterC);
        MTR_CLR_AllNotchFilter(&NotchFilterD);
    
        STR_MTR_Gvar.SpeedRegulator.SpdReguOut = 0;        //wzg输出速度调节器输出置零,伺服未运行
        STR_SpdRegu.ToqCompTemp = 0;					   //摩擦补偿中间值清零
    
        STR_SpdRegu.SpdRefInLatch = 0;                     //速度指令旧值
        STR_MTR_Gvar.FPGA.SpdFdbLast = STR_MTR_Gvar.FPGA.SpdFdb;
     }  
     ResetPDFFCtrl(&FSA_SpdRegulator);                      //wq 虚拟控制器复位
}


/*******************************************************************************
  函数名:  Nomal_SpdSchedueMode()       ^_^
  输入:   1.功能模块输入转速指令STR_MTR_Gvar.FUNCtoMTR.SpdRef
          2.运行状态位STR_MTR_Gvar.PUBFlg.bit.SERVORUNSTATUS
            PWM状态运行STR_MTR_Gvar.PUBFlg.bit.PWMSTATUSFLG
            运行模式STR_MTR_Gvar.PUBFlg.bit.RUNMOD
          3.转矩前馈增益值STR_MTR_Gvar.FunCode.GN_ToqFb_Kp 
          4.正反向Q轴电流指令限幅值STR_MTR_Gvar.GetIqRef.IqPosLmt
                STR_MTR_Gvar.GetIqRef.IqNegLmt
  输出:   经速度调节器和转矩前馈滤波后输出值 STR_MTR_Gvar.SpdReguOut
  子函数:  SpdRegulator() 
          ToqFeedForward()        
  描述:  为保证MTRSystem调度程序的干净和MTR_SpdRegulator.c文件的内敛性，
         将速度环调度内函数的调用也移至MTR_SpdRegulator.c文件
         其它超级模式如惯量辨识速度环的调度也放在对应的文件内。
         在电流调节器前端转矩指令限幅STR_MTR_Gvar.GetIqRef.IqPosLmt之前的所有输出限幅均限制在32767内
         该函数在时基中断程序（速度位置环调度）G_MTR_Task_TBINT(void)中调度执行
********************************************************************************/ 
void Nomal_SpdSchedueMode(void)
{
    int64  SpeedOutTmp = 0;

    //以下速度调节器运行时间为170/120 us
    Spd_PdffCtrl.PosLmt = STR_MTR_Gvar.GetIqRef.IqPosLmt;
    Spd_PdffCtrl.NegLmt = STR_MTR_Gvar.GetIqRef.IqNegLmt;
  
    FSA_SpdRegulator.PosLmt = Spd_PdffCtrl.PosLmt;
    FSA_SpdRegulator.NegLmt = Spd_PdffCtrl.NegLmt;

    if (1 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OpenFSAEn)
    {
        SpeedOutTmp = PDFFCtrl(&FSA_SpdRegulator, 0,\
                                STR_MTR_Gvar.FPGA.M_SpdFdbFlt, STR_MTR_Gvar.FPGA.M_SpdFdb);	
        STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOut = SpeedOutTmp;
    }
  
    if(FunCodeUnion.code.GN_SpdFbSel == 0)
	{
        if (AuxFunCodeUnion.code.H2F_FSAState == 2)
        {
            SpeedOutTmp = PDFFCtrl(&Spd_PdffCtrl,UNI_MTR_FUNCToMTR_List_16kHz.List.FSASpdCmd,\
                                0, 0);
        }
        else
        {
            SpeedOutTmp = PDFFCtrl(&Spd_PdffCtrl, UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef,\
                                STR_MTR_Gvar.FPGA.SpdFdb, STR_MTR_Gvar.FPGA.M_SpdFdb);
        }
	}
	else     //比例部分也使用M法测速值
	{
        if (AuxFunCodeUnion.code.H2F_FSAState == 2)
        {
            SpeedOutTmp = PDFFCtrl(&Spd_PdffCtrl,UNI_MTR_FUNCToMTR_List_16kHz.List.FSASpdCmd,\
                                0, 0);	
        }
        else
        {
            SpeedOutTmp = PDFFCtrl(&Spd_PdffCtrl, UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef,\
                                STR_MTR_Gvar.FPGA.M_SpdFdbFlt, STR_MTR_Gvar.FPGA.M_SpdFdb);	
        }
	}

    //转矩前馈   启用时为170/120 us
    SpeedOutTmp += ToqFeedForward();
       
    //摩擦力过零补偿+转矩观测器+限幅1 启用时为168/120 us
    SpeedOutTmp += ToqCompensate(); 

    //转矩指令获取处理，此处有一限幅，进行陷波器低通滤波后最后转矩指令赋值时又一限幅，
    //此处限幅主要为防输出值过大滤波退出饱和慢,但此处滤波运算会有量化误差影响中间环节处理
    //（如转矩指令到达信号处理）.   因此对前此处限幅进行略微放大处理


    //转矩指令低通滤波 74/120 us

    NewToqRefLowpassFilter.Input = (SpeedOutTmp << 10);
    MTR_NewLowPassFiltCalc(&NewToqRefLowpassFilter);
    SpeedOutTmp = (NewToqRefLowpassFilter.Output >> 10);

	ToqDisObserve(STR_MTR_Gvar.GetIqRef.IqRef);     //转矩观测器,抑制外力扰动,使用上周期的Iq值
	
    if (0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OpenFSAEn)
    {
        SpeedOutTmp += STR_SpdRegu.ToqDisturb;     //加入转矩观测值
    }

    STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOutPlusChirp = STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOut + UNI_MTR_FUNCToMTR_List_16kHz.List.FSAIqCmd;
    STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOut = STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOutPlusChirp + STR_SpdRegu.ToqDisturb;

    //陷波器滤波处理
    //单个运行最长时间 330/120us
    //4个运行最长时间 1200/120us
    if(NotchFilterAUpdateFlag != 1)
    {
        if(FunCodeUnion.code.AT_NotchFiltFreqA != 4000)  //陷波频率为上限时不进行陷波
        {
            NotchFilterA.NotchInput[0] = SpeedOutTmp;
            MTR_NotchFilter(&NotchFilterA);
            SpeedOutTmp = NotchFilterA.NotchOutput[0];
        }          
    }

    if(NotchFilterBUpdateFlag != 1)
    {
        if(FunCodeUnion.code.AT_NotchFiltFreqB != 4000) //陷波频率为上限时不进行陷波
        {
            NotchFilterB.NotchInput[0] = SpeedOutTmp;
            MTR_NotchFilter(&NotchFilterB);
            SpeedOutTmp = NotchFilterB.NotchOutput[0];
        }        
    }

    if(NotchFilterCUpdateFlag != 1)
	{
        if(FunCodeUnion.code.AT_NotchFiltFreqC != 4000) 
	    {                                                          
	        NotchFilterC.NotchInput[0] = SpeedOutTmp;
            MTR_NotchFilter(&NotchFilterC);
            SpeedOutTmp = NotchFilterC.NotchOutput[0];     
	    }
	}

    if(NotchFilterDUpdateFlag != 1)
	{
	    if(FunCodeUnion.code.AT_NotchFiltFreqD != 4000)  //陷波频率为上限时不进行陷波
        {
            NotchFilterD.NotchInput[0] = SpeedOutTmp;
            MTR_NotchFilter(&NotchFilterD);
            SpeedOutTmp = NotchFilterD.NotchOutput[0];   
        }
    }

    //输出限幅处理，饱和遇限处理
    if((int32)SpeedOutTmp >= Spd_PdffCtrl.PosLmt) 
    {
		SpeedOutTmp =  Spd_PdffCtrl.PosLmt;
		Spd_PdffCtrl.SaturaFlag = 2;
    }
    else if ((int32)SpeedOutTmp <= Spd_PdffCtrl.NegLmt) 
    {
		SpeedOutTmp =  Spd_PdffCtrl.NegLmt;
		Spd_PdffCtrl.SaturaFlag = 1;
    }
	else
	{
	    Spd_PdffCtrl.SaturaFlag = 0;
	}

    STR_MTR_Gvar.SpeedRegulator.SpdReguOut = (int32)SpeedOutTmp;
}

/*******************************************************************************
  函数名:  Static_Inline ToqFeedForward()      ^_^
  输入:   1.转矩前馈增益，转矩前馈滤波相关参数。
          2.速度调节器前端速度指令当前值和旧值
  输出:   1.速度前馈输出STR_SpdRegu.ToqFdFwd_Out
  子函数:无         
  描述:  包括转矩前馈（速度微分）和转矩前馈输出值滤波
         该函数在正常需要速度控制Nomal_SpdSchedueMode()中、转矩模式下的速度限制伪调节器SpdRegulator()中调用   
********************************************************************************/ 
Static_Inline int32 ToqFeedForward(void)
{
    int32 ToqFdFwd_err = 0;
    int64 ToqFdFwd_OutTmp = 0;

    if(FunCodeUnion.code.SL_SpdForwardChooseFlag == 0) 
    {
        return 0;
    }
    else if(FunCodeUnion.code.SL_SpdForwardChooseFlag == 1)
    {
        ToqFdFwd_err = UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef - STR_SpdRegu.SpdRefInLatch;
        STR_SpdRegu.SpdRefInLatch = UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef;

        //输出值计算
        ToqFdFwd_OutTmp = (int64)ToqFdFwd_err * (int32)FunCodeUnion.code.GN_ToqFb_Kp;

        ToqFdFwd_OutTmp = (ToqFdFwd_OutTmp * STR_SpdRegu.ToqFbScalCoef_Q38) >> 38;

        //前馈输出值限幅  #define MAX_MIN_LMT(A,Pos,Neg)  MAX(MIN(A,Pos),Neg) 上下限幅语句
        ToqFdFwd_OutTmp = MAX_MIN_LMT(ToqFdFwd_OutTmp,32767,-32767);

        NewToqFdFwdFilter.Input = (int32)ToqFdFwd_OutTmp;
        //转矩前馈低通滤波
        MTR_NewLowPassFiltCalc(&NewToqFdFwdFilter);

        return(NewToqFdFwdFilter.Output);
        
    }
    #if ECT_ENABLE_SWITCH

    else
    {
        if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTCSPMOD)
        {
			ToqFdFwd_OutTmp = (((int64)((int16)STR_CmdBufferVar.CSP_ToqRef)) * ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12))>>12;

        }
		else if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTCSVMOD)
		{
			ToqFdFwd_OutTmp =(((int64)((int16)ObjectDictionaryStandard.CstOffset.ToqOff)) * ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12))>>12;
			
			if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x20)== 0x20)
        	{
        	    ToqFdFwd_OutTmp = 0 - ToqFdFwd_OutTmp;
        	}
		}
        else
        {
    	    ToqFdFwd_OutTmp = 0;
        }

        //前馈输出值限幅  #define MAX_MIN_LMT(A,Pos,Neg)  MAX(MIN(A,Pos),Neg) 上下限幅语句
        ToqFdFwd_OutTmp = MAX_MIN_LMT(ToqFdFwd_OutTmp,32767,-32767);
        return(ToqFdFwd_OutTmp);
    }
    #endif
    
}


/*******************************************************************************
  函数名: 
  输入:   
          
  输出:   
  子函数:         
  描述: 转矩干扰观测器，用来抑制外力扰动！ 
           
********************************************************************************/
Static_Inline void ToqDisObserve(int32 IqCmd)
{
    int32  AccIq;      //计算出实际的加速转矩

	STR_SpdRegu.DeltaSpd = STR_MTR_Gvar.FPGA.SpdFdb - STR_MTR_Gvar.FPGA.SpdFdbLast;

	STR_MTR_Gvar.FPGA.SpdFdbLast = STR_MTR_Gvar.FPGA.SpdFdb;   //上周期速度反馈

	AccIq = (STR_SpdRegu.DobCoffUpdt_Q32 * STR_SpdRegu.DeltaSpd) >> 32;
	
	NewDobLowpassFilter.Input = IqCmd - AccIq;
    MTR_NewLowPassFiltCalc(&NewDobLowpassFilter); 
    STR_SpdRegu.ToqDisturb = NewDobLowpassFilter.Output;   	   //得到的干扰观测器估测转矩

	STR_SpdRegu.ToqDisturb = (STR_SpdRegu.ToqDisturb * FunCodeUnion.code.AT_ToqDisKp) >> 10;
}

/*******************************************************************************
  函数名: 
  输入:   
          
  输出:   
  子函数:         
  描述: 摩擦力补偿函数 
           
********************************************************************************/
Static_Inline int32 ToqCompensate(void)
{
	//只在位置模式下进行摩擦补偿
	if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.RunMod == POSMOD)
	{
		if(UNI_MTR_FUNCToMTR_List_16kHz.List.PosRef > 0)      //正向指令时补偿值
		{
		    if(UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef > 10000) //在转速指令过零后加入补偿
			{
			 	STR_SpdRegu.ToqCompTemp = (int32)((int16)FunCodeUnion.code.AT_ConstToqComp 
				                                   + (int16)FunCodeUnion.code.AT_ToqPlusComp);
			}
		}
		else if(UNI_MTR_FUNCToMTR_List_16kHz.List.PosRef < 0)   //反向指令时补偿值
		{
		    if(UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef < -10000)
			{	
				STR_SpdRegu.ToqCompTemp = (int32)((int16)FunCodeUnion.code.AT_ConstToqComp 
			                                      + (int16)FunCodeUnion.code.AT_ToqMinusComp);
			}		    
		}
	}
	else 
	{
	    STR_SpdRegu.ToqCompTemp = 0;
	}

	//将百分比转换成数字量	
	STR_SpdRegu.ToqCompensateValue = (STR_SpdRegu.ToqCompTemp * STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12) >> 12;

    return STR_SpdRegu.ToqCompensateValue;
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void SpdReguCoefUpdata(void)
{
    Spd_PdffCtrl.Kf_Q12 = UNI_MTR_FUNCToMTR_List_4Hz_32Bits.List.SpdKf_Q12;      //前馈系数
	//速度闭环中的运算为Fdb(1+DampingKf)，为节省资源提前在主循环中运算好1+DampingKf     ((0-100%)+1)<<12
	Spd_PdffCtrl.DampingKfPlus1_Q12 = UNI_MTR_FUNCToMTR_List_4Hz_32Bits.List.SpdDampingKf_Q12 + 4096L;      

    Spd_PdffCtrl.Kp = UNI_MTR_FUNCToMTR_List_16kHz.List.Spd_Kp;                  //比例系数
    Spd_PdffCtrl.Ki_Q10 = UNI_MTR_FUNCToMTR_List_16kHz.List.Spd_KiQ10;           //积分系数

    FSA_SpdRegulator.Kf_Q12 = 4096;      //前馈系数
    FSA_SpdRegulator.DampingKfPlus1_Q12 = 4096L;
    FSA_SpdRegulator.Kp = 250;                  //比例系数
    FSA_SpdRegulator.Ki_Q10 = 502;           //积分系数

    //姚虹新设计转矩指令低通滤波器初始化
    if(UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc != 0)
    {
        NewToqRefLowpassFilter.Fc = ((Uint32)1000000 << 11)
                                    / ((Uint32)UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc * PI_Q12);
    }
    else
    {
        NewToqRefLowpassFilter.Fc = NewToqRefLowpassFilter.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
    }
    
    if(NewToqRefLowpassFilter.Fc != NewToqRefLowpassFilter.LastFc)
    {
        NewToqRefLowpassFilter.LastFc = NewToqRefLowpassFilter.Fc;
        MTR_InitNewLowPassFilt(&NewToqRefLowpassFilter);
    }
}
/********************************* END OF FILE *********************************/



