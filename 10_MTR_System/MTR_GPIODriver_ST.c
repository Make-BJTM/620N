/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_GPIODriver_ST.c  
 创建人：   王治国、童文邹        创建日期：XXXX.XX.XX                     
 修改人：   朱祥华                修改日期：2012.02.10
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
//wzg #include "MTR_GlobalVariable.h"
#include "MTR_GPIODriver_ST.h"
//#include "stm32f2xx_fsmc.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
//定义GPIOx首地址，请查询13902-RM0008 v11（中文手册）.pdf 中2.3节存储器映像 第28页
STR_MTR_GPIO_Def * MTR_GPIOA = (STR_MTR_GPIO_Def *)(0x40020000); 
STR_MTR_GPIO_Def * MTR_GPIOB = (STR_MTR_GPIO_Def *)(0x40020400);
STR_MTR_GPIO_Def * MTR_GPIOC = (STR_MTR_GPIO_Def *)(0x40020800);
STR_MTR_GPIO_Def * MTR_GPIOD = (STR_MTR_GPIO_Def *)(0x40020C00);
STR_MTR_GPIO_Def * MTR_GPIOE = (STR_MTR_GPIO_Def *)(0x40021000);
STR_MTR_GPIO_Def * MTR_GPIOF = (STR_MTR_GPIO_Def *)(0x40021400);
STR_MTR_GPIO_Def * MTR_GPIOG = (STR_MTR_GPIO_Def *)(0x40021800);

STR_MTR_FSMC_BANK1_CS_REG_Def * BANK1_CS = (STR_MTR_FSMC_BANK1_CS_REG_Def *)(0xA0000000);

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitMTR_GPIO(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


/*******************************************************************************
  函数名: void InitMTR_GPIO()   初始化时调用电机模块所用的GPIO配置 
  输入:   无 
  输出:   无 
  子函数: 无
    1. 配置缺相输入信号PE1 GPIO_E1
    2. 配置风扇控制输出信号PC10即GPIOC_10、Brake制动电阻泄放PC11即GPIOC_11、软启动SoftStart继电器PC12即GPIOC12
********************************************************************************/
void InitMTR_GPIO(void)
{
    //SRAM/NOR闪存片选控制时序寄存器1
    BANK1_CS->BCR1.bit.MBKEN = 1;        //Bit0: 存储器块使能位 (Memory bank enable bit)
    BANK1_CS->BCR1.bit.MUXEN = 1;        //Bit1: 地址/数据复用使能位 (Address/data multiplexing enable bit)
    BANK1_CS->BCR1.bit.MTYP = 2;         //Bit2-3: 存储器类型 (Memory type)
    BANK1_CS->BCR1.bit.MWID = 1;         //Bit4-5: 存储器数据总线宽度 (Memory databus width)
    BANK1_CS->BCR1.bit.FACCEN = 1;       //Bit6: 闪存访问使能 (Flash access enable)
    BANK1_CS->BCR1.bit.BURSTEN = 0;      //Bit8: 成组模式使能 (Burst enable bit)
    BANK1_CS->BCR1.bit.WAITPOL = 0;      //Bit9: 等待信号极性 (Wait signal polarity bit)
    BANK1_CS->BCR1.bit.WRAPMOD = 0;      //Bit10: 支持非对齐的成组模式 (Wrapped burst mode support)
    BANK1_CS->BCR1.bit.WAITCFG = 0;      //Bit11: 配置等待时序 (Wait timing configuration)
    BANK1_CS->BCR1.bit.WREN = 1;         //Bit12: 写使能位 (Write enable bit)
    BANK1_CS->BCR1.bit.WAITEN = 0;       //Bit13: 等待使能位 (Wait enable bit)
    BANK1_CS->BCR1.bit.EXTMOD = 0;       //Bit14: 扩展模式使能 (Extended mode enable)
    BANK1_CS->BCR1.bit.ASYNCWAIT = 1;
    BANK1_CS->BCR1.bit.CBURSTRW = 0;     //Bit19: 成组写使能位 (Write burst enable)
    
    //SRAM/NOR闪存片选写时序寄存器1
    BANK1_CS->BTR1.bit.ADDSET = 3;        //Bit0-3: 地址建立时间 (Address setup phase duration)
    BANK1_CS->BTR1.bit.ADDHLD = 2;        //Bit4-7: 地址保持时间 (Address-hold phase duration)
    BANK1_CS->BTR1.bit.DATAST = 10;        //Bit8-15: 数据保持时间 (Data-phase duration)
    BANK1_CS->BTR1.bit.BUSTURN = 0;       //Bit16-19: 总线恢复时间 (Bus turnaround phase duration)
    BANK1_CS->BTR1.bit.CLKDIV = 1;        //Bit20-23: 时钟分频比(CLK信号) (Clock divide ratio (for CLK signal))
    BANK1_CS->BTR1.bit.DATLAT = 0;        //Bit24-27: (同步成组式NOR闪存的)数据保持时间 (Data latency (for synchronous burst NOR Flash))
    BANK1_CS->BTR1.bit.ACCMOD = 3;        //Bit28-29: 访问模式 (Access mode)

    //FSMC FPGA和DSP间的数据总线
    MTR_GPIOD->OSPEEDR.bit.OSPD14= MTR_GPIO_OSPD_50M;  //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT14   = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRH.bit.AFRH14   = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE14  = MTR_GPIO_MODE_AF;    //D0 PD14 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD14  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD15= MTR_GPIO_OSPD_50M;  //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT15   = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRH.bit.AFRH15   = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE15  = MTR_GPIO_MODE_AF;    //D1 PD15 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD15  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD0 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT0    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL0    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE0   = MTR_GPIO_MODE_AF;    //D2 PD0 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD0  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD1 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT1    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL1    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE1   = MTR_GPIO_MODE_AF;    //D3 PD1 为输出模式
    MTR_GPIOD->PUPDR.bit.PUPD1  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD7 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT7    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRL.bit.AFRL7    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE7   = MTR_GPIO_MODE_AF;    //D4 PE7 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD7  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD8 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT8    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH8    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE8   = MTR_GPIO_MODE_AF;    //D5 PE8  为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD8  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD9 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT9    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH9    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE9   = MTR_GPIO_MODE_AF;    //D6 PE9  为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD9  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD10 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT10    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH10    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE10   = MTR_GPIO_MODE_AF;    //D7 PE10 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD10  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD11 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT11    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH11    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE11   = MTR_GPIO_MODE_AF;    //D8 PE11 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD11   = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD12 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT12    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH12    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE12   = MTR_GPIO_MODE_AF;    //D9 PE12 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD12   = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD13 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT13    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH13    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE13   = MTR_GPIO_MODE_AF;    //D10 PE13 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD13  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD14 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT14    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH14    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE14   = MTR_GPIO_MODE_AF;    //D11 PE14 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD14   = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOE->OSPEEDR.bit.OSPD15 = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOE->OTYPER.bit.OT15    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOE->AFRH.bit.AFRH15    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOE->MODER.bit.MODE15   = MTR_GPIO_MODE_AF;    //D12 PE15 为输出模式，
    MTR_GPIOE->PUPDR.bit.PUPD15   = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD8  = MTR_GPIO_OSPD_50M;  //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT8     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRH.bit.AFRH8     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE8    = MTR_GPIO_MODE_AF;    //D13 PD8 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD8    = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD9  = MTR_GPIO_OSPD_50M;  //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT9     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRH.bit.AFRH9     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE9    = MTR_GPIO_MODE_AF;    //D14 PD9 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD9  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD10 = MTR_GPIO_OSPD_50M;  //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT10    = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRH.bit.AFRH10    = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE10   = MTR_GPIO_MODE_AF;    //D15 PD10 为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD10   = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD4  = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT4     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL4     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE4    = MTR_GPIO_MODE_AF;    //RD PD4为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD4  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD5  = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT5     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL5     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE5    = MTR_GPIO_MODE_AF;    //WR PD5为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD5  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOB->OSPEEDR.bit.OSPD7  = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOB->OTYPER.bit.OT7     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOB->AFRL.bit.AFRL7     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOB->MODER.bit.MODE7    = MTR_GPIO_MODE_AF;    //ADV PB7为输出模式，
    MTR_GPIOB->PUPDR.bit.PUPD7  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD6  = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT6     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL6     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE6    = MTR_GPIO_MODE_AF;    //Wait PD6为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD6  = MTR_GPIO_PUPDR_IPU;    //上拉模式

    MTR_GPIOD->OSPEEDR.bit.OSPD7  = MTR_GPIO_OSPD_50M; //最大速度50MHz
    MTR_GPIOD->OTYPER.bit.OT7     = MTR_GPIO_OTYPER_PP;    //普通推挽输出
    MTR_GPIOD->AFRL.bit.AFRL7     = MTR_GPIO_AFR_AF12;     //复用功能12
    MTR_GPIOD->MODER.bit.MODE7    = MTR_GPIO_MODE_AF;    //CS-FPGA PD7为输出模式，
    MTR_GPIOD->PUPDR.bit.PUPD7  = MTR_GPIO_PUPDR_IPU;    //上拉模式
}

/********************************* END OF FILE *********************************/
