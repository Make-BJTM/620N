/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_MultiBlockPos.h
 创建人：匡两传                 创建日期：2009.12.16
 修订人：何云壮                 修订日期：2012.02.03 
 描述： 
     1.多段位置功能的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_MULTIBLOCKPOS_H
#define FUNC_MULTIBLOCKPOS_H


#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define  MAX_BLOCKS  (16)   // 多段位置最大段数

#define MultiPCMD_Defaults {0,0,0,0,0,0,0,0,0,{0},{0},{0},{0}}

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
// 多段位置指令运行方式
enum MULTI_POS_RUNNING_TYPE {
    ONCE_TYPE = 0,      // 单次运行
    CYCLE_TYPE = 1,     // 逐段依次循环运行
	DI_SWITCH_TYPE = 2, // 通过外部DI端口切换段号
	CONTINUOUS_TYPE = 3 // 顺序运行(段与段之间没有等待时间), 又分为单次顺序运行
	                    // 与循环顺序运行, 其中循环顺序运行除一轮之外, 其余各轮
						// 可从指定段开始运行(功能码H11.05设置)
};

// 断使能时正在插补的话可能留有余量, 余量处理方式
enum REST_PULSE_HANDLING_TYPE {
    CONTINUE_REST = 0,  // 继续走完剩下的段数; 注意: 不是被打断的段余下的脉冲
	IGNORE_REST = 1     // 忽略剩余量重新从第一段开始执行
};

// 多段位置指令类型
enum POSCMD_TYPE {
    INCREAMENTAL_TYPE = 0, // 增量型位置指令
	ABSOLUTE_TYPE = 1      // 绝对型位置指令
};

/*FUNC_MultiBlockPos.c文件内调用的变量的结构体类型*/
typedef struct  _STR_MULTI_BLOCK_POS { 
    // 多段位置功能共用变量
    Uint16 ExeBlockNum;           //选择执行的总段数，最大16
    Uint16 CurrentBlock;          //记录当前段			  
    Uint16 WaitFlag;              //多段位置等待标志			   
    Uint16 RunFlag;               //多段位置运行标志
    Uint16 RunMode;               //多段运行模式
    Uint16 RefleshNum;            //段更新标志
    Uint16 RefType;               //位置指令类型
    Uint16 RemainderHndFlg;       //剩余位移处理方式
    Uint16 TimeUnit;              //等待时间单位
   
    // 各段参数		
    Uint32 RunSpeed[MAX_BLOCKS];          //记录每段恒速
    Uint32 RiseorDownTime[MAX_BLOCKS];    //记录每段的加减速时间(换算到以位置环周期为单位)
    int32  PulsNumPerBlock[MAX_BLOCKS];   //记录每段的总脉冲数
    Uint32 WaitTimePerBlock[MAX_BLOCKS+1];  //记录每段的等待时间  
}STR_MULTI_BLOCK_POS;	   


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern int32 MultiPosHandle(void);     // 实现多段位置功能, 伺服运行时调用
extern void MultiPosInit(void);       // 多段位置参数初始化, 伺服OFF时重新初始化
extern void MultiPosReset(void);      // 参数复位
extern void MultiPosRunUpdate(void);  // 实时更新多段位置参数


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_MultiBlockPos.h */

/********************************* END OF FILE *********************************/
