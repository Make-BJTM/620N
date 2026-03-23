/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "ECT_InterFace.h"
#include "FUNC_GlobalVariable.h"
#include "ECT_CSP.h"
#include "FUNC_ServoError.h"
#include "FUNC_ErrorCode.h"
#include "ECT_ESMDisplay.h"
#include "CANopen_DeviceCtrl.h"
#include "FUNC_PosCtrl.h"  
#include "CANopen_PV.h"
#include "ECT_PT.h"
#include "FUNC_FunCode.h"
#include "FUNC_InterfaceProcess.h"
#include "MTR_FUNCInterface.h"



/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
const Uint32 ComErrCodeTable[12]=
{
    ECATLINKFAIL,//总线掉线
    ECATLINKCLOSE,
    ECATLINKOVERTIME,
    ECATCARDLINKOVERTIME,
    ECATADDRCONFLICT,
    ECATADDROVERLIMIT,
    ECATSTATEMACHERRCHANGE,
    ECATSYNCLOST,
    ECATPARAERR,
    ECATCONFIGUREERR,
    EEPROMNULL,
    ECATINITFAILURE
};

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_ECATSYNCVAR  EcatSync;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */


/*******************************************************************************
  函数名:  void ESMStateMonitor(void)

  输入:    
  参数：      
  输出:   无       
  描述:   判断ESM状态
********************************************************************************/ 
void ESMStateMonitor(void)
{
	Uint16  ESM = 0;
    Uint8   CommState = 0;
    static Uint8   CommStateLatch = 0;
    static  Uint8 ESMStateLatch = 0;
	Uint32  ErrCode = 0;			// 错误状态		0 ：无错误   >0:状态码
    Uint64 Temp = 0;   

    CommState = C_CommState(&ESM);//从PREOP-->SAFEOP，才会将通讯故障清除   
	
    //2、ESM状态
    STR_ECTCSPVar.ESMState  = (Uint8)((ESM>>8)&0x3F);

	FunCodeUnion.code.CM_ESM = STR_ECTCSPVar.ESMState;

    //必须在程序外面读，否则捕捉不到
    if((ESMStateLatch==ESM_PREOPERATION)&&(STR_ECTCSPVar.ESMState == ESM_SAFEOPERATION))
    {
        STR_ECTCSPVar.SYNCPeriod = C_GetSyncCycle();
        STR_ECTCSPVar.SYNCPeriodRatio = (Uint64)STR_FUNC_Gvar.System.PosFreq * STR_ECTCSPVar.SYNCPeriod/((Uint64)1000000);
        
        //IS620N与载波频率关联   载波频率固定为8k或4k 周期125us 或250us
        //ETC伺服与速度环调度频率关联  速度环调度频率为32k  周期32.25us
        Temp = (Uint64)STR_ECTCSPVar.SYNCPeriod * 1000L;   //单位1ns           
        Temp = Temp % (Uint64)STR_FUNC_Gvar.System.CarPrd; 	//STR_FUNC_Gvar.System.CarPrd 单位ns

        if((Uint64)Temp != 0)
        {
            EcatSync.CarFreqDivSyncFreq = 0;
        }
        else
        {
            Temp = (Uint64)STR_ECTCSPVar.SYNCPeriod * (Uint64)STR_FUNC_Gvar.System.CarFreq; 
            EcatSync.CarFreqDivSyncFreq = Temp / 1000000L;       
        }
        
        EcatSync.TheoreticalSyncPrd_Half_25ns = (Uint32)(((Uint64)STR_ECTCSPVar.SYNCPeriod * (Uint64)10000) / 125L);
        EcatSync.TheoreticalSyncPrd_Half_25ns = EcatSync.TheoreticalSyncPrd_Half_25ns & 0xFFFFFFFE; 
    }

    //将E13报错移到外面来，因上位机会在转入OP检测到伺服有故障时，自动发故障复位指令
    if((STR_ECTCSPVar.ESMState > ESM_SAFEOPERATION)&&(FunCodeUnion.code.CM_FpgaSyncModeSel != 0))
    {
        Temp = (Uint64)STR_ECTCSPVar.SYNCPeriod * 1000L;   //单位1ns           
        Temp = Temp % (Uint64)STR_FUNC_Gvar.System.CarPrd; 	//STR_FUNC_Gvar.System.CarPrd 单位ns
        if((Uint64)Temp != 0)PostErrMsg(SYNCPERIODSETERR);
    }
    
    //无故障
    if(CommState == 0)//ESM状态显示+通讯连接指示
    {
        //如果当前面板显示故障为上一次的故障
        if((CommStateLatch!=0)&&(CommState==0))
        {
            if((STR_FUNC_Gvar.Monitor.HighLevelErrCode != 0)
                &&(STR_FUNC_Gvar.Monitor.HighLevelErrCode == (Uint16)(ComErrCodeTable[CommStateLatch-1]&0x0000FFFF)))
            {
                DeviceControlVar.AlmRstFlag = 1;
				
            }
        }
    }
    else//跳转到故障显示
    {
        if(CommState <= 12)
        {
        	if((CommState == 11)&&(FunCodeUnion.code.OEM_LocalModeEn==1)){}
			else
			{
				ErrCode = ComErrCodeTable[CommState-1];
	            PostErrMsg(ErrCode);
			}
        }
        else 
        {
            ErrCode = DEFUALTERR;
            PostErrMsg(ErrCode);
        }
    }

    ESMStateLatch = STR_ECTCSPVar.ESMState;
    CommStateLatch = CommState;

    if(STR_ECTCSPVar.ESMState==ESM_OPERATION)
    {
        STR_FUNC_Gvar.MonitorFlag.bit.ESMState = 1;
    }
    else
    {
        STR_FUNC_Gvar.MonitorFlag.bit.ESMState = 0;
    } 


//	if((RUN == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)&&(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==0)&&
//	   (STR_ServoMonitor.RunStateFlag.bit.InnerServoSon == 0))
//	{
//	    PostErrMsg(ECATSTATEMACHERRCHANGE);
//	}   
	   

    if(FunCodeUnion.code.CM_SyncErrThreshold > 100)
    {
        EcatSync.SyncErrThreshold = (FunCodeUnion.code.CM_SyncErrThreshold * 10L) /125L;
    }
    else
    {
        EcatSync.SyncErrThreshold = 8;
    }
	
}

/*******************************************************************************
  函数名:  void ECTInteruptUpdate(void)

  输入:    
  参数：      
  输出:   无       
  描述:   中断数据更新
********************************************************************************/ 
void ECTInteruptUpdate(void)
{
    ECTInteruptPosUpdate();
    ECTInteruptSpdShow();
    ECTInteruptToqUpdate();
    
}

/*******************************************************************************
  函数名:  
  输入:    
  参数：      
  输出:          
  描述:   FPGA  同步配置 
假设  
    SYNC0周期  Ps 
    载波周期Pc     
    理论上N = Ps/Pc
    余数R0 + R1 = Ps - N*Pc   
    R0 = X * 25ns   本周期需要补偿的 
    R1 = Y * 12.5ns 叠加到下个周期  
********************************************************************************/ 
Static_Inline void FpgaSyncModeConfig(void)
{
    static int32 Avr = 0;      //偏差
    static int32 Rem = 0;      //余数
    static int16 CarCnt = 0;    //载波计数器
    int32 Sum = 0;
    
    if(1 == EcatSync.FpgaSyncModeConfigDone)
    {
        if(STR_ECTCSPVar.ESMState != ESM_OPERATION)
        {
            //非OP模式下 切到自同步模式        
            SetSelfSyncMode();
            EcatSync.FpgaSyncModeConfigDone = 0;             
        }
        else
        {
            //Ecat同步中
            STR_FUNC_Gvar.System.FpgaSyncModeSt = 0; 
        }
    }
    else
    {
        if( (1 == FunCodeUnion.code.CM_FpgaSyncModeSel)
         && (0 != EcatSync.CarFreqDivSyncFreq)
         && (STR_ECTCSPVar.ESMState == ESM_OPERATION)
         && (0 == UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.ShortGndRunFlag)
         && (RUN != STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus) 
         && (DISPWM == STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus)
         && (STR_FUNC_Gvar.OscTarget.SyncLength > 0) )
        {
            //满足条件 切换Ecat同步
            SetEthercatSyncMode(); 
            EcatSync.FpgaSyncModeConfigDone = 1; 
            STR_FUNC_Gvar.System.FpgaSyncModeSt = 1;
        }
        else
        {
            //等待切换Ecat同步
            STR_FUNC_Gvar.System.FpgaSyncModeSt = 0; 
        }
    }

    if((1 == EcatSync.FpgaSyncModeConfigDone) && (EcatSync.CarFreqDivSyncFreq > 4)) 
    {
        //进入实时调节PWM周期阶段
        if(1 == GetEcatActive())
        {
            //实际测量值减去理论值
            //如果测量值大 说明sync周期大于理论周期 需要加大pwm周期
            Sum = (STR_FUNC_Gvar.OscTarget.SyncLength - EcatSync.TheoreticalSyncPrd_Half_25ns) >> 1;            
            Avr = Sum / EcatSync.CarFreqDivSyncFreq;  
            Rem = Sum - Avr * EcatSync.CarFreqDivSyncFreq;                    
            CarCnt = 0;            
        }

        if(0 == (CarCnt & 1))
        {              
            //PWM周期有上升沿和下降沿  补偿n相当于一个周期补偿量为2n
            if(Rem > 0)
            {   
                CompensatePwmPeriod(Avr + 1); 
                Rem --;
            }
            else if(Rem < 0)
            {   
                CompensatePwmPeriod(Avr - 1); 
                Rem ++;
            }            
            else
            {
                CompensatePwmPeriod(Avr);
            }
        }
         
        CarCnt ++; 
    }
    else
    {
        CompensatePwmPeriod(0); 
        CarCnt = 0;
        Avr = 0;      //偏差
        Rem = 0;      //余数        
    }                      
}


/*******************************************************************************
  函数名:  
  输入:    
  参数：      
  输出:          
  描述:   

********************************************************************************/
void FpgaSyncModeConfigAndCheck_ToqInt(void)
{
    static int8  FirFlag = 0;          

    STR_FUNC_Gvar.OscTarget.SyncLength = GetSyncLength(); //单位：12.5ns
    
    if(1 != FunCodeUnion.code.CM_FpgaSyncModeSel)return;

    FpgaSyncModeConfig(); 
    
    if(1 == EcatSync.FpgaSyncModeConfigDone)
    {
        if(3 > FirFlag)
        {
            if(1 == GetEcatActive())FirFlag ++;        
        }
        else
        {
            if(ABS(STR_FUNC_Gvar.OscTarget.SyncLength - EcatSync.TheoreticalSyncPrd_Half_25ns) >= EcatSync.SyncErrThreshold) PostErrMsg(SYNCBIASERR);
        }
    }
    else
    {
        FirFlag = 0; 
    }
}


/*******************************************************************************
  函数名:  
  输入:    
  参数：      
  输出:          
  描述:   
********************************************************************************/
void FpgaSyncModeConfig_SyncInt(void)
{


}

/*******************************************************************************
  函数名:  
  输入:    
  参数：      
  输出:          
  描述: esm 1:驱动情况下正转，2：驱动情况下反转 7：使能0速，8:非使能
      返回值1，报错状态
********************************************************************************/
Uint32 C_DeviceAppState(Uint16 *ESM)
{
    if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(STR_ServoMonitor.RunStateFlag.bit.InnerServoSon == 0))
	{
       if(STR_FUNC_Gvar.SpdCtrl.DovarReg_Zero==1)
	   {
	       *ESM = 7;   
	   }
       else
	   {
		   if(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt>0)
		   {
		       *ESM = 1;    
		   }
		   else
		   {
		       *ESM = 2;    
		   }
	   }		   		
		
		return 0;
	}
	else
	{
	    *ESM = 8;
        
		if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR)
		{
			return 1;
		}
		else
		{
			return 0;
		}			
	}

}




/********************************* END OF FILE *********************************/
