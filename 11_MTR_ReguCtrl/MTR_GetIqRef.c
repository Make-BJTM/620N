/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.    ^_^                
 文件名:    MTR_GetIqRef.c                                                              
 创建人：童文邹            创建日期：2008.09.02  
 修改人：朱祥华            修改日期：2011.10.28  
 描述： 
①转矩调节相关处理：转矩指令最大限幅值IqPoaMax(IqNegMax)，电流采样转换系数CurSampleCoef_Q10，软件过流点CriticalCurent；
②转矩指令的获取处理（包括转矩模式下速度限制和急停处理）；
③转矩指令的限幅处理；
④转矩指令滤波；
⑤软件过流报错处理（er200）；
⑥转矩到达DO信号输出处理。

 修改记录：  
    1. 2011.10.28      朱祥华  
       变更内容： STR_MTR_Gvar.CurSampleCoef_Q10电流模拟量与数字量转换系数此系数由于112与TI不一致，
          故此系数的计算TI挪至电流环调节器文件，112挪至FPGA文件内。
    2. 2011.10.05      朱祥华
       变更内容：转矩模式下的速度限制。早期版本为伪速度调节器与转矩指令输入取最小值作为转矩环输入，
          该版本当速度增益偏小时切入伪速度调节器的时机延时很多，导致速度经常限不住。
          后期姚虹版本以速度反馈为条件，仅当速度反馈超过限制速度H0719时即切入伪速度调节器，不再切回转矩环。
          考虑此功能市场应用主要为保证：①限制速度下运行转矩恒定；②达到限制速度时即先保证速度低于限制速度；
          ③在保证前面两条的前提下，可以在恒转矩模式和速度限制调节模式下自由“平滑”切换。
          因此改进该功能以速度反馈作为判断标准，且加入判断条件滞环，仅当速度连续X次超过限制速度才切入伪速度调节。
          且切入时刻的以转矩指令IqIn作为伪调节器的输出初始值（保证切换平滑）。
          仅当速度连续X次低于限制速度且调节器输出第一次等于转矩指令IqIn时（保证切换平滑）才切回转矩模式。
    3. 2011.10.06      朱祥华
       变更内容：转矩到达DO输出112#增加该功能，原来TI平台2.80版本中在电流环中调度，
          调度判断次数为40次，因DIDO的最后读取处理也是在主循环中执行的。所以后将其改至主循环中调度，
          主循环调度周期为1K，电流环为16K，在此设计判断次数暂定为6次。增加输出功能码FUNOUT18，转矩到达DO输出功能。
          新增H07.21，转矩到达基准值；新增H07.22，转矩到达有效值；新增H07.23，转矩到达无效值。
          当目前的转矩指令的绝对值-H07.21>=H07.22时，FUNOUT.18对应的DO输出有效。同理当目前的转矩指令的绝对值-H07.21<H07.23时,
          FUNOUT.18对应的DO输出无效。三个功能码单位为0.1%电机额定转矩，范围0~300.0%。
********************************************************************************/



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */    
#include "MTR_GetIqRef.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h" 
#include "MTR_Global_Filter.h"
#include "MTR_PDFFCtrl.h"          //转矩环中的伪速度调节器调用的速度调节(类似于滤波器的结构)

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define  SOFT_OVER_CURENT1  320L     //软件过流100%点 -> 3.2倍驱动器电流幅值
#define  SOFT_OVER_CURENT2  450L     //软件过流100%点 -> 4.5倍驱动器电流幅值

#define IQMAX       3000          //H0709，H0710，H0711，H0712上限
#define IQMAX1       3600          //针对400W和200W电机
//400W T_I曲线拟合  额定转矩2.8 变形公式=0.186636*10^(-3)*X^2+0.67355*X+128
#define M400WFAQ25  6262          //0.193185 Q10  
#define M400WFBQ15  22071         //0.67355 
#define M400WFC     128           //固定常数 
#define M400WJOINT  1160          //二次曲线和直线相交点 593和1156
//200W T_I曲线拟合  变形公式=0.30458*10^(-3)*X^2+0.28819*X+409
#define M200WFAQ25  10220        //0.31189 Q10  
#define M200WFBQ15  9443         //0.6808 
#define M200WFC     409          //固定常数   
#define M200WJOINT  1320         //二次曲线和直线相交点 1018和1318
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无



/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
STR_GET_IQ_REF  STR_GetIqRef;    //Q轴电流指令获取文件专用结构体

//伪速度调节器中的速度调节器变量的定义(类似于滤波器的那种结构方式)
STR_PDFFCONTROLLER     PseudoSpeedRegulator = PDFF_CONTROLLER_Defaults;

//定义转矩模式下转矩指令低通滤波器
STR_MTR_NEW_LOWPASS_FILTER    NewToqRefLowpassFilter_ToqMod={0};

//定义转矩模式下四组陷波器
//陷波器定义
STR_MTR_NotchFilter     NotchFilterA_ToqMod;
STR_MTR_NotchFilter     NotchFilterB_ToqMod;
STR_MTR_NotchFilter     NotchFilterC_ToqMod;
STR_MTR_NotchFilter     NotchFilterD_ToqMod;


//陷波器C/D更新标志
static Uint16 NotchFilterAUpdateFlag_ToqMod = 0;
static Uint16 NotchFilterBUpdateFlag_ToqMod = 0;
static Uint16 NotchFilterCUpdateFlag_ToqMod = 0;
static Uint16 NotchFilterDUpdateFlag_ToqMod = 0;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
Uint64 SpdCoef_Q38 = 0;         //速度调节器增益参数转换系数


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */      
//以下函数在上电启机初始化程序中调度执行 
void InitGetIqRef(void);                 //上电启机初始化更新电流调节相关系数
void InitIqLmtValue(void);            //转矩限制值初始化

//以下函数在后台程序 （主循环）中调度执行
void GetIqRefStopUpdate(void);              //停机更新转矩指令获取文件参数
void GetIqRefUpdate(void);               //运行更新转矩指令获取文件参数
void IqLmtUpdateFun(void);               //运行更新转矩限制
Static_Inline int32 T_ITrans(Uint16 val);
//以下函数在发波中断程序（电流环调度）中调度执行
void IqLmtFun(void);                  //转矩限制值
void GetIqRef(void);                  //Ｑ轴电流指令的获取
//void SWOvCur_FdbErrMonitor(void);     //软件er200过流报错监控功能函数

//以下函数在位置环中断执行
void ToqReachJudge(void);             //转矩到达判断DO输出

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
void ToqMod_SpdLmt(void);             //转矩模式下速度限制
void PseudoSpdRegu(void);             //伪速度调节器
int32 GetAiIqLmt(Uint16 sel);         //获取由Ai输入的转矩限制值

/*******************************************************************************
  函数名:  InitGetIqRef(void)        ^_^
  输入: 部分H00 H01组驱动器、电机功能码参数  H07_09 H07_10正反转内部转矩限制值
        STR_MTR_Gvar.FunCode.OEM_OCProtectPoint  H01_18  驱动器过流保护点
        STR_MTR_Gvar.System.ToqPrd_Q10        转矩环调节周期 单位是us  Q12格式 
  输出:   1.转矩指令上下限幅值STR_MTR_Gvar.IqPosMax、STR_MTR_Gvar.IqNegMax
          2.输出相电流有效值为电机、驱动器相应额定和最大电流值对应的IqRef输入值
            STR_MTR_Gvar.IqRate_MT；STR_MTR_Gvar.IqMax_MT
            STR_MTR_Gvar.IqRate_OEM；STR_MTR_Gvar.IqMax_OEM
          3.STR_GetIqRef.CriticalCur 驱动器软件过流点
  子函数:无         
  描述:   ①STR_MTR_Gvar.IqPosMax、STR_MTR_Gvar.IqNegMax基于H0709、H0710、电机和驱动器最大电流的内部转矩正反向保守值的计算
            由于涉及功能码H0709、H0710，此值还需要在运行更新
          ②STR_GetIqRef.CriticalCur 驱动器软件过流点的计算
          ③当输出的电机相有效值电流为电机额定电流H0011时，IqRef的输入值（即IqRate_MT）；
            当输出的电机相有效值电流为驱动器额定电流H0107时，IqRef的输入值（即IqRate_OEM）；
          ④STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 Tref->Iqref 内部给定转换系数，
            即百分比转矩指令（如H0703＝100%）转换成数字量Q轴电流指令系数
          ⑤相电流有效值的计算和滤波处理交由FUNC处理。
          该函数在G_MTR_SysInit_STAR(void)系统开始运行初始化（主循环初始化）中调度
          ⑥伪速度调节器量纲初始化和转矩到达功能初始化
********************************************************************************/ 
void InitGetIqRef(void)
{ 
    //在112－FPGA中IN对应电机额定电流有效值，即当绕组电流峰值为SQRT(2) ×IN时，则转换得到的Iq数值为±4096（Id=0条件下）。
    STR_MTR_Gvar.GetIqRef.IqRate_MT = 4096;

    //当输出电机相有效值电流为驱动器额定电流H0107时IqRef的输入值（即IqRate_OEM）；
    STR_MTR_Gvar.GetIqRef.IqRate_OEM = 
         (int32)((((int64)STR_MTR_Gvar.GetIqRef.IqRate_MT * FunCodeUnion.code.OEM_RateCurrent <<10)
              / FunCodeUnion.code.MT_RateCurrent)  >> 10);


                                        
    //Tref->Iqref 内部给定转换系数，即百分比转矩指令（如H0703＝100%）转换成数字量Q轴电流指令系数
    STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 = ((int32)STR_MTR_Gvar.GetIqRef.IqRate_MT << 12) / 1000L;
	STR_MTR_Gvar.GetIqRef.ToqDigToPer_Q12 = (1000L<< 12) / STR_MTR_Gvar.GetIqRef.IqRate_MT;
    
    if(FunCodeUnion.code.OEM_ServoSeri >= 5)
    {
        //750W及以上功率驱动器
        // 判定三相反馈电流软件过流点，3.2倍驱动器额定电流幅值， OEM_OCProtectPoint为软件过流点的调整增益
        //反馈STR_MTR_Gvar.FPGA.IuFdb为模拟量电流   电流单位为0.01A 注意过流点也需放大100倍
        STR_GetIqRef.Iuvw_CriticalCur = 
                    ((int64) FunCodeUnion.code.OEM_RateCurrent * SOFT_OVER_CURENT1 * SQRT2_Q10 >>10)
                                              * (int32)FunCodeUnion.code.OEM_OCProtectPoint / 10000L ;
    
        // 判定q轴电流软件过流点，3.2倍驱动器额定电流幅值， OEM_OCProtectPoint为软件过流点的调整增益
        STR_GetIqRef.Iq_CriticalCur = ((int64)STR_MTR_Gvar.GetIqRef.IqRate_OEM * SOFT_OVER_CURENT1 
                                         * (int32)FunCodeUnion.code.OEM_OCProtectPoint) / 10000L;
    }
    else
    {        
        //200W  400W驱动器软件过流点是4.50倍驱动器额定电流幅值
        STR_GetIqRef.Iuvw_CriticalCur = 
                    ((int32) FunCodeUnion.code.OEM_RateCurrent * SOFT_OVER_CURENT2 * SQRT2_Q10 >>10)
                                              * (int32)FunCodeUnion.code.OEM_OCProtectPoint / 10000L ;
    
        STR_GetIqRef.Iq_CriticalCur = ((int32)STR_MTR_Gvar.GetIqRef.IqRate_OEM * SOFT_OVER_CURENT2 
                                         * (int32)FunCodeUnion.code.OEM_OCProtectPoint) / 10000L;
    }


/*----------------------------------------------------------------------------------------------
  量纲转换系数 SpdCoef = 2*PI * Jmotor * Jratio *( 2*PI/60) * (1/Kt)* (IqRate_MT/In) * (1/100)
------------------------------------------------------------------------------------------------
  电机惯量     Jmotor -> STR_MTR_Gvar.FunCode.MT_Inertia        单位0.01㎏c㎡  分子乘10^(-6)
  惯量比       Jratio -> STR_MTR_Gvar.FunCode.GN_InertiaRatio   单位0.01       分子乘10^(-2)
  转矩系数     Kt     -> STR_MTR_Gvar.FunCode.MT_ToqCoe         单位0.01N/A    分母乘10^(-2)
  电机额定电流 In     -> STR_MTR_Gvar.FunCode.MT_RateCurrent    单位0.01A      分母乘10^(-2)
  速度内部定标 1/100  -> 速度反馈内部                           单位为0.01rpm  分子乘10^(-2)
      4096/(SQRT2*In) -> 额定转矩对应数字量为4096,对应电流为SQRT2*IN
------------------------------------------------------------------------------------------------
  因此最后为10^(-6)数量级   最终速度环比例增益单位为0.1Hz   积分增益单位为0.1Hz
------------------------------------------------------------------------------------------------*/

    /*若更改此部分量纲请谨记同步更改速度调节器定标和惯量辨识处定标*/

    //伪速度调节器的速度定标等相关参数初始化  
	if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)	  //直线电机处理
	{
    	SpdCoef_Q38 = (((Uint64)STR_MTR_Gvar.GetIqRef.IqRate_MT) << 30) / 10000;
    	SpdCoef_Q38 = SpdCoef_Q38 * FunCodeUnion.code.MT_Inertia;
    	SpdCoef_Q38 = ((SpdCoef_Q38 * 2 * PI_Q12) >> 4) / (10 * 10000L);      //H0800重新带一个小数点
    	SpdCoef_Q38 = SpdCoef_Q38 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);
	}
	else
	{  
    	SpdCoef_Q38 = (((Uint64)2 * PI_Q12 * STR_MTR_Gvar.GetIqRef.IqRate_MT) << 30) / 100000000L;
    	SpdCoef_Q38 = SpdCoef_Q38 * FunCodeUnion.code.MT_Inertia;
    	SpdCoef_Q38 = ((SpdCoef_Q38 * 2 * PI_Q12) >> 16)/ (60 * 10);      //H0800重新带一个小数点
    	SpdCoef_Q38 = SpdCoef_Q38 / ((Uint32)FunCodeUnion.code.MT_RateToq * 100L);    	
	}

	//惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
	PseudoSpeedRegulator.Kb_Scal_Q38 = SpdCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);  //惯量比

    //转矩模式下判断是否超速等待时间系数转换，即连续X次速度环速度反馈均超速即认为超速  H0716(单位0.1ms最小0.5ms默认1ms)
    STR_GetIqRef.OverLmtSpdWaitCnt = (Uint16)((int32)FunCodeUnion.code.TL_OVSpdLmtWaitT 
                                      * STR_MTR_Gvar.System.SpdFreq/10000);
    //#define MAX(A,B) (((A)>(B))? (A) : (B))   取参数A、B的最大值
    STR_GetIqRef.OverLmtSpdWaitCnt = MAX(STR_GetIqRef.OverLmtSpdWaitCnt,2);

    //将转矩基准，转矩到达开，转矩到达关，百分比转矩转换成内部的数字量
    STR_GetIqRef.ToqRchStandard = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                 * (int16)FunCodeUnion.code.TL_ToqRchStandard) >> 12;
    STR_GetIqRef.ToqRchOn = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                * (int16)FunCodeUnion.code.TL_ToqRchOn) >> 12; 
    STR_GetIqRef.ToqRchOff = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                 * (int16)FunCodeUnion.code.TL_ToqRchOff) >> 12;


    //姚虹新设计转矩指令低通滤波器初始化(用于转矩模式下)
    NewToqRefLowpassFilter_ToqMod.Fs = STR_MTR_Gvar.System.SpdFreq;     //转矩滤波时间初始化

    if(UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc != 0)
    {
        NewToqRefLowpassFilter_ToqMod.Fc = ((Uint32)1000000 << 11) 
                                           / ((Uint32)UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc * PI_Q12);
    }
    else
    {
        NewToqRefLowpassFilter_ToqMod.Fc = NewToqRefLowpassFilter_ToqMod.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
    }   

    NewToqRefLowpassFilter_ToqMod.LastFc = NewToqRefLowpassFilter_ToqMod.Fc;
    MTR_InitNewLowPassFilt(&NewToqRefLowpassFilter_ToqMod);
   
    //转矩模式下陷波器初始化，H09组的前两组陷波器使用新的模型
    MTR_NewNotchFilterInit(&NotchFilterA_ToqMod, MTR_NOTCHFILTERA);
    MTR_NewNotchFilterInit(&NotchFilterB_ToqMod, MTR_NOTCHFILTERB);
    MTR_NewNotchFilterInit(&NotchFilterC_ToqMod, MTR_NOTCHFILTERC);
    MTR_NewNotchFilterInit(&NotchFilterD_ToqMod, MTR_NOTCHFILTERD);

}   //end of InitGetIqRef()




/*******************************************************************************
  函数名: InitIqLmtValue()
  输入:   
  输出:      
    STR_MTR_Gvar.GetIqRef.IqPosMax 正向转矩内部限制保守值赋值
    STR_MTR_Gvar.GetIqRef.IqNegMax 负向转矩内部限制保守值赋值
  子函数:        
  描述:     转矩限制值初始化 
********************************************************************************/
void InitIqLmtValue()
{
    int16  OverLoadCoe = 0;
    int32  IqMax_MT = 0;
    int32  IqMax_OEM = 0;

    int32  IqPosMaxTemp = 0;   //转矩内部限制正向保守值计算的中间变量
    int32  IqNegMaxTemp = 0;   //转矩内部限制负向保守值计算的中间变量 

    //目前IS500驱动器现状：型号OEM_ServoSeri（H01_02）等于0至10时为220V系列50W至5kw不等，
    //型号OEM_ServoSeri（H01_02）等于10001至10007时为三相380V系列500W至7.5kw不等
    if( (FunCodeUnion.code.OEM_ServoSeri / 10000) < 2)   
    {        //IS500 400W及400W以下驱动器电机允许过载倍数为5倍。
        if( (FunCodeUnion.code.OEM_RatePower <= 40))     //根据功率等级判断
        {                                    
            OverLoadCoe     = 50;  //*** 大马拉小车时候最大允许的电机电流过载倍数
        }
        else
        {    //IS500其它驱动器电机允许过载倍数为4倍。
            OverLoadCoe     = 40; 
        }
    }
    else //2倍 IS550
    {
        OverLoadCoe     = 30; //可能会受到驱动器最大电流的限制 *** 先不管  IS550中去做
    }

    //当电机输出相电流有效值为电机最大电流时IqRef（即IqMax_MT）输入值的计算；根据电机过载能力限制最大电流 
    if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30)
    {
        IqMax_MT = ((int32)STR_MTR_Gvar.GetIqRef.IqRate_MT * FunCodeUnion.code.MT_MaxToqOrCur) 
                    / FunCodeUnion.code.MT_RateCurrent;
    }
    else
    {
        IqMax_MT = ((int32)STR_MTR_Gvar.GetIqRef.IqRate_MT * OverLoadCoe) / 10;                     
    }
    STR_MTR_Gvar.GetIqRef.IqMax_MT = IqMax_MT;

    //当电机输出的相电流有效值为驱动器最大电流H0108时，IqRef的输入值计算（即IqMax_OEM ）
     IqMax_OEM  = (int32)( ((int64)((4096L <<10) / FunCodeUnion.code.MT_RateCurrent) * FunCodeUnion.code.OEM_MaxCurrentOut) >> 10);       

    if((FunCodeUnion.code.OEM_ServoSeri>=20014)&&(FunCodeUnion.code.OEM_ServoSeri<=20015))
     {
         IqMax_OEM=10L*IqMax_OEM;  //最大电流扩大了单位扩大了
     }
    IqMax_OEM = MIN(IqMax_OEM,32760);             //驱动器最大电流对应的Q轴电流指令IqMax_OEM限幅
    STR_MTR_Gvar.GetIqRef.IqMax_OEM = IqMax_OEM; 

    //老60Z电机需要特殊处理为3.6倍最大电流出3倍力
    if( (14 == (FunCodeUnion.code.MT_MotorModel / 1000)) 
     && (FunCodeUnion.code.OEM_ServoSeri != 60003)
     && ( (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 703)
       || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 704)
       || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 804) ) )    
    {    	
	    IqPosMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX1) >> 12; 
	    IqNegMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX1) >> 12;
    }
    else if( (FunCodeUnion.code.OEM_ServoSeri != 60003)
          && ( (FunCodeUnion.code.MT_MotorModel == 703)
            || (FunCodeUnion.code.MT_MotorModel == 704)
            || (FunCodeUnion.code.MT_MotorModel == 804) ) )     
    {    	
		IqPosMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX1) >> 12; 
		IqNegMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX1) >> 12;  
    }
    else
    {
        //基于电机和驱动器最大电流的内部转矩正反向保守值的计算，涉及功能码H0709 H0710的设定需要移至停机更新中去
        IqPosMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX) >> 12; 
        IqNegMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * IQMAX) >> 12;  
    }

    //取IqPosMax，IqMax_MT，IqMax_OEM 中最小值；
    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);

    //取IqNegMax（绝对值），IqMax_MT，IqMax_OEM 中最小值；
    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);
    IqNegMaxTemp = (-1) * IqNegMaxTemp;

    STR_MTR_Gvar.GetIqRef.IqPosMax = IqPosMaxTemp;   //正向转矩内部限制值赋值
    
    STR_MTR_Gvar.GetIqRef.IqNegMax = IqNegMaxTemp;   //负向转矩内部限制值赋值

    //根据H0707 限制转矩,获取STR_MTR_Gvar.GetIqRef.IqPosLmt，STR_MTR_Gvar.GetIqRef.IqNegLmt。
    IqLmtFun();
	
}


/*******************************************************************************
  函数名: extern void GetIqRefUpdate(void)  ^_^
  输入:   1.用户设定上下限制转矩H0709 H0710 TL_ToqPlusLmtIn  TL_ToqMinusLmtIn
            电机驱动器对应最大Q轴电流限制
          2.转矩模式下判断是否超速等待时间系数转换，TL_OVSpdLmtWaitT
            即连续X次速度环速度反馈均超速即认为超速  H0716(单位0.1ms最小0.5ms默认1ms)
  输出:   1.转矩指令上下限幅值STR_MTR_Gvar.IqPosMax、STR_MTR_Gvar.IqNegMax  
          2.转矩模式下判断是否超速等待时间系数转换成等待次数STR_GetIqRef.OverLmtSpdWaitCnt
  子函数:无         
  描述:运行更新H0709  H0710功能码相关的内部转矩正反向保守值
       STR_MTR_Gvar.IqPosMax、STR_MTR_Gvar.IqNegMax基于电机驱动器最大电流和H0709 H0710的内部转矩正反向保守值的计算
       转矩模式下判断是否超速等待时间系数转换，TL_OVSpdLmtWaitT
       即连续X次速度环速度反馈均超速即认为超速  H0716(单位0.1ms最小0.5ms默认1ms) 最少需连续两次判断。
       该函数在G_MTR_Task_BKINT(void)后台运行任务（主循环）中调度
********************************************************************************/
void GetIqRefStopUpdate(void)
{
//    int32  IqPosMaxTemp = 0;   //转矩内部限制正向保守值计算的中间变量
//    int32  IqNegMaxTemp = 0;   //转矩内部限制负向保守值计算的中间变量 
//
//    
//    //基于电机和驱动器最大电流的内部转矩正反向保守值的计算，涉及功能码H0709 H0710的设定需要移至运行更新中去
//    IqPosMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.TL_ToqPlusLmtIn) >> 12; 
//    IqNegMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.TL_ToqMinusLmtIn) >> 12;
//
//    //取IqPosMax，IqMax_MT，IqMax_OEM 中最小值；
//    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
//    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);
//
//    //取IqNegMax（绝对值），IqMax_MT，IqMax_OEM 中最小值；
//    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
//    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);
//    IqNegMaxTemp = (-1) * IqNegMaxTemp;
//
//    STR_MTR_Gvar.GetIqRef.IqPosMax = IqPosMaxTemp;   //正向转矩内部限制值赋值
//    
//    STR_MTR_Gvar.GetIqRef.IqNegMax = IqNegMaxTemp;   //负向转矩内部限制值赋值

    //惯量比改为负载惯量与电机本体惯量比值，原为(负载+电机)/电机
    PseudoSpeedRegulator.Kb_Scal_Q38 = SpdCoef_Q38 * (FunCodeUnion.code.GN_InertiaRatio + 100L);    //惯量比

    MTR_ResetNewLowPassFilter(&NewToqRefLowpassFilter_ToqMod);

    //转矩模式下四组陷波器初始化，H09组的四个陷波器使用最新的陷波器
    MTR_NewNotchFilterInit(&NotchFilterA_ToqMod, MTR_NOTCHFILTERA);
    MTR_NewNotchFilterInit(&NotchFilterB_ToqMod, MTR_NOTCHFILTERB);
    MTR_NewNotchFilterInit(&NotchFilterC_ToqMod, MTR_NOTCHFILTERC);
    MTR_NewNotchFilterInit(&NotchFilterD_ToqMod, MTR_NOTCHFILTERD);

} //end of GetIqRefStopUpdate(void)


/*******************************************************************************
  函数名:  IqLmtUpdateFun(void)
  输入:   
  输出:  
  子函数:无         
  描述:  转矩指令限幅值更新
         于主循环初始化和主循环1KHz频率更新
       
********************************************************************************/
void IqLmtUpdateFun(void)
{
    //根据H0707 限制转矩,获取STR_MTR_Gvar.GetIqRef.IqPosLmt，STR_MTR_Gvar.GetIqRef.IqNegLmt。
    IqLmtFun();
}


/*******************************************************************************
  函数名:  GetIqRefUpdate(void)
  输入:   
  输出:  
  子函数:无         
  描述:  转矩指令获取实时更新函数
         于主循环初始化和主循环4Hz频率更新
       
********************************************************************************/
void GetIqRefUpdate(void)
{
    static  Uint16 NotchFiltFreqAOld_ToqMod = 0;
    static  Uint16 NotchFiltBandWidthAOld_ToqMod = 0;
    static  Uint16 NotchFiltDepthAOld_ToqMod = 0;
    static  Uint16 NotchFiltFreqBOld_ToqMod = 0;
    static  Uint16 NotchFiltBandWidthBOld_ToqMod = 0;
    static  Uint16 NotchFiltDepthBOld_ToqMod = 0;
    static  Uint16 NotchFiltFreqCOld_ToqMod = 0;
    static  Uint16 NotchFiltBandWidthCOld_ToqMod = 0;
    static  Uint16 NotchFiltDepthCOld_ToqMod = 0;
    static  Uint16 NotchFiltFreqDOld_ToqMod = 0;
    static  Uint16 NotchFiltBandWidthDOld_ToqMod = 0;
    static  Uint16 NotchFiltDepthDOld_ToqMod = 0;


//    //基于电机和驱动器最大电流的内部转矩正反向保守值的计算，涉及功能码H0709 H0710的设定需要移至运行更新中去
//    IqPosMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.TL_ToqPlusLmtIn) >> 12; 
//    IqNegMaxTemp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.TL_ToqMinusLmtIn) >> 12;
//
//    //取IqPosMax，IqMax_MT，IqMax_OEM 中最小值；
//    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
//    IqPosMaxTemp = MIN(IqPosMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);
//
//    //取IqNegMax（绝对值），IqMax_MT，IqMax_OEM 中最小值；
//    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_MT);
//    IqNegMaxTemp = MIN(IqNegMaxTemp,STR_MTR_Gvar.GetIqRef.IqMax_OEM);
//    IqNegMaxTemp = (-1) * IqNegMaxTemp;

//    STR_MTR_Gvar.GetIqRef.IqPosMax = IqPosMaxTemp;   //正向转矩内部限制值赋值
//    
//    STR_MTR_Gvar.GetIqRef.IqNegMax = IqNegMaxTemp;   //负向转矩内部限制值赋值
    //转矩模式下判断是否超速等待时间系数转换，即连续X次速度环速度反馈均超速即认为超速  H0716(单位0.1ms最小0.5ms默认1ms)
    STR_GetIqRef.OverLmtSpdWaitCnt = (Uint16)((int32)FunCodeUnion.code.TL_OVSpdLmtWaitT 
                                      * STR_MTR_Gvar.System.SpdFreq/10000);
    //#define MAX(A,B) (((A)>(B))? (A) : (B))   取参数A、B的最大值
    STR_GetIqRef.OverLmtSpdWaitCnt = MAX(STR_GetIqRef.OverLmtSpdWaitCnt,2);

    //将转矩基准，转矩到达开，转矩到达关，百分比转矩转换成内部的数字量
    STR_GetIqRef.ToqRchStandard = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                 * (int16)FunCodeUnion.code.TL_ToqRchStandard) >> 12;
    STR_GetIqRef.ToqRchOn = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                 * (int16)FunCodeUnion.code.TL_ToqRchOn) >> 12; 
    STR_GetIqRef.ToqRchOff = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 
                                 * (int16)FunCodeUnion.code.TL_ToqRchOff) >> 12;  
    //加入转矩环增益切换更新
    STR_MTR_Gvar.GetIqRef.IqPoint1 = ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.OEM_FirSwtchIq) >> 12;
    STR_MTR_Gvar.GetIqRef.IqPoint2 = ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.OEM_SecSwtchIq) >> 12;
    STR_MTR_Gvar.GetIqRef.IqPoint3 = ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.OEM_ThirdSwtchIq) >> 12;                               
    STR_MTR_Gvar.GetIqRef.IqPoint4 = ((int64)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * FunCodeUnion.code.OEM_FourthSwtchIq) >> 12;

    //斜率是增益百分比形式
    STR_MTR_Gvar.GetIqRef.FirSlope_Q10 = (((int32)(int16)FunCodeUnion.code.OEM_CapSecKpCoff - 1000L) << 10)
                                        / ((STR_MTR_Gvar.GetIqRef.IqPoint2 - STR_MTR_Gvar.GetIqRef.IqPoint1));
    STR_MTR_Gvar.GetIqRef.SecSlope_Q10 = (((int32)((int16)FunCodeUnion.code.OEM_CapThirdKpCoff - (int16)FunCodeUnion.code.OEM_CapSecKpCoff) ) << 10)
                                         / (STR_MTR_Gvar.GetIqRef.IqPoint4 - STR_MTR_Gvar.GetIqRef.IqPoint3);


    //姚虹新设计转矩指令低通滤波器初始化（用于转矩模式）
    NewToqRefLowpassFilter_ToqMod.Fs = STR_MTR_Gvar.System.SpdFreq;     //转矩滤波时间初始化

    if(UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc != 0)
    {
        NewToqRefLowpassFilter_ToqMod.Fc = ((Uint32)1000000 << 11) / 
                                           ((Uint32)UNI_MTR_FUNCToMTR_List_16kHz.List.ToqRefFilterTc * PI_Q12);
    }
    else
    {
        NewToqRefLowpassFilter_ToqMod.Fc = NewToqRefLowpassFilter_ToqMod.Fs;      //当转矩滤波时间为0时，不需要滤波，将截止频率设置得和采样频率一致即可！
    }
    
    if(NewToqRefLowpassFilter_ToqMod.Fc != NewToqRefLowpassFilter_ToqMod.LastFc)
    {
        NewToqRefLowpassFilter_ToqMod.LastFc = NewToqRefLowpassFilter_ToqMod.Fc;
        MTR_InitNewLowPassFilt(&NewToqRefLowpassFilter_ToqMod);
    }


    //手动设置陷波器初始化
    if((NotchFiltFreqAOld_ToqMod != FunCodeUnion.code.AT_NotchFiltFreqA)  \
      || (NotchFiltBandWidthAOld_ToqMod != FunCodeUnion.code.AT_NotchFiltBandWidthA) \
      || (NotchFiltDepthAOld_ToqMod != FunCodeUnion.code.AT_NotchFiltAttenuatLvlA))
    {
        NotchFilterAUpdateFlag_ToqMod = 1;
        MTR_NewNotchFilterInit(&NotchFilterA_ToqMod, MTR_NOTCHFILTERA);
        NotchFilterAUpdateFlag_ToqMod = 0; 
    }

    if((NotchFiltFreqBOld_ToqMod != FunCodeUnion.code.AT_NotchFiltFreqB)  \
      || (NotchFiltBandWidthBOld_ToqMod != FunCodeUnion.code.AT_NotchFiltBandWidthB) \
      || (NotchFiltDepthBOld_ToqMod != FunCodeUnion.code.AT_NotchFiltAttenuatLvlB))
    {
        NotchFilterBUpdateFlag_ToqMod = 1;
        MTR_NewNotchFilterInit(&NotchFilterB_ToqMod, MTR_NOTCHFILTERB);
        NotchFilterBUpdateFlag_ToqMod = 0; 
    }

    NotchFiltFreqAOld_ToqMod      = FunCodeUnion.code.AT_NotchFiltFreqA;
    NotchFiltBandWidthAOld_ToqMod = FunCodeUnion.code.AT_NotchFiltBandWidthA;
    NotchFiltDepthAOld_ToqMod     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlA;
    NotchFiltFreqBOld_ToqMod      = FunCodeUnion.code.AT_NotchFiltFreqB;
    NotchFiltBandWidthBOld_ToqMod = FunCodeUnion.code.AT_NotchFiltBandWidthB;
    NotchFiltDepthBOld_ToqMod     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlB;
    
    //自适应滤波器C/D参数更新
    if((NotchFiltFreqCOld_ToqMod != FunCodeUnion.code.AT_NotchFiltFreqC)  \
      || (NotchFiltBandWidthCOld_ToqMod != FunCodeUnion.code.AT_NotchFiltBandWidthC) \
      || (NotchFiltDepthCOld_ToqMod != FunCodeUnion.code.AT_NotchFiltAttenuatLvlC))
    {
        NotchFilterCUpdateFlag_ToqMod = 1;
        MTR_NewNotchFilterInit(&NotchFilterC_ToqMod, MTR_NOTCHFILTERC);
        NotchFilterCUpdateFlag_ToqMod = 0; 
    }

    if((NotchFiltFreqDOld_ToqMod != FunCodeUnion.code.AT_NotchFiltFreqD)  \
      || (NotchFiltBandWidthDOld_ToqMod != FunCodeUnion.code.AT_NotchFiltBandWidthD) \
      || (NotchFiltDepthDOld_ToqMod != FunCodeUnion.code.AT_NotchFiltAttenuatLvlD))
    {
        NotchFilterDUpdateFlag_ToqMod = 1;
        MTR_NewNotchFilterInit(&NotchFilterD_ToqMod, MTR_NOTCHFILTERD);
        NotchFilterDUpdateFlag_ToqMod = 0;
    }

    NotchFiltFreqCOld_ToqMod      = FunCodeUnion.code.AT_NotchFiltFreqC;
    NotchFiltBandWidthCOld_ToqMod = FunCodeUnion.code.AT_NotchFiltBandWidthC;
    NotchFiltFreqDOld_ToqMod      = FunCodeUnion.code.AT_NotchFiltFreqD;
    NotchFiltBandWidthDOld_ToqMod = FunCodeUnion.code.AT_NotchFiltBandWidthD;
    NotchFiltDepthCOld_ToqMod     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlC;
    NotchFiltDepthDOld_ToqMod     = FunCodeUnion.code.AT_NotchFiltAttenuatLvlD;

}



/*******************************************************************************
  函数名:  ToqLmtFun()
  输入:    功能码
  输出:    
  子函数:  GetAiLmt()       
  描述: 转矩模式下当前转矩指令限制值
********************************************************************************/ 
void IqLmtFun()
{
    int32 TempP = 0;
    int32 TempN = 0; 
    int32 TempIqP = 0;
	int32 TempIqN = 0;
    Uint16 TempToqP = 0;
    Uint16 TempToqN = 0;
    int32 IqPosLmtValue = 0;                 //正转转矩限制值,该函数内部计算使用
    int32 IqNegLmtValue = 0;                 //反转转矩限制值,该函数内部计算使用
    /*先将转矩指令限制值初始化为保守值*/
    IqPosLmtValue = STR_MTR_Gvar.GetIqRef.IqPosMax;
    IqNegLmtValue = STR_MTR_Gvar.GetIqRef.IqNegMax;

    /*根据功能码计算当前的转矩指令限制值*/
    switch(FunCodeUnion.code.TL_ToqLmtSource)   
    {
        /* 内部(保守值)*/
        case 0: 
				IqPosLmtValue = T_ITrans(FunCodeUnion.code.TL_ToqPlusLmtIn); 
				IqNegLmtValue = (-1)*T_ITrans(FunCodeUnion.code.TL_ToqMinusLmtIn);

		        break;                                       
                                                                           
        /*外部*/       
        case 1: if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Pcl == 1)              //DI外部输入的正转转矩指令限制值 G2需要从新命名
                {
                    IqPosLmtValue = T_ITrans(FunCodeUnion.code.TL_ToqPlusLmtTrig);         
                }
				else
				{
    				IqPosLmtValue = T_ITrans(FunCodeUnion.code.TL_ToqPlusLmtIn); 
				}
                if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Ncl == 1)
                {
                    IqNegLmtValue = (-1)*T_ITrans(FunCodeUnion.code.TL_ToqMinusLmtTrig);  
                }
				else
				{
                    IqNegLmtValue = (-1)*T_ITrans(FunCodeUnion.code.TL_ToqMinusLmtIn);  
                }
                break;
        //T_LMT        
        case 2: 
                #if ECT_ENABLE_SWITCH
                    TempToqP = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxPosToq);
                    TempToqN = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxNegToq);

                    IqPosLmtValue = T_ITrans(TempToqP);
                    IqNegLmtValue = (-1) * T_ITrans(TempToqN);               
                #else
                    IqPosLmtValue = GetAiIqLmt(FunCodeUnion.code.TL_ToqLmtAISel);
                    IqNegLmtValue = (-1) * IqPosLmtValue;               
                #endif
                
                break;
        //后两者较小值
        case 3: 
                #if ECT_ENABLE_SWITCH
                    TempToqP = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxPosToq);
                    TempToqN = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxNegToq);

                    IqPosLmtValue = T_ITrans(TempToqP);
                    IqNegLmtValue = (-1) * T_ITrans(TempToqN);               
                #else
                    IqPosLmtValue = GetAiIqLmt(FunCodeUnion.code.TL_ToqLmtAISel);
                    IqNegLmtValue = (-1) * IqPosLmtValue;               
                #endif

                TempP = T_ITrans(FunCodeUnion.code.TL_ToqPlusLmtTrig);         
                TempN = (-1)*T_ITrans(FunCodeUnion.code.TL_ToqMinusLmtTrig);   
                
                if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Pcl == 1) && (IqPosLmtValue > TempP))
                {
                    IqPosLmtValue = TempP; 
                }
                if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Ncl == 1) && (IqNegLmtValue < TempN))
                {
                    IqNegLmtValue = TempN; 
                }
                break;
        case 4: 
                #if ECT_ENABLE_SWITCH
                    TempToqP = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxPosToq);
                    TempToqN = MIN((Uint16)ObjectDictionaryStandard.ProToqMode1.MaxToq,(Uint16)ObjectDictionaryStandard.MaxToqLmt.MaxNegToq);

                    TempIqP = T_ITrans(TempToqP);
                    TempIqN = (-1) * T_ITrans(TempToqN);               
                #else
                    TempIqP = GetAiIqLmt(FunCodeUnion.code.TL_ToqLmtAISel);
                    TempIqN = (-1) * TempIqP;               
                #endif

    			TempP = T_ITrans(FunCodeUnion.code.TL_ToqPlusLmtIn); 
    			TempN = (-1)*T_ITrans(FunCodeUnion.code.TL_ToqMinusLmtIn);
                
                if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Pcl == 1)
                {
                    IqPosLmtValue = TempIqP; 
                }
				else
				{
					IqPosLmtValue = TempP;
				}
				
                if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.Ncl == 1)
                {
                    IqNegLmtValue = TempIqN; 
                }
				else
                {
                    IqNegLmtValue = TempN; 
                }
                break;
       default: 
                PostErrMsg(DEFUALTERR);    //系统参数出故障
                break;
    }
    
    if(IqPosLmtValue > STR_MTR_Gvar.GetIqRef.IqPosMax)
    {
        IqPosLmtValue = STR_MTR_Gvar.GetIqRef.IqPosMax;
    }
    
    if(IqNegLmtValue < STR_MTR_Gvar.GetIqRef.IqNegMax) 
    {
        IqNegLmtValue = STR_MTR_Gvar.GetIqRef.IqNegMax;
    }

    STR_MTR_Gvar.GetIqRef.IqPosLmt = IqPosLmtValue;
    STR_MTR_Gvar.GetIqRef.IqNegLmt = IqNegLmtValue; 
    
    STR_MTR_Gvar.GetIqRef.IqPosLmt_F = IqPosLmtValue * 1000L / STR_MTR_Gvar.GetIqRef.IqRate_MT;
    STR_MTR_Gvar.GetIqRef.IqNegLmt_F = IqNegLmtValue * 1000L / STR_MTR_Gvar.GetIqRef.IqRate_MT;         
}
 
/*******************************************************************************
  函数名:  GetAiIqLmt(Uint16 sel)
  输入:    功能码
  输出:    Ai输出的值
  子函数:         
  描述: 转矩模式下由Ai输入的转矩指令限制值
********************************************************************************/ 
#if (ECT_ENABLE_SWITCH==0)
Static_Inline int32 GetAiIqLmt(Uint16 sel)
{
    int32 ref = 0;

    switch(sel) 
    {
        case 1: 
                ref = ((UNI_MTR_FUNCToMTR_List_16kHz.List.AI1VoltOut * 12L) * (Uint16)FunCodeUnion.code.AI_ToqGain ) >> 15;

				if(ref<0)
                {
                    ref = (-1)*T_ITrans(-ref);
                }
				else
				{
				    ref = T_ITrans(ref); 
				}				
                break;

        case 2:
                ref = ((UNI_MTR_FUNCToMTR_List_16kHz.List.AI2VoltOut * 12L) * (Uint16)FunCodeUnion.code.AI_ToqGain ) >> 15;

				if(ref<0)
                {
                    ref = (-1)*T_ITrans(-ref);
                }
				else
				{
				    ref = T_ITrans(ref); 
				}
                break;

        //case 3:
//                ref = ((UNI_MTR_FUNCToMTR_List_16kHz.List.AI3SampVolt * 12L) * (Uint16)FunCodeUnion.code.AI_ToqGain ) >> 15;
//                ref = ((int32)STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * ref) >> 12;

        default: 
                PostErrMsg(DEFUALTERR);    //系统参数出故障
                break;
    }

    if(ref<0)
    {
        ref = (-1) * ref;                          //取绝对值
    }
    return ref;

}
#endif

/*******************************************************************************
  函数名:  GetIqRef()        ^_^
  输入:  STR_MTR_Gvar.FUNCtoMTR.IqIn转矩模式下转矩指令来源
         STR_MTR_Gvar.PUBFlg.bit.RUNMOD运行模式暂定先由PUB共用
         STR_MTR_Gvar.PUBFlg.bit.TOQSTOPFLG急停标志位
         STR_MTR_Gvar.FunCode.TL_EmergentToq急停转矩功能码
         STR_MTR_Gvar.GetIqRef.IqPosLmt正转转矩限制,计算出来的最后限制值
         STR_MTR_Gvar.GetIqRef.IqNegLmt反转转矩限制,计算出来的最后限制值
         STR_MTR_Gvar.FPGA.SpdFdb 反馈速度
         转矩指令滤波STR_MTR_Gvar.FunCode.TL_ToqFiltTime

  输出:  STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Clt;  扭矩限制DO信号输出 
         STR_MTR_Gvar.GetIqRef.IqRef;                    Iq电流调节器前端输入值
         STR_MTR_Gvar.GetIqRef.IqSource;        转矩模式下的外部Q轴电机给定，非调节器前端输入值

  子函数:Static_Inline ToqMod_SpdLmt()转矩模式下的速度限制

  描述: ①判断选取Q轴电流指令为速度调节器输出、外部转矩指令IqIn或者转矩模式下的伪速度调节器输入；
        ②接收到FUNC模块的急停标志位后，Q轴电流指令直接赋于急停转矩
        ③转矩指令滤波拟移入电流环，以缩短滤波采样周期。（松下、安川转矩指令滤波均在电流环）
        ④IqPosLmt  IqNegLmt的比较获取交由FUNC模块处理，对输入到调节器的转矩指令限幅，并输出限幅DO信号
          转矩限制放至最后，之前的限制均为32767，只在此限制为IqPosLmt  IqNegLmt。
        该函数在G_MTR_Task_WGINT发波中断运行任务（电流环）中调度
********************************************************************************/ 
void GetIqRef(void)
{
    int32  IqOutTmp = 0;

    //1,获取指令
    //如果控制模式在速度控制及以上，以速度环输出作为
    if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.RunMod > TOQMOD)
       &&(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ModSwitchPeriod == 0))              
    {
        if (0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OpenFSAEn)
        {
            STR_GetIqRef.IqOut = STR_MTR_Gvar.SpeedRegulator.SpdReguOut;       //速度调节器输出
        }
        else
        {
            STR_GetIqRef.IqOut = STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOut; 
        }
        ResetPDFFCtrl(&PseudoSpeedRegulator);  //复位伪速度调节器
    }
    else//FUNC获取的转矩指令IqIn与伪速度调节器进行逻辑比较得出IqOut
    {
        ToqMod_SpdLmt();                                      //转矩模式下的速度限制

        IqOutTmp = STR_GetIqRef.IqOut;

        //转矩模式下低通滤波处理
        NewToqRefLowpassFilter_ToqMod.Input = (int32)(IqOutTmp << 10);
        MTR_NewLowPassFiltCalc(&NewToqRefLowpassFilter_ToqMod);
        IqOutTmp = (int32)(NewToqRefLowpassFilter_ToqMod.Output >> 10);
        
        //陷波器滤波处理
        //单个运行最长时间 330/120us
        //4个运行最长时间 1200/120us
        if(NotchFilterAUpdateFlag_ToqMod != 1)
        {
            if(FunCodeUnion.code.AT_NotchFiltFreqA != 4000)  //陷波频率为上限时不进行陷波
            {
                NotchFilterA_ToqMod.NotchInput[0] = IqOutTmp;
                MTR_NotchFilter(&NotchFilterA_ToqMod);
                IqOutTmp = NotchFilterA_ToqMod.NotchOutput[0];
            }
        }

        if(NotchFilterBUpdateFlag_ToqMod != 1)
        {
            if(FunCodeUnion.code.AT_NotchFiltFreqB != 4000) //陷波频率为上限时不进行陷波
            {
                NotchFilterB_ToqMod.NotchInput[0] = IqOutTmp;
                MTR_NotchFilter(&NotchFilterB_ToqMod);
                IqOutTmp = NotchFilterB_ToqMod.NotchOutput[0];
            }        
        }

        if(NotchFilterCUpdateFlag_ToqMod != 1)
        {
            if(FunCodeUnion.code.AT_NotchFiltFreqC != 4000) 
            {
                NotchFilterC_ToqMod.NotchInput[0] = IqOutTmp;
                MTR_NotchFilter(&NotchFilterC_ToqMod);
                IqOutTmp = NotchFilterC_ToqMod.NotchOutput[0];
            }
        }

        if(NotchFilterDUpdateFlag_ToqMod != 1)
        {
            if(FunCodeUnion.code.AT_NotchFiltFreqD != 4000)  //陷波频率为上限时不进行陷波
            {
                NotchFilterD_ToqMod.NotchInput[0] = IqOutTmp;
                MTR_NotchFilter(&NotchFilterD_ToqMod);
                IqOutTmp = NotchFilterD_ToqMod.NotchOutput[0];
            }
        }

//    	//输出限幅处理，饱和遇限处理
//    	if((int32)IqOutTmp >= PseudoSpeedRegulator.PosLmt) 
//    	{
//			IqOutTmp  = PseudoSpeedRegulator.PosLmt;
//		//	PseudoSpeedRegulator.SaturaFlag = 2;
//    	}
//    	else if ((int32)IqOutTmp <= PseudoSpeedRegulator.NegLmt) 
//    	{
//			IqOutTmp  = PseudoSpeedRegulator.NegLmt;
//		//	PseudoSpeedRegulator.SaturaFlag = 1;
//    	}
//		else
//		{
//	    //	PseudoSpeedRegulator.SaturaFlag = 0;
//		}


        STR_GetIqRef.IqOut = (int32)IqOutTmp;

    }

    //接收到FUNC模块的急停标志位后，Q轴电流指令直接赋于急停转矩
        /* WZG1470--20111011更改开始 */
    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OTClamp == 1)
    {
        //STR_FUNC_Gvar.ToqCtrl.ToqCmdLatch = STR_FUNC_Gvar.ToqCtrl.ToqCmd;
        STR_GetIqRef.IqOut = STR_MTR_Gvar.SpeedRegulator.SpdReguOut;
    }     /* WZG1470--20111011更改结束 */
    else if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ZeroSpdStop == 1)     //零速停机时转矩指令来源为速度调节器的输出
    {
        STR_GetIqRef.IqOut = STR_MTR_Gvar.SpeedRegulator.SpdReguOut;
    }
    else if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ToqStop == 1)             //******* 急停标志 **********
    {
        if(STR_MTR_Gvar.FPGA.SpdFdb > 300000)                 //大于30rpm加入反向力矩
            STR_GetIqRef.IqOut = (-1L) * ((STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * (int32)FunCodeUnion.code.TL_EmergentToq)>>12);
        else if(STR_MTR_Gvar.FPGA.SpdFdb < -300000)           //小于-30rpm加入正向力矩
            STR_GetIqRef.IqOut = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * (int32)FunCodeUnion.code.TL_EmergentToq)>>12;
        else
            STR_GetIqRef.IqOut = 0;
    }

    //转矩限制最后输出处理，对输入到调节器的转矩指令限幅，并输出限幅DO信号
    //条件变为>=，否则前面IqOut已经转矩限幅，不会进入条件将DOvarReg_Clt置1.
    if(STR_GetIqRef.IqOut >= STR_MTR_Gvar.GetIqRef.IqPosLmt)
    {
        STR_GetIqRef.IqOut  = STR_MTR_Gvar.GetIqRef.IqPosLmt;
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Clt = 1;
    }
    else if(STR_GetIqRef.IqOut <= STR_MTR_Gvar.GetIqRef.IqNegLmt)
    {
        STR_GetIqRef.IqOut  = STR_MTR_Gvar.GetIqRef.IqNegLmt; 
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Clt = 1;
    }
    else
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Clt = 0;
    }

    if( (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == RUN) &&
        (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus == ENPWM) )
    {
        STR_MTR_Gvar.GetIqRef.IqRef = STR_GetIqRef.IqOut;
    }
    else
    {//非运行模式下,清零转矩指令和DO输出
        STR_MTR_Gvar.GetIqRef.IqRef = 0;

        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Clt = 0;
    }
} //end of GetIqRef(void)  


/*******************************************************************************
  函数名:  转矩模式下的速度限制ToqMod_SpdLmt()    ^_^
  输入:   1.转矩模式下速度限制值STR_MTR_Gvar.FUNCtoMTR.SpdLmt；STR_MTR_Gvar.FPGA.SpdFdb
          2.伪速度环调度标志位STR_MTR_Gvar.GetIqRef.PseudoSpdFlg与速度环的调度同步
          3.伺服运行状态STR_MTR_Gvar.FUNCtoMTR.SERVORUNSTATUS
          4.速度是否限幅滞环判断次数STR_GetIqRef.OverLmtSpdWaitCnt
  输出:   1.输出Q轴电流给定STR_GetIqRef.IqOut；
          2.速度限幅标志位STR_MTR_Gvar.GlobalFlag.bit.DOvarReg_Vlt
  子函数:PseudoSpdRegu()伪速度调节器
  描述:   转矩模式下的速度限制。早期版本为伪速度调节器与转矩指令输入取最小值作为转矩环输入，
          该版本当速度增益偏小时切入伪速度调节器的时机延时很多，导致速度经常限不住。
          后期姚虹版本以速度反馈为条件，仅当速度反馈超过限制速度H0719时即切入伪速度调节器，不再切回转矩环。
          考虑此功能市场应用主要为保证：①限制速度下运行转矩恒定；②达到限制速度时即先保证速度低于限制速度；
          ③在保证前面两条的前提下，可以在恒转矩模式和速度限制调节模式下自由“平滑”切换。
          因此改进该功能以速度反馈作为判断标准，且加入判断条件滞环，仅当速度连续X次超过限制速度才切入伪速度调节。
          且切入时刻的以转矩指令IqIn作为伪调节器的输出初始值（保证切换平滑）。
          仅当速度连续X次低于限制速度且调节器输出第一次等于转矩指令IqIn时（保证切换平滑）才切回转矩模式。
          该函数在后台程序 GetIqRef(void)（Q轴电流指令获取）中调度执行  
********************************************************************************/ 
Static_Inline void ToqMod_SpdLmt(void)
{
    static Uint16 OverLmtSpdCnt = 0;
    static Uint16 UnderLmtSpdCnt = 0;


    STR_GetIqRef.ToqCmd_Limited = UNI_MTR_FUNCToMTR_List_16kHz.List.ToqCmd;

    //转矩模式下转矩指令也需要进行转矩限制后输出  
    STR_GetIqRef.ToqCmd_Limited = MAX_MIN_LMT(STR_GetIqRef.ToqCmd_Limited,STR_MTR_Gvar.GetIqRef.IqPosLmt,STR_MTR_Gvar.GetIqRef.IqNegLmt);
    
	STR_GetIqRef.PseudoSpeedRef= (STR_GetIqRef.ToqCmd_Limited>=0)?UNI_MTR_FUNCToMTR_List_16kHz.List.SpdLmtPos:UNI_MTR_FUNCToMTR_List_16kHz.List.SpdLmtNeg;

    //非伺服运行状态处理
    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN)
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Vlt = 0;         //开始未超速
        STR_GetIqRef.OverLmtSpdFlg = 0;
    }
    else if(STR_MTR_Gvar.GetIqRef.PseudoSpdFlg)           //与速度环调度同步
    {     
        //反馈速度如果连续X次超过(限制速度+20rpm)即判定超速
    //    if(ABS(UNI_MTR_FUNCToMTR_List_16kHz.List.SpdLmt) < (ABS(STR_MTR_Gvar.FPGA.SpdFdb))) // + 200000
          if(((STR_GetIqRef.ToqCmd_Limited>=0)&&(STR_MTR_Gvar.FPGA.SpdFdb>=(UNI_MTR_FUNCToMTR_List_16kHz.List.SpdLmtPos)))
             ||((STR_GetIqRef.ToqCmd_Limited<=0)&&(STR_MTR_Gvar.FPGA.SpdFdb<= (UNI_MTR_FUNCToMTR_List_16kHz.List.SpdLmtNeg))))
        {
            UnderLmtSpdCnt = 0;                                  //未超速计数清零
            if(OverLmtSpdCnt >= STR_GetIqRef.OverLmtSpdWaitCnt)  //如果连续超速X次就置位超速标志        
            {
                STR_GetIqRef.OverLmtSpdFlg = 1;
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Vlt = 1;        //速度限幅DO输出置位
            }
            else
            {
                OverLmtSpdCnt++;                 //累计超速计数
            }
        }
        else
        {

            OverLmtSpdCnt = 0;                  //超速次数清零
            //如果连续计数未超速X次且伪速度调节器输出首次超过ToqCMD就清零超速标志防止Iqref突变 
            if((UnderLmtSpdCnt >= STR_GetIqRef.OverLmtSpdWaitCnt)
                &&(STR_GetIqRef.SaturaFlag > 0))  
            {
                STR_GetIqRef.OverLmtSpdFlg = 0;                   //如果连续未超速X次就清零超速标志 
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_Vlt = 0;         //速度限幅DO输出清零
            }                               
            else if( UnderLmtSpdCnt < STR_GetIqRef.OverLmtSpdWaitCnt)
            {
                UnderLmtSpdCnt++;               //累计未超速计数
            }
        }
    }

    //如果超速标志位为0或转矩指令方向变化，就用FUNC给定转矩值
    if(STR_GetIqRef.OverLmtSpdFlg == 0)     
    { //保证每次从转矩模式切换到伪速度调节器前调节器初始累积偏差为0，即积分为0，防止切换时转矩波动过大
        ResetPDFFCtrl(&PseudoSpeedRegulator);  //复位伪速度调节器

		PseudoSpeedRegulator.PosLmt = 32767L;
	    PseudoSpeedRegulator.NegLmt = -32767L;

        STR_GetIqRef.IqOut = (int32)STR_GetIqRef.ToqCmd_Limited;
	
    }
    else if((STR_MTR_Gvar.GetIqRef.PseudoSpdFlg == 1)&&(STR_GetIqRef.OverLmtSpdFlg == 1))
    {
        PseudoSpdRegu();      //进入伪速度调节器进行调节
		

        STR_GetIqRef.IqOut = (int32)STR_GetIqRef.PseudoSpdOut;

        //退出时将判断是否为伪速度调节器调度时刻标志清零，与速度环调度同步。
        STR_MTR_Gvar.GetIqRef.PseudoSpdFlg = 0;
    }

//    NewToqRefLowpassFilter_ToqMod.Input = (STR_GetIqRef.IqOut << 10);
//    MTR_NewLowPassFiltCalc(&NewToqRefLowpassFilter_ToqMod);
//    STR_GetIqRef.IqOut = (NewToqRefLowpassFilter_ToqMod.Output >> 10);
}



/*******************************************************************************
  函数名:  Static_Inline void PseudoSpdRegu(void)      ^_^
  输入:   1.速度环调节器指令、反馈和增益。 
          2.STR_MTR_Gvar.IqPosLmt转矩指令最后限幅值
  输出:   1.速度调节器输出SpdReguOutTemp 
  子函数: 借用速度环调节器SpdRegulator()         
  描述: ①转矩模式下的速度限制与速度模式不冲突，转矩模式与速度模式切换的时候会将调节器累积参数清零。
          且调节器增益参数保持一致。SpdRegulator()函数的设计保证独立性，
          在转矩模式下的速度限制也可调用此调节器只需更改速度指令的获取
         ②为保证从转矩模式恒转矩ToqCmd平滑切换至速度限制，
            伪调节器输出以ToqCmd为初始值，相当于加一个前馈值。
            即使在速度限制模式下也保证转矩指令不超过ToqCmd。
         该函数在时基中断程序（转矩模式下的速度限制函数）ToqMod_SpdLmt()中调度执行
********************************************************************************/ 
Static_Inline void PseudoSpdRegu(void)
{
   static int32  PseudoSpdReguOutTmp = 0;


   //转矩模式下伪速度调节器的参数
   PseudoSpeedRegulator.Kf_Q12 = UNI_MTR_FUNCToMTR_List_4Hz_32Bits.List.SpdKf_Q12;           //前馈系数
	//速度闭环中的运算为Fdb(1+DampingKf)，为节省资源提前在主循环中运算好1+DampingKf     ((0-100%)+1)<<12
   PseudoSpeedRegulator.DampingKfPlus1_Q12 = UNI_MTR_FUNCToMTR_List_4Hz_32Bits.List.SpdDampingKf_Q12 + 4096L;      

   PseudoSpeedRegulator.Kp = UNI_MTR_FUNCToMTR_List_16kHz.List.Spd_Kp;                  //比例系数
   PseudoSpeedRegulator.Ki_Q10 = UNI_MTR_FUNCToMTR_List_16kHz.List.Spd_KiQ10;           //积分系数

   //比如当ToqCmd_Limited为负时，伪速度调节器输入值必须可以大于的ToqCmd_Limited的绝对值，
   //否则最后转矩指令无法降低，速度也就无法降低, （ToqCmd_Limited为负时同理）
   //此处转矩限制幅度可比ToqCmd_Limited大一点，因为加入前馈ToqCmd_Limited后还会再进行输出限制

	if(FunCodeUnion.code.TL_SpdLmtReguSel == 0)
	{
	   PseudoSpeedRegulator.PosLmt = 32767L;
	   PseudoSpeedRegulator.NegLmt = -32767L;
	}
	else if(FunCodeUnion.code.TL_SpdLmtReguSel == 1)
	{
	   if(STR_GetIqRef.ToqCmd_Limited > 0)     //伪速度调节器输出限制 
	    {
	        PseudoSpeedRegulator.PosLmt = 0;
	        PseudoSpeedRegulator.NegLmt = -(STR_GetIqRef.ToqCmd_Limited<<1);
	    }
	    else        
	    {
	        PseudoSpeedRegulator.NegLmt = 0;
	        PseudoSpeedRegulator.PosLmt = -(STR_GetIqRef.ToqCmd_Limited<<1);
	    }
	}    

   PseudoSpdReguOutTmp = PDFFCtrl(&PseudoSpeedRegulator,STR_GetIqRef.PseudoSpeedRef,\
                                  STR_MTR_Gvar.FPGA.SpdFdb, STR_MTR_Gvar.FPGA.M_SpdFdb);

    //为保证从转矩模式恒转矩ToqCmd平滑切换至速度限制，伪调节器输出以ToqCmd为初始值，相当于加一个前馈值。
   STR_GetIqRef.PseudoSpdOut = PseudoSpdReguOutTmp + STR_GetIqRef.ToqCmd_Limited;

   //即使在速度限制模式下也保证转矩指令不超过ToqCmd和转矩限制值。

    if((STR_GetIqRef.ToqCmd_Limited >= 0) 
        && (STR_GetIqRef.PseudoSpdOut >= STR_GetIqRef.ToqCmd_Limited))     //伪速度调节器输出限制 
    {
        STR_GetIqRef.SaturaFlag = 2;
        STR_GetIqRef.PseudoSpdOut = STR_GetIqRef.ToqCmd_Limited;
    }
    else if((STR_GetIqRef.ToqCmd_Limited < 0) 
        && (STR_GetIqRef.PseudoSpdOut <= STR_GetIqRef.ToqCmd_Limited))
    {
        STR_GetIqRef.PseudoSpdOut = STR_GetIqRef.ToqCmd_Limited;
        STR_GetIqRef.SaturaFlag = 1;
    }
    else
    {
        STR_GetIqRef.SaturaFlag = 0;

//       if((STR_GetIqRef.ToqCmd_Limited >= 0))  //转矩指令大于零，电机正转
//       {
//            //此时伪速度调节器输出有三种情况：1、伪速度调节器输出为负且绝对值大于转矩指令，此时输出为负的转矩指令
//            //                                2、伪速度调节器输出为负且绝对值小于转矩指令，此时输出为伪速度调节器的输出
//            //                                3、伪速度调节器输出为正其绝对值小于转矩指令，此时输出仍为伪速度调节器的输出
//            if(ABS(STR_GetIqRef.PseudoSpdOut)>=ABS(STR_GetIqRef.ToqCmd_Limited))
//            {
//                STR_GetIqRef.PseudoSpdOut = -STR_GetIqRef.ToqCmd_Limited;
//            }
//       }
//       else   //转矩指令小于零，电机反转
//       {
//            //此时伪速度调节器输出有三种情况：1、伪速度调节器输出为正且绝对值大于转矩指令，此时输出为正的转矩指令
//            //                                2、伪速度调节器输出为正且绝对值小于转矩指令，此时输出为伪速度调节器的输出
//            //                                3、伪速度调节器输出为负其绝对值小于转矩指令，此时输出仍为伪速度调节器的输出
//            if(ABS(STR_GetIqRef.PseudoSpdOut)>=ABS(STR_GetIqRef.ToqCmd_Limited)) 
//            {
//                STR_GetIqRef.PseudoSpdOut = -STR_GetIqRef.ToqCmd_Limited;
//            }
//       }

        if(ABS(STR_GetIqRef.PseudoSpdOut)>=ABS(STR_GetIqRef.ToqCmd_Limited)) 
        {
            STR_GetIqRef.PseudoSpdOut = -STR_GetIqRef.ToqCmd_Limited;
        }
					
    }

   //  为1时只能增加 ，为2时只能减小
   // (PseudoSpeedRegulator.KiSum>>10)*PseudoSpeedRegulator.Kb_Scal_Q38>>38;
	if(FunCodeUnion.code.TL_SpdLmtSatura == 0)
	{
	if((STR_GetIqRef.ToqCmd_Limited >= 0) 
	&& (STR_GetIqRef.PseudoSpdOut <= (-STR_GetIqRef.ToqCmd_Limited)))
	{
		PseudoSpeedRegulator.SaturaFlag = 1;

		PseudoSpeedRegulator.KiSum=
			((int64)(-STR_GetIqRef.ToqCmd_Limited)<<48)/PseudoSpeedRegulator.Kb_Scal_Q38;
	 }
	else if((STR_GetIqRef.ToqCmd_Limited <= 0) 
		   && (STR_GetIqRef.PseudoSpdOut >= (-STR_GetIqRef.ToqCmd_Limited)))
	{
		PseudoSpeedRegulator.SaturaFlag = 2;
					
		PseudoSpeedRegulator.KiSum=
			(((int64)(-(int64)STR_GetIqRef.ToqCmd_Limited)<<38)/PseudoSpeedRegulator.Kb_Scal_Q38)<<10;

	}
	else  if(STR_GetIqRef.PseudoSpdOut>PseudoSpeedRegulator.PosLmt)
	{
	  PseudoSpeedRegulator.SaturaFlag = 2;
	}
	else if(STR_GetIqRef.PseudoSpdOut<PseudoSpeedRegulator.NegLmt)
	{
	  PseudoSpeedRegulator.SaturaFlag = 1;
	}
	else
	{
	  PseudoSpeedRegulator.SaturaFlag = 0;
	}
	}
	else
	{
		if(PseudoSpdReguOutTmp > PseudoSpeedRegulator.PosLmt)
	  	{
	      	PseudoSpeedRegulator.SaturaFlag = 2;
	    }
		else if(PseudoSpdReguOutTmp < PseudoSpeedRegulator.NegLmt)
	  	{
	      	PseudoSpeedRegulator.SaturaFlag = 1;
	    }
		else
	  	{
	      	PseudoSpeedRegulator.SaturaFlag = 0;
	   	}
	}
}



/*******************************************************************************
  函数名:   void PseudoSpdRegu(void)      ^_^
  输入: 功能码STR_GetIqRef.ToqRchStandard  STR_GetIqRef.ToqRchOn  STR_GetIqRef.ToqRchOff  
  输出:   STR_MTR_Gvar.GlobalFlag.bit.DOvarReg_ToqReach转矩到达DO输出
  子函数: 无        
  描述: 该功能原来在电流环中调度，调度判断次数为40次，因DIDO的最后读取处理也是在主循环中执行的。
        所以后将其改至在位置环软中断中调度，主循环调度周期为1K，位置环软中断为4K，所以设计判断次数暂定为10次。
        增加输出功能码FUNOUT18，转矩到达DO输出功能。新增H07.21，转矩到达基准值；
        新增H07.22，转矩到达有效值；新增H07.23，转矩到达无效值。
        当目前的转矩指令的绝对值-H07.21>=H07.22时，FUNOUT.18对应的DO输出有效。
        同理，当目前的转矩指令的绝对值-H07.21<H07.23时,FUNOUT.18对应的DO输出无效。
        三个功能码单位为0.1%电机额定转矩，范围0~300.0%。
        该函数在主循环实时更新 GetIqRefUpdate(void)中调度执行
********************************************************************************/ 
void ToqReachJudge(void)
{ 
    static Uint16   ToqRchCNT = 0;

    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN)
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_ToqReach = 0;
        ToqRchCNT = 0;
        
	    if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTCSTMOD) 
	    {
	        ObjectDictionaryStandard.DeviceControl.StatusWord.bit.TargetReached = 1;
	        
	    }
		else if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTTOQMOD)
		{
		   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.TargetReached = STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_ToqReach;;
		}		
		
		return;
    }

    if((ABS(STR_MTR_Gvar.GetIqRef.IqRef)- STR_GetIqRef.ToqRchStandard ) >= STR_GetIqRef.ToqRchOn)
    {
        ToqRchCNT++;       
        if(ToqRchCNT >= 10)
        { 
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_ToqReach = 1;
            ToqRchCNT = 10;
        }
    } 
    else 
    {
        ToqRchCNT = 0;     
    }

    if((ABS(STR_MTR_Gvar.GetIqRef.IqRef)- STR_GetIqRef.ToqRchStandard) < STR_GetIqRef.ToqRchOff)
    {   
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_ToqReach = 0;
    }
	#if ECT_ENABLE_SWITCH
    if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTCSTMOD) 
    {
        ObjectDictionaryStandard.DeviceControl.StatusWord.bit.TargetReached = 1;
        
    }
	else if(ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay == ECTTOQMOD)
	{
	   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.TargetReached = STR_MTR_Gvar.MTRtoFUNCFlag.bit.DOvarReg_ToqReach;;
	}
	#endif
    
}



/*******************************************************************************
  函数名: extern void SWOvCur_FdbErrMonitor(void)      ^_^    软件er200过流报错监控功能函数
  输入:   Q轴电流和三相电流IqFdb IuFdb IvFdb IwFdb 
  输出:   软件过流报错标志位
  子函数:无         
  描述:   由于IQ的最大值即为相电流最大值（驱动器电流软件过流时的Iqref即为驱动器过流时相电流的最大值）
          因此相电流和Q轴电流的限幅值均为 STR_GetIqRef.CriticalCur
          该函数在G_MTR_Task_WGINT()发波中断运行任务（电流环调度）中调度执行     
********************************************************************************/
//void SWOvCur_FdbErrMonitor(void)     //软件er200过流报错监控功能函数
//{   
//    static Uint16 DelayCnt = 0;
//    //相对于额定值  Iw值不考虑
//    //目前Iq 与 Iu Iv在同一尺度
//
//    if((ABS(STR_MTR_Gvar.FPGA.IqFdb) > STR_GetIqRef.Iq_CriticalCur)
//        ||(ABS(STR_MTR_Gvar.FPGA.IuFdb) > STR_GetIqRef.Iuvw_CriticalCur)
//        ||(ABS(STR_MTR_Gvar.FPGA.IvFdb) > STR_GetIqRef.Iuvw_CriticalCur)
//        ||(ABS(STR_MTR_Gvar.FPGA.IwFdb) > STR_GetIqRef.Iuvw_CriticalCur))
//    {
//        //过流故障
//        DelayCnt ++;  
//        if(DelayCnt >= 3)
//        {
//           *HostSon = DISPWM;
////           PostErrMsg(OVERCURRENT);     //软件过流故障200
//            if(ABS(STR_MTR_Gvar.FPGA.IqFdb) > STR_GetIqRef.Iq_CriticalCur)     PostErrMsg(SOVERCURRENT);
//            else if(ABS(STR_MTR_Gvar.FPGA.IuFdb) > STR_GetIqRef.Iuvw_CriticalCur)   PostErrMsg(S1OVERCURRENT);
//            else if(ABS(STR_MTR_Gvar.FPGA.IvFdb) > STR_GetIqRef.Iuvw_CriticalCur)   PostErrMsg(S2OVERCURRENT);
//            else if(ABS(STR_MTR_Gvar.FPGA.IwFdb) > STR_GetIqRef.Iuvw_CriticalCur)   PostErrMsg(S3OVERCURRENT);
//        }
//    }
//    else
//    {
//        DelayCnt = 0;
//    }
//}

/*******************************************************************************
  函数名:  T_ITrans
  输入:    功能码
  输出:    
  子函数:       
  描述: 由转矩指令计算电流指令
********************************************************************************/ 
Static_Inline int32 T_ITrans(Uint16 val)
{
    int32 temp11 =0;
	int32 Temp=0;

	//老60Z电机需要特殊处理
    if( ((14 == (FunCodeUnion.code.MT_MotorModel / 1000)) && (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 704)) 
     || (FunCodeUnion.code.MT_MotorModel == 704) )
	{
	    if(val>M400WJOINT)
        {
		    temp11 = ((Uint64)M400WFAQ25 * (Uint64)val * val)>>25;
		    temp11 = temp11 + ((Uint32)M400WFBQ15 *(Uint32)val>>15)+M400WFC;
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * temp11) >> 12;
	
        }
	    else 
	    {
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * val) >> 12;       
	    }	
	}
	else if( ((14 == (FunCodeUnion.code.MT_MotorModel / 1000)) && (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 804)) 
          || (FunCodeUnion.code.MT_MotorModel == 804) )
    {
	    if(val>M400WJOINT)
        {
		    temp11 = ((Uint64)M400WFAQ25 * (Uint64)val * val)>>25;
		    temp11 = temp11 + ((Uint32)M400WFBQ15 *(Uint32)val>>15)+M400WFC;
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * temp11) >> 12;
	
        }
	    else 
	    {
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * val) >> 12;       
	    }
    }
	else if( ((14 == (FunCodeUnion.code.MT_MotorModel / 1000)) && (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 703)) 
          || (FunCodeUnion.code.MT_MotorModel == 703) )
    {
 	    if(val>M200WJOINT)
        {
		    temp11 = ((Uint64)M200WFAQ25 * (Uint64)val * val)>>25;
		    temp11 = temp11 + ((Uint32)M200WFBQ15 *(Uint32)val>>15)+M200WFC;
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * temp11) >> 12;
	
        }
	    else 
	    {
	        Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * val) >> 12;       
	    }	
	}
	else
	{   
	    Temp = (STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12 * val) >> 12;    
	}
	return Temp; 	
}
/********************************* END OF FILE *********************************/




