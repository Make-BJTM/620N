/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_TorqPiTune.h                                                           
 创建人：熊飞                    创建日期：2012.03
 描述： 
    1.
    2.
 修改记录：  

********************************************************************************/
#ifndef __FUNC_TORQPITUNE_H
#define __FUNC_TORQPITUNE_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
//暂无

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类  debug用*/

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/ 
#define  AUQUI_COUNT_LIMIT            200       //电流波形采样点数限制

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
enum  PI_TUNE_STEP{       //pi调谐的状态变量
      TUNE_NO_START,
      TUNE_START,
      KP_TUNE,
      KI_TUNE,
      KP2_TUNE,
      KI2_TUNE,
      KEQ_TUNE,
      KED_TUNE,
      TUNE_OVER
      };

enum  KP_TUNE_STEP{       //Kp调谐的状态变量
      START_MOTOR,
      DATA_ACQUISITION,
      CHECK_RISE_TIME
      };

enum  KI_TUNE_STEP{       //Ki调谐的状态变量
      CHECK_OVER_SHOT,
      KI_START_MOTOR,
      KI_DATA_ACQUISITION
      };

enum  KEQ_TUNE_STEP{      //Keq调谐的状态变量
      KEQ_CHECK_STABLE_ERROR,
      KEQ_START_MOTOR,
      KEQ_DATA_ACQUISITION
      };

enum  KED_TUNE_STEP{      //Ked调谐的状态变量
      KED_START_MOTOR,
      KED_DATA_ACQUISITION,
      KED_CHECK_STABLE_ERROR,
      };

enum  CHECK_REQUEST_STEP{ //阶跃响应指标检验的状态变量
      CONTINUE_CHECK,
      GO_TO_TUNE,
      CHECK_OK,
      CHECK_FAIL
      };

enum  DATA_ACQUI_STEP{    //波形采样的状态变量
      NO_START,
      START_ACQUI,
      ACQUI_OVER
      };

struct TORQ_PI_TUNE{      //pi调谐结构体
       int32                     WaveData[AUQUI_COUNT_LIMIT];    //波形采样数组          
       int16                     AuquiCount;                     //波形采样序列号
       int32                     WaveEndSeri;                    //波形采样数组的截止序列号
       Uint16                    KeqTuneCount;                   //Keq调整次数
       Uint16                    KedTuneCount;                   //Ked调整次数
       int32                     CheckSeri;                      //检验指标时的序列号
       int32                     StableError;                    //稳态误差计算值
       int32                     ComputCount;                    //计算稳态误差时的累积次数
       Uint32                    PiTuneCnt;                      //torqlooppitune()函数调用计数
       Uint16                    DeltaKp;                        //Kp增量
       Uint16                    DeltaKi;                        //Kp增量
       };

struct SAVE_USER_PARA{  //用户参数结构体
      //转矩指令滤波参数 
       Uint16 ToqFiltTime;
       Uint16 ToqFiltTime2;

       //转矩限制参数
       Uint16 ToqLmtSource;
       Uint16 ToqPlusLmtIn;
   
       //转矩模式速度限制参数
       Uint16 SpdLmtSel;
       Uint16 SpdLmtIn;

        //H0806
       Uint16 ServoCtrlMode;
       //H09_00
       Uint16 ATModeSel; 
       };

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern struct TORQ_PI_TUNE   TorqPiTune;
extern enum  PI_TUNE_STEP    PiTuneStep;


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void TorqLoopPiTune(void);                   
extern void PiTuneDataAcqui(void);

#ifdef __cplusplus
}
#endif

#endif/* __SERVO_TORQPITUNE_H */
/************************************************************************************************
                                        END OF THIS FILE
**************************************************************************************************/
