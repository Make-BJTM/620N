#ifndef ETHERCATPROFILETORQUE_H
#define ETHERCATPROFILETORQUE_H

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

    int32  TargetToq;       //6071h 0.1%-->额定电流
    int32  ToqSoftStartRefLatch;
    int64  ToqSoftStartRefLatchQ12;

    int64  DeltaForward_Q12;   //每个电流环转矩指令增量
    int64  DeltaBackword_Q12;   //每个电流环转矩指令增量
    
    int16  ToqDemmandVal;
    int16  ToqActualVal;
    //Uint16 ToqSlope;
    
    //Uint16 MaxTorque;
    Uint16 MaxPosToq;
    Uint16 MaxNegToq;
    
}STR_ECTPTVAR;




/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

extern STR_ECTPTVAR STR_ECTPTVar;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

//extern  void InitECTPTMode(void);//转矩模式变量初始化
extern  int32 GetECTPTRef(void);//转矩指令来源

//extern  void ECTPTLimit(void);//转矩指令限制
extern void ECTToqShow(void);
void ECTInteruptToqUpdate(void);
#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of FUNC_GlobalVariable.h*/

/********************************* END OF FILE *********************************/









