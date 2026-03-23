/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名: FUNC_DiDo.h	
 创建人：童文邹                
 修订人：李浩                 创建日期：11.11.18 
 描述： 
     1.
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_DIDO_H
#define FUNC_DIDO_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define DO_TERMINALS   8
#define DI_TERMINALS  10

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

typedef struct{
    Uint16 DiFuncSel[9];        //DI分配,DI1~9 立即有效	   
    Uint16 DiLogicSel[9];       //有效方式选择,立即有效DI1~9 ->[0]~[8]	   
    Uint8  SpiDiValue;          //SPI读入DI的最新值
    Uint8  SpiDiValueLast;      //上一次SPI读入的DI值
    Uint8  GpioDi8Value;        //GPIO输入的DI8的值
    Uint8  GpioDi9Value;        //GPIO输入的DI9的值

    Uint16 DiPortEnState;       //10个Di端口功能是否使能有效的中间运算变量

    Uint16 DiPortChangeEn;      //Di端口变更后使能标志位

    //Uint16 XIntPosEnDi9;        //Di9端口中断定长是否使能
    Uint16 XIntPosEnDi9LogicLatch;  //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
    Uint16 TouchProbeEnDi8LogicLatch;  //探针1使能时，Di8Logic电平选择锁存，此时发生变化时应报Er.941
    Uint16 TouchProbeEnDi9LogicLatch;  //探针2使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
}STR_DI_STATE;

typedef struct{
    Uint16 DoFuncSel[5];       //Do分配,Do1~5 立即有效	   
    Uint16 DoLogicSel[5];      //有效方式选择,立即有效Do1~5 ->[0]~[4]

    Uint16 DoPortEnState;      //5个Do端口功能是否使能有效的中间运算变量
    Uint8  BrakeDoEn;          //普通抱闸功能Do端口分配使能
}STR_DO_STATE;


typedef struct{
    Uint16  Rsvd0:1;      //bit0
    Uint16  DiPort1:1;    //Bit1 端口1
    Uint16  DiPort2:1;    //Bit2 端口2
    Uint16  DiPort3:1;    //Bit3 端口3
    Uint16  DiPort4:1;    //Bit4 端口4
    Uint16  DiPort5:1;    //Bit5 端口5
    Uint16  DiPort6:1;    //Bit6 端口6
    Uint16  DiPort7:1;    //Bit7 端口7
    Uint16  DiPort8:1;    //Bit8 端口8
    Uint16  DiPort9:1;    //Bit9 端口9
    Uint16  Rsvd1:6;
}STR_DI_PORT;
typedef union{
    volatile Uint16                  all;
    volatile STR_DI_PORT             bit;
}UNI_DI_PORT;


/*DO位域结构体定义*/
typedef struct{
    Uint32 SRdy:1;    // bit 0 伺服状态准备好，可以接收S-ON信号
    Uint32 TGon:1;    // bit 1 伺服电机的转速高于设定值时ON（闭合）
    Uint32 Zero:1;    // bit 2 零速信号输出 
    Uint32 VCmp:1;    // bit 3 速度控制时，电机速度在设定范围内与速度指令一致时ON
    Uint32 Coin:1;    // bit 4 位置控制时，位置偏差脉冲到达设定范围内时ON(即定位完成信号)
    Uint32 Near:1;    // bit 5 定位接近
    Uint32 Clt:1;     // bit 6 转矩限制的确认信号
    Uint32 Vlt:1;     // bit 7 速度限制的确认信号
    Uint32 Blk:1;     // bit 8 抱闸信号
    Uint32 Warn:1;    // bit 9 警告输出信号
    Uint32 Alm:1;     // bit 10 检出故障时OFF
    Uint32 AlmCode:3; // bit 11~13 固定DO6~DO8输出 输出3为报警代码
    Uint32 Xintcoin:1;      // bit 14 中断定长完成信号 ZYJ
    Uint32 OrgOk:1;         // bit 15 原点回零OK
    Uint32 OrgOkElectric:1; // bit 16 原点回零OK
    Uint32 ToqReach:1;      // bit 17 转矩指令到达信号
    Uint32 VArr:1;          // bit 18 速度到达输出
    Uint32 AngIntRdy:1;     //bit 19  初始角度辨识完成
    Uint32 Rsvd1:12;        // bit  20～31 保留位
}STR_DOVARREG; 

/*DO共用体定义*/
typedef union{
    volatile Uint32          all;
    volatile STR_DOVARREG    bit;
}UNI_DOVARREG;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_DI_STATE   STR_DiState;



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitDiDoState(void);
extern void DiProcess(void);
extern void DoProcess(void);
extern void DealDiFromGpio(void);

extern void DoAllocateError(void);
extern void DiAllocateError(void);
extern void DiDoStateStopUpdata(void);
extern void TouchProbe1andDIConflictError(void);
extern void TouchProbe2andDIConflictError(void);
#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_DiDo.h */


/********************************* END OF FILE *********************************/
