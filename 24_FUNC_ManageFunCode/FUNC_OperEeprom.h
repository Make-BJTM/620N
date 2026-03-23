/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_OperEeprom.h
 创建人：童文邹                
 修订人：李浩                 创建日期：11.11.01 
 描述： 
     1.
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_OPEREEPROM_H
#define FUNC_OPEREEPROM_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h"
/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/

/*一页Eeprom可以存储的功能码数，功能码为16位，而型号为24LC08的Eeprom存储单位为8位 */
#define EEPROM_PAGE_BYTE        32  // EEPROM(24LC08)的1页的byte(8bit)
#define FUNCODE_BYTE            2   // 一个功能码的byte
#define EEPROM_PAGE_FUNCODE_NUM    (EEPROM_PAGE_BYTE / FUNCODE_BYTE) // EEPROM(24LC08)的1页可以写入的功能码个数




/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

/*读写Eeprom中的功能码所用到的结构体*/ 
typedef struct{

    Uint16 RwIndex;   // 要读写功能码的起始index, EEPROM的index
    Uint16 RwNumber;  // 要读写功能码的个数
    
    Uint16 RwData[EEPROM_PAGE_FUNCODE_NUM];  //读取功能码的存放数组
}STR_EEPROM_FUNCODE_RW;

/*Eeprom全部操作模式(对内)*/
typedef enum{ 

    ALL_NO_OPERATE = 0,
    ALL_WRITE_SERI = 1,
    ALL_WRITE_ALL  = 2,
    ALL_READ_SERI  = 3,
    ALL_READ_ALL   = 4,

    ALL_CUT_POWER_WRITE = 5,

    ALL_WRITE_ONE  = 6,
    ALL_ERR_STATE  = 7
    

}ENU_ALL_OPERATE_MODE;

/*Eeprom连续读取操作模式(对外)*/
typedef enum{ 

    SERI_NO_OPERATE = 0,
    SERI_WRITE_SERI = 1,
    SERI_WRITE_ALL  = 2,
    SERI_READ_SERI  = 3,
    SERI_READ_ALL   = 4,
    SERI_CUT_POWER_WRITE = 5


}ENU_SERI_OPERATE_MODE;
     
/* 读取Eeprom中功能码过程的几个工作状态*/
typedef enum{

    FUNCODE_RW_STATE_READ_DATA = 0,                   // 读取功能码在EEPROM的当前数据
    FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE = 1,      // 准备需要写入的数据
    FUNCODE_RW_STATE_WRITE_DATA = 2                     // 写入数据
                        
}ENU_FUNCODE_EEPROM_RW_FLAG;

/* 写入Eeprom中功能码过程的两种工作状态*/
typedef enum{

    FUNCODE_STATE_WRITE = 0,
    FUNCODE_STATE_READ_FOR_VERIFY_WRITE = 1

}ENU_FUNCODE_WRITE_STATE;

/*Eeprom读写故障类型，包括单个读写超时故障以及读写个数超限故障*/
typedef enum{

    EEPROM_NONE_ERROR = 0,              // Eeprom读写无故障
    EEPROM_WRITE_ERROR = 1,             // 写单个功能码超时错误
    EEPROM_READ_ERROR = 2,              // 读单个功能码超时错误
    EEPROM_FUNCODE_NUM_OVER_ERROR = 3   // 要保存的功能码个数越界错误, 数组funcCodeOneWriteIndex个数有限

}ENU_EEPROM_RW_ERROR;

/*Eeprom每次读写数据是否超时处理结构体，用于EepromRwWatchdog(), 在每一个数据读写过程中，当读写时间过长时则进行报错*/
typedef struct{
 
    Uint16 ReadFlag;    //读标志,调用ReadFunCode()函数，则readFlag标志为1，读完成，readFlag标志为0 
    Uint16 ReadTicker;  //记录每个数据读取过程时间，计数值

    Uint16 WriteFlag;   //写标志,调用WriteFunCode()函数，则WriteFlag标志为1,写完成，WriteFlag标志为0 
    Uint16 WriteTicker; //记录每个数据写入过程时间，计数值

}STR_EEPROM_RW_TIME;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitI2c(void);
extern void SaveToEepromOne(Uint16 Index);
extern void SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex);
extern void SaveToEepromAll(void);
extern void ReadFromEeprom(Uint16 Start,Uint16 End);
extern void ReadFromEepromAll(void);
extern Uint16 EepromProcess(void);


#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_OperEeprom.h */

/********************************* END OF FILE *********************************/

