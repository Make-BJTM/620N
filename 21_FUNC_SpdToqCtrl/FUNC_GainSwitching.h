/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_PosGainSpdGain.h                                                           
 创建人：   姚虹                   创建日期：2011.11.31
 修改人：   王治国                 修改日期：2012.02.05
 描述: 
    1.
 修改记录：  
    xx.xx.xx      XX
    1.      
    2.
 
********************************************************************************/
#ifndef __FUNC_GAINSWITCHING_H
#define __FUNC_GAINSWITCHING_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

typedef struct{
    Uint8   GnSwModeSel;       //模式选择

    //根据速度指令高低阈值切换用
    int32   SpdRefLvlMax;
    int32   SpdRefLvlMin;
    int64   SpdKpCoef_Q30;
    int64   SpdKiCoef_Q40;
    int64   PosKpCoef_Q30;
    int64   LowSpdKp_Q30;
    int64   HighSpdKp_Q30;
    int64   LowSpdKi_Q40;
    int64   HighSpdKi_Q40;
    int64   LowPosKp_Q30;
    int64   HighPosKp_Q30;

    //其它模式
    int32   ToqSwPointH;       //转矩指令高切换点
    int32   ToqSwPointL;       //转矩指令低切换点

    int32   SpdRefSwPointH;    //速度指令高切换点
    int32   SpdRefSwPointL;    //速度指令低切换点

    int32   DelSpdRefSwPointH;    //速度指令变化率高切换点
    int32   DelSpdRefSwPointL;    //速度指令变化率低切换点
    int32   DelSpdRefSwTmCnt;     //速度指令变化率切换模式时1ms转换成的位置环周期数
    int32   DelSpdRefSwRealTmCnt; //速度指令变化率切换模式时实时计数器
    int32   DelSpdRefSwRefLatch;  //速度指令变化率切换模式时速度锁存

    int32   SpdFdbSwPointH;    //速度反馈高切换点
    int32   SpdFdbSwPointM;    //速度反馈低切换点
    int32   SpdFdbSwPointL;    //速度反馈低切换点

    int32   PosErrPointH;      //位置偏差高切换点
    int32   PosErrPointL;      //位置偏差低切换点

    int32   PosReachPoint;     //位置定位完成幅度

    int32   RealTmCnt;        //增益切换时间实时计数
    int32   DelPosKp_Q10;         //切换时每周期累加的增益
    int32   PosGnSwTmCnt;     //增益切换时间转换成的位置环周期数

    int32   RealDlyTmCnt;     //增益切换延时时间实时计数
    int32   GnSwDlyTmCnt;     //增益切换延迟时间转换成的位置环周期数

    Uint8   GnFirToSecFlag;   //第一组相第二组增益切换
}STR_GAINSWITCHINGLOCALVARIABLE; 


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void GainSwitchingUpdate(void);
extern void GainSwitching(void);



#ifdef __cplusplus
}
#endif

#endif /* __FUNC_GAINSWITCHING_H */

/********************************* END OF FILE *********************************/
