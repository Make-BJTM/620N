/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_GainRegIndicatrix.h
 创建人：张小华
 修订人：                      修订日期：2012.09.05 
 描述： 
   	  增益调整指标监控

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/
#ifndef __FUNC_GAINREGINDICATRIX_H
#define __FUNC_GAINREGINDICATRIX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define  MHZ1MS      1000           //1ms转换成us

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

typedef struct{
    Uint16     SystemTimerLast;     //系统上次时钟
    Uint16     Timer1us;          //计时器  单位1us
    Uint16     Timer1ms;	       //计时器  单位1ms
    Uint16     PulseStop;          //脉冲发送停止 1:脉冲发送停止 0：脉冲发送经行中
	Uint16     PulseStopLast;       //脉冲发送完上次值
	Uint16     PulseMnStopLast;		//主循环中脉冲停止标志
	Uint16     AutoTuneLast;       //自调整上周期选择
    Uint16     DataUpEn;            //数据更新使能
    Uint16     PosLagTime;          //定位时间
    Uint16     Timer1msLatch1;      //计数器锁存值,保存脉冲停止发送的时间点
    Uint16     PosLagTimeStatus;   //定位时间标志位0：允许更行，1：不允许更新。
    Uint16     OscillateLevelAv;   //平均振动等级     
    Uint16     LoadRatAV;           //平均负载率
	int16      PulseDir;            //脉冲方向
    Uint16     FirOscRigidity;       //初次发生振动时的刚性
    Uint16     SetRigidity;          //自调整最终设置的刚性
    Uint16     MaxOscRigidity;       //自调整允许设置的最高刚性
    Uint32     OscillateLeveSum;    //振动等级累加和
	Uint32     LoadRatSum;      //求负载率时负载累加和
    int32      PulseMax;        //定位时最大过冲脉冲数
	
	Uint32     InerRatioCnt;  //惯量辨识结果处理次数
	Uint16     RatioIndex;	  //惯量比计算滑动平均滤波索引
	Uint16     RatioArray[8];    //用于惯量比滑动滤波的数组
	Uint32     RatioSum;	    //惯量辨识累加和，用于对计算结果滤波
	Uint16     NotchSetStep;      //陷波器设置步骤
	Uint16     SteadyFlag;        //惯量比稳定标识位
    Uint16     RatioIdenFlag;     //惯量比辨识标志位，1表示一个加减速周期内完成了一次惯量辨识
	Uint16     HandAutoTuneFlag;  //手动自调整标志位，1~自调整由H0d09触发，而非后台触发
}STR_PERINDEX_DETECTION;

/*FUNC_GUIControl.c文件内调用的变量的结构体类型*/


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_PERINDEX_DETECTION    STR_PerfIndexDet;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void PerformanceIndexCal(void) ;
extern void PerformanceIndexInit(void);
extern void IndicatrixStopUpData(void);
extern void IndicatrixDataUp(void);

extern void GainAutoTuneProcess(void);
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_FUNCODEDEFAULT_H */

/********************************* END OF FILE *********************************/


