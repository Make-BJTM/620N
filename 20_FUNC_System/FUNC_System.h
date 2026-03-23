/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_System.h                     
 创建人：   李浩                 修改日期：2012.02.10
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_SYSTEM_H
#define __FUNC_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h" 

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */




/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */   

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
//初始化相关调度程序
extern void FUNC_PeripheralConfig_RST(void);
extern void FUNC_Parameter_Frist_RST(void);
extern void FUNC_Parameter_Second_RST(void);
extern void FUNC_Interrupt_RST(void);  

//中断相关调度程序
extern void FUNC_CmdProcess_ToqInterrupt(void);
extern void FUNC_AdcStart_ToqInterrupt(void);
extern void FUNC_AuxFunc_ToqInterrupt(void);
extern void FUNC_System_AuxInterrupt(void);
extern void FUNC_PostionControl_PosInterrupt(void);

//主循环调度程序
extern void FUNC_MainLoop(void);

//看门狗初始化
extern void FUNC_InitAndEnableWatchDog(void);

void ZeroIndexISR(void);      // Z 脉冲中断服务函数
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_SYSTEM_H */

/********************************* END OF FILE *********************************/

