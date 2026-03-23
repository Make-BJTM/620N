/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Nikon_AbsRomOper.h  
 创建人：   王治国                  创建日期：2014.11.19
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __MTR_NIKON_ABSROMOPER_H
#define __MTR_NIKON_ABSROMOPER_H

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
    Uint16 AbsAngHigh:7;   //编码器绝对位置最高位
    Uint16 AbsComErr:1;    //通讯故障（RX端） AbsComErr = FramingError | TimeoutError | CrcError | CodeError | InfoError | DataError；
    Uint16 AbsEncErr:1;    //编码器故障（TX端）AbsEncErr = CountingError | MultiturnError | AbsOverSpeed | BatteryError；
    Uint16 AbsAlarm:1;     //编码器警告（TX端）AbsAlarm = DelimiterError | ParityError | BatteryAlarm | CounterOverflow | FullAbsStatus；
    Uint16 AbsComStatus:1; //通讯状态   0～空闲   1～忙碌
    Uint16 AbsMode:3;      //工作模式 0-No action（编码器被禁止），1-连续读数据（同步于电流环周期），2-随机读数据，
                            //3-随机读EEPROM，4-随机写EEPROM，5-复位绝对位置（即位置清零），
    Uint16 AbsType:2;      //编码器类型 0,3-未知类型，1-多摩川单圈式，2-多摩川多圈式
}STR_NK_ABSENCSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_NK_ABSENCSTATE_BIT     bit;
}UNI_NK_ABSENCSTATE_REG;


//绝对式编码器控制
typedef struct {
    Uint16 AbsTransferDelay:13;     //编码器数据传输延时
    Uint16 AbsMode:3;       //编码器工作模式  0-No action（编码器被禁止）  1-连续读数据（同步于电流环周期） 2-随机读数据
    //3-随机读EEPROM    4-随机写EEPROM    5-复位绝对位置（即位置清零）  6-复位报警信号  7-复位多圈数据及报警信号
}STR_NK_ABSENCCTRL_BIT;

typedef union 
{
    volatile Uint16                  all;
    volatile STR_NK_ABSENCCTRL_BIT      bit;
}UNI_NK_ABSENCTRL_REG;


//绝对式电机对EEPROM访问控制
typedef struct{
    Uint16 ByteToEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromMode:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_NK_MOTROMCTRL_BIT;

typedef union
{
    volatile Uint16              all;
    volatile STR_NK_MOTROMCTRL_BIT  bit;
}UNI_NK_MOTROMCTRL_REG;

//绝对式电机对EEPROM访问状态读取
typedef struct{
    Uint16 ByteFromEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromBusy:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_NK_MOTROMSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_NK_MOTROMSTATE_BIT     bit;
}UNI_NK_MOTROMSTATE_REG;


typedef struct{
	Uint16 TimeoutError:1;      //通讯超时错误（RX端）
    Uint16 FramingError:1;      //帧停止位错误（RX端）
	Uint16 CrcError:1;          //CRC校验错误（RX端）
	Uint16 DataError:1;         //数据字段错误（RX端）

    Uint16 AbsError:1;          //编码器故障（TX端）  CountingError + MultiturnError + BatteryError
    Uint16 AbsOverRun:1;        //编码器超限（TX端）  AbsOverSpeed + AbsOverHeat + MemError
	Uint16 Rsvd6:1;
	Uint16 AbsBusy:1;           //编码器繁忙（TX端）  RotBusy + MemBusy

	Uint16 Rsvd8:1;
	Uint16 Rsvd9:1;
	Uint16 BatteryAlarm:1;      //电池报警（TX端） 电池电压低   警告
	Uint16 Rsvd11:1;

	Uint16 Rsvd12:1;
	Uint16 AbsCntError:1;       //计数增量异常（算法）
	Uint16 AbsZeroError:1;      //零点搜索失败（算法）
	Uint16 AbsDivError:1;       //分频计算溢出（算法）
}STR_NKABSERR_BIT;

typedef union
{
    volatile Uint16                     all;
    volatile STR_NKABSERR_BIT           bit;
}UNI_NKABSERR_REG;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void NKAbsRom_InitDeal(void); 
extern void NKAbsRom_EncState(void); 
extern Uint8 NKAbsRom_RdErrStatus(void);
extern Uint8 NKAbsRom_ClcErrAndMultiTurn(Uint8 Mode);
extern void ClrNKAbsEncWarn(void);
extern void CalcNikonEncTransTime(void);
extern void NKAbsRomProcess(void);
extern Uint8 NKAbsEnc_SaveThetaOffset(void);

#ifdef __cplusplus
}
#endif

#endif /* __MTR_NIKON_ABSROMOPER_H */

/********************************* END OF FILE *********************************/
