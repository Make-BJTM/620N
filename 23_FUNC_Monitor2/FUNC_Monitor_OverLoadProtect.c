/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:  FUNC_Monitor_OverLoadProtect.C                                                        
 创建人：王军干、熊飞、肖明海   创建日期：2008.11.10
 修改人：朱祥华                 修改日期：2011.11.21 
 描述： 电机、驱动器过载保护
     1.
 修改记录：  
    1. 朱祥华       2011.11.21
       变更内容：过载保护是一个相对独立的功能，因此将其独立成一个文件两个函数供外部调用。
                 方便程序查阅和维护。
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "PUB_Library_Function.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_MotorOverLoad.h"
#include "FUNC_Filter.h"
#include "FUNC_Monitor_OverLoadProtect.h"
#include "FUNC_Monitor_OverLoadTable.h"
#include "FUNC_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define LOCKELESPD        240       //低速对应的电气转速240RPM 电周期为0.25s	

#define LOCKROTORELESPD   50        //堵转时对应的电气转速50RPM  按5对极电机10rpm机械转速计算

//根据驱动器功率段，确定对应过载保护相关曲线选择位
#define SizeE_Over7500w       1     //Size_E  7.5KW 驱动器
#define SizeE_5kwTo6kw        2     //Size_E  5KW 至6KW  驱动器
#define SizeCSizeD_Over1kw    3     //Size - C&D且1kw以上驱动器
#define SizeASizeB_Below1kw   4     //小功率Size - A&B且1kw以下驱动器

#if POWERDRIVER_TYPE==POWDRV_IS650
#define SizeH_Over30kw    8 
#endif   


//OVLoadErrValue为驱动器为从常温至报警温度度需要热量，程序假设其为1
//然后为计算方便，将其放大10的Q20倍 
#define OVLoadErrValue  10485760L            //驱动器、电机累积热量过载保护保护值10<<20
#define OVLoadWarnPara  104857L//(OVLoadErrValue/100)驱动器、电机累积热量过载保护警告值，为OVLoadErrValue的百分比
#define Inver_10Q20     104857L              //1/10的Q20格式



/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义*/  
STR_OverLoadProtect  STR_OVLoadProtect;

STR_BILINEAR_LOWPASS_FILTER   PhaseCurrentDisplayFilter={0};  //相电流低通滤波器
STR_BILINEAR_LOWPASS_FILTER   ValidCurrentFilter;  //有效电流低通滤波器


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
static  Uint16 LockFlag          = 1;   //电机堵转标志位 0～未堵转，1～堵转
Uint32  OEM_OVLoad_CurveT        = 0;   //驱动器允许的过载曲线对应时间
Uint32  OEM_HeatDissipate_CurveT = 0;   //驱动器散热曲线对应时间
Uint16  OEM_CurveIndex           = 0;   //驱动器过载曲线索引值
Uint32  OEM_CurPercent           = 0;   //驱动器电流百分比
Uint32  OEM_DeltaSumHeat         = 0;   //10ms驱动器累积热量值。
//Uint32  OVLoadWarnValue          = 0;   //驱动器、电机累积热量过载保护警告值为OVLoadErrValue百分比

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitOverLoadProtect(void);             //驱动器电机过载保护初始化
void OverLoadProtect_1k_Monitor(void);      //驱动器电机过载保护
void MTOffLine_1k_Monitor(void);             //电机动力线断线监控


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//以下函数在OverLoadProtect()中调用
Static_Inline void OEMOverLoadErrProcess(void);        //驱动器过载报警处理保护
#if POWERDRIVER_TYPE!=POWDRV_IS650
Static_Inline void MTOverLoadErrProcess(void);         //电机过载报警处理保护
#endif
//Size_E  7.5KW 驱动器过载保护处理函数
//Static_Inline void SizeE_Over7500w_OVLoadProtect(void);
//Size_E  5KW 至6KW  驱动器过载保护处理函数
Static_Inline void SizeE_5kwTo6kw_OVLoadProtect(void);
//Size - C&D且1kw以上驱动器过载保护处理函数
Static_Inline void SizeCSizeD_Over1kw_OVLoadProtect(void);
//小功率Size - A&B且1kw以下驱动器过载保护处理函数
Static_Inline void SizeASizeB_Below1kw_OVLoadProtect(void);

//平均负载率的计算10S为一个周期
Static_Inline void AvrLoadMonitor(Uint32 Load10ms);

//检测电机堵转
Static_Inline void LockedRotorDetection(void);
#if POWERDRIVER_TYPE==POWDRV_IS650
Static_Inline void SizeH_30KW_OVLoadProtect(void);
#endif

/*******************************************************************************
  函数名:  AvrLoadMonitor
  输入: 电流有效值load10ms(10ms内的平均值单位标幺)DP_USampCur
  输出:   电机的平均负载率DP_AvrLoad
  子函数:  无
  描述:  5S内的平均负载大小，相对额定电流，单位0.1% 
********************************************************************************/ 
Static_Inline void AvrLoadMonitor(Uint32 Load10ms)
{
    static Uint32 SumCur160ms = 0; //总电流累加值0.01A     
    static Uint32 AvrLoadArray[32] = {0};
    static Uint16 tim160ms = 0; //时间计算器
    static Uint32 SumCur   = 0;
	static Uint8  i        = 0;
    Uint32 tempLoad = 0; 

    SumCur160ms += ((Load10ms*Load10ms)>>10);//使用Q7格式input Q12;

    if(tim160ms++ == 31) //大致320ms更新一次平均负载
    {
        tempLoad  = SumCur160ms;//(SumCur160ms>>5);
            
        SumCur160ms = 0;
        tim160ms    = 0;

        SumCur = SumCur + tempLoad -AvrLoadArray[i];
		AvrLoadArray[i] = tempLoad ;
		
		i++;
		if(i>=32)
		{
			i= 0 ;
		}

        //更新平均负载显示0.1%.使用电机标幺值4096=MT_RateCurrent
        AuxFunCodeUnion.code.DP_AvrLoad = (qsqrt32(SumCur >> 10)*1000)>>7;//Q7 Q12;// /FunCodeUnion.code.MT_RateCurrent;
    }    
    
    STR_FUNC_Gvar.OscTarget.Osc_AvrLoad = AuxFunCodeUnion.code.DP_AvrLoad;    
}


/*******************************************************************************
  函数名:  LockedRotorDetection()
  输入:   
  输出:   
  子函数:  无
  描述:  检测电机堵转，报警ER630 
********************************************************************************/ 
Static_Inline void LockedRotorDetection()
{
	Uint8  LockSpeedFlag  = 0;
	Uint8  LockTorqueFlagP = 0;
    Uint8  LockTorqueFlagN = 0;
    Uint32 IqMaxP = 0;     //堵转过热保护，正向电流上限值。
    Uint32 IqMaxN = 0;     //堵转过热保护，反向电流上限值。
    int32  IqRefTemp = 0; 
	static Uint16 LockCnt = 0; //1ms计数一次

    IqRefTemp = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef;

	//若IqPosLmt，IqNegLmt 均大于80%驱动器
	//,电机最大电流中的最小值，去该两值中的最小者
	if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD)
    {
        if(ABS(IqRefTemp) >= STR_OVLoadProtect.LockRotorIqThreshold)
        {
            IqMaxP = ABS(IqRefTemp);
            IqMaxN = IqMaxP;
        }
        else
        {
            IqMaxP = STR_OVLoadProtect.LockRotorIqThreshold;
            IqMaxN = STR_OVLoadProtect.LockRotorIqThreshold; 
        }
    }
    else
    {
        if(UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqPosLmt_F >= STR_OVLoadProtect.LockRotorIqThreshold)
        {
            IqMaxP = UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqPosLmt_F - 10; 
        }
        else
        {
            IqMaxP = STR_OVLoadProtect.LockRotorIqThreshold ;
        }

        if(ABS(UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqNegLmt_F) >= STR_OVLoadProtect.LockRotorIqThreshold)
        {
            IqMaxN = ABS(UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqNegLmt_F) - 10;
        }
        else
        {
            IqMaxN = STR_OVLoadProtect.LockRotorIqThreshold ;
        }
	}

    if(FunCodeUnion.code.ER_LockedRotorSel==1) 
	{
		//判断标志位
		LockSpeedFlag  = (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < STR_OVLoadProtect.LockRotorSpdThreshold) ? 1 : 0;                //小于10rpm则认为堵转
        
        if(IqRefTemp >= 0)
        {
            LockTorqueFlagP = (IqRefTemp >= IqMaxP) ? 1 : 0;
        }
        else
        {
		    //转矩大于等于限幅值
		    LockTorqueFlagN = ((-IqRefTemp) >= IqMaxN) ? 1 : 0;
        }

		if((1==LockSpeedFlag)&&((1==LockTorqueFlagP)||(1==LockTorqueFlagN)))
		{
			LockCnt++;
			if(LockCnt>=FunCodeUnion.code.ER_LockedRotorTimer)
			{
				LockCnt=FunCodeUnion.code.ER_LockedRotorTimer ;
				PostErrMsg(MTOLOCKROTOR);
			}
		}
		else
		{
		    LockCnt=0;  
		}
	}
	else
	{
		LockCnt=0;
	} 
}


/*******************************************************************************
  函数名:  InitOverLoadProtect(void)     上电时初始化与过载保护相关配置（务必置于电流环相关和功能码初始化之后）    ^_^
  输入:    
  输出:    (本文件用)驱动器、电机运行电流百分比系数   根据驱动器功率选择过载保护相关曲线
  子函数:  无
  描述:    初始化时计算驱动器、电机运行电流百分比系数,并初始化驱动器过载保护曲线

  暂用输入：
   
********************************************************************************/ 
void InitOverLoadProtect(void)
{
    Uint32 Temp = 0;     

    //当输出电机相有效值电流为驱动器额定电流H0107时IqRef的输入值（即IqRate_OEM）；
    STR_OVLoadProtect.IqRate_OEM = 
         (int32)((((int64)1000 * FunCodeUnion.code.OEM_RateCurrent <<10)
              / FunCodeUnion.code.MT_RateCurrent)  >> 10);

    STR_OVLoadProtect.Inver_IqRateOEM_Q24 = (16777216L)/STR_OVLoadProtect.IqRate_OEM ; //Q24
    //电机降低额定电流值的倒数 (1/IqRateMT)*降低额度 16777216L = IQ24(1);
    STR_OVLoadProtect.Inver_IqRateMT_Q24  = (16777216L) / 1000;
    
    //当反馈的相有效电流小于10%额定电流且给定转矩指令达到限幅点且反馈转速小于25%额定转速多次则认为电机动力线断线
    STR_OVLoadProtect.OffLineJudge_Is = 1000  / 10; 
    //当反馈的相有效电流小于10%额定电流且给定转矩指令达到限幅点且反馈转速小于25%额定转速多次则认为电机动力线断线
    STR_OVLoadProtect.OffLineJudge_SpdFdb = (int32)FunCodeUnion.code.MT_RateSpd * 2500;
    //当限幅电流小于50%的额定电流时则不进行判断电机动力线断线
    STR_OVLoadProtect.OfflineJudge_Iqlmt = 1000  >> 1;
    

    //根据驱动器功率段，确定对应过载保护相关曲线选择位
#if POWERDRIVER_TYPE==POWDRV_IS650
	if((FunCodeUnion.code.OEM_ServoSeri >= 20000)	//30KW以上驱动器保护曲线一致
		  &&(FunCodeUnion.code.OEM_ServoSeri <= 200020))
	{
	   STR_OVLoadProtect.OVLoadCurveSel = SizeH_Over30kw;
	}
    else
	{
	   STR_OVLoadProtect.OVLoadCurveSel = SizeH_Over30kw;
	}
#else
    if((FunCodeUnion.code.OEM_ServoSeri == 10007)
	 ||	(FunCodeUnion.code.OEM_ServoSeri == 10))   //Size_E  7.5KW 驱动器
    {
        STR_OVLoadProtect.OVLoadCurveSel = SizeE_Over7500w;
    }
    else if( (FunCodeUnion.code.OEM_ServoSeri == 8) 
     ||(FunCodeUnion.code.OEM_ServoSeri == 9) 
	 ||(FunCodeUnion.code.OEM_ServoSeri == 10005)
     ||(FunCodeUnion.code.OEM_ServoSeri == 10006) ) //Size_E  2KW 5KW 6KW  驱动器
    {
        STR_OVLoadProtect.OVLoadCurveSel = SizeE_5kwTo6kw;
    }
    else if( (FunCodeUnion.code.OEM_ServoSeri == 6)
     || (FunCodeUnion.code.OEM_ServoSeri == 7)
     || (FunCodeUnion.code.OEM_ServoSeri == 10001)
     || (FunCodeUnion.code.OEM_ServoSeri == 10002)               
     || (FunCodeUnion.code.OEM_ServoSeri == 10003)
     || (FunCodeUnion.code.OEM_ServoSeri == 10004) )   //Size - C&D且1kw以上驱动器
    {
        STR_OVLoadProtect.OVLoadCurveSel = SizeCSizeD_Over1kw;
    }
    else   //小功率Size - A&B且1kw以下驱动器
    {
        STR_OVLoadProtect.OVLoadCurveSel = SizeASizeB_Below1kw;
    }
#endif

    /* H0B24 相电流显示用的滤波器初始化 */
    PhaseCurrentDisplayFilter.Ts = 1000;     //相电流显示滤波的Ts采样时间按1K进行采样
    PhaseCurrentDisplayFilter.Tc = 10000;   // tc = 10ms
    InitLowPassFilter(&PhaseCurrentDisplayFilter);

    ValidCurrentFilter.Ts = 1000;     //相有效电流滤波的Ts采样时间按1K进行采样
    ValidCurrentFilter.Tc = 4000;   // tc = 4ms
    InitLowPassFilter(&ValidCurrentFilter);

	InitMotorOverLoadProc(); 
    
	//驱动器最大电流的80%、电机2.4倍额定电流最小值  单位0.1%
    Temp = (Uint32)FunCodeUnion.code.OEM_MaxCurrentOut * 800 / (Uint32)FunCodeUnion.code.MT_RateCurrent;

     if((FunCodeUnion.code.OEM_ServoSeri>=20014)&&(FunCodeUnion.code.OEM_ServoSeri<=20015))
     {
         Temp=10L*Temp;  //最大电流扩大了单位扩大了
     }
	STR_OVLoadProtect.LockRotorIqThreshold = MIN(2400,Temp);

    STR_OVLoadProtect.LockRotorSpdThreshold =  LOCKROTORELESPD / FunCodeUnion.code.MT_PolePair; 

    STR_OVLoadProtect.LockSpdThreshold =  LOCKELESPD / FunCodeUnion.code.MT_PolePair; 
}


/*******************************************************************************
  函数名:  OverLoadProtect(void)     驱动器以及电机过载保护（主循环1ms调度运算）    ^_^
  输入:    
  输出:    驱动器、电机过载警告报警标志位
  子函数:  OEMOverLoadErrProcess();   //驱动器过载报警处理保护
           MTOverLoadErrProcess();    //电机过载报警处理保护
  描述:  驱动器以及电机过载保护
         调用该函数的地方：主循环1ms调度
         过载监控程序10ms监控一次，定子电流则1ms计算一次， 
         过载保护时取计算十次电流的平均值
********************************************************************************/ 
void OverLoadProtect_1k_Monitor(void)   
{
    static  Uint64 AbsIsFdbSum = 0;
    Uint64      OverLoadTemp = 0;
    static  Uint16 OverLoadProtect_Cnt = 0;
    int32   PhaseCurrEffValue;          //相电流有效值

#if POWERDRIVER_TYPE!=POWDRV_IS650
    int32  CurBaseMT_Q10;
#endif

    //定子电流反馈值Is计算   AbsIsFdb = sqrt(IqFdb^2 + IdFdb^2)  电流反馈数据最大为32767，
    OverLoadTemp = ((Uint64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb * UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb 
                  + (Uint64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb * UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb);

    if(OverLoadTemp > 1073741824L)       //IqFdb最大32768，因此OverLoadTemp不超过2^30=1073741824
    {
        OverLoadTemp = 1073741824L;
    }
    STR_OVLoadProtect.IsFdb_1ms = qsqrt((int32)OverLoadTemp); //相电流有效值

    AbsIsFdbSum +=  STR_OVLoadProtect.IsFdb_1ms; //10ms内计算累积10次的电流反馈和其可能最大为327680L（19位数）

    PhaseCurrEffValue = STR_OVLoadProtect.IsFdb_1ms;       //量纲数字量

    OverLoadProtect_Cnt++;     //计数10次，
    if(OverLoadProtect_Cnt == 10)
    {
        OverLoadProtect_Cnt = 0;
        //过载监控程序10ms监控一次，定子电流则1ms计算一次，除10求平均值
        OverLoadTemp = (Uint32)((AbsIsFdbSum * Inver_10Q20)>>20);  
        AbsIsFdbSum     = 0;  //计算后清零

        //将除以IqRateOEM转换成其倒数的Q24，即Inver_IqRateOEM_Q24，放至初始化中更新一次即可。
        //实际电流与驱动器额定电流比值的q12 目前取定子电流反馈作为驱动器过载的判断标准
        STR_OVLoadProtect.CurBaseOEM_Q12 = (Uint32)(((Uint64)OverLoadTemp * STR_OVLoadProtect.Inver_IqRateOEM_Q24) >> 12);
        //反馈电流与电机额度电流的 比值的q12  取定子电流反馈做为判断标准
        STR_OVLoadProtect.CurBaseMT_Q12 = (Uint32)(((Uint64)OverLoadTemp * STR_OVLoadProtect.Inver_IqRateMT_Q24) >> 12);

        // OVLoadWarnPara = OVLoadErrValue/100;  //驱动器、电机过载警告系数
        //OVLoadWarnValue = (Uint32)OVLoadWarnPara * FunCodeUnion.code.ER_OLWarnValue;   //过载警告时间

        OEMOverLoadErrProcess();         //驱动器过载警告报错保护处理程序

#if POWERDRIVER_TYPE!=POWDRV_IS650
		if((FunCodeUnion.code.MT_RatePower != 40)//400W 
		&&(FunCodeUnion.code.MT_RatePower != 20))//200W
        {
        	MTOverLoadErrProcess();          //电机过载警告报错保护处理程序
		}
#endif
		
        AvrLoadMonitor(STR_OVLoadProtect.CurBaseMT_Q12);
    }

	//电机堵转过热保护
	LockedRotorDetection();

#if POWERDRIVER_TYPE!=POWDRV_IS650
	//=======过载保护新算法===============	
	//60Z电机400W
	if((FunCodeUnion.code.MT_RatePower == 40)//400W 
	  ||(FunCodeUnion.code.MT_RatePower == 20))//200W
	{
		ValidCurrentFilter.Input= (Uint32)(((Uint64)STR_OVLoadProtect.IsFdb_1ms * STR_OVLoadProtect.Inver_IqRateMT_Q24) >> 14);
		LowPassFilter(&ValidCurrentFilter);
		CurBaseMT_Q10 = (Uint16)ValidCurrentFilter.Output;	//电流有效值

	    MotorHeatCalc(CurBaseMT_Q10);
	}
#endif
	//=======================================
    //进行了数字量到0.01A的单位换算
    PhaseCurrEffValue = (PhaseCurrEffValue * (Uint32)FunCodeUnion.code.MT_RateCurrent + 500) / 1000;

    // 相电流显示滤波 
    PhaseCurrentDisplayFilter.Input = PhaseCurrEffValue;
    LowPassFilter(&PhaseCurrentDisplayFilter);
    AuxFunCodeUnion.code.DP_USampCur = (Uint16)PhaseCurrentDisplayFilter.Output;         //H0B24 相电流有效值
}


/*******************************************************************************
  函数名:  MTOverLoadErrProcess(void)     电机过载保护报警处理程序    ^_^
  输入:    
  输出:    电机过载报警标志位
  子函数:  暂无         
  描述:  
********************************************************************************/ 
#if POWERDRIVER_TYPE!=POWDRV_IS650
Static_Inline void MTOverLoadErrProcess(void)
{
    Uint32  MT_OVLoad_CurveT = 0;          //允许的过载曲线对应时间
    Uint16  MT_CurveIndex = 0;            //过载曲线索引值
    Uint32  MT_CurPercent = 0;            //电流百分比
    Uint32  MT_HeatDissipate = 0;      //电机累积热量警告值

    //将实际电流转换为电机额定线电流有效值的百分数 
    //反馈电流与电机额定电流的比值百分数    单位0.1%
    MT_CurPercent = ((Uint32)STR_OVLoadProtect.CurBaseMT_Q12 * 1000) >> 12;

//假设电机为从常温至过载报警需要热量为1，计算在小于60%电流持续10ms(监控周期),
//在实际电流小于75%额定电流，持续10ms（监控周期）散发的热量（以10_Q20进行标幺）
//(0.01S/70S)*10_Q20=1497L
    if(MT_CurPercent < 1150)    //当线电流有效值在115%下时则为散热过程 计算每10ms内的散热量 
    {
        if(MT_CurPercent > 1000)
        {//电流在此条件下从过载保护温度降至常温需要时间210S,(0.01S/210S)*10_Q20=499  （以10_Q20进行标幺）
            MT_HeatDissipate = 499;                           
        }
        else if(MT_CurPercent > 750)
        {//电流在此条件下从过载保护温度降至常温需要时间105S,(0.01S/105S)*10_Q20=998L  （以10_Q20进行标幺）
            MT_HeatDissipate = 998;
        }
        else
        {//电流在此条件下从过载保护温度降至常温需要时间70S,(0.01S/70S)*10_Q20=1497L  （以10_Q20进行标幺）
            MT_HeatDissipate = 1497;
        }

        if(STR_OVLoadProtect.SumHeatMT10_Q20 > MT_HeatDissipate)
        {
            STR_OVLoadProtect.SumHeatMT10_Q20 -= MT_HeatDissipate;
        }
        else  STR_OVLoadProtect.SumHeatMT10_Q20 = 0;
    }
    else   //当线电流有效值在115%以上时则为电机发热过程   先确定发热曲线时间
    {
        if(MT_CurPercent > 2942) //电流有效值越大，从常温到报警点所需时间就越短。
        {
            MT_OVLoad_CurveT = 60;
        }
        else
        {
            MT_CurveIndex = (MT_CurPercent -1150) >> 6;
            MT_CurveIndex = MAX_MIN_LMT(MT_CurveIndex,(Motor_OVLoadTableNum-2),0);
            //曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))  其中Y为过载曲线时间 X代表电流百分比 
            //其中X表格查询的最小单位为每份64（代表6.4%的额定转矩），所以X2-X1＝64   X1=1150+OEM_CurveIndex*64
            MT_OVLoad_CurveT = ((Uint32)(Motor_OVLoadTable[MT_CurveIndex] - Motor_OVLoadTable[MT_CurveIndex+1]) *
                                (MT_CurPercent - 1150 - MT_CurveIndex * 64)) >> 6;    //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
            MT_OVLoad_CurveT  =  (Uint32)Motor_OVLoadTable[MT_CurveIndex] - MT_OVLoad_CurveT; //计算Yi=Y1-dY

            MT_OVLoad_CurveT =MAX_MIN_LMT(MT_OVLoad_CurveT,60000L,60L); 
            if(MT_CurPercent < 1982)
            {
                MT_OVLoad_CurveT = MT_OVLoad_CurveT * FunCodeUnion.code.ER_MotOLProtect_Kp/100;
            }
        }                          
        //假设电机为从常温至过载报警需要热量为1，则在持续恒定的X%电流运行情况下，其产生1热量所需的时间
        //查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
        //即为0.01s/(MT_OVLoad_CurveT/10)=1/(MT_OVLoad_CurveT*10)，将其放大10的Q20倍   
        STR_OVLoadProtect.SumHeatMT10_Q20 += ((Uint32)1<<20)/MT_OVLoad_CurveT; 
    }

    //OVLoadErrValue为驱动器为从常温至报警温度度需要热量，程序假设其为1
    //然后为计算方便，将其放大10的Q20倍 
    if(FunCodeUnion.code.ER_MTOLClose != 1)      //默认为零则开启电机过载保护
    {   
        //电机过载警告
        //if((STR_OVLoadProtect.SumHeatMT10_Q20 > OVLoadWarnValue)&&
        //    (STR_OVLoadProtect.SumHeatMT10_Q20 <= OVLoadErrValue))     //OVLoadErrValue=10<<20
        //{
            //PostErrMsg(MTOLWARN);        //电机过载警告
        //}
        //else
        //{
            //WarnAutoClr(MTOLWARN);      //清除电机过载警告
        //}

        //电机过载故障
        if(STR_OVLoadProtect.SumHeatMT10_Q20 > OVLoadErrValue)
        {
            PostErrMsg(MTOL1);          //电机过载报错
        }
    }  
}
#endif

/*******************************************************************************
  函数名:  OEMOverLoadErrProcess(void)     驱动器过载保护报警处理程序    ^_^
  输入:    
  输出:    驱动器过载报警标志位
  子函数:  ①Size_E7.5KW驱动器过载保护处理程序，            SizeE_Over7500w_OVLoadProtect()
           ②Size_E5KW至6KW 驱动器过载保护处理程序，        SizeE_5kwTo6kw_OVLoadProtect();
           ③Size_C&D且1kw以上驱动器过载保护处理程序，      SizeE_Over7500w_OVLoadProtect();
           ④小功率Size_A&B且1kw以下驱动器过载保护处理程序，SizeE_Over7500w_OVLoadProtect();
  描述:    根据驱动器硬件结构特点，过载保护表单按功率型号分为四类，故驱动器保护程序需根据类型选择
           也分为四个,程序根据驱动器功率段，确定对应散热曲线,另可根据驱动器情况添加新的过载保护曲线
********************************************************************************/ 
Static_Inline void OEMOverLoadErrProcess(void)
{
    //将实际电流转换为驱动器额定的百分数，单位0.1%
    OEM_CurPercent = ((Uint32)STR_OVLoadProtect.CurBaseOEM_Q12 * 1000) >> 12;

//根据驱动器功率段，确定对应散热曲线,可根据驱动器情况添加新的过载保护曲线
    switch (STR_OVLoadProtect.OVLoadCurveSel)
    {

    #if POWERDRIVER_TYPE==POWDRV_IS650
		case SizeH_Over30kw:     
             SizeH_30KW_OVLoadProtect();
			 break;
    #endif    
        case SizeE_Over7500w:          //Size_E  7.5KW 驱动器
             SizeE_5kwTo6kw_OVLoadProtect();
             break;

        case SizeE_5kwTo6kw:           //Size_E  5KW 至6KW  驱动器
             SizeE_5kwTo6kw_OVLoadProtect();
             break;

        case SizeCSizeD_Over1kw:       //Size - C&D且1kw以上驱动器
             SizeCSizeD_Over1kw_OVLoadProtect();
             break;

        case SizeASizeB_Below1kw:      //小功率Size - A&B且1kw以下驱动器
             SizeASizeB_Below1kw_OVLoadProtect();
             break;

      default: 
             PostErrMsg(DEFUALTERR);    //系统参数出故障
             break;
    }

    //OVLoadErrValue为驱动器为从常温至报警温度度需要热量程序假设其为1
    //然后为计算方便，将其放大10的Q20倍 
    //if((STR_OVLoadProtect.SumHeatOEM10_Q20 > OVLoadWarnValue)&&
    //   (STR_OVLoadProtect.SumHeatOEM10_Q20 <= OVLoadErrValue))
    //{               
        //PostErrMsg(OEMOLWARN);       //驱动器过载报警
    //}
    //else
    //{
        //WarnAutoClr(OEMOLWARN);     //清除驱动器过载报警
    //}
    if(FunCodeUnion.code.ER_MTOLClose < 2)
    {
        if((STR_OVLoadProtect.SumHeatOEM10_Q20) > OVLoadErrValue)
        {
            //故障状态      
            PostErrMsg(OEMOL);         //驱动器过载报错
        }
    }
}

#if POWERDRIVER_TYPE==POWDRV_IS650
/*******************************************************************************
  函数名:  SizeH_30KW_OVLoadProtect(void)       ^_^
  输入:  
  输出:   驱动器每监控周期热量累积积分值STR_OVLoadProtect.SumHeatOEM10_Q20
  子函数:  无
  描述:    假设驱动器为从常温至75度需要热量为1，则当在持续恒定的X%电流运行情况下，其产生1热量所需的时间
      查表得为XXXs，那么在X%电流持续10ms(过载保护监控周期),其所获取的热量为0.01S/XXXs
      若在重载情况下则OEM获取热量0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，并将其放大10的Q20倍 
      若在轻载情况下则OEM散发热量0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，
      然后为计算方便，将其放大10的Q20倍，最后将当次贡献的热量计入驱动器每监控周期热量累积积分值
      STR_OVLoadProtect.SumHeatOEM10_Q20    
********************************************************************************/ 
Static_Inline void SizeH_30KW_OVLoadProtect(void)
{   //由于实际测试时发现7.5KW在115%恒定额定电流下运行温度仍会持续上升
    Uint32 Freq001HZ=0;
	Uint32 Utepm11=0;
	
	//由于实际测试时发现7.5KW在115%恒定额定电流下运行温度仍会持续上升
    //所以30kw机器数据实验测定时以102.4%为散热、过载保护曲线分界点
    if(OEM_CurPercent < 1150)   //实际线电流有效值与驱动器电流额定比小于108.8%时 散热曲线
    {  //查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
	    if(OEM_CurPercent <=1024)
		{
			//查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
        OEM_CurveIndex = (OEM_CurPercent) >> 6;

        OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeH30_LightLoadTableNum - 2),0);

        OEM_HeatDissipate_CurveT  = SizeH30_LightLoadTable[OEM_CurveIndex];

        //对查表单得出保护曲线时间根据实测值进行限幅处理，以保证查错表单时不会偏差太多
	        OEM_HeatDissipate_CurveT = MAX_MIN_LMT(OEM_HeatDissipate_CurveT,792,42);
        //假设驱动器为从报警温度降至常温需要散发热量为1，则在X恒定的持续恒定的X%电流运行情况下，其
        //散发1热量所需的时间查表得为XXXs，那么在XX%电流持续10ms(监控周期),其所散发的热量为0.01S/XXXS
        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍后即为
        //(10<<20)/(OEM_HeatDissipate_CurveT*10)=(1<<20)/(OEM_HeatDissipate_CurveT)即从报警温度降至常温需要散发热量放大至10<<20 
        OEM_DeltaSumHeat = ((Uint32)1<<20)/OEM_HeatDissipate_CurveT;
        if(STR_OVLoadProtect.SumHeatOEM10_Q20 > OEM_DeltaSumHeat)
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 -= OEM_DeltaSumHeat; 
        }
        else
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 = 0;
	        } 
		}
		else  //热量不耗散
		{
		    OEM_DeltaSumHeat=0;   
        } 
    }                
    else       //实际线电流有效值与驱动器电流额定比大于102.2%时，驱动器获取热量
    {
//        if(OEM_CurPercent > 2494)
//        {
//            OEM_OVLoad_CurveT = 3;
//        }
//        else
        {
            
            Freq001HZ = ((Uint32)ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt)*FunCodeUnion.code.MT_PolePair)*100/60; //保留0.01HZ精度

			LockFlag = Freq001HZ < 500 ? 1 : 0;   //小于5HZ则认为堵转
            
            if(LockFlag)    //过载保护堵转曲线
            {
			    Utepm11   = ((Uint32)13*Freq001HZ) + 26214;
			    OEM_CurPercent = (OEM_CurPercent<<15)/Utepm11; 
            }

            OEM_CurveIndex = (OEM_CurPercent -1150) >> 6;
			OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeH30_OVLoadTableNum - 2),0);
            OEM_OVLoad_CurveT  =  ((Uint32)(SizeH30_OVLoadTable[OEM_CurveIndex] - SizeH30_OVLoadTable[OEM_CurveIndex+1]) *
                                  (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;        //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
            OEM_OVLoad_CurveT  =  (Uint32)SizeH30_OVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT;   //计算Yi=Y1-dY
           
            OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,36000,5);

			
			
        }
        //假设驱动器为从常温至75度需要热量为1，则在持续恒定的X%电流运行情况下，其产生1热量所需的时间
        //查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
        //即为0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，将其放大10的Q20倍   
        STR_OVLoadProtect.SumHeatOEM10_Q20 += ((Uint32)1<<20)/OEM_OVLoad_CurveT;
    }
}
#endif


/*******************************************************************************
  函数名:  SizeE_Over7500w_OVLoadProtect(void)     Size_E  7.5KW 驱动器过载保护处理程序    ^_^
  输入:  
  输出:   驱动器每监控周期热量累积积分值STR_OVLoadProtect.SumHeatOEM10_Q20
  子函数:  无
  描述:    假设驱动器为从常温至75度需要热量为1，则当在持续恒定的X%电流运行情况下，其产生1热量所需的时间
      查表得为XXXs，那么在X%电流持续10ms(过载保护监控周期),其所获取的热量为0.01S/XXXs
      若在重载情况下则OEM获取热量0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，并将其放大10的Q20倍 
      若在轻载情况下则OEM散发热量0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，
      然后为计算方便，将其放大10的Q20倍，最后将当次贡献的热量计入驱动器每监控周期热量累积积分值
      STR_OVLoadProtect.SumHeatOEM10_Q20    
********************************************************************************/ 
//Static_Inline void SizeE_Over7500w_OVLoadProtect(void)
//{   //由于实际测试时发现7.5KW在115%恒定额定电流下运行温度仍会持续上升
//    //所以7.5kw机器数据实验测定时以102.4%为散热、过载保护曲线分界点
//    if(OEM_CurPercent < 1022)   //实际线电流有效值与驱动器电流额定比小于102.2%时 散热曲线
//    {  //查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
//        OEM_CurveIndex = (OEM_CurPercent) >> 6;
//
//        OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE75_LightLoadTableNum - 2),0);
//
//        OEM_HeatDissipate_CurveT  = SizeE75_LightLoadTable[OEM_CurveIndex];
//
//        //对查表单得出保护曲线时间根据实测值进行限幅处理，以保证查错表单时不会偏差太多
//        OEM_HeatDissipate_CurveT = MAX_MIN_LMT(OEM_HeatDissipate_CurveT,179,60);
//        //假设驱动器为从报警温度降至常温需要散发热量为1，则在X恒定的持续恒定的X%电流运行情况下，其
//        //散发1热量所需的时间查表得为XXXs，那么在XX%电流持续10ms(监控周期),其所散发的热量为0.01S/XXXS
//        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍后即为
//        //(10<<20)/(OEM_HeatDissipate_CurveT*10)=(1<<20)/(OEM_HeatDissipate_CurveT)即从报警温度降至常温需要散发热量放大至10<<20 
//        OEM_DeltaSumHeat = ((Uint32)1<<20)/OEM_HeatDissipate_CurveT;
//        if(STR_OVLoadProtect.SumHeatOEM10_Q20 > OEM_DeltaSumHeat)
//        {
//            STR_OVLoadProtect.SumHeatOEM10_Q20 -= OEM_DeltaSumHeat; 
//        }
//        else
//        {
//            STR_OVLoadProtect.SumHeatOEM10_Q20 = 0;
//        } 
//    }                
//    else       //实际线电流有效值与驱动器电流额定比大于102.2%时，驱动器获取热量
//    {
////        if(OEM_CurPercent > 2494)
////        {
////            OEM_OVLoad_CurveT = 3;
////        }
////        else
//        {
//            LockFlag = (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < STR_OVLoadProtect.LockSpdThreshold) ? 1 : 0;   //小于60rpm则认为堵转
//            OEM_CurveIndex = (OEM_CurPercent -1022) >> 6;
//            if(LockFlag)    //过载保护堵转曲线
//            {   
//                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE75_LockOVLoadTableNum - 2),0);
//                //曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))  其中Y为过载曲线时间 X代表电流百分比 
//                //其中X表格查询的最小单位为每份64（代表6.4%的额定转矩），所以X2-X1＝64   X1=1150+OEM_CurveIndex*64
//                OEM_OVLoad_CurveT = ((Uint32)(SizeE75_LockOVLoadTable[OEM_CurveIndex] - SizeE75_LockOVLoadTable[OEM_CurveIndex+1]) *
//                                    (OEM_CurPercent - 1022 - OEM_CurveIndex * 64)) >> 6;    //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
//                OEM_OVLoad_CurveT  =  (Uint32)SizeE75_LockOVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT; //计算Yi=Y1-dY
//
//                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,3287,5);
//            }
//            else   //不堵转采用的曲线
//            { 
//                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE75_OVLoadTableNum - 2),0);
//                OEM_OVLoad_CurveT  =  ((Uint32)(SizeE75_OVLoadTable[OEM_CurveIndex] - SizeE75_OVLoadTable[OEM_CurveIndex+1]) *
//                                      (OEM_CurPercent - 1022 - OEM_CurveIndex * 64)) >> 6;        //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
//                OEM_OVLoad_CurveT  =  (Uint32)SizeE75_OVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT;   //计算Yi=Y1-dY
//               
//                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,3658,5);
//            }
//        }
//        //假设驱动器为从常温至75度需要热量为1，则在持续恒定的X%电流运行情况下，其产生1热量所需的时间
//        //查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
//        //即为0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，将其放大10的Q20倍   
//        STR_OVLoadProtect.SumHeatOEM10_Q20 += ((Uint32)1<<20)/OEM_OVLoad_CurveT;
//    }
//}


/*******************************************************************************
  函数名:  SizeE_5kwTo6kw_OVLoadProtect(void)     Size_E 5KW至6KW 驱动器过载保护处理程序   ^_^
  输入:   实际线电流有效值与驱动器电流额定比OEM_CurPercent
  输出:   驱动器每监控周期热量累积积分值STR_OVLoadProtect.SumHeatOEM10_Q20
  子函数:  无
  描述:    假设驱动器为从常温至75度需要热量为1，则当在持续恒定的X%电流运行情况下，其产生1热量所需的时间
      查表得为XXXs，那么在X%电流持续10ms(过载保护监控周期),其所获取的热量为0.01S/XXXs
      若在重载情况下则OEM获取热量0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，并将其放大10的Q20倍 
      若在轻载情况下则OEM散发热量0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，
      然后为计算方便，将其放大10的Q20倍，最后将当次贡献的热量计入驱动器每监控周期热量累积积分值
      STR_OVLoadProtect.SumHeatOEM10_Q20    
********************************************************************************/ 
Static_Inline void SizeE_5kwTo6kw_OVLoadProtect(void)
{   
    //对查表单得出保护曲线时间根据实测值进行限幅处理，以保证查错表单时不会偏差太多
    //散热曲线
    if(OEM_CurPercent < 1150)  //当实际线电流有效值与驱动器电流额定比小于115%时
    {  //查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
        OEM_CurveIndex = (OEM_CurPercent) >> 6;

        OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE5060_LightLoadTableNum - 2),0);

        OEM_HeatDissipate_CurveT  = SizeE5060_LightLoadTable[OEM_CurveIndex];

        OEM_HeatDissipate_CurveT =MAX_MIN_LMT(OEM_HeatDissipate_CurveT,164,37);
        //假设驱动器为从报警温度降至常温需要散发热量为1，则在持续恒定的X%电流运行情况下，其
        //散发1热量所需的时间查表得为XXXs，那么在XX%电流持续10ms(监控周期),其所散发的热量为0.01S/XXXS
        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍   
        OEM_DeltaSumHeat = ((Uint32)1<<20)/OEM_HeatDissipate_CurveT;
        if(STR_OVLoadProtect.SumHeatOEM10_Q20 > OEM_DeltaSumHeat)
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 -= OEM_DeltaSumHeat; 
        }
        else
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 = 0;
        }
    }
    else       //实际线电流有效值与驱动器电流额定比大于102.2%时，驱动器获取热量
    {
//        if(OEM_CurPercent > 2494)
//        {
//            OEM_OVLoad_CurveT = 6;
//        }
//        else
        {
            LockFlag = (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < STR_OVLoadProtect.LockSpdThreshold) ? 1 : 0;  //小于60rpm则认为堵转
            OEM_CurveIndex = (OEM_CurPercent -1150) >> 6;
            if(LockFlag)    //堵转曲线
            {   
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE5060_LockOVLoadTableNum - 2),0);
                //曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))  其中Y为过载曲线时间 X代表电流百分比 
                //其中X表格查询的最小单位为每份64（代表6.4%的额定转矩），所以X2-X1＝64   X1=1150+OEM_CurveIndex*64
                OEM_OVLoad_CurveT = ((Uint32)(SizeE5060_LockOVLoadTable[OEM_CurveIndex] - SizeE5060_LockOVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;    //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT = (Uint32)SizeE5060_LockOVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT; //计算Yi=Y1-dY

                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,7369,10);
            }
            else   //不堵转采用的曲线
            { 
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeE5060_OVLoadTableNum - 2),0);
                OEM_OVLoad_CurveT  =  ((Uint32)(SizeE5060_OVLoadTable[OEM_CurveIndex] - SizeE5060_OVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;        //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT  =  (Uint32)SizeE5060_OVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT;   //线性插值计算
               
                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,7964,10);
            }
        }
        //假设驱动器为从常温至75度需要热量为1，则在持续恒定的X%电流运行情况下，其产生1热量所需的时间
        //查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
        //即为0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，将其放大10的Q20倍   
        STR_OVLoadProtect.SumHeatOEM10_Q20 += ((Uint32)1<<20)/OEM_OVLoad_CurveT;
    }
}


/*******************************************************************************
  函数名:  SizeCSizeD_Over1kw_OVLoadProtect(void)     Size - C&D且1kw以上驱动器过载保护处理程序   ^_^
  输入:   实际线电流有效值与驱动器电流额定比OEM_CurPercent
  输出:   驱动器每监控周期热量累积积分值STR_OVLoadProtect.SumHeatOEM10_Q20
  子函数:  无
  描述:    假设驱动器为从常温至75度需要热量为1，则当在持续恒定的X%电流运行情况下，其产生1热量所需的时间
      查表得为XXXs，那么在X%电流持续10ms(过载保护监控周期),其所获取的热量为0.01S/XXXs
      若在重载情况下则OEM获取热量0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，并将其放大10的Q20倍 
      若在轻载情况下则OEM散发热量0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，
      然后为计算方便，将其放大10的Q20倍，最后将当次贡献的热量计入驱动器每监控周期热量累积积分值
      STR_OVLoadProtect.SumHeatOEM10_Q20    
********************************************************************************/ 
Static_Inline void SizeCSizeD_Over1kw_OVLoadProtect(void)
{   
    //对查表单得出保护曲线时间根据实测值进行限幅处理，以保证查错表单时不会偏差太多
    //散热曲线
    if(OEM_CurPercent < 1150)  //当实际线电流有效值与驱动器电流额定比小于115%时
    {  //查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
        OEM_CurveIndex = (OEM_CurPercent) >> 6;

        OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeCD10_LightLoadTableNum - 2),0);

        OEM_HeatDissipate_CurveT  = SizeCD10_LightLoadTable[OEM_CurveIndex];

        OEM_HeatDissipate_CurveT =MAX_MIN_LMT(OEM_HeatDissipate_CurveT,1695,60);
        //假设驱动器为从报警温度降至常温需要散发热量为1，则在X恒定的持续恒定的X%电流运行情况下，其
        //散发1热量所需的时间查表得为XXXs，那么在XX%电流持续10ms(监控周期),其所散发的热量为0.01S/XXXS
        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍后即为
        //(10<<20)/(OEM_HeatDissipate_CurveT*10)=(1<<20)/(OEM_HeatDissipate_CurveT)即从报警温度降至常温需要散发热量放大至10<<20 
        OEM_DeltaSumHeat = ((Uint32)1<<20)/OEM_HeatDissipate_CurveT;
        if(STR_OVLoadProtect.SumHeatOEM10_Q20 > OEM_DeltaSumHeat)
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 -= OEM_DeltaSumHeat; 
        }
        else
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 = 0;
        }
    }
    else       //实际线电流有效值与驱动器电流额定比大于102.2%时，驱动器获取热量
    {
//        if(OEM_CurPercent > 2494)
//        {
//            OEM_OVLoad_CurveT = 5;
//        }
//        else
        {
            LockFlag = (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < STR_OVLoadProtect.LockSpdThreshold) ? 1 : 0;    //小于60rpm则认为堵转
            OEM_CurveIndex = (OEM_CurPercent -1150) >> 6;
            if(LockFlag)    //堵转曲线
            {   
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeCD10_LockOVLoadTableNum - 2),0);
                //曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))  其中Y为过载曲线时间 X代表电流百分比 
                //其中X表格查询的最小单位为每份64（代表6.4%的额定转矩），所以X2-X1＝64   X1=1150+OEM_CurveIndex*64
                OEM_OVLoad_CurveT  =  ((Uint32)(SizeCD10_LockOVLoadTable[OEM_CurveIndex] - SizeCD10_LockOVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;    //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT  =  (Uint32)SizeCD10_LockOVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT; //计算Yi=Y1-dY

                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,36000,5);
            }
            else   //不堵转采用的曲线
            { 
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeCD10_OVLoadTableNum - 2),0);
                OEM_OVLoad_CurveT  =  ((Uint32)(SizeCD10_OVLoadTable[OEM_CurveIndex] - SizeCD10_OVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;        //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT  =  (Uint32)SizeCD10_OVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT;   //线性插值计算
              
                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,55556,26);
            }
        }
        //假设驱动器为从常温至75度需要热量为1，则在恒定的X%电流运行情况下，其
        //产生1热量所需的时间查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
        //即为0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，将其放大10的Q20倍   
        STR_OVLoadProtect.SumHeatOEM10_Q20 += ((Uint32)1<<20)/OEM_OVLoad_CurveT;
    }
}


/*******************************************************************************
  函数名:  SizeASizeB_Below1kw_OVLoadProtect(void)     小功率Size - A&B且1kw以下驱动器   ^_^
  输入:   实际线电流有效值与驱动器电流额定比OEM_CurPercent
  输出:   驱动器每监控周期热量累积积分值STR_OVLoadProtect.SumHeatOEM10_Q20
  子函数:  无
  描述:    假设驱动器从常温至75度需要热量为1，则当在恒定的持续恒定的X%电流运行情况下，其产生1热量所需的时间
      查表得为XXXs，那么在X%电流持续10ms(过载保护监控周期),其所获取的热量为0.01S/XXXs
      若在重载情况下则OEM获取热量0.01s/(OEM_OVLoad_CurveT/10)=1/(OEM_OVLoad_CurveT*10)，并将其放大10的Q20倍 
      若在轻载情况下则OEM散发热量0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，
      然后为计算方便，将其放大10的Q20倍，最后将当次贡献的热量计入驱动器每监控周期热量累积积分值
      STR_OVLoadProtect.SumHeatOEM10_Q20    
********************************************************************************/ 
Static_Inline void SizeASizeB_Below1kw_OVLoadProtect(void)
{   
    //对查表单得出保护曲线时间根据实测值进行限幅处理，以保证查错表单时不会偏差太多
    //散热曲线
    if(OEM_CurPercent < 1150)  //当实际线电流有效值与驱动器电流额定比小于115%时
    {  //查表索引OEM_CurveIndex为OEM_CurPercent/64  如6%电流时为表格数组第0位   6.4%电流时为表格数组第1位
        OEM_CurveIndex = (OEM_CurPercent) >> 6;

        OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeAB10_LightLoadTableNum - 2),0);

        OEM_HeatDissipate_CurveT  = SizeAB10_LightLoadTable[OEM_CurveIndex];

        OEM_HeatDissipate_CurveT =MAX_MIN_LMT(OEM_HeatDissipate_CurveT,482,45);
        //假设驱动器为从报警温度降至常温需要散发热量为1，则在X恒定的持续恒定的X%电流运行情况下，其
        //散发1热量所需的时间查表得为XXXs，那么在XX%电流持续10ms(监控周期),其所散发的热量为0.01S/XXXS
        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍后即为
        //(10<<20)/(OEM_HeatDissipate_CurveT*10)=(1<<20)/(OEM_HeatDissipate_CurveT)即从报警温度降至常温需要散发热量放大至10<<20  
        OEM_DeltaSumHeat = ((Uint32)1<<20)/OEM_HeatDissipate_CurveT;
        if(STR_OVLoadProtect.SumHeatOEM10_Q20 > OEM_DeltaSumHeat)
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 -= OEM_DeltaSumHeat; 
        }
        else
        {
            STR_OVLoadProtect.SumHeatOEM10_Q20 = 0;
        }
    }
    else       //实际线电流有效值与驱动器电流额定比大于102.2%时，驱动器获取热量
    {     
//        if(OEM_CurPercent > 2942)
//        {
//            OEM_OVLoad_CurveT = 6;
//        }
//        else
        {
            LockFlag = (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < STR_OVLoadProtect.LockSpdThreshold) ? 1 : 0;  //小于60rpm则认为堵转
            OEM_CurveIndex = (OEM_CurPercent -1150) >> 6;
            if(LockFlag)    //堵转曲线
            {   
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeAB10_LockOVLoadTableNum - 2),0);
                //曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))  其中Y为过载曲线时间 X代表电流百分比 
                //其中X表格查询的最小单位为每份64（代表6.4%的额定转矩），所以X2-X1＝64   X1=1150+OEM_CurveIndex*64
                OEM_OVLoad_CurveT  =  ((Uint32)(SizeAB10_LockOVLoadTable[OEM_CurveIndex] - SizeAB10_LockOVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;    //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT  =  (Uint32)SizeAB10_LockOVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT; //计算Yi=Y1-dY

                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,60000,5);
            }
            else   //不堵转采用的曲线
            { 
                OEM_CurveIndex = MAX_MIN_LMT(OEM_CurveIndex,(SizeAB10_OVLoadTableNum - 2),0);
                OEM_OVLoad_CurveT  =  ((Uint32)(SizeAB10_OVLoadTable[OEM_CurveIndex] - SizeAB10_OVLoadTable[OEM_CurveIndex+1]) *
                                      (OEM_CurPercent - 1150 - OEM_CurveIndex * 64)) >> 6;        //计算(Y1-Y2)*(Xi-X1)/(X2-X1)
                OEM_OVLoad_CurveT  =  (Uint32)SizeAB10_OVLoadTable[OEM_CurveIndex] - OEM_OVLoad_CurveT;   //线性插值计算
               
                OEM_OVLoad_CurveT =MAX_MIN_LMT(OEM_OVLoad_CurveT,60000,28);
            }
        }
        //假设驱动器为从常温至75度需要热量为1，则在恒定的持续恒定的X%电流运行情况下，其产生1热量所需的时间
        //查表得为XXXs，那么在X%电流持续10ms(监控周期),其所获取的热量为0.01S/XXXs
        //即为0.01s/(OEM_HeatDissipate_CurveT/10)=1/(OEM_HeatDissipate_CurveT*10)，将其放大10的Q20倍后即为
        //(10<<20)/(OEM_HeatDissipate_CurveT*10)=(1<<20)/(OEM_HeatDissipate_CurveT)即从报警温度降至常温需要散发热量放大至10<<20 
        STR_OVLoadProtect.SumHeatOEM10_Q20 += ((Uint32)1<<20)/OEM_OVLoad_CurveT;
    }
}


/*******************************************************************************
  函数名:  MTOffLine_1k_Monitor(void)     电机动力线断电故障监控（主循环1ms调度运算）    ^_^
  输入:   
  输出:    电机动力线断电报错
  子函数:  
  描述:     非转矩模式时当反馈的相有效电流小于10%额定电流且给定转矩指令达到限幅点且速度小于额定转速的1/4,
    满足条件1000次则认为电机动力线断线，加入速度判断条件为防止由于反电势作用导致转矩达到限幅但反馈一直上不去导致误报
    加入OffLineJudge_Is判断是因为电路可能会有零漂等原因作用，导致虽然断线但AD采样电流反馈仍不为零
    转矩模式时，当反馈的相有效电流小于10%额定电流，指令大于50%额定电流，且速度小于额定转速的1/4,
    满足条件1000次则认为电机动力线断线,加入指令大于50%额定电流是因为指令小于20%以下时可能会误报。
    因此若转矩限制小于50%，或转矩模式时转矩指令小于50%时，则无法报出电机动力线断线
********************************************************************************/ 
void MTOffLine_1k_Monitor(void)   
{   
    //非伺服运行状态或正反限幅电流小于50%的额定电流时则不进行判断电机动力线断线,以免误报
    if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN) 
      ||((ABS(UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqNegLmt_F)) <= STR_OVLoadProtect.OfflineJudge_Iqlmt)
      ||((ABS(UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqPosLmt_F)) <= STR_OVLoadProtect.OfflineJudge_Iqlmt))
    {
        return;
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod > TOQMOD) //非转矩模式时
    { //当反馈的相有效电流小于10%的额定电流，转矩指令且达到转矩限幅值，且速度小于额定转速的1/4,满足条件3000次则认为电机动力线断线
     //加入速度判断条件为防止由于反电势作用导致转矩达到限幅但反馈一直上不去导致误报，如转速指令6000，由于饱和一直上不去6000时。
     //加入OffLineJudge_Is判断是因为电路可能会有零漂等原因作用，导致虽然断线但AD采样电流反馈仍不为零
        if((STR_FUNC_Gvar.ToqCtrl.DovarReg_Clt == 1)
           &&((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb)) < STR_OVLoadProtect.OffLineJudge_SpdFdb)
           &&(STR_OVLoadProtect.IsFdb_1ms < STR_OVLoadProtect.OffLineJudge_Is))
        {
            STR_OVLoadProtect.Offline_Cnt ++;
        }
        else if(STR_OVLoadProtect.Offline_Cnt > 0 )
        {
           STR_OVLoadProtect.Offline_Cnt --;
        }
    }
    else //转矩模式时,当反馈的相有效电流小于10%额定电流，指令大于50%额定电流，且速度小于额定转速的1/4,
    {    //满足条件1000次则认为电机动力线断线,加入速度判断条件为防止由于反电势作用导致转矩达到限幅但反馈一直上不去导致误报
         //加入OffLineJudge_Is判断是因为电路可能会有零漂等原因作用，导致虽然断线但AD采样电流反馈仍不为零
        if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < STR_OVLoadProtect.OffLineJudge_SpdFdb)
           &&(STR_OVLoadProtect.IsFdb_1ms < STR_OVLoadProtect.OffLineJudge_Is)
           &&((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef)) > (1000 >> 1)))
        {
            STR_OVLoadProtect.Offline_Cnt ++;
        }
        else if(STR_OVLoadProtect.Offline_Cnt > 0 )
        {
           STR_OVLoadProtect.Offline_Cnt --;
        }
    }


    //持续1s则判断为断电故障
    if(STR_OVLoadProtect.Offline_Cnt >= 1000)
    {
        PostErrMsg(MTOFFLINE);    //电机输出三相断线警告
        STR_OVLoadProtect.Offline_Cnt = 1000;
    }
    else if(STR_OVLoadProtect.Offline_Cnt <= 990)
    {
        WarnAutoClr(MTOFFLINE);      //清除输出欠压警告
    }
}

/********************************* END OF FILE *********************************/
