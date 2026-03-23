                                          /*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_AI.h                                                           
 创建人：童文邹                创建日期：2008.11 
 修改人：熊飞                  修改日期：2011.12 
 描述： 
    1.
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef __FUNC_AI_H
#define __FUNC_AI_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
//暂无

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 


typedef struct{
    int32   ZeroDrift1;     //零漂1
    int32   Offset1;        //偏置1
    int32   DeadT1;         //死区1
    int32   DispVolt1;      //显示电压1

    int32   ZeroDrift2;     //零漂2
    int32   Offset2;        //偏置2
    int32   DeadT2;         //死区2
    int32   DispVolt2;      //显示电压2

}STR_AI_VAR;




/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//暂无

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void AiConstInit(void);            //AI常数初始化函数，停机更新
extern void Ai1Process(void);             //AI1变量计算，实时调用
extern void Ai2Process(void);             //AI2，变量计算，实时调用
//extern void Ai3Process(void);             //AI3变量计算，实时调用
extern void AiShow(void);                 //H0B组 AI 采样电压值显示
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_AI_H*/

/********************************* END OF FILE *********************************/
