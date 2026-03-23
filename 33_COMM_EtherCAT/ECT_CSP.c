
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "ECT_CSP.h"
#include "CANopen_OD.h"
#include "CANopen_Pub.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_GPIODriver.h"
#include "PUB_Main.h"
#include "FUNC_ServoError.h"
#include "FUNC_ErrorCode.h"
#include "CANopen_PP.h"
#include "FUNC_PosCtrl.h"          
#include "FUNC_FunCode.h"
#include "CANopen_DeviceCtrl.h"
#include "FUNC_FullCloseLoop.h"
#include "FUNC_CSPCmdBuffer.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_ECTCSPVAR STR_ECTCSPVar = {4,1000,262140,1048576,0,0,0,0,0,0,0,0,3,0,0}; // 
static int32 preInterpltPoint =0;//上次插补位置，用来求位置变化量--指令单位
STR_ECTCSPPOSREF STR_ECTCSPPosRef;
Uint16 ExcCnt = 0;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */



/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
//__inline void CSPPointCal(void); 
int64 CSPPosRef(STR_ECTCSPPOSREF *p,int64 CmdInput);
//void CSPClear(void);
void preInterpltPointUpdata(void);

/*******************************************************************************
  函数名: void InitECTCSP(void)

  输入:   无 
  输出:   无
  子函数:无
  描述:  插补模式下,初始化插补变量清零
********************************************************************************/ 
void InitECTCSP(void)
{
	STR_ECTCSPVar.SYNCPeriodRatio= 4;//默认值1ms对应4个位置环周期
	CSPClear();
	preInterpltPointUpdata();
}

/*******************************************************************************
  函数名: void ECTCSPUpdata(void)

  输入:
  输出:   
  子函数:无
  描述:  
********************************************************************************/ 
void ECTCSPUpdata(void)
{
	preInterpltPointUpdata();
    CSPClear();
}



/*******************************************************************************
  函数名: void ECTCSPMaxSpeedUpdate(void)

  输入:
  输出:   
  子函数:无
  描述:  CSP最大限速计算
********************************************************************************/ 
void ECTCSPMaxSpeedUpdate(void)
{
    int32 MotorMaxSpeed =0;
	int64  Temp = 0;
    //速度限制
    MotorMaxSpeed = Rpm2UserVel(&STR_Vel_Factor,(int32)FunCodeUnion.code.MT_MaxSpd);

    STR_ECTCSPVar.MaxInterpltPosCmd = (UserVelUnit2IncpsUnit(&STR_VelEnc_Factor,MotorMaxSpeed))>>16;
	Temp = STR_ECTCSPVar.MaxInterpltPosCmd + (STR_ECTCSPVar.MaxInterpltPosCmd/50);
	STR_ECTCSPVar.MaxDeltaTarPos = 	Temp * (int64)STR_ECTCSPVar.SYNCPeriodRatio;
}


/*******************************************************************************
  函数名: void preInterpltPointUpdata(void)

  输入:
  输出:   
  子函数:无
  描述:  计算初始插补位置-指令单位
********************************************************************************/ 
void preInterpltPointUpdata(void)
{
    int64 PosTemp = 0;

    if(FunCodeUnion.code.BP_ModeSelet == 9)
    {
        if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
        {
			PosTemp = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
        				//+ (int64)STR_FUNC_Gvar.PosCtrl.PosAmplifErr 
        				//- STR_FUNC_Gvar.PosCtrl.PosFdb;
        }
		else
		{

            if((FunCodeUnion.code.FC_FeedbackMode == 1)&&(FunCodeUnion.code.FC_ExInErrFilterTime!=0))//外部位置反馈,外部位置偏差
            {
        		PosTemp = STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
        				+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos; 
        			   // - STR_FUNC_Gvar.PosCtrl.ExPosFdb;	//by huangxin201711 解决模式切换后607a与6064对不齐问题
                
            }
            else if((FunCodeUnion.code.FC_FeedbackMode == 1)&&(FunCodeUnion.code.FC_ExInErrFilterTime==0))//外部位置反馈,内部位置偏差
            {
        		PosTemp = STR_FUNC_Gvar.PosCtrl.PosAmplifErr
        			    + STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos; 
        				//- STR_FUNC_Gvar.PosCtrl.PosFdb;	//by huangxin201711 解决模式切换后607a与6064对不齐问题
            }
            else if(FunCodeUnion.code.FC_FeedbackMode == 0)//内部位置反馈，内部位置偏差
            {
        		PosTemp = (int64)STR_FUNC_Gvar.PosCtrl.PosAmplifErr
        				+ STR_InnerGvarPosCtrl.CurrentAbsPos ;
        			//	- STR_FUNC_Gvar.PosCtrl.PosFdb;		//by huangxin201711 解决模式切换后607a与6064对不齐问题
            }
            else
            {
                PostErrMsg(MULTPOSCLASHFULCLOP);    //参数设置错误
            }
        }
		preInterpltPoint = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,PosTemp);
    }
}



/*******************************************************************************
  函数名: WriteIntpltFIFO(void)

  输入:   
  输出:   无
  子函数:无
  描述:  中断写入插补数据
********************************************************************************/ 
void WriteCSPFIFO(void)
{
    int32 TargetPos = 0;
    int64 Temp64 = 0;
    static int32 poslatch = 0;


	TargetPos = STR_CmdBufferVar.CSP_TargetPos;

    STR_FUNC_Gvar.OscTarget.InputTarPosRT = TargetPos;
	STR_CanSyscontrol.preMode = ObjectDictionaryStandard.DeviceControl.ModesOfOperation; // by huangxin201711_25 0x6060在sync中更新到preMode，以保证模式切换发生在sync之后的第一个位置环

	if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(STR_CanSyscontrol.Mode == ECTCSPMOD)
        &&(STR_FUNC_Gvar.MonitorFlag.bit.BrakeUnLock==0)&&(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==1))
	{
        if(STR_ECTCSPVar.CSPRunFlag == 0)
        {
            ECTCSPUpdata();
            poslatch = preInterpltPoint;
            STR_ECTCSPVar.CSPRunFlag = 1;
        }
        
		//位置指令取反
//        if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x80)== 0x80)//607E bit7=1位置指令反向
//		{
//		    TargetPos = 0 - TargetPos;
//		}

        //获取对象值 每一个同步周期更新一次

        /*if(STR_ECTCSPVar.OTDeal == 1)//超程中
        {
            STR_ECTCSPVar.InterpltPoint= TargetPos;
        }
        else*/
        {
            STR_ECTCSPVar.InterpltPoint= TargetPos;
        }
	    
        Temp64 = (int32)(TargetPos - poslatch);
        poslatch = TargetPos;
        Temp64 = Temp64 * (int64)STR_Pos_Factor.PosFactor6093_Numerator /(int64)STR_Pos_Factor.PosFactor6093_Denominator;
        
        if((ABS(Temp64)>=STR_ECTCSPVar.MaxDeltaTarPos))
    	{
    		STR_ECTCSPVar.P_overspeedcnt++;
    		if(STR_ECTCSPVar.P_overspeedcnt>=FunCodeUnion.code.CM_CSPCmdIncErrCnt)
    		{
    		    STR_ECTCSPVar.P_overspeedcnt=FunCodeUnion.code.CM_CSPCmdIncErrCnt;
    		    PostErrMsg(PULSINERR);
    		}
    	}
    	else
    	{
    		STR_ECTCSPVar.P_overspeedcnt = 0;
    	}
        FunCodeUnion.code.CM_CSPCmdErrCnt = STR_ECTCSPVar.P_overspeedcnt;
	}
    else
    {
        STR_ECTCSPVar.CSPRunFlag = 0;
    }
}
/*******************************************************************************
  函数名: CSPPointCal(void)

  输入:  
  输出:   无
  子函数:无
  描述:  计算位置指令增量-编码器单位 ，同时在指令端进行软件位置限制判断
********************************************************************************/ 
void CSPPointCal(void)	
{        
    int32 userDeltPos=0;
    int32 temp=0;
    
    userDeltPos = STR_ECTCSPVar.InterpltPoint;

    //userDeltPos = CanopenPosLmt(temp);
    
    temp = userDeltPos - preInterpltPoint;

	STR_ECTCSPVar.InterpltDltRef = UserPosUnit2IncpUnit(&STR_Pos_Factor,temp);
    STR_ECTCSPVar.InterpltSumRef += STR_ECTCSPVar.InterpltDltRef;
    preInterpltPoint = userDeltPos;
}
/*******************************************************************************
  函数名:  int32 InterpltPosRefCal(void)
  输入:    
  参数：      
  输出:          
  描述:    完成插补计算，输出位置指令，调度应该在位置环        
********************************************************************************/ 
int32 ECTCSPPosCmd(void)
{   
    STR_ECTCSPPOSREF *pInterplt = &STR_ECTCSPPosRef;
	int32  MaxInterpltPos=0;
	int32 temp3 = 0;

    if((DeviceControlVar.OEMStatus != STATUS_OPERENABLE) || (STR_FUNC_Gvar.MonitorFlag.bit.ESMState!=1))
    {
        //ip mode inactive
        CSPClear();
        //drive follow the command value
        ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;
        return 0;
    }    
   
    ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 1;//drive follow the command value
	
    if(STR_ECTCSPVar.CSPRunFlag == 1)
    {
        if(ExcCnt >1)
        {
            ExcCnt--;
            temp3 = STR_ECTCSPPosRef.PosRefAve;
        }
        else
        {
            ExcCnt = STR_ECTCSPVar.SYNCPeriodRatio;
			CSPPointCal();
			STR_ECTCSPPosRef.PosRefAve = CSPPosRef(pInterplt,STR_ECTCSPVar.InterpltDltRef);
            temp3 = STR_ECTCSPPosRef.PosRefAve;
        }

    }
    else
    {
        temp3 = 0;
    }

    STR_ECTCSPVar.InterpltIncPer = temp3;

    //位置指令1的跳变补偿
    if(STR_ECTCSPVar.RefPosRemainder>0)
    {
        if(STR_ECTCSPVar.InterpltIncPer>=0)
        {
            STR_ECTCSPVar.InterpltIncPer = STR_ECTCSPVar.InterpltIncPer +1;
            STR_ECTCSPVar.RefPosRemainder = STR_ECTCSPVar.RefPosRemainder - 1;
        }
    }
    else if(STR_ECTCSPVar.RefPosRemainder<0)
    {
        if(STR_ECTCSPVar.InterpltIncPer<=0)
        {
            STR_ECTCSPVar.InterpltIncPer = STR_ECTCSPVar.InterpltIncPer -1;
            STR_ECTCSPVar.RefPosRemainder = STR_ECTCSPVar.RefPosRemainder + 1;
        }
    }

    //速度限制---不超过6000rpm，避免报Er.B00
	MaxInterpltPos = STR_ECTCSPVar.MaxInterpltPosCmd + STR_ECTCSPVar.MaxInterpltPosCmd/50;

    //速度限制---不超过6000rpm，避免报Er.B00
    if(STR_ECTCSPVar.InterpltIncPer >=MaxInterpltPos)
    {            

        STR_ECTCSPVar.RefPosRemainder += STR_ECTCSPVar.InterpltIncPer - MaxInterpltPos;
        STR_ECTCSPVar.InterpltIncPer = MaxInterpltPos;

    }
    else if((STR_ECTCSPVar.InterpltIncPer + MaxInterpltPos)<=0)
    {
        STR_ECTCSPVar.RefPosRemainder += STR_ECTCSPVar.InterpltIncPer + MaxInterpltPos;
        STR_ECTCSPVar.InterpltIncPer = -MaxInterpltPos;
    }


	STR_ECTCSPVar.InterpltSumRef -= STR_ECTCSPVar.InterpltIncPer;

    return STR_ECTCSPVar.InterpltIncPer;
}
/*******************************************************************************
  函数名:  void IPClear(void)
  输入:    
  参数：      
  输出:          
  描述:    插补清除函数         
********************************************************************************/ 
void CSPClear(void)
{
	STR_ECTCSPVar.OTLatchPosCmd = 0;
	STR_ECTCSPVar.P_overspeedcnt=0;
	STR_ECTCSPVar.RefPosRemainder = 0;
	STR_ECTCSPVar.InterpltSumRef = 0;
	STR_ECTCSPVar.InterpltDltRef = 0;
	ExcCnt = 0;	
}
/*******************************************************************************
  函数名:  InterPltPosRef(STR_ECTCSPPOSREF *p,int64 CmdInputQ16,int32 InterpltPeriod)
  输入:    CmdInputQ16	          //插补位移
		   InterpltPeriod          //插补周期计数值
  参数：       
  输出:    PosRefAveQ16        //平均位置指令
  描述:    对位置指令进行均分计算，同时判断上位机同步帧误差是否超过±1/4设定值         
********************************************************************************/ 
int64 CSPPosRef(STR_ECTCSPPOSREF *p,int64 CmdInput)
{
    if (STR_ECTCSPVar.SYNCPeriodRatio == 0)	  //当同步周期为0时，退出程序，返回0
    {
        return 0;
    }

    p->InPut= CmdInput;    //该同步周期要执行的位置指令总数
    
    p->PosRefAve = (p->InPut +  STR_ECTCSPVar.RefPosRemainder) / (int64)STR_ECTCSPVar.SYNCPeriodRatio;

    STR_ECTCSPVar.RefPosRemainder = (p->InPut +  STR_ECTCSPVar.RefPosRemainder) % (int64)STR_ECTCSPVar.SYNCPeriodRatio;
    
    
    return (p->PosRefAve);    
}

/********************************* END OF FILE *********************************/
