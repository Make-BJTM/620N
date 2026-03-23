/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_ServoMonitor.c
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
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_ServoMonitor.h"
#include "FUNC_ServoError.h"
#include "FUNC_StopProcess.h"
#include "FUNC_DiDo.h"
#include "FUNC_ErrorCode.h"   
#include "FUNC_FunCode.h" 
#include "FUNC_Filter.h" 
#include "FUNC_ManageFunCode.h"
#include "FUNC_ModeSelect.h"
#include "PUB_Main.h"
#include "CANopen_OD.h"
#include "CANopen_DeviceCtrl.h"
#include "ECT_Probe.h"
#include "CANopen_Pub.h"
#include "CANopen_PP.h"
#include "CANopen_Home.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define  RunAwayDELAYTIME  30   //飞车监控处理延迟处理时间，单位ms

#define  TIMESTAMPCNT      99   //时间戳记录用，100对应1k调度，0.1s

#define  GDCHECK 0              //使能GD是否发生检测功能


#define  BK_OFF_STATE_SPD     200000              //使能GD是否发生检测功能
/*
//-----------使能与关PWM----------------
#define   ENPWM          0
#define   DISPWM         1
*/
//-----------动态制动--------------------
#define   ENGD           0  
#define   DISGD          1  
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_SERVO_MONITOR  STR_ServoMonitor;

UNI_WARN_RELEASE_FLAG   UNI_WarnReg = {0};   //警告是否解除不显示标志位变量

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
Uint8 ServoOn = 0;          //监控模块内部使用的伺服On信息值 1-为Di输入了伺服On信号，0-为Di输入了伺服Off信号；  -》SvOn_m
Uint8 ServoOnLast = 0;      //监控模块内部使用的上一次伺服On信息值      -》SvEnDI

Uint8 ServoRunStatusUpdate = 0;   //监控模块运行状态跟新变量


Uint32 ServoRunTime = 0;    //伺服运行时间记录替代全局变量	gstr_Gvar.TimeStamp  //G2_LH 12.15 


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void ServoMonitor(void);
void InitMonitorHardWare(void);
void ServoRunManage(void);
void MonitorShow(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
void OverTravelMonitor(void);
//void GDMonitor(void);
void RunStatusMonitor(void);
void BrakeOperate(void);
void RunAwayMointor(void);
void NormalRunStateSwitch(void);
void ErrorRunStateSwitch(void);
void EmergencyStopMonitor(void);


/*******************************************************************************
  函数名: 
  输入:   
  输出:  
  子函数:          
  描述: 初始化伺服监控硬件        
********************************************************************************/ 
void InitMonitorHardWare()
{       
//检测GD电路
#if GDCHECK
    WriteGDStop(ENGD);
    DELAY_US(20L);
    GDMonitor();
  
    WriteGDStop(DISGD);
    DELAY_US(20L);
    GDMonitor();
#endif
    //伺服OFF时的停机状态
    ServoStopStatus(STR_ServoMonitor.StopCtrlVar.Err1StopState);

    ServoRunTime = ((int32)FunCodeUnion.code.ES_ServoTime_H<<16) + FunCodeUnion.code.ES_ServoTime_L;   //将伺服运行时间的时间戳初始化，每次上电初始化一次，即初始化上次运行时的保存值 

    ServoRunTime += 300;  //记录时间戳 每次上电加30s

   	AuxFunCodeUnion.code.DP_ServoTime_Lo = ServoRunTime & 0x0FFFF;    //H0B19 总上电时间低16位
    AuxFunCodeUnion.code.DP_ServoTime_Hi = ServoRunTime >> 16;        //H0B19 总上电时间高16位
    FunCodeUnion.code.ES_ServoTime_L  = AuxFunCodeUnion.code.DP_ServoTime_Lo;  //运行故障时的时间记录 ES_ServoTime_L 每个一个小时存一次，这样导致总上电时间DP_ServoTime_Lo存在1小时的误差
    FunCodeUnion.code.ES_ServoTime_H  = AuxFunCodeUnion.code.DP_ServoTime_Hi;  //运行故障时的时间记录 ES_ServoTime_L 每个一个小时存一次，这样导致总上电时间DP_ServoTime_Lo存在1小时的误差

}//end of 硬件初始化


/*******************************************************************************
  函数名: 
  输入:   
  输出:  
  子函数:          
  描述:  伺服监控调度
         
********************************************************************************/ 
void ServoMonitor()
{
	#if ECT_ENABLE_SWITCH
    //CANopenPosLimt();    
	#endif
    DiProcess();  //获取外部DI端子状态

    RunStatusMonitor();  //--伺服ON-OFF处理

#if GDCHECK
    GDMonitor();  //GD电路检测
#endif
    
    EmergencyStopMonitor();  //紧急停机监控，当紧急停机Di上升沿响应，下降沿解除

    OverTravelMonitor();  //超程监控,只有端子被选择并有效时产生超程处理响应

    StopProcess();  //扫描停机处理

	#if ECT_ENABLE_SWITCH
    CanopenDeviceControlFunc();
	#endif
    

    ErrProcess_MaimLoop(); //故障处理主循环调度函数

    DoProcess();   //更新外部DO状态

    DoAllocateError(); 

}


/*******************************************************************************
  函数名: Static_Inline void EmergencyStopMonitor()
  输入:   紧急停机Di和功能码是否有效
  输出:   电机停止
  子函数: 
  描述:   

********************************************************************************/ 
Static_Inline void EmergencyStopMonitor()
{
    static Uint8  EmergencyStopDiLast = 0;   //记录上次状态

    //if((STR_FUNC_Gvar.DivarRegHi.bit.EmergencyStop == 1)&&(DeviceControlVar.StatusStep == STATUS_OPERENABLE))  //紧急停机产生
	if(STR_FUNC_Gvar.DivarRegHi.bit.EmergencyStop == 1)
	{
		if(EmergencyStopDiLast == 0)         
        {
            PostErrMsg(EMERGENCYSTOPWARN);  //报出紧急停机警告		
        }		
		if(DeviceControlVar.StatusStep == STATUS_OPERENABLE)
	    {
	        //置紧急停机响应标志位
	#if ECT_ENABLE_SWITCH
	        if((STR_CanSyscontrol.Mode ==ECTPOSMOD)||(STR_CanSyscontrol.Mode ==ECTHOMMOD))
	        {
	            CanopenHomeReset();
	            CanopenPPReset();
				CanopenPPPosBuffReset();
	            CanopenHomingReset();
	        }
	#endif        
			//上升沿
//			if(EmergencyStopDiLast == 0)         
//	        {
//	            PostErrMsg(EMERGENCYSTOPWARN);  //报出紧急停机警告		
//	        }
	
	        if(UNI_WarnReg.bit.EMSTOPWARNFLG == 0) PostErrMsg(EMERGENCYSTOPWARN);  //报出紧急停机警告,并进行处理
	    }
	}
    else if((EmergencyStopDiLast == 1)&&(STR_FUNC_Gvar.DivarRegHi.bit.EmergencyStop == 0)&&(DeviceControlVar.StatusStep != STATUS_OPERENABLE))  //下降沿时紧急停机解除
    {

        STR_ServoMonitor.StopCtrlFlag.bit.EMSClampFlg = 0;
		STR_ServoMonitor.StopCtrlFlag.bit.EMSClrReguFlg = 0;

        WarnAutoClr(EMERGENCYSTOPWARN);  //清除紧急停机警告

        //防止位置环下位置偏差不清零出现执行累计脉冲的函数，一直清除脉冲偏差，
        //防止紧急停机后，仍有位置指令时报ER.b00
		//不能设置清速度调节器标志位有效
        STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;
    }

    EmergencyStopDiLast = STR_FUNC_Gvar.DivarRegHi.bit.EmergencyStop;
}

/*******************************************************************************
  函数名: Static_Inline void OverTravelMonitor() 
  输入:   STR_FUNC_Gvar.DivarRegLw.bit.Pot; STR_FUNC_Gvar.DivarRegLw.bit.Not (超程Di使能信号)
          STR_ServoMonitor.RunStateFlag.bit.RefDir; (电机旋转方向标志位)
  输出:   STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck (超程停机响应标志位)
  子函数:   
  描述:  对是否发生超程进行监控，当有正反向超程Di信号时进行报警处理
         当发生正反向超程时，则使能超程停机响应标志位，当超程解除时，
         则清除超程时的调节器偏差,开启PWM波。
         (注明；相比#112V1.20，将速度模式下JOG使能时，禁止超程给取消了)       
********************************************************************************/ 
Static_Inline void OverTravelMonitor()
{
    static Uint8  ExecOnceFlag = 0;   //超程执行一次标志位
    static Uint8  POTWarnOnceFlag = 0;   //正向超程警告一次标志位
    static Uint8  NOTWarnOnceFlag = 0;   //反向超程警告一次标志位

    static Uint16  OTDirLatch = 0;
    static int32  OTPostionLatch = 0;

    static Uint32  OTPosLatchThreshold = 0;


    /* 以下是先进行超程方向检测*/
    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD)  //转矩控制时的转矩指令方向
    {
        if(STR_FUNC_Gvar.ToqCtrl.ToqCmdLatch >=0)
        { 
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 0;  //正方向标志位
        }
        else if(STR_FUNC_Gvar.ToqCtrl.ToqCmdLatch < 0)
        {
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 1;  //负方向标志位
        }
    }
    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == SPDMOD) //速度控制时的速度指令方向
    {
        if (STR_FUNC_Gvar.SpdCtrl.SpdRefLatch >= 0)
        {
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 0;   //正转标志
        }
        else if(STR_FUNC_Gvar.SpdCtrl.SpdRefLatch < 0)
        {
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 1;   //反转标志
        }
    }
    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod  == POSMOD)    //位置控制时的位置指令方向
    {
        if (STR_FUNC_Gvar.PosCtrl.PosRefLatch > 0) 
        {
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 0;   //正转标志 
        }
        else if(STR_FUNC_Gvar.PosCtrl.PosRefLatch < 0)
        {
            STR_ServoMonitor.RunStateFlag.bit.RefDir = 1;   //反转标志 
        }
    }
    
    //如果超程停机标志位无效 ,复位OverTravelStopDeal()内部两个标志位
    if(0 == STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck)
    {
        ExecOnceFlag = 0;
    }
    //在故障复位后，要把这些标志位清零，保证故障复位后，仍处于超程情况下，面板仍能报出Er950/952
    if (UNI_WarnReg.bit.NOTWARNFLG == 0)
    {
        NOTWarnOnceFlag = 0;
    }
    if (UNI_WarnReg.bit.PNOTWARNFLG == 0)
    {
        POTWarnOnceFlag = 0;
    }

    /* 当Di有超程发生信号时，就需要进行超程警告处理，与当前的转速方向无关 */
    if( (STR_FUNC_Gvar.DivarRegLw.bit.Pot==1)||(STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) )
    {
            //正向限位(作为原点)且正向回零中 不报警           
            if(((STR_FUNC_Gvar.DivarRegLw.bit.Pot==1)&&(POTWarnOnceFlag == 0) )               
                &&(STR_FUNC_Gvar.PosCtrl.HomStats != 1)) 
            {
                PostErrMsg(PNOTWARN);         //报出正向超程警告故障,并进行处理；
                POTWarnOnceFlag = 1;             //清除超程报警一次标志位
            }
            else if( (STR_FUNC_Gvar.DivarRegLw.bit.Pot==0)&&(STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) )
            {
                WarnAutoClr(PNOTWARN);        //清除正向超程警告
                POTWarnOnceFlag = 0;             //清除超程报警一次标志位
            }
            
            //负向限位(作为原点)且负向回零中 不报警              
            if(((STR_FUNC_Gvar.DivarRegLw.bit.Not == 1)&&(NOTWarnOnceFlag == 0) )
                &&(STR_FUNC_Gvar.PosCtrl.HomStats != 2)) 
            {
                PostErrMsg(NOTWARN);         //报出负向超程警告故障,并进行处理；
                NOTWarnOnceFlag = 1;                //清除超程报警一次标志位
            }
            else if( (STR_FUNC_Gvar.DivarRegLw.bit.Pot==1)&&(STR_FUNC_Gvar.DivarRegLw.bit.Not == 0) )
            {
                WarnAutoClr(NOTWARN);   //清除反向超程警告
                NOTWarnOnceFlag = 0;                //清除超程报警一次标志位
        	}

    }
    else
    {
        if (FunCodeUnion.code.BP_OTStopAuxFun == 1)       //启用超程停机特殊处理方式
        {
            if  ((POTWarnOnceFlag == 1)&&
                 (OTDirLatch == 0)&&
                 (STR_ServoMonitor.RunStateFlag.bit.RefDir == 0)&&
                 (OTPostionLatch < UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue))
            {
                PostErrMsg(PNOTWARN);         //报出正向超程警告故障,并进行处理；
                POTWarnOnceFlag = 1;             //清除超程报警一次标志位
            }
            else if ((NOTWarnOnceFlag == 1)&&
                     (OTDirLatch == 1)&&
                     (STR_ServoMonitor.RunStateFlag.bit.RefDir == 1)&&
                     (OTPostionLatch > UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue))
            {
                PostErrMsg(NOTWARN);         //报出负向超程警告故障,并进行处理；
                NOTWarnOnceFlag = 1;                //清除超程报警一次标志位
            }
            else
            {
                WarnAutoClr(PNOTWARN);  //清除正向超程警告
                WarnAutoClr(NOTWARN);   //清除反向超程警告
                POTWarnOnceFlag = 0;
                NOTWarnOnceFlag = 0;
            }
        }
        else        //未启用超程停机特殊处理方式
        {
            WarnAutoClr(PNOTWARN);  //清除正向超程警告
            WarnAutoClr(NOTWARN);   //清除反向超程警告
            POTWarnOnceFlag = 0;
            NOTWarnOnceFlag = 0;
        }
    }


    /* 根据指令方向：正转时且正超程有效，置超程相应标志位有效；   反转时且反超程有效，置超程相应标志位有效。*/
    if( ((STR_ServoMonitor.RunStateFlag.bit.RefDir == 0) && (STR_FUNC_Gvar.DivarRegLw.bit.Pot == VALID))
        ||((STR_ServoMonitor.RunStateFlag.bit.RefDir == 1) && (STR_FUNC_Gvar.DivarRegLw.bit.Not == VALID))
      )
    {
//        //Soff时，待电机转速小于100转后切换到Soff停机处理
//        if(RUN != STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)         
//        {
//            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;
//            STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = 0;
//
//            STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 0;
//
//            ExecOnceFlag = 0;
//        }
//        else 
        if(((STR_FUNC_Gvar.PosCtrl.HomStats == 1)
            && (STR_FUNC_Gvar.DivarRegLw.bit.Pot == VALID))
            ||((STR_FUNC_Gvar.PosCtrl.HomStats == 2)
            && (STR_FUNC_Gvar.DivarRegLw.bit.Not == VALID))  )
            //正向限位为原点且正向回零中，反向限位为原点且反向回零中不进行限位处理
        {
            //上述两种状态不超程停机
        }
        else
        {
            if (FunCodeUnion.code.BP_OTStopAuxFun == 1)       //启用超程停机特殊处理方式
            {
                if (ExecOnceFlag == 0)
                {
                    OTPostionLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
                    OTDirLatch = STR_ServoMonitor.RunStateFlag.bit.RefDir;
    
                    if (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd) //当前的速度小于100rpm时才去进行锁存值处理
                    {
    
                        OTPosLatchThreshold = (UNI_FUNC_MTRToFUNC_InitList.List.EncRev / 1024L);   //按照定位完成时默认位置偏差数量级来定补偿值量
    
                        if (STR_FUNC_Gvar.DivarRegLw.bit.Pot == VALID)
                        {
                            OTPostionLatch = (OTPostionLatch + OTPosLatchThreshold);
                        }
                        else if (STR_FUNC_Gvar.DivarRegLw.bit.Not == VALID)
                        {
                            OTPostionLatch = (OTPostionLatch - OTPosLatchThreshold);
                        }
                    }
                }
            }

            //相比#112原点复归或者JOG点动模式时,超程监控也起作用
            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = VALID;   //执行超程响应
            STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = VALID;
            ExecOnceFlag = 1;

            if (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR)  //确保故障和超程同时发生时，故障停机之后，Di刹车停机响应标志位无效
            {
                STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = INVALID;   //不执行超程响应
                STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = INVALID;    //不执行超程响应
            }
            
        }
    }
    /* 若超程已经有效时，再向相反方向运行时，超程故障停机解除，响应标志位清零，同时调节器变量也应清除一次. (REFDIR 0-正方向，1-负方向) */
    else if(  ((STR_ServoMonitor.RunStateFlag.bit.RefDir == 1)&&(STR_FUNC_Gvar.DivarRegLw.bit.Pot == 1)) ||
              ((STR_ServoMonitor.RunStateFlag.bit.RefDir == 0)&&(STR_FUNC_Gvar.DivarRegLw.bit.Not == 1))
           ) // 正向超程时，反转；反向超程时，正转时，则解除超程停机
    {

        STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;  //清除超程响应标志位
        STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = 0;
        STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;

        STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 0;
        
        if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(ExecOnceFlag == 1))
        {
             ExecOnceFlag = 0;

             //防止在超程停机方式过程中，超程Di信号瞬间变化为无效，
             //此时超程停机函数不执行，从而急转矩停机标志位没有清零，因而一直处于急转矩停机过程中！
             STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;

             STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

             //STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
             //STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;//防止位置环下位置偏差不清零出现执行累计脉冲的函数。 
             STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;

             #if GDCHECK
             DELAY_US(1);
             #endif                            
             WriteGDStop(DISGD);   //解除动态制动
        }
    }
    else if( ( STR_FUNC_Gvar.DivarRegLw.bit.Pot == 0) && (STR_FUNC_Gvar.DivarRegLw.bit.Not == 0 ) )
    {

        if (FunCodeUnion.code.BP_OTStopAuxFun == 1)       //启用超程停机特殊处理方式
        {
            if (ExecOnceFlag == 0)
            {
                STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;  //清除超程响应标志位
                STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = 0;
                STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;

                STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 0;
            }
            else
            {
    
                 if( ((OTDirLatch == 0)&&
                     (STR_ServoMonitor.RunStateFlag.bit.RefDir == 0)&&
                     (OTPostionLatch < UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue)) ||
                     ((OTDirLatch == 1)&&
                     (STR_ServoMonitor.RunStateFlag.bit.RefDir == 1)&&
                     (OTPostionLatch > UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue))
                   )
                 {
                     STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 1;
                     return;
                 }
                 else
                 {
                     STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;  //清除超程响应标志位
                     STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = 0;
                     STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;
    
                     STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 0;

                     if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(ExecOnceFlag == 1))
                     {
                         ExecOnceFlag = 0;
    
                         //防止在超程停机方式过程中，超程Di信号瞬间变化为无效，
                         //此时超程停机函数不执行，从而急转矩停机标志位没有清零，因而一直处于急转矩停机过程中！
                         STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
    
                         STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
    
                         //STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
                         //STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;//防止位置环下位置偏差不清零出现执行累计脉冲的函数。 
                         STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;
    
                         #if GDCHECK
                         DELAY_US(1);
                         #endif
                         WriteGDStop(DISGD);   //解除动态制动
                     }
                 }
            }
        }
        else
        {
            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;  //清除超程响应标志位
            STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;

            STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos = 0;

            if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(ExecOnceFlag == 1))
            {
                ExecOnceFlag = 0;

                //防止在超程停机方式过程中，超程Di信号瞬间变化为无效，
                //此时超程停机函数不执行，从而急转矩停机标志位没有清零，因而一直处于急转矩停机过程中！
                STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;

                STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

                //STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
                //STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1;//防止位置环下位置偏差不清零出现执行累计脉冲的函数。 
                STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;

                #if GDCHECK
                DELAY_US(1);
                #endif                            
                WriteGDStop(DISGD);   //解除动态制动
            }
        }
    }
}

/*******************************************************************************
  函数名: 
  输入:  
  输出:  
  子函数:    
  描述: GD电路失效检测,硬件不支持，暂时先不采用该功能，因此该函数为空
********************************************************************************/ 
//GD电路失效检测,硬件不支持，暂时先不采用该功能，因此该函数为空
/*Static_Inline void GDMonitor()
{


}
*/
/*******************************************************************************
  函数名: void RunStatusMonitor()
  输入:  
  输出:  
  子函数:    
  描述: 伺服运行状态监控，包含正常运行时的状态切换和故障时的运行状态切换
********************************************************************************/ 
Static_Inline void RunStatusMonitor()
{
    static Uint8 SonInhibit  = 0;

    //第一次SON需要初始角辨识后延时5s 再SON
    if( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)      
     || ((FunCodeUnion.code.MT_EncoderSel & 0x0ff) == 0x02) )
    { 
        if((STR_ServoMonitor.SonDelayCnt < 20) && (1 == STR_ServoMonitor.RunStateFlag.bit.FstSonFlag))
        {
            STR_ServoMonitor.SonDelayCnt ++;
        }
    }
    else
    {
        STR_ServoMonitor.SonDelayCnt = 20;
    }

    //UV相电流的校正时或校正后不允许Son wzg1470
    if(STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn == 1) SonInhibit = 1;

    //参数自调谐期间或UV相电流校正或绝对编码器操作时，不能DI端子SON  wzg1470
	#if ECT_ENABLE_SWITCH
    if( ( (1 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) 
         || (1 == AuxFunCodeUnion.code.OEM_ResetABSTheta) )
      && ((STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID)||(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon == 1)) )
    {
        //第一次SON需要初始角辨识时 不报错       
        if( ( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)       //直线电机第一次使能时不报警
           || ((FunCodeUnion.code.MT_EncoderSel & 0x0ff) == 0x02) )
         && (STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0) )
        {
        }
        else 
        {
            PostErrMsg(SONVALIDERR);
        }
    }
    #else
    if( ( (1 == STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn) 
         || (1 == AuxFunCodeUnion.code.OEM_ResetABSTheta) )
      &&(STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID) )
    {
        //第一次SON需要初始角辨识时 不报错       
        if( ( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)       //直线电机第一次使能时不报警
           || ((FunCodeUnion.code.MT_EncoderSel & 0x0ff) == 0x02) )
         && (STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0) )
        {
        }
        else 
        {
            PostErrMsg(SONVALIDERR);
        }
    }
	#endif
    
    if(STR_ServoMonitor.RunStateFlag.bit.InnerServoSon == 1)
    {
		#if ECT_ENABLE_SWITCH
		if(((STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID)||(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon == 1)) 
           && (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))       //必须保证已经进入使能状态才进行判定
        {   
            //第一次SON需要初始角辨识时 不报错        
            if( ( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)       //直线电机第一次使能时不报警
               || ((FunCodeUnion.code.MT_EncoderSel & 0x0ff) == 0x02) )
             && (STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0) )
            {
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;
            }
            else
            {
                PostErrMsg(SONVALIDERR);
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
            }
        }
        else
        {
            STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;
        }
		#else
		if((STR_FUNC_Gvar.DiDoOutput.Son_Flt == VALID) 
           && (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))       //必须保证已经进入使能状态才进行判定
        {   
            //第一次SON需要初始角辨识时 不报错        
            if( ( ((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)       //直线电机第一次使能时不报警
               || ((FunCodeUnion.code.MT_EncoderSel & 0x0ff) == 0x02) )
             && (STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0) )
            {
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;
            }
            else
            {
                PostErrMsg(SONVALIDERR);
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
            }
        }
        else
        {
            STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;
        }
		#endif
    }
    else 
    {
        if(STR_ServoMonitor.SonDelayCnt >= 20) 
        {
			#if ECT_ENABLE_SWITCH
			if((STR_FUNC_Gvar.DiDoOutput.Son_Flt==1)&&(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon==1))
            {
                PostErrMsg(SONVALIDERR);
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
            }         
			else if((STR_FUNC_Gvar.DiDoOutput.Son_Flt==1)||(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon==1))
			{
				STR_ServoMonitor.RunStateFlag.bit.ServoOn = 1;
			}
			else
			{
				STR_ServoMonitor.RunStateFlag.bit.ServoOn = 0;
			}
			#else
			STR_ServoMonitor.RunStateFlag.bit.ServoOn = STR_FUNC_Gvar.DiDoOutput.Son_Flt;
			#endif
		}
        else
        {
            STR_ServoMonitor.RunStateFlag.bit.ServoOn = 0;
        }
    }

    //故障时内部Son清零
    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR)
    {
        STR_ServoMonitor.RunStateFlag.bit.InnerServoSon = INVALID;
		#if ECT_ENABLE_SWITCH
        STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon = INVALID;
		#endif
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    //判断伺服是否准备好，伺服尚未准备好时son均为无效 
    if(STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 0) 
    //此处原zyj1101 更改下电检测到三相驱动电均断线后不允许Son,后李飞建议下电检测到三相驱动电均断线后立即进入RDY 
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    if(STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk == 0)
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    //掉电时零速停机标志位有效时
    if(1 == STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag)
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }
     
    //紧急停机(仿紧急停机按钮)功能，当紧急停机功能有效时，进行伺服Off停机处理
    if((SonInhibit == 1) || (AuxFunCodeUnion.code.OS_CommDicnctStop==1))
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    /* 下面进行抱闸操作 */
    BrakeOperate();  

    #if ECT_ENABLE_SWITCH
    ComControlBK();
    #endif


    //判断伺服是否准备好，son无效 
    if(STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 0) 
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    //解决因为欠压继电器打开FPGA关闭PWM，但是FUNC模块还未关闭PWM，从而速度调节器还继续工作的Bug
    if(STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg == 1 )
    {
        STR_ServoMonitor.RunStateFlag.bit.ServoOn = INVALID;
    }

    /* 抱闸操作结束 */
    STR_ServoMonitor.RunStateFlag.bit.ServoEnable = (STR_ServoMonitor.RunStateFlag.bit.ServoOnLast<<1) + STR_ServoMonitor.RunStateFlag.bit.ServoOn;

    STR_ServoMonitor.RunStateFlag.bit.ServoOnLast = STR_ServoMonitor.RunStateFlag.bit.ServoOn;    

    if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd) //100.0000rpm
    {
        STR_ServoMonitor.RunStateFlag.bit.ZeroSpdFlag = 1;
    }
    else           //if (FunCodeUnion.code.BP_StopNo1 == 1)
    {
        STR_ServoMonitor.RunStateFlag.bit.ZeroSpdFlag = 0;
    }

    if( (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 1 )&& 
        ((STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0)||(STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000))
      )  //当伺服Read=1准备好且无故障时，进行伺服运行状态切换监控;
    {
        RunAwayMointor(); //飞车故障监控

        NormalRunStateSwitch();   //正常运行时伺服状态的切换  
        
        STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus = ServoRunStatusUpdate;   //全局调用的运行状态标志位进行跟新
    }
    else
    {
        ErrorRunStateSwitch();    //发生故障时伺服状态的切换

        STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus = ServoRunStatusUpdate;   //全局调用的运行状态标志位进行跟新
    }

}


/*******************************************************************************
  函数名: void BrakeOperate()
  输入:   ServoOn;UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;STR_FUNC_Gvar.Monitor.HighLevelErrCode 
  输出:   STR_FUNC_Gvar.Monitor.DovarReg_Blk
  子函数: EnableDriver(DISPWM)       
  描述:  Do输出为1有效 -> 机械抱闸器通电 -> 抱闸打开/释放 (电机可旋转)
         Do输出为0无效 -> 机械抱闸器不通电 -> 抱闸使能/锁死 (电机不可旋转)
********************************************************************************/ 
Static_Inline void BrakeOperate()
{
    static Uint16 BrakeZSCnt = 0;       //停止时，或零速停机后的延时  BlkEnCnt
    static Uint16 BrakeTRCnt = 0;       //旋转时，停机延时计数        BlkTDCnt
    static Uint16 BrakeTRCnt2 = 0;      //旋转时，H0210判断时间处理计时
    static Uint16 BrakeUnlockCnt = 0;   //伺服ON时抱闸打开，不接收指令的过程，延时计数  
    static Uint8  SonBrkRefDelayFlg = 0;  //伺服on抱闸打开指令接收延时
    static Uint16 BrakeUnlockCnt2 = 0;
    static Uint8  H0209_EndFlg = 0;       //H0209结束标志位

    static Uint8  BrakeServoOnLast = 0;	  //锁存Son信号，SOFF抱闸处理使用
    Uint8  ServoState = 0;       //抱闸函数内使用的伺服使能状态中间变量  -》SvEn_m

    /* --抱闸工作方式-- */
    //Do输出为1有效 -> 机械抱闸器通电 -> 抱闸打开/释放 (电机可旋转)
    //Do输出为0无效 -> 机械抱闸器不通电 -> 抱闸使能/锁死 (电机不可旋转)

    /* 以下为伺服On，抱闸器通电，抱闸打开，之后电机可旋转时，抱闸操作处理 */
    if(STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 1)  //只有在Do分配了抱闸功能后，才进行抱闸操作处理
    {
        if( (STR_ServoMonitor.RunStateFlag.bit.ServoOn == VALID) && 
            ((STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0) || (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000))
          )
        {
            //---------------------------//
            if (H0209_EndFlg == 0)
            {
                STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock = 1;  // 在延时期间不接收位置/速度/转矩指令,将其相应标志位置1
            }
            //----------------------------//
            if(BrakeUnlockCnt >= FunCodeUnion.code.BP_SonBrkDelayTime)
            {
                 STR_FUNC_Gvar.Monitor.DovarReg_Blk = VALID;   //在无故障且伺服On同时，打开抱闸电源，让抱闸释放，
                 SonBrkRefDelayFlg = 1;
                 BrakeUnlockCnt = 2000;
                 STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver = 1;  //表明此时抱闸松开
            }
            else
            {
                BrakeUnlockCnt ++;      //BrakeUnlockCnt延时标志
            }
            //-----------------------------//
            if (SonBrkRefDelayFlg == 1)
            {
                if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd)   //100.0000rpm
                {
                    if(BrakeUnlockCnt2 <= FunCodeUnion.code.BP_SonBrkDelay)   //延时固定时间200ms，
                    {
                        BrakeUnlockCnt2++;
                    }
                    else
                    {
                        STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock = 0;  //200ms延时完后，开始接收指令，此时将其相应标志位清零
                        BrakeUnlockCnt2 = 1000;   //H0209最大值的2倍，确保当H0209在运行设定中由小值改到大值时再一次执行不接受指令的处理
                        H0209_EndFlg = 1;
                    }
                }
                else
                {
                    BrakeUnlockCnt2 = 1000;
                    STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock = 0;  //200ms延时完后，开始接收指令，此时将其相应标志位清零
                    H0209_EndFlg = 1;
                }
            }
        }
        else
        {
            BrakeUnlockCnt = 0;
            BrakeUnlockCnt2 = 0;

            SonBrkRefDelayFlg = 0;
            H0209_EndFlg = 0;
        }

        /* 以下为伺服Off或故障时，抱闸器掉电，抱闸使能锁死，之后电机不可旋转时，抱闸操作处理 */
        // 此时的抱闸使能锁死处理，分为两种状态，分别处理，即：状态1 原为接近零速停机时的状态；状态2 原为旋转时的状态 
        // 伺服Off时，根据状态1或状态2，分别进行处理
        // 故障时，抱闸使能分两种情况处理：情况1：立即关断PWM ； 情况2：零速停机
        // 我司程序中，故障1时无零速停机方式，故障2时有零速停机方式，
        // 情况1无零速停机要求时，抱闸处理：处理方式则和伺服Off时，且从旋转到停止时的处理方式一样 
        // 情况2有零速停机要求时，抱闸处理：处理方式是，先以零速停机，然后和伺服off时，且原状态为接近零速停机时的处理方式一样 
        // 伺服是否使能的中间状态变量ServoState； 当 ServoState=2 时，表明由伺服On变化到伺服Off ；当 ServoState=1 时，表明由伺服Off变化到伺服On ；
        ServoState = (BrakeServoOnLast << 1) + STR_ServoMonitor.RunStateFlag.bit.ServoOn;  //当 ServoState=2 时，表明由伺服On变化到伺服Off ；当 ServoState=1 时，表明由伺服Off变化到伺服On ；
        BrakeServoOnLast = STR_ServoMonitor.RunStateFlag.bit.ServoOn;
		 
        //伺服OFF时且转速低于100rpm(接近零速)时的，输出抱闸器电源使能信号，并置位零速时抱闸使能处理标志位                      
        //伺服故障发生且零速停机作用后，转速低于100rpm后时，抱闸使能锁死生效，并置位零速时抱闸使能处理标志位
        if( ((ServoState == 2)||(STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrake == 1)) && 
            (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < BK_OFF_STATE_SPD)      //20.0000rpm
          )
        {
            STR_FUNC_Gvar.Monitor.DovarReg_Blk = INVALID;         //抱闸生效
            STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake = 1;  //零速抱闸延时标志使能
        }
        //伺服OFF时且电机为旋转转速大于100rpm时，不输出抱闸器电源使能信号，并置位旋转时抱闸使能处理标志位
        //在故障1刚发生时，不输出抱闸器电源使能信号，并置位旋转时抱闸使能处理标志位 .(故障1无零速停机方式)
        //在故障2刚发生且非零速停机方式时，不输出抱闸器电源使能信号，并置位旋转时抱闸使能处理标志位 .
        else if( ((ServoState == 2) && (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= BK_OFF_STATE_SPD)) ||
                 ((STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0x6000)) ||
                 ((STR_FUNC_Gvar.Monitor.HighLevelErrCode >= 0x6000) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0xE000))//&& (STR_ServoMonitor.StopCtrlVar.Err2StopMode != 2))
               )
        {
            STR_ServoMonitor.BrakeFlag.bit.TravelBrake = 1;
        }

                 /*上面程序是产生两种抱闸处理状态标志位*/
        /* ----------------------------------------------------- */
                /*下面程序是根据两种状态标志位进行相应操作*/

        //旋转时伺服OFF，或者故障停机(立即关PWM的状态)的抱闸使能处理	
        if(STR_ServoMonitor.BrakeFlag.bit.TravelBrake == 1)
        {            
	        if( (STR_ServoMonitor.RunStateFlag.bit.ServoOn == VALID) && 
	            ((STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0) || (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000))
	          )
			{
				//重新SON				
				STR_ServoMonitor.BrakeFlag.bit.TravelBrake = 0;
				STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
				BrakeTRCnt = 0;
				BrakeTRCnt2 = 0;
				if(ServoState == 1)  //导入解决配抱闸复位故障后50ms内使能不出力
					STR_ServoMonitor.BrakeFlag.bit.ErrResetPWMOn = 1;			
			}
			//若原状态为旋转时，则在H0212延迟时间内 或 转速大于H0211时，输出抱闸器电源使能信号，让其按原停机方式进行停机
            //若原状态为旋转时，则延迟时间大于H0212 或 转速小于H0211时，不输出抱闸器电源使能信号
            else if( (BrakeTRCnt > FunCodeUnion.code.BP_Off2ClaspTime) || 
                (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < ((int32)FunCodeUnion.code.BP_ClaspSpdLmt * 10000L))
              )
             {
                 if (BrakeTRCnt2 > 50)
                 {
                     STR_ServoMonitor.BrakeFlag.bit.TravelBrake = 0;   //清除旋转时停机抱闸标志位，执行一次
                     BrakeTRCnt = 0;
                     BrakeTRCnt2 = 0;
                     //STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
                     STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver = 0;  //此时抱闸锁死
					 if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= ((int32)FunCodeUnion.code.BP_ClaspSpdLmt * 10000L))
                     	STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                 }
                 else
                 {
                     STR_FUNC_Gvar.Monitor.DovarReg_Blk = INVALID; //抱闸生效
                     BrakeTRCnt2 ++;
                     STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;
                 }                 
             }
             else
             {
                  BrakeTRCnt ++;
                  STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;  // 此处的ServoOn=1，是为了保证在延迟时间位达到时，仍能使STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake 置1，进入该零速停机状态
                  STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 1;   //置零速停机标志位，保持零速状态
             }
        }
		else
		{
             BrakeTRCnt = 0;
             BrakeTRCnt2 = 0;
		}

        //速度接近零速时伺服OFF，或者故障时为零速停机时的抱闸使能处理
        if(STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake == 1)
        {
	        if( (STR_ServoMonitor.RunStateFlag.bit.ServoOn == VALID) && 
	            ((STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0) || (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000))
	          )
			{
				//重新SON            
                STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0; 
                STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake = 0; 
                BrakeZSCnt = 0;	
			}
			else if(BrakeZSCnt < FunCodeUnion.code.BP_Clasp2OffTime)  //虽然抱闸器电源不通电，但抱闸接触端抱闸锁死有一定延时，因此在H0210延时时间内为零速停机状态。
            {
                BrakeZSCnt ++;
                STR_ServoMonitor.RunStateFlag.bit.ServoOn = VALID;  // 此处的ServoOn=1，是为了保证在延迟时间位达到时，仍能使STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake 置1，进入该零速停机状态
                STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 1;   //置零速停机标志位，保持零速状态
            }
            else  //若延时时间超过设定值时，则清除相应标志位
            {
                if(STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrake == 1)  //若为故障2零速停机方式，且在延时时间超过设定值之后，则应关断PWM
                {
                    STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrake = 0;
                    STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrakeOver = 1;
                    STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                }

                STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0; 
                STR_ServoMonitor.BrakeFlag.bit.ZeroSpdBrake = 0; //伺服OFF关断PWM是在后面处理
                BrakeZSCnt = 0;
                
                STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver = 0;  //此时抱闸锁死
            }
        }
		else
		{
			BrakeZSCnt = 0;		
		}
	}		
    else
    {
        STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver = 0;  //此时抱闸锁死
    }
}

/*******************************************************************************
  函数名: Static_Inline void RunAwayMointor()
  输入:  UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb；UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb；
  输出:  是否飞车故障处理
  子函数: PostErrMsg(UVW_FEICHE)      
  描述: 飞车监控，当发生飞车时，进行飞车故障处理 
********************************************************************************/ 
Static_Inline void RunAwayMointor()
{
    static Uint8 RunAwayFlag = 0;       //飞车是否发生标志位，若为1表明飞车发生，此时只能重新上电后，才能进行飞车监控处理
    static Uint8 RunAwayCnt_T = 0;      //转矩模式下飞车时延时计数值
    static Uint16 RunAwayCnt_PV = 0;    //位置或速度模式下飞车时延时计数值
    static Uint16 RunAwayCnt_PT = 0;    //转矩摸下伪速度调节器模式下飞车时延时计数值
    static int64 RunAwayToqFdb = 0;     //飞车时Q轴电流累加和
    static int64 RunAwaySpdFdb = 0;     //飞车时速度反馈累加和

    static int32 RunAwaySpdFdbLatch = 0;    //飞车速度判断时锁存上次的速度反馈信息
    static Uint8 RunAwayInit = 0;

    int32  RunAwayAccSpdValue = 0;          //飞车速度判断时速度增量

    if(0 == RunAwayInit )
    {
        RunAwayInit = 1;
    }

    if((1 == AuxFunCodeUnion.code.OEM_ResetABSTheta ) ||
       (1 == STR_FUNC_Gvar.MonitorFlag.bit.ToqStop )||
//	   (((STR_FUNC_Gvar.Monitor.DovarReg_Blk==0)||(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck==VALID)||(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck==VALID))&&
	   (((STR_FUNC_Gvar.Monitor.DovarReg_Blk==0)||(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck==VALID))&&
	   (STR_FUNC_Gvar.MonitorFlag.bit.RunMod==TOQMOD)&&(STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 1))
      )//角度辨识的时候不作飞车判断 超程时急转矩停机阶段也不进行飞车监控
    {
        RunAwayFlag = 0;
        RunAwayCnt_T = 0;
        RunAwayCnt_PV = 0;
        RunAwayToqFdb = 0;
        RunAwaySpdFdb = 0;

        RunAwaySpdFdbLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
        return;
    }
    
    if((STR_FUNC_Gvar.MonitorFlag.bit.RunMod==TOQMOD)&&(STR_FUNC_Gvar.ToqCtrl.DovarReg_Vlt==0))
    {
        RunAwayCnt_PV = 0;
        RunAwayCnt_PT = 0;

        //在伺服On，功能码使能了飞车监控且未发生过飞车，速度大于50转时，才进行飞车监控处理
        if( (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
         && (FunCodeUnion.code.ER_RunAwaySel)
         && (RunAwayFlag == 0)         
         && (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > 500000L) ) //50.0000rpm
        {
            RunAwayCnt_T++;

            RunAwayToqFdb += (int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb;
            RunAwaySpdFdb += (int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;

            if(RunAwayCnt_T > RunAwayDELAYTIME) 
            {
                if(   (RunAwayToqFdb > (int64)(1000 >> 3))&&((RunAwaySpdFdb +(int64)1000000) < 0)
                    ||((RunAwayToqFdb + (int64)(1000 >> 3))<0)&&(RunAwaySpdFdb >(int64)1000000)
                  )
                {
                    RunAwayFlag = 1;
                    PostErrMsg(UVW_FEICHE);
                }
                else
                {
                    RunAwayToqFdb = 0;   //累加和要清零
                    RunAwaySpdFdb = 0;
                    RunAwayCnt_T = 0;    //计数值也要清零
                }
            }
        }
        else
        {
            RunAwayToqFdb = 0;   //累加和要清零
            RunAwaySpdFdb = 0;
            RunAwayCnt_T = 0;    //计数值也要清零
        }
    }
    else if((STR_FUNC_Gvar.MonitorFlag.bit.RunMod==TOQMOD)&&(STR_FUNC_Gvar.ToqCtrl.DovarReg_Vlt==1))
    {
        RunAwayCnt_T = 0;
        RunAwayToqFdb = 0;   //累加和要清零
        RunAwaySpdFdb = 0;
        RunAwayCnt_PV = 0;

        RunAwayAccSpdValue = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb - RunAwaySpdFdbLatch;

        if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(FunCodeUnion.code.ER_RunAwaySel == 1))
        {

            if((STR_FUNC_Gvar.ToqCtrl.ToqCmd >= 0) 
               && (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb < (-5000000L))) 
            {
                if(RunAwayAccSpdValue > 0L)
                {
                    RunAwayCnt_PT = 0;
                }
                else if(RunAwayAccSpdValue < 0L)
                {
                    RunAwayCnt_PT++;
                }
            }
            else if( (STR_FUNC_Gvar.ToqCtrl.ToqCmd <= 0) 
                  && (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb > 5000000L) )
            {
                if(RunAwayAccSpdValue < 0L)
                {
                    RunAwayCnt_PT = 0;
                }
                else if(RunAwayAccSpdValue > 0L)
                {
                    RunAwayCnt_PT++;
                }
            }
            else
            {
                 RunAwayCnt_PT = 0;
            }
            if(RunAwayCnt_PT > 15)
            {
                PostErrMsg(UVW_FEICHE);
                RunAwayCnt_PT = 17;
            }
        }
    }
    else
    {
        RunAwayCnt_T = 0;
        RunAwayToqFdb = 0;   //累加和要清零
        RunAwaySpdFdb = 0;
        RunAwayCnt_PT = 0;

        RunAwayAccSpdValue = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb - RunAwaySpdFdbLatch;

        if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(FunCodeUnion.code.ER_RunAwaySel == 1))
        {

            if((STR_FUNC_Gvar.SpdCtrl.SpdRef >= 0) 
               && (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb < (-5000000L))) 
            {
                if(RunAwayAccSpdValue > 0L)
                {
                    RunAwayCnt_PV = 0;
                }
                else if(RunAwayAccSpdValue < 0L)
                {
                    RunAwayCnt_PV++;
                }
            }
            else if( (STR_FUNC_Gvar.SpdCtrl.SpdRef <= 0) 
                  && (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb > 5000000L) )
            {
                if(RunAwayAccSpdValue < 0L)
                {
                    RunAwayCnt_PV = 0;
                }
                else if(RunAwayAccSpdValue > 0L)
                {
                    RunAwayCnt_PV++;
                }
            }
            else
            {
                 RunAwayCnt_PV = 0;
            }
            if(RunAwayCnt_PV > 15)
            {
                PostErrMsg(UVW_FEICHE);
                RunAwayCnt_PV = 17;
            }
        }
    }
    RunAwaySpdFdbLatch = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb;
}

/*******************************************************************************
  函数名: 
  输入:  
  输出:  
  子函数:    
  描述: 伺服运行状态监控，包含正常运行时的状态切换和故障时的运行状态切换
********************************************************************************/ 
Static_Inline void NormalRunStateSwitch()
{
    switch(STR_ServoMonitor.RunStateFlag.bit.ServoEnable)   //状态切换：
    {
        case 0:       
            if(STR_ServoMonitor.RunStateFlag.bit.ZeroSpdFlag == 1)    //使能禁止
            {               
                 ServoRunStatusUpdate = RDY;
            }
            else
            {
                 if (   (STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop == 1)&&(STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == ENPWM))
                 {
                     ServoRunStatusUpdate = RUN;
                 }
                 else
                 {
                     ServoRunStatusUpdate = RDY;
                 }
            }
            break; 

        case 1:      //禁止--使能          	   	
            STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck = INVALID; 
            
            if(STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck == 1)   //故障1停机处理
            {
                //故障1的相关清零
                STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck = 0;
                STR_ServoMonitor.StopCtrlFlag.bit.FirErr1Stop = 0;
            }
 
            if(STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck == 1)   //故障2停机处理
            {
                if((STR_ServoMonitor.StopCtrlFlag.bit.FirErr2Stop == 1)&&(STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop == 1))
                {
                    STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
                    STR_ServoMonitor.StopCtrlFlag.bit.FirErr2Stop = 0;
                }
                STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck = 0;
            }
            
            if ((STR_FUNC_Gvar.MonitorFlag.bit.OTAckForLockPos == 0))  //解决超程停机时，选择0和2停机方式时，发生超程，关伺服后，位置不动，再开伺服仍能运行的Bug !
            {
                STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;
                STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

                ServoRunStatusUpdate = RUN;
            }

            WriteGDStop(DISGD);

            #if GDCHECK
            DELAY_US(1);
            #endif 


            break;
                
        case 2:      //使能--禁止  
            STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck = VALID;
            ServoRunStatusUpdate = RUN;
            break;

        case 3:     //使能状态
            ServoRunStatusUpdate = RUN;	
            STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck = INVALID;
            if(STR_ServoMonitor.BrakeFlag.bit.ErrResetPWMOn == 1 )	 //by huangxin201804 _1 解决复位故障后立刻使能，电机不出力问题
            {
				STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;
				STR_ServoMonitor.BrakeFlag.bit.ErrResetPWMOn = 0;
			} 		
            break;

        default:
            break;
    }

	#if ECT_ENABLE_SWITCH
	if(((STR_FUNC_Gvar.DiDoOutput.Son_Flt == 1)||(STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon == 1))
       && (STR_ServoMonitor.RunStateFlag.bit.InnerServoSon == 0))     //确保不是内部使能
	{
	    if(STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0)
		{
            //只在直线电机时置位
            if( (((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30) && (FunCodeUnion.code.ER_AngIntSel != 2))
             || (((FunCodeUnion.code.MT_EncoderSel & 0xff) == 0x02) && (FunCodeUnion.code.ER_AngIntSel != 2)))
            {
                AuxFunCodeUnion.code.OEM_ResetABSTheta = 1;
            }		    
		}
	}
    #else
	if((STR_FUNC_Gvar.DiDoOutput.Son_Flt == 1)
       && (STR_ServoMonitor.RunStateFlag.bit.InnerServoSon == 0))     //确保不是内部使能
	{
	    if(STR_ServoMonitor.RunStateFlag.bit.FstSonFlag == 0)
		{
            //只在直线电机时置位
            if( (((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30) && (FunCodeUnion.code.ER_AngIntSel != 2))
             || (((FunCodeUnion.code.MT_EncoderSel & 0xff) == 0x02) && (FunCodeUnion.code.ER_AngIntSel != 2)))
            {
                AuxFunCodeUnion.code.OEM_ResetABSTheta = 1;
            }		    
		}
	}
	#endif
}

/*******************************************************************************
  函数名: 
  输入:  
  输出:  
  子函数:    
  描述: 伺服运行状态监控，包含正常运行时的状态切换和故障时的运行状态切换
********************************************************************************/ 
Static_Inline void ErrorRunStateSwitch()
{
    if((STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0x6000))   //若果发生故障1时，则运行状态切换为故障状态
    {
        ServoRunStatusUpdate = ERR; 
        STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrakeOver = 0; 
        STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
    }

    else if((STR_FUNC_Gvar.Monitor.HighLevelErrCode >= 0x6000)&&(STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0xE000) )  
    {
        if(STR_ServoMonitor.StopCtrlVar.Err2StopMode != 2 )   //故障2发生时，在非零速停机方式下，运行状态切换到故障状态
        { 
            ServoRunStatusUpdate = ERR; 
            STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrakeOver = 0;   
            STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
        }
        else if( (STR_ServoMonitor.StopCtrlVar.Err2StopMode == 2) && 
                 (STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == DISPWM) && 
                 ((STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrakeOver == 1)||(STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 0)) 
               )        //故障2发生时，在零速停机方式下，且无抱闸功能或有抱闸但已经零速停机结束后，其运行状态切换到故障状态
        {
            ServoRunStatusUpdate = ERR; 
            STR_ServoMonitor.BrakeFlag.bit.ErrZeroSpdBrakeOver = 0;   
            STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr = 1;
        }     
    }

    //恢复到nrd状态
    else if( (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 0) &&
             (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != NRD) && 
             ((STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0) || (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000))    
           )
    {
        ServoRunStatusUpdate = NRD;

        //伺服Off响应标志位
        //将其它故障响应标志位置为无效
        STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck = INVALID; 
        STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck = INVALID;
        STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck = INVALID;
        STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck   = INVALID;
        STR_FUNC_Gvar.MonitorFlag.bit.OTAckForHome    = INVALID;
           
        STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
        STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

        WriteGDStop(DISGD);
    }
}


/*******************************************************************************
  函数名: void ServoRunManage()
  输入:  
  输出:  
  子函数:    
  描述: 伺服运行状态的管理
********************************************************************************/ 
void ServoRunManage()
{

    //判断伺服是否准备好状态（包括电压OK，电流采样OK），移至电机模块，
    //电机模块负责传送伺服准备好状态给功能模块即可
    if(STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == VALID)  //电机电流检测放到FPGA中检测
    {
        STR_FUNC_Gvar.Monitor.DovarReg_SRdy = VALID;  //表示外部DOrdy状态，故障时此信号亦无效。   
    }
    else
    {
        STR_FUNC_Gvar.Monitor.DovarReg_SRdy = INVALID;
    }

    if (STR_ServoMonitor.StopCtrlFlag.bit.EmergencyStopAck == VALID)
    {
        STR_FUNC_Gvar.Monitor.DovarReg_SRdy = INVALID;
    }

    if((STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0xE000) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode != 0)) 
    {    
        STR_FUNC_Gvar.Monitor.DovarReg_SRdy = INVALID;
    }

   //ModSelect();    //	by huangxin201711_24 ModSelect()模式切换转移到位置环，以解决模式切换时位置不准的问题
}


/*******************************************************************************
  函数名: void MonitorShow()
  输入:  
  输出:  
  子函数:    
  描述: 监控程序中的相关变量的H0B组显示功能，因此该函数1K的调度频率
********************************************************************************/ 
void MonitorShow()
{
    static Uint16 TimeStampCnt = 0;    //用于时间戳计数
    static Uint32 SaveServoTimeCnt = 0;
	static Uint32 SaveAdaptFilterCnt = 0;
    static Uint16 H0902Latch = 0; 

    if( TimeStampCnt++ == TIMESTAMPCNT)  //时间戳单位：0.1s (即0.1s计一次值)，因此该函数1K的调度频率
    {
        TimeStampCnt = 0;
        ServoRunTime++;  //记录时间戳

       	AuxFunCodeUnion.code.DP_ServoTime_Lo = ServoRunTime & 0x0FFFF;    //H0B19 总上电时间低16位
        AuxFunCodeUnion.code.DP_ServoTime_Hi = ServoRunTime >> 16;        //H0B19 总上电时间高16位
        FunCodeUnion.code.ES_ServoTime_L  = AuxFunCodeUnion.code.DP_ServoTime_Lo;  //运行故障时的时间记录 ES_ServoTime_L 每个一个小时存一次，这样导致总上电时间DP_ServoTime_Lo存在1小时的误差
        FunCodeUnion.code.ES_ServoTime_H  = AuxFunCodeUnion.code.DP_ServoTime_Hi;  //运行故障时的时间记录 ES_ServoTime_L 每个一个小时存一次，这样导致总上电时间DP_ServoTime_Lo存在1小时的误差
    }

    //每隔1小时，保存一次运行时间,由于此函数调度是1K，所以3600000 = 60*60*1000
    if(SaveServoTimeCnt > 3600000L) 
    {
        SaveServoTimeCnt = 0;
                                                       //母线电压OK_不欠压的情形下保存运行时间,
        if(STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk == 1)  //当掉电保存功能添加后有输出欠压标志位时，此处应该采用欠压标志位，这个母线电压OK标志位可以做位MTR模块内部的，不用作为接口传递给FUNC
        {
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.ES_ServoTime_L));   //ES_ServoTime_L/H 在MonitorShow中每隔0.1s自加一
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.ES_ServoTime_H));   //ES_ServoTime_L/H 在每隔1小时，将其相应的功能码存入Eeprom中                
        }                                                                      //因此即使正常运行中，掉电前后两次运行时间最大误差为1个小时
    }
    else
    {
        SaveServoTimeCnt++;
    }
	if(SaveAdaptFilterCnt > 1800000L) 
    {
		SaveAdaptFilterCnt=0;
		if(STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk == 1)  //当掉电保存功能添加后有输出欠压标志位时，此处应该采用欠压标志位，这个母线电压OK标志位可以做位MTR模块内部的，不用作为接口传递给FUNC
        {
		    if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 1)
			{
			    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlC));
			}
			else if(STR_FUNC_Gvar.System.AdaptiveFilterMode == 2)
			{
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthC));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlC));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqD));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthD));
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlD));
			}
	    }
	}
	else 
	{
		SaveAdaptFilterCnt++;
	}

    //退出自动辨识时保存参数
    if((1 == H0902Latch) && (0 == STR_FUNC_Gvar.System.AdaptiveFilterMode))
	{
	    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlC));
	}
	else if((2 == H0902Latch) && (0 == STR_FUNC_Gvar.System.AdaptiveFilterMode))
	{
		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlC));
		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltFreqD));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltBandWidthD));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_NotchFiltAttenuatLvlD));
	}

    H0902Latch = STR_FUNC_Gvar.System.AdaptiveFilterMode;

    AuxFunCodeUnion.code.DP_Udc = (int16)STR_FUNC_Gvar.ADC_Samp.Udc_Live; //H0B26 母线电压值显示 
}
/********************************* END OF FILE *********************************/



