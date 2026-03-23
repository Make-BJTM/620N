/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_PanelKeyFunArray.h
 创建人：童文邹                创建日期：2008.10
 修改人：王治国                修改日期：2011.11.09
 描述：
       1.本文件声明了按键操作函数,提供给按键函数数组使用
       2.
 修改记录：
    1 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_PANELKEYFUNARRAY_H
#define FUNC_PANELKEYFUNARRAY_H 

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_PanelKey.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 常规类型定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//空函数
extern void NullFuntion(STR_PANELOPERRATION *p);
//第0级菜单-按键函数
extern void KeyModeFunZero(STR_PANELOPERRATION *p);
extern void KeyLongShiftFunZero(STR_PANELOPERRATION *p);
//第1级菜单-按键函数
extern void KeyModeFun1st(STR_PANELOPERRATION *p);
extern void KeyUpFun1st(STR_PANELOPERRATION *p);
extern void KeyDownFun1st(STR_PANELOPERRATION *p);
extern void KeyShiftFun1st(STR_PANELOPERRATION *p);
extern void KeySetFun1st(STR_PANELOPERRATION *p);
//第2级菜单-按键函数
extern void KeyModeFun2nd(STR_PANELOPERRATION *p);
extern void KeyUpFun2nd(STR_PANELOPERRATION *p);
extern void KeyDownFun2nd(STR_PANELOPERRATION *p);
extern void KeyShiftFun2nd(STR_PANELOPERRATION *p);
extern void KeySetFun2nd(STR_PANELOPERRATION *p);
//第3级菜单-按键函数
extern void KeyModeFun3rd(STR_PANELOPERRATION *p);
extern void KeyUpFun3rd(STR_PANELOPERRATION *p);
extern void KeyDownFun3rd(STR_PANELOPERRATION *p);
extern void KeyShiftFun3rd(STR_PANELOPERRATION *p);
extern void KeyLongShiftFun3rd(STR_PANELOPERRATION *p);
extern void KeySetFun3rd(STR_PANELOPERRATION *p);
//第4级菜单-按键函数
extern void KeyModeFun4th(STR_PANELOPERRATION *p);
extern void KeyUpFun4th(STR_PANELOPERRATION *p);
extern void KeyDownFun4th(STR_PANELOPERRATION *p);
extern void KeyShiftFun4th(STR_PANELOPERRATION *p);
extern void KeySetFun4th(STR_PANELOPERRATION *p);
//第5级菜单-按键函数
extern void KeyModeFun5th(STR_PANELOPERRATION *p);
//第6级菜单-按键函数
extern void KeyModeFun6th(STR_PANELOPERRATION *p);
extern void KeyLongSetFun6th(STR_PANELOPERRATION *p);
//第7级菜单-按键函数
extern void KeyModeFun7th(STR_PANELOPERRATION *p);




/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_PANELKEYFUNARRAY_H */

/********************************* END OF FILE *********************************/




