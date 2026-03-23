/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_ServoError.c
 创建人: 王军干
 修改人：李浩                修改日期：11.12.13 
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
#include "FUNC_ServoError.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ServoMonitor.h"
#include "FUNC_ManageFunCode.h"
#include "PUB_Main.h"
#include "CANopen_DeviceCtrl.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void PostErrMsg(Uint32 PostErrCode);
void WarnAutoClr(Uint32 PostWarnCode);
void ErrProcess_MaimLoop(void);

extern void CanopenServorErrorMsgFresh(Uint16 PostErrCode,Uint16 InnerErrCode);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void ErrorMsgRecord(Uint16 PostErrCode,Uint16 InnerErrCode);
Static_Inline void WarnGenerate(Uint16 PostWarnCode);
Static_Inline void WarnRegClr(Uint16 WarnCode);
Static_Inline void ErrRecordMsgDisplay(void);

Static_Inline void AlrmReset(void);
Static_Inline void ErrMsgDoProcess(void); 
Static_Inline void ErrRecordProcess(void);

/*******************************************************************************
  函数名:  void PostErrMsg(Uint32 PostErrCode)
  输入:    ErrCode 故障码
  输出:    故障响应标志位，以及高级别的故障信息
  子函数:  无      
  描述:  传递故障信息，并根据故障信息作预处理，主要是置故障响应标志位，以及保存
         高级别的故障码等
********************************************************************************/ 
void PostErrMsg(Uint32 PostErrCode)
{
    Uint8   StopRunMode = 0xFF;
    Uint16  ErrLookUpTemp = 0;   //故障类别查询临时变量(0->No1.不可复位故障，1->No1.可复位故障 ，3->No2.故障 ，7->warn.警告)
    Uint16  InnerErrCode = 0;

    InnerErrCode = PostErrCode >> 16;
    PostErrCode = PostErrCode & 0xFFFF;

	#if DRIVER_TYPE == SERVO_650N
    if((FunCodeUnion.code.ER_UseInProduce==1)&&
	    ( (PostErrCode==(ENFBCHKERR&0xFFFF))
	     ||(PostErrCode==(ENCDERR_Z&0xFFFF))
	     ||(PostErrCode==(MOTORTEMPOT&0xFFFF))
	     ||(PostErrCode==(CHABSROMERR&0xFFFF))
	     ||(PostErrCode==(CHABSROMERR1&0xFFFF))
		 ||(PostErrCode==(FPGAINITERR&0xFFFF))
		 ||(PostErrCode==(OVERSPD&0xFFFF))
		 ||(PostErrCode==(MTOFFLINE&0xFFFF))
		 ||((PostErrCode&0x00000FFF)==0x0120)
		 ||((PostErrCode&0x00000FFF)==0x0755))
	    )
	{
	    //FunCodeUnion.code.SL_SpdCMDFromKeyBoard=0;
		return;   
	}
	#endif


    /* 初始化参数有误就不再处理其他的错误 */
    if( (STR_FUNC_Gvar.Monitor.ErrCode == (PostErrCode & 0xFFFF)) ||
        (STR_FUNC_Gvar.Monitor.ErrCode == (INITOEMPARAERR & 0xFFFF)) ||
        (STR_FUNC_Gvar.Monitor.ErrCode == (INITSYSTMPERR & 0xFFFF)) || 
        (STR_FUNC_Gvar.Monitor.ErrCode == (CTRLPOWERUDVDC1 & 0xFFFF)) || 
        (STR_FUNC_Gvar.Monitor.ErrCode == (ENFBCHKERR & 0xFFFF)) ||
        (STR_FUNC_Gvar.Monitor.ErrCode == (ENCDERR_Z & 0xFFFF)) ||
        (STR_FUNC_Gvar.Monitor.ErrCode == (CHABSROMERR & 0xFFFF)) )
    {  
        return;
    }

    STR_FUNC_Gvar.Monitor.ErrCode = PostErrCode & 0xFFFF;  //将当前故障码信息记录于全局的故障变量中		        
    ErrLookUpTemp = (ERRCODEMSK & PostErrCode) >> ERRMSKBIT;  //用于查询故障类型(0->No1.不可复位故障，1->No1.可复位故障 ，3->No2.故障 ，7->warn.警告)  

    switch(ErrLookUpTemp)  //置相应标志,均能产生故或者警告标志位，按故障类型决定是否响应故障
    {
        case 0:  /* 伺服故障1，不可复位 */
                 STR_ServoMonitor.ErrorFlag.bit.Err1UnRstFlag  = VALID;    //不复位故障1标志位置1
                 STR_FUNC_Gvar.Monitor.DovarReg_Alm = VALID;

                 STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck = VALID;    //故障1停机响应标志位置1
                 StopRunMode = STR_ServoMonitor.StopCtrlVar.Err1StopMode;  //当前停机方式为故障1停机方式
                 
				 #if ECT_ENABLE_SWITCH

                 if(ObjectDictionaryStandard.DeviceControl.StatusWord.bit.VoltageEnabled == 1)
                 {
                    DeviceControlVar.StatusStep = STATUS_FAULTACTIVE;
                 }
                 else
                 {
                    DeviceControlVar.StatusStep = STATUS_NRD_FAULT;
                 }
                 ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Fault = 1;
                 DeviceControlVar.OEMStatus =  DeviceControlVar.StatusStep; 
				 #endif             
                 break;
        
        case 1:  /* 伺服故障1，可复位 */
                 STR_ServoMonitor.ErrorFlag.bit.Err1RstFlag    = VALID;   //可复位故障1标志位置1
                 STR_FUNC_Gvar.Monitor.DovarReg_Alm = VALID;

                 STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck = VALID;   //可复位与不可复位的故障1响应为同，都为故障1停机响应标志位置1               
                 StopRunMode = STR_ServoMonitor.StopCtrlVar.Err1StopMode; //当前停机方式为故障1停机方式
				 
				 #if ECT_ENABLE_SWITCH
                 if(ObjectDictionaryStandard.DeviceControl.StatusWord.bit.VoltageEnabled == 1)
                 {
                    DeviceControlVar.StatusStep = STATUS_FAULTACTIVE;
                 }
                 else
                 {
                    DeviceControlVar.StatusStep = STATUS_NRD_FAULT;
                 }
                 ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Fault = 1;
                 DeviceControlVar.OEMStatus =  DeviceControlVar.StatusStep;
				 #endif             
				 break;
        
        case 3:  /* 伺服故障2，可复位 */
                 STR_ServoMonitor.ErrorFlag.bit.Err2RstFlag   = VALID;                         
                 STR_FUNC_Gvar.Monitor.DovarReg_Alm = VALID; 
                                       
                 STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck = VALID;   //故障2停机响应标志位置1
                 StopRunMode = STR_ServoMonitor.StopCtrlVar.Err2StopMode;
				 
				 #if ECT_ENABLE_SWITCH
                 if(ObjectDictionaryStandard.DeviceControl.StatusWord.bit.VoltageEnabled == 1)
                 {
                    DeviceControlVar.StatusStep = STATUS_FAULTACTIVE;
                 }
                 else
                 {
                    DeviceControlVar.StatusStep = STATUS_NRD_FAULT;
                 }
                 ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Fault = 1;
                 DeviceControlVar.OEMStatus =  DeviceControlVar.StatusStep; 
				 #endif             
                 break;
       
       case 7:  /* 警告，不做出响应，只供显示用 */
                 WarnGenerate(PostErrCode);       
                 STR_FUNC_Gvar.Monitor.DovarReg_Warn = VALID;
             
				 break;  
        
       default: /* 参数出故障 */
                STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                break;
    }              
        
    /*需要立即关断PWM的停机(0-自由停机，1-DB停机)（警告时不作处理）*/
    if( (StopRunMode < 2) && (PostErrCode < 0xE000) )
    {             
        STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
    }
   
    /* 若产生的故障最高级别低于或等于当前产生的故障级别,则更新最高级别故障 */
    if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0x0104)
     && ( (STR_FUNC_Gvar.Monitor.ErrCode == 0x0136)
       || (STR_FUNC_Gvar.Monitor.ErrCode == 0x0A34)
       || (STR_FUNC_Gvar.Monitor.ErrCode == 0x0740) ) )
    {
        //解决测试问题，报104的故障后又报编码器的故障，优先显示编码器的故障
        STR_FUNC_Gvar.Monitor.HighLevelErrCode = STR_FUNC_Gvar.Monitor.ErrCode; 
        //H0B45显示内部故障
        AuxFunCodeUnion.code.DP_InnerErrCode = InnerErrCode;    
    }
    else if( STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0 )   
    {
        STR_FUNC_Gvar.Monitor.HighLevelErrCode = STR_FUNC_Gvar.Monitor.ErrCode; 
        //H0B45显示内部故障
        AuxFunCodeUnion.code.DP_InnerErrCode = InnerErrCode;
    }
    else if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0xE000) > (STR_FUNC_Gvar.Monitor.ErrCode & 0xE000) )
    {
        STR_FUNC_Gvar.Monitor.HighLevelErrCode = STR_FUNC_Gvar.Monitor.ErrCode;
        //H0B45显示内部故障
        AuxFunCodeUnion.code.DP_InnerErrCode = InnerErrCode;             
    }   
    else if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0xE000) == (STR_FUNC_Gvar.Monitor.ErrCode & 0xE000) )
    {
	    if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0x0FFF) > (STR_FUNC_Gvar.Monitor.ErrCode & 0x0FFF) )
        {
		    STR_FUNC_Gvar.Monitor.HighLevelErrCode = STR_FUNC_Gvar.Monitor.ErrCode;
            //H0B45显示内部故障
            AuxFunCodeUnion.code.DP_InnerErrCode = InnerErrCode;
        }
    }

    ErrorMsgRecord(PostErrCode,InnerErrCode);  //保存故障信息

	#if ECT_ENABLE_SWITCH
    CanopenServorErrorMsgFresh(STR_FUNC_Gvar.Monitor.HighLevelErrCode,InnerErrCode); 
	#endif

    STR_ServoMonitor.ErrorFlag.bit.ErrDoRecFlg = 1;  //置故障Do输出标志位

    /* 置故障信息面板显示标志位，该标志位的清零是在面板显示程序中进行的 */
    //有故障时置故障面板显示标志位为1
    if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode!=0)&&(STR_FUNC_Gvar.Monitor.HighLevelErrCode<0xE000)  )
    {
        STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow = 1;
    }
    //在有警告时，面板是否显示标志位处理
    else if((STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000) && (UNI_WarnReg.all != 0))
    {
        if (FunCodeUnion.code.BP_AlmDispSel == 0)    //当功能码H0215=0时，面板显示警告
        {
            STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow = 1;
        }
        else if(FunCodeUnion.code.BP_AlmDispSel ==1) //当功能码H0215=1时，面板不显示警告
        {
            STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow = 0;
        }
    }
}


/*******************************************************************************
  函数名:  Static_Inline void ErrorMsgRecord(Uint16 PostErrCode,Uint16 InnerErrCode)
  输入:    当前故障码PostErrCode
  输出:    FunCodeUnion.code.ErrMsgBuff[] 
  子函数:  SaveToEepromSeri() ； GetCodeIndex()   
  描述:  故障信息记录，属于PostErrMsg()的子函数，即当每发生一次故障或者警告时
         将故障信息，逐个记录于10个故障信息记录的结构体数组中，该故障信息存入
         Eeprom中，并提供给H0b组功能码显示用。        
********************************************************************************/ 
Static_Inline void ErrorMsgRecord(Uint16 PostErrCode,Uint16 InnerErrCode)
{
    Uint16 Temp = 0;
    static Uint8  ErrBuffCnt = 0;    //故障缓存指针 
   
    for(Temp = 0; Temp < 4; Temp++)  //与缓存里边的故障作比较,若有相同的，则不存故障信息
    {
        if(STR_ServoMonitor.ErrorVar.ErrCodeBuff[Temp] == PostErrCode ) return;
    }
    /* 若为新的故障，则将其存入缓存 */
	STR_ServoMonitor.ErrorVar.ErrCodeBuff[ErrBuffCnt] = PostErrCode;
    ErrBuffCnt++;  	    
    if(ErrBuffCnt > 3)
    {
        ErrBuffCnt = 0;
    }

    //H0B组FPGA状态信息更新
    UpdateH0B_FPGA_State();

    //不掉电时存储故障记录
    if((0 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave)
      && (0 == FunCodeUnion.code.ErrMsgSaveDisable) )
    {
        if((0x3740 == InnerErrCode) && (0x0740 == PostErrCode))STR_ServoMonitor.ErrorFlag.bit.RdEncEepromEn = 1;
        
        /* 保存故障时的信息,将故障信息存入功能码故障数组中 */
        FunCodeUnion.code.ES_ErrMsgTail++ ;
        if(FunCodeUnion.code.ES_ErrMsgTail >= 10)
        {
            FunCodeUnion.code.ES_ErrMsgTail = 0;
        }
    
        //新平台中更改后的应该书写的变量名 
        //时间戳、UV相电流、速度值、母线电压值、DIDO状态(实际电平状态)
        Temp = FunCodeUnion.code.ES_ErrMsgTail;
    
        FunCodeUnion.code.ErrMsgBuff[Temp].ErrCode    = PostErrCode;
        FunCodeUnion.code.ErrMsgBuff[Temp].TimeStampL = FunCodeUnion.code.ES_ServoTime_L;    
        FunCodeUnion.code.ErrMsgBuff[Temp].TimeStampH = FunCodeUnion.code.ES_ServoTime_H;   
        FunCodeUnion.code.ErrMsgBuff[Temp].Spd = (int16)(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb/10000L);
        FunCodeUnion.code.ErrMsgBuff[Temp].Iu  = (int16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IuFdb;
        FunCodeUnion.code.ErrMsgBuff[Temp].Iv  = (int16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IvFdb;
        FunCodeUnion.code.ErrMsgBuff[Temp].Udc     = (int16)STR_FUNC_Gvar.ADC_Samp.Udc_Live;   
        FunCodeUnion.code.ErrMsgBuff[Temp].DiState = AuxFunCodeUnion.code.DP_DIState;  //DI1~9
        FunCodeUnion.code.ErrMsgBuff[Temp].DoState = AuxFunCodeUnion.code.DP_DOState;  //DO1~5 
        FunCodeUnion.code.ErrMsgBuff[Temp].FpgaAbsRomErr = AuxFunCodeUnion.code.DP_FpgaAbsRomErr;
        FunCodeUnion.code.ErrMsgBuff[Temp].FpgaSysState  = AuxFunCodeUnion.code.DP_FpgaSysState;
        FunCodeUnion.code.ErrMsgBuff[Temp].FpgaSysErr  = AuxFunCodeUnion.code.DP_FpgaSysErr;
        FunCodeUnion.code.ErrMsgBuff[Temp].EncStBit1  = 0;
        FunCodeUnion.code.ErrMsgBuff[Temp].EncStBit2  = 0;
        FunCodeUnion.code.ErrMsgBuff[Temp].InnerErrCode  = InnerErrCode;
        FunCodeUnion.code.ErrMsgBuff[Temp].FpgaTmFlt  = AuxFunCodeUnion.code.DP_FpgaTmFlt;

        STR_ServoMonitor.ErrorFlag.bit.ErrMsgRecordEn = 1; 
    }
}

/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  子函数:    
  描述:  
********************************************************************************/ 
Static_Inline void WarnGenerate(Uint16 PostWarnCode)
{
    if ( PostWarnCode > 0xE000)      // 当时警告码时才进行操作
    {
        switch(PostWarnCode)
        {
            case (IDENTIFYWARN & 0xFFFF):           //0xEA40     参数辨识失败
                UNI_WarnReg.bit.IDENTIFYWARNFLG = 1;
                break;
		    #if CAN_ENABLE_SWITCH
            case (CANPASSIVEERR & 0xFFFF):           //0xE996  总线被动错误警告     
                UNI_WarnReg.bit.CANPASSIVEERRFLG  = 1;
                break;
            case (CANRECOVBUSOFF & 0xFFFF):           //0xE995   总线恢复警告
                UNI_WarnReg.bit.CANRECOVBUSOFFFLG  = 1;
                break;
		    #endif
		    #if (CAN_ENABLE_SWITCH||ECT_ENABLE_SWITCH)
            case (ODVALUEERR & 0xFFFF):           //0xE998  对象字典数值设置错误
                UNI_WarnReg.bit.ODVALUEERRFLG  = 1;
                break;
		    #endif
            case (COMMADDRCONFLICT & 0xFFFF):     //0xE994      CANLINK地址冲突
                UNI_WarnReg.bit.COMMADDRCONFLICTFLG = 1;
                break;
            case (POWERPLWARN & 0xFFFF):          //0xE990    电源缺相警告
                UNI_WarnReg.bit.POWPLWARNFLG = 1;
                break;
            case (ABSMOTALARM & 0xFFFF):          //0xE980    编码器警告
                UNI_WarnReg.bit.ABSMOTALARMFLG = 1;
                break;
            case (NOTWARN & 0xFFFF):              //0xE952    负向超程警告
                UNI_WarnReg.bit.NOTWARNFLG = 1;
                break;
            case (PNOTWARN & 0xFFFF):             //0xE950    正向超程警告
                UNI_WarnReg.bit.PNOTWARNFLG = 1;
                break;
            case (PRAMSVWARN & 0xFFFF):           //0xE942    参数存储频繁警告
                UNI_WarnReg.bit.PRAMSVWARNFLG = 1;
                break;
            case (PCHGDWARN & 0xFFFF):            //0xE941    需要重新接通电源的参数变
                UNI_WarnReg.bit.MCHGDWARNFLG = 1;
                break;
            case (MTOFFLINE & 0xFFFF):            //0xE939    电机动力线断线
                UNI_WarnReg.bit.MTOFFLINEFLG = 1;
                break;
            case (RBTOOSMALLWARN & 0xFFFF):       //0xE922    外置再生泄放电阻阻值过小
                UNI_WarnReg.bit.RBTOOSMALLFLG = 1;
                break;
            case (BLKOVERCURRENT & 0xFFFF):       //0xE921    再生制动电阻过载
                UNI_WarnReg.bit.BLKOVERCURRENTFLG = 1;
                break;
            case (RBOVERLOADWARN & 0xFFFF):       //0xE920    再生制动电阻过载
                UNI_WarnReg.bit.RBOVERLOADFLG = 1;
                break;
            case (EMERGENCYSTOPWARN & 0xFFFF):    //0xE900    紧急停机警告
                UNI_WarnReg.bit.EMSTOPWARNFLG = 1;
                break;
            case (AIZEROBIASOVWARN & 0xFFFF):       //0xE831 AI零偏过大 
                UNI_WarnReg.bit.AIZEROBIASOVWARNFLG = 1;
                break;
            case (ORIGINOVERTIME & 0xFFFF):       //0xE601 原点复归回零超时错误
                UNI_WarnReg.bit.ORIGINOVERTIMEFLG = 1;
                break;
            case (PULSOUTSETWARN & 0xFFFF):       //0xE110 分频脉冲输出设定故障
                UNI_WarnReg.bit.PULSOUTSETWARNFLG = 1;
                break;
	        case (MTOLWARN& 0xFFFF):             //0xE909电机过载警告
	            UNI_WarnReg.bit.MTOLWARNFLG= 1;
	            break;
	        case (ENCDBATWARN& 0xFFFF):             //0xE730 编码器电池报警
	            UNI_WarnReg.bit.ENCDBATWARNFLG= 1;
	            break;
	        case (ENCDTXCOMMERR& 0xFFFF):             //0xE732 编码器TX端通讯校验错误
	            UNI_WarnReg.bit.ENCDTXCOMMERRFLG= 1;
	            break;
		   case (ENCODEROT& 0xFFFF):		           //0xE760 编码器过热
                UNI_WarnReg.bit.ENCODEROTFLG= 1;
			    break;
            default:
                break;
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
Static_Inline void WarnRegClr(Uint16 WarnCode)
{
    switch(WarnCode)
    {
        case (IDENTIFYWARN & 0xFFFF) :             //0xEA40    参数辨识失败
            UNI_WarnReg.bit.IDENTIFYWARNFLG = 0;
            break;
	    #if CAN_ENABLE_SWITCH
        case (CANPASSIVEERR & 0xFFFF):           //0xE996  总线被动错误警告     
            UNI_WarnReg.bit.CANPASSIVEERRFLG  = 0;
            break;
        case (CANRECOVBUSOFF & 0xFFFF):           //0xE995   总线恢复警告
            UNI_WarnReg.bit.CANRECOVBUSOFFFLG  = 0;
            break;
		#endif
        
	    #if (CAN_ENABLE_SWITCH||ECT_ENABLE_SWITCH)
        case (ODVALUEERR & 0xFFFF):           //0xE998  对象字典数值设置错误
            UNI_WarnReg.bit.ODVALUEERRFLG  = 0;
            break;
	    #endif
        
        case (COMMADDRCONFLICT & 0xFFFF) :         //0xE994    CANLINK地址冲突
            UNI_WarnReg.bit.COMMADDRCONFLICTFLG = 0;
            break;
        case (POWERPLWARN & 0xFFFF) :              //0xE990    电源缺相警告解除
            UNI_WarnReg.bit.POWPLWARNFLG = 0;
            break;
        case (ABSMOTALARM & 0xFFFF) :             //0xE980    //编码器算法异常警告
            UNI_WarnReg.bit.ABSMOTALARMFLG = 0;
            break;
        case (NOTWARN & 0xFFFF) :                  //0xE952    负向超程警告解除
            UNI_WarnReg.bit.NOTWARNFLG = 0;
            break;
        case (PNOTWARN & 0xFFFF) :                 //0xE950    正向超程警告解除
            UNI_WarnReg.bit.PNOTWARNFLG = 0;
            break;
        case (PRAMSVWARN & 0xFFFF) :               //0xE942    参数存储频繁警告
            UNI_WarnReg.bit.PRAMSVWARNFLG = 0;
            break;
        case (PCHGDWARN & 0xFFFF) :                //0xE941    需要重新接通电源的参数变解除
            UNI_WarnReg.bit.MCHGDWARNFLG = 0;
            break;
        case (MTOFFLINE & 0xFFFF) :                //0xE939    电源缺相警告解除
            UNI_WarnReg.bit.MTOFFLINEFLG = 0;
            break;
        case (RBTOOSMALLWARN & 0xFFFF) :           //0xE922    外置再生泄放电阻阻值过小解除
            UNI_WarnReg.bit.RBTOOSMALLFLG = 0;
            break;
       case (BLKOVERCURRENT & 0xFFFF) :           //0xE921    外置再生泄放电阻电流过大解除     
			UNI_WarnReg.bit.BLKOVERCURRENTFLG=0;
			break;
        case (RBOVERLOADWARN & 0xFFFF) :           //0xE920    再生制动电阻过载解除
            UNI_WarnReg.bit.RBOVERLOADFLG = 0;
            break;
        case (EMERGENCYSTOPWARN & 0xFFFF):         //0xE900    紧急停机警告
            UNI_WarnReg.bit.EMSTOPWARNFLG = 0;
            break;
        case (AIZEROBIASOVWARN & 0xFFFF) :         //0xE831    AI零偏过大 
            UNI_WarnReg.bit.AIZEROBIASOVWARNFLG = 0;
            break;
        case (ORIGINOVERTIME & 0xFFFF):            //0xE601    原点复归回零超时错误
            UNI_WarnReg.bit.ORIGINOVERTIMEFLG = 0;
            break;
        case (PULSOUTSETWARN & 0xFFFF) :           //0xE110    分频脉冲输出设定故障
            UNI_WarnReg.bit.PULSOUTSETWARNFLG = 0;
            break;
		case (MTOLWARN& 0xFFFF):		           //0xE909   电机过载警告
			UNI_WarnReg.bit.MTOLWARNFLG= 0;
			break;
		case (ENCDBATWARN& 0xFFFF):		           //0xE730 编码器电池报警
			UNI_WarnReg.bit.ENCDBATWARNFLG= 0;
			break;
		case (ENCDTXCOMMERR& 0xFFFF):		       //0xE732 编码器TX端通讯校验错误
			UNI_WarnReg.bit.ENCDTXCOMMERRFLG= 0;
			break;
		case (ENCODEROT& 0xFFFF):		           //0xE760 编码器过热
			UNI_WarnReg.bit.ENCODEROTFLG= 0;
			break;
        default:
            break;
    }
}


/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  子函数:    
  描述:  
********************************************************************************/ 
void WarnAutoClr(Uint32 PostWarnCode)
{
    Uint16  HighLevelWarn = 0xEFFF;
    
    PostWarnCode = PostWarnCode & 0xFFFF;

    if((UNI_WarnReg.all == 0) || (PostWarnCode < 0xE000))
    {
        return;
    }

    WarnRegClr(PostWarnCode);

   if ( (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000)&&(UNI_WarnReg.all == 0) )
   {
        STR_FUNC_Gvar.Monitor.HighLevelErrCode = 0;

        STR_FUNC_Gvar.Monitor.ErrCode = 0;

        STR_FUNC_Gvar.Monitor.DovarReg_Warn = INVALID;
   }
   else if ( (STR_FUNC_Gvar.Monitor.HighLevelErrCode > 0xE000)&&(UNI_WarnReg.all != 0) )
   {
        if (UNI_WarnReg.bit.IDENTIFYWARNFLG == 1)   //0xEA40 参数辨识失败
        {
            HighLevelWarn = (HighLevelWarn >= (IDENTIFYWARN & 0xFFFF)) ? (IDENTIFYWARN & 0xFFFF) : HighLevelWarn;
        }
	    #if CAN_ENABLE_SWITCH
        if (UNI_WarnReg.bit.CANPASSIVEERRFLG  == 1)   //0xE996  总线被动错误警告
        {
            HighLevelWarn = (HighLevelWarn >= (CANPASSIVEERR & 0xFFFF)) ? (CANPASSIVEERR & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.CANRECOVBUSOFFFLG  == 1)  //0xE995   总线恢复警告
        {
            HighLevelWarn = (HighLevelWarn >= (CANRECOVBUSOFF & 0xFFFF)) ? (CANRECOVBUSOFF & 0xFFFF) : HighLevelWarn;
        }
		#endif

	    #if (CAN_ENABLE_SWITCH||ECT_ENABLE_SWITCH)
        if (UNI_WarnReg.bit.ODVALUEERRFLG  == 1) //0xE998  对象字典数值设置错误
        {
            HighLevelWarn = (HighLevelWarn >= (ODVALUEERR & 0xFFFF)) ? (ODVALUEERR & 0xFFFF) : HighLevelWarn;
        }
        #endif
        if (UNI_WarnReg.bit.COMMADDRCONFLICTFLG == 1)   //0xE994      CANLINK地址冲突
        {
            HighLevelWarn = (HighLevelWarn >= (COMMADDRCONFLICT & 0xFFFF)) ? (COMMADDRCONFLICT & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.POWPLWARNFLG == 1)          //0xE990   电源缺相警告  
        {
            HighLevelWarn = (HighLevelWarn >= (POWERPLWARN & 0xFFFF)) ? (POWERPLWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.ABSMOTALARMFLG == 1)       //0xE980    //编码器算法异常警告
        {
            HighLevelWarn = (HighLevelWarn >= (ABSMOTALARM & 0xFFFF)) ? (ABSMOTALARM & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.NOTWARNFLG == 1)            //0xE952    负向超程警告
        {
            HighLevelWarn = (HighLevelWarn >= (NOTWARN & 0xFFFF)) ? (NOTWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.PNOTWARNFLG == 1)           //0xE950    正向超程警告
        {
            HighLevelWarn = (HighLevelWarn >= (PNOTWARN & 0xFFFF)) ? (PNOTWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.PRAMSVWARNFLG == 1)          //0xE942    参数存储频繁警告 
        {
            HighLevelWarn = (HighLevelWarn >= (PRAMSVWARN & 0xFFFF)) ? (PRAMSVWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.MCHGDWARNFLG == 1)          //0xE941    需要重新接通电源的参数变更 
        {
            HighLevelWarn = (HighLevelWarn >= (PCHGDWARN & 0xFFFF)) ? (PCHGDWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.MTOFFLINEFLG == 1)          //0xE939    电机动力线断线    
        {
            HighLevelWarn = (HighLevelWarn >= (MTOFFLINE & 0xFFFF)) ? (MTOFFLINE & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.RBTOOSMALLFLG == 1)         //0xE922    外接再生制动电阻阻值过小 
        {
            HighLevelWarn = (HighLevelWarn >= (RBTOOSMALLWARN & 0xFFFF)) ? (RBTOOSMALLWARN & 0xFFFF) : HighLevelWarn;
        }
		if (UNI_WarnReg.bit.BLKOVERCURRENTFLG == 1)         // 0xE921    再生制动电阻电流过大
        {
            HighLevelWarn = (HighLevelWarn >= (BLKOVERCURRENT & 0xFFFF)) ? (BLKOVERCURRENT & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.RBOVERLOADFLG == 1)         // 0xE920    再生制动电阻过载 
        {
            HighLevelWarn = (HighLevelWarn >= (RBOVERLOADWARN & 0xFFFF)) ? (RBOVERLOADWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.MTOLWARNFLG == 1)           //0xE909 电机过载警告
        {
            HighLevelWarn = (HighLevelWarn >= (MTOLWARN & 0xFFFF)) ? (MTOLWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.EMSTOPWARNFLG == 1)         // 0xE900    紧急停机警告 
        {
            HighLevelWarn = (HighLevelWarn >= (EMERGENCYSTOPWARN & 0xFFFF)) ? (EMERGENCYSTOPWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.AIZEROBIASOVWARNFLG == 1)      //0xE831 AI零偏过大
        {
            HighLevelWarn = (HighLevelWarn >= (AIZEROBIASOVWARN & 0xFFFF)) ? (AIZEROBIASOVWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.ENCODEROTFLG == 1)           //0xE760 编码器过热
        {
            HighLevelWarn = (HighLevelWarn >= (ENCODEROT & 0xFFFF)) ? (ENCODEROT & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.ENCDTXCOMMERRFLG == 1)      //0xE732 编码器TX端通讯校验错误
        {
            HighLevelWarn = (HighLevelWarn >= (ENCDTXCOMMERR & 0xFFFF)) ? (ENCDTXCOMMERR & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.ENCDBATWARNFLG == 1)         //0xE730 编码器电池报警 
        {
            HighLevelWarn = (HighLevelWarn >= (ENCDBATWARN & 0xFFFF)) ? (ENCDBATWARN & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.ORIGINOVERTIMEFLG == 1)       //0xE601 原点复归回零超时错误
        {
            HighLevelWarn = (HighLevelWarn >= (ORIGINOVERTIME & 0xFFFF)) ? (ORIGINOVERTIME & 0xFFFF) : HighLevelWarn;
        }
        if (UNI_WarnReg.bit.PULSOUTSETWARNFLG == 1)      //0xE110 分频脉冲输出设定故障
        {
            HighLevelWarn = (HighLevelWarn >= (PULSOUTSETWARN & 0xFFFF)) ? (PULSOUTSETWARN & 0xFFFF) : HighLevelWarn;
        }


        STR_FUNC_Gvar.Monitor.HighLevelErrCode = (HighLevelWarn != 0xEFFF)?  HighLevelWarn : STR_FUNC_Gvar.Monitor.HighLevelErrCode;
        STR_FUNC_Gvar.Monitor.ErrCode = STR_FUNC_Gvar.Monitor.HighLevelErrCode; 
   }
}
/*******************************************************************************
  函数名: void ErrProcess_MaimLoop(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 故障处理主循环调度函数
    2.
********************************************************************************/
void ErrProcess_MaimLoop(void)
{
    AlrmReset();            //故障复位
    ErrMsgDoProcess();      //故障DO信号处理
    ErrRecordProcess();     //故障记录处理
    ErrRecordMsgDisplay();  //H0B组故障功能码显示
}


/*******************************************************************************
  函数名:  void AlrmReset(STR_SERVO_MONITOR *p)
  输入:    STR_FUNC_Gvar.DivarRegLw.bit.AlmRst 功能码 ，STR_FUNC_Gvar.Monitor.HighLevelErrCode
           STR_ServoMonitor.ErrorFlag.bit.Err1UnRstFlag
  输出:    STR_ServoMonitor.ErrorVar.XXX ; STR_FUNC_Gvar.DovarReg.bit.XXX ; WarnReg.all
  子函数:  无     
  描述:    根据Di或功能码故障复位使能信号，进行故障信息复位操作，将各故障信息
           进行相应的清除。        
********************************************************************************/ 
Static_Inline void AlrmReset(void)
{
    static Uint8 Posclrflag = 0;
	
	//故障记录存储期间，不能进行故障复位
    if(1 == STR_ServoMonitor.ErrorFlag.bit.ErrMsgRecordEn) return;
    //只有在故障复位Di或者功能码使能时才进行复位操作
	#if ECT_ENABLE_SWITCH
    if( ((STR_FUNC_Gvar.DivarRegLw.bit.AlmRst == VALID)||(AuxFunCodeUnion.code.FA_DefaultRst == 1) || (DeviceControlVar.AlmRstFlag == 1)) &&
        (STR_PUB_Gvar.MTRAlmRst == 0) ) 
    {
        AuxFunCodeUnion.code.FA_DefaultRst = 0;
        DeviceControlVar.AlmRstFlag = 0;

        //伺服Off时可以进行故障复位，或者警告时且伺服ON时也能复位，有故障1或故障2时且伺服On时也不能复位
		if(  ((VALID == STR_FUNC_Gvar.DiDoOutput.Son_Flt) ||
            (1 == STR_ServoMonitor.RunStateFlag.bit.InnerServoSon)
            || (1 == STR_ServoMonitor.RunStateFlag.bit.CanopenServoSon)) 
          && (STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0xE000) )
		{
            return;
        }
        
        STR_PUB_Gvar.MTRAlmRst = 1;
    }
    #else
    if( ((STR_FUNC_Gvar.DivarRegLw.bit.AlmRst == VALID)||(AuxFunCodeUnion.code.FA_DefaultRst == 1)) &&
        (STR_PUB_Gvar.MTRAlmRst == 0) ) 
	{
        AuxFunCodeUnion.code.FA_DefaultRst = 0;

        //伺服Off时可以进行故障复位，或者警告时且伺服ON时也能复位，有故障1或故障2时且伺服On时也不能复位
		if(  ((VALID == STR_FUNC_Gvar.DiDoOutput.Son_Flt) ||
            (1 == STR_ServoMonitor.RunStateFlag.bit.InnerServoSon)) 
          && (STR_FUNC_Gvar.Monitor.HighLevelErrCode < 0xE000) )
		{
            return;
        }
        STR_PUB_Gvar.MTRAlmRst = 1;
    }

#endif        
    //当前无故障时,H0B45清零
    if((AuxFunCodeUnion.code.DP_InnerErrCode != 0) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode == 0))
    {
        AuxFunCodeUnion.code.DP_InnerErrCode = 0;
		
		#if ECT_ENABLE_SWITCH
	        ObjectDictionaryStandard.ComEntryErrCode.ErrorCode = 0;
		STR_SerErrCode.ServorErrorCode = 0;
		#endif

    }

    if(STR_PUB_Gvar.MTRAlmRst == 2)   //等待MTR模块故障清除执行完成
    {
        STR_PUB_Gvar.MTRAlmRst = 0;
        
        /* 清除Err2RstFlag，Err1RstFlag，WarnFlag 可复位故障及警告标志位 */
        STR_ServoMonitor.ErrorFlag.all = STR_ServoMonitor.ErrorFlag.all & ALRMRST;
        STR_FUNC_Gvar.Monitor.DovarReg_Warn = INVALID;
         
        if(STR_ServoMonitor.ErrorFlag.bit.Err1UnRstFlag == INVALID)   //若没有不可复位的故障1，则清除故障显示码以及Do故障变量 
        {
            STR_FUNC_Gvar.Monitor.DovarReg_Alm = INVALID;
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 0;
        }

		if((STR_FUNC_Gvar.Monitor.HighLevelErrCode>0xE000)&&(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))Posclrflag = 0;
		else  Posclrflag = 1;

        if(STR_FUNC_Gvar.Monitor.HighLevelErrCode >0x1000 ) //若最高级别的故障为可复位故障，则清除故障信息
        {
            
			#if ECT_ENABLE_SWITCH
	        ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Fault = 0;
	        ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Warning = 0;
	        ObjectDictionaryStandard.ComEntryErrCode.ErrorCode = 0;
			STR_SerErrCode.ServorErrorCode = 0;
			#endif
			
            STR_FUNC_Gvar.Monitor.HighLevelErrCode = 0;	  //清除最高级别故障      
            STR_FUNC_Gvar.Monitor.ErrCode = 0;            //清除当前故障码

            STR_ServoMonitor.ErrorVar.ErrCodeBuff[0] = 0;     //清除故障缓存内容 
            STR_ServoMonitor.ErrorVar.ErrCodeBuff[1] = 0;
            STR_ServoMonitor.ErrorVar.ErrCodeBuff[2] = 0;
            STR_ServoMonitor.ErrorVar.ErrCodeBuff[3] = 0;

            STR_FUNC_Gvar.Monitor.DovarReg_Alm = 0;
            STR_FUNC_Gvar.Monitor.DovarReg_Warn = 0;
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 0;

            STR_FUNC_Gvar.MonitorFlag.bit.ErrResetExecut = 1;   //故障复位已执行标志位
        }

        UNI_WarnReg.all = 0x0; //清除警告标志位
        if(Posclrflag==1)
		{
		 	STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1; //位置环相关参数清零
		 	Posclrflag = 0;
		}
		
    }
}

/*******************************************************************************
  函数名:  void ErrMsgDoProcess(STR_SERVO_MONITOR *p)
  输入:   STR_FUNC_Gvar.Monitor.HighLevelErrCode 
  输出:   STR_FUNC_Gvar.Monitor.DovarReg_AlmCode 
  子函数: 无      
  描述:   把当前最高级别的故障码类型信息，用Do输出出去
         
********************************************************************************/ 
Static_Inline void ErrMsgDoProcess(void)
{
    Uint16 ErrCodeTemp = 0;
	if( STR_ServoMonitor.ErrorFlag.bit.ErrDoRecFlg == 1 )           
	{
        ErrCodeTemp = (0x0FFF & STR_FUNC_Gvar.Monitor.HighLevelErrCode); //获取最高级故障码

        /* 故障码Do输出显示 */                                   // AL2 AL1 AL0
        if( ((ErrCodeTemp >= 0x600) && (ErrCodeTemp < 0x700))||
	        ((ErrCodeTemp >= 0x950) && (ErrCodeTemp < 0x980)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 0;              // L   L   L

	    if( ((ErrCodeTemp >= 0x300) && (ErrCodeTemp < 0x400))||
	        ((ErrCodeTemp >= 0x930) && (ErrCodeTemp < 0x940))||
            ((ErrCodeTemp >= 0xB00) && (ErrCodeTemp < 0xC00)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 4;              // H   L   L 

	    if( ((ErrCodeTemp >= 0x500) && (ErrCodeTemp < 0x600))||
	        ((ErrCodeTemp >= 0xA00) && (ErrCodeTemp < 0xB00)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 2;              // L   H   L

        if( ((ErrCodeTemp >= 0x200) && (ErrCodeTemp < 0x300))||
	        ((ErrCodeTemp >= 0x901) && (ErrCodeTemp < 0x920)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 6;              // H   H   L

        if( (ErrCodeTemp >= 0x980) && (ErrCodeTemp < 0x9A0) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 1;              // L   L   H

	    if( ((ErrCodeTemp >= 0xC00) && (ErrCodeTemp < 0xF00))||
	        ((ErrCodeTemp >= 0x920) && (ErrCodeTemp < 0x930)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 5;              // H   L   H

        if( ((ErrCodeTemp >= 0x400) && (ErrCodeTemp < 0x500))||
	        ((ErrCodeTemp >= 0x940) && (ErrCodeTemp < 0x950)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 3;              // L   H   H

	    if( ((ErrCodeTemp >= 0x100) && (ErrCodeTemp < 0x200))||
	        ((ErrCodeTemp >= 0x700) && (ErrCodeTemp <= 0x900)) )
            STR_FUNC_Gvar.Monitor.DovarReg_AlmCode = 7;              // H   H   H
    
        STR_ServoMonitor.ErrorFlag.bit.ErrDoRecFlg = 0;   //故障Do输出标志位清零   
	}
}

/*******************************************************************************
  函数名: void ErrRecordProcess(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 故障记录处理
    2.
********************************************************************************/
Static_Inline void ErrRecordProcess(void)
{
    static Uint16 Step = 0;
    Uint32 Counter = 0;
    Uint16 IndexStart = 0;
    Uint16 IndexEnd = 0;      

    if(1 == STR_ServoMonitor.ErrorFlag.bit.ErrMsgRecordEn)
    {       
        
		
		switch(Step)       
        {
            case 0:
                //选用汇川20位编码器
                if( ((FunCodeUnion.code.MT_EncoderSel & 0xff) == 0x13) 
                 && (STR_ServoMonitor.ErrorFlag.bit.RdEncEepromEn == 1) )
                {
                    if(STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == DISPWM) 
                    {
                        AuxFunCodeUnion.code.DP_EncStBit1 = 0;
                        //AuxFunCodeUnion.code.DP_EncStBit2 = 0;
                        AuxFunCodeUnion.code.MT_OperAbsROM = 2;
                        Step = 1;
                        STR_ServoMonitor.ErrorFlag.bit.RdEncEepromEn = 0;
                    }
                    else
                    {
                        Step = 0;
                    }
                }
                else
                {
                    Step = 2;
                }
                break;

            case 1:
                if(2 != AuxFunCodeUnion.code.MT_OperAbsROM)
                {
                    FunCodeUnion.code.ErrMsgBuff[FunCodeUnion.code.ES_ErrMsgTail].EncStBit1  = AuxFunCodeUnion.code.DP_EncStBit1;
                    //FunCodeUnion.code.ErrMsgBuff[FunCodeUnion.code.ES_ErrMsgTail].EncStBit2  = AuxFunCodeUnion.code.DP_EncStBit2;
                    
                    Step = 2;
                }
                break;

            case 2:
                Step = 0;
                STR_ServoMonitor.ErrorFlag.bit.ErrMsgRecordEn = 0;
                
                STR_ServoMonitor.ErrorFlag.bit.RdEncEepromEn = 0;

                //存储伺服运行时间 故障序号
                IndexStart = GetCodeIndex(FunCodeUnion.code.ES_ServoTime_L);   
                IndexEnd = GetCodeIndex(FunCodeUnion.code.ES_ErrMsgTail);
                SaveToEepromSeri(IndexStart,IndexEnd);
            
                //存储故障存储数组数据
                IndexStart = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[0].ErrCode);   //故障存储数组起始序号
                IndexEnd = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[4].FpgaTmFlt);      //故障存储数组结束序号
                SaveToEepromSeri(IndexStart,IndexEnd);
            
                IndexStart = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[5].ErrCode);   //故障存储数组起始序号
                IndexEnd = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[9].FpgaTmFlt);      //故障存储数组结束序号
                SaveToEepromSeri(IndexStart,IndexEnd);                
                break;

            default:
                Step = 0;
                break;        
        }
        return;    
    }

    if(FunCodeUnion.code.BP_InitServo != 2)    return;  

    //清除所有的记录
    IndexStart = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[0].ErrCode);     //故障存储数组起始序号
    IndexEnd = HOffSv00INDEX - 1;                                           //故障存储数组结束序号

    for(Counter=IndexStart;Counter<=IndexEnd;Counter++)    
    {
        FunCodeUnion.all[Counter] = 0;
    } 

    //存储伺服运行时间 故障序号
    IndexStart = GetCodeIndex(FunCodeUnion.code.ES_ServoTime_L);   
    IndexEnd = GetCodeIndex(FunCodeUnion.code.ES_ErrMsgTail);
    SaveToEepromSeri(IndexStart,IndexEnd);

    //存储故障存储数组数据
    IndexStart = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[0].ErrCode);   //故障存储数组起始序号
    IndexEnd = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[4].FpgaTmFlt);      //故障存储数组结束序号
    SaveToEepromSeri(IndexStart,IndexEnd); 

    IndexStart = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[5].ErrCode);   //故障存储数组起始序号
    IndexEnd = GetCodeIndex(FunCodeUnion.code.ErrMsgBuff[9].FpgaTmFlt);      //故障存储数组结束序号
    SaveToEepromSeri(IndexStart,IndexEnd); 

    FunCodeUnion.code.BP_InitServo = 0;
}

/*******************************************************************************
  函数名: GetWarningFlag(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 获取警告标志
********************************************************************************/
void GetWarningFlag(void)
{
	 #if ECT_ENABLE_SWITCH
	 if(UNI_WarnReg.all!=0)
	 {
	     ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Warning = 1;
	 }
	 else
	 {
	     ObjectDictionaryStandard.DeviceControl.StatusWord.bit.Warning = 0;
	 }
	 
	 #endif
}





/*******************************************************************************
  函数名:  Static_Inline void ErrRecordMsgDisplay()
  输入:    功能码
  输出:    功能码
  子函数:  无    
  描述:  根据功能码的选择将十组故障时存储的信息，用H0b34显示出来。
         当无故障时且H0b33=0(显示当前故障)时，显示的故障信息应该为零
********************************************************************************/ 
Static_Inline void ErrRecordMsgDisplay()     //放在MonitorShow()函数中调用
{
    int16 ErrIndex = 0;  

    if((STR_FUNC_Gvar.Monitor.ErrCode == 0) && (AuxFunCodeUnion.code.DP_DefotNo == 0))
    {
        AuxFunCodeUnion.code.DP_ErrCode      = 0;
        AuxFunCodeUnion.code.DP_DefotTime_Lo = 0;
        AuxFunCodeUnion.code.DP_DefotTime_Hi = 0;
        AuxFunCodeUnion.code.DP_CurtU_Defot  = 0;
        AuxFunCodeUnion.code.DP_CurtV_Defot  = 0;
        AuxFunCodeUnion.code.DP_Spd_Defot    = 0;
        AuxFunCodeUnion.code.DP_Udc_Defot    = 0;
        AuxFunCodeUnion.code.DP_DI_Defot     = 0;
        AuxFunCodeUnion.code.DP_DO_Defot     = 0;

        AuxFunCodeUnion.code.DP_FpgaAbsRomErr_Defot = 0;
        AuxFunCodeUnion.code.DP_FpgaSysState_Defot  = 0;
        AuxFunCodeUnion.code.DP_FpgaSysErr_Defot    = 0;
        AuxFunCodeUnion.code.DP_EncStBit1_Defot     = 0;
        //AuxFunCodeUnion.code.DP_EncStBit2_Defot     = 0;
        AuxFunCodeUnion.code.DP_InnerErrCode_Defot  = 0;
        AuxFunCodeUnion.code.DP_FpgaTmFlt_Defot     = 0;
        return;
    }

    //若当前没有故障时，ES_ErrMsgTail表示的是上一次的故障信息，
    //再+1表示的是当前最近一次的故障，但实际上当前最近故障为零无故障，所以后面将故障显示信息清零。
    if(STR_FUNC_Gvar.Monitor.ErrCode == 0)
    {
        ErrIndex = (FunCodeUnion.code.ES_ErrMsgTail - AuxFunCodeUnion.code.DP_DefotNo + 11) % 10;
    }
    else   //有故障时根据H0b33的值显示需要显示的第几次故障信息
    {
        ErrIndex = (FunCodeUnion.code.ES_ErrMsgTail - AuxFunCodeUnion.code.DP_DefotNo + 10) % 10;
    }

    //H0B34 故障码
    AuxFunCodeUnion.code.DP_ErrCode      = FunCodeUnion.code.ErrMsgBuff[ErrIndex].ErrCode;

    //H0B35 所选故障时间戳
    AuxFunCodeUnion.code.DP_DefotTime_Lo = FunCodeUnion.code.ErrMsgBuff[ErrIndex].TimeStampL;
    AuxFunCodeUnion.code.DP_DefotTime_Hi = FunCodeUnion.code.ErrMsgBuff[ErrIndex].TimeStampH;

    //H0B_38  所选故障时当前U相瞬时电流 0.01A
    AuxFunCodeUnion.code.DP_CurtU_Defot  = FunCodeUnion.code.ErrMsgBuff[ErrIndex].Iu;

    //H0B_39  所选故障时当前V相瞬时电流 0.01A
    AuxFunCodeUnion.code.DP_CurtV_Defot  = FunCodeUnion.code.ErrMsgBuff[ErrIndex].Iv;

    //H0B37 所选故障时的当前转速
    AuxFunCodeUnion.code.DP_Spd_Defot    = FunCodeUnion.code.ErrMsgBuff[ErrIndex].Spd;

    //H0B40 所选故障时的母线电压
    AuxFunCodeUnion.code.DP_Udc_Defot    = FunCodeUnion.code.ErrMsgBuff[ErrIndex].Udc;

    //H0B41 故障时输入端子状态
    AuxFunCodeUnion.code.DP_DI_Defot     = FunCodeUnion.code.ErrMsgBuff[ErrIndex].DiState;

    //H0B42 所选故障时输出端子状态
    AuxFunCodeUnion.code.DP_DO_Defot     = FunCodeUnion.code.ErrMsgBuff[ErrIndex].DoState;

    //H0B_46  所选故障时FPGA给出绝对编码器故障信息
    AuxFunCodeUnion.code.DP_FpgaAbsRomErr_Defot = FunCodeUnion.code.ErrMsgBuff[ErrIndex].FpgaAbsRomErr;

    //H0B_47  所选故障时FPGA给出的系统状态信息
    AuxFunCodeUnion.code.DP_FpgaSysState_Defot  = FunCodeUnion.code.ErrMsgBuff[ErrIndex].FpgaSysState;

    //H0B_48  所选故障时FPGA给出的系统故障信息
    AuxFunCodeUnion.code.DP_FpgaSysErr_Defot     = FunCodeUnion.code.ErrMsgBuff[ErrIndex].FpgaSysErr;

    //H0B_49  所选故障时20位编码器状态位
    AuxFunCodeUnion.code.DP_EncStBit1_Defot     = FunCodeUnion.code.ErrMsgBuff[ErrIndex].EncStBit1;

    //H0B_50  所选故障时20位编码器状态位
    //AuxFunCodeUnion.code.DP_EncStBit2_Defot     = FunCodeUnion.code.ErrMsgBuff[ErrIndex].EncStBit2;

    //H0B_51  所选故障时内部故障码
    AuxFunCodeUnion.code.DP_InnerErrCode_Defot  = FunCodeUnion.code.ErrMsgBuff[ErrIndex].InnerErrCode;

    //H0B_52  所选故障时故障时FPGA超时故障标准位
    AuxFunCodeUnion.code.DP_FpgaTmFlt_Defot  = FunCodeUnion.code.ErrMsgBuff[ErrIndex].FpgaTmFlt;
}
/********************************* END OF FILE *********************************/

