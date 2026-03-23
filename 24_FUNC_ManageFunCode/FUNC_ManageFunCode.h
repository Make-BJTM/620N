/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_ManageFunCode.h
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
#ifndef __FUNC_MANAGEFUNCODE_H
#define __FUNC_MANAGEFUNCODE_H

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
extern void InitFunCode(void);
extern void ResetFunCode(void);
extern void InitElcLabel(void);
extern void ElcLabelDeal(void);

//面板按键部分调度函数
extern void PanelKey(void);
extern void PanelDisplay(void);

//Eeprom存储部分调度函数
extern void InitI2c(void);
extern Uint16 EepromProcess(void);
extern void EepromRwWatchDog(void);

//示波器电流环中断程序调用函数
extern void OscilloscopeSampling(void);
//示波器while主循环程序调用函数
extern void OscilloscopeProcess(void);

//Eeprom存储部分功能函数
extern void SaveToEepromOne(Uint16 Index);
extern void SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex);


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_MANAGEFUNCODE_H */

/********************************* END OF FILE *********************************/
