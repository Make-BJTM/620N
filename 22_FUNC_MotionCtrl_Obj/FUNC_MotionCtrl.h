/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_MotionCtrl.h            
 修订人：王治国                 创建日期：2011.10.28 
 描述： 
    1.运动控制模块的头文件，运动控制模块调度函数声明
    2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_MOTIONCTRL_H
#define FUNC_MOTIONCTRL_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

/*运动控制模块调度函数的声明*/
extern void PosControl(void);
extern void PosFbCalc_ToqInt(void);
extern void InitPosCtrl(void); 
extern void PosCtrlStopUpdata(void);
extern void PosCtrlUpdata(void);
extern void PosCtrl_MainLoop(void);
extern void ClrPosReg(void);
extern void PosShow(void);

extern void InitPosRegu(void);
extern void PosErrCLRSignalFilter(void);
extern void PosReguStopUpdata(void);
extern void PosReguUpdata(void);
extern void PosRegulator(void);  
extern void PosFbCalc_PosSched(void); 

//extern void PosErrCalcAndClr(void);
extern Uint32 HandWheelSample(void);

void DoHoming(void);          // 执行原点回归过程
extern void LowOscSelfCal(void);

//extern void FullCloseLoopModeSet(int32 * pPosFdb, Uint32 *pExPosCoef, int32 ExPosFdbAbsValue,Uint8 ExPosFeedbackFlag); 
//  
//extern void FullCloseLoopErrorCheck(Uint8 ExPosFeedbackFlag);  //偏差过大保护，打滑保护


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* __FUNC_MOTIONCTRL_H */

/********************************* END OF FILE *********************************/
