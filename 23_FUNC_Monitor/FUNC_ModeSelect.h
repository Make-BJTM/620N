/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_ModeSelect.h
 创建人：王军干
 修改人：李浩                修改日期：11.12.13 
 描述：
       1.
       2.      
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/
#ifndef FUNC_MODESELECT_H
#define FUNC_MODESELECT_H 

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
/* 常规类型定义 */   
 

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
/* 运行状态切换标志位位域结构体定义 */
typedef struct{
    //需内部Son使能
    Uint16 ResetABSThetaEn:1;        //多摩川绝对位置编码器绝对编码器初始角辨识
    Uint16 InertiaSon:1;             //惯量辨识Son
    Uint16 JOGSon:1;                 //JOG运行Son
    Uint16 TorqPiTuneSonLatch:1;     //电流环PI参数自调谐Son
    Uint16 FricSon:1;                //摩擦辨识内部Son
    Uint16 IncAngInitSon:1;          //省线编码器初始角度辨识内部Son
    Uint16 GUIWorkSon:1;             //后台模式Son
    Uint16 GUICtrlEnable:1;         //后台模式使能标志位

    //不需要内部Son使能
    Uint16 UVAdjustRatioLatch:1;     //UV相电流平衡校正使能锁存
    Uint16 OperAbsROMEn:1;           //多摩川绝对位置编码器ROM区读写使能
    Uint16 RunModLatch:3;            //模式锁存
    Uint16 ResearchZInSon:1;          //使能信号

    //保留
    Uint16 Rsvd:3;
}STR_MODSELECT_FLAG;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void ModSelect(void);
#if ECT_ENABLE_SWITCH
extern void ECTModeSelect(void);
#endif
#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_MODESELECT_H */

/********************************* END OF FILE *********************************/



