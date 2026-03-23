/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_Home.h
 创建人：肖明海                 创建日期：2010.01.26
 修订人：何云壮                 修订日期：2012.02.09 
 描述： 
     1.原点回归功能的头文件

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_HOME_H
#define FUNC_HOME_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define HomeAttribDflts {0, 0, 0, 0, 0,  0, 0, 0, 0, 0, \
                         0, 0, 0, 0, 0,  0, 0, 0, 0, 0, \
						 0, 0, 0, 0, 0,  0, 0, {0} }

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct{
	Uint16 IsOffset:1;     // 1: 原点偏移指定值; 0: 不偏移
	Uint16 IsLimited:1;	   // 1: 遇限反向找原点; 0: 不反向找    
	Uint16 Rsvd1:14;	    
}STR_ORGSEL_BIT;    	    
	    
typedef union{
	volatile Uint16 all;
	volatile STR_ORGSEL_BIT bit;
}UNI_ORGSEL;

// 原点回归属性参数结构体
typedef struct _STR_HOME_ATTRIB {
    Uint16 Step;              //原点回归流程步骤标志
    Uint16 Mode;              //原点复位模式
	Uint16 Trigger;           //原点开关触发标志
	Uint16 Elector;           //电气回零标志
	
	Uint32 TimeCount;         //回零计时器
	Uint16 Status;            //回零状态显示, 第一位为1表示普通回零成功; 第二位为1表示电气回零成功
	                          //第三位为1表示回零过程超时
    Uint16 HomActFlg;         //回零进行中标志 1:正在回零中，0:回零未进行
    Uint16 HighSpeedSearch;   //高速搜索原点速度    
    Uint16 LowSpeedSearch;    //低速搜索原点速度
    Uint16 HomeEnable;        //回零使能控制
	
    Uint32 RiseDownTime;      //搜索原点时的加减速时间(换算到以位置环周期为单位)    
    Uint16 SearchTime;        //查找原点的时间              
    int32  Offset;            //原点坐标偏移量(回原点之后将原点坐标设置为该值)
	int64  Dist;              //根据原点查找时间计算的回零距离
    
	Uint16 DITriggerOld;      //原点开关DI触发状态(前一刻)
	Uint16 Near;              //减速点状态
	Uint16 NearBkp;           //减速点状态备份
	Uint16 Zero;              //原点状态
	
	Uint16 ZeroBkp;           //原点状态备份
	Uint16 ZeroIndex;         //电机Z信号锁存
	int32  MoveDist;          //原点复归后的机械偏移距离
	Uint16 DirInvFlag;        //遇限反向标志
	
	Uint16 LenCmpsFlag;       //长度补偿标志，确保超程后仍以同一侧为原点
	Uint16 ZeroPosFlag;       //1：起始位置在原点开关处 0：不在
	Uint16 OTStatus;          //0:未超程 1：正向超程 2：负向超程 3：第二次超程
	Uint16 ZeroDownEdge;      //Z信号边沿 1:下降沿 0：上升沿


	Uint16 NearUpEdge;        //减速点信号边沿	
    volatile UNI_ORGSEL OriginSel;   //原点复归过程中偏移值使用及遇限位选择项
} STR_HOME_ATTRIB;


/*FUNC_Home.c文件内调用的变量的结构体类型*/


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void HomingInitOnce(void);    // 上电复位原点回归相关参数(上电执行一次)
void HomingStopUpdate(void);  // 停机更新原点回归相关参数
void DoHoming(void);          // 执行原点回归过程
int32 HomingPosCmd(void);     // 回原点时每个插补周期的位置指令
void HomingReset(void);       // 复位插补结构
void HomParaUpdateRealTim(void);//回零参数实时更新

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_Home.h */
