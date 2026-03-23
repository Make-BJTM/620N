/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_SpdCtrl.c
 创建人：   何俊辉                 创建日期：2008.11.03
 修改人：   王治国                 修改日期：2011.10.10
 描述：
    1. 获取速度调节器参考指令
    2. 输出函数void GetSpdRef(void)
 修改记录：
    2011.10.10  王治国
    1. 原void SpdZeroSpdLock()函数更名为int32 SpdZeroClamp(int32 SpdRef);
       只保留模拟量速度指令零速钳位(零位固定)功能。
    2. gstr_ServoSystem.g_LoopFlag的命名暂时不做更改，做调度时在更改    wzg20111018
    3. 没有JOG功能、多段速度、斜坡函数功能
    4. 如果添加多段速度、斜坡函数功能，那么要再本文件中添加一个速度控制初始化
       函数，在该函数中调度多段速度、斜坡函数参数的初始化
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "CANopen_Pub.h"

#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_MultiBlockSpeed.h"
#include "FUNC_SpdCtrl.h"
#include "FUNC_Filter.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ModeSelect.h"
#include "CANopen_OD.h"
#include "CANopen_PV.h"
//#include "FUNC_StopProcess.h"
//#include "FUNC_ServoMonitor.h"
#include "CANopen_DeviceCtrl.h"
#include "CANopen_PP.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//单次离线惯量辨识转动圈数计算系数  ((1/1000)*(1/60）* 100)* 2^20)    Q20 zxh
#define INERTIA_CIRCLEPARAQ20   (1748L)     

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/
/* 结构体变量定义 枚举变量定义 */
STR_LOCAL_SPDCONTROL    STR_SpdCtrl; 

STR_BILINEAR_LOWPASS_FILTER     SpeedDisplayFilter = BILINEAR_LOWPASS_FILTER_Defaults;  //定义电机实际输出转速显示滤波器
STR_BILINEAR_LOWPASS_FILTER     SpeedDoFilter = BILINEAR_LOWPASS_FILTER_Defaults;       //定义转速DO滤波器

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

extern  void InitSpdCtrl(void);
extern  void SpdCtrlStopUpdate(void);
extern  void SpdCtrlUpdate(void);
extern  void GetSpdRef(void);
extern  void SpeedShow(void);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
static int32 SpdCmdSource(Uint16 SpdSel);        //根据速度指令来源选择速度指令
Static_Inline void NormalSourceSpdSoftStartInit(void);
Static_Inline void SpdCmdABSel(void);
Static_Inline int32 SpdCmdLimit(int32 SpdRef);
Static_Inline void SpdZeroClamp(void);

Static_Inline void DeltaSpeedSource(void);
Static_Inline void SpdSoftStart(void);
Static_Inline int32 OffLnInertiaSpdRefDeal(void);
/*******************************************************************************
  函数名: extern  void InitSpdCtrl(void);
  输入:   无
  输出:   无
  子函数: 无
    1.
    2.
********************************************************************************/
void InitSpdCtrl(void)
{
    Uint32 Temp = 0;

    #if ECT_ENABLE_SWITCH

    InitCanopenPVMode();       //PV上电初始化  

    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        MultiBlockSpeedInit();
    }

    #else
    MultiBlockSpeedInit();

    #endif
    
    NormalSourceSpdSoftStartInit();

    /* 速度Do滤波器初始化 */
    SpeedDoFilter.Ts = 1000;  //滤波的Ts采样时间按1K进行采样 (((Uint32)1000000 << 10) / 1000) >> 10;
    SpeedDoFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDoFilt;
    SpeedDoFilter.InitLowPassFilter(&SpeedDoFilter);
    
    /* 速度显示滤波器初始化 */
    SpeedDisplayFilter.Ts = 1000;  //速度显示滤波的Ts采样时间按1K进行采样 (((Uint32)1000000 << 10) / 1000) >> 10;
    SpeedDisplayFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDispFilt;
    SpeedDisplayFilter.InitLowPassFilter(&SpeedDisplayFilter);
    //离线惯量辨识
    STR_SpdCtrl.InertiaIdy_WaitTime = (FunCodeUnion.code.InertiaWaitTime * STR_FUNC_Gvar.System.SpdFreq) / 1000L;
    STR_SpdCtrl.InertiaIdy_MaxSpd = FunCodeUnion.code.InertiaSpdMax * 10000L;
    Temp = ((Uint32)STR_FUNC_Gvar.System.SpdFreq * (Uint32)FunCodeUnion.code.InertiaAcceTime) / 1000L;
    STR_SpdCtrl.InertiaIdy_DltaSpd = (10000L *  FunCodeUnion.code.InertiaSpdMax)/ Temp;
    FunCodeUnion.code.InertiaIdy_Circle = (Uint16)((INERTIA_CIRCLEPARAQ20 * FunCodeUnion.code.InertiaSpdMax 
                                            * FunCodeUnion.code.InertiaAcceTime) >> 20);

}


/*******************************************************************************
  函数名: extern  void SpdCtrlStopUpdate(void);
  输入:   无
  输出:   无
  子函数: 无
    1.
    2.
********************************************************************************/
void SpdCtrlStopUpdate(void)
{
    Uint32 Temp = 0;
    //离线惯量辨识
    STR_SpdCtrl.InertiaIdy_WaitTime = (FunCodeUnion.code.InertiaWaitTime * STR_FUNC_Gvar.System.SpdFreq) / 1000L;
    STR_SpdCtrl.InertiaIdy_MaxSpd = FunCodeUnion.code.InertiaSpdMax * 10000L;
    Temp = ((Uint32)STR_FUNC_Gvar.System.SpdFreq * (Uint32)FunCodeUnion.code.InertiaAcceTime) / 1000L;
    STR_SpdCtrl.InertiaIdy_DltaSpd = (10000L *  FunCodeUnion.code.InertiaSpdMax)/ Temp;
    FunCodeUnion.code.InertiaIdy_Circle = (Uint16)((INERTIA_CIRCLEPARAQ20 * FunCodeUnion.code.InertiaSpdMax 
                                            * FunCodeUnion.code.InertiaAcceTime) >> 20);

}
/*******************************************************************************
  函数名: extern  void SpdCtrlRunUpdate(void);
  输入:   无
  输出:   无
  子函数: 无
    1.
    2.
********************************************************************************/
void SpdCtrlUpdate(void)
{
    #if ECT_ENABLE_SWITCH
    CanSpdCtrlUpdate();
    #endif
    
    //普通速度斜坡处理H0605和H0606
    NormalSourceSpdSoftStartInit();

    /* 速度Do滤波器初始化 */
    SpeedDoFilter.Ts = 1000;  //滤波的Ts采样时间按1K进行采样 (((Uint32)1000000 << 10) / 1000) >> 10;
    SpeedDoFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDoFilt;
    SpeedDoFilter.InitLowPassFilter(&SpeedDoFilter);

    /* 速度显示滤波器初始化 */
    SpeedDisplayFilter.Ts = 1000;  //速度显示滤波的Ts采样时间按1K进行采样 (((Uint32)1000000 << 10) / 1000) >> 10;
    SpeedDisplayFilter.Tc = 1000L * FunCodeUnion.code.ER_SpdDispFilt;
    SpeedDisplayFilter.InitLowPassFilter(&SpeedDisplayFilter);

}
/*******************************************************************************
  函数名: extern void GetSpdRef(void)
  输入:   无
  输出:   无
  子函数:  int32 SpdCmdABSel(void)
           int32 SpdRefLmt(int32 SpdRef);
           int16 ZeroSpdLock(void)
  描述:
    1. 获取速度调节器的速度参考指令
    2. 第一版无速度斜坡函数和多段速度函数
    （下一版将速度斜坡函数和多段速度函数、多段转矩、多段位置独立出来一个文件）
********************************************************************************/
void GetSpdRef(void)
{        
    int32  SpdRefTemp = 0;

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD)          //位置控制模式
    {
        SpdRefTemp = STR_FUNC_Gvar.PosCtrl.PosReguOut;
        STR_SpdCtrl.SpdSoftStartRefLatch = SpdRefTemp;
#if ECT_ENABLE_SWITCH
       // STR_SpdCtrl.SpdSoftStartRefLatchQ16 = SpdRefTemp<<16;
		STR_SpdCtrl.SpdSoftStartRefLatchQ16 = ((int64)SpdRefTemp)<<16;		//by huangxin201803 _2 溢出
#endif		
    }
    else if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == SPDMOD)                //速度控制模式
    {
        if(1 == STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn)
        {
            SpdRefTemp = OffLnInertiaSpdRefDeal();
        }
        else if (0 == STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn)
        {
            //速度模式且后台模式不使能
            SpdCmdABSel();                                                //速度指令选择

            STR_SpdCtrl.SpdCommand = SpdCmdLimit(STR_SpdCtrl.SpdCommand); //速度指令限制
            STR_SpdCtrl.SpdCmdLatch = STR_SpdCtrl.SpdCommand;

            if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)/*&&(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==1)*/)   //伺服On使能时，进行速度调节器输入指令赋值
            {
                SpdSoftStart();					   //斜坡函数处理在此位置

                #if ECT_ENABLE_SWITCH
                //drive follow the command Value
                if(STR_CanSyscontrol.Mode == ECTCSVMOD)
				{
				   if(DeviceControlVar.OEMStatus == STATUS_OPERENABLE)
				   {
				   	   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 1;
				   }
				   else
				   {
				   	   ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;
				   }
				} 
                #endif
            }
			else
			{
			    STR_SpdCtrl.SpdSoftStartRefLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
                #if ECT_ENABLE_SWITCH
				STR_SpdCtrl.SpdSoftStartRefLatchQ16 = ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb)<<16;
                if(STR_CanSyscontrol.Mode == ECTCSVMOD)
				{
					ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;
				}
                #endif
            }                                                
            
			SpdZeroClamp();                        //零速钳位(零位固定)

            if(STR_SpdCtrl.SpdZeroClampFlag == 0)
            {
            	SpdRefTemp = STR_SpdCtrl.SpdSoftStartRefLatch;            
            }
            else  //发生零速钳位(零位固定-SpdZeroClampFlag有效时)处理
            {
                SpdRefTemp = STR_SpdCtrl.InnerPosReguOutput;     //调节器输入速度指令来源于内建位置环输出
            }
        }
    }
    else if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD)  //转矩模式时，进行速度调节器输入指令的清零
    {
        SpdRefTemp = 0;
        STR_SpdCtrl.SpdSoftStartRefLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
#if ECT_ENABLE_SWITCH
		STR_SpdCtrl.SpdSoftStartRefLatchQ16 = ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb)<<16;
#endif
	}

    STR_FUNC_Gvar.SpdCtrl.SpdRefLatch = SpdRefTemp; //超程用

    //零速停机 急停转矩停机  抱闸处理
    if((STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop == 1) || (STR_FUNC_Gvar.MonitorFlag.bit.ToqStop == 1) ||
       ((STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock == 1)&&(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == SPDMOD))
       || (STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == DISPWM))
    {
		STR_FUNC_Gvar.SpdCtrl.SpdRef = 0;
        STR_SpdCtrl.SpdSoftStartRefLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
        
		#if ECT_ENABLE_SWITCH
		STR_SpdCtrl.SpdSoftStartRefLatchQ16 = ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb)<<16;
        #endif
    }
    else if(STR_FUNC_Gvar.MonitorFlag.bit.OTClamp == 1) //超程处理
    {
        STR_FUNC_Gvar.SpdCtrl.SpdRef = STR_FUNC_Gvar.PosCtrl.PosReguOut;

        //超程后，解除超程，速度软启动锁存值为超程前的锁存值，加速斜坡无作用hy11615
		//此处必须这么用，否则超程后，如果停机时，速度反馈出现过冲，软启动锁存值为负，电机将开始运行
        //STR_SpdCtrl.SpdSoftStartRefLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
    }
    else
    {
        STR_FUNC_Gvar.SpdCtrl.SpdRef = SpdRefTemp;
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == SPDMOD) 
	{	
		if(( (STR_FUNC_Gvar.DivarRegLw.bit.Pot == 1) && (STR_FUNC_Gvar.SpdCtrl.SpdRef > 0) ) 
	              || ( (STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) && (STR_FUNC_Gvar.SpdCtrl.SpdRef < 0) ))
		{
		  STR_FUNC_Gvar.SpdCtrl.SpdRef = 0 ;
		}
	}
}

/*******************************************************************************
  函数名: static int32 SpdCmdSource(Uint16 SpdSel)
  输入:  速度选择来源
  输出:  速度指令
  子函数: 无
  描述:
    1. 根据速度指令来源选择速度指令
    2. 第一版中，速度指令来源限于数字给定和模拟给定
    3. 多段指令选择调度在版函数中
********************************************************************************/
static int32 SpdCmdSource(Uint16 SpdSel)
{
    int32 Ref = 0;

    switch(SpdSel)
    {
        case 0:
            Ref = 10000L * (int16)FunCodeUnion.code.SL_SpdCMDFromKeyBoard;//H06_03键盘设定
            break;
            
#if ((DRIVER_TYPE == SERVO_620N)||(DRIVER_TYPE == SERVO_650N))
        case 1:
            //采样电压速度增益计算 单位0.0001rpm
            Ref = 0;
            break;
        case 2:
            //采样电压速度增益计算 单位0.0001rpm
            Ref = 0;
            break;
#else
        case 1:
            //采样电压速度增益计算 单位0.0001rpm
            Ref = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain;
            Ref = (Ref * (int64)STR_FUNC_Gvar.AI.AI1VoltOut)>>15;
            break;
        case 2:
            //采样电压速度增益计算 单位0.0001rpm
            Ref = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain ;
            Ref = (Ref * (int64)STR_FUNC_Gvar.AI.AI2VoltOut)>>15;
            break;
#endif
        case 3:
            Ref = 0;           //620P  删除模拟速度指令输入AI3
            break;
        case 4://点动 
            Ref = 0;
            break;
        case 5:
            Ref = MultiBlockSpeedDeal();      // 多段指令选择调度在此位置
            break;

        default:
            Ref = 0;
            PostErrMsg(DEFUALTERR);
            break;
    }
    return Ref;
}
/*******************************************************************************
  函数名: Static_Inline int32 SpdCmdABSel(void)
  输入:   无
  输出:   无
  子函数: int32 SpdCmdSource(Uint16 SpdSel)
  描述:
    1. 根据功能码H0600 H0601 H0602设定选择速度指令
    2. 根据DivarRegLw.bit.SPDDirSel及功能码H0202设定，改变速度指令方向
********************************************************************************/
Static_Inline void SpdCmdABSel(void)
{
	int32 Temp = 0;
    int32 SpdRef = 0;

    int64 Temp111=0;

    if(STR_FUNC_Gvar.Monitor.SpdJOGCmd != 0)
    {
        //面板点动
        if(STR_FUNC_Gvar.Monitor.SpdJOGCmd == 0x11)
        {
            SpdRef = (10000L) * FunCodeUnion.code.SL_JOGSpdCommand;
        }
        else if(STR_FUNC_Gvar.Monitor.SpdJOGCmd == 0x21)
        {
            SpdRef = (-10000L) * FunCodeUnion.code.SL_JOGSpdCommand;
        }
        else
        {
            SpdRef = 0;
        }
    }
    else if(1 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn)
    {
        //DI正向点动
        SpdRef = 10000L * FunCodeUnion.code.SL_JOGSpdCommand;
    }
    else if(2 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn)
    {
        //DI反向点动
        SpdRef = (-10000L) * FunCodeUnion.code.SL_JOGSpdCommand;
    }
    else if(3 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn)
    {
        //D取消DI点动，减速
        SpdRef = 0;
    }
    #if ECT_ENABLE_SWITCH
    else if(FunCodeUnion.code.BP_ModeSelet==9)
	{
        STR_FUNC_Gvar.OscTarget.InputTarSpd = (int32)ObjectDictionaryStandard.ProVelMode2.TargetVelocity;

        //急停过程中\暂停\模式切换
        if((DeviceControlVar.OEMStatus != STATUS_OPERENABLE)
            ||(STR_CanSyscontrol.ModeSwitchFlag == 1))
        {
            SpdRef = 0;
        }
        //运行状态
        else
        {
            if(STR_CanSyscontrol.Mode == ECTSPDMOD)
            {
                Temp111 =(int64)((int32)ObjectDictionaryStandard.ProVelMode2.TargetVelocity);
            }
            else if(STR_CanSyscontrol.Mode == ECTCSVMOD)
            {
                Temp111 = (int64)((int32)ObjectDictionaryStandard.ProVelMode2.TargetVelocity) 
                          + (int64)((int32)ObjectDictionaryStandard.CstOffset.VelOff);
            }
            SpdRef = UserVelUnit2RpmUnit(&STR_VelEnc_Factor,Temp111);
        }
        
        //607E bit6=1速度指令反向
        if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x40)== 0x40)
    	{
    	    SpdRef = 0 - SpdRef;
    	}
        
    }
    else if(FunCodeUnion.code.OEM_LocalModeEn==1)
    #endif
    {
        switch(FunCodeUnion.code.SL_SpdABSwitchFlag)
        {
            case 0:    //选择指令A来源
               SpdRef = SpdCmdSource(FunCodeUnion.code.SL_SpdACommandFrom);
               break;
            case 1:    //选择指令B来源
               SpdRef = SpdCmdSource(FunCodeUnion.code.SL_SpdBCommandFrom);
               break;
            case 2:    //选择指令A+B来源
               SpdRef = SpdCmdSource(FunCodeUnion.code.SL_SpdACommandFrom);
               Temp = SpdCmdSource(FunCodeUnion.code.SL_SpdBCommandFrom);
               SpdRef += Temp;
               break;
            case 3:
                if(STR_FUNC_Gvar.DivarRegLw.bit.CmdSel==0) //cmd =0 ,对应A，IO输入切换
                {
                     SpdRef = SpdCmdSource(FunCodeUnion.code.SL_SpdACommandFrom);
                }
                else   //cmd =1 ,对应B，IO输入切换
                {
                     SpdRef = SpdCmdSource(FunCodeUnion.code.SL_SpdBCommandFrom);
                }
                break;
            case 4:   //通讯给定
                SpdRef = (int32)A_SHIFT16_PLUS_B(AuxFunCodeUnion.code.CC_CommSendSpeedH,AuxFunCodeUnion.code.CC_CommSendSpeedL);
                SpdRef = SpdRef * 10;
                break;
            default:
                SpdRef = 0;
                PostErrMsg(DEFUALTERR);
                break;
        }
    }
    
    //用DivarRegLw.bit.SPDDirSel控制换向
    SpdRef = STR_FUNC_Gvar.DivarRegLw.bit.SPDDirSel ? (- SpdRef) : SpdRef;
    
    STR_SpdCtrl.SpdCommand = SpdRef;
}
/*******************************************************************************
  函数名: Static_Inline int32 SpdRefLmt( int32 SpdRef);
  输入:   无
  输出:   无
  子函数: 无
  描述:
    1. 根据功能码H0015 H0607 H0608 H0609设定，对速度指令进行限幅
********************************************************************************/
Static_Inline int32 SpdCmdLimit(int32 SpdRef)
{
	int32 Temp = 0;

    
    #if ECT_ENABLE_SWITCH
    if((FunCodeUnion.code.BP_ModeSelet == 9)
        &&(0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd)
        &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn))
    {
        SpdRef = CANopenVelLmt(SpdRef);
    }
    else if((FunCodeUnion.code.OEM_LocalModeEn==1)
            ||(0 != STR_FUNC_Gvar.Monitor.SpdJOGCmd)
            ||(0 != STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn))
    #endif
    {
        if(FunCodeUnion.code.MT_MaxSpd > FunCodeUnion.code.SL_SpdMaxLimit)    //需要用电机的最大转速和最大转速限制进行关联限制
        {
            Temp = 10000L * (int16)FunCodeUnion.code.SL_SpdMaxLimit;//最大转速限制
        }
        else
        {
            Temp = 10000L * (int16)FunCodeUnion.code.MT_MaxSpd;//电机最大转速
        }

        //最大速度限制，正向限制
        SpdRef = MIN(SpdRef,Temp);

        //最大速度限制，负向限制
        Temp = - Temp;
        SpdRef = MAX(SpdRef,Temp);

        //正向速度限制
        Temp = 10000L * (int16)FunCodeUnion.code.SL_SpdPositiveLimit;
        SpdRef = MIN(SpdRef,Temp);

        //负向速度限制
        Temp = (-10000L) * (int16)FunCodeUnion.code.SL_SpdNegativeLimit;
        SpdRef = MAX(SpdRef,Temp);
    }
    
    return (int32)SpdRef;
}

/*******************************************************************************
  函数名: Static_Inline int32 SpdZeroClamp(int32 SpdRef);
  输入:   无
  输出:   无
  子函数: 无
  描述:
    1. 运行状态且DivarRegLw.bit.ZClamp有效时，执行零速钳位(零位固定)功能。
    2. 防抖、回差功能：
       当检测到模拟量通道输入的速度指令绝对值连续5次小于H0615设定的
       速度时，伺服进入锁定状态。
       伺服进入锁定状态后，当检测到模拟量通道输入的速度指令绝对值大
       于H0615+SpdZeroClampTolerance时，退出锁定状态。
    3. 伺服进入锁定状态时，运行内建位置环，采用比例控制，位置指令恒定
       为0，调节器的比例系数为H08_02，
    4. 调节器调度频率与位置模式位置环调节器调度频率一致。
    5. 在本函数内设定零速钳位(零位固定)有效标志位
       SpdZeroClampFlag
********************************************************************************/
Static_Inline void SpdZeroClamp(void)
{
    static int32 InnerPosReguErrSum = 0;
    static Uint8 DelayCnt = 0;

    Uint32 SpdZeroClampOnLevel = 0;
    Uint32 SpdZeroClampOffLevel = 0;
    int64  Product = 0;

    //运行状态且DivarRegLw.bit.ZClamp有效时，执行该功能
    if((STR_FUNC_Gvar.DivarRegLw.bit.ZClamp == 1) && (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))
    {
        //计算进入或退出伺服进入锁定状态的速度指令阀值
        if(FunCodeUnion.code.SL_SpdZeroClampTolerance > 100)
        {
            SpdZeroClampOnLevel = 10000L * FunCodeUnion.code.SL_SpdZeroClampTolerance;
            SpdZeroClampOffLevel = SpdZeroClampOnLevel + 200000L;
        }
        else
        {
            SpdZeroClampOnLevel = 10000L * FunCodeUnion.code.SL_SpdZeroClampTolerance;
            SpdZeroClampOffLevel = SpdZeroClampOnLevel + 100000L;
        }

        if(STR_SpdCtrl.SpdZeroClampFlag == 0)
        {
            //当速度指令绝对值连续5次小于SpdZeroClampOnLevel时，伺服进入锁定状态
            if((Uint32)ABS(STR_SpdCtrl.SpdSoftStartRefLatch) <= SpdZeroClampOnLevel)
            {
                if(DelayCnt < 5)
                {
                    DelayCnt ++;
                }
                else
                {
                    //进入内建位置调节器
                    STR_SpdCtrl.SpdZeroClampFlag = 1;
                    InnerPosReguErrSum = 0;
                    STR_SpdCtrl.InnerPosReguOutput = 0;
                }
            }
            else
            {
                DelayCnt = 0;
            }
        }
        else
        {
            //当速度指令绝对值大于于SpdZeroClampOffLevel时，伺服退出锁定状态
            if((Uint32)ABS(STR_SpdCtrl.SpdSoftStartRefLatch) > SpdZeroClampOffLevel)
            {
                //退出内建位置调节器，复位参数
                STR_SpdCtrl.SpdZeroClampFlag = 0;
                InnerPosReguErrSum = 0;
                STR_SpdCtrl.InnerPosReguOutput = 0;
                DelayCnt = 0;
            }
            else if(STR_FUNC_Gvar.ScheldularFlag.bit.ZClampInnerPosFlag == 1)
             //gstr_ServoSystem.g_LoopFlag的命名暂时不做更改    wzg20111018
             //调节器调度频率与位置模式位置环调节器调度频率一致。
            {
                 //进行内建位置调节器调节
                 InnerPosReguErrSum = InnerPosReguErrSum - STR_FUNC_Gvar.PosCtrl.PosFdb;
                 Product = (int64)InnerPosReguErrSum * (int64)(int16)FunCodeUnion.code.GN_Pos_Kp;
                 Product = (int64)Product * (int32)STR_FUNC_Gvar.PosCtrl.KpCoef_Q15;
                 STR_SpdCtrl.InnerPosReguOutput = (int32)(Product >> 15) ;

                 //进行速度指令限幅
                 STR_SpdCtrl.InnerPosReguOutput = SpdCmdLimit(STR_SpdCtrl.InnerPosReguOutput);
            }
        }
    }
    else
    {
        //退出内建位置调节器，复位参数
        STR_SpdCtrl.SpdZeroClampFlag = 0;
        InnerPosReguErrSum = 0;
        STR_SpdCtrl.InnerPosReguOutput = 0;
        DelayCnt = 0;
    }

    //内建位置环调节器调度标志位清零
    STR_FUNC_Gvar.ScheldularFlag.bit.ZClampInnerPosFlag = 0;
}

/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述:
********************************************************************************/
Static_Inline void NormalSourceSpdSoftStartInit(void)
{
    Uint32 SoftRiseTime = 0;      //加速时间变量
    Uint32 SoftDownTime = 0;      //减速时间变量
    Uint32 Temp = 0;              //速度增量

    SoftRiseTime = (Uint32)FunCodeUnion.code.SL_SpdCMDRiseTime; //速度环的加速时间参数 H0605
    SoftDownTime = (Uint32)FunCodeUnion.code.SL_SpdCMDDownTime; //速度环的减速时间参数 H0606

    /* 加速时的速度增量的计算 */
    if(SoftRiseTime == 0)
    {
        STR_SpdCtrl.NormalDeltaSpdRise_Q16 = (int64)((((int64)FunCodeUnion.code.SL_SpdMaxLimit * 10000)<<16) << 1); //0加减速时间对应的加减速时的最大速度增量
    }
    else
    {
        Temp = ( SoftRiseTime * (Uint32)STR_FUNC_Gvar.System.SpdFreq );
        SoftRiseTime = Temp / 1000;   //除于ms时间单位,等于1s内加速的次数	
        STR_SpdCtrl.NormalDeltaSpdRise_Q16 = (int64)((((int64)10000 * SPDUPDOWM_MAXREF)<<16) / (int64)SoftRiseTime);   //每一次加速时的，加速的增量
    }

    /* 减速时的速度增量的计算 */
    if(SoftDownTime == 0)
    {
        STR_SpdCtrl.NormalDeltaSpdDown_Q16 = (int64)((((int64)FunCodeUnion.code.SL_SpdMaxLimit * 10000)<<16) << 1); //0加减速时间对应的加减速时的最大速度增量
    }
    else
    {
        Temp = ( SoftDownTime * (Uint32)STR_FUNC_Gvar.System.SpdFreq );
        SoftDownTime = (Temp / 1000);   //除于ms时间单位,等于1s内减速的次数
        STR_SpdCtrl.NormalDeltaSpdDown_Q16 = (int64)((((int64)10000 * SPDUPDOWM_MAXREF)<<16) / (int64)SoftDownTime);  //每一次减速时的，减速的增量
    }
}

/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述:
********************************************************************************/
Static_Inline void DeltaSpeedSource(void)
{

    #if ECT_ENABLE_SWITCH
    if((FunCodeUnion.code.BP_ModeSelet == 9)
        &&(0 == STR_FUNC_Gvar.Monitor.SpdJOGCmd)
        &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn))
    {
        ECTPVAccLmt();
    }
    else if((FunCodeUnion.code.OEM_LocalModeEn==1)
             ||(0 != STR_FUNC_Gvar.Monitor.SpdJOGCmd)
             ||(0 != STR_FUNC_Gvar.MonitorFlag.bit.DIJOGEn))
    {
        if( FunCodeUnion.code.SL_SpdBCommandFrom==5 && 
            (FunCodeUnion.code.SL_SpdABSwitchFlag == 1 || FunCodeUnion.code.SL_SpdABSwitchFlag == 2 || 
            (FunCodeUnion.code.SL_SpdABSwitchFlag==3 && STR_FUNC_Gvar.DivarRegLw.bit.CmdSel==1) ) 
            )   //加减速增量来源于多段速度
        {
            STR_SpdCtrl.DeltaSpeedRise_Q16 =  MultiBlockDeltaSpdRise_Q16;
            STR_SpdCtrl.DeltaSpeedDown_Q16 =  MultiBlockDeltaSpdDown_Q16;
        }
        else   //加减速增量不来源于多段速度
        {
            STR_SpdCtrl.DeltaSpeedRise_Q16 =  STR_SpdCtrl.NormalDeltaSpdRise_Q16;
            STR_SpdCtrl.DeltaSpeedDown_Q16 =  STR_SpdCtrl.NormalDeltaSpdDown_Q16;
        }
    }
    #else
    if( FunCodeUnion.code.SL_SpdBCommandFrom==5 && 
        (FunCodeUnion.code.SL_SpdABSwitchFlag == 1 || FunCodeUnion.code.SL_SpdABSwitchFlag == 2 || 
        (FunCodeUnion.code.SL_SpdABSwitchFlag==3 && STR_FUNC_Gvar.DivarRegLw.bit.CmdSel==1) ) 
        )   //加减速增量来源于多段速度
    {
        STR_SpdCtrl.DeltaSpeedRise_Q16 =  MultiBlockDeltaSpdRise_Q16;
        STR_SpdCtrl.DeltaSpeedDown_Q16 =  MultiBlockDeltaSpdDown_Q16;
    }
    else   //加减速增量不来源于多段速度
    {
        STR_SpdCtrl.DeltaSpeedRise_Q16 =  STR_SpdCtrl.NormalDeltaSpdRise_Q16;
        STR_SpdCtrl.DeltaSpeedDown_Q16 =  STR_SpdCtrl.NormalDeltaSpdDown_Q16;
    }
    #endif    
}

/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述:
********************************************************************************/
Static_Inline void SpdSoftStart(void)
{
    int64   SpdAccelTemp_Q16 = 0;      //加减速时速度指令中间变量
    int64   SpdCommand_Q16;             //速度指令Q10格式
    int64   SoftStartSpdRefLatch_Q16;   //速度调节器输入的速度指令Q10格式

    SpdCommand_Q16 = (int64)STR_SpdCtrl.SpdCommand << 16;
    SoftStartSpdRefLatch_Q16 = STR_SpdCtrl.SpdSoftStartRefLatchQ16;

    if(SoftStartSpdRefLatch_Q16 == SpdCommand_Q16)
    {
        SpdAccelTemp_Q16 = SpdCommand_Q16;
        return;
    }

    //SoftStartSpdRefLatch_Q10 = STR_SpdCtrl.SpdSoftStartRefLatch;

    DeltaSpeedSource();

    if (STR_SpdCtrl.SpdSoftStartRefLatchQ16 < SpdCommand_Q16)
    {
        if(STR_SpdCtrl.SpdSoftStartRefLatchQ16 >= 0)
        {
            SpdAccelTemp_Q16 = SoftStartSpdRefLatch_Q16 + STR_SpdCtrl.DeltaSpeedRise_Q16;    //正向加速；
        }
        else
        {
            SpdAccelTemp_Q16 = SoftStartSpdRefLatch_Q16 + STR_SpdCtrl.DeltaSpeedDown_Q16;   //反向减速；
			if((SpdAccelTemp_Q16>=0)&&(STR_SpdCtrl.SpdCommand>=0))
			{
			    SpdAccelTemp_Q16 =0;
			}
        }

        if(SpdAccelTemp_Q16 < SpdCommand_Q16)
        {
            STR_SpdCtrl.SpdSoftStartRefLatchQ16 = SpdAccelTemp_Q16;
        }
        else
        {
            SpdAccelTemp_Q16 = SpdCommand_Q16;
		    STR_SpdCtrl.SpdSoftStartRefLatchQ16 = SpdAccelTemp_Q16;
        }
    }
    else
    {
        if(STR_SpdCtrl.SpdSoftStartRefLatchQ16 >0 )
        {
            SpdAccelTemp_Q16 = SoftStartSpdRefLatch_Q16 - STR_SpdCtrl.DeltaSpeedDown_Q16;    //正向减速；    
			if((SpdAccelTemp_Q16<=0)&&(STR_SpdCtrl.SpdCommand<=0))
			{
			    SpdAccelTemp_Q16=0;
			}  
        }
        else
        {
            SpdAccelTemp_Q16 = SoftStartSpdRefLatch_Q16 - STR_SpdCtrl.DeltaSpeedRise_Q16;    //反向加速；    
        }

        if(SpdAccelTemp_Q16 > SpdCommand_Q16)
        {
            STR_SpdCtrl.SpdSoftStartRefLatchQ16 = SpdAccelTemp_Q16;
        }
        else
        {
            SpdAccelTemp_Q16 = SpdCommand_Q16;
			STR_SpdCtrl.SpdSoftStartRefLatchQ16 = SpdAccelTemp_Q16;
        }
    }
    STR_SpdCtrl.SpdSoftStartRefLatch = STR_SpdCtrl.SpdSoftStartRefLatchQ16>>16;
}
/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述:
********************************************************************************/
void SpeedShow(void)
{
    int32 SpdTemp = 0;

    // 实际电机转速输出显示需要进行滤波处理
    SpeedDisplayFilter.Input = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
    SpeedDisplayFilter.LowPassFilter(&SpeedDisplayFilter);
    SpdTemp = SpeedDisplayFilter.Output;

    AuxFunCodeUnion.code.DP_MotorSpd = (SpdTemp + Sign_NP(SpdTemp) * 5000) / 10000;    //H0B00_速度显示计算（加滤波）
    STR_FUNC_Gvar.OscTarget.SpdFdbFlt = AuxFunCodeUnion.code.DP_MotorSpd;

    SpdTemp = (SpdTemp + Sign_NP(SpdTemp) * 500) / 1000;    //H0B55_速度显示0.1rpm计算（加滤波）
    AuxFunCodeUnion.code.DP_MotorSpdL = (int16)SpdTemp;
    AuxFunCodeUnion.code.DP_MotorSpdH = (int16)(SpdTemp >> 16);

    //MTR模块速度调节器输入,转矩模式或Soff时该值等于0
    AuxFunCodeUnion.code.DP_SpdRef = ( STR_FUNC_Gvar.SpdCtrl.SpdRef + 
                                       Sign_NP(STR_FUNC_Gvar.SpdCtrl.SpdRef) * 5000) / 10000; //H0B01_用于显示速度指令

    // 实际电机转速输出显示需要进行滤波处理
    SpeedDoFilter.Input = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb / 100;
    SpeedDoFilter.LowPassFilter(&SpeedDoFilter);
    SpdTemp = SpeedDoFilter.Output / 100;

    //滤波后的绝对速度提供给中断定长用于当H0526=0且速度绝对值小于10rpm时，不响应中断定长
    STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt = SpdTemp;

    SpdTemp = ABS(SpdTemp);

    //零速信号 判断
    if(SpdTemp <= (int32)FunCodeUnion.code.SL_ZeroSpdTolerance)
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_Zero = 1;
    }
    else 
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_Zero = 0;
    }

    //旋转信号 判断
    if( SpdTemp >= (int32)FunCodeUnion.code.SL_SpdZeroTolerance)
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_TGon = 1;
    }
    else
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_TGon = 0;
    }

    //速度到达判断      由于我司用的是Do滤波后的速度反馈，滤波比较大，所以速度一致和速度到达都不用加滞环
    SpdTemp = SpeedDoFilter.Output / 100;
    SpdTemp = ABS(SpdTemp);

    if(SpdTemp <= (int32)FunCodeUnion.code.SL_SpdArriveTolerance)
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_VArr = 0;
    }
    else
    {
        STR_FUNC_Gvar.SpdCtrl.DovarReg_VArr = 1;
    }

	/*CANopen 402添加   hy11615 2013-12-27*/
    #if ECT_ENABLE_SWITCH
    if(FunCodeUnion.code.BP_ModeSelet == 9)
    {
        CANopenSpdMonitor();
    }
    else if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        //速度一致 判断
        if( (STR_FUNC_Gvar.MonitorFlag.bit.RunMod != SPDMOD) || (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN) )
        {
            STR_FUNC_Gvar.SpdCtrl.DovarReg_VCmp = 0;
            return;
        }

        SpdTemp = SpeedDoFilter.Output - (STR_SpdCtrl.SpdCmdLatch / 100);
        SpdTemp = ABS(SpdTemp / 100);

        if(SpdTemp <= FunCodeUnion.code.SL_SpdReachTolerance)
        {
            STR_FUNC_Gvar.SpdCtrl.DovarReg_VCmp = 1;
        }
        else
        {
            STR_FUNC_Gvar.SpdCtrl.DovarReg_VCmp = 0;
        }
    }
    else
    {

    }
    #endif

    
}

/*******************************************************************************
  函数名: extern  void InitSpdCtrl(void);
  输入:   无
  输出:   无
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline int32 OffLnInertiaSpdRefDeal(void)
{
    static int32    InertiaSpdRef = 0;
    static Uint32   InertiaCnt = 0;
    static int8     InertiaState;
    static int8     NextInertiaState = 0;

    if(FunCodeUnion.code.OffLnInertiaModeSel == 1)   //JOG模式
    {
        if(STR_FUNC_Gvar.Monitor.OffLnInrtCmd == 0x11)  //正转
        {
            if(InertiaSpdRef < (STR_SpdCtrl.InertiaIdy_MaxSpd - STR_SpdCtrl.InertiaIdy_DltaSpd))
            {
                InertiaSpdRef += STR_SpdCtrl.InertiaIdy_DltaSpd;
            }
            else
            {
                InertiaSpdRef = STR_SpdCtrl.InertiaIdy_MaxSpd;
            }
        }
        else if(STR_FUNC_Gvar.Monitor.OffLnInrtCmd == 0x21)  //反转
        {
            if(InertiaSpdRef > (STR_SpdCtrl.InertiaIdy_DltaSpd - STR_SpdCtrl.InertiaIdy_MaxSpd))
            {
                InertiaSpdRef -= STR_SpdCtrl.InertiaIdy_DltaSpd;
            }
            else
            {
                InertiaSpdRef = 0 - STR_SpdCtrl.InertiaIdy_MaxSpd;
            }
        }
        else
        {
            if(InertiaSpdRef > STR_SpdCtrl.InertiaIdy_DltaSpd)
            {
                InertiaSpdRef -= STR_SpdCtrl.InertiaIdy_DltaSpd;
            }
            else if(InertiaSpdRef < -STR_SpdCtrl.InertiaIdy_DltaSpd)
            {
                InertiaSpdRef += STR_SpdCtrl.InertiaIdy_DltaSpd;
            }
            else
            {
                InertiaSpdRef = 0;
            }
        }
    }
    else     //正反三角波模式    默认
    {
        if((STR_FUNC_Gvar.Monitor.OffLnInrtCmd != 0x11) && (STR_FUNC_Gvar.Monitor.OffLnInrtCmd != 0x21))
        {
            InertiaCnt = 0;
            InertiaState = 0;
            InertiaSpdRef = 0;
            NextInertiaState = 0;
            return(InertiaSpdRef);
        }

        switch(InertiaState)
        {
            case 0:    //无有效按键
                InertiaSpdRef = 0;
                InertiaCnt = 0;
                if(STR_FUNC_Gvar.Monitor.OffLnInrtCmd == 0x11)
                {
                    InertiaState = 1;
                }
                else if(STR_FUNC_Gvar.Monitor.OffLnInrtCmd == 0x21)
                {
                    InertiaState = 3;
                }
                break;
            case 1:    //正向加速
                if(InertiaSpdRef < (STR_SpdCtrl.InertiaIdy_MaxSpd - STR_SpdCtrl.InertiaIdy_DltaSpd))
                {
                    InertiaSpdRef += STR_SpdCtrl.InertiaIdy_DltaSpd;
                }
                else
                {
                    InertiaState = 2;
                }
                break;
            case 2:    //正向减速
                if(InertiaSpdRef > STR_SpdCtrl.InertiaIdy_DltaSpd)
                {
                    InertiaSpdRef -= STR_SpdCtrl.InertiaIdy_DltaSpd;
                }
                else
                {
                    InertiaSpdRef = 0;
                    InertiaCnt = 0;
                    NextInertiaState = 3;
                    InertiaState = 5;
                }
                break;
            case 3:    //反向加速
                if(InertiaSpdRef > (STR_SpdCtrl.InertiaIdy_DltaSpd - STR_SpdCtrl.InertiaIdy_MaxSpd))
                {
                    InertiaSpdRef -= STR_SpdCtrl.InertiaIdy_DltaSpd;
                }
                else
                {
                    InertiaState = 4;
                }
                break;
            case 4:    //反向减速速
                if(InertiaSpdRef < -STR_SpdCtrl.InertiaIdy_DltaSpd)
                {
                    InertiaSpdRef += STR_SpdCtrl.InertiaIdy_DltaSpd;
                }
                else
                {
                    InertiaSpdRef = 0;
                    InertiaCnt = 0;
                    NextInertiaState = 1;
                    InertiaState = 5;
                }
                break;
            case 5:    //延时等待
                InertiaSpdRef = 0;

                InertiaCnt ++;
                if(InertiaCnt >= STR_SpdCtrl.InertiaIdy_WaitTime)
                {
                    InertiaCnt = 0;
                    InertiaState = NextInertiaState;
                    NextInertiaState = 0;
                }
                break;
            default:
                InertiaState = 0;
                NextInertiaState = 0;
                break;
        }
    }

    return(InertiaSpdRef);
}

/********************************* END OF FILE *********************************/
