/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_ServoError.h
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
#ifndef FUNC_SERVOERROR_H
#define FUNC_SERVOERROR_H 

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
/*伺服故障标志位位结构体定义*/
typedef struct{
    Uint16  Err2RstFlag:1;     // 0 :故障2标志位	            -》Err2EnFR
    Uint16  Err1RstFlag:1;     // 1 :复位故障1标志位            -》Err1EnFR
    Uint16  Err1UnRstFlag:1;   // 2 :不可复位故障1标志位        -》Err1UnFR 
    Uint16  ErrDoRecFlg:1;     // 3 :故障信息Do输出标志位       -》ErrRecFlg
    Uint16  ErrMsgRecordEn:1;  // 4 :故障存储使能
    Uint16  RdEncEepromEn:1;   // 5 :故障读编码器参数使能
    Uint16  Rsvd:10;           // 6-15: 保留 
}STR_SERVO_ERROR_FLAG;

/*伺服故障标志位共用体定义*/
typedef union{
    volatile Uint16                  all;
    volatile STR_SERVO_ERROR_FLAG    bit;
}UNI_SERVO_ERROR_FLAG;

/*伺服故障变量结构体定义*/
typedef struct{
    
    Uint16  ErrCodeBuff[4];    //故障码缓存

}STR_SERVO_ERROR_VARIABLE;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void ErrProcess_MaimLoop(void);
extern void GetWarningFlag(void);

#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_SERVOERROR */

/********************************* END OF FILE *********************************/



