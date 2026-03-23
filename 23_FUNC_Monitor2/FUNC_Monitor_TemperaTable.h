/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:  FUNC_Monitor_TemperaTable.h                                                        
 创建人：王军干、熊飞、肖明海   创建日期：2009.02.xx
 修改人：朱祥华                 修改日期：2012.02.06 
 描述：  IPM模块温度曲线表格头文件
 修改记录：  
    1. 朱祥华    2012.02.06 
       变更内容：整理表单

add by xmh 20009-10-27 start
       参照IS500驱动器与电机参数表单(2.80)温度曲线  IPM过热温度点外置为85度，内置为90度
       0  -0.4kw     SizeA         铝基板曲线                  IPM过热报错点90度
       0.5kw-1.5kw   SizeB、SizeC  外置模块曲线                IPM过热报错点85度          
       2.0kw-7.5kw   SizeD、SizeE  BSM,Infinon两种为同样的曲线 IPM过热报错点90度
       其他          BSM曲线                                   IPM过热报错点90度
// 邓开余根据 陈文纪提供的数据更新后的表格 2009-11-27

********************************************************************************/
#ifndef FUNC_MONITOR_TEMPERATABLE_H
#define FUNC_MONITOR_TEMPERATABLE_H

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
//暂无



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern const Uint16 TemperaTable_AluBaseBoard[];   //0  -0.4kw     SizeA   铝基板曲线 
extern const Uint16 TemperaTable_AluBaseBoard400w[]; //0.4kw
extern const Uint16 TemperaTable_ExternalModule[]; //0.5kw-1.5kw   SizeB、SizeC  外置模块曲线
extern const Uint16 TemperaTable_BSMModule[];      //2.0kw-7.5kw   SizeD、SizeE  BSM,Infinon两种为同样的曲线



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//暂无



#ifdef __cplusplus
}
#endif

#endif /*FUNC_Monitor_TemperaTable.h */    

/********************************* END OF FILE *********************************/
