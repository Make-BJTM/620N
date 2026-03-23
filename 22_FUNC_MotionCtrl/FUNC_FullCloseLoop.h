                                /******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_FullCloseLoop.h
 创建人：刘伟                 创建日期：2012.9.19
 
 描述： 
     1.全闭环功能的头文件
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_FULLCLOSELOOP_H
#define FUNC_FULLCLOSELOOP_H


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


/*FUNC_MultiBlockPos.c文件内调用的变量的结构体类型*/
typedef struct  _STR_FULLCLOSELOOP { 
    // 多段位置功能共用变量
    Uint16  FeedbackMode;               //lw2012919   编码器反馈模式
	Uint16  ExCoderDir;                 //lw2012919   外部编码器工作方式
    //Uint32  ExCoderLine;                //lw2012919   外部编码器线数
    int64   ExInnerCoderParaQ20;           //lw2012919   电机旋转一圈外部编码器脉冲数
    Uint32  MixCtrlMaxPulse;            //lw2012919   混合控制偏差最大值
    Uint16  MixCtrlPulseClr;            //lw2012919   混合控制偏差清除
    //int16   ExPosFeedbackFlag;        //外部闭环标志
    int32   ExP2InerPosCoffQ7;       //外环转换至内环速度系数 Q12

    int32   ExPosAmplifErr;           //外部位置随动偏差
    int32   ExPosAmplifErrLast;             //外部位置偏差备份

    int32   InPosFdb;                 //内部位置反馈，用于位置偏差过大判断
	int32   ExPosFdb;                 //外部位置反馈
	int32   EncRev;                   //电机1圈编码器反馈脉冲数
    int32   DoubleFeedLowPassTs;                    //采样时间(us)
    int32   DoubleFeedLowPassTc;                    //滤波时间(us)
    int32   DoubleFeedLowPassKa;
    int32   DoubleFeedLowPassInput;
	int32   DoubleFeedLowPassOutput;
	int32   DoubleFeedLowPassOutputLatch;
	int32   DoubleFeedLowPassRemainder;
}STR_FULLCLOSE_LOOP;	 


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_FULLCLOSE_LOOP STR_FullCloseLoop;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

extern void FullCloseLoopErrorCheck(Uint8 ExPosFeedbackFlag);  //偏差过大保护，打滑保护
//全闭环反馈，转换系数等处理
extern void FullCloseLoopModeSet(int32 * pPosFdb, Uint32 *pExPosCoefQ7, int32 ExPosFdbAbsValue,Uint8 ExPosFeedbackFlag);   
//全闭环初始化，输入电机线数
extern void FullCloseLoopInit(Uint32 EncRev);
//全闭环参数清零
extern void ClearFullLoopVar(void);
//双位置全闭环时位置偏差计算
extern int32 DoublePosFeedbackCal(int32 PosRef,int32 *PosAmplifErr);
//全闭环相关的偏差，位置反馈显示
extern void FullCloseParaDisp(void);

extern void FullCloseParaRst(void);

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_MultiBlockPos.h */

/********************************* END OF FILE *********************************/
