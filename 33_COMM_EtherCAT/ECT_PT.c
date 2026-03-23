/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "ECT_PT.h"
#include "FUNC_InterfaceProcess.h"
#include "CANopen_DeviceCtrl.h"
#include "FUNC_GlobalVariable.h" 
#include "CANopen_OD.h"
#include "CANopen_Pub.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_FunCode.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
const int16 SineToqTable_1k[16]=
{
    0,   
    95, 
    176,  
    230,  
    250,  
    230,  
    176,   
    95,    
    0,  
    -95, 
    -176, 
    -230, 
    -250,
    -230,
    -176,  
    -95  
};

const int16 SineToqTable_1dot5k[11]=
{
    0,  
    138,  
    230, 
    245,  
    176,   
    49,  
    -95, 
    -207,
    -249, 
    -208,  
    -96
};

const int16 SineToqTable_2k[8]=
{
    0,  
    176, 
    250,  
    176,    
    0,
    -176,
    -250,
    -176   
};

const int16 SineToqTable_2dot5k[32]=
{
    0,  
    207, 
    230,   
    48, 
    -176, 
    -245,  
    -95, 
    138,  
    250,  
    138,  
    -95, 
    -245, 
    -176,   
    48,
    230,  
    207,   
    0, 
    -207, 
    -230,  
    -48,  
    176,  
    245,  
    95, 
    -138, 
    -250, 
    -138,   
    95,  
    245,
    176,  
    -48, 
    -230, 
    -207    
};

const int16 SineToqTable_3k[16]=
{
    0,  
    230,  
    176,  
    -95,
    -250,  
    -95,  
    176,  
    230,   
     0, 
    -231, 
    -176,   
    95,  
    250,  
    95,
    -177, 
    -230
};

const int16 SineToqTable_3dot5k[32]=
{
    0,  
    245,   
    95, 
    -207,
    -176,  
    138,  
    230,  
    -48, 
    -250,  
    -48, 
    231,  
    138, 
    -176,
    -207,
    95, 
    245,   
    0, 
    -245, 
    -95,  
    208, 
    176,
    -139,
    -230,  
    49,  
    249,   
    48, 
    -231,
    -138,
    177, 
    207,  
    -96, 
    -245
};

const int16 SineToqTable_4k[4]=
{
    0,  
    250, 
    0,
    -250
};



/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
STR_ECTPTVAR STR_ECTPTVar;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline DeltaToqSource(void);
Static_Inline void ToqSoftStart(void);
Static_Inline int32 ECTToqCmdTest(void);
void ECTPTDisplay(void);//转矩模式变量显示
/*******************************************************************************
  函数名: extern  void InitECTPTMode(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   转矩模式初始化
********************************************************************************/






/*******************************************************************************
  函数名: extern  void GetECTPTRef(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   获得转矩模式下转矩指令来源
********************************************************************************/
int32 GetECTPTRef(void)
{
    int32 ToqCmdValue      = 0;

    STR_FUNC_Gvar.OscTarget.InputTarToq = (int32)(int16)ObjectDictionaryStandard.ProToqMode1.TargetToq;

    //暂停、快速停车命令有效时，目标转矩为0
    if((DeviceControlVar.OEMStatus != STATUS_OPERENABLE)||
        (STR_CanSyscontrol.ModeSwitchFlag == 1))
    {
        ToqCmdValue = 0;
    }
    //获取目标转矩    
    else
    {
        if(STR_CanSyscontrol.Mode == ECTTOQMOD)
        {
            ToqCmdValue = (int32)(((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 
                          * ((int64)((int16)ObjectDictionaryStandard.ProToqMode1.TargetToq)))>>12);
        }
        else if(STR_CanSyscontrol.Mode == ECTCSTMOD)
        {
            ToqCmdValue = (int32)((int16)ObjectDictionaryStandard.ProToqMode1.TargetToq)
                           + (int32)((int16)ObjectDictionaryStandard.CstOffset.ToqOff);
            ToqCmdValue = (int32)(((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ((int64)ToqCmdValue))>>12);
            //ToqCmdValue = ECTToqCmdTest();

        }
        //607E的bit5 = 1，转矩指令反向
        if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x20)== 0x20)
    	{
    	    ToqCmdValue = 0 - ToqCmdValue;
    	}

        /*转矩指令方向切换*/
        if(STR_FUNC_Gvar.DivarRegLw.bit.TOQDirSel == 1)   //DI 的#112变量名的命名规则在G2中还需重新定义
        {                            
            ToqCmdValue = (-1L) * ToqCmdValue;                    
        }
    }
    
    STR_ECTPTVar.TargetToq = ToqCmdValue;

	if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)&&(STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus == ENPWM))   //伺服On使能时，进行转矩调节器输入指令赋值
    {
        if(/*STR_FUNC_Gvar.MonitorFlag.bit.ESMState==*/1)
        {
            ToqSoftStart();
            if(STR_CanSyscontrol.Mode == ECTCSTMOD)
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
        }
        else
        {
            STR_ECTPTVar.ToqSoftStartRefLatch = 0;
            STR_ECTPTVar.ToqSoftStartRefLatchQ12 = 0;
            if(STR_CanSyscontrol.Mode == ECTCSTMOD)ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;
        }


	}
	else
	{
        STR_ECTPTVar.ToqSoftStartRefLatchQ12 = 0/*((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 
                                        * ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb))*/;
        STR_ECTPTVar.ToqSoftStartRefLatch =  0/*((int32)((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 
                                        * ((int64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb))>>12*/;
        //drive follow the command value
        if((STR_CanSyscontrol.Mode == ECTCSTMOD))ObjectDictionaryStandard.DeviceControl.StatusWord.bit.OperationModeSpecific1 = 0;
	}
    
    return STR_ECTPTVar.ToqSoftStartRefLatch;

}


/*******************************************************************************
  函数名: Static_Inline DeltaToqSource(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   获得转矩模式下每个转矩环转矩指令增量
********************************************************************************/
Static_Inline DeltaToqSource(void)
{
    int64 ToqSlope= 0;
//    int64 DeltaToqQ12 = 0;
    int64 PosDeltaToqMax = 0;
    int64 NegDeltaToqMax = 0;
    int64 DeltaForward_Q12 =0;
    int64 DeltaBackword_Q12 =0;
    int32 MaxToqPos = 0;
    int32 MaxToqNeg =0;


    PosDeltaToqMax  = (int64)UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqPosLmt_F; 
    NegDeltaToqMax  = (-1) * ((int64)UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.IqNegLmt_F);

    if(STR_ECTPTVar.TargetToq > 0)
    {
        MaxToqPos = ((STR_ECTPTVar.TargetToq >= PosDeltaToqMax) ? PosDeltaToqMax : STR_ECTPTVar.TargetToq);

        MaxToqNeg = NegDeltaToqMax;
    }
    else if(STR_ECTPTVar.TargetToq < 0)
    {
        MaxToqPos = PosDeltaToqMax;
        MaxToqNeg = ((((-1)*STR_ECTPTVar.TargetToq) >= NegDeltaToqMax) ? NegDeltaToqMax : ((-1)*STR_ECTPTVar.TargetToq));
    }
	else
	{
        MaxToqPos = PosDeltaToqMax;
        MaxToqNeg = NegDeltaToqMax;
	}

    ToqSlope = (int64)ObjectDictionaryStandard.ProToqMode2.ToqSlope;

    if(ToqSlope == 0)ToqSlope = 1;
    
    //① 首先计算加减速加速度
    
    //正向最大转矩对应的1s转矩百分比增量
    PosDeltaToqMax  = PosDeltaToqMax  * (int64)STR_FUNC_Gvar.System.ToqFreq;

    //负向最大转矩对应的1s转矩百分比增量
    NegDeltaToqMax  = NegDeltaToqMax * (int64)STR_FUNC_Gvar.System.ToqFreq;

    //为了方便Uint32格式数据的中间过程处理不溢出，在此处进行判断
    //若转矩指令百分比增量大于电机最大转矩百分比，则强制为最大转矩百分比
    if(ToqSlope > PosDeltaToqMax)
    {
        DeltaForward_Q12 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * PosDeltaToqMax)
                            /(int64)STR_FUNC_Gvar.System.ToqFreq;

    }
    else
    {
        DeltaForward_Q12 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ToqSlope)
                            /(int64)STR_FUNC_Gvar.System.ToqFreq;
    }

    
    if(ToqSlope > NegDeltaToqMax)
    {
        DeltaBackword_Q12 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * NegDeltaToqMax)
                            /(int64)STR_FUNC_Gvar.System.ToqFreq;

    }
    else
    {
        DeltaBackword_Q12 = ((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ToqSlope)
                            /(int64)STR_FUNC_Gvar.System.ToqFreq;
    }
    
    
    STR_ECTPTVar.DeltaForward_Q12 = DeltaForward_Q12;
    STR_ECTPTVar.DeltaBackword_Q12 = DeltaBackword_Q12;

    //② 区分不同情况下的加速度
    //快速停车
    if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == VALID)
    {
        if((STR_ServoMonitor.StopCtrlVar.CanQuickStopMode == 5)||
            (STR_ServoMonitor.StopCtrlVar.CanQuickStopMode == 4))
        {
            STR_ECTPTVar.DeltaForward_Q12 = DeltaForward_Q12;
            STR_ECTPTVar.DeltaBackword_Q12 = DeltaBackword_Q12;
        } 
    }
    //暂停
    else if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == VALID)
    {
        if((STR_ServoMonitor.StopCtrlVar.CanHaltStopMode == 5)||
            (STR_ServoMonitor.StopCtrlVar.CanHaltStopMode == 4))
        {
            STR_ECTPTVar.DeltaForward_Q12 = DeltaForward_Q12;
            STR_ECTPTVar.DeltaBackword_Q12 = DeltaBackword_Q12;
        } 
    }
    //周期同步转矩模式除快速停机、暂停，正常运行时无减速时间    
    else if((STR_CanSyscontrol.Mode == ECTCSTMOD)&&((STR_CanSyscontrol.ModeSwitchFlag != 1))) 
    {
        STR_ECTPTVar.DeltaForward_Q12 = (int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * MaxToqPos;
        STR_ECTPTVar.DeltaBackword_Q12 = (int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * MaxToqNeg;
    }
    //PV正常运行
    else
    {
        STR_ECTPTVar.DeltaForward_Q12 = DeltaForward_Q12;
        STR_ECTPTVar.DeltaBackword_Q12 = DeltaBackword_Q12;
    }
    
}
/*******************************************************************************
  函数名: int32 ToqSoftStart(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   获得转矩模式下转矩指令来源
********************************************************************************/
Static_Inline void ToqSoftStart(void)
{
    int64   ToqAccelTemp_Q12 = 0;      //转矩指令中间变量
    int64   ToqCommand_Q12;             //转矩指令Q12格式
    int64   SoftStartToqRefLatch_Q12;   //转矩调节器输入的转矩指令Q12格式

    ToqCommand_Q12 = ((int64)STR_ECTPTVar.TargetToq)<<12;
    SoftStartToqRefLatch_Q12 = STR_ECTPTVar.ToqSoftStartRefLatchQ12;
    
    //if(STR_ECTPTVar.ToqSoftStartRefLatch == STR_ECTPTVar.TargetToq)
    if(SoftStartToqRefLatch_Q12 == ToqCommand_Q12)
    {
        ToqAccelTemp_Q12 = ToqCommand_Q12;
        return;
    }
    //SoftStartToqRefLatch_Q12 = ((int64)STR_ECTPTVar.ToqSoftStartRefLatch)<<12;
    DeltaToqSource();

    if (STR_ECTPTVar.ToqSoftStartRefLatchQ12 < ToqCommand_Q12)
    {
        if(STR_ECTPTVar.ToqSoftStartRefLatchQ12 >= 0)
        {
            ToqAccelTemp_Q12 = SoftStartToqRefLatch_Q12 + STR_ECTPTVar.DeltaForward_Q12;    //正向加速；
        }
        else
        {
            ToqAccelTemp_Q12 = SoftStartToqRefLatch_Q12 + STR_ECTPTVar.DeltaBackword_Q12;   //反向减速；

			if((ToqAccelTemp_Q12>=0)&&(STR_ECTPTVar.TargetToq>=0))
			{
			    ToqAccelTemp_Q12 =0;
			}

        }



        if(ToqAccelTemp_Q12 < ToqCommand_Q12)
        {
            STR_ECTPTVar.ToqSoftStartRefLatchQ12 = ToqAccelTemp_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatch =(int32)(ToqAccelTemp_Q12>>12); 
        }
        else
        {
            ToqAccelTemp_Q12 = ToqCommand_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatchQ12 = ToqAccelTemp_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatch =(int32)(ToqAccelTemp_Q12>>12) ;
        }
        
    }
    else
    {
        if(STR_ECTPTVar.ToqSoftStartRefLatchQ12 >0 )
        {
            ToqAccelTemp_Q12 = SoftStartToqRefLatch_Q12 - STR_ECTPTVar.DeltaForward_Q12;    //正向减速；    
			if((ToqAccelTemp_Q12<=0)&&(STR_ECTPTVar.TargetToq<=0))
			{
			    ToqAccelTemp_Q12=0;
			}  

        }
        else
        {
            ToqAccelTemp_Q12 = SoftStartToqRefLatch_Q12 - STR_ECTPTVar.DeltaBackword_Q12;    //反向加速；    
        }

        if(ToqAccelTemp_Q12 > ToqCommand_Q12)
        {
            STR_ECTPTVar.ToqSoftStartRefLatchQ12 = ToqAccelTemp_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatch =(int32)(ToqAccelTemp_Q12>>12); 
        }
        else
        {
            ToqAccelTemp_Q12 = ToqCommand_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatchQ12 = ToqAccelTemp_Q12;
            STR_ECTPTVar.ToqSoftStartRefLatch =(int32)(ToqAccelTemp_Q12>>12); 
        }
    }
}

/*******************************************************************************
  函数名: void ECTInteruptToqUpdate(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:  转矩指令与转矩反馈显示--中断更新
********************************************************************************/
void ECTInteruptToqUpdate(void)
{
    if(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==0)return;
    ECTPTDisplay();

}
/*******************************************************************************
  函数名: void ECTToqShow(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:  转矩指令与转矩反馈显示
********************************************************************************/
void ECTToqShow(void)
{
    if(STR_FUNC_Gvar.MonitorFlag.bit.ESMState==1)return;
    ECTPTDisplay();

}
/*******************************************************************************
  函数名: void ECTPTDisplay(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:  转矩指令与转矩反馈显示
********************************************************************************/
void ECTPTDisplay(void)
{
    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
    {
        ObjectDictionaryStandard.ProToqMode1.ToqDemandValue = (int16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef; 
    }
    else
    {
        ObjectDictionaryStandard.ProToqMode1.ToqDemandValue = 0; 
    }
    ObjectDictionaryStandard.ProToqMode1.ToqActualValue = (int16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb; 
}

/*******************************************************************************
  函数名: int32 ToqSoftStart(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   获得转矩模式下转矩指令来源
********************************************************************************/
Static_Inline int32 ECTToqCmdTest(void)
{
    int32 ToqCmdValue      = 0;
    static Uint8 i=0;
    
    if(FunCodeUnion.code.TL_Rsvd04>0)//转矩指令来自于内部正弦表
    {
        switch(FunCodeUnion.code.TL_Rsvd04)
        {
            case 1://1.0k的正弦表

            if(i<15)i++;
            else i=0;
            ToqCmdValue = SineToqTable_1k[i];
            break;
            
            case 2://1.5k的正弦表
            
            if(i<10)i++;
            else i=0;
            ToqCmdValue = (int32)SineToqTable_1dot5k[i];
            
            break;
            
            case 3://2.0k的正弦表
            
            if(i<7)i++;
            else i=0;
            ToqCmdValue = SineToqTable_2k[i];
            break;
            
            case 4://2.5k的正弦表
            
            if(i<31)i++;
            else i=0;
            ToqCmdValue = SineToqTable_2dot5k[i];
            break;

            case 5://3.0k的正弦表
            
            if(i<15)i++;
            else i=0;
            ToqCmdValue = SineToqTable_3k[i];
            break;

            case 6://3.5k的正弦表
            
            if(i<31)i++;
            else i=0;
            ToqCmdValue = SineToqTable_3dot5k[i];
            break;
            
            case 7://4.0k的正弦表
            
            if(i<3)i++;
            else i=0;
            ToqCmdValue = SineToqTable_4k[i];
            break;
        }
        ToqCmdValue = (ECTToqCmdTest()+250)>>1;
        ToqCmdValue = (int32)((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ((int64)ToqCmdValue)>>12);

    }
    else//转矩指令来自于6071
    {
        ToqCmdValue = (int32)((int16)ObjectDictionaryStandard.ProToqMode1.TargetToq)
                       + (int32)((int16)ObjectDictionaryStandard.CstOffset.ToqOff);
        ToqCmdValue = (int32)(((int64)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ((int64)ToqCmdValue))>>12);
    }
    return ToqCmdValue;
}

/********************************* END OF FILE *********************************/
