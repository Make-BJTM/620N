/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_System.c                                                           
 创建人：朱祥华            创建日期：2011.11.09 
 修改人：XX                修改日期：XX.XX.XX 
 描述： 
    1. 电机全局变量声明文件
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
#include "MTR_InterfaceProcess.h"   
#include "MTR_System.h"
#include "MTR_GPIODriver_ST.h"
#include "MTR_FPGAInterface.h" 
#include "MTR_GetIqRef.h"
#include "MTR_SpdRegulator.h"
#include "FUNC_MTRInterface.h"
#include "PUB_Main.h"

//辅助功能
#include "MTR_InertiaIdy.h"
#include "MTR_AngleInit.h"
#include "MTR_InerFricIden.h"
#include "MTR_GPIODriver_ST.h"   //临时放置

#include "MTR_AbsEncAngleInit.h"
#include "MTR_AbsRomOper.h"
#include "MTR_Nikon_AbsRomOper.h"
#include "MTR_Tamagawa_AbsRomOper.h"

#if HDH_ENCODER_SW
    #include "MTR_HDH_RomOper.h"
#endif

#include "stm32f4xx.h"
#include "MTR_RDCOper.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
STR_MTR_GLOBALVARIABLE              STR_MTR_Gvar; 


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */




/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明的*/
//初始化相关调度程序
void MTR_PeripheralConfig_RST(void);
void MTR_Parameter_Frist_RST(void);
void MTR_Parameter_Second_RST(void);
void MTR_Interrupt_RST(void);
void MTR_UpdateSysFreqAndPrd(void);
//中断相关调度程序
void MTR_GetPara_ToqInterrupt(void);
void MTR_ReguControl_ToqInterrupt(void);
void MTR_System_AuxInterrupt(void);  //电机模块的辅助中断处理主要软件启动ST芯片的ADC和读取ADC采样值
void MTR_PostionControl_PosInterrupt(void);//MTR位置环控制中断处理

//主循环调度程序
void MTR_MainLoop(void);

//
void ZPosErrDeal(void);

void AbsRom_InitDeal(void);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void MTR_SYS_Schedular_RealTime(void); //任务调度器，置各种任务的标志位
Static_Inline void Init_Tim5EncInterfaceMode(void);

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void MTR_PeripheralConfig_RST()
{
    InitMTR_GPIO();       //初始化功能模块的GPIO
    
    if(0 == FPGA_IsFPGARdy())     //判断FPGA是否准备好接收读写数据；
    {
        PostErrMsg(FPGAINITERR);
    } 
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void MTR_Parameter_Frist_RST()
{
    /* 以下是FUNC模块传递给MTR模块，供MTR使用的接口变量，在初始化时调用一次 */
    G_FUNC_MTRGetList_4Hz_32Bits((Uint32 *) &UNI_MTR_FUNCToMTR_List_4Hz_32Bits.all[0]);

    G_FUNC_MTRGetList_16kHz((Uint32 *) &UNI_MTR_FUNCToMTR_List_16kHz.all[0]);

    /* 以上是FUNC模块传递给MTR模块，供MTR使用的接口变量，在初始化时调用一次 */

    MTR_UpdateSysFreqAndPrd();  //该函数内有载波，三环调度频率接口必须在接口函数后调用

    InitFPGA();               //把需要配置完DSP的内容后再配置FPGA内容

    InitGetIqRef();           //转矩指令获取初始化

    InitIqLmtValue();        //转矩限制值初始化

    InitSpdRegulator();       //速度调节器初始化

    SpdReguStopUpdata();     //速度调节器停机更新

    GetIqRefStopUpdate();    //获取转矩指令停机更新

    SpdReguUpdata();   //速度调节器运行更新

    IqLmtUpdateFun();  //转矩限制运行更新

    GetIqRefUpdate();  //获取转矩指令运行更新

    Init_OnLineInertiaIdy();     //在线惯量辨识上电初始化程序
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void MTR_Parameter_Second_RST()
{


}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void MTR_Interrupt_RST()
{
    Init_Tim5EncInterfaceMode();
    Init_FPGAInterrupt();
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void Init_Tim5EncInterfaceMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef    TIM5_TimeBaseStructure;     

    // 6. A/U输入          PA0   ->  FPGA管脚 L15  电路图网络IO4   
    // 7. B/V输入          PA1   ->  FPGA管脚 L16  电路图网络IO5
    //只对增量式编码器有效
    if(0x0000 != (FunCodeUnion.code.MT_EncoderSel & 0x0f0)) return; 

    /* GPIOA Configuration: TIM5 CH1 (PA0) and TIM5 CH2 (PA1) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Connect TIM pins to AF3 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM5);

    TIM_DeInit(TIM5);
    TIM_TimeBaseStructInit(&TIM5_TimeBaseStructure);

    // 确定定时器5的时基 
    TIM5_TimeBaseStructure.TIM_Period            = STR_MTR_Gvar.FPGA.EncRev - 1;
    TIM5_TimeBaseStructure.TIM_Prescaler         = 0;
    TIM5_TimeBaseStructure.TIM_ClockDivision     = 0;
    TIM5_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM5_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM5,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);

    /* TIM enable counter */
    TIM_Cmd(TIM5, ENABLE);
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/
void ZPosErrDeal(void)
{
    static int32  ZPosFir = 0x7FFFFFFF;
    static int32 ErrMax = 0;
    int32 Temp = 0;

    //只对增量式编码器有效
    if(0x0000 != (FunCodeUnion.code.MT_EncoderSel & 0x0f0)) return;

    STR_MTR_Gvar.FPGA.PosFdbAbsVal_ZInt = STR_MTR_Gvar.FPGA.PosFdbAbsValue;

    if(0x7FFFFFFF == ZPosFir)
    {
        ZPosFir = TIM_GetCounter(TIM5);
        AuxFunCodeUnion.code.DP_ZPosErr = 0;
        return;
    }

    Temp = (int32)TIM_GetCounter(TIM5) - (int32)ZPosFir;

    if(Temp >= (int32)(STR_MTR_Gvar.FPGA.EncRev >> 1)) Temp = Temp - (int32)STR_MTR_Gvar.FPGA.EncRev;
    if(Temp <= (0 - (int32)(STR_MTR_Gvar.FPGA.EncRev >> 1))) Temp = Temp + (int32)STR_MTR_Gvar.FPGA.EncRev; 

    if(Temp > 9999) Temp = 9999;
    else if(Temp < -9999) Temp = -9999;

    AuxFunCodeUnion.code.DP_ZPosErr = (Uint16)Temp;

    if(ABS(ErrMax) < ABS(Temp))
    {
        ErrMax = Temp; 
        AuxFunCodeUnion.code.DP_ZPosErrMax = (Uint16)ErrMax;
    }
}
/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:    上电时更新功能模块内所需调用的相关调度频率和周期
********************************************************************************/

Static_Inline void MTR_UpdateSysFreqAndPrd()
{
    STR_MTR_Gvar.System.CarFreq   = FunCodeUnion.code.OEM_CarrWaveFreq;   //PWM载波频率

    if(FunCodeUnion.code.OEM_ToqLoopFreqSel == 1) //转矩环调节频率选择
    {
        STR_MTR_Gvar.System.ToqFreq   = FunCodeUnion.code.OEM_CarrWaveFreq << 1; //转矩环调节频率
    }
    else
    {
        STR_MTR_Gvar.System.ToqFreq   = FunCodeUnion.code.OEM_CarrWaveFreq;       //转矩环调节频率
    }

    STR_MTR_Gvar.System.SpdFreq   = STR_MTR_Gvar.System.ToqFreq / FunCodeUnion.code.OEM_SpdLoopFreqScal;   //速度环调节频率

    STR_MTR_Gvar.System.SpdCnt      = 0; 

    STR_MTR_Gvar.System.ToqPrd_Q10 = ((Uint32)1000000 << 10) / STR_MTR_Gvar.System.ToqFreq;     //周期单位为us 
    STR_MTR_Gvar.System.SpdPrd_Q10 = ((Uint32)1000000 << 10) / STR_MTR_Gvar.System.SpdFreq;     //周期单位为us 
} 



/*******************************************************************************
  函数名:  MTR_GetPara_ToqInterrupt(void)      
  输入: 
  输出:  
  子函数:无         
  描述:   

********************************************************************************/ 
void MTR_GetPara_ToqInterrupt(void)
{
    //GetFPGAParam()和 MTR_SYS_Schedular_RealTime()运行最短时间 364/120us
    //运行最长时间 386/120us

    //任务调度器，置各种任务的标志位 34/120us  
    MTR_SYS_Schedular_RealTime();

    //接收FPGA至DSP状态参数  366/120us 
    GetFPGAParam(); 

    if((0x13 == FunCodeUnion.code.MT_EncoderSel) || (0x16 == FunCodeUnion.code.MT_EncoderSel))    //汇川编码器
    {
    #if HC_ENC_SW
        AbsRom_EncState();      //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    #endif     
    }
#if NOKIN_ENC_SW
    else if(0x12 == FunCodeUnion.code.MT_EncoderSel)   //尼康绝对式编码器
    {        
        NKAbsRom_EncState();      //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    }
#endif

#if TAMAGAWA_ENC_SW
    else if(0x10 == FunCodeUnion.code.MT_EncoderSel)   //尼康绝对式编码器
    {        
        TAMAGAbsRom_EncState();      //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    }
#endif
#if HDH_ENCODER_SW
    else if(0x11 == FunCodeUnion.code.MT_EncoderSel)    //海德汉绝对式编码器
    {        
        HDHAbsRom_EncState();      //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    }
#endif 
    //旋变解码芯片运行状态
    else if(FunCodeUnion.code.MT_EncoderSel == 0x20)
    {
        RDConverterState();
    }
    else
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;
    }


}  



/*******************************************************************************
  函数名:   MTR_SYS_Schedular_RealTime(void)
  输入:    
  输出:    
  子函数:         
  描述:   任务调度器，置各种任务的标志位
********************************************************************************/ 
Static_Inline void MTR_SYS_Schedular_RealTime(void)
{
    //速度环
    STR_MTR_Gvar.System.SpdCnt ++;
    if( STR_MTR_Gvar.System.SpdCnt >= FunCodeUnion.code.OEM_SpdLoopFreqScal)
    {
        STR_MTR_Gvar.System.SpdCnt = 0;
        STR_MTR_Gvar.ScheldularFlag.bit.SpdFlg = 1;                    //置速度环调度标志 bit01 2K
        STR_MTR_Gvar.GetIqRef.PseudoSpdFlg = 1;
    }
}

/*******************************************************************************
  函数名:   MTR_ReguControl_ToqInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:   电机模块电流环中断处理，包括速度调节器和转矩指令的获取和处理
********************************************************************************/ 
void MTR_ReguControl_ToqInterrupt(void)
{
    //FUNC模块传递给MTR模块，供MTR使用的接口变量 94/120 us
    G_FUNC_MTRGetList_16kHz((Uint32 *) &UNI_MTR_FUNCToMTR_List_16kHz.all[0]);

    //将条件判断从函数内部移动到调度处
    //不使能角度辨识时运行时间 38/120 us
    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0)    //省线式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)
        {
            if(FunCodeUnion.code.ER_AngIntSel == 0)
            {
                AngInt_IncEncResetFromZ();         //进行角度辨识操作，需要找Z信号
            }
            else if(FunCodeUnion.code.ER_AngIntSel == 1)
            {
                AngInt_IncEncReset();              //进行角度辨识操作,不需要找Z信号
            }
            else          //电压注入方式辨识
            {
                AngInt_CurVct();
            }
        }
    }
    else if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)    //光栅尺编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)
        {
            if(FunCodeUnion.code.ER_AngIntSel == 2)    //电压注入方式
            {
                AngInt_CurVct();
            }
            else       //不用找Z方式进行辨识
            {
                AngInt_IncEncReset();              //进行角度辨识操作,不需要找Z信号
            }
        }
    }
#if HC_ENC_SW
    else if((FunCodeUnion.code.MT_EncoderSel == 0x13) || (FunCodeUnion.code.MT_EncoderSel == 0x16))    //汇川绝对式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)    //H0D03绝对式编码器电角度初始化
        {
            SerialCommEncAngleInit();      
        }
        
        if((AuxFunCodeUnion.code.MT_OperAbsROM == 1)      //H0D04编码器ROM区写操作
           || (AuxFunCodeUnion.code.MT_OperAbsROM == 2))    //H0D04编码器ROM区读操作
        {
            AbsRomProcess();
        }         
    }
#endif

#if NOKIN_ENC_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x12)    //尼康绝对式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)    //H0D03绝对式编码器电角度初始化
        {
            SerialCommEncAngleInit();      
        }
        
        if((AuxFunCodeUnion.code.MT_OperAbsROM == 1)      //H0D04编码器ROM区写操作
           || (AuxFunCodeUnion.code.MT_OperAbsROM == 2))    //H0D04编码器ROM区读操作
        {
            if(0 == FunCodeUnion.code.DisRdEncEepromPwrOn) NKAbsRomProcess();
            else AuxFunCodeUnion.code.MT_OperAbsROM = 0;
        }         
    }
#endif

#if TAMAGAWA_ENC_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x10)    //多摩川绝对式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)    //H0D03绝对式编码器电角度初始化
        {
            SerialCommEncAngleInit();      
        }
                 
        if((AuxFunCodeUnion.code.MT_OperAbsROM == 1)      //H0D04编码器ROM区写操作
           || (AuxFunCodeUnion.code.MT_OperAbsROM == 2))    //H0D04编码器ROM区读操作
        {
            if(0 == FunCodeUnion.code.DisRdEncEepromPwrOn) TAMAGAbsRomProcess();
            else AuxFunCodeUnion.code.MT_OperAbsROM = 0;
        }         
    }
#endif

#if HDH_ENCODER_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x11)    //绝对式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)    //H0D03绝对式编码器电角度初始化
        {
            SerialCommEncAngleInit();                   
        }
        
        if((AuxFunCodeUnion.code.MT_OperAbsROM == 1)      //H0D04编码器ROM区写操作
           || (AuxFunCodeUnion.code.MT_OperAbsROM == 2))    //H0D04编码器ROM区读操作
        {        
            HDHAbsRomProcess();         
        }         
    }
#endif
    else if(FunCodeUnion.code.MT_EncoderSel == 0x20)    //绝对式编码器
    {
        if(AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)    //H0D03绝对式编码器电角度初始化
        {
            SerialCommEncAngleInit();                   
        }        
    }


    //非转矩模式下运行最大时间 1880/120us
    if(0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdToqLoopNact)
    {   //若非惯量辨识则为正常的速度转矩环调度模式
        //速度控制任务，调度速度环调度条件，位置环模式|速度环模式+速度环调节周期到来标志
        if(STR_MTR_Gvar.ScheldularFlag.bit.SpdFlg == 1 )      
        { 		
            if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdReguDatClr == 1) ||
                (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ToqStop == 1))
            {
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdReguDatClr = 0;
                SpdReguDatClr();
            }          

            //最大运行时间1740/120us
            if ( (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == RUN) && 
                 (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus == ENPWM) &&
                 ( ((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.RunMod > TOQMOD) &&
                    (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ModSwitchPeriod == 0)) || 
                   (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.OTClamp == 1) || 
                   (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ZeroSpdStop == 1) ) 
               )  //表示伺服处于使能状态
            {
                Nomal_SpdSchedueMode();
            }
            else
            {
               SpdReguDatClr();
            }
            
            //非转矩模式下运行时间 136/120us
            GetIqRef(); 

        }//end of  速度环 
    }

    //UV相电流的校正,使相电流采样值在通同一直流时能保持平衡    
    //未启用时18/120us
    if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.UVAdjustRatioEn == 0) 
    {
        STR_MTR_Gvar.FPGA.UVAdjustResult = 0;
    }
    else
    {
        FPGA_UToVCoff();
    }

    //FPGA参数配置
    SetFPGAParam();

    SpdReguCoefUpdata();  //速度环调节器系数和转矩指令滤波系数更新

    //SWOvCur_FdbErrMonitor();   //软件过流监控应移至电流环

    //在线惯量辨识使能时 运行时间120/120 us
    if(0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdToqLoopNact)
    {   
        if(STR_MTR_Gvar.ScheldularFlag.bit.SpdFlg == 1 )      
        {
            if(STR_MTR_Gvar.GlobalFlag.bit.InertiaIdyEn == 1)
            {
                OnLnInertia_Sample(); //在线惯量辨识数据采集
            }
        }
    }

    if(FunCodeUnion.code.InertiaIdyCountModeSel != 0)     //启动在线惯量辨识时
    {
        FricProcess();
    }

#if HC_ENC_SW
    if((FunCodeUnion.code.MT_EncoderSel == 0x13)  && (23 == (FunCodeUnion.code.MT_ABSEncVer / 1000)))
    {
        if(AuxFunCodeUnion.code.FA_AbsEncRst == 1) AuxFunCodeUnion.code.FA_AbsEncRst = HC2ndAbsRom_ClcErrAndMultiTurn(1);
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 2) AuxFunCodeUnion.code.FA_AbsEncRst = HC2ndAbsRom_ClcErrAndMultiTurn(2);
    }
#endif
    //多圈绝对编码器 
#if NOKIN_ENC_SW
    if(FunCodeUnion.code.MT_EncoderSel == 0x12)
    {
        if(1 == AuxFunCodeUnion.code.FA_NK_HDH_AbsEncRdErrStatus) AuxFunCodeUnion.code.FA_NK_HDH_AbsEncRdErrStatus = NKAbsRom_RdErrStatus();
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 1) AuxFunCodeUnion.code.FA_AbsEncRst = NKAbsRom_ClcErrAndMultiTurn(1);
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 2) AuxFunCodeUnion.code.FA_AbsEncRst = NKAbsRom_ClcErrAndMultiTurn(2);
    }
#endif
#if TAMAGAWA_ENC_SW
    if(FunCodeUnion.code.MT_EncoderSel == 0x10)
    {
        if(AuxFunCodeUnion.code.FA_AbsEncRst == 1) AuxFunCodeUnion.code.FA_AbsEncRst = TAMAGAbsRom_ClcErrAndMultiTurn(1);
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 2) AuxFunCodeUnion.code.FA_AbsEncRst = TAMAGAbsRom_ClcErrAndMultiTurn(2);         
    }
#endif

#if HDH_ENCODER_SW
    if(FunCodeUnion.code.MT_EncoderSel == 0x11)
    {    
        if(AuxFunCodeUnion.code.FA_NK_HDH_AbsEncRdErrStatus == 1) AuxFunCodeUnion.code.FA_NK_HDH_AbsEncRdErrStatus = HDHAbsReadErrReg();
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 1) AuxFunCodeUnion.code.FA_AbsEncRst = HDHAbsReset();
        else if(AuxFunCodeUnion.code.FA_AbsEncRst == 2) AuxFunCodeUnion.code.FA_AbsEncRst = HDHAbsResetAndMultiTurn();    
    }
#endif

    if(2 == AuxFunCodeUnion.code.FA_SoftRst)     //软件复位
    { 
        AuxFunCodeUnion.code.FA_SoftRst = AbsRom_SoftRstProcess();
    }

    if(2 == UNI_EleLabel.Label.JumpFlag)     //下载加密
    {
        UNI_EleLabel.Label.JumpFlag = AbsRom_SoftRstProcess();
    }

    STR_MTR_Gvar.ScheldularFlag.bit.SpdFlg = 0;   // 速度环调节标志清零

}
/*******************************************************************************
  函数名:   MTR_PostionControl_PosInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:     电机模块的软中断处理：转矩指令限制值获取，实时性要求不高移至位置环软中断处理
********************************************************************************/ 
void MTR_PostionControl_PosInterrupt(void)//MTR位置环控制中断处理
{
    //以下函数实时性要求不高移至位置环软中断
    ToqReachJudge();      //转矩到达判断DO输出 86/72=1.19us
}


/*******************************************************************************
  函数名:   MTR_System_AuxInterrupt(void)
  输入:    
  输出:    
  子函数:         
  描述:     电机模块的辅助中断处理主要软件启动ST芯片的ADC和读取ADC采样值
********************************************************************************/ 
void MTR_System_AuxInterrupt(void)
{

}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void MTR_MainLoop(void)
{
    static Uint8  UpdatePrescaler = 0;

    if(UpdatePrescaler == 0)
    {
        /* 以下是FUNC模块传递给MTR模块，供MTR使用的接口变量，4Hz主循环中调度 */
        G_FUNC_MTRGetList_4Hz_32Bits((Uint32 *) &UNI_MTR_FUNCToMTR_List_4Hz_32Bits.all[0]);
        /* 以下是FUNC模块传递给MTR模块，供MTR使用的接口变量，4Hz主循环中调度 */

        if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN) //非伺服运行状态
        {
            MainLoopStopUpdateFPGA();    //主循环更新FPGA相关控制参数  
            SpdReguStopUpdata();     //速度调节器停机更新
	        
            GetIqRefStopUpdate();    //获取转矩指令停机更新
            FricRest();             //惯量及摩擦力辨识参数复位 
        }
        //以下是运行更新
        MainLoopFPGAUpdate(); //主循环运行更新参数如PI电流环参数
		GetIqRefUpdate();  //获取转矩指令运行更新   
        SpdReguUpdata();   //速度调节器运行更新
        FricUpdate();     //惯量及摩擦力辨识参数更新

        if(FunCodeUnion.code.InertiaIdyCountModeSel == 0)  //在使能时如果不在线辨识也需要复位参数
        {
            FricRest();     //惯量及摩擦力辨识参数复位
        }
    }

    IqLmtUpdateFun();       

    //功能码更新256ms一次
    UpdatePrescaler ++;

    //if(FunCodeUnion.code.InertiaIdyCountModeSel == 0)
	{
	    OnLnInertia_MainLoopSchedule();//在线惯量辨识主循环调度处理程序
	}
   if((FunCodeUnion.code.MT_EncoderSel == 0x13) && (23 == (FunCodeUnion.code.MT_ABSEncVer / 1000)))
   {
    #if HC_ENC_SW
        ClrHC2ndAbsEncWarn();
    #endif
    }
#if NOKIN_ENC_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x12) ClrNKAbsEncWarn();
#endif

#if TAMAGAWA_ENC_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x10) ClrTAMAGAbsEncWarn();
#endif

}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void AbsRom_InitDeal(void)
{
    
	STR_MTR_Gvar.AbsRom.AbsPosDetection = 0;
	if(0 != FunCodeUnion.code.BP_AbsPosDetectionSel) 
    {
        if(141 == (FunCodeUnion.code.MT_MotorModel / 100))
		{
			STR_MTR_Gvar.AbsRom.AbsPosDetection = FunCodeUnion.code.BP_AbsPosDetectionSel;
		}
		else
		{
			//使能绝对位置检测系统时判断电机型号是否符合,如果不符合报122故障 
			if(0 != FunCodeUnion.code.BP_AbsPosDetectionSel)  PostErrMsg(ABSPOSMATCHERR);		
		}       
    }

    switch(FunCodeUnion.code.MT_MotorModel) 
    {
#if HC_ENC_SW
        case 14000: 
        case 14101:
		case 14200: 
            HCAbsRom_InitDeal();
            break; 
#endif
#if NOKIN_ENC_SW
        case 14020:
        case 14120:
        case 14021:
        case 14121:
        case 14022:
        case 14122:
            NKAbsRom_InitDeal();
            break;
#endif 

#if TAMAGAWA_ENC_SW
        case 14130:
            TAMAGAbsRom_InitDeal();
            break;
#endif 

#if HDH_ENCODER_SW
        case 14140:
        case 14141:
            HDHAbsRom_InitDeal();
            break; 
#endif
        default: 
            PostErrMsg(NULLABSMOTORNUM);    
            break;
    }
}

/********************************* END OF FILE *********************************/

