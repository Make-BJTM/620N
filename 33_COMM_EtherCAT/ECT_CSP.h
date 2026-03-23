#ifndef CANOPEN_INTERPOLATIONPOS_H
#define CANOPEN_INTERPOLATIONPOS_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	

#define GetSYNCTime_1MHzClk()    (*TIM10_CNT)      //系统时钟频率1MHz

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

typedef struct
{
    Uint32 SYNCPeriodRatio;//同步周期与插补周期的比值
    Uint32 SYNCPeriod;	//同步周期，单位ns
	int32 MaxInterpltPosCmd;//最大位置指令限制	
	int64 MaxDeltaTarPos;//最大位置指令增量
		
	int32 InterpltPoint;//插补目标位置
    int64 InterpltDltRef; //本段位置指令增量--编码器单位
	int64 InterpltSumRef;//当前未发送的位置指令
    int32 InterpltIncPer;    //每次位置环的位置指令   
    int64 RefPosRemainder;//细分后的位置指令余数

	
	int32   OTLatchPosCmd;    //超程记录的限位方向的多发的位置指令
	Uint8   OTDeal;
    Uint8   CSPRunFlag;//
	Uint16  P_overspeedcnt;
    
    int32   PosLoopCnt;//
    int8    ESMState;

}STR_ECTCSPVAR;

typedef struct{
    int64   InPut;                    //本同步周期输入位置指令总数
    int64   PosRefAve;
}STR_ECTCSPPOSREF;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

extern STR_ECTCSPVAR STR_ECTCSPVar;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 

extern int32 ECTCSPPosCmd(void); //计算插补位置指令
extern void ECTCSPUpdata(void);//参数更新
extern void InitECTCSP(void); //参数复位
extern void CSPClear(void);
extern void ECTCSPMaxSpeedUpdate(void);
extern void CSPPointCal(void);
//写环形缓冲
extern void WriteCSPFIFO(void); //SYNC 时调用

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif

