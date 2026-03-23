/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_HDH_RomOper.h  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   王治国                 修改日期：2014.08.18
 描述： 
    1.
    2.
 修改记录：  
    2014.08.18
    1.支持海德汉编码器
    2.
********************************************************************************/ 
#ifndef __MTR_HDH_ABSPOMOPER_H
#define __MTR_HDH_ABSPOMOPER_H

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
//编码器运行状态  0x22
typedef struct{
    Uint16 AbsAngHigh:4;   //编码器绝对位置最高位，当前分辨率为17位，因而只用这里的一位
    Uint16 Rsvd:2;         //保留位
	Uint16 AbsDataValid:1;  //当编码器数据传输完全正确时（未发生通讯等错误），该位为1，否则为0；
    Uint16 AbsComErr:1;    //通讯故障（RX端） AbsComErr = FramingError | TimeoutError | CrcError | CodeError | InfoError | DataError；
    Uint16 AbsEncErr:1;    //编码器故障（TX端）AbsEncErr = CountingError | MultiturnError | AbsOverSpeed | BatteryError；
    Uint16 AbsAlarm:1;     //编码器警告（TX端）AbsAlarm = DelimiterError | ParityError | BatteryAlarm | CounterOverflow | FullAbsStatus；
    Uint16 AbsComStatus:1; //通讯状态   0～空闲   1～忙碌
    Uint16 AbsMode:3;      //工作模式
    Uint16 AbsType:2;      //编码器类型
}STR_HDHABSENCSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_HDHABSENCSTATE_BIT     bit;
}UNI_HDHABSENCSTATE_REG;


//串行编码器故障状态 0x20
typedef struct{
	Uint16 TimeoutError:1;      //通讯超时错误（RX端）
    Uint16 FramingError:1;      //帧停止位错误（RX端）
	Uint16 CrcError:1;          //CRC校验错误（RX端）
	Uint16 DataError:1;         //数据字段错误（RX端）
    Uint16 CountingError:1;     //位置计数错误（TX端）
    Uint16 Rsvd:8;              //保留
	Uint16 AbsCntError:1;       //计数增量异常（算法）
	Uint16 AbsZeroError:1;      //零点搜索失败（算法）
	Uint16 AbsDivError:1;       //分频计算溢出（算法）
}STR_HDHABSERR_BIT; 

typedef union
{
    volatile Uint16                     all;
    volatile STR_HDHABSERR_BIT             bit;
}UNI_HDHABSERR_REG;


//绝对式编码器控制  0x14
typedef struct {
    Uint16 AbsTransferDelay:13;     //编码器数据传输延时
    Uint16 AbsMode:3;       //编码器工作模式  
}STR_HDHABSENCCTRL_BIT;

typedef union 
{
    volatile Uint16                  all;
    volatile STR_HDHABSENCCTRL_BIT      bit;
}UNI_HDHABSENCTRL_REG;   

//绝对式电机对EEPROM访问控制或状态读取   0x3B
typedef struct{
    Uint16 ByteToEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromMode:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_HDHMOTROMCTRL_BIT;

typedef union
{
    volatile Uint16              all;
    volatile STR_HDHMOTROMCTRL_BIT  bit;
}UNI_HDHMOTROMCTRL_REG;

//绝对式电机对EEPROM访问控制2    0x3E
typedef struct {
    Uint16 MrsCode:8;     
    Uint16 ByteToEeprom2:8;       
}STR_HDHMOTROMCTRL2_BIT;

typedef union 
{
    volatile Uint16                  all;
    volatile STR_HDHMOTROMCTRL2_BIT      bit;
}UNI_HDHMOTROMCTRL2_REG;


//绝对式电机对EEPROM访问控制或状态读取     0x3B
typedef struct{
    Uint16 ByteFromEeprom:8;      //EEPROM中读取或写入的数据
    Uint16 EepromAddr:7;        //EEPROM地址
    Uint16 EepromBusy:1;      //数据读写控制或EEPROM状态位
                                //当为读写控制时，置1启动对绝对式编码器内部EEPROM数据的访问，置0无影响
                                //当为EEPROM状态位时，读取为0时表示空闲状态，为1时表示忙碌状态
}STR_HDHMOTROMSTATE_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_HDHMOTROMSTATE_BIT     bit;
}UNI_HDHMOTROMSTATE_REG;


//绝对式电机对EEPROM访问控制或状态读取2  0x3C
typedef struct{
    Uint16 AbsCpgDelay:8;       //线传播延迟测量值
    Uint16 ByteFromEeprom2:8;   //EEPROM读出数据 
}STR_HDHMOTROMSTATE2_BIT;

typedef union
{
    volatile Uint16                  all;
    volatile STR_HDHMOTROMSTATE2_BIT     bit;
}UNI_HDHMOTROMSTATE2_REG;  

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void HDHAbsRom_InitDeal(void);    
extern void HDHAbsRomProcess(void);       
extern void HDHAbsEncAngleInit(void);     
extern void HDHAbsRom_EncState(void);
extern Uint8 HDHAbsReadErrReg(void);
extern Uint8 HDHAbsReset(void);
extern Uint8 HDHAbsResetAndMultiTurn(void);
extern void CalcHDHEncTransTime(void);
extern Uint16 HDHAbsEnc_SaveThetaOffset(void);

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_HDH_ABSPOMOPER_H */

/********************************* END OF FILE *********************************/
