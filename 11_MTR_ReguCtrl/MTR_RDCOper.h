/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_RDCOper.h  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   王治国                 修改日期：2012.06.28
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __MTR_RDCOPER_H
#define __MTR_RDCOPER_H

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
/* 结构体变量类型定义 枚举变量类型定义 */ 
//编码器运行状态
typedef struct{
    Uint16 AbsAngHigh:4;   //编码器绝对位置最高位，当前分辨率为17位，因而只用这里的一位
    Uint16 Rsvd:2;         //保留位
	Uint16 AbsDataValid:1;  //当编码器数据传输完全正确时（未发生通讯等错误），该位为1，否则为0；
    Uint16 AbsComErr:1;    //通讯故障（RX端） AbsComErr = FramingError | TimeoutError | CrcError | CodeError | InfoError | DataError；
    Uint16 AbsEncErr:1;    //编码器故障（TX端）AbsEncErr = CountingError | MultiturnError | AbsOverSpeed | BatteryError；
    Uint16 AbsAlarm:1;     //编码器警告（TX端）AbsAlarm = DelimiterError | ParityError | BatteryAlarm | CounterOverflow | FullAbsStatus；
    Uint16 AbsComStatus:1; //通讯状态   0～空闲   1～忙碌
    Uint16 AbsMode:3;      //工作模式 0-No action（编码器被禁止），1-连续读数据（同步于电流环周期），2-随机读数据，
                            //3-随机读EEPROM，4-随机写EEPROM，5-复位绝对位置（即位置清零），
    Uint16 AbsType:2;      //编码器类型 0,3-未知类型，1-多摩川单圈式，2-多摩川多圈式
}STR_RDCSTATE_BIT;

typedef union
{
    volatile Uint16                 all;
    volatile STR_RDCSTATE_BIT       bit;
}UNI_RDCSTATE_REG;

typedef struct{
	Uint16 Rsvd0:2;             
    Uint16 ParityError:1;   //奇偶校验错误（RX端）   不处理
	Uint16 DataError:1;     //数据类型错误（RX端）   不处理

	Uint16 DosFlag:1;       //RDC信号畸变（TX端）    连续3次报错
    Uint16 LotFlag:1;       //RDC跟踪失效（TX端）    连续3次报错
	Uint16 Rsvd1:2;

	Uint16 Rsvd2:4;

	Uint16 Rsvd3:1;
	Uint16 AbsCntError:1;       //计数增量异常（算法） 立即报错
	Uint16 AbsZeroError:1;      //零点搜索失败（算法）
	Uint16 AbsDivError:1;       //分频计算溢出（算法）
}STR_RDCERR_BIT;


typedef union
{
    volatile Uint16             all;
    volatile STR_RDCERR_BIT     bit;
}UNI_RDCERR_REG;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void RDConverterState(void);     //得到旋变解码芯片运行状态
extern void CalcRDCTransTime(void);     //计算总通讯时间

#ifdef __cplusplus
}
#endif

#endif /* __MTR_RDCOPER_H */

/********************************* END OF FILE *********************************/
