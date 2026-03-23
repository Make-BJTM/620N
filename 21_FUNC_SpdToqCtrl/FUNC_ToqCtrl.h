/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_ToqCtrl.h
 创建人：李浩                创建日期：11.09.23 
 描述： 
     1.转矩控制的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_TORQCONTROL_H
#define FUNC_TORQCONTROL_H


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

/*定义FUNC_ToqCtrl.c文件内调用的变量的结构体类型*/



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */






/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

/*FUNC_ToqCtrl.c模块共享全局函数的声明*/
extern void ToqCtrlCmdAndLmt(void);

extern void TorqueShow(void);

extern void SpdLmtSel(void);

extern void ToqRefSel(void);
#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_ToqCtrl.h */

/********************************* END OF FILE *********************************/

