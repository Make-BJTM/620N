/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_AbsRomOper.h  
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
#ifndef __MTR_ABSPOMOPER_H
#define __MTR_ABSPOMOPER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
//存入电机ROM中前2个是校验字,如果读出来的参数与FLASH不同,需要报错并重新写入电机参数
#define ABSROM_CHECK_WORD   0xAA55

//可以保存到电机ROM中的字长度  
#define ABS_ROM_WORDS_LEN   50
  
//可以保存到电机ROM中的字节长度           
#define ABS_ROM_BYTES_LEN   100   

//可以保存到电机ROM中的字长度  
#define TAMAGABS_ROM_WORDS_LEN   31
  
//可以保存到电机ROM中的字节长度           
#define TAMAGABS_ROM_BYTES_LEN   62  

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
}STR_ABSENCSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_ABSENCSTATE_BIT     bit;
}UNI_ABSENCSTATE_REG;


//绝对式编码器控制
typedef struct {
    Uint16 AbsTransferDelay:13;     //编码器数据传输延时
    Uint16 AbsMode:3;       //编码器工作模式  0-No action（编码器被禁止）  1-连续读数据（同步于电流环周期） 2-随机读数据
    //3-随机读EEPROM    4-随机写EEPROM    5-复位绝对位置（即位置清零）  6-复位报警信号  7-复位多圈数据及报警信号
}STR_ABSENCCTRL_BIT;

typedef union 
{
    volatile Uint16                  all;
    volatile STR_ABSENCCTRL_BIT      bit;
}UNI_ABSENCTRL_REG;


//绝对式电机对EEPROM访问控制或状态读取
typedef struct{
    Uint16 ByteToEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromMode:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_MOTROMCTRL_BIT;

typedef union
{
    volatile Uint16              all;
    volatile STR_MOTROMCTRL_BIT  bit;
}UNI_MOTROMCTRL_REG;

//绝对式电机对EEPROM访问控制或状态读取
typedef struct{
    Uint16 ByteFromEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromBusy:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_MOTROMSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_MOTROMSTATE_BIT     bit;
}UNI_MOTROMSTATE_REG;


typedef union{
    Uint8   all_8Bits[100];
    Uint16   all_16Bits[50];
}UNI_ABSROMDATA;

typedef struct{
	Uint16 TimeoutError:1;      //通讯超时错误（RX端）
    Uint16 FramingError:1;      //帧停止位错误（RX端）
	Uint16 CrcError:1;          //CRC校验错误（RX端）
	Uint16 DataError:1;         //数据字段错误（RX端）
    Uint16 CountingError:1;     //位置计数错误（TX端）
	Uint16 AbsOverSpeed:1;      //编码器超速（TX端）
	Uint16 AbsOverHeat:1;       //编码器过热（TX端）
	Uint16 FullAbsStatus:1;     //完全绝对状态（TX端）
    Uint16 AbsRomSfLow:2;       //编码器的SF[1:0]
    Uint16 AbsRomSfHigh:2;      //编码器的SF[7:6]
    Uint16 rsvd:1;              //保留
	Uint16 AbsCntError:1;       //计数增量异常（算法）
	Uint16 AbsZeroError:1;      //零点搜索失败（算法）
	Uint16 AbsDivError:1;       //分频计算溢出（算法）
}STR_ABSERR_BIT;

typedef struct{
	Uint16 TimeoutError:1;      //通讯超时错误（RX端）
    Uint16 FramingError:1;      //帧停止位错误（RX端）
	Uint16 CrcError:1;          //CRC校验错误（RX端）
	Uint16 DataError:1;         //数据字段错误（RX端）

    Uint16 CountingError:1;     //位置计数错误（TX端）
	Uint16 AbsOverSpeed:1;      //编码器超速（TX端）    不报警
	Uint16 AbsOverHeat:1;       //编码器过热（TX端）    警告
	Uint16 FullAbsStatus:1;     //完全绝对状态（TX端）  不报警

	Uint16 RotBusy:1;           //单圈解算错误（TX端）  重新上电
	Uint16 MultiturnError:1;    //多圈计数错误（TX端）  重新上电
	Uint16 BatteryAlarm:1;      //电池报警（TX端） 电池电压低   警告
	Uint16 BatteryError:1;      //电池失效（TX端） 断电过程中 电池电压低   故障复位 0x72=0x11 复位后立即生效

    Uint16 rsvd:1;              //保留
	Uint16 AbsCntError:1;       //计数增量异常（算法）
	Uint16 AbsZeroError:1;      //零点搜索失败（算法）
	Uint16 AbsDivError:1;       //分频计算溢出（算法）
}STR_HC2NDABSERR_BIT;

typedef union
{
    volatile Uint16                     all;
    volatile STR_ABSERR_BIT             bit;
    volatile STR_HC2NDABSERR_BIT        HC2ndbit;
}UNI_ABSERR_REG;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void HCAbsRom_InitDeal(void);    
extern void AbsRomProcess(void);           
extern void AbsRom_EncState(void);
extern Uint8 HC2ndAbsRom_ClcErrAndMultiTurn(Uint8 Mode);     //复位多圈编码器故障及多圈参数
extern void ClrHC2ndAbsEncWarn(void);          //清多圈编码器故障
extern void CalcHCEncTransTime(void);          //计算总通讯时间
extern Uint8 HCAbsEnc_SaveThetaOffset(void);   //存储角度辨识后的编码器位置偏置
extern void FunCodeSaveInAbsRomInit(void);     //初始化存储在编码器ROM中的功能码


extern Uint8 AbsRom_SoftRstProcess(void); //软件复位时禁止绝对式编码器读写
#ifdef __cplusplus
}
#endif

#endif /* __FUNC_ABSPOMOPER_H */

/********************************* END OF FILE *********************************/
