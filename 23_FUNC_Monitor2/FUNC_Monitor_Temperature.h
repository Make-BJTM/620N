/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_Temperature.h

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
#ifndef FUNC_MONITOR_TEMPERATURE_H
#define FUNC_MONITOR_TEMPERATURE_H

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
//暂无



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//----------------------------------------------------------------------------
//AD采样函数
//片内AD采样数据结构体,此结构体内的变量直接可以被算法调用，其量纲符合算法调度
typedef struct{
     Uint16  ADCValue;
     Uint16  ErrValue;    
}STR_MONITOR_IPMTEMPERATURE;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */







/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitTemperatureProcess(void);     //上电初始化IPM温度测量参数配置
extern void Tempera_1k_Monitor(void);         //母线电压监控相关报错
extern void GetIPM_4Hz_Temperature(void);    //获取IPM温度交由H0b组显示



#ifdef __cplusplus
}
#endif

#endif /*FUNC_Monitor_Temperature.h*/    

/********************************* END OF FILE *********************************/
