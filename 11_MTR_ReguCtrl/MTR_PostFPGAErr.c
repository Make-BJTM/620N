/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    
 创建人：                          创建日期：  
 修改人：                          修改日期： 
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
#include "MTR_FPGAInterface.h" 
#include "MTR_InterfaceProcess.h"
#include "MTR_GlobalVariable.h"
#include "MTR_AbsRomOper.h"
#include "MTR_Nikon_AbsRomOper.h"
#include "MTR_Tamagawa_AbsRomOper.h"
#include "MTR_HDH_RomOper.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
UNI_FPGA_SYSERR_REG     UNI_FPGA_SysErr = {0};     //⑦系统故障状态寄存器
UNI_FPGA_TMFLT_REG      UNI_FPGA_TmFlt = {0};      //超时故障状态寄存器

extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;
extern volatile union_LNRENCTRL2_REG    LnrEnCtrl2_Reg;       //直线/全闭环外部编码器设置2

extern UNI_ABSERR_REG           AbsErr_Reg;  

#if NOKIN_ENC_SW
    extern UNI_NKABSERR_REG         NKAbsErr_Reg;
    extern UNI_NK_ABSENCSTATE_REG   NKAbsEncState_Reg;
#endif
#if TAMAGAWA_ENC_SW
    extern UNI_TAMAGABSERR_REG      TAMAGAbsErr_Reg;
#endif
#if HDH_ENCODER_SW
extern UNI_HDHABSERR_REG           HDHAbsErr_Reg;           
#endif

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
Uint8 FPGAFltToleranceSt = 0;    //0 允许容错  1已经报不可容错故障，禁止容错 

extern volatile Uint16 *PWMPrd_Mode;    //PWM周期和模式；
extern volatile Uint16 *SysErr;         //系统故障状态
extern volatile Uint16 *TmFlt;          //超时故障状态
extern volatile Uint16 *HostAlarmClr;   //报警清除命令
extern volatile Uint16 *AbsEncErr;      //绝对式编码器故障状态
extern volatile Uint16 *AbsEncState;    //绝对式编码器运行状态

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void FPGA_PostErr(void);               //得到FPGA的报警信号及处理

void ClrHC2ndAbsEncWarn(void);         
void ClrNKAbsEncWarn(void);
void ClrTAMAGAbsEncWarn(void);
void Init_FPGAInterrupt(void);              //初始化FPGA 寄存器PWMPrd_Mod

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void ExecTmFltTolerance(void);
Static_Inline void AdcFltTolerance(void);
Static_Inline void OvrSpdFltTolerance(void);
Static_Inline void McuLostFltTolerance(void);
Static_Inline void RotZCntFltTolerance(void);
Static_Inline void ClcFPGAFltReg(void);
Static_Inline void  LnrEncFltTolerance(void);


/*******************************************************************************
  函数名:  

  输  入:   
  输  出:    
  子函数:                                       
  描  述:   
********************************************************************************/
void Init_FPGAInterrupt(void)
{
    UNI_FPGA_SysErr.all = *SysErr;          //获取FPGA给出的报警信息

    if(UNI_FPGA_SysErr.bit.McuLost == 1) PostErrMsg(MCULOST);

    //清除故障
    ClcFPGAFltReg();

    *PWMPrd_Mode  = STR_MTR_Gvar.FPGA.PWMPrdMode.all; 
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void ClcFPGAFltReg(void)
{
    UNI_FPGA_AlarmClr.bit.HostFltClr = 1;
    *HostAlarmClr = UNI_FPGA_AlarmClr.all;
    UNI_FPGA_AlarmClr.bit.HostFltClr = 1;
    *HostAlarmClr = UNI_FPGA_AlarmClr.all;
    UNI_FPGA_AlarmClr.bit.HostFltClr = 0;
    *HostAlarmClr = UNI_FPGA_AlarmClr.all;
}


/*******************************************************************************
  ① 函数名:  FPGA_PostErr(void)    得到FPGA的报警信号及处理    ^_^
  输入:    FPGA的报错寄存器UNI_FPGA_SysErr
  输出:er208 FPGA系统采样运算超时   er834 FPGA报出的AD采样故障   er201 硬件过流故障
       er207 D/Q电流溢出   erA35 Z断线   er741编码器AB干扰故障
  子函数:无
  描述: 得到FPGA的报警信号及处理
        在GetFPGAParam(void)（从FPGA中获取数据）中调用该函数
********************************************************************************/ 
void FPGA_PostErr(void)
{
    UNI_FPGA_SysErr.all = *SysErr;          //获取FPGA给出的报警信息
    AbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态    

    //打噪声始终会出现7860丢时钟的情况，因此屏蔽
    //Uint32  SdmFltU:1;         //bit 1 U相电流Sigma——Delta调制器故障  
    //Uint32  SdmFltV:1;         //bit 2 V相电流Sigma——Delta调制器故障

    AbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态  

    FPGAFltToleranceSt = 0;

    if((UNI_FPGA_SysErr.bit.Sto1 == 1) || (UNI_FPGA_SysErr.bit.Sto2 == 1))    //STO故障
    {
        *HostSon = DISPWM;
        PostErrMsg(STOINOFF);
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.STOState = 1;

        FPGAFltToleranceSt = 1;
    }
    else
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.STOState = 0;
    }

    //IGBT过流, Er201
    if(UNI_FPGA_SysErr.bit.GateKillFlt == 1)    
    {
        *HostSon = DISPWM;
        PostErrMsg(HWOVERCURRENT);
        FPGAFltToleranceSt = 1;
    }

    if(UNI_FPGA_SysErr.bit.OvrCurU == 1)       
    {
        *HostSon = DISPWM;
        PostErrMsg(UOVERCURRENT);
        FPGAFltToleranceSt = 1;
    }

    if(UNI_FPGA_SysErr.bit.OvrCurV == 1)   
    {
        *HostSon = DISPWM;
        PostErrMsg(VOVERCURRENT);
        FPGAFltToleranceSt = 1;
    }

//    if((UNI_FPGA_SysErr.bit.RotEncAbFlt == 1) || (UNI_FPGA_SysErr.bit.RotEncZFlt == 1))   //编码器AB相同时翻转或Z相缺失常发生在编码器本身问题
//    if(UNI_FPGA_SysErr.bit.RotEncZFlt == 1)   //Z相缺失
//    {
//        *HostSon = DISPWM;
//        PostErrMsg(MULTIRERR);
//        FPGAFltToleranceSt = 1;
//    }

    if(UNI_FPGA_SysErr.bit.OvrCurFlt == 1)  //D/Q轴电流溢出，常发生在U,V 相电流异常, Er207
    {
        PostErrMsg(DQOVRCUR);
        FPGAFltToleranceSt = 1;
    }

    if(UNI_FPGA_SysErr.bit.RotEncUvwFlt == 1)//编码器UVW相逻辑错误,表示上电读到的UVW信息错误
    {
        PostErrMsg(ENFBCHKERR);
        FPGAFltToleranceSt = 1;
    }

    ExecTmFltTolerance();

    McuLostFltTolerance();
	
	LnrEncFltTolerance();
  
     
    if(0x13 == FunCodeUnion.code.MT_EncoderSel)    //汇川编码器
    {
#if HC_ENC_SW
        if((STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 0) 
        && (0 != (AbsErr_Reg.all & 0x1F)))
        {
            PostErrMsg(ENCDERR_Z2);
        }

        if(AbsErr_Reg.bit.AbsCntError == 1)     //计数增量异常
        {
			PostErrMsg(ENCDERR_Z4);
            FPGAFltToleranceSt = 1;
        }

        if(23 == (FunCodeUnion.code.MT_ABSEncVer / 1000))
        {
            if(AbsErr_Reg.HC2ndbit.RotBusy == 1)  PostErrMsg(ENCDERR_Z3);
    
            if(AbsErr_Reg.HC2ndbit.AbsOverHeat == 1) PostErrMsg(ENCODEROT);
                            
            if(0 != STR_MTR_Gvar.AbsRom.AbsPosDetection)
            {  
                if(AbsErr_Reg.HC2ndbit.BatteryAlarm == 1) PostErrMsg(ENCDBATWARN);     //0xe730

                if(AbsErr_Reg.HC2ndbit.BatteryError == 1) PostErrMsg(ENCDBATERR);      //0x6731
    
                if(AbsErr_Reg.HC2ndbit.MultiturnError == 1) PostErrMsg(ENCDMULTIERR);   //0x6733
            }
        } 

        if( (1 == STR_MTR_Gvar.AbsRom.AbsPosDetection) 
         && ( ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32767) || ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32768) ) )
        {
            if(0 == FunCodeUnion.code.EncMultOvDisable) PostErrMsg(ENCDMULTIOV);    //0x6735
        }
#endif 
    }

#if NOKIN_ENC_SW
    else if(0x12 == FunCodeUnion.code.MT_EncoderSel)   //尼康绝对式编码器
    {
        static Uint8 PwmEnFirFlag = 0;
        static Uint8 AbsErrorCnt = 0;
            
        NKAbsErr_Reg.all      = AbsErr_Reg.all;       //故障状态 

        if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 0) 
        {
            if(0 == STR_MTR_Gvar.AbsRom.AbsPosDetection)
            {
                //AbsError:1;  编码器故障（TX端）  CountingError + MultiturnError + BatteryError
                //跟电池有关，用多圈编码器不接电池时上电该位为1，不报错
                if(0 != (NKAbsErr_Reg.all & 0x0F)) PostErrMsg(NKENCRXERR0);
            }
            else
            {
                if(0 != (NKAbsErr_Reg.all & 0x1F)) PostErrMsg(NKENCRXERR0);
            }
        }
        else if(0 == STR_MTR_Gvar.AbsRom.AbsPosDetection)
        {
            if(ENPWM == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus) PwmEnFirFlag = 1; 

            if(0 == PwmEnFirFlag)
            {
                if((NKAbsErr_Reg.bit.AbsError == 1) && (AbsErrorCnt < 8)) AbsErrorCnt++;
                if(AbsErrorCnt >= 8) PostErrMsg(NKENCRXERR0);
            }            
        } 

        if(NKAbsErr_Reg.bit.AbsCntError == 1)     //计数增量异常
        {
            PostErrMsg(NKENCRXERR1);
            FPGAFltToleranceSt = 1;
        }
         
        if(NKAbsErr_Reg.bit.AbsOverRun == 1)     //编码器超限（TX端）
        {
            PostErrMsg(NKENCOVERLIM);
        }
            
        if(NKAbsErr_Reg.bit.AbsBusy == 1)
        {
            NKAbsEncState_Reg.all = *AbsEncState;     //绝对式编码器运行状态

            if( NKAbsEncState_Reg.bit.AbsMode == 1) PostErrMsg(NKENBUSY);     
        }
    
        if(0 != STR_MTR_Gvar.AbsRom.AbsPosDetection)
        {
            if(NKAbsErr_Reg.bit.BatteryAlarm == 1)
            {
                PostErrMsg(ENCDBATWARN);  //0xe730
            }
        } 

        if( (1 == STR_MTR_Gvar.AbsRom.AbsPosDetection) 
         && ( ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32767) || ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32768) ) )
        {
            if(0 == FunCodeUnion.code.EncMultOvDisable) PostErrMsg(ENCDMULTIOV);    //0x6735
        }
    }
#endif 

#if TAMAGAWA_ENC_SW
    else if(0x10 == FunCodeUnion.code.MT_EncoderSel)   // 绝对式编码器
    {        
        TAMAGAbsErr_Reg.all      = AbsErr_Reg.all;       //故障状态 

        if((STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 0) 
        && (0 != (TAMAGAbsErr_Reg.all & 0x1F)) )
        {
            PostErrMsg(ENCDERR_Z2);
        }

        if(TAMAGAbsErr_Reg.bit.AbsCntError == 1)     //计数增量异常
        {
            PostErrMsg(ENCDERR_Z4);
            FPGAFltToleranceSt = 1;
        }

        if(TAMAGAbsErr_Reg.bit.AbsOverHeat == 1)     
        {
            PostErrMsg(ENCODEROT);
        }
    
        if(0 != STR_MTR_Gvar.AbsRom.AbsPosDetection)
        {
            if(TAMAGAbsErr_Reg.bit.BatteryAlarm == 1)
            {
                PostErrMsg(ENCDBATWARN);  //0xe730
            }
    
            if(TAMAGAbsErr_Reg.bit.BatteryError == 1)
            {
                PostErrMsg(ENCDBATERR);  //0x6731
            }
    
            //可以根据实际情况处理，刀架非标屏蔽该故障
            if((TAMAGAbsErr_Reg.bit.MultiturnError == 1)  && (1 == STR_MTR_Gvar.AbsRom.AbsPosDetection))
            {
                PostErrMsg(ENCDMULTIERR);  //0x6733
            }
    
            if( (1 == STR_MTR_Gvar.AbsRom.AbsPosDetection) 
             && ( ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32767) || ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 32768) ) )
            {
                if(0 == FunCodeUnion.code.EncMultOvDisable) PostErrMsg(ENCDMULTIOV);    //0x6735
            } 
        }
    }          
#endif

#if HDH_ENCODER_SW
    else if(0x11 == FunCodeUnion.code.MT_EncoderSel)   // 绝对式编码器
    {
        HDHAbsErr_Reg.all      = AbsErr_Reg.all;       //故障状态     

        if((STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 0) 
        && (0 != (HDHAbsErr_Reg.all & 0x1F)) )
        {
            PostErrMsg(ENCDERR_Z2);
        }

        if(HDHAbsErr_Reg.bit.AbsCntError == 1)     //计数增量异常
        {
            PostErrMsg(ENCDERR_Z4);
            FPGAFltToleranceSt = 1;
        }
            
        if( (1 == STR_MTR_Gvar.AbsRom.AbsPosDetection) 
         && ( ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 2047) || ((Uint16)AuxFunCodeUnion.code.DP_AbsExtData == 2048) ) )
        {
            if(0 == FunCodeUnion.code.EncMultOvDisable) PostErrMsg(ENCDMULTIOV);    //0x6735
        }
    }          
#endif

    if(0 == FPGAFltToleranceSt)
    {
        RotZCntFltTolerance();
    
        AdcFltTolerance();
    
        OvrSpdFltTolerance(); 
    }

    if((AbsErr_Reg.bit.AbsZeroError == 1) || (AbsErr_Reg.bit.AbsDivError == 1))//编码器算法异常 ER.980
    {
        PostErrMsg(ABSMOTALARM);
    }
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void  LnrEncFltTolerance(void)
{
    if(LnrEnCtrl2_Reg.bit.LnrFltEnbl == 0) return;

    if(FunCodeUnion.code.FC_FeedbackMode == 0)return;

    UNI_FPGA_TmFlt.all = *TmFlt;

	if(UNI_FPGA_TmFlt.bit.LnrEncFlt == 1)
    {
        PostErrMsg(EXENCDSCLERR);
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void  ExecTmFltTolerance(void)
{
    if(UNI_FPGA_SysErr.bit.ExecTmFlt == 0) return;
     
    //系统采样或运算超时 ，用于表示FOC计算启动延时设置错误, Er208    
    if((AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)
        && ((FunCodeUnion.code.ER_AngIntSel == 2)||(FunCodeUnion.code.ER_AngIntSel == 3)))     //电压注入时不报警，清除104
    {
        ClcFPGAFltReg();  
    }
    else
    {
        UNI_FPGA_TmFlt.all = *TmFlt;
        FPGAFltToleranceSt = 1; 

        if(UNI_FPGA_TmFlt.bit.McuTmFlt == 1)
        {
            PostErrMsg(EXECTMFLT0);
        }
        else if(UNI_FPGA_TmFlt.bit.AbsTmFlt == 1)
        {
            //屏蔽该故障
            FPGAFltToleranceSt = 0;             
        }
        else if(UNI_FPGA_TmFlt.bit.FocTmFlt == 1)
        {
            PostErrMsg(EXECTMFLT2);
        }
        else if(UNI_FPGA_TmFlt.bit.AdcTmFlt == 1)
        {
            PostErrMsg(EXECTMFLT3);
        }
        else 
        {
            //兼容老版本FPGA程序，其它平台需要慎重处理
            PostErrMsg(EXECTMFLT4);
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
Static_Inline void AdcFltTolerance(void)
{
    static Uint8 AdcFltCnt = 0;

    if( (1 == STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn)
     && (1 == UNI_FPGA_SysErr.bit.AdcFlt) )     //A/D转换器故障, Er835
    {
        //必须连续3次标志位置1 才报错
        if(AdcFltCnt > 2) 
        {
            PostErrMsg(FPGA_AD_SAMPLE_ERR);
        }
        else 
        {
            AdcFltCnt ++;
            ClcFPGAFltReg();
        }
    }
    else
    {
        AdcFltCnt = 0;
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void OvrSpdFltTolerance(void)
{
    static Uint16 OvrSpdFltCnt = 0;

    if(0 == UNI_FPGA_SysErr.bit.OvrSpdFlt) 
    {
        OvrSpdFltCnt = 0;
        return;
    }
     
    //超速溢出  
    //上电后如果有3次标志位置1，再报错。 不要求连续报错 
    if(OvrSpdFltCnt > 2)
    {
        PostErrMsg(FPGAOVERSPD);
    }
    else
    {            
        OvrSpdFltCnt ++;

        //清除故障
        ClcFPGAFltReg();
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Static_Inline void McuLostFltTolerance(void)
{
    if(UNI_FPGA_SysErr.bit.McuLost == 1)    //MCU访问FPGA间隔超时 0x0100
    {
        if((AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)
            && ((FunCodeUnion.code.ER_AngIntSel == 2)||(FunCodeUnion.code.ER_AngIntSel == 3)))     //电压注入时不报警，清除104
        {
            //清除故障
            ClcFPGAFltReg();
        }
        else
        {
            PostErrMsg(MCULOST);

            FPGAFltToleranceSt = 1;
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
Static_Inline void RotZCntFltTolerance(void)
{
    static Uint16 RotZCntFltCnt = 0;

    if(0 == UNI_FPGA_SysErr.bit.RotZCntFlt)  return;  
    //编码器Z相计数错误

    //上电后如果有两次标志位置1，再报错。 不要求连续报错       
    if(RotZCntFltCnt > 1)
    {
        PostErrMsg(ENCDERR_AB);
    }
    else
    {            
        RotZCntFltCnt ++;
        //清除故障
        ClcFPGAFltReg();
    }
}

/*******************************************************************************
  函数名:  void ClrHC2ndAbsEncWarn(void)
  输入:   无
  输出:   
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void ClrHC2ndAbsEncWarn(void)
{
    if(AbsErr_Reg.HC2ndbit.BatteryAlarm == 0)
    {
        WarnAutoClr(ENCDBATWARN);  //0xe730
    }

    if(AbsErr_Reg.HC2ndbit.AbsOverHeat == 0)
    {
        WarnAutoClr(ENCODEROT);  //0xE760    编码器过热
    }
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
void ClrNKAbsEncWarn(void)
{
#if NOKIN_ENC_SW
    if(NKAbsErr_Reg.bit.AbsOverRun == 0)     //编码器超限（TX端）
    {
        WarnAutoClr(NKENCOVERLIM);
    }

    if(0 != STR_MTR_Gvar.AbsRom.AbsPosDetection)
    {
        if(NKAbsErr_Reg.bit.BatteryAlarm == 0)
        {
            WarnAutoClr(ENCDBATWARN);  
        }
    } 
#endif
}

/*******************************************************************************
  函数名:   
  输入:   无
  输出:   
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void ClrTAMAGAbsEncWarn(void)
{
#if TAMAGAWA_ENC_SW
    if(TAMAGAbsErr_Reg.bit.BatteryAlarm == 0)
    {
        WarnAutoClr(ENCDBATWARN);  //0xe730
    }

    if(TAMAGAbsErr_Reg.bit.AbsOverHeat == 0)
    {
        WarnAutoClr(ENCODEROT);  //0xE760    编码器过热
    }
#endif
}


/********************************* END OF FILE *********************************/
