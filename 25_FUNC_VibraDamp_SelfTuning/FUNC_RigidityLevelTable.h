/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_RigidityLevelTable.h
 创建人:    朱祥华               创建日期：2012.07.24      
 描述:
    1. 
    2. 

 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/ 
#ifndef __FUNC_RIGIDITYLEVELTABLE_H
#define __FUNC_RIGIDITYLEVELTABLE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
typedef struct {     // bits   description
   Uint16 SpdKpOk:1;           // 0  H0800增益更新完毕
   Uint16 SpdTiOk:1;           // 1  H0801增益更新完毕
   Uint16 PosKpOk:1;           // 2  H0802增益更新完毕
   Uint16 ToqFiltTimeOk:1;     // 3  H0705增益更新完毕
   //，0~刚性表未改变 1~刚性表更改后初次进行参数初始化 2~增益参数更新延时处理
   Uint16 Status:2;            // 4~5 更新状态
   Uint16 RSVD:10;         // 15:6  RSVD
}STR_FUNC_RIGIDITYLEVEL_BITS;

typedef union {
   volatile Uint16                        all;
   volatile STR_FUNC_RIGIDITYLEVEL_BITS   bit;
}UNI_FUNC_RIGIDITYLEVEL_REG;

typedef struct{
    Uint16 LevelOld;    //旧的刚性等级
    Uint16 LevelNew;    //新的刚性等级

    Uint16 SpdKp_Old;       //刚性等级更改后旧的0800比例值
    Uint16 SpdTi_Old;       //刚性等级更改后旧的0801比例值
    Uint16 PosKp_Old;       //刚性等级更改后旧的0802比例值
    Uint16 ToqFiltTime_Old; //刚性等级更改后旧的0705比例值

    Uint16 SpdKp_New;       //刚性等级更改后新的0800比例值
    Uint16 SpdTi_New;       //刚性等级更改后新的0801比例值
    Uint16 PosKp_New;       //刚性等级更改后新的0802比例值
    Uint16 ToqFiltTime_New; //刚性等级更改后新的0705比例值

    Uint16 SpdKp_CalTemp;   //H0800增益延时累加计算中间值
    Uint16 SpdTi_CalTemp;   //H0801增益延时累加计算中间值
    Uint16 PosKp_CalTemp;   //H0802增益延时累加计算中间值
    Uint16 ToqFiltTime_CalTemp;//H0705增益延时累加计算中间值

    UNI_FUNC_RIGIDITYLEVEL_REG  Flag;
}STR_FUNC_RIGIDITY;


//刚性表数据结构
typedef struct
{
	Uint16 PosKp;				 //位置环比例
    Uint16 SpdKp;				 //速度环比例
	Uint16 SpdTi;				 //速度环积分
	Uint16 ToqFiltTime;			 //转矩积分时间
}STR_RGDTYTABLE;
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern void Rigidity_LevelProcess(void);

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_DRIVERPARA_H */

/********************************* END OF FILE *********************************/
