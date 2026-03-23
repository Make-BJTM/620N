/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_PDFFCtrl.h  
 创建人：   XXXXXX                 创建日期：2010.08.31                     
 修改人：   XXXXXX                 修改日期：2012.02.10
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_XXX_H
#define __FUNC_XXX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
//双线性变换方法一阶低通滤波器结构体默认值

#define PDFF_CONTROLLER_Defaults    {   \
    0,                                  \
    0,                                  \
    0,                                  \
    0,                                  \
    0,                                  \
    0,                                  \
    0,                                  \
    0,                                  \
    0   }

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
//PDFF控制器
typedef struct{
    int32 Kf_Q12;                //前馈系数  0-4096
	//速度闭环中的运算为Fdb(1+DampingKf)，为节省资源提前在主循环中运算好1+DampingKf     ((0-100%)+1)<<12
	int16 DampingKfPlus1_Q12;   
    int32 Ki_Q10;                //积分系数
    int32 Kp;                    //比例系数
    int64 Kb_Scal_Q38;           //定标系数 

    int64 KiSum;                 //积分累加 
    Uint32 SaturaFlag;       //饱和标志位

    int32 PosLmt;                //正向输出限制值
    int32 NegLmt;                //反向输出限制值
}STR_PDFFCONTROLLER;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */   

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
Static_Inline void ResetPDFFCtrl(STR_PDFFCONTROLLER *p)
{
    p->KiSum = 0;
    p->SaturaFlag = 0;
}

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern int32 PDFFCtrl(STR_PDFFCONTROLLER *p, int32 Ref, int32 Fdb_P, int32 Fdb_I);    //PDFF控制器


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_XXX_H */

/********************************* END OF FILE *********************************/
