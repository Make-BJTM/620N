/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_LineInterplt.h
 创建人：肖明海                 创建日期：2010.01.06
 修订人：何云壮                 修订日期：2012.02.03 
 描述： 
     1.直线插补功能的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_LINEINTERPLT_H
#define FUNC_LINEINTERPLT_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define LineInterpltAttribDflts {0, 0, 0, 0, 0,  0, 0, 0, 0, 0, \
                                 0, 0, 0, 0, 0,  0, 0, 0}

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	
#define LINEAMPBIT   (16)

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


/*FUNC_LineChabu.c文件内调用的变量的结构体类型*/
// 直线插补模块属性参数结构体
typedef struct _STR_LINE_INTERPOLATION_ATTRIB
{
    Uint8 IntpltStatus;            //0-插补完成，未进行插补; 1－正在插补
	
	Uint8 PlanDecAgain;            // 再规划减速段一次标志
	int64 StartPulseQ16;           //启动速度对应每个插补周期的脉冲个数, Q16
	int64 StopPulseQ16;            //停止速度对应每个插补周期的脉冲个数, Q16
	int64 UpPulseRevQ16;           //加速脉冲增量每个插补周期, Q16
	int64 AvergePulseQ16;          //匀速脉冲个数每个插补周期, Q16
		
    int64 DownPulseRevQ16;         //减速脉冲增量每个插补周期, Q16
	int64 UpLengthQ16;             //加速段长度, Q16
	int64 DownLengthQ16;           //减速段长度, Q16
	int64 AvergeLengthQ16;         //匀速段长度, Q16
	int64 LineLengthQ16;           //直线段总长度, Q16

	int64 LineRemainLengthQ16;     //当前线段剩余长度, Q16
	int16 LineDir;                 //插补方向 1＝正方向 －1＝负方向
 	int64 PlanIntpltValQ16;        //当前插补周期预计插补脉冲个数, Q16
	int32 PlanValueRemainQ16;      //当前插补周期脉冲剩余个数, Q16
	int32 RealIntpltVal;           //当前插补周期实际插补脉冲数

	Uint16 MbpContinue;            //多段位置运行方式3(连续运行)特殊处理标志
	int64 MbpExtraPlsQ16;         //多段位置运行方式3(连续运行), 存储当前段多或少插补的脉冲数, Q16 
} STR_LINE_INTERPLT_ATTRIB;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
// 插补参数复位 
void LineIntpltReset(STR_LINE_INTERPLT_ATTRIB *pAtb);

// 实现插补过程, 给出插补位置指令
int32 LineIntplt(STR_LINE_INTERPLT_ATTRIB *pAtb);

// 初始化插补参数, 启动插补过程
// 注意: 参数 UpTime, DownTime 都以插补周期为单位
void LineIntpltInit(int32 StartSpeed, int32 RunSpeed, int32 StopSpeed,
                    Uint32 UpTime, Uint32 DownTime, int64 LineDist,
					STR_LINE_INTERPLT_ATTRIB *pAtb);

// 记录插补结构变量的地址					
void RecordIntpltStruct(STR_LINE_INTERPLT_ATTRIB *pAtb, Uint16 index);
					
// 提前结束插补过程时重置减速段长度
void ResetIntpltDownLength(STR_LINE_INTERPLT_ATTRIB *pAtb);

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_LineInterplt.h */
