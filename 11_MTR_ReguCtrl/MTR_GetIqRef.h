/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                   
 文件名:   MTR_GetIqRef.h
 创建人：童文邹            创建日期：2008.09.02  
 修改人：朱祥华            修改日期：2011.10.28  
 描述： 
    1.伺服速度环变量及函数声明
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef MTR_GETIQREF_H
#define MTR_GETIQREF_H



#ifdef __cplusplus
extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 



/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
//暂无



/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	
//暂无



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
typedef struct{         //Ｑ轴电流指令的获取文件专用结构体isvo_test
    int32   ToqCmd_Limited;           //经过转矩限制后的转矩指令
    int32   IqOut;                    // 输出 OK

    //转矩模式下的速度限制用
    int32  PseudoSpdOut;              // 伪速度调节器输出
    Uint16  SaturaFlag;
    Uint16  OverLmtSpdFlg;            //是否超过限制速度标志位
    Uint16  OverLmtSpdWaitCnt;        //连续X次速度环速度反馈均超速即认为超速
	int32   PseudoSpeedRef;

    //转矩限制和保护类    
    int16   ToqLmtUdcDwn;             // 电压下降时转矩限制
    int32   Iuvw_CriticalCur;         //三相反馈电流过流保护点 单位为0.01A
    int32   Iq_CriticalCur;           //Iq过流保护点  转换成数字量后值 
    int16   Iqc_MT;                   //电机长期工作电流
    int16   Iqc_OEM;                  //驱动器长期工作电流

    //转矩基准，转矩到达开，转矩到达关百分比转矩转换成内部的数字量
    int32 ToqRchStandard;
    int32 ToqRchOn;
    int32 ToqRchOff;
}STR_GET_IQ_REF;




/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_GET_IQ_REF  STR_GetIqRef;    //Q轴电流指令获取文件专用结构体



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//以下函数在上电启机初始化程序 G_MTR_SysInit_STAR()中调度执行 
extern void InitGetIqRef(void);                 //上电启机初始化更新电流调节相关系数
extern void InitIqLmtValue(void);            //转矩限制值初始化

//以下函数在后台程序 G_MTR_Task_BKINT()（主循环）中调度执行
extern void GetIqRefStopUpdate(void);        //运行更新转矩指令获取文件参数
extern void GetIqRefUpdate(void);         //运行更新转矩指令获取文件参数
extern void IqLmtUpdateFun(void);         //运行更新转矩指令限幅

//以下函数在发波中断程序 G_MTR_Task_WGINT()（电流环调度）中调度执行
extern void IqLmtFun(void);                  //转矩限制值
extern void GetIqRef(void);                  //Ｑ轴电流指令的获取
extern void SWOvCur_FdbErrMonitor(void);     //软件er200过流报错监控功能函数

//以下函数在位置环中断执行
extern void ToqReachJudge(void);             //转矩到达判断DO输出

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


#ifdef __cplusplus
}
#endif

#endif  /* MTR_GetIqRef.h */

/********************************* END OF FILE *********************************/
