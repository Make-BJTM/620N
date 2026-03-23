/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_TorqPiTune.c                                                           
 创建人：熊飞                    创建日期：2012.03
 描述：
  1. 

 修改记录：  

********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_GlobalVariable.h"
#include "FUNC_TorqPiTune.h "
#include "FUNC_FunCode.h"
#include "FUNC_AuxFunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_ManageFunCode.h"
#include "FUNC_InterfaceProcess.h" 

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义*/
//PI参数限制
#define KP_LIMIT           10000
#define KI_LIMIT           5000
#define KEQ_UPLIMIT        1500
#define KEQ_DOWNLIMIT      100
#define KED_UPLIMIT        1500
#define KED_DOWNLIMIT      100
/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
struct TORQ_PI_TUNE       TorqPiTune;
struct SAVE_USER_PARA     UserPara;

enum  PI_TUNE_STEP        PiTuneStep ;
enum  KP_TUNE_STEP        KpTuneStep ;
enum  KI_TUNE_STEP        KiTuneStep ;
enum  KEQ_TUNE_STEP       KeqTuneStep;
enum  KED_TUNE_STEP       KedTuneStep;
enum  CHECK_REQUEST_STEP  StableErrorStep,OverShotStep,RiseTimeStep;
enum  DATA_ACQUI_STEP     DataAcquiStep;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
int32 Rise_Time = 0;
int16 Stable_Error = 0;
int32 TorqRef_Of_StepResponse = 0;
int32 Torq_Ref_90By100 = 0;
int32 Torq_Ref_At_Request_Overshot = 0;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void PiTuneDataAcqui(void);        //电流阶跃波形采集,放在电流环中调用
void TorqLoopPiTune(void);         //自动调谐主函数,放在主循环中调用

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void   PiTuneInital(struct TORQ_PI_TUNE *p);
Static_Inline void   GetintoPiTuneMode(void); 
 
Static_Inline void   KpTuneStepFunc(struct TORQ_PI_TUNE *p);
Static_Inline void   CheckRiseTime(struct TORQ_PI_TUNE *p); 

Static_Inline void   KiTuneStepFunc(struct TORQ_PI_TUNE *p);
Static_Inline void   CheckOverShot(struct TORQ_PI_TUNE *p);

Static_Inline void   KeqTuneStepFunc(struct TORQ_PI_TUNE *p);
Static_Inline void   IqCheckStableError(struct TORQ_PI_TUNE *p);

Static_Inline void   KedTuneStepFunc(struct TORQ_PI_TUNE *p);
Static_Inline void   IdCheckStableError(struct TORQ_PI_TUNE *p);

Static_Inline void   GetoutofPiTune(struct TORQ_PI_TUNE *p);

/*******************************************************************************
函数名: void PiTuneDataAcqui(void)
输入  : struct TORQ_PI_TUNE *p  
输出  : 无      
描述  : 电流阶跃波形采集， 放在电流环中调用
********************************************************************************/
void PiTuneDataAcqui(void)
{
    struct TORQ_PI_TUNE *p;

    p = &TorqPiTune;

    if(DataAcquiStep == START_ACQUI)     //判断是否启动采样
    {
        if( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)     //伺服是否已启动
        {
            if(p->AuquiCount< AUQUI_COUNT_LIMIT )          //采样如在200次以内，就继续采样，否则停止
            {
                //考虑到运算舍入误差, 绝对差值小于5时有效
                if(ABS(TorqRef_Of_StepResponse - UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) < 5 )
                {
                    if(PiTuneStep != KED_TUNE)  //不是D轴调谐时，就采样Q轴反馈电流
                    {
                        p->WaveData[p->AuquiCount++] = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb;
                    }
                    else                        //是D轴调谐时，就采样D轴反馈电流
                    {
                        p->WaveData[p->AuquiCount++] = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb;
                    }
                }
                else
                {
                    //如果速度限制引起转矩指令变化，则停止采样。
                    if((p->AuquiCount > 10) && 
                       ABS(TorqRef_Of_StepResponse - UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) > 100 )
                    {
                        p->WaveEndSeri = p->AuquiCount;
                        DataAcquiStep = ACQUI_OVER;
                    }
                }
            }
            else
            {
                p->WaveEndSeri = AUQUI_COUNT_LIMIT;
                DataAcquiStep = ACQUI_OVER;
            }
        } 
    }
}

/*******************************************************************************
函数名: void TorqLoopPiTune(void)
输入  : struct TORQ_PI_TUNE *p  
输出  : 无  
描述  : 自动调谐主函数，放在主循环中调用，已验证的调用位置是在监控中，1K调度频率
********************************************************************************/ 
void TorqLoopPiTune(void)
{

    STR_FUNC_Gvar.ToqPiTune.Step = 0;

    switch(PiTuneStep)   //判断自调谐的状态 
    {
        //未启动调谐 
        case TUNE_NO_START:
             if(STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1) PiTuneStep = TUNE_START;
             break;
                     
        //启动调谐
        case TUNE_START:
             GetintoPiTuneMode();
             PiTuneInital(&TorqPiTune);
             PiTuneStep = KP_TUNE;
             break;

        //开始调谐Kp
        case KP_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 1;
             KpTuneStepFunc(&TorqPiTune);
             break;

        //开始调谐Ki
        case KI_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 2;
             KiTuneStepFunc(&TorqPiTune);
             break;

       //开始调谐第二组比例增益
        case KP2_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 3;
             KpTuneStepFunc(&TorqPiTune);
             break;

       //开始调谐第二组积分补偿因子
        case KI2_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 4;
             KiTuneStepFunc(&TorqPiTune);
             break;

        //开始调谐Keq
        case KEQ_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 5;
             KeqTuneStepFunc(&TorqPiTune);
             break;

        //开始调谐Ked
        case KED_TUNE:
             STR_FUNC_Gvar.ToqPiTune.Step = 6;
             KedTuneStepFunc(&TorqPiTune);
             break; 

        //结束调谐
        case TUNE_OVER:
             STR_FUNC_Gvar.ToqPiTune.Step = 7;
             if(1 != AuxFunCodeUnion.code.MT_OperAbsROM)
             {
                GetoutofPiTune(&TorqPiTune);
                PiTuneStep = TUNE_NO_START;
             }
             break;

        default:
            PiTuneStep = TUNE_NO_START;
            break;
    }
}

/*******************************************************************************
函数名: void PiTuneInital(void)
输入  : struct TORQ_PI_TUNE *p  
输出  : 无      
描述  : 正式调谐前，参数初始化
********************************************************************************/
Static_Inline void PiTuneInital(struct TORQ_PI_TUNE *p)
{                                 
    KpTuneStep        = START_MOTOR;
    KiTuneStep        = CHECK_OVER_SHOT;
    KeqTuneStep       = KEQ_CHECK_STABLE_ERROR;
    KedTuneStep       = KED_START_MOTOR;
    StableErrorStep   = CONTINUE_CHECK;
    OverShotStep      = CONTINUE_CHECK;
    RiseTimeStep      = CONTINUE_CHECK;

    DataAcquiStep     = NO_START;  
    STR_FUNC_Gvar.ToqPiTune.ToqRef  = 0;

    p->AuquiCount     = 0;
    p->WaveEndSeri    = 0;
    p->KeqTuneCount   = 0;
    p->KedTuneCount   = 0;
    p->CheckSeri    = 0;
    p->StableError    = 0;
    p->ComputCount    = 0;

    p->PiTuneCnt = 0;

    p->DeltaKp = 320;
    p->DeltaKi = 80;

    if(0 == STR_FUNC_Gvar.ToqPiTune.Step)
    {
        //要求的上升时间对应的电流环周期数  
        Rise_Time                    = (int32)AuxFunCodeUnion.code.FA_RiseTime1 + 1;
        //提供的阶跃指令 单位0.1%
        TorqRef_Of_StepResponse      = 250;
        FunCodeUnion.code.GN_ServoCtrlMode = 1;
    }
    else
    {
        //要求的上升时间对应的电流环周期数  
        Rise_Time                    = (int32)AuxFunCodeUnion.code.FA_RiseTime2 + 2;
        //提供的阶跃指令 单位0.1%
        TorqRef_Of_StepResponse      = 1000;
        FunCodeUnion.code.GN_ServoCtrlMode = 0;    
    }

    //要求的稳态误差 单位0.1%
    Stable_Error                 = (int32)AuxFunCodeUnion.code.FA_StableError;
    //90%转矩指令（用于求上升时间）单位0.1%
    Torq_Ref_90By100             = (int32)TorqRef_Of_StepResponse * 90 / 100;
    //要求的超调转矩  单位0.1%
    Torq_Ref_At_Request_Overshot = (int32)TorqRef_Of_StepResponse * ((int32)AuxFunCodeUnion.code.FA_OverShot + 1000) / 1000;        
}

/*******************************************************************************
函数名: void KpTuneStepFunc(void)
输入  : struct TORQ_PI_TUNE *p  
输出  : 无  
描述  : 电流环比例增益调整的主调用函数 负责状态间的转移
********************************************************************************/
Static_Inline void KpTuneStepFunc(struct TORQ_PI_TUNE *p)
{
    switch(KpTuneStep)
    {
        case START_MOTOR:   //启动伺服
             if(p->PiTuneCnt>1000)
             {
                 if ((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < 100000L)&&
                     ( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
                 {
                     STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 1;
                     KpTuneStep = DATA_ACQUISITION;
                     p->PiTuneCnt = 0;
                 }
             }
             else
             {
                p->PiTuneCnt ++;
             }
             break;
        
        case DATA_ACQUISITION:    //波形采样
             switch(DataAcquiStep)
             {
                 case NO_START:
                      DataAcquiStep = START_ACQUI;
                      p->AuquiCount = 0;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = TorqRef_Of_StepResponse;
                      break;
                 case ACQUI_OVER:
                      STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 0;
                      DataAcquiStep = NO_START;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = 0;  
                      KpTuneStep    = CHECK_RISE_TIME;
                      RiseTimeStep  = CONTINUE_CHECK;
                      p->CheckSeri  = 0;
                      p->PiTuneCnt = 0;                          
                      break;
                 default:break;
             }
             break;
    
        case CHECK_RISE_TIME:    //检验上升时间
             CheckRiseTime(p);
             break;
       
        default:break;
    }
}

/*******************************************************************************
函数名: Uint16 CheckRiseTime(int32 *wave,Uint16 StepRespEnd) 
输入  : 无  
输出  : 无  
描述  : 检验上升时间
********************************************************************************/
Static_Inline void CheckRiseTime(struct TORQ_PI_TUNE *p) 
{
    switch(RiseTimeStep)
    {
        case CONTINUE_CHECK:   //继续检验
             if(p->WaveData[p->CheckSeri] < Torq_Ref_90By100)  //查找第一个数值超过90%转矩指令的点
             {
                 if(p->CheckSeri >= p->WaveEndSeri - 1)      //如果查到采样数组的最后，都没有一个数值超过90%的，那就放弃查找，直接调整Kp            
                     RiseTimeStep = GO_TO_TUNE;
                 else                                                      //如果没查到最后，就继续查找
                 {
                     RiseTimeStep = CONTINUE_CHECK;
                     p->CheckSeri++;
                 }
             }
             else                                                          //找到了
             {
                 if(p->CheckSeri< Rise_Time)               //如果满足上升时间的指标，就合格了
                 {
                     if(10 == p->DeltaKp)
                     {
                        RiseTimeStep = CHECK_OK;
                        p->DeltaKp = 320;
                     }
                     else
                     {
                         if(STR_FUNC_Gvar.ToqPiTune.Step < 3)
                         {
                             FunCodeUnion.code.OEM_CapIqKp -= p->DeltaKp;                       //H01_54;
                             FunCodeUnion.code.OEM_CapIdKp = FunCodeUnion.code.OEM_CapIqKp;     //H01_52;
                         }
                         else
                         {
                             FunCodeUnion.code.OEM_CurIqKpSec -= p->DeltaKp;                          //H01_27;
                             FunCodeUnion.code.OEM_CurIdKpSec = FunCodeUnion.code.OEM_CurIqKpSec;     //H01_24;                 
                         }
                         p->DeltaKp = p->DeltaKp >> 1;

                         RiseTimeStep = GO_TO_TUNE;                            //否则调整Kp
                     }
                 }
                 else
                 {
                     RiseTimeStep = GO_TO_TUNE;                            //否则调整Kp
                 }
             }
             break;

        case GO_TO_TUNE:   //调整参数
             if((FunCodeUnion.code.OEM_CapIqKp > KP_LIMIT) ||(FunCodeUnion.code.OEM_CurIqKpSec > KP_LIMIT))  //如果参数超限，就退出调谐并报警
             {
                 PiTuneStep = TUNE_OVER;
                 PostErrMsg(IDENTIFYWARN_001);
             }
             else                                                                                           //否则调整Kp,然后启动伺服
             {    
                 if(STR_FUNC_Gvar.ToqPiTune.Step < 3)
                 {
                     if((p->DeltaKp == 320) && (FunCodeUnion.code.OEM_CapIqKp > 5000) && (FunCodeUnion.code.OEM_CapIqKi < 80))
                     {
                        FunCodeUnion.code.OEM_CapIqKp = 100;        //H01_54;
                        FunCodeUnion.code.OEM_CapIdKp = 100;        //H01_52;                     
                        FunCodeUnion.code.OEM_CapIdKi += 20;        //H01_53;  
                        FunCodeUnion.code.OEM_CapIqKi += 10;        //H01_55;    
                     }
                     else
                     {
                        FunCodeUnion.code.OEM_CapIqKp += p->DeltaKp;                       //H01_54;
                        FunCodeUnion.code.OEM_CapIdKp = FunCodeUnion.code.OEM_CapIqKp;     //H01_52;
                     }
                 }
                 else
                 {
                     if((p->DeltaKp == 320) && (FunCodeUnion.code.OEM_CurIqKpSec > 2000) && (FunCodeUnion.code.OEM_CurIqKiSec < 80))
                     {
                        FunCodeUnion.code.OEM_CurIdKpSec = 100;        //H01_54;
                        FunCodeUnion.code.OEM_CurIqKpSec = 100;        //H01_52;                     
                        FunCodeUnion.code.OEM_CurIdKiSec += 20;        //H01_53;  
                        FunCodeUnion.code.OEM_CurIqKiSec += 10;        //H01_55;    
                     }
                     else
                     {                     
                        FunCodeUnion.code.OEM_CurIqKpSec += p->DeltaKp;                                 //H01_27;
                        FunCodeUnion.code.OEM_CurIdKpSec = FunCodeUnion.code.OEM_CurIqKpSec;     //H01_24;                 
                     }
                 }

                 KpTuneStep = START_MOTOR;
             }    
             break;

        case CHECK_OK:    //检验合格
             //进入下一步，Ki调谐
             if(STR_FUNC_Gvar.ToqPiTune.Step < 3)
             {
                PiTuneStep = KI_TUNE;
             }
             else
             {
                PiTuneStep = KI2_TUNE;
             }
             p->CheckSeri = Rise_Time - 1;   //为下一步检验超调量而初始化参数，必须在这里，从上升时间开始检测 
             break;

        default:break;
    }
}

/*******************************************************************************
函数名: void KiTuneStepFunc(void)
输入  : 无  
输出  : 无  
描述  : 电流环积分时间调整的主调用函数，负责状态间的转移
********************************************************************************/
Static_Inline void KiTuneStepFunc(struct TORQ_PI_TUNE *p)
{            
    switch(KiTuneStep)
    {
        case CHECK_OVER_SHOT:      //检验超调
             CheckOverShot(p);
             break; 

        case KI_START_MOTOR:       //启动伺服
             if(p->PiTuneCnt>1000)    //自上次伺服停机开始计时，要超过1000次调用(1ms)才能再次启动伺服
             {
                 //延时以及判断速度的目的，一方面是为了调谐时电机不至于跳动太激烈，另一方面
                 //是为了使阶跃时不至于太早进入速度限制而影响采样数据的有效性
                 //先判断伺服速度是否低于10.0000rpm,以及是否处于rdy状态
                 if ((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < 100000L)&&
                     ( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY)) 
                 {
                     STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 1;
                     KiTuneStep = KI_DATA_ACQUISITION;
                     p->PiTuneCnt = 0;
                 }
             }
             else
             {
                p->PiTuneCnt ++;
             }
             break;

        case KI_DATA_ACQUISITION:  //波形采样
             switch(DataAcquiStep)
             {
                 case NO_START:
                      DataAcquiStep = START_ACQUI;
                      p->AuquiCount = 0;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = TorqRef_Of_StepResponse;
                      break;
                 case ACQUI_OVER:
                      STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 0;
                      DataAcquiStep   = NO_START;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = 0; 
                      KiTuneStep      = CHECK_OVER_SHOT;
                      StableErrorStep = CONTINUE_CHECK;
                      p->CheckSeri    = Rise_Time - 1;   //从上升时间开始检测
                      p->PiTuneCnt = 0; 
                      break;
                 default:break;
             }
             break; 
             
        default:
            break;
    }
}

/*******************************************************************************
函数名: Uint16 CheckOverShot(int32 *wave,Uint16 StepRespEnd)
输入  : 无  
输出  : 无  
描述  : 检验超调量
********************************************************************************/
Static_Inline void CheckOverShot(struct TORQ_PI_TUNE *p)
{
    switch(OverShotStep)
    {
        case CONTINUE_CHECK:       //继续检验
             if(p->CheckSeri < p->WaveEndSeri - 1)    //查找峰值，并检验超调。如果采样的数据还没查完，可以继续查找峰值;如果查到最后，都没有出现峰值，
                                                            //那就报错，因为这是很异常的情况，有可能是阶跃时过早进入速度限制，导致采样过早结束，可增加电机阻力再试
             {
                 if(p->WaveData[p->CheckSeri] <= p->WaveData[(p->CheckSeri)+1])   //如果前值不大于后值，就继续查找峰值，否则就认为得到了峰值                       
                 {    
                     OverShotStep = CONTINUE_CHECK;
                     p->CheckSeri++;
                 }
                 else
                 {    
                     //如果超调刚好满足指标，那就检验合格，否则调整Ki
                     //另外，如果刚调完KP，就已经超调很多，没办法，只能认为超调已合格，因为上升时间优先
                     if(p->WaveData[p->CheckSeri] >= Torq_Ref_At_Request_Overshot)
                     {
                         if(5 == p->DeltaKi)
                         {
                            OverShotStep = CHECK_OK;
                            p->DeltaKi = 80;
                         }
                         else
                         {
                             if(STR_FUNC_Gvar.ToqPiTune.Step < 3) 
                             {
                                 FunCodeUnion.code.OEM_CapIqKi -= p->DeltaKi;       //H01_55
                                 FunCodeUnion.code.OEM_CapIdKi = FunCodeUnion.code.OEM_CapIqKi << 1;            //H01_53
                             }
                             else 
                             {
                                 FunCodeUnion.code.OEM_CurIqKiSec -= p->DeltaKi;      //H01_28
                                 FunCodeUnion.code.OEM_CurIdKiSec = FunCodeUnion.code.OEM_CurIqKiSec << 1;      //H01_25
                             }
                             p->DeltaKi = p->DeltaKi >> 1;
    
                             OverShotStep = GO_TO_TUNE; 
                         }
                     }                         
                     else
                     {
                         OverShotStep = GO_TO_TUNE; 
                     }   
                 }               
             }
             else
             {
                 OverShotStep = CHECK_FAIL;  
             }
             break;

        case GO_TO_TUNE:      //调整Ki
             if((FunCodeUnion.code.OEM_CapIqKi > KI_LIMIT) ||(FunCodeUnion.code.OEM_CurIqKiSec > KI_LIMIT))//如果参数超限，就退出调谐并报警，否则调整Ki
             {
                 PiTuneStep = TUNE_OVER;
                 PostErrMsg(IDENTIFYWARN_002);
             }
             else
             {             
                 if(STR_FUNC_Gvar.ToqPiTune.Step < 3) 
                 {
                     FunCodeUnion.code.OEM_CapIqKi += p->DeltaKi;      //H01_55
                     FunCodeUnion.code.OEM_CapIdKi = FunCodeUnion.code.OEM_CapIqKi << 1;            //H01_53
                 }
                 else 
                 {
                     FunCodeUnion.code.OEM_CurIqKiSec += p->DeltaKi;      //H01_28
                     FunCodeUnion.code.OEM_CurIdKiSec = FunCodeUnion.code.OEM_CurIqKiSec << 1;      //H01_25
                 }

                 KiTuneStep   = KI_START_MOTOR;
                 OverShotStep = CONTINUE_CHECK;
             }             
             break;

        case CHECK_OK:     //检验合格
             if(STR_FUNC_Gvar.ToqPiTune.Step < 3)
             {
                //调整第二组增益
                PiTuneStep = KP2_TUNE;   
                PiTuneInital(p);
             }
             else
             {
                //进入Keq的调谐
                PiTuneStep = KEQ_TUNE;
                p->CheckSeri = Rise_Time<<1;    //为D轴稳态误差检验而初始化参数，必须在这里 
             }                        
             break;

        case CHECK_FAIL:   //检验失败，退出调谐并报警
             PiTuneStep = TUNE_OVER;          
             PostErrMsg(IDENTIFYWARN_000);
             break;

        default: 
             break;
    }
}

/*******************************************************************************
函数名: void KeqTuneStepFunc(void)
输入  : 无  
输出  : 无          
描述  : Q轴反电势补偿的主调用函数 负责状态间的转移
********************************************************************************/
Static_Inline void KeqTuneStepFunc(struct TORQ_PI_TUNE *p)
{
    switch(KeqTuneStep)
    {
        case KEQ_CHECK_STABLE_ERROR:    //检验稳态误差
             IqCheckStableError(p);
             break;
                 
        case KEQ_START_MOTOR:           //启动伺服
             if(p->PiTuneCnt > 1000)
             {
                if ((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < 100000L)&&
                    ( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
                {
                    STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 1;
                    KeqTuneStep = KEQ_DATA_ACQUISITION;
                    p->PiTuneCnt = 0;
                }
            }
            else
            {
                p->PiTuneCnt++;
            }
             break;

        case KEQ_DATA_ACQUISITION:      //波形采样   
             switch(DataAcquiStep)
             {
                 case NO_START:
                      DataAcquiStep = START_ACQUI;
                      p->AuquiCount = 0;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = TorqRef_Of_StepResponse;                      
                      break;
                 case ACQUI_OVER:
                      STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 0;
                      DataAcquiStep   = NO_START;
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = 0; 
                      KeqTuneStep     = KEQ_CHECK_STABLE_ERROR;
                      StableErrorStep = CONTINUE_CHECK;
                      p->StableError  = 0;
                      p->ComputCount  = 0;
                      p->CheckSeri    = Rise_Time<<1; 
                      p->PiTuneCnt = 0;
                      break;
                 default:break;
             }             
             break;
        default: 
            break;
    }
}

/*******************************************************************************
函数名: Uint16 IqCheckStableError(int32 *wave,Uint16 StepRespEnd)
输入  : 无  
输出  : 无  
描述  : Q轴电流稳态误差检验函数
********************************************************************************/
Static_Inline void IqCheckStableError(struct TORQ_PI_TUNE *p)
{
    switch(StableErrorStep)
    {
        case CONTINUE_CHECK:   //继续检验
             ////稳态误差计算起始序列号：2*Rise_Time
             if((Rise_Time<<1) >= p->WaveEndSeri)    //如果计算起始序列号超出了采样数组，则没有办法计算稳态误差，报警；
                                                                //此时应对电机提供阻力，防止阶跃时过早进入速度受限状态
                 StableErrorStep = CHECK_FAIL;
             else
             {
                 if(p->CheckSeri < p->WaveEndSeri) //从头到尾梳理采样数据            
                 {
                     p->StableError += p->WaveData[p->CheckSeri];   //累积电流反馈
                     StableErrorStep = CONTINUE_CHECK;                          //继续累积
                     p->CheckSeri++;                                //相关计数器递增
                     p->ComputCount++;
                 }
                 else                                               //采样数据梳理结束，就计算稳态误差，并检验
                 {
                     p->StableError = p->StableError/p->ComputCount - TorqRef_Of_StepResponse;
                     p->StableError += 5;     //防止Q轴补偿过大，做一个0.5%的偏移处理
                     if((p->StableError <= Stable_Error)&&(p->StableError >= -Stable_Error)) //如果计算的误差在指标范围之内，就合格，否则调整Keq
                          StableErrorStep = CHECK_OK;
                     else
                          StableErrorStep = GO_TO_TUNE;
                 }
             }
             break;

        case GO_TO_TUNE:      //调整参数
             if((FunCodeUnion.code.OEM_KeqGain <= KEQ_DOWNLIMIT) ||
             (FunCodeUnion.code.OEM_KeqGain >= KEQ_UPLIMIT) ||
             (p->KeqTuneCount > 100)) //如果参数超限就报警，并退出调谐
             {
                 PostErrMsg(IDENTIFYWARN_003);
                 PiTuneStep = TUNE_OVER;
             }
             else                                                                                                //否则继续调整参数
             {
                  if(p->StableError > Stable_Error)      //如果稳态误差值为正，就减小Ked，否则增大，与D轴不同
                  {
                      FunCodeUnion.code.OEM_KeqGain -= 100;    //H01_23;  Q轴反电势补偿系数
                  }
                  else if(p->StableError < -Stable_Error)
                  {
                      FunCodeUnion.code.OEM_KeqGain += 100;    //H01_23;  Q轴反电势补偿系数
                  }
                  p->KeqTuneCount++;
                  KeqTuneStep= KEQ_START_MOTOR;//进入下一步：启动伺服
             } 
             break;

        case CHECK_OK:       //检验合格
             PiTuneStep = KED_TUNE;          //进入下一步：Ked调谐
             break;

        case CHECK_FAIL:     //检验失败
             PostErrMsg(IDENTIFYWARN_000);
             PiTuneStep = TUNE_OVER; 
             break;

        default:
            break;
    }
}

  
/*******************************************************************************
函数名: void KedTuneStepFunc(void)
输入  : 无  
输出  : 无  
描述  : D轴反电势补偿的主调用函数 负责状态的转移
********************************************************************************/
Static_Inline void KedTuneStepFunc(struct TORQ_PI_TUNE *p)
{
    switch(KedTuneStep)
    {
        case KED_START_MOTOR:       //启动伺服
             if(p->PiTuneCnt > 1000)         //自上次伺服停机开始计时，要超过1000次调用(1ms)才能再次启动伺服
             {
                 //延时以及判断速度的目的，一方面是为了调谐时电机不至于跳动太激烈，另一方面
                 //是为了使阶跃时不至于太早进入速度限制而影响采样数据的有效性
                 //先判断伺服速度是否低于10.0000rpm,以及是否处于rdy状态
                 if ((ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < 100000L)&&
                     ( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RDY))
                 {
                     STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 1;                         //启动伺服
                     KedTuneStep = KED_DATA_ACQUISITION;   //进入下一步：波形采样
                     p->PiTuneCnt = 0;
                 }
             }
             else
             {
                p->PiTuneCnt ++;
             }
             break;

        case KED_DATA_ACQUISITION:  //波形采样       
             switch(DataAcquiStep)                         //判断波形采集的状态
             {
                 case NO_START:                            //如果还没启动
                      DataAcquiStep = START_ACQUI;         //那就启动采样
                      p->AuquiCount =0;                    //并初始化采样序列寄存器
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = TorqRef_Of_StepResponse;                       
                      break;
                 case ACQUI_OVER:                          //如果采样结束
                      STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 0;                         //那就停止伺服
                      DataAcquiStep   = NO_START;          //并复位波形采集状态
                      STR_FUNC_Gvar.ToqPiTune.ToqRef = 0;
                      KedTuneStep     = KED_CHECK_STABLE_ERROR;  //准备进入下一步：检验稳态误差
                      StableErrorStep = CONTINUE_CHECK;          //复位稳态误差检验的状态
                      p->StableError  = 0;                       //复位相关寄存器
                      p->ComputCount  = 0;
                      p->CheckSeri    = Rise_Time<<1; 
                      p->PiTuneCnt = 0;
                      break;
                 default:
                    break;
             }             
             break;
        
        case KED_CHECK_STABLE_ERROR:
             IdCheckStableError(p);
             break;

        default: 
            break;
    }
}

/*******************************************************************************
函数名: Uint16 IdCheckStableError(int32 *wave,Uint16 StepRespEnd)
输入  : 无  
输出  : 无  
描述  : D轴电流稳态误差检验函数 如果检验成功，就把调谐好的所有电流环参数保存到E2ROM
********************************************************************************/
Static_Inline void IdCheckStableError(struct TORQ_PI_TUNE *p)
{
    switch(StableErrorStep)
    {
        case CONTINUE_CHECK:  //继续检验
            //稳态误差计算起始序列号：2*Rise_Time
            if((Rise_Time<<1) >= p->WaveEndSeri)       //如果计算起始序列号超出了采样数组，则没有办法计算稳态误差，报警；
            {               
                //此时应对电机提供阻力，防止阶跃时过早进入速度受限状态
                StableErrorStep = CHECK_FAIL;
            }
            else
            {
                if(p->CheckSeri < p->WaveEndSeri) //可能溢出，需再做考虑
                {
                    p->StableError += p->WaveData[p->CheckSeri]; //累积电流反馈
                    StableErrorStep = CONTINUE_CHECK;
                    p->CheckSeri++;
                    p->ComputCount++;
                }
                else
                {
                    p->StableError = p->StableError/p->ComputCount; //计算稳态误差，与Q轴计算式不同
                    if((p->StableError <= Stable_Error)&&(p->StableError >= -Stable_Error)) 
                    {
                        //如果在误差范围以内，则认为检验合格，否则需要调整KED
                        StableErrorStep = CHECK_OK;
                    }
                    else
                    {
                        StableErrorStep = GO_TO_TUNE;
                    }
                }
            }
            break;
    
        case GO_TO_TUNE:    //调整参数
            //如果超限，则报警 并 退出调谐；否则调整参数，然后启动伺服
            if( (FunCodeUnion.code.OEM_KedGain <= KED_DOWNLIMIT) ||
                (FunCodeUnion.code.OEM_KedGain >= KED_UPLIMIT) ||
                (p->KedTuneCount > 100) )
            {
                PostErrMsg(IDENTIFYWARN_004);
                PiTuneStep = TUNE_OVER;
            }
            else
            {
                if(p->StableError > Stable_Error)     //如果稳态误差值为正，就增大Ked，否则减小，与Q轴不同
                {
                    FunCodeUnion.code.OEM_KedGain += 100;    //H01_22;  D轴反电势补偿系数
                }
                else if(p->StableError < -Stable_Error)
                {
                    FunCodeUnion.code.OEM_KedGain -= 100;    //H01_22;  D轴反电势补偿系数
                }
            
                p->KedTuneCount++;
                KedTuneStep= KED_START_MOTOR;  //状态转移
            } 
            break;
    
        case CHECK_OK:   //检验合格
             //调谐成功
             PiTuneStep = TUNE_OVER;

             if(1 == AuxFunCodeUnion.code.FA_TorqPiTune)
             {
                 //存储参数
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_KedGain));    //H01_22;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_KeqGain));    //H01_23;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CurIdKpSec)); //H01_24;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CurIqKpSec)); //H01_27;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CurIdKiSec)); //H01_25;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CurIqKiSec)); //H01_28;
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CapIdKp));    //H01_52;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CapIdKi));    //H01_53;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CapIqKp));    //H01_54;  
                 SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_CapIqKi));    //H01_55;

                 if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x10) AuxFunCodeUnion.code.MT_OperAbsROM = 1;
             }         
            break;
    
        case CHECK_FAIL:  //检验失败，报警，退出调谐
            PostErrMsg(IDENTIFYWARN_000);
            PiTuneStep = TUNE_OVER; 
            break;
    
        default:
            break;
    }
}


/*******************************************************************************
函数名: void GetintoPiTuneMode(void)
输入  : 无  
输出  : 无
描述  : 设置参数，进入自动调谐状态
********************************************************************************/
Static_Inline void GetintoPiTuneMode(void)
{
    //转矩指令滤波参数
    UserPara.ToqFiltTime    = FunCodeUnion.code.TL_ToqFiltTime;
    UserPara.ToqFiltTime2   = FunCodeUnion.code.TL_ToqFiltTime2;

    //转矩限制参数
    UserPara.ToqLmtSource   = FunCodeUnion.code.TL_ToqLmtSource;
    UserPara.ToqPlusLmtIn   = FunCodeUnion.code.TL_ToqPlusLmtIn;
   
    //转矩模式速度限制参数
    UserPara.SpdLmtSel      = FunCodeUnion.code.TL_SpdLmtSel;
    UserPara.SpdLmtIn       = FunCodeUnion.code.TL_SpdLmtIn;

    //H0806
    UserPara.ServoCtrlMode = FunCodeUnion.code.GN_ServoCtrlMode;
    //H0900
    UserPara.ATModeSel = FunCodeUnion.code.AutoTuningModeSel;
            
    //转矩指令滤波参数  
    FunCodeUnion.code.TL_ToqFiltTime   = 0;   //H0705
    FunCodeUnion.code.TL_ToqFiltTime2  = 0;   //H0706

    //转矩限制参数
    FunCodeUnion.code.TL_ToqLmtSource  = 0;   //H0707
    FunCodeUnion.code.TL_ToqPlusLmtIn  = 3000;//H0709
   
    //转矩模式速度限制参数
    FunCodeUnion.code.TL_SpdLmtSel     = 0;   //H0717
    FunCodeUnion.code.TL_SpdLmtIn      = FunCodeUnion.code.MT_RateSpd>>1;   //H0719 一半的额定转速,不想太高，以免振动过大

    //H0900
    FunCodeUnion.code.AutoTuningModeSel = 0;

    //PI调节起始值
    FunCodeUnion.code.OEM_KedGain   = 1000;    //H01_22; 
    FunCodeUnion.code.OEM_KeqGain   = 1000;    //H01_23;
      
    FunCodeUnion.code.OEM_CurIdKpSec = 100;    //H01_24; 
    FunCodeUnion.code.OEM_CurIqKpSec = 100;    //H01_27;  
    FunCodeUnion.code.OEM_CurIdKiSec = 40;    //H01_25;  
    FunCodeUnion.code.OEM_CurIqKiSec = 20;    //H01_28;

    FunCodeUnion.code.OEM_CapIdKp = 100;    //H01_52; 
    FunCodeUnion.code.OEM_CapIqKp = 100;    //H01_54;  
    FunCodeUnion.code.OEM_CapIdKi = 40;    //H01_53;  
    FunCodeUnion.code.OEM_CapIqKi = 20;    //H01_55; 
}


/*******************************************************************************
函数名: void GetoutofPiTuneMode(void)
输入  : struct TORQ_PI_TUNE *p  
输出  : 无  
描述  : 退出自动调谐状态
********************************************************************************/
Static_Inline void GetoutofPiTune(struct TORQ_PI_TUNE *p)
{
    //退出调谐模式
    STR_FUNC_Gvar.ToqCtrl.TorqPiTuneSon = 0; 
    AuxFunCodeUnion.code.FA_TorqPiTune = 0;

    //转矩指令滤波参数
    FunCodeUnion.code.TL_ToqFiltTime  = UserPara.ToqFiltTime;
    FunCodeUnion.code.TL_ToqFiltTime2 = UserPara.ToqFiltTime2;

    //转矩限制参数
    FunCodeUnion.code.TL_ToqLmtSource  = UserPara.ToqLmtSource;
    FunCodeUnion.code.TL_ToqPlusLmtIn  = UserPara.ToqPlusLmtIn;
   
    //转矩模式速度限制参数
    FunCodeUnion.code.TL_SpdLmtSel     = UserPara.SpdLmtSel;
    FunCodeUnion.code.TL_SpdLmtIn      = UserPara.SpdLmtIn;

    //H0806
    FunCodeUnion.code.GN_ServoCtrlMode = UserPara.ServoCtrlMode;
    //H0900
    FunCodeUnion.code.AutoTuningModeSel = UserPara.ATModeSel;
}

/************************************************************************************************
                                        END OF THIS FILE
**************************************************************************************************/
