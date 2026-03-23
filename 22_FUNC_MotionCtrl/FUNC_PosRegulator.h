/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_PosRegulator.h
 创建人：高小峰                
 修订人：李浩                 创建日期：11.10.19 
 描述： 
     1.
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/
#ifndef FUNC_POSREGULATOR_H
#define FUNC_POSREGULATOR_H

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
typedef struct{
    Uint8  ClrSignNew  :1;    //bit 0   脉冲差清除开关量前一次输入
    Uint8  ClrSignLast :1;    //bit 1   脉冲差清除开关量前一次输入
    Uint8  ClrSignState:2;    //bit 2-3 脉冲差清除开关量输入状态
    Uint8  ClrFlag     :1;    //bit 4   脉冲差清楚标志位
    Uint8  GpioB10FltStatue :1; //bit5  GPIOB10_Clr滤波后的信号状态
    Uint8  Rsvd:2;           
}STR_POSERR_CLEAR_BIT;
 

/*定义FUNC_EleGear.c文件内调用的变量的结构体类型*/

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitPosRegu(void);

extern void PosReguStopUpdata(void);

extern void PosReguUpdata(void);

extern void PosRegulator(void);

extern void PosErrCalcAndClr(void);

extern void ClrPosErrAndRegulator(void);


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_PosRegulator.h */

/********************************* END OF FILE *********************************/


