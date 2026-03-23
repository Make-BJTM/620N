/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_PhaseLack.h

 创建人：童文邹、肖明海      创建日期：2012.02.06
 修改人：朱祥华              修改日期：2012.02.06 
 描述： 
      温度监控保护模块，目前只包括IPM温度模块监控
      驱动器过载数据表单分以下四种类型实验测试得到
 修改记录：
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_MONITOR_PHASELACK_H
#define FUNC_MONITOR_PHASELACK_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
//输入准备好即非三相驱动电均断线状态 为1
#define INPUT_RDY     1

//输入未准备好三相驱动电均断线状态     为0
#define INPUT_NRD     2


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//----------------------------------------------------------------------------
//缺相监控检测
typedef struct{
    Uint16 PL_PinStatus;       //记录电流环的PL信号状态
}STR_MONITOR_PHASELACK;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_MONITOR_PHASELACK   STR_Monit_PhaseLack; //缺相结构体定义   


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void PL_1k_Monitor(void);                 //输入电源缺相主循环1K监控
extern void InitPL_Process(void);              //初始化PL信号滤波窗口宽度


#ifdef __cplusplus
}
#endif

#endif /*FUNC_Monitor_PhaseLack.h*/    

/********************************* END OF FILE *********************************/
