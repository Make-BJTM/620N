/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_ToqCtrl.c
 创建人：李浩                创建日期：11.09.23 
 描述： 1.转矩模式下的转矩指令源的获取. 
        2.转矩模式下的转矩限幅的幅值获取
		3.转矩模式下的速度限制值的获取
 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
 
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_ToqCtrl.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "ECT_PT.h"
#include "CANopen_PV.h"
#include "CANopen_OD.h"
#include "CANopen_Pub.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/





/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */





/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 






/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */




	
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void TorqueShow(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void ToqRefSel(void);   //转矩模式下转矩指令源的获取. //后期删除#112，变成G2版本后还需定义成static 防止其他文件调用
//void ToqLmtFun(void);	//转矩模式下获取当前的转矩指令限制值. //后期删除#112，变成G2版本后还需定义成static 防止其他文件调用
void SpdLmtSel(void);   //转矩模式线转速限制值选择. //后期删除#112，变成G2版本后还需定义成static 防止其他文件调用 
int32 RefSource(Uint16 sel);    //转矩模式下获取A/B的转矩指令. //后期删除#112，变成G2版本后还需定义成static 防止其他文件调用 
#if ((DRIVER_TYPE != SERVO_620N)&&(DRIVER_TYPE != SERVO_650N))
int32 GetAiSpdLmt(Uint16 sel);     //转矩模式下获取由Ai输入的转矩指令值. //后期删除#112，变成G2版本后还需定义成static 防止其他文件调用
#endif

/*******************************************************************************
  函数名:  ToqRefSel()
  输入:    
  输出:    
  子函数:  RefSource()       
  描述:  转矩模式下转矩指令源选择开关
********************************************************************************/  
void ToqRefSel()
{
     int32   ToqCmdValue = 0;

    //电流环PI参数自调谐
    if(STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1)
    {
        ToqCmdValue = (int32)((UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * STR_FUNC_Gvar.ToqPiTune.ToqRef)>>12);
    }
    else
    {

#if ECT_ENABLE_SWITCH
        
        if(FunCodeUnion.code.BP_ModeSelet==9)
        {
            ToqCmdValue = GetECTPTRef();
			
        }
        else if(FunCodeUnion.code.OEM_LocalModeEn==1)
        {
            /*转矩模式下输入转矩指令源选择*/
            switch(FunCodeUnion.code.TL_ToqCmdSel)
            {
                case 0: 
                    ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                    break;

                case 1: 
                    ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB);
                    break;

                case 2: 
                    ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB) + RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                    break;

                case 3: 
                    if(!STR_FUNC_Gvar.DivarRegLw.bit.CmdSel)        //0->A;1->B
                    {
                        ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                    }
                    else
                    {
                        ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB);
                    }
                    break; 
               
                case 4: 
                    //量纲0.001%
                    ToqCmdValue = (int32)A_SHIFT16_PLUS_B(AuxFunCodeUnion.code.CC_CommSendTorqH, AuxFunCodeUnion.code.CC_CommSendTorqL);
                    //量纲0.1%
                    ToqCmdValue = ToqCmdValue / 100;   
                    //量纲由0.1%转换为数字量
                    ToqCmdValue = (int32)((int32)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ToqCmdValue) >> 12;
                    break;
               
                default: 
                    PostErrMsg(DEFUALTERR); //#112报错机制，G2中需要修改 ，此处在进行监控模块编写时还需进行修改
                    break;
            }

            /*转矩指令方向切换*/
            if(STR_FUNC_Gvar.DivarRegLw.bit.TOQDirSel == 1)   //DI 的#112变量名的命名规则在G2中还需重新定义
            {                            
                ToqCmdValue = (-1L) * ToqCmdValue;                    
            }
        }
        
#else
        /*转矩模式下输入转矩指令源选择*/
        switch(FunCodeUnion.code.TL_ToqCmdSel)
        {
            case 0: 
                ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                break;

            case 1: 
                ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB);
                break;

            case 2: 
                ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB) + RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                break;

            case 3: 
                if(!STR_FUNC_Gvar.DivarRegLw.bit.CmdSel)        //0->A;1->B
                {
                    ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceA);
                }
                else
                {
                    ToqCmdValue = RefSource(FunCodeUnion.code.TL_ToqCmdSourceB);
                }
                break; 
           
            case 4: 
                //量纲0.001%
                ToqCmdValue = (int32)A_SHIFT16_PLUS_B(AuxFunCodeUnion.code.CC_CommSendTorqH, AuxFunCodeUnion.code.CC_CommSendTorqL);
                //量纲0.1%
                ToqCmdValue = ToqCmdValue / 100;   
                //量纲由0.1%转换为数字量
                ToqCmdValue = (int32)((int32)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ToqCmdValue) >> 12;
                break;
           
            default: 
                PostErrMsg(DEFUALTERR); //#112报错机制，G2中需要修改 ，此处在进行监控模块编写时还需进行修改
                break;
        }

        /*转矩指令方向切换*/
        if(STR_FUNC_Gvar.DivarRegLw.bit.TOQDirSel == 1)   //DI 的#112变量名的命名规则在G2中还需重新定义
        {                            
            ToqCmdValue = (-1L) * ToqCmdValue;                    
        }

#endif        
    }

    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD) 
	{
		if(( (STR_FUNC_Gvar.DivarRegLw.bit.Pot == 1) && (ToqCmdValue > 0) ) 
	              || ( (STR_FUNC_Gvar.DivarRegLw.bit.Not == 1) && (ToqCmdValue < 0) ))
		{
		  ToqCmdValue = 0 ;
		}
	}
    STR_FUNC_Gvar.ToqCtrl.ToqCmd = ToqCmdValue;
    STR_FUNC_Gvar.ToqCtrl.ToqCmdLatch = STR_FUNC_Gvar.ToqCtrl.ToqCmd;

	
}


/*******************************************************************************
  函数名:  RefSource(Uint16 sel)
  输入:    功能码
  输出:    转矩模式下转矩指令值
  子函数:         
  描述: 转矩模式下获取来自A/B的转矩指令
********************************************************************************/ 
Static_Inline int32 RefSource(Uint16 sel)
{
    int32 ref = 0;

    switch(sel)
    {

        case 0: ref = (int32)((UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * (int16)FunCodeUnion.code.TL_ToqCmdInner)>>12);  //G2_LH
                break;
                 
#if ((DRIVER_TYPE == SERVO_620N)||(DRIVER_TYPE == SERVO_650N))
        case 1:
                ref = 0;
                break;
        case 2:
                ref = 0;
                break;
#else
        case 1:
                ref = ((STR_FUNC_Gvar.AI.AI1VoltOut * 12L) * (Uint16)FunCodeUnion.code.AI_ToqGain ) >> 15;
                ref = ((int32)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ref) >> 12;
                break;
        case 2:
                ref = ((STR_FUNC_Gvar.AI.AI2VoltOut * 12L) * (Uint16)FunCodeUnion.code.AI_ToqGain ) >> 15;
                ref = ((int32)UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * ref) >> 12;
                break;
#endif
                
        case 3: 
                ref = 0;                //620P删除Ai3

                break;

        case 5:  
                 //MultiBlockDeal();
                 //ref = (int32)((gstr_Gvar.ToqRefConst_Q12 * gstr_Gvar.MultiBlockCMD)>>12);    // 百分比 0.1                                    
                 break;

        case 4:  //ref = gstr_Gvar.PidOut1;
                 break;
                 
        case 6:  //ref = gstr_Gvar.PidOut1;
                 break;
                 
        case 7:  //ref = ((UNI_FUNC_MTRToFUNC_InitList.List.ToqCoef_Q12 * gstr_Gvar.SciToqOut)>>12);		  //G2_LH
                 break;
                 
        default: PostErrMsg(DEFUALTERR);   //#112报错机制，G2中需要修改 ，此处在进行监控模块编写时还需进行修改
                 break;
       
    }
    return ref;
}


/*******************************************************************************
  函数名:  SpdLimtSel()
  输入:   功能码
  输出:   转矩模式下正负向速度限制  
  子函数:  GetAiLmt()       
  描述: 转矩模式下转速限制选择
********************************************************************************/  
void SpdLmtSel()
{
    int32 TempSpd = 0;
    int32 SpdPosLmtRef = 0;
    int32 SpdNegLmtRef = 0;

#if ((DRIVER_TYPE != SERVO_620N)&&(DRIVER_TYPE != SERVO_650N))
    int32 AiSpd = 0;
#endif
    if(2 == FunCodeUnion.code.TL_SpdLmtSel)
    {
        if(0 == STR_FUNC_Gvar.DivarRegHi.bit.V_SEL)
        {
            SpdPosLmtRef = 10000L * FunCodeUnion.code.TL_SpdLmtIn;    //内部速度正向限制值   (保守值)
            SpdNegLmtRef = SpdPosLmtRef; //内部速度负向限制值   (保守值)
        }
        else
        {
            SpdPosLmtRef = 10000L * FunCodeUnion.code.TL_SpdLmtNegIn;    //内部速度正向限制值   (保守值)
            SpdNegLmtRef = SpdPosLmtRef; //内部速度负向限制值   (保守值)
        }
    }
    else
    {
        SpdPosLmtRef = 10000L * FunCodeUnion.code.TL_SpdLmtIn;    //内部速度正向限制值   (保守值)
        SpdNegLmtRef = 10000L * FunCodeUnion.code.TL_SpdLmtNegIn; //内部速度负向限制值   (保守值)
    }


    
#if ((DRIVER_TYPE == SERVO_620N)||(DRIVER_TYPE == SERVO_650N))
    if(1 == FunCodeUnion.code.TL_SpdLmtSel)
    {
        TempSpd = UserVelUnit2RpmUnit(&STR_VelEnc_Factor,(Uint32)ObjectDictionaryStandard.ProPosMode.MaxProfileVelocity);

        SpdPosLmtRef = (TempSpd < SpdPosLmtRef)? TempSpd : SpdPosLmtRef;

        SpdNegLmtRef = (TempSpd < SpdNegLmtRef)? TempSpd : SpdNegLmtRef;
    }
    
#else
    
    /*获得转矩模式下速度限制值不能超过外部Ai速度限制值*/    
    if(1 == FunCodeUnion.code.TL_SpdLmtSel)
    {
        AiSpd = GetAiSpdLmt(FunCodeUnion.code.TL_SpdLmtAiSel);

        SpdPosLmtRef = (AiSpd < SpdPosLmtRef)? AiSpd : SpdPosLmtRef;

        SpdNegLmtRef = (AiSpd < SpdNegLmtRef)? AiSpd : SpdNegLmtRef;
    }
#endif

    /*转矩模式下转速限制值不能超过最大转速限制值*/
    TempSpd = 10000L * (Uint32)FunCodeUnion.code.MT_MaxSpd;    //最大转速限制值   (无正负)

    SpdPosLmtRef = (SpdPosLmtRef > TempSpd)? TempSpd : SpdPosLmtRef;
    
    SpdNegLmtRef = (SpdNegLmtRef > TempSpd)? TempSpd : SpdNegLmtRef;

    SpdNegLmtRef = (-1L) * SpdNegLmtRef;  // 转矩模式下正负向转速限制值对称 


    /*传给MTR模块的速度限制值只是gstr_ToqCtrl.SpdLmt  不用传gstr_ToqCtrl.SpdPosLmtRef和gstr_ToqCtrl.SpdNegLmtRef*/
    if(STR_FUNC_Gvar.ToqCtrl.ToqCmd >= 0)          //** G2_LH IqOut 
    {
        STR_FUNC_Gvar.ToqCtrl.SpdLmt = SpdPosLmtRef;   
    }
    else if(STR_FUNC_Gvar.ToqCtrl.ToqCmd < 0)                   
    {
        STR_FUNC_Gvar.ToqCtrl.SpdLmt = SpdNegLmtRef;      
    }
 
	STR_FUNC_Gvar.ToqCtrl.SpdLmtPos=SpdPosLmtRef;

    STR_FUNC_Gvar.ToqCtrl.SpdLmtNeg=SpdNegLmtRef;
    
}

/*******************************************************************************
  函数名:  GetAiSpdLmt(Uint16 sel)
  输入:    功能码
  输出:    Ai输出的值
  子函数:         
  描述: 转矩模式下由Ai输入的速度限制值
********************************************************************************/  
#if ((DRIVER_TYPE != SERVO_620N)&&(DRIVER_TYPE != SERVO_650N))
Static_Inline int32 GetAiSpdLmt(Uint16 sel)
{
    int32 ref = 0;

    switch(sel) 
    {
        case 1: 
            //采样电压速度增益计算 单位0.0001rpm
            ref = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain ;
            ref = (ref * (int64)STR_FUNC_Gvar.AI.AI1VoltOut)>>15;
            break;
        case 2:
            //采样电压速度增益计算 单位0.0001rpm
            ref = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain ;
            ref = (ref * (int64)STR_FUNC_Gvar.AI.AI2VoltOut)>>15;
            break;
//        case 3:
//            //采样电压速度增益计算 单位0.0001rpm
//            ref = 12000L * (Uint16)FunCodeUnion.code.AI_SpdGain  >> 15;
//            ref *= STR_FUNC_Gvar.AI.AI3VoltOut;
//            break;

        default: PostErrMsg(DEFUALTERR);           //#112报错机制，G2中需要修改 ，此处在进行监控模块编写时还需进行修改
                  break;
    }

    if(ref<0)
    {
        ref = (-1L) * ref;                          //取绝对值
    }

    return ref;

}
#endif
/*******************************************************************************
  函数名: void ToqueShow() 
  输入:    
  输出:    
  子函数:         
  描述: 转矩控制中相关变量的输出显示
********************************************************************************/
void TorqueShow()
{

    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
    {
        AuxFunCodeUnion.code.DP_ToqRef = (int16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef; 
    }
    else
    {
        AuxFunCodeUnion.code.DP_ToqRef = 0;
    }

#if ((DRIVER_TYPE == SERVO_620N)||(DRIVER_TYPE == SERVO_650N))

    ECTToqShow();

#endif

}
/********************************* END OF FILE *********************************/

