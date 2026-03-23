/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_ModeSelect.c
 创建人：王军干
 修改人：李浩                修改日期：11.12.09 
 描述： 
     1.
     2.

 修改记录：  
     1.xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_ServoMonitor.h"
#include "FUNC_ServoError.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_FunCode.h"
#include "FUNC_ManageFunCode.h"
#include "FUNC_ModeSelect.h"
#include "FUNC_FunCode.h"
#include "FUNC_AuxFunCode.h"
#include "CANopen_OD.h"
#include "CANopen_Pub.h"
#include "CANopen_PP.h"
#include "FUNC_PosCtrl.h"          
#include "CANopen_Home.h"
#include "ECT_CSP.h"
#include "ECT_ESMDisplay.h"
/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define  DISBALE          (0)
// 后台模式复位
#define     GUICTRL_RESET           (0)
// 退出后台模式
#define     GUICTRL_QUIT            (40)
// 后台模式控制伺服进行机械特性分析
#define     GUICTRL_SPDFSA          (50)
// 后台模式控制伺服JOG
#define     GUICTRL_POSJOGEN        (60)
// 后台速度模式
#define     GUI_SPD                 (0)
// 后台位置模式
#define     GUI_POS                 (1)
// 后台转矩模式
#define     GUI_TOQ                 (2)


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
static STR_MODSELECT_FLAG   ModSelectFlag = {0};
static Uint8 ServoRunMode = 0;
Uint8 PV2PPSwitchFlag;
Uint8 PV2IPSwitchFlag;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//void ModSelect(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void InertiaHandle(void);     //惯量辨识
Static_Inline void TorqPiTuneHandle(void);  //电流环PI参数自调谐
Static_Inline void SpdJOGHandle(void);      //速度JOG
Static_Inline void GUIWorkHandle(void);     // 实现后台模式控制伺服
Static_Inline void ResZeroIndexHandle(void);
#if CAN_ENABLE_SWITCH
void CanopenModeSelect(void);//CANopen
#endif

//#if ECT_ENABLE_SWITCH
//void ECTModeSelect(void);
//#endif
/*******************************************************************************
  函数名: void ModSelect(void)
  输入:  模式选择功能码，M-Sel切换DI输入状态
  输出:  运行模式  RUNMOD
  子函数:    
  描述: 更新功能码的伺服运行模式
********************************************************************************/ 
void ModSelect(void)
{
    static Uint8 FirOnPower = 0;


    //绝对式编码器电机初始角辨识
    if(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.AbsEncAngInitEn == 1)  //初始角度辨识内部使能
    {
        STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 1;         //屏蔽速度环
        ModSelectFlag.ResetABSThetaEn = 1;
    }
    else
    {     //内部不使能的情况下需要判定是不是绝对式编码器和角度辨识同时满足
        if( (AuxFunCodeUnion.code.OEM_ResetABSTheta == 1) 
         && ( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x10) 
		   || ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x20) ))   
        {
            ModSelectFlag.ResetABSThetaEn = 0;

            if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)    //只在使能确实关闭后才开启速度环
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 0;      //结束辨识开启速度环
            }
        }
    }

    //多摩川绝对位置编码器ROM区读写时,返回
    if(0 == ModSelectFlag.OperAbsROMEn)
    {
        if (AuxFunCodeUnion.code.MT_OperAbsROM != 0) 
        {
            if( (0x10 ==  (FunCodeUnion.code.MT_EncoderSel & 0xf0)) &&
                (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN))
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 1;
                ModSelectFlag.OperAbsROMEn = 1;
            }
            else
            {
                AuxFunCodeUnion.code.MT_OperAbsROM = 0;
            }
        }
    }
    else if(ModSelectFlag.OperAbsROMEn == 1)
    {
        if(AuxFunCodeUnion.code.MT_OperAbsROM == 0)
        {
            STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 0;
            ModSelectFlag.OperAbsROMEn = 0;
        }
        else
        {
            return;
        }
    }

    //UV相电流平衡校正
    if(STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn == 0)
    {
        //首次进入UV相电流平衡校正  条件1.上升沿  2.soff  3.非参数自调谐  4. FPGA版本大于等于6120
        if((ModSelectFlag.UVAdjustRatioLatch == 0) && (AuxFunCodeUnion.code.FA_UVAdjustRatio == 1) &&
           (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY) &&
           (STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 0) )
        {
            ModSelectFlag.UVAdjustRatioLatch = 1;
            STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn = 1;
            STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 1;
        }
        else
        {
            ModSelectFlag.UVAdjustRatioLatch = 0;
            AuxFunCodeUnion.code.FA_UVAdjustRatio = 0;
        }
    }
    else if((STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn == 1) &&
            (UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.UVAdjustResult != 0))  //校正结束
    {
        ModSelectFlag.UVAdjustRatioLatch = 0;
        AuxFunCodeUnion.code.FA_UVAdjustRatio = 0;
        STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn = 0;
        STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 0;
        FunCodeUnion.code.OEM_V2UCalCoff = (Uint16)UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.UVAdjustResult;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_V2UCalCoff));
        PostErrMsg(PCHGDWARN);
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn == 1) return;

    //惯量辨识模式
    InertiaHandle();

    //电流环PI参数自调谐
    TorqPiTuneHandle();

    //速度JOG
    SpdJOGHandle();

    //速度搜索Z信号
    ResZeroIndexHandle();
    // 后台模式
    GUIWorkHandle();

    if(STR_FUNC_Gvar.FricIdentify.FricIdenCmd == 1)    //摩擦辨识内部使能
    {
        ModSelectFlag.FricSon = 1;  
    }
    else
    {
        ModSelectFlag.FricSon = 0;
    }


    //省线式电机初始角辨识
    if(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.IncEncAngInitEn == 1)  //初始角度辨识内部使能
    {
        STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 1;         //屏蔽速度环

        STR_ServoMonitor.RunStateFlag.bit.AngIntRdy  = INVALID;      //清除辨识完成标志

        ModSelectFlag.IncAngInitSon = 1;
    }
    else
    {     //内部不使能的情况下需要判定是不是省线式编码器和角度辨识同时满足
		if(STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact == 1)		     //进入辨识后才判定什么时候结束
        { 
            if(((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0)           //省线式编码器和直线电机可运行
               || ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30))   
            {
                ModSelectFlag.IncAngInitSon = 0;

#if ECT_ENABLE_SWITCH
                if((((STR_FUNC_Gvar.DiDoOutput.Son_Flt == INVALID)&&(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon == 0))
                   &&(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)) 
                   || ((STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID)||(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon == 1)))
                {
                    STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 0;       //结束辨识开启速度环
					STR_ServoMonitor.RunStateFlag.bit.FstSonFlag     = 1;       //提示上电后已经完成过角度辨识

                    STR_ServoMonitor.RunStateFlag.bit.AngIntRdy  = VALID;      //置辨识完成标志
                }
#else
                if(((STR_FUNC_Gvar.DiDoOutput.Son_Flt == INVALID)
                   &&(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)) 
                   || (STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID))
                {
                    STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact = 0;       //结束辨识开启速度环
					STR_ServoMonitor.RunStateFlag.bit.FstSonFlag     = 1;       //提示上电后已经完成过角度辨识

                    STR_ServoMonitor.RunStateFlag.bit.AngIntRdy  = VALID;      //置辨识完成标志
                }
#endif
            }
        }
    }

    if(STR_ServoMonitor.SonDelayCnt >= 20)
    {
        STR_FUNC_Gvar.Monitor.DovarReg_AngIntRdy  = STR_ServoMonitor.RunStateFlag.bit.AngIntRdy;
    }
    else
    {
        STR_FUNC_Gvar.Monitor.DovarReg_AngIntRdy  = INVALID;      //置辨识完成标志
    }

    if(1 == (ModSelectFlag.JOGSon + ModSelectFlag.InertiaSon + ModSelectFlag.TorqPiTuneSonLatch 
              + ModSelectFlag.FricSon + ModSelectFlag.IncAngInitSon + ModSelectFlag.GUIWorkSon
              + ModSelectFlag.ResetABSThetaEn+ModSelectFlag.ResearchZInSon)) 
    {
        STR_ServoMonitor.RunStateFlag.bit.InnerServoSon = 1;
    }
    else 
    {
        STR_ServoMonitor.RunStateFlag.bit.InnerServoSon = 0;
    }

    //得到全局标志位
    STR_FUNC_Gvar.MonitorFlag.bit.InnerServoSon = STR_ServoMonitor.RunStateFlag.bit.InnerServoSon;

    //非速度JOG模式 非惯量辨识模式 非电流环参数自调整情况下
    // 非后台模式
    if((0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd) && 
       (0 == STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn) && 
       (0 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) &&
       (0 == ModSelectFlag.GUICtrlEnable)&&
       (0 == STR_FUNC_Gvar.Monitor.ResZIndexcmd))
    {
        switch(FunCodeUnion.code.BP_ModeSelet) // 运行模式功能码
        {
              case 4: //P-S
                    if(STR_FUNC_Gvar.DivarRegLw.bit.M1Sel)
                         ServoRunMode = POSMOD;
                    else
                         ServoRunMode = SPDMOD;
#if ECT_ENABLE_SWITCH
                     STR_CanSyscontrol.Mode = 0;
					 ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif                         
                    break;

              case 3: //S-T
                    if(STR_FUNC_Gvar.DivarRegLw.bit.M1Sel)
                         ServoRunMode = SPDMOD;
                    else
                         ServoRunMode = TOQMOD;
#if ECT_ENABLE_SWITCH
                     STR_CanSyscontrol.Mode = 0;
					 ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                    break;

              case 5: //P-T
                    if(STR_FUNC_Gvar.DivarRegLw.bit.M1Sel)
                         ServoRunMode = POSMOD;
                    else
                         ServoRunMode = TOQMOD;
#if ECT_ENABLE_SWITCH
                    STR_CanSyscontrol.Mode = 0;
					ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                    break;

              case 6://P-S-T
                    if(STR_FUNC_Gvar.DivarRegLw.bit.M1Sel)
                      ServoRunMode = POSMOD;
                    else if(STR_FUNC_Gvar.DivarRegLw.bit.M2Sel)
                      ServoRunMode = SPDMOD;
                    else
                      ServoRunMode = TOQMOD;
#if ECT_ENABLE_SWITCH
                    STR_CanSyscontrol.Mode = 0;
				    ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                    break;
                    //11，10--位置，01-速度，00-转矩
                    
              case 2:
                   ServoRunMode = TOQMOD;
#if ECT_ENABLE_SWITCH
                   STR_CanSyscontrol.Mode = 0;
				   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                   break;

              case 0:
                   ServoRunMode = SPDMOD;
#if ECT_ENABLE_SWITCH
                   STR_CanSyscontrol.Mode = 0;
				   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                   break;

              case 1: //P,S,T,不需要切换模式
              case 7:
                   ServoRunMode = POSMOD;
#if ECT_ENABLE_SWITCH
                   STR_CanSyscontrol.Mode = 0;
				   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 0;
#endif              
                   break;

#if CAN_ENABLE_SWITCH
              case 8:
                   CanopenModeSelect();				   
                   break;
//	by huangxin201711_22 模式切换转移到位置环，以解决模式切换时位置不准的问题
//#elif  ECT_ENABLE_SWITCH                  
//
//              case 9:
//                   ECTModeSelect();				   
//                   break;

#endif
             

              default:
                   break;
        } //end of switch(FunCodeUnion.code.BP_ModeSelet)

        //添加DI速度JOG功能
        if(STR_FUNC_Gvar.DivarRegLw.bit.JogCmdP ^ STR_FUNC_Gvar.DivarRegLw.bit.JogCmdN == 1)
        {
            if(1 == STR_FUNC_Gvar.DivarRegLw.bit.JogCmdP)
            {
                STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn = 1;
            }
            else
            {
                STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn = 2;
            }
        }
        else if((1 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn) || (2 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn))
        {
            STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn = 3;                              
        }
        else if((3 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn) && (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < 10))
        {
            STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn = 0;
        }

        if(0 != STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn)
        {
            ServoRunMode = SPDMOD;            
        }
    } //end of if(0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd) && ...

    //判断在模式切换时进行调节器变量清零
    if (FirOnPower == 0)
    {
        ModSelectFlag.RunModLatch = ServoRunMode;//上电第一次的模式值
        FirOnPower = 1;
    }
    if((ModSelectFlag.RunModLatch != ServoRunMode)&&(FirOnPower == 1))
    {
        if( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN ) //只有在运行过程中时，而且发生了切换动作，才进行补偿
        {
            if((ModSelectFlag.RunModLatch == SPDMOD)&&( ServoRunMode == POSMOD)) ////Spd -> Pos 由速度环向位置环切换时
            {
                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;

                STR_FUNC_Gvar.MonitorFlag.bit.SpdSwitchPosOffset = 1;
            }
            else if((ModSelectFlag.RunModLatch == TOQMOD)&&( ServoRunMode == SPDMOD)) //Toq -> Spd
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;    //直接清零不补偿
                STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod = 1;
            }
            else if ((ModSelectFlag.RunModLatch == TOQMOD)&&( ServoRunMode == POSMOD))  //Toq -> Pos 由转矩环向位置环切换时
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;    //先清零再补偿
                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;

                STR_FUNC_Gvar.MonitorFlag.bit.ToqSwitchPosOffset = 1;
                STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod = 1;
            }
            else if ((ModSelectFlag.RunModLatch == POSMOD)&&( ServoRunMode == SPDMOD))  //Pos -> Spd
            {
                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;
            }
            else if ((ModSelectFlag.RunModLatch == POSMOD)&&( ServoRunMode == TOQMOD))  //Pos -> Toq
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;    //直接清零不补偿
                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;
            }
            else if ((ModSelectFlag.RunModLatch == SPDMOD)&&( ServoRunMode == TOQMOD))  //SPD -> TOQ
            {
                STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;    //直接清零不补偿
            }
        }
        else  //在非模式切换时，不进行补偿，直接将相应调节器清零
        {
            STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
            STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;
        }
    }

    STR_FUNC_Gvar.MonitorFlag.bit.RunMod = ServoRunMode;

    ModSelectFlag.RunModLatch = ServoRunMode;//保存上一次的模式值
}
/*******************************************************************************
  函数名: void InertiaHandle(void)
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 惯量辨识
********************************************************************************/
Static_Inline void InertiaHandle(void)
{
    //惯量辨识
    static Uint16 Inertia_HeartBeatCnt = 0;
	Uint16 HeartBeartMax = 0;
	HeartBeartMax = 2*STR_FUNC_Gvar.System.PosFreq;

    if(0 != STR_FUNC_Gvar.Monitor.OffLnInrtCmd)
    {
        if(0 != ModSelectFlag.InertiaSon)
        {
            //惯量辨识心跳处理
            if(AuxFunCodeUnion.code.FA_OffLnInrtMod == 0x41) //退出惯量辨识前SOFF
            {
                ModSelectFlag.InertiaSon = 0;
                Inertia_HeartBeatCnt = 0;
            }
            else if( (AuxFunCodeUnion.code.FA_OffLnInrtMod == 0x01) ||
                     (AuxFunCodeUnion.code.FA_OffLnInrtMod == 0x11) || 
                     (AuxFunCodeUnion.code.FA_OffLnInrtMod == 0x21) )
            {
                STR_FUNC_Gvar.Monitor.OffLnInrtCmd = AuxFunCodeUnion.code.FA_OffLnInrtMod;
                Inertia_HeartBeatCnt = 0;
            }
            else
            { 
                Inertia_HeartBeatCnt ++;
               // if(Inertia_HeartBeatCnt > 2000)  //如果两秒内H0D02没有赋值,退出惯量辨识前SOFF
				if(Inertia_HeartBeatCnt > HeartBeartMax)  //如果两秒内H0D02没有赋值,退出惯量辨识前SOFF	  换到位置环，计数器增加速度为位置环频率
                {
                    ModSelectFlag.InertiaSon = 0;
                    Inertia_HeartBeatCnt = 0;
                }
            }
        }
        else
        {
            Inertia_HeartBeatCnt ++;
            //延时10ms 退出惯量辨识模式
            if(Inertia_HeartBeatCnt >= 10)
            {
                Inertia_HeartBeatCnt = 0;
                STR_FUNC_Gvar.Monitor.OffLnInrtCmd = 0;
                STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn = 0;
            }
        }
    }
    else
    {
        //首次进入惯量辨识模式  条件1：非参数自调谐 2：非JOG 3：soff 4:非后台模块
        if((AuxFunCodeUnion.code.FA_OffLnInrtMod == 1) &&
           (STR_FUNC_Gvar.Monitor.SpdJOGCmd == 0) &&
           (STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 0) &&
           (ModSelectFlag.GUICtrlEnable == 0) &&
           (STR_FUNC_Gvar.Monitor.ResZIndexcmd==0)&&
           (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
        {
            //首次进入惯量辨识
            STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn = 1;
            STR_FUNC_Gvar.Monitor.OffLnInrtCmd = 1;
            ModSelectFlag.InertiaSon = 1;
            ServoRunMode = SPDMOD;
        }
        else
        {
            STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn = 0;
            STR_FUNC_Gvar.Monitor.OffLnInrtCmd = 0;
            ModSelectFlag.InertiaSon = 0;
        }

        Inertia_HeartBeatCnt = 0;
    }

    AuxFunCodeUnion.code.FA_OffLnInrtMod = 0;
}


/*******************************************************************************
  函数名: void TorqPiTuneHandle(void)
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 电流环PI参数自调谐
********************************************************************************/
Static_Inline void TorqPiTuneHandle(void)
{
    if(STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1)
    {
        ModSelectFlag.TorqPiTuneSonLatch = STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon;
        if(AuxFunCodeUnion.code.FA_TorqPiTune == 0) STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn = 0;
    }
    else
    {
        //首次进入参数自调谐  条件1：非惯量辨识模式 2：非JOG 3：soff  4:非后台模块
        if((AuxFunCodeUnion.code.FA_TorqPiTune != 0) &&
           (STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn == 0) &&
           (STR_FUNC_Gvar.Monitor.SpdJOGCmd == 0) && 
           (STR_FUNC_Gvar.Monitor.ResZIndexcmd==0)&&
           (ModSelectFlag.GUICtrlEnable == 0) &&
           (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
        {         
            STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn = 1;
            ServoRunMode = TOQMOD;    //切换到转矩模式
        }
        else
        {
            AuxFunCodeUnion.code.FA_TorqPiTune = 0;
        }

        ModSelectFlag.TorqPiTuneSonLatch = 0;
    }
}
/*******************************************************************************
  函数名: void SpdJOGHandle(void)
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 速度JOG
********************************************************************************/
Static_Inline void SpdJOGHandle(void)
{
    //JOG
    static Uint16 JOG_HeartBeatCnt = 0;
    static Uint16 H0604_Latch = 100;
    static Uint16 H0605_Latch = 200;
    static Uint16 H0606_Latch = 200;
	Uint16 HeartBeartMax = 0;
	HeartBeartMax = 2*STR_FUNC_Gvar.System.PosFreq;

    //锁存H0604  H0605 H0606
    if(0x80 == AuxFunCodeUnion.code.FA_Jog)
    {
        H0604_Latch = FunCodeUnion.code.SL_JOGSpdCommand;
        H0605_Latch = FunCodeUnion.code.SL_SpdCMDRiseTime;
        H0606_Latch = FunCodeUnion.code.SL_SpdCMDDownTime;
        if(FunCodeUnion.code.SL_SpdCMDRiseTime < 20) FunCodeUnion.code.SL_SpdCMDRiseTime = 20;
        if(FunCodeUnion.code.SL_SpdCMDDownTime < 20) FunCodeUnion.code.SL_SpdCMDDownTime = 20;
    }

    //还原H0604  H0605 H0606
    if(0x81 == AuxFunCodeUnion.code.FA_Jog)
    {
        FunCodeUnion.code.SL_JOGSpdCommand = H0604_Latch;
        FunCodeUnion.code.SL_SpdCMDRiseTime = H0605_Latch;
        FunCodeUnion.code.SL_SpdCMDDownTime = H0606_Latch;
    }

    //速度JOG模式判断
    if(0 != STR_FUNC_Gvar.Monitor.SpdJOGCmd)
    {
        if(ModSelectFlag.JOGSon == 1)
        {
            if(AuxFunCodeUnion.code.FA_Jog == 0x41) //退出速度JOG前SOFF
            {
                ModSelectFlag.JOGSon = 0;
                JOG_HeartBeatCnt = 0;
            }
            else if( (AuxFunCodeUnion.code.FA_Jog == 0x01) ||
                     (AuxFunCodeUnion.code.FA_Jog == 0x11) || 
                     (AuxFunCodeUnion.code.FA_Jog == 0x21) )
            {
                STR_FUNC_Gvar.Monitor.SpdJOGCmd = AuxFunCodeUnion.code.FA_Jog;
                JOG_HeartBeatCnt = 0;
            }
            else
            {
                JOG_HeartBeatCnt ++;
                
			//	if(JOG_HeartBeatCnt > 2000)  //如果两秒内H0D11没有赋值,退出速度JOG前SOFF
                if(JOG_HeartBeatCnt > HeartBeartMax )  //如果两秒内H0D11没有赋值,退出速度JOG前SOFF	   模式切换调到了位置环，计数器增加速度为位置环频率
				{
                    ModSelectFlag.JOGSon = 0;
                    JOG_HeartBeatCnt = 0;
                    //还原H0604  H0605 H0606
                    FunCodeUnion.code.SL_JOGSpdCommand = H0604_Latch;
                    FunCodeUnion.code.SL_SpdCMDRiseTime = H0605_Latch;
                    FunCodeUnion.code.SL_SpdCMDDownTime = H0606_Latch;
                }
            }
        }
        else
        {
            JOG_HeartBeatCnt ++;
            //延时10ms 退出JOG
            if(JOG_HeartBeatCnt >= 10)
            {
                STR_FUNC_Gvar.Monitor.SpdJOGCmd = 0;
                JOG_HeartBeatCnt = 0;
                //还原H0604  H0605 H0606
                FunCodeUnion.code.SL_JOGSpdCommand = H0604_Latch;
                FunCodeUnion.code.SL_SpdCMDRiseTime = H0605_Latch;
                FunCodeUnion.code.SL_SpdCMDDownTime = H0606_Latch;
            }
        }
    }
    else
    {
        //首次进入速度JOG模式 条件1：非惯量辨识模式 2：非参数自调谐模式 3：soff  4:非后台模块
        if((0x01 == AuxFunCodeUnion.code.FA_Jog) &&
           (0 == STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn) &&
           (0 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) && 
           (0 == ModSelectFlag.GUICtrlEnable) &&
           (RDY == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus))
        {
            ModSelectFlag.JOGSon = 1;
            STR_FUNC_Gvar.Monitor.SpdJOGCmd = 1;
            ServoRunMode = SPDMOD;
        }

        JOG_HeartBeatCnt = 0;
    }

    AuxFunCodeUnion.code.FA_Jog = 0;
}
/*******************************************************************************
  函数名: void GUIWorkHandle() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 后台模式控制伺服
********************************************************************************/
Static_Inline void GUIWorkHandle()
{
    static Uint16 u16HeartBeatCnt = 0;   // 后台通信正常心跳计数器
    Uint16 ServoRunStatusTemp = 0;
	Uint16 HeartBeartMax = 0;
	HeartBeartMax = 6*STR_FUNC_Gvar.System.PosFreq;

    // 关联伺服运行状态标志位与后台相关功能码
    AuxFunCodeUnion.code.ServoRunState = STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus;

    ServoRunStatusTemp = STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus;
    ServoRunStatusTemp = ServoRunStatusTemp << 14;
    AuxFunCodeUnion.code.CurveWarne = AuxFunCodeUnion.code.CurveWarne & 0x3FFF;
    AuxFunCodeUnion.code.CurveWarne = AuxFunCodeUnion.code.CurveWarne | ServoRunStatusTemp;
    
    // 后台模式时需要读取的速度环调度频率
    AuxFunCodeUnion.code.FS_SpdLoopFreq = STR_FUNC_Gvar.System.SpdFreq;

    //首次进入后台模式 条件1：非惯量辨识模式 2：非参数自调谐模式 3：非JOG 4：soff  
     if((ModSelectFlag.GUICtrlEnable == 0) &&
       (STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn == 0) &&
       (STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 0) && 
       (STR_FUNC_Gvar.Monitor.SpdJOGCmd == 0) &&
       (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
    {
        switch (AuxFunCodeUnion.code.GUIWorkMode)
        {
        case GUICTRL_SPDFSA:
            if(GUI_SPD == AuxFunCodeUnion.code.GUIModeSelet)
            {
                ServoRunMode = SPDMOD;
                ModSelectFlag.GUICtrlEnable = 1;
                STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 1;
                STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            }
            else
            {
                //出错退出
                STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
                STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            }
            break;

        case GUICTRL_POSJOGEN:
            if(GUI_POS == AuxFunCodeUnion.code.GUIModeSelet)
            {
                ServoRunMode = POSMOD;
                ModSelectFlag.GUICtrlEnable = 1;
                STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
                STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 1;
            }
            else
            {
                //出错退出
                STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
                STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            }
            break;

        default:
                STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
                STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            break;
        }

        // 关联伺服启动标志与后台相关功能码
        ModSelectFlag.GUIWorkSon = 0;
        // 后台通信正常心跳计数器清0
        u16HeartBeatCnt = 0 ;
    }
    else if(ModSelectFlag.GUICtrlEnable == 1)
    {
        switch (AuxFunCodeUnion.code.GUIWorkMode)
        {
        case GUICTRL_SPDFSA:
            if(STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn == 1)
            {
                u16HeartBeatCnt = 0;
            }
            break;
        case GUICTRL_POSJOGEN:
            if(STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork == 1)
            {
                u16HeartBeatCnt = 0;
            }
            break;
        case GUICTRL_QUIT:
            // 控制伺服时的相关标志位, 关闭内部伺服ON
            ModSelectFlag.GUICtrlEnable = 0;
		    AuxFunCodeUnion.code.CurveSel      = 0 ;
		    AuxFunCodeUnion.code.CurveServoON  = 0 ;
		    AuxFunCodeUnion.code.JogDir        = 0 ;
            STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
        default:
            break;
        }

        // 心跳计数器累加及判断
        u16HeartBeatCnt++;
       // if (u16HeartBeatCnt >= 6000)		  //模式切换调到位置环，计数器速度增加到4倍了
		if (u16HeartBeatCnt >= HeartBeartMax)
        {
            // 心跳计数器计数已满, 断开后台模式, 复位后台模式
            //u16HeartBeatCnt = 6000;	  //模式切换调到位置环，计数器速度增加到4倍了
			u16HeartBeatCnt = HeartBeartMax;
            ModSelectFlag.GUIWorkSon = 0;

            // 控制伺服时的相关标志位, 关闭内部伺服ON
            ModSelectFlag.GUICtrlEnable = 0;
            STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
            STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
		    AuxFunCodeUnion.code.CurveSel      = 0 ;
		    AuxFunCodeUnion.code.CurveServoON  = 0 ;
		    AuxFunCodeUnion.code.JogDir        = 0 ;
        }
        else
        {
            // 关联伺服启动标志与后台相关功能码
            ModSelectFlag.GUIWorkSon = AuxFunCodeUnion.code.CurveServoON;
        }
    }
    else
    {
        ModSelectFlag.GUICtrlEnable = 0;
		AuxFunCodeUnion.code.CurveSel      = 0 ;
		AuxFunCodeUnion.code.CurveServoON  = 0 ;
		AuxFunCodeUnion.code.JogDir        = 0 ;
        STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn = 0;
        STR_FUNC_Gvar.MonitorFlag.bit.PosJogWork = 0;
        ModSelectFlag.GUIWorkSon = 0;
    }

    // 后台模式控制伺服时, 功能标志指令只有效一次, 用完就复位
    // 需要上位机不断地写入功能标志指令, 才能使所选的功能持续
    // 这是便于实现上位机控制伺服时监测通信是否断开
	if(AuxFunCodeUnion.code.FA_AutoTune == 0)		     //只有在没有手动启动运动Jog功能时才对模式复位
	{
        AuxFunCodeUnion.code.GUIWorkMode = GUICTRL_RESET;
	}

    if (AuxFunCodeUnion.code.FS_Mode == 2)
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.OpenFSAEn = STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn;
    }
    else
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.OpenFSAEn = 0;
    }
}
/*******************************************************************************
  函数名: void ResZeroIndexHandle(void)
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 寻找Z信号
********************************************************************************/
Static_Inline void ResZeroIndexHandle(void)
{
    static Uint16 H0604_Latch = 100;
    static Uint16 H0605_Latch = 200;
    static Uint16 H0606_Latch = 200;

    //锁存H0604  H0605 H0606
    if(0x10 == AuxFunCodeUnion.code.ResZeroIndexEn)
    {
        H0605_Latch = FunCodeUnion.code.SL_SpdCMDRiseTime;
        H0606_Latch = FunCodeUnion.code.SL_SpdCMDDownTime;
        H0604_Latch = FunCodeUnion.code.SL_JOGSpdCommand ;
        
        AuxFunCodeUnion.code.ResZeroIndexEn = 0x11;
    }
    else if(0x12 == AuxFunCodeUnion.code.ResZeroIndexEn)
    {
        
        if(FunCodeUnion.code.SL_JOGSpdCommand<30)
        {
            FunCodeUnion.code.SL_JOGSpdCommand = 30;   
        }
        else if(FunCodeUnion.code.SL_JOGSpdCommand>200)
        {
            FunCodeUnion.code.SL_JOGSpdCommand = 200;
        }

        FunCodeUnion.code.SL_SpdCMDRiseTime = 50;
        FunCodeUnion.code.SL_SpdCMDDownTime = 50;
        
        if((0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd)&&
          (0 == STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn) &&
          (0 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) && 
          (0 == ModSelectFlag.GUICtrlEnable) &&
          (RDY == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus))
        {
            ServoRunMode = SPDMOD;
            ModSelectFlag.ResearchZInSon = 1;
            STR_FUNC_Gvar.Monitor.ResZIndexcmd = 1;
        }

        
    }
    else if(0x20 == AuxFunCodeUnion.code.ResZeroIndexEn)
    {
        FunCodeUnion.code.SL_JOGSpdCommand  = H0604_Latch;
        FunCodeUnion.code.SL_SpdCMDRiseTime = H0605_Latch;
        FunCodeUnion.code.SL_SpdCMDDownTime = H0606_Latch;
        ModSelectFlag.ResearchZInSon = 0;
        STR_FUNC_Gvar.Monitor.ResZIndexcmd=0;
        AuxFunCodeUnion.code.ResZeroIndexEn =0;
    }

}

/*******************************************************************************
  函数名: void CanopenModeSelect(void) 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: Canopen模式选择
********************************************************************************/
#if CAN_ENABLE_SWITCH
void CanopenModeSelect(void)
{
    Uint8 temp111 = 0;
    Uint8 temp222 = 0;
    static Uint8 ServoRunModeLatch = 0;

    temp111 = ObjectDictionaryStandard.DeviceControl.ModesOfOperation;
    ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 1;
    switch(temp111)
    {
        case 1:
            temp222 = CANOPENPROPOSMOD; //PP模式
			ServoRunMode = POSMOD;
            break;

        case 3:
            temp222 = CANOPENPROVELMOD;//PV
			ServoRunMode = SPDMOD;
            break;

        case 4:
            temp222 = CANOPENPROTOQMOD;//PT
			ServoRunMode = TOQMOD;
            break;

        case 6:
            temp222 = CANOPENHOMMOD;  //回零模式
			ServoRunMode = POSMOD;
            break;
            
        case 7:
            temp222 = CANOPENINTPMD;  //插补模式
			ServoRunMode = POSMOD;
            break;
	    

        default:
            //用PDO写了不支持的模式  6061不变
            STR_CanSyscontrol.Mode = ServoRunModeLatch;
            break;
    }
     
    if(temp222 != ServoRunModeLatch)
    {
        //参考施耐德，禁止run状态下从其他模式切入插补模式和回零模式
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
        {
            if(ServoRunModeLatch == CANOPENPROPOSMOD)
            {
                CanopenPPReset();//复位插补防止切换回PP时，执行未发送的位置指令
				CanopenPPPosBuffReset();
                
                switch(temp222)
                {
                    case CANOPENPROVELMOD:
                        ServoRunMode = SPDMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    case CANOPENPROTOQMOD:
            			ServoRunMode = TOQMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    case CANOPENHOMMOD:
                        CanopenHomingModeFuncVarStop();
                        break;

                    case CANOPENINTPMD:
                        InterpltInnerParaUpdata(STR_FUNC_Gvar.System.PosFreq);
                        break;

                    default:
                        break;
                }
            }
            else if(ServoRunModeLatch == CANOPENPROVELMOD)
            {
                //首先执行斜坡停机，停机完成后，再从PV模式切换到PP模式
                if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
                {
                    STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
                    temp222 = CANOPENPROVELMOD;//保持为速度模式
                    ServoRunMode = SPDMOD;
                }
                else
                {
                    switch(temp222)
                    {
                        case CANOPENPROPOSMOD:
                            PV2PPSwitchFlag = 1;
                            ServoRunMode = POSMOD;
                            break;

                        case CANOPENPROTOQMOD:
                			ServoRunMode = TOQMOD;
                            break;

                        case CANOPENHOMMOD:
                            //防止错误指令导致回零立刻启动
                            CanopenHomingModeFuncVarStop();
                            ServoRunMode = POSMOD;
                            break;

                        case CANOPENINTPMD:
                            //防止错误指令导致插补立刻启动
                            PV2IPSwitchFlag = 1;
                            InterpltInnerParaUpdata(STR_FUNC_Gvar.System.PosFreq);
                            ServoRunMode = POSMOD;
                            break;

                        default:
                            break;
                    }
                    STR_CanSyscontrol.ModeSwitchFlag = 0;
                }
            }
            else if(ServoRunModeLatch == CANOPENPROTOQMOD)
            {
                //首先执行斜坡停机，停机完成后，再从PT模式切换到其他模式
                if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
                {
                    STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
                    temp222 = CANOPENPROTOQMOD;//保持为转矩模式
                    ServoRunMode = TOQMOD;
                }
                else
                {
                    switch(temp222)
                    {
                        case CANOPENPROPOSMOD:
                            PV2PPSwitchFlag = 1;
                            ServoRunMode = POSMOD;
                            break;

                        case CANOPENPROVELMOD:
                            ServoRunMode = SPDMOD;
                            break;

                        case CANOPENHOMMOD:
                            //防止错误指令导致回零立刻启动
                            CanopenHomingModeFuncVarStop();
                            ServoRunMode = POSMOD;
                            break;

                        case CANOPENINTPMD:
                            //防止错误指令导致插补立刻启动
                            PV2IPSwitchFlag = 1;
                            InterpltInnerParaUpdata(STR_FUNC_Gvar.System.PosFreq);
                            ServoRunMode = POSMOD;
                            break;

                        default:
                            break;
                    }
                    STR_CanSyscontrol.ModeSwitchFlag = 0;
                }
            }
            else if(ServoRunModeLatch == CANOPENHOMMOD)
            {
                //允许在原点回零完成\未启动\发生超时故障后切入其他模式
                if(STR_InnerGvarPosCtrl.MutexBit.bit.CanopenHomeWork == 0)
                {
                    WarnAutoClr(ORIGINOVERTIME);
                    switch(temp222)
                    {
                        case CANOPENPROPOSMOD:
                            CanopenHomeReset();
                            ServoRunMode = POSMOD;
                            break;

                        case CANOPENPROVELMOD:
                            ServoRunMode = SPDMOD;
                            STR_PosCtrlVar.PosCmdReachLimit= 0;
                            break;

                        case CANOPENPROTOQMOD:
                			ServoRunMode = TOQMOD;
                            STR_PosCtrlVar.PosCmdReachLimit= 0;
                            break;
                            
                        case CANOPENINTPMD:
                            ServoRunMode = POSMOD;
                            //ClrPosReg();
                            InterpltInnerParaUpdata(STR_FUNC_Gvar.System.PosFreq);
                            break;

                        default:
                            break;
                    }
                }
                else
                {
                    temp222 = CANOPENHOMMOD;
                }
            }
            else if(ServoRunModeLatch == CANOPENINTPMD)
            {
                //允许在原点回零完成\未启动\发生超时故障后切入其他模式
                IPClear();

                switch(temp222)
                {
                    case CANOPENPROPOSMOD:
                        STR_PosCtrlVar.New_SetPoint = 0;
                        ServoRunMode = POSMOD;
                        break;

                    case CANOPENPROVELMOD:
                        ServoRunMode = SPDMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    case CANOPENPROTOQMOD:
            			ServoRunMode = TOQMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;
                        

                    case CANOPENHOMMOD:
                        ServoRunMode = POSMOD;
                        CanopenHomingModeFuncVarStop();
                        break;

                    default:
                        break;
                }
            }

        }
        else//停机或故障状态下允许模式切换
        {
        }
    }
    STR_CanSyscontrol.Mode = temp222;    
    ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay = STR_CanSyscontrol.Mode;
    ServoRunModeLatch = STR_CanSyscontrol.Mode;
}
#endif
/*******************************************************************************
  函数名: void ECTModeSelect(void) 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: Canopen模式选择
********************************************************************************/
void ECTModeSelect(void)
{
    Uint8 temp111 = 0;
    Uint8 temp222 = 0;
    static Uint8 ServoRunModeLatch = 0;

    if(EcatSync.FpgaSyncModeConfigDone)
	{
 		temp111 = STR_CanSyscontrol.preMode;  // by huangxin201711_25 0x6060在sync中更新到preMode，以保证模式切换发生在sync之后的第一个位置环
	}
	else
	{
		temp111 = ObjectDictionaryStandard.DeviceControl.ModesOfOperation;
	}
    ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Remote = 1;
    switch(temp111)
    {
        case 1:
            temp222 = ECTPOSMOD; //PP模式
			ServoRunMode = POSMOD;
            break;

        case 3:
            temp222 = ECTSPDMOD;//PV
			ServoRunMode = SPDMOD;
            break;

        case 4:
            temp222 = ECTTOQMOD;//PT
			ServoRunMode = TOQMOD;
            break;
            
        case 6:
            temp222 = ECTHOMMOD;  //回零模式
			ServoRunMode = POSMOD;
            break;
            
        case 8:
            temp222 = ECTCSPMOD;  //周期同步位置模式
			ServoRunMode = POSMOD;
            break;
	    
        case 9:
            temp222 = ECTCSVMOD;  //周期同步速度模式
			ServoRunMode = SPDMOD;
            break;

        case 10:
            temp222 = ECTCSTMOD;  //周期同步转矩模式
			ServoRunMode = TOQMOD;
            break;
            
        default:
            //用PDO写了不支持的模式  6061不变
            STR_CanSyscontrol.Mode = ServoRunModeLatch;
            break;
    }
     
    if(temp222 != ServoRunModeLatch)
    {
        //参考施耐德，禁止run状态下从其他模式切入插补模式和回零模式
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
        {
            if(ServoRunModeLatch == ECTPOSMOD)//1轮廓位置模式
            {
                CanopenPPReset();//复位插补防止切换回PP时，执行未发送的位置指令
				CanopenPPPosBuffReset();
                
                switch(temp222)
                {
                    case ECTSPDMOD://3pv
                    case ECTCSVMOD://9csv
                        ServoRunMode = SPDMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    case ECTHOMMOD://6hm
                        CanopenHomingModeFuncVarStop();
                        break;
                        
                    case ECTCSPMOD://8csp
                        PV2IPSwitchFlag = 1;
                        break;
                        

                    case ECTTOQMOD://4tq
                    case ECTCSTMOD://10cst
                        ServoRunMode = TOQMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    default:
                        break;
                }
            }
            else if(ServoRunModeLatch == ECTSPDMOD)//3pv
            {
                if(temp222 != ECTCSVMOD)//8CSV模式立即切换
                {
                    //首先执行斜坡停机，停机完成后，再从PV模式切换
                    if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
                    {
                        STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
                        temp222 = ECTSPDMOD;//保持为速度模式
                        ServoRunMode = SPDMOD;
                    }
                    else
                    {
                        switch(temp222)
                        {
                            case ECTPOSMOD://1pp
                                PV2PPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTHOMMOD://6hm
                                //防止错误指令导致回零立刻启动
                                CanopenHomingModeFuncVarStop();
                                ServoRunMode = POSMOD;
                                break;

                            case ECTCSPMOD://7csp
                                PV2IPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTTOQMOD://4tq
                            case ECTCSTMOD://10cst
                                ServoRunMode = TOQMOD;
                                break;

                            default:
                                break;
                        }
                        STR_CanSyscontrol.ModeSwitchFlag = 0;
                    }
                }
            }
            else if(ServoRunModeLatch == ECTTOQMOD)//4tq
            {
                if(temp222 != ECTCSTMOD)//8CSV模式立即切换
                {
                    //首先执行斜坡停机，停机完成后，再从PV模式切换
//                    if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
//                    {
//                        STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
//                        temp222 = ECTTOQMOD;//保持为速度模式
//                        ServoRunMode = TOQMOD;
//                    }
//                    else
                    {
                        switch(temp222)
                        {
                            case ECTPOSMOD://1pp
                                PV2PPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTHOMMOD://6hm
                                CanopenHomingModeFuncVarStop();
                                ServoRunMode = POSMOD;
                                break;

                            case ECTCSPMOD://7csp
                                PV2IPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTSPDMOD://3pv
                            case ECTCSVMOD://9csv
                                ServoRunMode = SPDMOD;
                                break;

                            default:
                                break;
                        }
                        STR_CanSyscontrol.ModeSwitchFlag = 0;
                    }
                }
            }
            else if(ServoRunModeLatch == ECTHOMMOD)//6hm
            {
                //允许在原点回零完成\未启动\发生超时故障后切入其他模式
                if((STR_InnerGvarPosCtrl.MutexBit.bit.CanopenHomeWork == 0)&&
                    (STR_CanopenHome.HomingStatus.bit.HomingEn== 0))
                {
                    WarnAutoClr(ORIGINOVERTIME);

                    switch(temp222)
                    {
                        case ECTPOSMOD://1pp
                            CanopenHomeReset();
                            ServoRunMode = POSMOD;
                            break;

                        case ECTSPDMOD://3pv
                        case ECTCSVMOD://9csv
                            ServoRunMode = SPDMOD;
                            STR_PosCtrlVar.PosCmdReachLimit= 0;
                            break;

                        case ECTCSPMOD://7csp
                            PV2IPSwitchFlag = 1;
                            ServoRunMode = POSMOD;
                            break;

                        case ECTTOQMOD://4tq
                        case ECTCSTMOD://10cst
                            ServoRunMode = TOQMOD;
                            break;
                            
                        default:
                            break;
                    }
                }
                else
                {
                    temp222 = ECTHOMMOD;
                }
            }
            else if(ServoRunModeLatch == ECTCSPMOD)//8周期同步位置模式
            {
                CSPClear();

                switch(temp222)
                {
                    case ECTPOSMOD://1pv
                        PV2PPSwitchFlag = 1;
                        ServoRunMode = POSMOD;
                        break;
                        
                    case ECTSPDMOD://3pv
                    case ECTCSVMOD://9csv
                        ServoRunMode = SPDMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    case ECTHOMMOD://6hm
                        CanopenHomingModeFuncVarStop();
                        break;
                        
                    case ECTTOQMOD://4tq
                    case ECTCSTMOD://10cst
                        ServoRunMode = TOQMOD;
                        STR_PosCtrlVar.PosCmdReachLimit= 0;
                        break;

                    default:
                        break;
                }
            }
            else if(ServoRunModeLatch == ECTCSVMOD)//9csv
            {
                if(temp222 != ECTSPDMOD)//8CSV模式立即切换
                {
                    //首先执行斜坡停机，停机完成后，再从PV模式切换
//                    if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
//                    {
//                        STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
//                        temp222 = ECTCSVMOD;//保持为速度模式
//                        ServoRunMode = SPDMOD;
//                    }
//                    else
//                    {
                        switch(temp222)
                        {
                            case ECTPOSMOD://1pp
                                PV2PPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTHOMMOD://6hm
                                //防止错误指令导致回零立刻启动
                                CanopenHomingModeFuncVarStop();
                                ServoRunMode = POSMOD;
                                break;

                            case ECTCSPMOD://7csp
                                PV2IPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTTOQMOD://4tq
                            case ECTCSTMOD://10cst
                                ServoRunMode = TOQMOD;
                                break;

                            default:
                                break;
                        }
                        STR_CanSyscontrol.ModeSwitchFlag = 0;
                    //}
                }
            }
            else if(ServoRunModeLatch == ECTCSTMOD)//4tq
            {
                if(temp222 != ECTTOQMOD)//8CSV模式立即切换
                {
                    //首先执行斜坡停机，停机完成后，再从PV模式切换
//                    if((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 10000L))
//                    {
//                        STR_CanSyscontrol.ModeSwitchFlag = 1;//模式切换中
//                        temp222 = ECTCSTMOD;//保持为速度模式
//                        ServoRunMode = TOQMOD;
//                    }
//                    else
                    {
                        switch(temp222)
                        {
                            case ECTPOSMOD://1pp
                                PV2IPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTHOMMOD://6hm
                                CanopenHomingModeFuncVarStop();
                                ServoRunMode = POSMOD;
                                break;

                            case ECTCSPMOD://7csp
                                PV2IPSwitchFlag = 1;
                                ServoRunMode = POSMOD;
                                break;

                            case ECTSPDMOD://3pv
                            case ECTCSVMOD://9csv
                                ServoRunMode = SPDMOD;
                                break;

                            default:
                                break;
                        }
                        STR_CanSyscontrol.ModeSwitchFlag = 0;
                    }
                }
            }
        }
        else//停机或故障状态下允许模式切换
        {
        }
    }
    STR_CanSyscontrol.Mode = temp222;    
    //ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay = STR_CanSyscontrol.Mode;								//// by huangxin201711_26 新的模式返回给上位机的时序不变
	ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay = ObjectDictionaryStandard.DeviceControl.ModesOfOperation;//// by huangxin201711_26 新的模式返回给上位机的时序不变
    ServoRunModeLatch = STR_CanSyscontrol.Mode;
}






/********************************* END OF FILE *********************************/

