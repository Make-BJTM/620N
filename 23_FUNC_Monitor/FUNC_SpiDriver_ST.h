/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名: FUNC_SpiDriver_ST.h	
 创建人：童文邹                
 修订人：李浩                 创建日期：11.11.28 
 描述： 
     1.
	 2.

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

#ifndef FUNC_SPIDRIVER_ST_H
#define FUNC_SPIDRIVER_ST_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

#define SPI_RX_STEP   1
#define SPI_TX_STEP   2

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	
#define SPI2_SSOutputEnable()       (FUNC_SPI2->CR2.bit.SSOE = 1)
#define SPI2_SSOutputDisable()      (FUNC_SPI2->CR2.bit.SSOE = 0)

#define SPI2_I2S_DMA_RX_Enable()    (FUNC_SPI2->CR2.bit.RXDMAEN = 1)
#define SPI2_I2S_DMA_RX_Disable()   (FUNC_SPI2->CR2.bit.RXDMAEN = 0)

#define SPI2_I2S_DMA_Tx_Enable()    (FUNC_SPI2->CR2.bit.TXDMAEN = 1)
#define SPI2_I2S_DMA_Tx_Disable()   (FUNC_SPI2->CR2.bit.TXDMAEN = 0)

#define SPI2_Enable()               (FUNC_SPI2->CR1.bit.SPE = 1)
#define SPI2_Disable()              (FUNC_SPI2->CR1.bit.SPE = 0)
/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
//定义SPI的CR1控制寄存器的位域结构体类型
typedef struct{
    Uint32  CPHA:1;         //bit0 时钟相位
    Uint32  CPOL:1;         //bit1 时钟极性
    Uint32  MSTR:1;         //bit2 主设备选择
    Uint32  BR:3;           //bit3-5 波特率控制
    Uint32  SPE:1;          //bit6 SPI使能
    Uint32  LSBFIRST:1;     //bit7 帧格式
    Uint32  SSI:1;          //bit8 内部从设备选择
    Uint32  SSM:1;          //bit9 软件从设备管理
    Uint32  RXONLY:1;       //bit10 只接受
    Uint32  DFF:1;          //bit11 数据帧格式
    Uint32  CRCNEXT:1;      //bit12 下一个发送CRC
    Uint32  CRCEN:1;        //bit13 硬件CRC校验使能
    Uint32  BIDIOE:1;       //bit14 双向模式下的输出使能
    Uint32  BIDIMODE:1;     //bit15 双向数据模式使能
    Uint32  Rsvd:16;        //bit16-31 保留位
}STR_SPIx_CR1_REG; 
typedef union{
    volatile Uint32                 all;
    volatile STR_SPIx_CR1_REG       bit; 
}UNI_SPIx_CR1_REG;

//定义SPI的CR2控制寄存器的位域结构体类型
typedef struct{
    Uint32  RXDMAEN:1;      //bit0 接收缓冲区DMA使能
    Uint32  TXDMAEN:1;      //bit1 发送缓冲区DMA使能
    Uint32  SSOE:1;         //bit2 SS输出使能
    Uint32  Rsvd0:1;        //bit3 保留
	Uint32  FRF:1  ;        //bit4 帧模式
    Uint32  ERRIE:1;        //bit5 错误中断使能
    Uint32  RXNEIE:1;       //bit6 接收缓冲区非空中断使能
    Uint32  TXEIE:1;        //bit7 发送缓冲区空中断使能
    Uint32  Rsvd:24;        //bit8-31 保留
}STR_SPIx_CR2_REG; 
typedef union{
    volatile Uint32                 all;
    volatile STR_SPIx_CR2_REG       bit; 
}UNI_SPIx_CR2_REG;

//定义SPI的SR状态寄存器的位域结构体类型
typedef struct{
    Uint32  RXNE:1;         //bit0 接收缓冲非空
    Uint32  TXE:1;          //bit1 发送缓冲为空
    Uint32  CHSIDE:1;       //bit2 声道
    Uint32  UDR:1;          //bit3 下溢标志位
    Uint32  CRCERR:1;       //bit4 CRC错误标志
    Uint32  MODF:1;         //bit5 模式错误
    Uint32  OVR:1;          //bit6 溢出标志
    Uint32  BSY:1;          //bit7 忙标志
	Uint32  TIFRFE:1;       //bit8 TI帧格式错误
    Uint32  Rsvd:23;        //bit8-31 保留
}STR_SPIx_SR_REG; 
typedef union{
    volatile Uint32                 all;
    volatile STR_SPIx_SR_REG       bit; 
}UNI_SPIx_SR_REG;

//定义SPI的DR数据寄存器的位域结构体类型
typedef struct{
    Uint32  DR:16;          //bit0-15 数据寄存器 
    Uint32  Rsvd:16;        //bit16-31 保留
}STR_SPIx_DR_REG; 
typedef union{
    volatile Uint32                 all;
    volatile STR_SPIx_DR_REG       bit; 
}UNI_SPIx_DR_REG;

//定义SPI的CRC多项式寄存器的位域结构体类型
typedef struct{
    Uint32  CRCPOLY:16;     //bit0-15 CRC多项式寄存器 
    Uint32  Rsvd:16;        //bit16-31 保留
}STR_SPIx_CRCPR_REG; 
typedef union{
    volatile Uint32                 all;
    volatile STR_SPIx_CRCPR_REG     bit; 
}UNI_SPIx_CRCPR_REG;

//定义SPI的RX CRC寄存器的位域结构体类型
typedef struct{
    Uint32  RXCRC:16;       //bit0-15 接收CRC寄存器 
    Uint32  Rsvd:16;        //bit16-31 保留
}STR_SPIx_RXCRCR_REG; 
typedef union{
    volatile Uint32                  all;
    volatile STR_SPIx_RXCRCR_REG     bit; 
}UNI_SPIx_RXCRCR_REG;

//定义SPI的TX CRC寄存器的位域结构体类型
typedef struct{
    Uint32  TXCRC:16;       //bit0-15 接收CRC寄存器 
    Uint32  Rsvd:16;        //bit16-31 保留
}STR_SPIx_TXCRCR_REG; 
typedef union{
    volatile Uint32                  all;
    volatile STR_SPIx_TXCRCR_REG     bit; 
}UNI_SPIx_TXCRCR_REG;

//定义SPI的I2S配置寄存器的位域结构体类型
typedef struct{
    Uint32  CHLEN:1;        //bit0 接收CRC寄存器 
    Uint32  DATLEN:2;       //bit1-2 待传输数据长度
    Uint32  CKPOL:1;        //bit3 静止态时钟极性
    Uint32  I2SSTD:2;       //bit4-5 I2S标准选择
    Uint32  Rsvd0:1;        //bit6 保留
    Uint32  PCMSYNC:1;      //bit7 PCM帧同步
    Uint32  I2SCFG:2;       //bit8-9 I2S 模式设置
    Uint32  I2SE:1;         //bit10  I2S 使能
    Uint32  I2SMOD:1;       //bit11 I2S模式选择
    Uint32  Rsvd1:20;       //bit12-31 保留
}STR_SPIx_I2SCFGR_REG; 
typedef union{
    volatile Uint32                   all;
    volatile STR_SPIx_I2SCFGR_REG     bit; 
}UNI_SPIx_I2SCFGR_REG;

//定义SPI的I2S预分频寄存器的位域结构体类型
typedef struct{
    Uint32  I2SDIV:8;       //bit0-7 I2S线性预分频 
    Uint32  ODD:1;          //bit8 奇系数预分频
    Uint32  MCKOE:1;        //bit9 主设备时钟输出使能
    Uint32  Rsvd:23;        //bit10-31 保留
}STR_SPIx_I2SPR_REG; 
typedef union{
    volatile Uint32                  all;
    volatile STR_SPIx_I2SPR_REG     bit; 
}UNI_SPIx_I2SPR_REG;


typedef struct
{
    volatile UNI_SPIx_CR1_REG         CR1;
    volatile UNI_SPIx_CR2_REG         CR2;
    volatile UNI_SPIx_SR_REG          SR;
    volatile UNI_SPIx_DR_REG          DR;
    volatile UNI_SPIx_CRCPR_REG       CRCPR;
    volatile UNI_SPIx_RXCRCR_REG      RXCRCR;
    volatile UNI_SPIx_TXCRCR_REG      TXCRCR;
    volatile UNI_SPIx_I2SCFGR_REG     I2SCFGR;
    volatile UNI_SPIx_I2SPR_REG       I2SPR;
}STR_FUNC_SPI_Def;


/* SPI_DI 结构体定义 */
typedef struct{
    Uint16 SpiDi1:1;    //9~15；数字输入,可用表示实际意义的名称代替；高电平有效（1）。待规划
    Uint16 SpiDi2:1;    
    Uint16 SpiDi3:1; 
    Uint16 SpiDi4:1;
    Uint16 SpiDi5:1;
    Uint16 SpiDi6:1;
    Uint16 SpiDi7:1;

    Uint16 Rsvd:9;
}STR_SPI_DI;

/* SPI_DI 共用体定义 */
typedef union{
    Uint16        all;
    STR_SPI_DI    bit; 
}UNI_SPI_DI;

/* SPI_DO 结构体定义 */
typedef struct{           
    Uint16 Rsvd:8;

    Uint16 SpiDo1:1;    //8~15；数字输出,可用表示实际意义的名称代替；高电平有效（1）。待规划
    Uint16 SpiDo2:1;    
    Uint16 SpiDo3:1; 
    Uint16 SpiDo4:1;
    Uint16 SpiDo5:1;

    Uint16 Rsvd1:3;
}STR_SPI_DO;

/* SPI_DO 共用体定义 */
typedef union{
    Uint16        all;
    STR_SPI_DO    bit; 
}UNI_SPI_DO;



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern UNI_SPI_DI   UNI_SpiDiReg;
extern UNI_SPI_DO   UNI_SpiDoReg;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitSpi(void); 
extern void SpiRxTxScan(Uint16 Step);

#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* __FUNC_SPIDRIVER_ST_H */


/********************************* END OF FILE *********************************/


