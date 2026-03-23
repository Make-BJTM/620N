#ifndef ETHERCATPROFILEPOSITION_H
#define ETHERCATPROFILEPOSITION_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct{
    int8   PosSyncModeConfigDone;     
}STR_ECATPOSSYNCVAR;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_ECATPOSSYNCVAR STR_EcatPosSync;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 


extern void InitTim7(void);
extern void InitTim7IRQ(void);
extern void Sync0PosControlSet(void);
extern void PosSyncCheck(void);
//extern void PosErrCalibration(void);

#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of FUNC_GlobalVariable.h*/

/********************************* END OF FILE *********************************/









