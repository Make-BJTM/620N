/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:	FUNC_MultiBlockSpeed.h                                                           
 创建人：何俊辉                创建日期：08.11.03
 修改人：姚虹                  修改时间：10.02.05 
 修改人：李浩                  修改时间：12.03.13
 描述： 

    
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_MULTI_BLOCK_SPEED_H
#define __FUNC_MULTI_BLOCK_SPEED_H

#ifdef __cplusplus
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
 
/*定义FUNC_MultiBlockSpeed.c文件内调用的变量的结构体类型*/
typedef struct{
    int32   CurrentCommand;        //当前段对应的指令
    Uint16  CurrentBlockNum;       //当前段码

    Uint32  BaseCounter;           //计数器3 
    Uint32  Counter1;              //运行时间计数器1
    Uint32  Counter2;              //运行时间计算器2

    Uint16  RunFlag;               //多段运行标志 
    Uint16  DataRefreshFlag;       //初始化斜坡参数标志

    int16   BlockCommand[16];      //16段对应的速度指令
    Uint16  RiseDownMode[16];      //加减时间选择
    Uint16  RunTimeGived[16];      //运行时间

    Uint16  StepInc;               //段数增量,默认为一段一段往后增加，可设置；

    int64   DeltaRiseArray_Q16[5];     //斜坡上升速度增量
    int64   DeltaDownArray_Q16[5];     //斜坡下降速度增量	

}STR_MULTI_BLOCK_SPEED;



	
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern int64   MultiBlockDeltaSpdRise_Q16;
extern int64   MultiBlockDeltaSpdDown_Q16;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern int32 MultiBlockSpeedDeal(void);


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_MULTI_BLOCK_SPEED_H */

/********************************* END OF FILE *********************************/
