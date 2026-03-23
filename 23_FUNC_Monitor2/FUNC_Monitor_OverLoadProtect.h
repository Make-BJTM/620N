/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_OverLoadProtect.h
 创建人：王军干              创建日期：2008.11.10
 修改人：朱祥华              修改日期：2011.11.21 
 描述： 
    1.电机、驱动器过载保护
 修改记录：
    1. 2011.11.21     朱祥华
       变更内容： 将过载保护从监控模块中独立出来 
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_MONITOR_OVERLOADPROTECT_H
#define FUNC_MONITOR_OVERLOADPROTECT_H

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
//过载保护监控
typedef struct{
    int16    IsFdb_1ms;           //每一ms计算一次相有效值电流即sqrt(Iq*Iq + Id*Id)
    int16    OffLineJudge_Is;     //动力线断线判断相有效电流
    int32    OffLineJudge_SpdFdb; //动力线断线判断相有效速度反馈值
    int16    OfflineJudge_Iqlmt;  //转矩限制值小于此值时不进行判断电机断线
    int16    Offline_Cnt;         //动力线断线累积次数

    int32    LockSpdThreshold;    //电机过载堵转速度反馈阈值
    Uint16   CurBaseOEM_Q12;      //基于驱动器的电流百分比
    Uint16   CurBaseMT_Q12;       //基于电机电流的百分比
    Uint32   DeltaIsrSvCnt;       //电机过载运算每次经过的电流环中断次数
    int32    IqRate_OEM;           //当电机相有效值电流为驱动器额定电流时对应IqRef（即IqRate_OEM）输入值
    int32    Inver_IqRateOEM_Q24; //IqRateOEM的倒数，用于计算当前电流的百分比
    int32    Inver_IqRateMT_Q24;  //IqRateMT 的倒数，用于计算当前电流的百分比
    Uint16   OVLoadCurveSel;      //根据驱动器功率选择过载保护相关曲线
    Uint32   SumHeatOEM10_Q20;    //驱动器每监控周期热量累积积分值，常温至75度时假设为1，现放大10的Q20倍
    Uint32   SumHeatMT10_Q20;     //电机每监控周期热量累积积分值，常温至过载报警产生热量假设为1，现放大10的Q20倍

    int32    LockRotorIqThreshold; //检测电机堵转转矩指令阈值
    int32    LockRotorSpdThreshold; //检测电机堵转速度反馈阈值
}STR_OverLoadProtect;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_OverLoadProtect  STR_OVLoadProtect;


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitOverLoadProtect(void);          //驱动器电机过载保护初始化
extern void OverLoadProtect_1k_Monitor(void);         //驱动器电机过载保护
extern void MTOffLine_1k_Monitor(void);           //电机动力线断线监控


#ifdef __cplusplus
}
#endif

#endif /* FUNC_Monitor_OverLoadProtect.h*/    

/********************************* END OF FILE *********************************/
