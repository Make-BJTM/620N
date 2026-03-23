/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名: FUNC_I2cDriver_ST.h	
 创建人：童文邹                
 修订人：李浩                 创建日期：11.11.14 
 描述： 
     1.
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_I2CDRIVER_ST_H
#define FUNC_I2CDRIVER_ST_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/




/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void InitI2c_ST(void);
Uint8 I2cRwDeal(Uint8* DataBuffer, Uint16 Address, Uint8 DataNum, Uint8 RwMode);


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* __FUNC_I2CDRIVER_ST_H */


/********************************* END OF FILE *********************************/






