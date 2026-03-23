#ifndef ETHERCATESMDISPLAY_H
#define ETHERCATESMDISPLAY_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

//ESM状态字宏指令
#define  ESM_INITIAl          0x01
#define  ESM_PREOPERATION     0x02
#define  ESM_SAFEOPERATION    0x04
#define  ESM_OPERATION        0x08
#define  ESM_FAULT            0x10
#define  ESM_STOP             0x20



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct{
    int8   FpgaSyncModeConfigDone;     
   
    int16  CarFreqDivSyncFreq;      //Sync与载波 周期比值 
    int32  TheoreticalSyncPrd_Half_25ns;     //理论Sync周期 单位12.5us
    int32  SyncErrThreshold;
}STR_ECATSYNCVAR;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_ECATSYNCVAR  EcatSync;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void ESMStateMonitor(void);
extern void ECTInteruptUpdate(void);
extern void FpgaSyncModeConfigAndCheck_ToqInt(void);
extern void FpgaSyncModeConfig_SyncInt(void);
extern void ECTInteruptUpdate(void);

#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of FUNC_GlobalVariable.h*/

/********************************* END OF FILE *********************************/
