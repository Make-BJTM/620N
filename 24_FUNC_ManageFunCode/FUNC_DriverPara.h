/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_DriverPara.h  
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.      
    2. 	   
********************************************************************************/ 
#ifndef __FUNC_DRIVERPARA_H
#define __FUNC_DRIVERPARA_H

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
/* 结构体变量类型定义 枚举变量类型定义 */ 
typedef struct{
    Uint16 RsdServoSeri;        //H01_03 保留的用于比较的驱动器型号
    Uint16 VoltClass;           //H01_04 电压级
    Uint16 RatePower;           //H01_05 额定功率
    Uint16 MaxPowerOut;         //H01_06 最大输出功率
    Uint16 RateCurrent;         //H01_07 驱动器额定输出电流
    Uint16 MaxCurrentOut;       //H01_08 驱动器最大输出电流
    Uint16 SdmClkEnbl;          //H01_09 Σ-Δ调制器外部时钟使能
    Uint16 CarrWaveFreq;        //H01_10 载波频率
    Uint16 ToqLoopFreqSel;      //H01_11 电流环调制频率选择
    Uint16 SpdLoopFreqScal;     //H01_12 速度环调度分频系数
    Uint16 PosLoopFreqScal;     //H01_13 位置环调度分频系数
    Uint16 PwmDeadT;            //H01_14 死区时间
    Uint16 UdcOver;             //H01_15 直流母线过压保护点
    Uint16 UdcLeak;             //H01_16 直流母线电压泄放点
    Uint16 UdcLow;              //H01_17 直流母线电压欠压点
    Uint16 OCProtectPoint;      //H01_18 驱动器过流保护点
    Uint16 DeadComp;            //H01_20 死区补偿时间
    Uint16 RBMinOhm;            //H02_21 驱动器允许的能耗电阻最小值
    Uint16 RBChoice;            //H02_25 能耗电阻设置
    Uint16 RBPSizeI;            //H02_2x 能耗电阻功率容量
    Uint16 RBOhmI;              //H02_2x 能耗电阻阻值 
    Uint16 MotorModel;          //H00_00 默认电机型号
    Uint16 IS7860Gain;          //H01_19 7860采样系数
    Uint16 OvrCurUV;            //H01_64 U V相7860检测保护点 
}STR_DRIVERPARA;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern void DealDriverPara(void);

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_DRIVERPARA_H */

/********************************* END OF FILE *********************************/
