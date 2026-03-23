/*************** (C) COPYRIGHT 2013  Inovance Technology Co., Ltd****************
* File Name		: ECT_InterFace.h
* Author		: 	
* Version		: 
* Date			: 
* Description	: 
* Modify		:
********************************************************************************/

#ifndef __ECT_INTERFACE_H
#define __ECT_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"
/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

//通讯读功能码
extern Uint16 C_Read_FunCode(Uint16 GroupIndex,Uint8 Offset,Uint8*dataSize,Uint32* pData);
//通讯写功能码
extern Uint16 C_Write_FunCode(Uint16 GroupIndex,Uint8 Offset,Uint8 dataSize,Uint32 Data,Uint8 EepromFlag);

//功能码地址映射
//wPDOx---记录同步位置模式下，607A映射的PDO
extern Uint8 *C_pGetFuncodeAddr(Uint16 GroupIndex, Uint8 Offset);

//返回设备应用层当前状态，由应用层提供
extern Uint32 C_DeviceAppState(Uint16 * ESM);

//返回通信层运行状态，由通信层提供
extern Uint8  C_CommState(Uint16 * ESM);

// 返回通讯周期
extern Uint32 C_GetSyncCycle(void);

//写环形缓冲
//extern void WriteCSPFIFO(void); //SYNC 时调用


//记录总线上的错误
extern void C_ECT_DrvStatus (Uint16 Port0CRC,Uint16 Port1CRC,Uint16 ForwardErr,
Uint16 HandleErr,Uint16 LinkLost,Uint16 SyncLostCnt,Uint8 SaveFlag);
extern void C_EcatSetXmlVersion(Uint16 Ver);
extern void C_EcatSetAlStatusCode(Uint16 alStatusCode);

//测试程序
extern void ECTTest(void);
extern  void   EcatMainLoop(Uint16 SyncErrLimit,Uint16* StationAdd,Uint16* pAlias,Uint8 Mode);
extern  void   PDI_Isr(void);
extern  void   Sync0_Isr(void);
extern  void   Sync1_Isr(void);


#ifdef __cplusplus
}
#endif

#endif
