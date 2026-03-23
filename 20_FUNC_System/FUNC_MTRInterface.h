/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_MTRInterface.h
 创建人:    王治国                  创建日期：2012.03.26
 描述:
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_MTRINTERFACE_H
#define __FUNC_MTRINTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h" 
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "CANopen_OD.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/
#define     FUNCToMTR_LIST_4Hz_32Bits_NUM   3        //FUNC传到MTR数据(4Hz传一次)数量
#define     FUNCToMTR_LIST_16kHz_NUM        13       //FUNC传到MTR数据(16kHz传一次)数量

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
extern void G_FUNC_MTRGetList_4Hz_32Bits(Uint32 * pMTR_List_4Hz_32Bits_HeadAddr);
extern void G_FUNC_MTRGetList_16kHz(Uint32 * pMTR_List_4Hz_HeadAddr);

//Eeprom存储部分功能函数
extern void SaveToEepromOne(Uint16 Index);
extern void SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex);

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_MTRINTERFACE_H */

/********************************* END OF FILE *********************************/
