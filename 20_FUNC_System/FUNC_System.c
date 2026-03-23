/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_System.c
 创建人：李浩                创建日期：11.09.26 
 描述： 
     1.FUNC功能模块内的全局变量
     2.

 修改记录：  
     1.xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "stm32f4xx.h"
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_System.h"
#include "FUNC_InterfaceProcess.h" 
#include "FUNC_GPIODriver.h" 
#include "FUNC_SpiDriver_ST.h"
#include "FUNC_WWDG.h" 
#include "CANopen_Pub.h"
#include "CANopen_Home.h"
//通信模块
#include "COMM_System.h"

//监控模块
#include "FUNC_AI.h"
#include "FUNC_DiDo.h"
#include "FUNC_AoDriver_ST.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ServoMonitor.h"


//速度转矩控制模块
#include "FUNC_ToqCtrl.h"
#include "FUNC_SpdCtrl.h"
#include "FUNC_GainSwitching.h"
#include "FUNC_TorqPiTune.h "
#include "CANopen_PV.h"
#include "CANopen_Pub.h"
#include "CANopen_InterFace.h"
#include "CANopen_OD.h"
#include "ECT_CSP.h"
#include "CANopen_DeviceCtrl.h"
#include "ECT_InterFace.h"
#include "ECT_ESMDisplay.h"
#include "ECT_Probe.h"
#include "ECT_PP.h"

//运动控制模块
#include "FUNC_MotionCtrl.h"   //只声明位置环的输出函数，起到隔离位置环的功能

//功能码管理模块
#include "FUNC_FunCode.h"
#include "FUNC_ManageFunCode.h" 

//自调节及后台模式模块
#include "FUNC_FSASpeedCmd.h"
#include "FUNC_Fft.h"

#include "FUNC_FricIdentify.h"  
#include "FUNC_RigidityLevelTable.h"

//原来MTR模块监控部分头文件
#include "FUNC_MotorOverLoad.h"
#include "FUNC_Monitor_OverLoadProtect.h"
#include "FUNC_Monitor_Temperature.h"
#include "FUNC_Monitor_PhaseLack.h"
#include "FUNC_AdcDriver_ST.h"
#include "FUNC_Monitor_DCBusVolt.h"

#include "ECT_InterFace.h"
#include "FUNC_CSPCmdBuffer.h"
#include "FUNC_ModeSelect.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define     DEBUG_WD_ENABLE         1       

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_FUNC_GLOBALVARIABLE    STR_FUNC_Gvar;   //功能模块全局变量结构体
UNI_HardFaultInformation   HFInfor;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
volatile Uint32 FPGA_ExtInterruptCnt = 0;
STR_FUNC_WWDG_Def * FUNC_WWDG = (STR_FUNC_WWDG_Def *)(0x40002C00);

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

//初始化相关调度程序
void FUNC_PeripheralConfig_RST(void);
void FUNC_Parameter_Frist_RST(void);
void FUNC_Parameter_Second_RST(void);
void FUNC_Interrupt_RST(void);
void FUNC_UpdateSysFreqAndPrd(void);
void FUNC_InitAndEnableWatchDog(void);

//中断相关调度程序
void FUNC_CmdProcess_ToqInterrupt(void);
void FUNC_AdcStart_ToqInterrupt(void);
void FUNC_AuxFunc_ToqInterrupt(void);
void FUNC_PostionControl_PosInterrupt(void);
void FUNC_System_AuxInterrupt(void);   //功能模块的辅助中断处理主要处理伺服Reset时通讯显示和Reset完成后通讯处理


//主循环调度程序
extern void FUNC_MainLoop(void);
//extern void CANopenCycleRun(Uint16 addr, Uint16 baud);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void H0B_FunCode_Display(void);
//H30组功能码更新
Static_Inline void H30_FunCode_Update(void);
//任务调度器，置各种任务的标志位
Static_Inline void FUNC_SYS_Schedular_RealTime(void);

Static_Inline void MonitFPGA_ExternalInterrupt(void);

Static_Inline void ReadUniqueDeviceID(void);

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_PeripheralConfig_RST(void)
{
    InitFUNC_GPIO();      //初始化功能模块的GPIO
    InitSpi();            //初始化SPI
    InitI2c();            //初始化Eeprom中所调用的I2C
    InitTim3();           //初始化AO用定时器Tim3和Tim8  wzg20120320
    InitAdc();            //初始化AD采样
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_Parameter_Frist_RST(void)
{
    InitFunCode();        //初始化功能码
#if ECT_ENABLE_SWITCH
    CanopenSupportedDriveModeSet();
#endif    
    FUNC_UpdateSysFreqAndPrd();

    Rigidity_LevelProcess();        //刚性表初始化
    GainSwitchingUpdate();          //增益切换初始化
    AoConst_Update();

	ReadUniqueDeviceID();
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_Parameter_Second_RST(void)           
{    
/* 以下是MTR传递到FUNC模块，供FUNC使用的接口变量——Init初始化上电调用一次 */ 

    G_MTR_FUNCGetInitList( (Uint32 *) &UNI_FUNC_MTRToFUNC_InitList.all[0] );    //接口变量传递  MTR传给FUNC模块，供FUNC模块使用

/* 以上是MTR传递到FUNC模块，供FUNC使用的接口变量——Init初始化上电调用一次 */

    //原来MTR模块监控部分函数
    InitDCBusVoltProcess();   //初始化与母线电压有关配置（务必置于功能码初始化之后）

#if POWERDRIVER_TYPE==POWDRV_IS650
    InitBrkCurrentCheck();
#endif

    InitPL_Process();         //初始化PL信号滤波窗口宽度

    InitTemperatureProcess(); //IPM温度监控程序初始化

    StopUpdateDCBusBrake();   //母线电压泄放初始化

    InitOverLoadProtect();    //驱动器电机过载保护初始化

    AiConstInit();             //AiAo相关初始化

    InitDiDoState();           //DiDo相关初始化       
#if ECT_ENABLE_SWITCH
    TouchProbeInit();//探针状态初始化
    InitTim7();           //位置环定时器
	InitTim7IRQ();
#endif
    InitPosCtrl();             //位置控制上电初始化

    InitPosRegu();             //位置调节器上电初始化

    InitSpdCtrl();             //速度控制上电初始化

    ServoRunManage();          //伺服运行模式初始化
    StopModeInit();            //伺服停机方式与停机故障初始化
    InitMonitorHardWare();     //GD停机初始化以及故障时伺服OFF停机初始化  

    FftInit();//fft参数初始化
	
    //COMM模块初始化
    COMM_SciInit();


    //电子标签初始化
    InitElcLabel();
}



/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_Interrupt_RST(void)
{


}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_InitAndEnableWatchDog(void)
{
    //if(STR_PUB_Gvar.AllInitDone == 0) return;
#if DEBUG_WD_ENABLE        //可以采用窗口看门狗实现软复位
    
    FUNC_WWDG->CFR.bit.WDGTB = 2;        //CK计时器时钟(PCLK1除以4096)除以8    (30MHz/4096)/8 = 915 Hz (~1092 us)

    FUNC_WWDG->CFR.bit.W = 0x7F;        // Set Window value to 127,才保证可随时刷新 

    FUNC_WWDG->CR.bit.T = 0x53;          //WWDG timeout = 1092 us * 20 = 21.8 ms

    FUNC_WWDG->SR.bit.EWIF = 0;          // Clear EWI flag

    //WWDG_EnableIT();     // Enable EW interrupt 

    FUNC_WWDG->CR.bit.WDGA = 1;          //Enable WWDG
#endif
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:    上电时更新功能模块内所需调用的相关调度频率和周期
********************************************************************************/ 
Static_Inline void FUNC_UpdateSysFreqAndPrd(void)
{
    if(FunCodeUnion.code.OEM_ToqLoopFreqSel == 1) //转矩环调节频率选择
    {
        STR_FUNC_Gvar.System.ToqFreq   = FunCodeUnion.code.OEM_CarrWaveFreq << 1;
    }
    else
    {
        STR_FUNC_Gvar.System.ToqFreq   = FunCodeUnion.code.OEM_CarrWaveFreq;
    } 

    STR_FUNC_Gvar.System.CarFreq = FunCodeUnion.code.OEM_CarrWaveFreq;

    STR_FUNC_Gvar.System.SpdFreq   = STR_FUNC_Gvar.System.ToqFreq / FunCodeUnion.code.OEM_SpdLoopFreqScal;   //速度环调节频率
    STR_FUNC_Gvar.System.PosFreq   = STR_FUNC_Gvar.System.ToqFreq / FunCodeUnion.code.OEM_PosLoopFreqScal;   //位置环调节频率

    STR_FUNC_Gvar.System.SpdMsk    = FunCodeUnion.code.OEM_SpdLoopFreqScal;  
    STR_FUNC_Gvar.System.PosMsk    = FunCodeUnion.code.OEM_PosLoopFreqScal; 

	STR_FUNC_Gvar.System.Const4KMsk  = 1;//STR_FUNC_Gvar.System.ToqFreq /4000 -1; //msx,for fft sample
    STR_FUNC_Gvar.System.CarPrd = ((Uint64)1000000000LL) / (Uint32)FunCodeUnion.code.OEM_CarrWaveFreq;
    STR_FUNC_Gvar.System.ToqPrd_Q10 = ((Uint32)1000000 << 10) / STR_FUNC_Gvar.System.ToqFreq;
    STR_FUNC_Gvar.System.SpdPrd_Q10 = STR_FUNC_Gvar.System.ToqPrd_Q10 * STR_FUNC_Gvar.System.SpdMsk ;    //等价于  STR_FUNC_Gvar.SpdLoopPrd_Q10
    STR_FUNC_Gvar.System.PosPrd_Q10 = STR_FUNC_Gvar.System.ToqPrd_Q10 * STR_FUNC_Gvar.System.PosMsk;    //等价于 STR_FUNC_Gvar.PosLoopPrd_Q10
    STR_FUNC_Gvar.System.HalfPosPrd = STR_FUNC_Gvar.System.PosPrd_Q10>>11;


    STR_FUNC_Gvar.System.SpdCnt = 0;
    STR_FUNC_Gvar.System.PosCnt = 1;    //与速度环错开一拍
}

/*******************************************************************************
  函数名:  void FUNC_SYS_Schedular_RealTime(void)
  输入:    
  输出:    
  子函数:         
  描述:   任务调度器，置各种任务的标志位
********************************************************************************/ 
Static_Inline void FUNC_SYS_Schedular_RealTime(void)
{
    static Uint8 FpgaSyncModeStLatch = 0;    
    
	//速度环
    STR_FUNC_Gvar.System.SpdCnt ++;
    if(STR_FUNC_Gvar.System.SpdCnt >= STR_FUNC_Gvar.System.SpdMsk)
    {
        STR_FUNC_Gvar.System.SpdCnt = 0;
        STR_FUNC_Gvar.ScheldularFlag.bit.SpdFlag = 1;                    //置速度环调度标志 bit01 2K
    }
    if((FunCodeUnion.code.CM_FpgaSyncModeSel<2)
        ||(STR_EcatPosSync.PosSyncModeConfigDone ==0))//自同步与全同步版本
    {
        //位置环    与速度环错开一拍         
        if((0 == FpgaSyncModeStLatch) && (1 == GetEcatActive()))
        {
			STR_FUNC_Gvar.System.PosCnt = STR_FUNC_Gvar.System.PosMsk;
		}
		else
		{
			STR_FUNC_Gvar.System.PosCnt ++;
		}
		 
		if(STR_FUNC_Gvar.System.PosCnt >= STR_FUNC_Gvar.System.PosMsk) 
        {
            STR_FUNC_Gvar.System.PosCnt = 0;        
            STR_FUNC_Gvar.ScheldularFlag.bit.PosFlag = 1;                   //置位置环调度标志bit012 1K
            STR_FUNC_Gvar.ScheldularFlag.bit.ZClampInnerPosFlag = 1;            //wzg20111018零速钳位(零位固定)函数内建位置环调度标志位
        }
    }
    else
    {
        STR_FUNC_Gvar.System.PosCnt = 0;
    }

    FpgaSyncModeStLatch = GetEcatActive();
}

/*******************************************************************************
  函数名:  void FUNC_AdcStart_ToqInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:   
********************************************************************************/ 
void FUNC_AdcStart_ToqInterrupt(void)
{
    ADC_START_ADC1();

    FpgaSyncModeConfigAndCheck_ToqInt(); 
    
    PosSyncCheck();
    
}
/*******************************************************************************
  函数名:  void FUNC_CmdProcess_ToqInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:   任务调度器，置各种任务的标志位
********************************************************************************/ 
void FUNC_CmdProcess_ToqInterrupt(void)
{
    //任务调度器，置各种任务的标志位  36/120us
    FUNC_SYS_Schedular_RealTime();

    //获取ADC采样值  256/120us
    Get_ADC_ChannelValueAIUdc_16k(); 

    //MTR 传递给FUNC模块，供功能模块调用的接口变量  188/120us
    G_MTR_FUNCGetList_16kHz( (Uint32 *) &UNI_FUNC_MTRToFUNC_FastList_16kHz.all[0]);
        
    FUNC_Interrupt16kHz_InterfaceDeal();     //获取接口变量
//    //位置调节器运算和速度环频率保持一致
//    if(1 == STR_FUNC_Gvar.ScheldularFlag.bit.SpdFlag)
//    {    
//        PosFbCalc();           //获取16K刷新频率时的位置反馈
//
//        //全闭环位置反馈
//        FullCloseLoopModeSet(&STR_FUNC_Gvar.PosCtrl.PosFdb,&STR_FUNC_Gvar.PosCtrl.ExPosCoef,UNI_FUNC_MTRToFUNC_FastList_16kHz.List.ExPosFdbAbsValue,STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag);
//
//        PosErrCalcAndClr();    //位置偏差计算及清零
//
//        if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod > SPDMOD)
//        {
//            FullCloseLoopErrorCheck(STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag);   //全闭环偏差计算
//        }
//        
//
//        if( (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN) && (STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == ENPWM) &&
//            ( (STR_FUNC_Gvar.MonitorFlag.bit.RunMod > SPDMOD) || (STR_FUNC_Gvar.MonitorFlag.bit.OTClamp == 1) ) )  //表示伺服处于使能状态
//        {
//            PosRegulator();     //调节器计算
//        }
//        else
//        {
//            STR_FUNC_Gvar.PosCtrl.PosReguOut = 0;
//            STR_FUNC_Gvar.PosCtrl.FdFwdOut = 0;
//        }
//    }

//    STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 0;      //速度调节器清除标志位清零，该标志位不能放在MTR模块SpdReguDatClr()当中,故只能放在此位置!!!    

    //2个AI未采用时 518/120us 
    Ai1Process();
    Ai2Process();
    //Ai3Process();


    // (速度指令部分还是在速度环调度运行2014-06-26)为了提交速度指令响应速度,速度指令部分重新移植到电流环
    //速度模式：斜坡函数启用时454/120us  未启用斜坡函数时386us
    //转矩模式：98/120us
    if( (1 == STR_FUNC_Gvar.ScheldularFlag.bit.SpdFlag) &&
        (0 == STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact) )
    {
        STR_FUNC_Gvar.ScheldularFlag.bit.SpdFlag = 0;     //清除标识

        GetSpdRef();    //获取速度指令包括多段速度和速度指令加减速

        if ( (SPDMOD == STR_FUNC_Gvar.MonitorFlag.bit.RunMod)
            &&((1 == STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn) || (AuxFunCodeUnion.code.H2F_FSAState != 0)))
        {
            // 生成后台模式频谱分析速度指令
            GenerateFSASpdCmd();
        }
    }
    //转矩指令来源获取   转矩指令在电流环
    //速度环调度条件，位置环模式|速度环模式+速度环调节周期到来标志
    //速度模式：38/120us
    //转矩模式：194/120us
    if(0 == STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact)
    {
        if((STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD) || 
           (STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod == 1))
          {
              ToqRefSel();
          }
    }
	//用作自适应滤波器的FFT处理
	FftProcess();
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_AuxFunc_ToqInterrupt(void)
{
    static Uint32 Cnt_4s = 0;

    
    //速度调节器清除标志位清零，该标志位不能放在MTR模块SpdReguDatClr()当中,故只能放在此位置!!!

    STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 0;

#if POWERDRIVER_TYPE==POWDRV_IS650
	INT_16KBrkCurrentCheck();
#endif
        
    INT_16K_DCBusBrkProcess();  //母线电压制动泄放处理

    //摩擦力辨识 
    if(AuxFunCodeUnion.code.FA_FricIdentify == 1)
    {
        Fric_Iden();      
    } 

    //电流环自调谐，电流阶跃波形采集
    if(0 == STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact)
    {   
        PiTuneDataAcqui();          
    }

    G_MTR_FUNCGetFSAList_16kHz( (Uint32 *) &UNI_FUNC_MTRToFUNC_FSAFastList_16kHz.all[0] );
    PosFbCalc_ToqInt();     //位置反馈计算
    GetComValue();

    //示波器采样数据 
    OscilloscopeSampling(); 
      
	if (AuxFunCodeUnion.code.FS_Mode == 2)
    {
        AuxFunCodeUnion.code.OS_SampleLen = 2;
    }
    else
    {
        AuxFunCodeUnion.code.OS_SampleLen = 1;
    }
    FSASendCtrl();  

     //位置控制任务调度,位置环调度条件,位置环模式+位置环周期到来标志
    if( STR_FUNC_Gvar.ScheldularFlag.bit.PosFlag == 1 )
    {
        //触发软中断
        STR_PUB_Gvar.SoftInterruptEn = 1;

        //清除标识
        STR_FUNC_Gvar.ScheldularFlag.bit.PosFlag = 0;
    }

    USART1_RcvDealToqInt();  //转矩中断中查询接收到的数据

    FPGA_ExtInterruptCnt ++;          //监控FPGA中断

    //如果上次FPGA中断运行时间大于电流环调度周期,报警告Er.940
    if(STR_PUB_Gvar.ToqInterrupt_PRTime > (STR_FUNC_Gvar.System.ToqPrd_Q10 >> 10))  PostErrMsg(INTRUNOVERTIME);

    if(Cnt_4s > ((Uint32)STR_FUNC_Gvar.System.ToqFreq << 2))
    {
        Cnt_4s = 0;
        AuxFunCodeUnion.code.ToqInt_PRTime = 0;
    }
    else
    {
        Cnt_4s ++;
    }

    if(STR_PUB_Gvar.ToqInterrupt_PRTime > AuxFunCodeUnion.code.ToqInt_PRTime)
    {
        AuxFunCodeUnion.code.ToqInt_PRTime = STR_PUB_Gvar.ToqInterrupt_PRTime;
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_PostionControl_PosInterrupt(void)
{
    static Uint32 Cnt_4s = 0;
#if ECT_ENABLE_SWITCH

    if(STR_EcatPosSync.PosSyncModeConfigDone==1)
    {
        PosFbCalc_PosSched(); 

        //PosErrCalibration();
    }


    MCZindexClear();
//导入孙正海修改无法jog的bug修改  by huangxin201804 _2
	if((0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd) && 
	   (0 == STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn) && 
	   (0 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) &&
//	   (0 == STR_FUNC_Gvar.Monitor2Flag.bit.GUICtrl_Enable)&&
	   (0 == STR_FUNC_Gvar.Monitor.ResZIndexcmd))
	{
		if(FunCodeUnion.code.BP_ModeSelet==9)
		{
			ECTModeSelect(); 
		}
	}

	ModSelect();
						   
#endif
    DealDiFromGpio();

	
	PosControl();

    //位置调节器清零
    if( (STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr == 1) || 
        (STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop == 1) || 
        (STR_FUNC_Gvar.MonitorFlag.bit.ToqStop == 1) ||
        (STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact == 1) )
    {
        ClrPosReg();        //位置调节器相关参数清零
        STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 0;  
    }

        //伺服使能时在位置模式下时，启动位置调节
    if( (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN) && (STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == ENPWM) &&
        ( (STR_FUNC_Gvar.MonitorFlag.bit.RunMod > SPDMOD) || (STR_FUNC_Gvar.MonitorFlag.bit.OTClamp == 1) ) )  //表示伺服处于使能状态
    {
        PosRegulator();

        if(FunCodeUnion.code.AT_LowOscMod == 1)
        {
            LowOscSelfCal();       //低频自动测试
        }
    }
    else
    {
        //ClrPosReg();      //此处清零有问题，后期需要考虑  位置调节器相关参数清零
        STR_FUNC_Gvar.PosCtrl.PosReguOut = 0;
        STR_FUNC_Gvar.PosCtrl.FdFwdOut = 0;
    }


    if(0 == STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact)
    { 
        GainSwitching(); //增益切换
   
        SpdLmtSel();    //转矩环速度限制
    }

    //当转矩模式切换到位置或速度模式时，切换过渡标志位清零
    if(1 == STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod)
    {
        STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod = 0;
    }

    //DealDiFromGpio();

    //如果软中断运行时间大于位置环调度周期,报警告Er.940
    if(STR_PUB_Gvar.PosInterrupt_PRTime > (STR_FUNC_Gvar.System.PosPrd_Q10 >> 10))  PostErrMsg(INTRUNOVERTIME);

    ServiceDog();   //喂狗

    if(Cnt_4s > ((Uint32)STR_FUNC_Gvar.System.PosFreq << 2))
    {
        Cnt_4s = 0;
        AuxFunCodeUnion.code.PosInt_PRTime = 0;
    }
    else
    {
        Cnt_4s ++;
    }

    if(STR_PUB_Gvar.PosInterrupt_PRTime > AuxFunCodeUnion.code.PosInt_PRTime)
    {
        AuxFunCodeUnion.code.PosInt_PRTime = STR_PUB_Gvar.PosInterrupt_PRTime;
    }
}

/*******************************************************************************
  函数名:   void FUNC_System_AuxInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:    功能模块的辅助中断处理主要处理伺服Reset时通讯显示和Reset完成后通讯处理
********************************************************************************/ 
void FUNC_System_AuxInterrupt(void)
{
    static Uint8 DispPrescaler = 0;
    
    if( STR_PUB_Gvar.AllInitDone == 1 )  //初始化完成
    {
   
    }
    else //初始化未完成 
    {
        //初始化未完成 显示 rESEt
        if(( DispPrescaler & 0x3) == 0)
        {
            SpiRxTxScan(SPI_RX_STEP);
            PanelDisplay();
            EepromRwWatchDog(); 
            SpiRxTxScan(SPI_TX_STEP);
        }
        DispPrescaler ++;
    }

}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void FUNC_MainLoop(void)
{
    static Uint8 UpdatePrescaler = 0;
    static Uint8 CutoffSaveDone = 0;
    static Uint32 Cnt_4s = 0;
    static Uint8  PowerOnCnt = 0;

    Uint32 Counter = 0;
	Uint16 StationAlias;

    /* 以下是主循环中由MTR传递给FUNC，供FUNC使用的接口变量，该接口变量调度频率为 1K */ 

    G_MTR_FUNCGetList_1kHz( (Uint32 *) &UNI_FUNC_MTRToFUNC_SlowList_1kHz.all[0] ); //接口变量传递  MTR传给FUNC模块，供FUNC模块使用

    /* 以上是主循环中由MTR传递给FUNC，供FUNC使用的接口变量，该接口变量调度频率为 1K */
 
    //功能码更新256ms一次
    UpdatePrescaler ++;
#if ECT_ENABLE_SWITCH
    //放在之前更新
    FactorUpdate();
#endif
    CANopenPosLimt();
	
	if(UpdatePrescaler == 128)
    {
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)
        {
            //电机与驱动器匹配判断
            if(FunCodeUnion.code.MT_RateCurrent > FunCodeUnion.code.OEM_RateCurrent)
            {
                PostErrMsg(OVDRVRATECUR); //产品匹配故障
            }

            if(0 != FunCodeUnion.code.BP_AbsPosDetectionSel)
            {
                //使能绝对位置检测系统时判断电机型号是否符合,如果不符合报120故障 
                //我司使用绝对位置功能时，电机编号前三位是141，电机参数存储在编码器内部
                if((14100 != FunCodeUnion.code.MT_AbsEncType) || (141 != (FunCodeUnion.code.MT_MotorModel / 100)))
                {                 
                    PostErrMsg(ABSPOSMATCHERR);
                }
            } 
            
            //DiDoStateStopUpdata();  //DiDo 停机更新
            StopModeInit();      //用于初始化故障与保护需要停机设定的功能码以及停机模式的初始化
            PosCtrlStopUpdata(); 
            PosReguStopUpdata(); 
            SpdCtrlStopUpdate();
            InitMotorOverLoadProc();

            StopUpdateDCBusBrake();                          
			FftStopUpdate(); 
        }
        //以下为运行更新
        AiConstInit();
        SpdCtrlUpdate();         
        PosCtrlUpdata();  
        PosReguUpdata();

        GetIPM_4Hz_Temperature();//IPM温度显示（20Hz更新即可）
        //AoConst_Update();       
    }

    //原来MTR模块监控部分函数
    //母线电压监控,10ms开始检测
    if(PowerOnCnt > 10)  //母线电压监控
    {
        DCBusVoltErr_Monitor();//zxh20120223 
        DCBusVoltBrake_Monitor();
        
    }
    else
    {
        PowerOnCnt ++;
    }

	CSPBufferVarUpdate();
    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)MultiBlockSpeedInit();

    CtrlDCBusVoltBrake_Monitor();

    PL_1k_Monitor();                //输入电源缺主循环1K监控
    
    DcBusVolt_SoftStart();


    OverLoadProtect_1k_Monitor();   //驱动器电机过载保护

    MTOffLine_1k_Monitor();         //电机动力线断线监控

    LineVolt_Monitor();

    Tempera_1k_Monitor();           //温度监控（暂时只有IPM温度监控）


    if((UpdatePrescaler & 0x03) == 1)
    {
        Rigidity_LevelProcess();     //更新刚性表
        GainSwitchingUpdate();     //增益切换更新函数 4ms更新一次
		FftUpdate();			   //更新FFT计算参数
    }

    PosCtrl_MainLoop();
    
    if((STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate == 1)||(STR_FUNC_Gvar.MonitorFlag.bit.ErrResetExecut == 1))
    {
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)
        {
            DiDoStateStopUpdata();
            STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate = 0;
        }
        AoConst_Update();

    } 

    //H30组功能码更新
    H30_FunCode_Update();

    //调度频率250Hz
    if((UpdatePrescaler & 0x03) == 1)
    {
        PanelKey();
    }
    else if((UpdatePrescaler & 0x03) == 3) 
    {
        PanelDisplay();
    }

    H0B_FunCode_Display();   //H0B组变量显示，该在主循环中调度，其调度频率为1K 

    TorqLoopPiTune();    //电流环自动调谐主函数

    SpiRxTxScan(SPI_RX_STEP);
    
    ServoMonitor();

    ServoRunManage();

    SpiRxTxScan(SPI_TX_STEP);

    EepromRwWatchDog();

    AoProcess();

    EepromProcess();

	DoHoming();      // 原点回归
#if ECT_ENABLE_SWITCH
    CanopenHomingModeFunc();//Canopen Homing Mode
#endif
	
    COMM_ModbusDeal_MainLoop();     //Modbus主循环调度函数

    OscilloscopeProcess();  //示波器主循环调度函数

#if ECT_ENABLE_SWITCH
    //H0c05默认是0，读取上位机写入EEPROM中的站号，如果未接上位机则是1，如果H0C05中非0，则取决于设定值。
	StationAlias = FunCodeUnion.code.CM_StationAlias;
//    EcatMainLoop(FunCodeUnion.code.CM_SyncErrCnt,(Uint16 *)&FunCodeUnion.code.CM_StationAdd,FunCodeUnion.code.CM_StationAlias,&StationAlias,FunCodeUnion.code.CM_SyncDetecMethod);
    EcatMainLoop(FunCodeUnion.code.CM_SyncErrCnt,(Uint16 *)&FunCodeUnion.code.CM_StationAdd, 
                (Uint16 *)&FunCodeUnion.code.CM_StationAlias,FunCodeUnion.code.CM_SyncDetecMethod);
	if(StationAlias !=FunCodeUnion.code.CM_StationAlias)
	{
		StationAlias = FunCodeUnion.code.CM_StationAlias;
		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_StationAlias));
	}

    ESMStateMonitor();
#endif

    ResetFunCode();

    //掉电保存
    if((1 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave)
      && (0 == CutoffSaveDone))
    {
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++; 
            if(Counter >= 2000000L) break;

            //不要忘了喂狗
            #if DEBUG_WD_ENABLE 
            ServiceDog();
            #endif
        }
        CutoffSaveDone = 1;
    }

    if(0 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave)
    {
        CutoffSaveDone = 0;
    }

#if DEBUG_WD_ENABLE 
    if((AuxFunCodeUnion.code.FA_SoftRst == 1) && 
    ( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN )&&
//    ( STR_ServoMonitor.ErrorFlag.bit.Err1UnRstFlag == 0) &&
    ( EepromProcess() != 1) )
    {
    	if((FunCodeUnion.code.MT_EncoderSel == 0x13)||(FunCodeUnion.code.MT_EncoderSel == 0x16)) AbsRom_BaudSet_2MHZ();   //by huangxin201804 _4 导入孙正海修改误报136bug
        STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave = 1;
        //掉电功能码保存
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++; 
            if(Counter >= 2000000L) break;

            //不要忘了喂狗
            #if DEBUG_WD_ENABLE 
            ServiceDog();
            #endif            
        }

        AuxFunCodeUnion.code.FA_SoftRst = 2; 
    }
    else if(AuxFunCodeUnion.code.FA_SoftRst == 3)
    {
        static Uint8 ResetCnt = 0;

        ResetCnt ++;
        if(ResetCnt > 10) WDSoftwareReset();  //利用WWDG触发软件复位            
    }
    else
    {
        AuxFunCodeUnion.code.FA_SoftRst = 0;
    }

    ServiceDog();   //喂狗
#endif 

    ElcLabelDeal(); 

    MonitFPGA_ExternalInterrupt();

    if(Cnt_4s > 4000)
    {
        Cnt_4s = 0;
        AuxFunCodeUnion.code.DP_MainLoop_PSTMax = 0;
        AuxFunCodeUnion.code.DP_MainLoop_PRTMax = 0;
    }
    else
    {
        Cnt_4s ++;
    }

    if(STR_PUB_Gvar.MainLoop_PSTime > AuxFunCodeUnion.code.DP_MainLoop_PSTMax)
    {
        AuxFunCodeUnion.code.DP_MainLoop_PSTMax = STR_PUB_Gvar.MainLoop_PSTime;
    }

    if(STR_PUB_Gvar.MainLoop_PRTime > AuxFunCodeUnion.code.DP_MainLoop_PRTMax)
    {
        AuxFunCodeUnion.code.DP_MainLoop_PRTMax = STR_PUB_Gvar.MainLoop_PRTime;
    }
}
/*******************************************************************************
  函数名:  void H0B_FunCode_Display()
  输入:    
  输出:    
  子函数:         
  描述:  H0B 组功能码显示
********************************************************************************/
Static_Inline void H0B_FunCode_Display(void)
{
    PosShow();  
    SpeedShow();
    TorqueShow(); 
    AiShow();   
    MonitorShow();
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  H30 组功能码显示
********************************************************************************/

Static_Inline void H30_FunCode_Update(void)
{
    Uint16 Temp = 0;

    AuxFunCodeUnion.code.CR_SVST = STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy;
    
    Temp = STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus;
    AuxFunCodeUnion.code.CR_SVST += Temp << 12;     
    
    AuxFunCodeUnion.code.H2F_HighLvlErrCode = STR_FUNC_Gvar.Monitor.HighLevelErrCode;   
}

/**
 * @brief  Return the Main Stack Pointer
 *
 * @return Main Stack Pointer
 *
 * Return the current value of the MSP (main stack pointer)
 * Cortex processor register
 */
__asm Uint32 __get_MSP0(void)
{
  mrs r0, msp
  bx lr
}

/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
void HardFault_Handler(void)
{
    Uint32 SPAddr;    
    Uint16 i = 0;
    Uint16 j = 0;

    DINT;

    SPAddr = __get_MSP0();

    //先记录堆栈信息
    HFInfor.Reg_32Bits[0] =  * ((Uint32 *)SPAddr);         //R0
    HFInfor.Reg_32Bits[1] =  * ((Uint32 *)(SPAddr + 4));   //R1
    HFInfor.Reg_32Bits[2] =  * ((Uint32 *)(SPAddr + 8));   //R2
    HFInfor.Reg_32Bits[3] =  * ((Uint32 *)(SPAddr + 12));   //R3
    HFInfor.Reg_32Bits[4] =  * ((Uint32 *)(SPAddr + 16));   //R12
    HFInfor.Reg_32Bits[5] =  * ((Uint32 *)(SPAddr + 20));   //LR
    HFInfor.Reg_32Bits[6] =  * ((Uint32 *)(SPAddr + 24));   //PC
    HFInfor.Reg_32Bits[7] =  * ((Uint32 *)(SPAddr + 28));   //PSR

    //记录寄存器信息
    HFInfor.Reg_32Bits[8] =  * ((Uint32 *)(0xE000ED28));   //CFSR
    HFInfor.Reg_32Bits[9] =  * ((Uint32 *)(0xE000ED2C));   //HFSR
    HFInfor.Reg_32Bits[10] =  * ((Uint32 *)(0xE000ED30));   //DFSR
    HFInfor.Reg_32Bits[11] =  * ((Uint32 *)(0xE000ED3C));   //AFSR
    HFInfor.Reg_32Bits[12] =  * ((Uint32 *)(0xE000ED38));   //BFSR
    HFInfor.Reg_32Bits[13] =  * ((Uint32 *)(0xE000ED34));  	//MMAR   内存管理错误发生的精确地址
                                                           	//BAR    总线错误发生的精确地址 与MMAR共享一个物理存储空间    
    //Fault 状态寄存器清零
    * ((Uint32 *)(0xE000ED28)) = 0xffffffff;          
    * ((Uint32 *)(0xE000ED2C)) = 0xffffffff;               //防止影响下一次Fault的响应 


    STR_PUB_Gvar.AllInitDone = 2;

    while(1)
    {    
        ServiceDog();
        
        SpiRxTxScan(SPI_RX_STEP);
        PanelDisplay(); 
        SpiRxTxScan(SPI_TX_STEP); 
        
        j = 5000;    
        while(j--)
        {
          i = 21;    //测试结果如上
          while(i--);
        }
    }
}
/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void MonitFPGA_ExternalInterrupt(void)
{
    static Uint16 DelayCnt = 0;
    static Uint32 FPGA_ExtInterruptCntLatch = 0;
    static Uint16 TimeLatch = 0;
    Uint16 Temp = 0;

    if(DelayCnt < 1000) 
    {   
        DelayCnt ++;
        FPGA_ExtInterruptCntLatch = FPGA_ExtInterruptCnt;
        TimeLatch = GetSysTime_1MHzClk();
        return;
    } 
    
    Temp = GetSysTime_1MHzClk() - TimeLatch;       
    if(Temp > 2000) 
    {
        if( (FPGA_ExtInterruptCntLatch == FPGA_ExtInterruptCnt)     //角度辨识不报警
         && (AuxFunCodeUnion.code.OEM_ResetABSTheta != 1) 
         && (Temp < 10000) ) 
        {
            PostErrMsg(FPGAINTERR); 
            STR_FUNC_Gvar.ScheldularFlag.bit.FPGAIntErr = 1;
            if(2 == UNI_EleLabel.Label.JumpFlag)   UNI_EleLabel.Label.JumpFlag = 3; 

            //获取ADC采样值 
            Get_ADC_ChannelValueAIUdc_16k(); 
            ADC_START_ADC1(); 
        }

        TimeLatch = GetSysTime_1MHzClk();
        FPGA_ExtInterruptCntLatch = FPGA_ExtInterruptCnt; 
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void ReadUniqueDeviceID(void)
{
	 Uint16 * p = (Uint16 *)0x1FFF7A10;

	 FunCodeUnion.code.H0F_Rsvd30 = *p;

	 p++;
	 FunCodeUnion.code.H0F_Rsvd31 = *p;

	 p++;
	 FunCodeUnion.code.H0F_Rsvd32 = *p;

	 p++;
	 FunCodeUnion.code.H0F_Rsvd33 = *p;

	 p++;
	 FunCodeUnion.code.H0F_Rsvd34 = *p;

	 p++;
	 FunCodeUnion.code.H0F_Rsvd35 = *p;
}
/********************************* END OF FILE *********************************/

