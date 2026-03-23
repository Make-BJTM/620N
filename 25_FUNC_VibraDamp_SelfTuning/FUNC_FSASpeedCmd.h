/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_FSASpeedCmd.h
 创建人：马世贤
 修订人：何云壮                 修订日期：2012.09.06 
 描述： 
     1.生成频谱分析速度指令

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_FSASPEEDCMD_H
#define FUNC_FSASPEEDCMD_H


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


/*FUNC_GUIControl.c文件内调用的变量的结构体类型*/


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void GenerateFSASpdCmd(void);      // 生成频谱分析速度指令
void FSASendCtrl(void);             // 控制发送开环频率特性分析数据给上位机

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_FSASPEEDCMD_H */

