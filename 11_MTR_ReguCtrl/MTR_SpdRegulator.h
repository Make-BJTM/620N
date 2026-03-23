/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                   
 文件名: MTR_SpdRegulator.h 
 创建人：高小峰            创建日期：2008-10-31 
 修改人：朱祥华            修改日期：2011.10.09                                                           
 描述： 
    1.伺服速度环变量及函数声明
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef MTR_SPDREGULATOR_H
#define MTR_SPDREGULATOR_H



#ifdef __cplusplus
extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 



/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
#define STR_SPDREGULATOR_DEFAULT  {0,0,0,0,0,0,0,0,0}


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */




/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
//面板菜单结构体类型定义
typedef struct{
    Uint64 SpdScalCoef_Q38;         //速度调节器增益参数转换系数
	Uint64 DobCoff_Q32;				//转矩干扰观测器转换系数,未更新惯量比
    Uint64 DobCoffUpdt_Q32;         //转矩干扰观测器转换系数,更新惯量比
    int64  ToqFbScalCoef_Q38;       //转矩前馈增益参数转换系数    
    int32  SpdRefInLatch;           //速度指令旧值--转矩前馈使用
	int32  ToqDisturb;             //干扰转矩观测值
	int32  DeltaSpd;			   //连续两周期的速度差值，用以求取加速度
    int32  ToqCompTemp;
    int32  ToqCompensateValue;	    //摩擦力过零补偿时的转矩最终补偿量
}STR_SPDREGULATOR;





/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */ 

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//以下函数在上电启机初始化程序 G_MTR_SysInit_STAR()中调度执行 
extern void InitSpdRegulator(void);  //速度环上电启机初始化功能码 

extern void SpdReguUpdata(void);      //速度环运行更新生效功能码 
extern void SpdReguStopUpdata(void);      //速度环停机更新 
extern void SpdReguCoefUpdata(void);  //速度环调节器系数和转矩指令滤波系数更新

//以下函数在后台程序 G_MTR_Task_BKINT()（主循环）中调度执行
extern void SpdReguDatClr(void);     //速度环累积参数清除 

//以下函数在时基中断程序（速度位置环调度）G_MTR_Task_TBINT()中调度执行
extern void Nomal_SpdSchedueMode(void);   //正常的速度环调度模式
//wzg extern void SpdRegulator(void);       //速度调节器子函数  此函数需要在转矩模式下的速度限制中借用
//wzg extern void ToqRef_Filter(void);      //转矩指令滤波 
//wzg extern void OvSpdFdbErrMonitor(void);     //速度反馈超速报错监控

#ifdef __cplusplus
}
#endif

#endif  /* MTR_SpdRegulator.h */	

/********************************* END OF FILE *********************************/
