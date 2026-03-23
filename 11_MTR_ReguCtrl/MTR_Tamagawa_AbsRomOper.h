/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Tamagawa_AbsRomOper.h  
 创建人：   王治国                  创建日期：2014.11.19
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __MTR_TAMAGAWA_ABSROMOPER_H
#define __MTR_TAMAGAWA_ABSROMOPER_H

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
}STR_TAMAG_ABSENCSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_TAMAG_ABSENCSTATE_BIT     bit;
}UNI_TAMAG_ABSENCSTATE_REG;


//绝对式编码器控制
typedef struct {
    Uint16 AbsTransferDelay:13;     //编码器数据传输延时
    Uint16 AbsMode:3;               //编码器工作模式 
}STR_TAMAG_ABSENCCTRL_BIT;

typedef union 
{
    volatile Uint16                     all;
    volatile STR_TAMAG_ABSENCCTRL_BIT      bit;
}UNI_TAMAG_ABSENCTRL_REG;


//绝对式电机对EEPROM访问控制
typedef struct{
    Uint16 ByteToEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromMode:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_TAMAG_MOTROMCTRL_BIT;

typedef union
{
    volatile Uint16                      all;
    volatile STR_TAMAG_MOTROMCTRL_BIT    bit;
}UNI_TAMAG_MOTROMCTRL_REG;


//绝对式电机对EEPROM访问状态读取
typedef struct{
    Uint16 ByteFromEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromBusy:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_TAMAG_MOTROMSTATE_BIT;

typedef union
{
    volatile Uint16                         all;
    volatile STR_TAMAG_MOTROMSTATE_BIT      bit;
}UNI_TAMAG_MOTROMSTATE_REG;


typedef struct{
	Uint16 TimeoutError:1;      //通讯超时错误（RX端）  只在编码器上电 连续读数据模式前几个周期检测 
    Uint16 FramingError:1;      //帧停止位错误（RX端）  忽略
	Uint16 CrcError:1;          //CRC校验错误（RX端）   忽略
	Uint16 DataError:1;         //数据字段错误（RX端）  忽略

    Uint16 CountingError:1;     //位置计数错误（TX端）  忽略
	Uint16 AbsOverSpeed:1;      //编码器超速（TX端）    忽略
	Uint16 AbsOverHeat:1;       //编码器过热（TX端）    警告
	Uint16 FullAbsStatus:1;     //完全绝对状态（TX端）  忽略

    Uint16 CommError:1;         //通讯校验错误（TX端）  忽略
    Uint16 MultiturnError:1;    //多圈计数错误  故障 
    Uint16 BatteryAlarm:1;      //电池报警       警告
    Uint16 BatteryError:1;      //电池失效      故障

    Uint16 CounterOverflow:1;   //多圈计数器溢出（TX端） 故障 
	Uint16 AbsCntError:1;       //计数增量异常（算法） 故障
	Uint16 AbsZeroError:1;      //零点搜索失败（算法） 警告
	Uint16 AbsDivError:1;       //分频计算溢出（算法） 警告
}STR_TAMAGABSERR_BIT;

typedef union
{
    volatile Uint16                     all;
    volatile STR_TAMAGABSERR_BIT           bit;
}UNI_TAMAGABSERR_REG;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void TAMAGAbsRom_InitDeal(void);
extern void TAMAGAbsRom_EncState(void);
extern void CalcTAMAGEncTransTime(void);
extern void TAMAGAbsRomProcess(void);
extern Uint8 TAMAGAbsEnc_SaveThetaOffset(void);
extern Uint8 TAMAGAbsRom_ClcErrAndMultiTurn(Uint8 Mode); 
extern void ClrTAMAGAbsEncWarn(void);

#ifdef __cplusplus
}
#endif

#endif /* __MTR_TAMAGAWA_ABSROMOPER_H */

/********************************* END OF FILE *********************************/
