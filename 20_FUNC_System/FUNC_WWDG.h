/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_WWDG.h
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_WWDG_H
#define __FUNC_WWDG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
#define     ServiceDog()              (FUNC_WWDG->CR.bit.T = 0x53)
#define     DisableWDog()             (FUNC_WWDG->CR.bit.WDGA = 0)
#define     WDSoftwareReset()           (FUNC_WWDG->CR.bit.T = 0)

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */ 

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//WWDG_CR控制寄存器结构体类型定义
typedef struct{
    Uint32  T:7;            //7位计数器
    Uint32  WDGA:1;         //激活位
    Uint32  Rsvd:24;         //保留
}STR_WWDG_CR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_WWDG_CR_BIT         bit; 
}UNI_WWDG_CR_REG; 

//WWDG_CFR配置寄存器结构体类型定义
typedef struct{
    Uint32  W:7;                //7位窗口值
    Uint32  WDGTB:2;            //时基
    Uint32  EWI:1;              //提前唤醒中断
    Uint32  Rsvd:22;            //保留
}STR_WWDG_CFR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_WWDG_CFR_BIT         bit; 
}UNI_WWDG_CFR_REG; 


//WWDG_SR状态寄存器结构体类型定义
typedef struct{
    Uint32  EWIF:1;             //提前唤醒中断标志
    Uint32  Rsvd:31;            //保留
}STR_WWDG_SR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_WWDG_SR_BIT         bit; 
}UNI_WWDG_SR_REG;

//WWDG寄存器结构体类型定义
typedef struct
{
    volatile UNI_WWDG_CR_REG     CR;
    volatile UNI_WWDG_CFR_REG    CFR;
    volatile UNI_WWDG_SR_REG     SR;
}STR_FUNC_WWDG_Def;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_FUNC_WWDG_Def * FUNC_WWDG;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void FUNC_InitAndEnableWatchDog(void);

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_WWDG_H*/

/********************************* END OF FILE *********************************/
