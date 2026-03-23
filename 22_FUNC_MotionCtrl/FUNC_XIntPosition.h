/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_XintPosition.h
 修订人：何云壮                 修订日期：2012.02.09 
 描述： 
     1.中断定长功能的头文件
	 2.       

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_XINTPOSITION_H
#define FUNC_XINTPOSITION_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define XintPosAttribDflts  {0}


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct _STR_XINTPOS_ATTRIB
{
    int32 PosXintLatch;  // 发生中断时锁存位置
} STR_XINTPOS_ATTRIB;



/*FUNC_XintPosition.c文件内调用的变量的结构体类型*/


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern int32 XintPosCmd(void);


extern void XintPosDrvConfig(void);    // 底层中断配置, 与硬件相关

extern void XintPosEnJudgment(int32 DeltaPosFdb);

extern void XintPosReset(void);

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_XintPosition.h */

