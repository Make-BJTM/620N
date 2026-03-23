/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_CommInterface.h
 创建人:    邓开余
 修改人:    王治国              创建日期: 2011.10.31
 描述:
    1.功能模块与通信模块的接口有功能码和示波器缓冲区数组
    2.通信模块通过本文件的COMMWrRdFuncode来读取功能码
    3.示波器缓冲数组作为全局变量,通信模块直接进行访问
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_COMMINTERFACE_H
#define __FUNC_COMMINTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h" 

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/
// OperationMode 宏定义
#define     READSERIES    0x01
#define     WRITESERIES   0x02
#define     WRITEONE      0x03

#define     COMM_TYPE_MODBUS  0x00
#define     COMM_TYPE_CANLINK  0x01
#define     COMM_TYPE_CANOPEN  0x02
/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
typedef union{
    Uint8                   all_8Bits[24576];
    Uint16                  all_16Bits[12288];
    int32                   all_32Bits[6144];
}UNI_OSCILLOSCOPEBUFFER;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern UNI_OSCILLOSCOPEBUFFER   UNI_OsciBuffer;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//功能码读、写操作
extern Uint16 COMMWrRdFuncode(const Uint8 Group_Input,const Uint8 Offset,const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode,Uint8 CommType);
extern Uint16 COMMWrRdEleLabel(const Uint8 Offset, const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode);


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_COMMINTERFACE_H */

/********************************* END OF FILE *********************************/
