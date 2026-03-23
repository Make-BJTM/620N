/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_EleGear.h
 创建人：高小峰                
 修订人：李浩                 创建日期：11.10.18 
 描述： 
     1.电子齿轮结构体类型定义
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_ELEGEAR_H
#define FUNC_ELEGEAR_H

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

/*定义FUNC_EleGear.c文件内调用的变量的结构体类型*/
typedef struct{
    int32   InPut;                    //电子齿轮前位置给定(增量)

    int32   GearRemainder;            //电子齿轮计算的余数

    int32   Numerator;                //电子齿轮分子 (最大值为500)  
    int32   Denominator;              //电子齿轮分母

    Uint16  GroupSelLatch;            //电子齿轮组使能选择锁存

}STR_ELECTRONIC_GEAR;
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern int32 EleGearRatioCalc(STR_ELECTRONIC_GEAR *p,int32 CommandInput);

extern void EleGearInit(STR_ELECTRONIC_GEAR *p);
extern void EleGearRatioErrCheck(void);
extern void EleGearGetValue(STR_ELECTRONIC_GEAR *p,int32 CommandInput);
/*FUNC_EleGear.c模块共享全局函数的声明*/


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_EleGear.h */

/********************************* END OF FILE *********************************/
