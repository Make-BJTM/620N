
#ifndef ETHERCATTOUCHPROBE_H
#define ETHERCATTOUCHPROBE_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类 */	


/* 宏定义 函数类 */	



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct{
Uint8 TrigEnbl:1;
Uint8 TrigSrc:1;//0-中断信号为DI8，1-中断信号为ENC1_Z（直线编码器Z脉冲）
Uint8 TrigMode:2;//0-无触发，1-上升沿触发，2-下降沿触发，3-边沿触发（注意中断信号的两个边沿之间的时差应大于同步周期，否则按最后一个边沿捕获数据）；
Uint8 PosFbkMux:1;//0-电机编码器位置反馈，1-直线编码器位置反馈
Uint8 Rsvd3:3;
}STRPROBECONFIG_BIT;

typedef union{
    Uint8 all;
    volatile STRPROBECONFIG_BIT bit;
}UNI_PROBECONFIG;


typedef struct{
Uint8 Enable:1;
Uint8 TrigMode:1;
Uint8 TrigSource:1;
Uint8 Rsvd3:1;
Uint8 RiseEdgeEnable:1;
Uint8 DownEdgeEnable:1; 
Uint8 Rsvd:2;
}STRPROBE1CTRL_BIT;

typedef union{
    Uint8 all;
    volatile STRPROBE1CTRL_BIT bit;
}UNI_PROBE1CTRL;

typedef struct{
Uint8 Enable:1;
Uint8 RiseEdgeStored:1;
Uint8 DownEdgeStored:1;
Uint8 Rsvd3:1;
Uint8 Rsvd4:1;
Uint8 Rsvd5:1;
Uint8 TrigSource:1;
Uint8 TrigStatus:1; 
}STRPROBE1STATUS_BIT;

typedef union{
    Uint8 all;
    volatile STRPROBE1STATUS_BIT bit;
}UNI_PROBE1STATUSL;

typedef struct{
Uint8 Enable:1;
Uint8 TrigMode:1;
Uint8 TrigSource:1;
Uint8 Rsvd3:1;
Uint8 RiseEdgeEnable:1;
Uint8 DownEdgeEnable:1; 
Uint8 Rsvd:2;
}STRPROBE2CTRL_BIT;

typedef union{
    Uint8 all;
    volatile STRPROBE2CTRL_BIT bit;
}UNI_PROBE2CTRL;

typedef struct{
Uint8 Enable:1;
Uint8 RiseEdgeStored:1;
Uint8 DownEdgeStored:1;
Uint8 Rsvd3:1;
Uint8 Rsvd4:1;
Uint8 Rsvd5:1;
Uint8 TrigSource:1;
Uint8 TrigStatus:1; 
}STRPROBE2STATUS_BIT;

typedef union{
    Uint8 all;
    volatile STRPROBE2STATUS_BIT bit;
}UNI_PROBE2STATUSL;

typedef struct{
    UNI_PROBECONFIG    Probe1Config;
    UNI_PROBECONFIG    Probe2Config;
    
    UNI_PROBE1CTRL     Probe1Ctrl;
    UNI_PROBE1STATUSL  Probe1Status;
    int32 Pbobe1RiseEdgeValue;
    int32 Pbobe1DownEdgeValue;
    Uint16 TP1PosCnt;
    Uint16 TP1NegCnt;
    
    UNI_PROBE2CTRL     Probe2Ctrl;
    UNI_PROBE2STATUSL  Probe2Status;
    int32 Pbobe2RiseEdgeValue;
    int32 Pbobe2DownEdgeValue;
    Uint16 TP2PosCnt;
    Uint16 TP2NegCnt;

    Uint32 AbsPosLow;
    Uint32 AbsPosHig;
}STR_PROBEVARIBLES;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

extern STR_PROBEVARIBLES  STR_ProbeVar;



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void TouchProbeInit(void);
extern void TouchProbeFunc(int32 DeltaPosFdb);
extern void TouchProbeZeroIndexISR(void);


#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of FUNC_GlobalVariable.h*/

/********************************* END OF FILE *********************************/ 


