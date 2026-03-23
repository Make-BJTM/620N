/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_PosCmdFilter.h
 创建人：高小峰                
 修订人：李浩                 创建日期：12.05.30 
 描述： 
     1.转矩控制的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_POS_CMD_FILTER_H
#define FUNC_POS_CMD_FILTER_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"






/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/



/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	




/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 

/*定义位置指令平滑(平均值)滤波器调用的变量的结构体类型*/
typedef struct{

    int32    Input;                    //平滑(平均值)滤波器的输入
    int32    Output;                   //平滑(平均值)滤波器的输出
    Uint16   FilterTime;               //位置指令平均值滤波时间（ms）  
    Uint16   FltTmeDiv;                //平均值滤波时间次数倒数
    Uint16   FilterBuffCnt;            //位置指令平均值滤波时间计数
    int32    CmdRemainderSum;          //位置指令平均滤波余数总和累加
    int32    Remainder;                //位置指令平均滤波余数
    int32    CmdBuffArr[1280];          //128个平均滤波缓存器，所以最大滤波时间为128ms,参考xigama2
    int32    CmdBuffSum;               //CmdBuffSum为位置环平均值滤波缓冲数组总和求和，当电机速度为3000rpm时，CmdBuffSum = 500
    int16    UnfullBuffCnt;            //刚上电时，位置指令平均值滤波器缓冲数组为填满时的计数值

}STR_POS_AVERAGE_FILTER;



/*定义位置指令低通滤波器调用的变量的结构体类型*/

//位置环调用的双线性变换的一阶低通滤波器结构体默认值
#define POS_LOWPASS_FILTER_Defaults        { 0 }



//位置环调用的双线性变换的一阶低通滤波器
typedef struct{
    int32 Ts;                    //采样时间(us)
    int32 Tc;                    //滤波时间(us) 

    int32 Ka;                    //滤波系数

    int32  Input;                //当前滤波输入 对外接口 
    int32  Output;               //当前滤波输出 对外接口
    int64  OutputQ14;             //脉冲输出Q格式

    int32  Remainder;   //上一次滤波器计算余数
    int64  RemainderQ14;
}STR_POS_LOWPASS_FILTER;






/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */






/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void PosSmoothFilter(STR_POS_AVERAGE_FILTER *p);
extern void ResetPosSmoothFilter(STR_POS_AVERAGE_FILTER *p);


extern void InitPosLowPassFilter(STR_POS_LOWPASS_FILTER *p);
extern void PosCmdLowPassFilter(STR_POS_LOWPASS_FILTER *p);
extern void ResetPosLowPassFilter(STR_POS_LOWPASS_FILTER *p);


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_PosCmdFilter.h */

/********************************* END OF FILE *********************************/

