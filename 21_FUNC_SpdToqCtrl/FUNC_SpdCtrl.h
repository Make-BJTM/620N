/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:	FUNC_SpdCtrl.h                                                           
 创建人：   王治国              创建日期：2011.10.10
 描述： 
    1. 速度控制头文件
    
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_SPDCTRL_H
#define __FUNC_SPDCTRL_H

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

/*定义FUNC_SpdCtrl.c文件内调用的变量的结构体类型*/
typedef struct{
    int32   SpdCommand;                 //由速度指令相关处理函数处理后的速度指令值
    int32   SpdCmdLatch;                //速度指令锁存,DO速度到达使用

    Uint8   SpdZeroClampFlag;           //零速钳位(零位固定-内建位置环)使能的标志位
    int32   InnerPosReguOutput;

    /* 以下为速度斜坡处理相关变量的定义 */
    int64   DeltaSpeedRise_Q16;         //斜坡上升速度增量
    int64   DeltaSpeedDown_Q16;         //斜坡下降速度增量
    int64   NormalDeltaSpdRise_Q16;     //常规(非多段速度使能)时的加速速度增量
    int64   NormalDeltaSpdDown_Q16;     //常规(非多段速度使能)时的减速速度增量
    int32   SpdSoftStartRefLatch;       //速度指令锁存
    int64   SpdSoftStartRefLatchQ16;       //速度指令锁存

    //离线惯量辨识使用
    Uint32  InertiaIdy_WaitTime;        //每次惯量辨识之后的等待次数
    int32   InertiaIdy_DltaSpd;         //每次加速速度
    int32   InertiaIdy_MaxSpd;          //辨识到达的最大速度
}STR_LOCAL_SPDCONTROL;
	
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

extern STR_LOCAL_SPDCONTROL    STR_SpdCtrl;
extern  void InitSpdCtrl(void);
extern  void SpdCtrlStopUpdate(void);
extern  void SpdCtrlUpdate(void);
extern  void GetSpdRef(void);
extern  void SpeedShow(void);
extern void MultiBlockSpeedInit(void);

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_SPDCTRL_H */

/********************************* END OF FILE *********************************/
