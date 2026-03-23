/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Global_Filter.h  
 创建人：   梅仕峰                 创建日期：2010.08.31                     
 修改人：   王治国、熊飞、朱祥华   修改日期：2012.02.10
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef MTR_GLOBAL_FILTER_H
#define MTR_GLOBAL_FILTER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

//不同陷波器的宏定义
#define  MTR_NOTCHFILTERA   1
#define  MTR_NOTCHFILTERB   2
#define  MTR_NOTCHFILTERC   3
#define  MTR_NOTCHFILTERD   4
//#define  MTR_NOTCHFILTERAA  5
//#define  MTR_NOTCHFILTERBB  6

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

//姚虹设计的新双线性变化低通滤波器结构体类型定义
typedef struct{
    int32 Fs;         //采样频率
    int32 LastFc;     //上次的截止频率
    int32 Fc;         //截止频率
    int32 Num[2];     //滤波器系数   
    int32 Den[2];
    int32 Input;        //本周期输入输出值
    int32 Output;
    int32 InputLast;    //上周期输入输出值
    int32 OutputLast;
    int32 Rem;          // 余数
}STR_MTR_NEW_LOWPASS_FILTER;


//陷波滤波器结构体类型定义
typedef struct
{
    int32   NotchInput[3];	  //保存的输入参数
	int32   NotchOutput[3];   //保存的输出参数    

	int32   NumCoeff[3];      //数字陷波器分子系数
	int32   DenCoeff[3];	  //数字陷波器分母系数
    int32   DenDivCoeff_Q30;      //DenCoeff[0]的倒数
    int32   DenDivCoeffRmn_Q30;  //倒数的截尾误差

    Uint16  SampFreq;         //采样频率
    Uint16  NotchFreq;        //带阻滤波中心频率
	Uint16  DeltaFreq_3dB;    //3dB衰减时的频率差
    Uint16  NotchDepth;       //陷波器深度

    int64   NotchRmn[2];       //陷波器输出截尾
}STR_MTR_NotchFilter;


/* Inline Function --------------------------------------------------------*/
/* 内联函数定义 */

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.  
    2.
********************************************************************************/
Static_Inline void MTR_ResetNewLowPassFilter(STR_MTR_NEW_LOWPASS_FILTER *p)
{

    p->Output = 0;
    p->InputLast = 0;    //上周期输入输出值
    p->OutputLast = 0;
    p->Rem        = 0;
}


/*******************************************************************************
  函数名: void MTR_RST_AllNotchFilter()
  输入:      
  输出:   无 
  描述:   清除陷波器A/B/C/D/AA/BB的Input和Output存储区，一般用在模式切换时调用。
  实现方法: 清零。
********************************************************************************/
Static_Inline void MTR_CLR_AllNotchFilter(STR_MTR_NotchFilter *p)
{
    p->NotchInput[0]  = 0;
	p->NotchInput[1]  = 0;
	p->NotchInput[2]  = 0;
	p->NotchOutput[0] = 0;
	p->NotchOutput[1] = 0;
	p->NotchOutput[2] = 0;
    p->NotchRmn[0]    = 0;
    p->NotchRmn[1]    = 0;
}


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */   
extern void MTR_NewNotchFilterInit(STR_MTR_NotchFilter *p, Uint16 Type);
extern void MTR_NotchFilter(STR_MTR_NotchFilter *p);

//姚虹新设计的低通滤波器
extern void MTR_InitNewLowPassFilt(STR_MTR_NEW_LOWPASS_FILTER *p);
extern void MTR_NewLowPassFiltCalc(STR_MTR_NEW_LOWPASS_FILTER *p);

#ifdef __cplusplus
}
#endif

#endif /*MTR_Global_Filter.h */

/********************************* END OF FILE *********************************/
