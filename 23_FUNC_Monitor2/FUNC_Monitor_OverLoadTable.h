/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_OverLoadTable.h

 创建人：王军干              创建日期：2008.11.10
 修改人：朱祥华              修改日期：2011.11.21 
 描述： 
      电机、驱动器过载保护数据表单。
      驱动器过载数据表单分以下四种类型实验测试得到
    Size_E  7.5KW 驱动器       Size_E  5KW 至6KW  驱动器 
    Size - C&D且1kw以上驱动器  小功率Size - A&B且1kw以下驱动器
 修改记录：
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_MONITOR_OVERLOADTABLE_H
#define FUNC_MONITOR_OVERLOADTABLE_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define  SizeE75_OVLoadTableNum       25  //SizeE 7.5KW驱动器运转过载曲线 (249.4%-102.2%)/(6.4%)=23  共24+1组数据   
#define  SizeE75_LockOVLoadTableNum   25  //SizeE 7.5KW驱动器堵转过载曲线 (249.4%-102.2%)/(6.4%)=23  共24+1组数据
#define  SizeE75_LightLoadTableNum    17  //SizeE 7.5KW驱动器散热轻载曲线 (96%-0%)/(6.4%)=15  共16+1组数据

#define  SizeE5060_OVLoadTableNum     23  //SizeE 5&6KW驱动器运转过载曲线 (249.4%-115%)/(6.4%)=21  共22+1组数据
#define  SizeE5060_LockOVLoadTableNum 23  //SizeE 5&6KW驱动器堵转过载曲线 (249.4%-115%)/(6.4%)=21  共22+1组数据
#define  SizeE5060_LightLoadTableNum  19  //SizeE 5&6KW驱动器散热轻载曲线 (108.8%-0%)/(6.4%)=17  共18+1组数据

#define  SizeCD10_OVLoadTableNum      23  //SizeC&D且1kw以上驱动器运转过载曲线 (249.4%-115%)/(6.4%)=21  共22+1组数据 
#define  SizeCD10_LockOVLoadTableNum  23  //SizeC&D且1kw以上驱动器过载保护曲线 (249.4%-115%)/(6.4%)=21  共22+1组数据
#define  SizeCD10_LightLoadTableNum   19  //SizeC&D且1kw以上驱动器轻载曲线    (108.8%-0%)/(6.4%)=17  共18+1组数据

#define  SizeAB10_OVLoadTableNum      40  //小功率Size - A&B且1kw以下驱动器运转过载曲线 (358.2%-115%)/(6.4%)=38  共39+1组数据
#define  SizeAB10_LockOVLoadTableNum  40  //小功率Size - A&B且1kw以下驱动器过载保护曲线  (294.2%-115%)/(6.4%)=28  共29+1组数据
#define  SizeAB10_LightLoadTableNum   19  //小功率Size - A&B且1kw以下驱动器轻载曲线    (108.8%-0%)/(6.4%)=17  共18+1组数据

#define  Motor_OVLoadTableNum         30  // Load30TableMOT    (294.2%-115%)/(6.4%)=28  共29+1组数据

#if POWERDRIVER_TYPE==POWDRV_IS650
#define  SizeH30_OVLoadTableNum      14  //SizeH30kw
//#define  SizeH30_LockOVLoadTableNum  15  //SizeH30kw
#define  SizeH30_LightLoadTableNum   18  //SizeH30kw
#endif

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//暂无



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern const Uint16 SizeE75_OVLoadTable[SizeE75_OVLoadTableNum];          //SizeE 7.5KW驱动器运转过载曲线xf 20110706   
extern const Uint16 SizeE75_LockOVLoadTable[SizeE75_LockOVLoadTableNum];  //SizeE 7.5KW驱动器堵转过载曲线
extern const Uint16 SizeE75_LightLoadTable[SizeE75_LightLoadTableNum];    //SizeE 7.5KW驱动器散热轻载曲线

extern const Uint16 SizeE5060_OVLoadTable[SizeE5060_OVLoadTableNum];         //SizeE 5&6KW驱动器运转过载曲线
extern const Uint16 SizeE5060_LockOVLoadTable[SizeE5060_LockOVLoadTableNum]; //SizeE 5&6KW驱动器堵转过载保护曲线
extern const Uint16 SizeE5060_LightLoadTable[SizeE5060_LightLoadTableNum];   //SizeE 5&6KW驱动器散热轻载曲线

extern const Uint16 SizeCD10_OVLoadTable[SizeCD10_OVLoadTableNum];         //SizeC&D且1kw以上驱动器运转过载曲线
extern const Uint16 SizeCD10_LockOVLoadTable[SizeCD10_LockOVLoadTableNum]; //SizeC&D且1kw以上驱动器过载保护曲线
extern const Uint16 SizeCD10_LightLoadTable[SizeCD10_LightLoadTableNum];   //SizeC&D且1kw以上驱动器轻载曲线

extern const Uint16 SizeAB10_OVLoadTable[SizeAB10_OVLoadTableNum];         //小功率Size - A&B且1kw以下驱动器运转过载曲线
extern const Uint16 SizeAB10_LockOVLoadTable[SizeAB10_LockOVLoadTableNum]; //小功率Size - A&B且1kw以下驱动器过载保护曲线
extern const Uint16 SizeAB10_LightLoadTable[SizeAB10_LightLoadTableNum];   //小功率Size - A&B且1kw以下驱动器轻载曲线

extern const Uint16 Motor_OVLoadTable[Motor_OVLoadTableNum];   // 电机过载曲线Load30TableMOT

#if POWERDRIVER_TYPE==POWDRV_IS650
extern const Uint16 SizeH30_OVLoadTable[SizeH30_OVLoadTableNum];
//extern const Uint16 SizeH30_LockOVLoadTable[SizeH30_LockOVLoadTableNum];
extern const Uint16 SizeH30_LightLoadTable[SizeH30_LightLoadTableNum];
#endif




/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 




#ifdef __cplusplus
}
#endif

#endif /*FUNC_Monit_OverLoadTable.h*/    

/********************************* END OF FILE *********************************/
