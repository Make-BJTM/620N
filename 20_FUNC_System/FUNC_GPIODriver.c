/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_GPIODriver.c  
 创建人：   王治国                 创建日期：2012.01.10                     

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
#include "FUNC_GlobalVariable.h"
#include "FUNC_GPIODriver.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
STR_FUNC_GPIO_Def * FUNC_GPIOA = (STR_FUNC_GPIO_Def *)(0x40020000);
STR_FUNC_GPIO_Def * FUNC_GPIOB = (STR_FUNC_GPIO_Def *)(0x40020400);
STR_FUNC_GPIO_Def * FUNC_GPIOC = (STR_FUNC_GPIO_Def *)(0x40020800);
STR_FUNC_GPIO_Def * FUNC_GPIOD = (STR_FUNC_GPIO_Def *)(0x40020C00);
STR_FUNC_GPIO_Def * FUNC_GPIOE = (STR_FUNC_GPIO_Def *)(0x40021000);
STR_FUNC_GPIO_Def * FUNC_GPIOF = (STR_FUNC_GPIO_Def *)(0x40021400);
STR_FUNC_GPIO_Def * FUNC_GPIOG = (STR_FUNC_GPIO_Def *)(0x40021800);
STR_FUNC_GPIO_Def * FUNC_GPIOH = (STR_FUNC_GPIO_Def *)(0x40021C00);
STR_FUNC_GPIO_Def * FUNC_GPIOI = (STR_FUNC_GPIO_Def *)(0x40022000);

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitFUNC_GPIO(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void InitFUNC_GPIO(void)
{
    //下面两个直连IO，IO8和IO9 用于Di8和Di9功能
    FUNC_GPIOD->PUPDR.bit.PUPDR2     = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOD->MODER.bit.MODER2     = FUNC_GPIO_Mode_IN;	  //输入模式  PD2 IO8 DI9

    FUNC_GPIOD->PUPDR.bit.PUPDR3     = FUNC_GPIO_PuPd_NOPULL; 
    FUNC_GPIOD->MODER.bit.MODER3     = FUNC_GPIO_Mode_IN;	  //输入模式  PD3 IO9 DI8

    //配置SPI  R/L 引脚
    FUNC_GPIOB->OSPEEDR.bit.OSPEEDR12 = FUNC_GPIO_Speed_25MHz;
    FUNC_GPIOB->OTYPER.bit.OT12       = FUNC_GPIO_OType_PP;
    GPIO_ResetSPICSRL();
    FUNC_GPIOB->MODER.bit.MODER12     = FUNC_GPIO_Mode_OUT;

    //PC8   ->  FPGA管脚 P16   电路图网络INT2
    FUNC_GPIOC->PUPDR.bit.PUPDR8 = FUNC_GPIO_PuPd_DOWN;        //下拉
    FUNC_GPIOC->MODER.bit.MODER8 = FUNC_GPIO_Mode_IN;          //输出模式，

    //PC13  ->  FPGA管脚 R14   电路图网络IO7
    FUNC_GPIOC->PUPDR.bit.PUPDR13 = FUNC_GPIO_PuPd_DOWN;       //下拉
    FUNC_GPIOC->MODER.bit.MODER13 = FUNC_GPIO_Mode_OUT;        //输出模式，

    //PB11  ->  FPGA管脚 A15   电路图网络IO3
    FUNC_GPIOB->PUPDR.bit.PUPDR11 = FUNC_GPIO_PuPd_DOWN;       //下拉
    FUNC_GPIOB->MODER.bit.MODER11 = FUNC_GPIO_Mode_OUT;        //输出模式，

    //PB10  ->  FPGA管脚 B14   电路图网络IO6
    FUNC_GPIOB->PUPDR.bit.PUPDR14 = FUNC_GPIO_PuPd_DOWN;       //下拉
    FUNC_GPIOB->MODER.bit.MODER14 = FUNC_GPIO_Mode_OUT;        //输出模式，

    //PE1 风扇Fan控制端口
    FUNC_GPIOE->OSPEEDR.bit.OSPEEDR1= FUNC_GPIO_Speed_2MHz;   //最大速度2MHz
    FUNC_GPIOE->OTYPER.bit.OT1 = FUNC_GPIO_OType_PP;   //普通推挽输出
    GPIO_Disable_FAN();                               //置1  Bit Set/Reset  关闭风扇
    FUNC_GPIOE->MODER.bit.MODER1 = FUNC_GPIO_Mode_OUT;   //PE1为输出模式，

    //PC12 软启动Relay_DSP继电器
    FUNC_GPIOC->OSPEEDR.bit.OSPEEDR12= FUNC_GPIO_Speed_2MHz;   //最大速度2MHz
    FUNC_GPIOC->OTYPER.bit.OT12 = FUNC_GPIO_OType_PP;   //普通推挽输出
    GPIO_Enable_SoftStart();                           //置1  Bit Set/Reset 继电器打开接入限流电阻，软启动使能
    FUNC_GPIOC->MODER.bit.MODER12 = FUNC_GPIO_Mode_OUT;   //PC12为输出模式

	#if DRIVER_TYPE == SERVO_650N
    //PTC输入 温度报警
    FUNC_GPIOA->PUPDR.bit.PUPDR4     = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOA->MODER.bit.MODER4     = FUNC_GPIO_Mode_IN;	  //输入模式  PD2 IO8 DI9	
	#endif
}  


/********************************* END OF FILE *********************************/
