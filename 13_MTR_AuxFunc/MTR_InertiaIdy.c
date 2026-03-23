/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: MTR_InertiaIdy.c                                                                
 创建人：朱祥华            创建日期：2011.10.31 
 修改人：朱祥华 杜金明     修改日期：2012.03.13  
 描述： 
    1.惯量辨识模块    
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */    
#include "MTR_GlobalVariable.h"    
#include "MTR_Global_Filter.h"
#include "MTR_InterfaceProcess.h"   
#include "MTR_InertiaIdy.h"
#include "FUNC_MTRInterface.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//-------------
#define Inertia_CircleParaQ20 1748L  //1/600 Q20 //zxh110505

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无   


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
STR_OnLnInertia        OnLnInertia;

STR_MTR_NEW_LOWPASS_FILTER  InertiaIdy_LowpassFilter={0};

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
Uint64 InertiaIdy_SpdCoef_Q32 = 0;         //速度伪调节器增益参数转换系数

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void Init_OnLineInertiaIdy(void);               //在线惯量辨识上电初始化
void OnLnInertia_Sample(void);                  //在线惯量辨识信息采集程序
void OnLnInertia_MainLoopSchedule(void);         //在线惯量辨识主循环调度处理程序

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void OnLnInertia_Logic1(void);                  //在线惯量辨识逻辑处理1
Static_Inline void OnLnInertia_Logic2(void);                   //在线惯量辨识逻辑处理2
Static_Inline void OnLnInertia_Compute(void);                  //在线惯量辨识计算程序


/*******************************************************************************
  函数名:  Init_OnLineInertiaIdy()   ^_^  在线惯量辨识上电初始化
  输入:   
  输出:   
  子函数:        
  描述: 在线惯量辨识主循环调度程序
********************************************************************************/
void Init_OnLineInertiaIdy(void)   //请务必置于功能码更新和电流环参数计算更新之后!!
{
/*----------------------------------------------------------------------------------------------
  Acceleration formula: Te1 = J_total *(2PI/60) * (Spdmax-0)/DeltaT1;
  Deceleration formula: Te2 = J_total *(2PI/60) * (0-Spdmax)/DeltaT2;
  由上述两式可得：(Te1-Te2) = J_total *(2PI/60) * Delta_Spd * (1/DeltaT1 + 1/DeltaT2)    
  式中Delta_Spd为(最大转速Spdmax-0)即为H0900; DeltaT1 = DeltaCnt1 /SpdFreq;   DeltaT2 = DeltaCnt2 /SpdFreq  因此有如下式
  Kt=Tn/In  
  Te=Kt*Is("Is" is phase RMS)=Kt * Iq_analog/sqrt(2)
  Iq_analog = Iq_data * sqrt(2)*In/4096
  Te=Kt * Iq_data * sqrt(2)*In/IqRate_MT
  J_total =(Te1-Te2)*(60/2PI) * DeltaCnt1*DeltaCnt2/[(DeltaCnt1+DeltaCnt2) * Spdmax * SpdFreq] 
          =Kt*In*(Iq_data1-Iq_data2)/IqRate_MT * (60/2PI) * DeltaCnt1*DeltaCnt2/[(DeltaCnt1 + DeltaCnt2) * Spdmax * SpdFreq]
  J_ratio =J_total/J_motor
          =[Kt*In*(60/2PI)/(IqRate_MT*J_motor*SpdFreq)] * {(Iq_data1-Iq_data2)* DeltaCnt1*DeltaCnt2/[(DeltaCnt1 + DeltaCnt2)*Spdmax]}
  J_CalCoef= Kt*In*(60/2PI)/(IqRate_MT*J_motor*SpdFreq)
  计算结果最后乘修正系数Inertia_ModifyParaQ20
------------------------------------------------------------------------------------------------
  转矩系数     Kt      -> FunCodeUnion.code.MT_ToqCoe         单位0.01N/A    量纲系数乘10^(-2)
  电机额定电流 In      -> FunCodeUnion.code.MT_RateCurrent    单位0.01A      量纲系数乘10^(-2)
  惯量比       J_ratio -> FunCodeUnion.code.GN_InertiaRatio   单位0.01       量纲系数乘10^2
  电机惯量     J_motor -> FunCodeUnion.code.MT_Inertia        单位0.01㎏c㎡  量纲系数除10^(-6)
------------------------------------------------------------------------------------------------
  The order of magnitude is 10^4
------------------------------------------------------------------------------------------------*/
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)
	{
		OnLnInertia.CalCoef_Q30 = ((Uint64)FunCodeUnion.code.MT_RateToq * 100L)<<30;
    	OnLnInertia.CalCoef_Q30 = OnLnInertia.CalCoef_Q30 /STR_MTR_Gvar.GetIqRef.IqRate_MT;
    	OnLnInertia.CalCoef_Q30 = (OnLnInertia.CalCoef_Q30 * 100)/FunCodeUnion.code.MT_Inertia;
    	OnLnInertia.CalCoef_Q30 = (OnLnInertia.CalCoef_Q30 * 100)/STR_MTR_Gvar.System.SpdFreq;	
	}
	else
	{
		OnLnInertia.CalCoef_Q30 = ((Uint64)FunCodeUnion.code.MT_RateToq * 100L)<<32;
    	OnLnInertia.CalCoef_Q30 = OnLnInertia.CalCoef_Q30 / (2 * PI_Q12) ;
    	OnLnInertia.CalCoef_Q30 = ((OnLnInertia.CalCoef_Q30 * 60) <<10)/STR_MTR_Gvar.GetIqRef.IqRate_MT;
    	OnLnInertia.CalCoef_Q30 = OnLnInertia.CalCoef_Q30 * 100/FunCodeUnion.code.MT_Inertia;
    	OnLnInertia.CalCoef_Q30 = OnLnInertia.CalCoef_Q30 * 100/STR_MTR_Gvar.System.SpdFreq;
	}

    //惯量滤波器初始化，可更换位置
    InertiaIdy_LowpassFilter.Fc = 50;          //低通滤波截止频率1Hz 
    InertiaIdy_LowpassFilter.Fs = 1000;       //采样频率 1000Hz
    MTR_InitNewLowPassFilt(&InertiaIdy_LowpassFilter);    //更新转矩指令滤波参数后谨记执行此程序
}

/*******************************************************************************
  函数名:  OnLnInertia_MainLoopSchedule()   ^_^  在线惯量辨识主循环调度程序
  输入:   
  输出:   
  子函数:        
  描述: 在线惯量辨识主循环调度程序
********************************************************************************/
void OnLnInertia_MainLoopSchedule(void)
{
    static Uint32 SaveInertia_Cnt = 0;
	static int16  SaveInertia_Comp = 0;
    static Uint16 Init_InertiaRatio = 50000;

    AuxFunCodeUnion.code.OnLineInertiaRatio = FunCodeUnion.code.GN_InertiaRatio;

    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OffLnInertiaModeEn == 1)   //离线惯量辨识使能
    {
        STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn = 1;
        SaveInertia_Cnt = 0;
        Init_InertiaRatio = 50000;
    }
    else if(FunCodeUnion.code.InertiaIdyCountModeSel == 0)     //H0903设定惯量辨识模式 
    {
        STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn = 0;
        SaveInertia_Cnt = 0;
        Init_InertiaRatio = 50000;

        //惯量值加低通滤波减小冲击
        InertiaIdy_LowpassFilter.Input = FunCodeUnion.code.GN_InertiaRatio;
        MTR_NewLowPassFiltCalc(&InertiaIdy_LowpassFilter);

    }
    else if(FunCodeUnion.code.InertiaIdyCountModeSel >= 1)
    {
        STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn = 1;         
        
        //第一次进入在线辨识时读取H0815，Init_InertiaRatio初始值50000，而惯量比值范围为0~12000
        //若Init_InertiaRatio == 50000符合则为第一次进入此函数存入
        if(Init_InertiaRatio == 50000)
        {
            Init_InertiaRatio = FunCodeUnion.code.GN_InertiaRatio;

            //保证首次进入将显示辨识初值为H0815
            STR_MTR_Gvar.InertiaIdy.RatioNow = FunCodeUnion.code.GN_InertiaRatio;
        }

        //惯量值加低通滤波减小冲击
        InertiaIdy_LowpassFilter.Input = (STR_MTR_Gvar.InertiaIdy.RatioNow);
        MTR_NewLowPassFiltCalc(&InertiaIdy_LowpassFilter);
        //写入E2PROM计数用
        SaveInertia_Cnt ++;

      //此处处理辨识结果UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.InertiaIdy_RatioNow
      //快速更新辨识结果至H0815，且每隔半小时存入e2prom
        if((SaveInertia_Cnt >= 1900000)&&(Init_InertiaRatio != FunCodeUnion.code.GN_InertiaRatio))
        {
            SaveInertia_Cnt = 100000;

            Init_InertiaRatio = FunCodeUnion.code.GN_InertiaRatio;

            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_InertiaRatio));
        }

        //开启惯量辨识后，在100秒的时候，先写一次eeprom
        if(SaveInertia_Cnt ==99999)
        {
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_InertiaRatio));
        }
    }
    //若不使能在线辨识,保证首次进入将显示辨识初值为H0815
    if(0 == STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn)
    {
        STR_MTR_Gvar.InertiaIdy.RatioNow = FunCodeUnion.code.GN_InertiaRatio;
    }

    AuxFunCodeUnion.code.RatioNow = STR_MTR_Gvar.InertiaIdy.RatioNow;

    //若不使能在线辨识则返回
    if(0 == STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn)
    {
        return;
    }

    //将主循环的逻辑判断和计算分为3个步骤,分别在主循环3ms内平均执行。
    if(OnLnInertia.Flag.bit.Status == 1)
    {  //进入状态逻辑判断1
        OnLnInertia_Logic1();
        OnLnInertia.Flag.bit.Status = 2;
    }
    else if (OnLnInertia.Flag.bit.Status == 2)
    {  //进入状态逻辑判断2
        OnLnInertia_Logic2();
        OnLnInertia.Flag.bit.Status = 3;
    }
    else if ((OnLnInertia.Flag.bit.Status == 3)&&(OnLnInertia.Flag.bit.Enable_Calculation==1))
    {  //进行惯量比值计算
        OnLnInertia_Compute();
        OnLnInertia.Flag.bit.Status = 0;
    }
    else  OnLnInertia.Flag.bit.Status = 0;


    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OffLnInertiaModeEn == 1)   //离线惯量辨识使能
    {
        AuxFunCodeUnion.code.OnLineInertiaRatio = STR_MTR_Gvar.InertiaIdy.RatioNow;   //H2F49
    }
    else   //在线惯量辨识
    {
        SaveInertia_Comp = FunCodeUnion.code.GN_InertiaRatio - STR_MTR_Gvar.InertiaIdy.RatioNow;
        if((SaveInertia_Comp >= 10 )||(SaveInertia_Comp <= -10 ))
        {
            FunCodeUnion.code.GN_InertiaRatio = InertiaIdy_LowpassFilter.Output;
        }
        else
        {
            FunCodeUnion.code.GN_InertiaRatio = STR_MTR_Gvar.InertiaIdy.RatioNow;
        }
    }
}

/********************************************************************************************************************
  函数名:  OnLnInertia_Sample()   ^_^  在线惯量辨识信息采集函数
  输入:   

  输出:   
  子函数:无         
  描述: 在线惯量辨识信息采集函数
        该函数在速度环中调度执行    
********************************************************************************/
void OnLnInertia_Sample(void)
{
   static int32 OnLnInertiaSpd = 0;
   //**************************单元信息记录***************************  
    
    OnLnInertiaSpd = STR_MTR_Gvar.FPGA.SpdFdb/100;
    if(OnLnInertia.SpdPeriodCnt <= 99)  
    { 
        OnLnInertia.firstHalfSpdFdbAdd += OnLnInertiaSpd;      //单元前半程速度累积
        OnLnInertia.firstHalfSpdRefAdd += UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef;    //单元前半程速度指令累积
        OnLnInertia.firstHalfPosRefAdd += UNI_MTR_FUNCToMTR_List_16kHz.List.PosRef;    //单元前半程位置指令累积
    }
    else if(OnLnInertia.SpdPeriodCnt >= 100) 
    {
        OnLnInertia.secondHalfSpdFdbAdd += OnLnInertiaSpd;        //单元后半程速度累积
        OnLnInertia.secondHalfSpdRefAdd += UNI_MTR_FUNCToMTR_List_16kHz.List.SpdRef;    //单元后半程速度累积
        OnLnInertia.secondHalfPosRefAdd += UNI_MTR_FUNCToMTR_List_16kHz.List.PosRef;    //单元后半程位置指令累积
    }
    
    OnLnInertia.DeltaIqFdbadd += STR_MTR_Gvar.FPGA.IqFdb;             //信息单元电流采集 
    
    OnLnInertia.SpdPeriodCnt++;
    //OnLnInertia.SpdPeriodCnt %= 200;          //200/16000=12.5ms      //信息单元划分计数器     暂定每个单元时间为12.5ms
    if(OnLnInertia.SpdPeriodCnt >= 200)
    {
        OnLnInertia.SpdPeriodCnt = 0;
    }
    
    if(OnLnInertia.SpdPeriodCnt >= 185)   //提高低速辨识精度的补偿算法
    {
        OnLnInertia.SpdComp += OnLnInertiaSpd;                                                 //补偿速度采集
        OnLnInertia.IqComp += ((OnLnInertia.SpdPeriodCnt-184)* STR_MTR_Gvar.FPGA.IqFdb);                 //补偿电流采集
    }
    else if(OnLnInertia.SpdPeriodCnt == 0)
    {
        OnLnInertia.Flag.bit.Status = 1;
        OnLnInertia.SpdFdbLatch = OnLnInertiaSpd;
        
        //锁存下面变量
        OnLnInertia.DeltaIqFdbaddLatch = OnLnInertia.DeltaIqFdbadd;               //单元电流和清零
        OnLnInertia.firstHalfSpdFdbAddLatch = OnLnInertia.firstHalfSpdFdbAdd ;           //前半程速度和清零
        OnLnInertia.secondHalfSpdFdbAddLatch = OnLnInertia.secondHalfSpdFdbAdd;           //后半程速度和清零
        OnLnInertia.secondHalfSpdRefAddLatch = OnLnInertia.secondHalfSpdRefAdd;
        OnLnInertia.firstHalfSpdRefAddLatch = OnLnInertia.firstHalfSpdRefAdd;
        OnLnInertia.secondHalfPosRefAddLatch = OnLnInertia.secondHalfPosRefAdd;
        OnLnInertia.firstHalfPosRefAddLatch = OnLnInertia.firstHalfPosRefAdd;
        OnLnInertia.IqCompLatch = OnLnInertia.IqComp;
        OnLnInertia.SpdCompLatch = OnLnInertia.SpdComp;
        //各个中间量清零
        OnLnInertia.DeltaIqFdbadd = 0;               //单元电流和清零
        OnLnInertia.firstHalfSpdFdbAdd = 0 ;           //前半程速度和清零
        OnLnInertia.secondHalfSpdFdbAdd = 0 ;           //后半程速度和清零
        OnLnInertia.secondHalfSpdRefAdd = 0;
        OnLnInertia.firstHalfSpdRefAdd = 0;
        OnLnInertia.secondHalfPosRefAdd = 0;
        OnLnInertia.firstHalfPosRefAdd = 0;
        OnLnInertia.IqComp = 0;
        OnLnInertia.SpdComp = 0;
    }
}

/*******************************************************************************
  函数名:  OnLnInertia_Sample()   ^_^  在线惯量辨识惯量辨识逻辑处理1
  输入:  
  输出:  

  子函数:无         
  描述: 惯量辨识逻辑处理1
        该函数于在线惯量辨识主循环调度程序执行
********************************************************************************/
Static_Inline void OnLnInertia_Logic1(void)
{
    int32 OnLnInertiaTempSpd = 0;
    int32 OnLnInertiaTempPos = 0;
    
    OnLnInertiaTempSpd = OnLnInertia.secondHalfSpdRefAddLatch - OnLnInertia.firstHalfSpdRefAddLatch;
    OnLnInertiaTempPos = OnLnInertia.secondHalfPosRefAddLatch - OnLnInertia.firstHalfPosRefAddLatch;

    if( ((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.RunMod==SPDMOD)&&(ABS(OnLnInertiaTempSpd)<=100))
    ||((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.RunMod==POSMOD)&&(ABS(OnLnInertiaTempPos)<=10)) )
    {
           OnLnInertia.ForbidCnt++;
        if(OnLnInertia.ForbidCnt>1000)  OnLnInertia.ForbidCnt=100;
    }
    else
    {
      OnLnInertia.ForbidCnt=0;        
    }

    OnLnInertia.BeforeState=OnLnInertia.CurrentState;   //首先记录前一次状态
    OnLnInertia.OriginSpdFdb=OnLnInertia.TerminationSpdFdb;                //速度变化量记录
    OnLnInertia.TerminationSpdFdb=(OnLnInertia.SpdFdbLatch + OnLnInertia.SpdCompLatch);
    OnLnInertia.Idntfy_DltaSpd=OnLnInertia.TerminationSpdFdb - OnLnInertia.OriginSpdFdb;//单元速度差计算
    //**************************************低速补偿处理************************************
     OnLnInertia.OriginIqComp=OnLnInertia.TerminationIqComp;
     OnLnInertia.TerminationIqComp=OnLnInertia.IqCompLatch;

     OnLnInertia.DeltaIqFdbaddLatch=(OnLnInertia.DeltaIqFdbaddLatch<<4)
                        + OnLnInertia.OriginIqComp-OnLnInertia.TerminationIqComp;
     //***************************************************************************************
    //运动方向为正转
    if((OnLnInertia.OriginSpdFdb>32000)&&(OnLnInertia.TerminationSpdFdb>32000)
           &&(OnLnInertia.ForbidCnt<=3))
    {
        OnLnInertia.Flag.bit.Forward=1 ;         //正转标志置1  为正转

        if((OnLnInertia.secondHalfSpdFdbAddLatch-OnLnInertia.firstHalfSpdFdbAddLatch)>50000L)
        {
              OnLnInertia.Forward_SpeedUpCnt1+=1;
        }
          //判断为加速
        if((OnLnInertia.secondHalfSpdFdbAddLatch-OnLnInertia.firstHalfSpdFdbAddLatch)>50000L)
        {
            OnLnInertia.Forward_SpeedUpCnt+=1;        //正向加速单元次数记录          
            OnLnInertia.Forward_SpeedUpAdd+=OnLnInertia.Idntfy_DltaSpd;
            OnLnInertia.Forward_SpdUp_IqFdbadd+=OnLnInertia.DeltaIqFdbaddLatch;   //正向加速电流累积
            OnLnInertia.CurrentState=1;
          }
          //判断为减速
        if((OnLnInertia.secondHalfSpdFdbAddLatch-OnLnInertia.firstHalfSpdFdbAddLatch)<-50000)
        {
            OnLnInertia.Forward_SpeedDownCnt+=1;  //正向减速单元次数记录
            OnLnInertia.Forward_SpeedDownAdd+=OnLnInertia.Idntfy_DltaSpd;                
            OnLnInertia.Forward_SpdDown_IqFdbadd+=OnLnInertia.DeltaIqFdbaddLatch;  //正向减速电流累积
            OnLnInertia.CurrentState=2;
        }
    }
    //运动方向为反转
    if((OnLnInertia.OriginSpdFdb<-32000)&&(OnLnInertia.TerminationSpdFdb<-32000)
       &&(OnLnInertia.ForbidCnt<=3))
       {
        OnLnInertia.Flag.bit.Forward=0;          //正转标志置0   为反转
          //判断为加速
        if((OnLnInertia.secondHalfSpdFdbAddLatch-OnLnInertia.firstHalfSpdFdbAddLatch)<-50000)
        {
            OnLnInertia.Reverse_SpeedUpCnt+=1;        //反向加速单元次数记录
            OnLnInertia.Reverse_SpeedUpAdd+=OnLnInertia.Idntfy_DltaSpd;
            OnLnInertia.Reverse_SpdUp_IqFdbadd+=OnLnInertia.DeltaIqFdbaddLatch;   //反向加速电流累积
            OnLnInertia.CurrentState=4;
        }
          //判断为减速
        if((OnLnInertia.secondHalfSpdFdbAddLatch-OnLnInertia.firstHalfSpdFdbAddLatch)>50000)
          {
            OnLnInertia.Reverse_SpeedDownCnt+=1;        //反向加速单元次数记录
            OnLnInertia.Reverse_SpeedDownAdd+=OnLnInertia.Idntfy_DltaSpd;
            OnLnInertia.Reverse_SpdDown_IqFdbadd+=OnLnInertia.DeltaIqFdbaddLatch;   //反向加速电流累积
            OnLnInertia.CurrentState=5;
        }                
    }
    //各个中间量清零
    OnLnInertia.DeltaIqFdbaddLatch=0;               //单元电流和清零
    OnLnInertia.firstHalfSpdFdbAddLatch=0 ;           //前半程速度和清零
    OnLnInertia.secondHalfSpdFdbAddLatch=0 ;           //后半程速度和清零
    OnLnInertia.secondHalfSpdRefAddLatch=0;
    OnLnInertia.firstHalfSpdRefAddLatch=0;
    OnLnInertia.secondHalfPosRefAddLatch=0;
    OnLnInertia.firstHalfPosRefAddLatch=0;
    OnLnInertia.IqCompLatch = 0;
    OnLnInertia.SpdCompLatch = 0;
    OnLnInertia.SpdFdbLatch = 0;
}

/*******************************************************************************
  函数名:  OnLnInertia_Logic2()   ^_^  惯量辨识逻辑处理2
  输入:  

  输出:   

  子函数:无         
  描述: 惯量辨识逻辑处理2
        该函数于在线惯量辨识主循环调度程序执行
********************************************************************************/
Static_Inline void OnLnInertia_Logic2(void)
{
    //可计算条件扫描
    if((OnLnInertia.Forward_SpeedUpCnt>=10)&&(OnLnInertia.Forward_SpeedUpAdd>=320000))
    {
        OnLnInertia.Flag.bit.Forward_SpeedUp_Mature=1;
    }
    else  OnLnInertia.Flag.bit.Forward_SpeedUp_Mature=0;
    
    //正向减速
    if((OnLnInertia.Forward_SpeedDownCnt>=10)&&(OnLnInertia.Forward_SpeedDownAdd<=-320000))  
    {
      OnLnInertia.Flag.bit.Forward_SpeedDown_Mature=1;
    }
    else   OnLnInertia.Flag.bit.Forward_SpeedDown_Mature=0;
     
     //反向加速
    if((OnLnInertia.Reverse_SpeedUpCnt>=10)&&(OnLnInertia.Reverse_SpeedUpAdd<=-320000))      
    {
      OnLnInertia.Flag.bit.Reverse_SpeedUp_Mature=1;
    }
    else   OnLnInertia.Flag.bit.Reverse_SpeedUp_Mature=0;
    
      //反向减速
    if((OnLnInertia.Reverse_SpeedDownCnt>=10)&&(OnLnInertia.Reverse_SpeedDownAdd>=320000))  
    {
      OnLnInertia.Flag.bit.Reverse_SpeedDown_Mature=1;
    }
    else   OnLnInertia.Flag.bit.Reverse_SpeedDown_Mature=0;

      //信息提取并清零
    if((OnLnInertia.Flag.bit.Forward_SpeedUp_Mature==1)
       &&(OnLnInertia.Flag.bit.Forward_SpeedDown_Mature==1)
       &&( OnLnInertia.BeforeState!=OnLnInertia.CurrentState))
    {
           //提取正向的加速电流和、减速电流和、加速速度变化量、减速速度变化量、加速单元次数、减速单元次数
        OnLnInertia.Compute_IqFdb1 = OnLnInertia.Forward_SpdUp_IqFdbadd;
        OnLnInertia.Compute_IqFdb2 = OnLnInertia.Forward_SpdDown_IqFdbadd;
        OnLnInertia.Compute_DeltaSpd1 = OnLnInertia.Forward_SpeedUpAdd;
        OnLnInertia.Compute_DeltaSpd2 = OnLnInertia.Forward_SpeedDownAdd;
        OnLnInertia.Compute_CellCnt1 =  OnLnInertia.Forward_SpeedUpCnt; 
        OnLnInertia.Compute_CellCnt2 =  OnLnInertia.Forward_SpeedDownCnt;
        OnLnInertia.Flag.bit.Enable_Calculation=1;      //可计算标志置1      
        //提取后，将各变量清零
        OnLnInertia.Forward_SpdUp_IqFdbadd=0;
        OnLnInertia.Forward_SpdDown_IqFdbadd=0;
        OnLnInertia.Forward_SpeedUpAdd=0;
        OnLnInertia.Forward_SpeedDownAdd=0;
        OnLnInertia.Forward_SpeedUpCnt=0; 
        OnLnInertia.Forward_SpeedUpCnt1=0;
        OnLnInertia.Forward_SpeedDownCnt=0; 
    }          
    
    if((OnLnInertia.Flag.bit.Reverse_SpeedUp_Mature==1)
    &&(OnLnInertia.Flag.bit.Reverse_SpeedDown_Mature==1)
    &&( OnLnInertia.BeforeState!=OnLnInertia.CurrentState))
    {
       //提取反向的加速电流和、减速电流和、加速速度变化量、减速速度变化量、加速单元次数、减速单元次数
    //    if(OnLnInertia.ForbidCnt)
        OnLnInertia.Compute_IqFdb1 = OnLnInertia.Reverse_SpdUp_IqFdbadd;
        OnLnInertia.Compute_IqFdb2 = OnLnInertia.Reverse_SpdDown_IqFdbadd;
        OnLnInertia.Compute_DeltaSpd1 = OnLnInertia.Reverse_SpeedUpAdd;
        OnLnInertia.Compute_DeltaSpd2 = OnLnInertia.Reverse_SpeedDownAdd;
        OnLnInertia.Compute_CellCnt1 =  OnLnInertia.Reverse_SpeedUpCnt; 
        OnLnInertia.Compute_CellCnt2 =  OnLnInertia.Reverse_SpeedDownCnt;
        OnLnInertia.Flag.bit.Enable_Calculation=1;    //可计算标志置1

        //提取后，将各变量清零
        OnLnInertia.Reverse_SpdUp_IqFdbadd=0;
        OnLnInertia.Reverse_SpdDown_IqFdbadd=0;
        OnLnInertia.Reverse_SpeedUpAdd=0;
        OnLnInertia.Reverse_SpeedDownAdd=0;
        OnLnInertia.Reverse_SpeedUpCnt=0; 
        OnLnInertia.Reverse_SpeedDownCnt=0;    
    }
}
/*******************************************************************************
  函数名:  OnLnInertia_Sample()   ^_^  在线惯量辨识计算
  输入:   

  输出:   1. 惯量比H0815
          
  子函数:无         
  描述: 惯量比计算
        该函数在速度环处理程序GetSpdRef_InertiaIdy_SpdProcess()0 中调度执行   
        
                     加速平均电流  -  减速平均电流
        惯量比= ——————————————————————————————————————  * 惯量比计算系数    
                加速平均速度变化量 - 减速平均速度变化量
********************************************************************************/
Static_Inline void OnLnInertia_Compute(void)
{
    int64 InertiaOutTemp = 0;
    int32 InertiaOutTemp1 = 0;
    int16   InertiaOutAverageCnt=4; 	//预先取平均值次数
    int16   InertiaOutFilterCnt=2;    //滤波系数

    
    OnLnInertia.Compute_AverageIqFdb1=(int32)(OnLnInertia.Compute_IqFdb1/OnLnInertia.Compute_CellCnt1);          //加速平均单元电流
    OnLnInertia.Compute_AverageIqFdb2=(int32)(OnLnInertia.Compute_IqFdb2/OnLnInertia.Compute_CellCnt2);            //减速平均单元电流

    OnLnInertia.Compute_AverageDeltaSpd1=OnLnInertia.Compute_DeltaSpd1/OnLnInertia.Compute_CellCnt1;              //加速平均单元速度变化
    OnLnInertia.Compute_AverageDeltaSpd2=OnLnInertia.Compute_DeltaSpd2/OnLnInertia.Compute_CellCnt2;              //减速平均单元速度变化

    InertiaOutTemp = (OnLnInertia.Compute_AverageIqFdb1- OnLnInertia.Compute_AverageIqFdb2)*OnLnInertia.CalCoef_Q30;      
    InertiaOutTemp = (InertiaOutTemp*100L)/( OnLnInertia.Compute_AverageDeltaSpd1-OnLnInertia.Compute_AverageDeltaSpd2);

    InertiaOutTemp = InertiaOutTemp>>30;
    InertiaOutTemp1 =(int32)((ABS(InertiaOutTemp))<<7);    

    //加权处理
    OnLnInertia.InertiaRatioNum++;


	//根据H0903设置，设定均值滤波次数
    if(FunCodeUnion.code.InertiaIdyCountModeSel == 0)
    {   

		 InertiaOutAverageCnt=6; 	    //预先取平均值次数
         InertiaOutFilterCnt=3;          //滤波系数
    }

	else if(FunCodeUnion.code.InertiaIdyCountModeSel == 1)	 //几乎无变化
    {   

		 InertiaOutAverageCnt=16; 	    //预先取平均值次数
         InertiaOutFilterCnt=7;          //滤波系数
    }

 	else if(FunCodeUnion.code.InertiaIdyCountModeSel == 2)	 //缓慢变化
    {   

		InertiaOutAverageCnt=10; 	    //预先取平均值次数
        InertiaOutFilterCnt=5;          //滤波系数
    }

 	else if(FunCodeUnion.code.InertiaIdyCountModeSel == 3)	 //急剧变化
    {   

		 InertiaOutAverageCnt=6; 	    //预先取平均值次数
         InertiaOutFilterCnt=3;          //滤波系数
    }

	//进行平均值滤波
    if(OnLnInertia.InertiaRatioNum>=1000)   OnLnInertia.InertiaRatioNum=100;

    if(OnLnInertia.InertiaRatioNum<=InertiaOutAverageCnt)
    {
       OnLnInertia.InertiaOut4 += InertiaOutTemp1;    
       OnLnInertia.InertiaOut5 = OnLnInertia.InertiaOut4/OnLnInertia.InertiaRatioNum;
    }
    else
    {        
       OnLnInertia.InertiaOut5 = ((OnLnInertia.InertiaOut5<<InertiaOutFilterCnt)- 
                OnLnInertia.InertiaOut5+InertiaOutTemp1)>>InertiaOutFilterCnt;                
    }
    OnLnInertia.InertiaOut6 = (OnLnInertia.InertiaOut5>>7) - 100L;
    OnLnInertia.Flag.bit.Enable_Calculation=0;    //可计算标志清零    
	if(OnLnInertia.InertiaRatioNum>1)
	{
        STR_MTR_Gvar.InertiaIdy.RatioNow = MAX_MIN_LMT(OnLnInertia.InertiaOut6,12000L,0);
	}
}

/********************************* END OF FILE *********************************/
