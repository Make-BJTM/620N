/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_InerFricIden.h  
 创建人：   姚虹                  创建日期：2014.6.24                     
 修改人：   
 描述： 
    1. 定义在线辨识摩擦力及惯量所需结构体
    2. 定义浮点运算所需结构体
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __MTR_INERFRICIDEN_H
#define __MTR_INERFRICIDEN_H

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


/* Inline Function --------------------------------------------------------*/
/* 内联函数定义 */

//在线惯量辨识结构体
typedef struct
{
    int32 SpdSum;      //速度累加，用于计算滤波后的速度
    int32 TeSum;       //力矩累加，用于计算滤波后的转矩
    int32 NewSpd;    //滤波后速度
    int32 OldSpd;    //上次滤波后速度
    int32 NewTe;     //滤波后转矩
    int32 AccPosSum;   //加速速度和
    int32 AccNegSum;   //减速速度和
    int32 TePosSum;    //加速转矩和
    int32 TeNegSum;    //减速转矩和
    int32 JRatio;      //计算出的惯量比
    int32 RatioSum;    //滤波时惯量比的和
    int16 FiltRatio[8];  //用于对惯量比进行滤波的数组
    int16 FiltIndex;     //惯量比滤波的索引值
    int16 i;             //采样过程中计数器
    int16 j;
    int16 m;
	int16 SmpCnt;        //采样次数
    int16 SampState;     //0～允许加速采样，1～允许减速采样
    int16 PosFric;   //正向摩擦力
    int16 NegFric;   //负向摩擦力
    int16 StartCalFlag;  //启动计算的标志位
}STR_INFRID;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */   
 
 
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

extern void FricProcess(void);
extern void FricRest(void);
extern void FricUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __MTR_INERFRICIDEN_H */
/********************************* END OF FILE *********************************/
