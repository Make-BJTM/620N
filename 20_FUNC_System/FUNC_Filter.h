/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_Filter.h  
 创建人：   梅仕峰                 创建日期：2010.08.31                     
 修改人：   王治国                 修改日期：2012.02.10
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_Filter_H
#define __FUNC_Filter_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

//中值滤波宏定义
#define MidFilter_Defaults     {0,0,0,0,0}

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

//双线性变换方法一阶低通滤波器结构体默认值
#define BILINEAR_LOWPASS_FILTER_Defaults            {       \
    (void (*) (STR_BILINEAR_LOWPASS_FILTER *p)) InitLowPassFilter, \
    (void (*) (STR_BILINEAR_LOWPASS_FILTER *p)) LowPassFilter,     \
    0,  \
    0,  \
    0,  \
    0,  \
    0,  \
    0,  \
    0,  \
    0,  }


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

//双线性变换方法一阶低通滤波器
typedef struct{
    void (* InitLowPassFilter)();
    void (* LowPassFilter)();
    int32 Ts;        //采样时间(us)
    int32 Tc;        //滤波时间(us)

    int32 Ka_Q20;    //滤波系数
    int32 Kb_Q20;    //滤波系数

    int32  Input;    //当前滤波输入,对外接口
    int32  Output;   //当前滤波输出,对外接口

    int64  OutputLatch_Q9;  //上一次滤波输出
    int64  InputLatch_Q9;   //上一次滤波输入
}STR_BILINEAR_LOWPASS_FILTER;


//中值滤波器结构体类型定义
typedef struct{
    int32  InPut;        //当前滤波输入 对外接口
    int32  OutPut;       //当前滤波输出 对外接口
    int32  SeriData[3];  //连续的3个输入值
}STR_MidFilter;

//位置指令陷波器结构体
//陷波滤波器结构体类型定义
typedef struct
{
    int32   Input[3];	  //保存的输入参数
	int32   Output[3];   //保存的输出参数    

	int32   NumCoeff[3];      //数字陷波器分子系数
	int32   DenCoeff[3];	  //数字陷波器分母系数
    int32   DenDivCoeff_Q30;   //DenCoeff[0]的倒数
    int32   DenDivCoeffRmn_Q30;  //倒数的截尾误差

    Uint32  SampFreq;         //采样频率
    Uint32  NotchFreq;        //带阻滤波中心频率
	Uint32  DenNotchFreq;     //分母部分中心频率
    Uint32  NumSigma;		 //陷波器分子阻尼比
    Uint32  DenSigma;       //陷波器分母阻尼比

    int64   PosRmn[2];       //陷波器输出截尾
    int32   PosSumIn;    	   //输入脉冲和
    int32   PosSumOut; 		   //输出脉冲和
    int32   PosDelta;     //输入脉冲和与输出脉冲和的脉冲差   
}STR_POSNOTCH;
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
Static_Inline void ResetLowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p)
{
    p->Output = 0;
    p->OutputLatch_Q9 = 0;
    p->InputLatch_Q9 = 0;
}



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */   
extern void InitLowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p);
extern void LowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p);
extern void ResetLowPassFilter(STR_BILINEAR_LOWPASS_FILTER *p);

extern void MidFilter(STR_MidFilter *p);
extern void AvgFilter(STR_MidFilter *p);

extern void PosNotchInit(STR_POSNOTCH *p);
extern void PosNotchFilter(STR_POSNOTCH *p);
extern void PosNotchFilterClr(STR_POSNOTCH *p);
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_XXX_H */

/********************************* END OF FILE *********************************/
