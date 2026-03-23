#ifndef ETHERCATPUBLIC_H
#define ETHERCATPUBLIC_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif

#include "PUB_GlobalPrototypes.h"
/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 


typedef struct{
    Uint32 InPut;		
    Uint32 ElecRatio6091_Numerator;
    Uint32 ElecRatio6091_Denominator; 
    Uint64 ElecRatioRemainderQ16;
}STR_ELECRATIO;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//extern STR_ELECRATIO  STR_ElecRatio;

//extern void InitECTElecRatio(STR_ELECRATIO *p);
//extern void ECTElecRatioUpdate(STR_ELECRATIO *p);


#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of EHTERCATPublic.h*/

/********************************* END OF FILE *********************************/
