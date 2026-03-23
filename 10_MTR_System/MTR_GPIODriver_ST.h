/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_GPIODriver_ST.h  
 创建人：   王治国、童文邹        创建日期：XXXX.XX.XX                     
 修改人：   朱祥华                修改日期：2012.02.10
 描述： 
    1. 初始化时调用电机模块所用的GPIO配置
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef MTR_GPIODRIVER_ST_H
#define MTR_GPIODRIVER_ST_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

//端口输入上下拉配置
#define    MTR_GPIO_PUPDR_FLOATING        0x00             //浮空输入模式
#define    MTR_GPIO_PUPDR_IPU             0x01             //上拉模式  BRy = 1
#define    MTR_GPIO_PUPDR_IPD             0x02             //下拉模式  BSy = 1


//端口输出速度
#define    MTR_GPIO_OSPD_2M               0X00             //2Mhz输出速度
#define    MTR_GPIO_OSPD_25M  			  0X01             //25Mhz输出速度
#define    MTR_GPIO_OSPD_50M              0X02             //50Mhz输出速度
#define    MTR_GPIO_OSPD_100M             0X03             //100Mhz输出速度

//端口输出类型
#define    MTR_GPIO_OTYPER_PP             0X00             //推挽式输出
#define    MTR_GPIO_OTYPER_OD             0X01             //开漏式输出

// 端口模式位
#define    MTR_GPIO_MODE_IN               0x00             //输入模式
#define    MTR_GPIO_MODE_Out	          0x01             //输出模式
#define    MTR_GPIO_MODE_AF               0x02             //复用模式
#define    MTR_GPIO_MODE_AIN              0x03             //模拟输入模式

//端口复用功能位
#define    MTR_GPIO_AFR_AF0                0               //复用功能0
#define    MTR_GPIO_AFR_AF1                1               //复用功能1
#define    MTR_GPIO_AFR_AF2                2               //复用功能2
#define    MTR_GPIO_AFR_AF3                3               //复用功能3
#define    MTR_GPIO_AFR_AF4                4               //复用功能4
#define    MTR_GPIO_AFR_AF5                5               //复用功能5
#define    MTR_GPIO_AFR_AF6                6               //复用功能6
#define    MTR_GPIO_AFR_AF7                7               //复用功能7
#define    MTR_GPIO_AFR_AF8                8               //复用功能8
#define    MTR_GPIO_AFR_AF9                9               //复用功能9
#define    MTR_GPIO_AFR_AF10              10               //复用功能10
#define    MTR_GPIO_AFR_AF11              11               //复用功能11
#define    MTR_GPIO_AFR_AF12              12               //复用功能12
#define    MTR_GPIO_AFR_AF13              13               //复用功能13
#define    MTR_GPIO_AFR_AF14              14               //复用功能14
#define    MTR_GPIO_AFR_AF15              15               //复用功能15



/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */ 

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//根据GPIO寄存器地址和偏移地址仿照TI例程重新定义GPIOx各配置寄存器，
//请查询STM32F205 Reference Manual-RM0008 v11（中文手册）.pdf 中8.2节存储器映像 第754页
//①GPIOx_MODER端口配置结构体类型定义
//00:输入(默认)
//01:输出
//10:复用模式
//11:模拟量模式
//端口模式配置寄存器(GPIOx_MODER) (x=A..E) 偏移地址：0x00
typedef struct{
    Uint32  MODE0:2;        //端口模式位
    Uint32  MODE1:2;        //端口模式位
    Uint32  MODE2:2;        //端口模式位
    Uint32  MODE3:2;        //端口模式位
    Uint32  MODE4:2;        //端口模式位
    Uint32  MODE5:2;        //端口模式位
    Uint32  MODE6:2;        //端口模式位
    Uint32  MODE7:2;        //端口模式位
    Uint32  MODE8:2;        //端口模式位
    Uint32  MODE9:2;        //端口模式位
    Uint32  MODE10:2;       //端口模式位
    Uint32  MODE11:2;       //端口模式位
    Uint32  MODE12:2;       //端口模式位
    Uint32  MODE13:2;       //端口模式位
    Uint32  MODE14:2;       //端口模式位
    Uint32  MODE15:2;       //端口模式位
    
}STR_GPIOx_MOD_BIT; 

typedef union{
    volatile Uint32             all;
    volatile STR_GPIOx_MOD_BIT  bit; 
}UNI_GPIOx_MOD_REG; 

//②GPIOx_OTYPER端口配置结构体类型定义
//0:推挽式(默认)
//1:开漏式
//端口输出类型配置寄存器(GPIOx_OTYPER) (x=A..L)   偏移地址：0x04
typedef struct{
    Uint32  OT0:1;          //端口输出类型位
    Uint32  OT1:1;          //端口输出类型位
    Uint32  OT2:1;          //端口输出类型位
    Uint32  OT3:1;          //端口输出类型位
    Uint32  OT4:1;          //端口输出类型位
    Uint32  OT5:1;          //端口输出类型位
    Uint32  OT6:1;          //端口输出类型位
    Uint32  OT7:1;          //端口输出类型位
    Uint32  OT8:1;          //端口输出类型位
    Uint32  OT9:1;          //端口输出类型位
    Uint32  OT10:1;         //端口输出类型位
    Uint32  OT11:1;         //端口输出类型位
    Uint32  OT12:1;         //端口输出类型位
    Uint32  OT13:1;         //端口输出类型位
    Uint32  OT14:1;         //端口输出类型位
    Uint32  OT15:1;         //端口输出类型位
    Uint32  rsvd:16;
}STR_GPIOx_OT_BIT;

typedef union{
    volatile Uint32            all;
    volatile STR_GPIOx_OT_BIT  bit; 
}UNI_GPIOx_OT_REG;

//②GPIOx_OSPEEDER端口输出速度配置结构体类型定义
//00:2M
//01:25M
//10:50M
//11:100M
//端口输出速度配置寄存器(GPIOx_OSPEEDER) (x=A..L)   偏移地址：0x08
typedef struct{
    Uint32  OSPD0:2;          //端口输出速度位
    Uint32  OSPD1:2;          //端口输出速度位
    Uint32  OSPD2:2;          //端口输出速度位

    Uint32  OSPD3:2;          //端口输出速度位
    Uint32  OSPD4:2;          //端口输出速度位
    Uint32  OSPD5:2;          //端口输出速度位
    Uint32  OSPD6:2;          //端口输出速度位
    Uint32  OSPD7:2;          //端口输出速度位
    Uint32  OSPD8:2;          //端口输出速度位
    Uint32  OSPD9:2;          //端口输出速度位
    Uint32  OSPD10:2;         //端口输出速度位
    Uint32  OSPD11:2;         //端口输出速度位
    Uint32  OSPD12:2;         //端口输出速度位
    Uint32  OSPD13:2;         //端口输出速度位
    Uint32  OSPD14:2;         //端口输出速度位
    Uint32  OSPD15:2;         //端口输出速度位
}STR_GPIOx_OSPD_BIT;

typedef union{
    volatile Uint32              all;
    volatile STR_GPIOx_OSPD_BIT  bit; 
}UNI_GPIOx_OSPD_REG;

//②GPIOx_PUPD端口配置结构体类型定?
//00:悬空
//01:上拉
//10:下拉
//11:保留?
//复位值:0x64000000(PORTA) ,0X00000100(PORTB),0X00000000(OTHER)
//端口上下拉配置寄存器(GPIOx_PUPD) (x=A..L)   偏移地址：0x0C
typedef struct{
    Uint32  PUPD0:2;          //端口上下拉配置位
    Uint32  PUPD1:2;          //端口上下拉配置位
    Uint32  PUPD2:2;          //端口上下拉配置位
    Uint32  PUPD3:2;          //端口上下拉配置位
    Uint32  PUPD4:2;          //端口上下拉配置位
    Uint32  PUPD5:2;          //端口上下拉配置位
    Uint32  PUPD6:2;          //端口上下拉配置位
    Uint32  PUPD7:2;          //端口上下拉配置位
    Uint32  PUPD8:2;          //端口上下拉配置位
    Uint32  PUPD9:2;          //端口上下拉配置位
    Uint32  PUPD10:2;         //端口上下拉配置位
    Uint32  PUPD11:2;         //端口上下拉配置位
    Uint32  PUPD12:2;         //端口上下拉配置位
    Uint32  PUPD13:2;         //端口上下拉配置位
    Uint32  PUPD14:2;         //端口上下拉配置位
    Uint32  PUPD15:2;         //端口上下拉配置位
}STR_GPIOx_PUPD_BIT;

typedef union{
    volatile Uint32               all;
    volatile STR_GPIOx_PUPD_BIT   bit; 
}UNI_GPIOx_PUPD_REG;
//③GPIOx_IDR端口输入数据结构体类型定义
//端口输入数据寄存器(GPIOx_IDR) (x=A..l)   地址偏移：0x10
typedef struct{
    Uint32  IDR0:1;
    Uint32  IDR1:1;
    Uint32  IDR2:1;
    Uint32  IDR3:1;
    Uint32  IDR4:1;
    Uint32  IDR5:1;
    Uint32  IDR6:1;
    Uint32  IDR7:1;
    Uint32  IDR8:1;
    Uint32  IDR9:1;
    Uint32  IDR10:1;
    Uint32  IDR11:1;
    Uint32  IDR12:1;
    Uint32  IDR13:1;
    Uint32  IDR14:1;
    Uint32  IDR15:1;
    Uint32  Rsvd:16;
}STR_GPIOx_IDR_BIT;
typedef union{
    volatile Uint32             all;
    volatile STR_GPIOx_IDR_BIT  bit;
}UNI_GPIOx_IDR_REG;

//④GPIOx_ODR端口输出数据结构体类型定义
//端口输出数据寄存器(GPIOx_ODR) (x=A..E)   地址偏移：14h
typedef struct{
    Uint32  ODR0:1;
    Uint32  ODR1:1;
    Uint32  ODR2:1;
    Uint32  ODR3:1;
    Uint32  ODR4:1;
    Uint32  ODR5:1;
    Uint32  ODR6:1;
    Uint32  ODR7:1;
    Uint32  ODR8:1;
    Uint32  ODR9:1;
    Uint32  ODR10:1;
    Uint32  ODR11:1;
    Uint32  ODR12:1;
    Uint32  ODR13:1;
    Uint32  ODR14:1;
    Uint32  ODR15:1;
    Uint32  Rsvd:16;
}STR_GPIOx_ODR_BIT;
typedef union{
    volatile Uint32              all;
    volatile STR_GPIOx_ODR_BIT   bit;
}UNI_GPIOx_ODR_REG;

//⑤GPIOx_BSRR端口输出数据结构体类型定义  
//BSX: 0无效，1置位
//BRX:0无效，1复位
//端口位设置/清除寄存器(GPIOx_BSRR) (x=A..E)   地址偏移：0x18
typedef struct{
    Uint32  BS0:1;
    Uint32  BS1:1;
    Uint32  BS2:1;
    Uint32  BS3:1;
    Uint32  BS4:1;
    Uint32  BS5:1;
    Uint32  BS6:1;
    Uint32  BS7:1;
    Uint32  BS8:1;
    Uint32  BS9:1;
    Uint32  BS10:1;
    Uint32  BS11:1;
    Uint32  BS12:1;
    Uint32  BS13:1;
    Uint32  BS14:1;
    Uint32  BS15:1;
    Uint32  BR0:1;
    Uint32  BR1:1;
    Uint32  BR2:1;
    Uint32  BR3:1;
    Uint32  BR4:1;
    Uint32  BR5:1;
    Uint32  BR6:1;
    Uint32  BR7:1;
    Uint32  BR8:1;
    Uint32  BR9:1;
    Uint32  BR10:1;
    Uint32  BR11:1;
    Uint32  BR12:1;
    Uint32  BR13:1;
    Uint32  BR14:1;
    Uint32  BR15:1;
}STR_GPIOx_BSRR_BIT;
typedef union{
    volatile Uint32               all;
    volatile STR_GPIOx_BSRR_BIT   bit;
}UNI_GPIOx_BSRR_REG;

//⑥GPIOx_AFRL(x=A..E)端口复用功能低位结构体类型定义  
//bit0-bit4: AF0-AF15
//端口复用寄存器(GPIOx_AFRL)    地址偏移：0x20
typedef struct{
    Uint32  AFRL0:4; 	//复用功能选择
    Uint32  AFRL1:4;    //复用功能选择
    Uint32  AFRL2:4;    //复用功能选择
    Uint32  AFRL3:4;    //复用功能选择
    Uint32  AFRL4:4;    //复用功能选择
    Uint32  AFRL5:4; 	//复用功能选择
    Uint32  AFRL6:4; 	//复用功能选择
    Uint32  AFRL7:4; 	//复用功能选择
}STR_GPIOx_AFRL_BIT; 
typedef union{
    volatile Uint32                 all;
    volatile STR_GPIOx_AFRL_BIT     bit;
}UNI_GPIOx_AFRL_REG;

//⑥GPIOx_AFRH(x=A..E)端口复用功能高位结构体类型定义  
//bit0-bit4: AF0-AF15
//端口复用寄存器(GPIOx_AFRH)    地址偏移：0x24
typedef struct{
    Uint32  AFRH8:4; 	 //复用功能选择
    Uint32  AFRH9:4;     //复用功能选择
    Uint32  AFRH10:4;    //复用功能选择
    Uint32  AFRH11:4;    //复用功能选择
    Uint32  AFRH12:4;    //复用功能选择
    Uint32  AFRH13:4; 	 //复用功能选择
    Uint32  AFRH14:4; 	 //复用功能选择
    Uint32  AFRH15:4; 	 //复用功能选择
}STR_GPIOx_AFRH_BIT; 
typedef union{
    volatile Uint32                all;
    volatile STR_GPIOx_AFRH_BIT    bit;
}UNI_GPIOx_AFRH_REG;
//⑦GPIOx_LCKR(x=A..E)端口输出数据结构体类型定义   偏移地址：0x1c
typedef struct{
    Uint32  LCKR0:1;
    Uint32  LCKR1:1;
    Uint32  LCKR2:1;
    Uint32  LCKR3:1;
    Uint32  LCKR4:1;
    Uint32  LCKR5:1;
    Uint32  LCKR6:1;
    Uint32  LCKR7:1;
    Uint32  LCKR8:1;
    Uint32  LCKR9:1;
    Uint32  LCKR10:1;
    Uint32  LCKR11:1;
    Uint32  LCKR12:1;
    Uint32  LCKR13:1;
    Uint32  LCKR14:1;
    Uint32  LCKR15:1;
    Uint32  Rsvd:16;
}STR_GPIOx_LCKR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_LCKR_BIT      bit;
}UNI_GPIOx_LCKR_REG;

//GPIOx寄存器结构体类型定义
typedef struct
{
    volatile UNI_GPIOx_MOD_REG     MODER;   //端口模式配置寄存器(GPIOx_MODER) (x=A..L) 偏移地址：0x00
    volatile UNI_GPIOx_OT_REG      OTYPER;   //②端口配置高寄存器(GPIOx_OTYPER) (x=A..L)       偏移地址：0x04
	volatile UNI_GPIOx_OSPD_REG    OSPEEDR; //端口输出速度配置寄存器(GPIOx_OSPEEDR) (x=A..L)   偏移地址：0x08
	volatile UNI_GPIOx_PUPD_REG    PUPDR; //端口上下拉配置寄存器(GPIOx_PUPD) (x=A..L)   偏移地址：0x0C
    volatile UNI_GPIOx_IDR_REG     IDR;   //③端口输入数据寄存器(GPIOx_IDR) (x=A..L)     偏移地址：0x10
    volatile UNI_GPIOx_ODR_REG     ODR;   //④端口输出数据寄存器(GPIOx_ODR) (x=A..L)     偏移地址：14h
    volatile UNI_GPIOx_BSRR_REG    BSRR;  //⑤端口位设置/清除寄存器(GPIOx_BSRR) (x=A..L) 偏移地址：0x18
    volatile UNI_GPIOx_LCKR_REG    LCKR;  //⑦端口配置锁定寄存器(GPIOx_LCKR) (x=A..L)    偏移地址：0x1c
	volatile UNI_GPIOx_AFRL_REG    AFRL;  //端口复用功能低位寄存器(GPIOx_AFRL(x=A..L)      偏移地址：0x20
	volatile UNI_GPIOx_AFRH_REG    AFRH;   //⑥端口复用功能高位寄存器(GPIOx_AFRH(x=A..L)      偏移地址：0x24
}STR_MTR_GPIO_Def;


//SRAM/NOR闪存片选控制寄存器结构体类型定义
typedef struct{
    Uint32  MBKEN:1;        //Bit0: 存储器块使能位 (Memory bank enable bit)
    Uint32  MUXEN:1;        //Bit1: 地址/数据复用使能位 (Address/data multiplexing enable bit)
    Uint32  MTYP:2;         //Bit2-3: 存储器类型 (Memory type)
    Uint32  MWID:2;         //Bit4-5: 存储器数据总线宽度 (Memory databus width)
    Uint32  FACCEN:1;       //Bit6: 闪存访问使能 (Flash access enable)
    Uint32  Rsvd7:1;        //Bit7: 保留
    Uint32  BURSTEN:1;      //Bit8: 成组模式使能 (Burst enable bit)
    Uint32  WAITPOL:1;      //Bit9: 等待信号极性 (Wait signal polarity bit)
    Uint32  WRAPMOD:1;      //Bit10: 支持非对齐的成组模式 (Wrapped burst mode support)
    Uint32  WAITCFG:1;      //Bit11: 配置等待时序 (Wait timing configuration)
    Uint32  WREN:1;         //Bit12: 写使能位 (Write enable bit)
    Uint32  WAITEN:1;       //Bit13: 等待使能位 (Wait enable bit)
    Uint32  EXTMOD:1;       //Bit14: 扩展模式使能 (Extended mode enable)
	Uint32  ASYNCWAIT:1;    //Bit15; 异步传输等待信号
	Uint32  Rsvd16_18:3;    //Bit16-18: 保留
    Uint32  CBURSTRW:1;     //Bit19: 成组写使能位 (Write burst enable)
    Uint32  Rsvd20_31:12;   //Bit20-31: 保留
}STR_FSMC_BCRx_BIT;
typedef union{
    volatile Uint32               all;
    volatile STR_FSMC_BCRx_BIT    bit; 
}UNI_FSMC_BCRx_REG;


//SRAM/NOR闪存写时序寄存器结构体类型定义
typedef struct{
    Uint32  ADDSET:4;        //Bit0-3: 地址建立时间 (Address setup phase duration)
    Uint32  ADDHLD:4;        //Bit4-7: 地址保持时间 (Address-hold phase duration)
    Uint32  DATAST:8;        //Bit8-15: 数据保持时间 (Data-phase duration)
    Uint32  BUSTURN:4;       //Bit16-19: 
    Uint32  CLKDIV:4;        //Bit20-23: 时钟分频比(CLK信号) (Clock divide ratio (for CLK signal))
    Uint32  DATLAT:4;        //Bit24-27: (同步成组式NOR闪存的)数据保持时间 (Data latency (for synchronous burst NOR Flash))
    Uint32  ACCMOD:2;        //Bit28-29: 访问模式 (Access mode)
    Uint32  Rsvd30_31:2;     //Bit30-31: 保留
}STR_FSMC_BTRx_BWTRx_BIT;
typedef union{
    volatile Uint32                    all;
    volatile STR_FSMC_BTRx_BWTRx_BIT   bit; 
}UNI_FSMC_BTRx_BWTRx_REG;


//SRAM/NOR闪存片选寄存器结构体类型定义
typedef struct
{
    volatile UNI_FSMC_BCRx_REG           BCR1;   //SRAM/NOR闪存片选控制寄存器1
    volatile UNI_FSMC_BTRx_BWTRx_REG     BTR1;   //SRAM/NOR闪存片选时序寄存器1
    volatile UNI_FSMC_BCRx_REG           BCR2;   //SRAM/NOR闪存片选控制寄存器2
    volatile UNI_FSMC_BTRx_BWTRx_REG     BTR2;   //SRAM/NOR闪存片选时序寄存器2
    volatile UNI_FSMC_BCRx_REG           BCR3;   //SRAM/NOR闪存片选控制寄存器3
    volatile UNI_FSMC_BTRx_BWTRx_REG     BTR3;   //SRAM/NOR闪存片选时序寄存器3
    volatile UNI_FSMC_BCRx_REG           BCR4;   //SRAM/NOR闪存片选控制寄存器4
    volatile UNI_FSMC_BTRx_BWTRx_REG     BTR4;   //SRAM/NOR闪存片选时序寄存器4
}STR_MTR_FSMC_BANK1_CS_REG_Def;


//SRAM/NOR闪存写时序寄存器结构体类型定义
typedef struct
{
    volatile UNI_FSMC_BTRx_BWTRx_REG     BWTR1;   //SRAM/NOR闪存片选时序寄存器1
    volatile UNI_FSMC_BTRx_BWTRx_REG     BWTR2;   //SRAM/NOR闪存片选时序寄存器2
    volatile UNI_FSMC_BTRx_BWTRx_REG     BWTR3;   //SRAM/NOR闪存片选时序寄存器3
    volatile UNI_FSMC_BTRx_BWTRx_REG     BWTR4;   //SRAM/NOR闪存片选时序寄存器4
}STR_MTR_FSMC_BANK1_WR_REG_Def;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//定义GPIOx首地址，请查询STM32 Reference Manual-RM0008 v11（中文手册）.pdf 中2.3节存储器映像 第28页
extern  STR_MTR_GPIO_Def     *MTR_GPIOA;   //GPIOA首地址为(0x40010800)
extern  STR_MTR_GPIO_Def     *MTR_GPIOB;   //GPIOB首地址为(0x40010C00)
extern  STR_MTR_GPIO_Def     *MTR_GPIOC;   //GPIOC首地址为(0x40011000)
extern  STR_MTR_GPIO_Def     *MTR_GPIOD;   //GPIOD首地址为(0x40011400)
extern  STR_MTR_GPIO_Def     *MTR_GPIOE;   //GPIOE首地址为(0x40011800)
extern  STR_MTR_GPIO_Def     *MTR_GPIOF;   //GPIOF首地址为(0x40011C00)
extern  STR_MTR_GPIO_Def     *MTR_GPIOG;   //GPIOG首地址为(0x40012000)




/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitMTR_GPIO(void);

#ifdef __cplusplus
}
#endif

#endif /* MTR_GPIODRIVER_ST_H */

/********************************* END OF FILE *********************************/
