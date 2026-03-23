/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_GPIODriver.h  
 创建人：   王治国                 创建日期：2012.01.10                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_GPIODRIVER_H
#define __FUNC_GPIODRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

// 端口配置位
#define  FUNC_GPIO_Mode_IN      0x00
#define  FUNC_GPIO_Mode_OUT     0x01
#define  FUNC_GPIO_Mode_AF      0x02
#define  FUNC_GPIO_Mode_AN      0x03  

#define  FUNC_GPIO_OType_PP     0x00
#define  FUNC_GPIO_OType_OD     0x01

#define  FUNC_GPIO_Speed_2MHz   0x00
#define  FUNC_GPIO_Speed_25MHz  0x01
#define  FUNC_GPIO_Speed_50MHz  0x02
#define  FUNC_GPIO_Speed_100MHz 0x03 

#define  FUNC_GPIO_PuPd_NOPULL  0x00
#define  FUNC_GPIO_PuPd_UP      0x01
#define  FUNC_GPIO_PuPd_DOWN    0x02


#if DRIVER_TYPE == SERVO_620N
//PD2   DI9
#define GPIO_ReadDI9DSP()     (((FUNC_GPIOD->IDR.bit.IDR2)==1)? 1 : 0)
//PD3   DI8
#define GPIO_ReadDI8DSP()     (((FUNC_GPIOD->IDR.bit.IDR3)==1)? 1 : 0)
#else
//PD2   DI9
#define GPIO_ReadDI9DSP()     (((FUNC_GPIOD->IDR.bit.IDR2)==1)? 0 : 1)
//PD3   DI8
#define GPIO_ReadDI8DSP()     (((FUNC_GPIOD->IDR.bit.IDR3)==1)? 0 : 1)
#endif


//PB12  SPI_L/R
#define GPIO_SetSPICSRL()       (FUNC_GPIOB->BSRR.bit.BS12 = 1)	//置1
#define GPIO_ResetSPICSRL()     (FUNC_GPIOB->BSRR.bit.BR12 = 1)	//清零

//PE1 GPIO_E1 for Fan风扇   输出量 
#define GPIO_Enable_FAN()    (FUNC_GPIOE->BSRR.bit.BR1 = 1)   //清零 Bit Reset 打开风扇
#define GPIO_Disable_FAN()   (FUNC_GPIOE->BSRR.bit.BS1 = 1)  //置1  Bit Set/Reset  关闭风扇


//PC13 GPIO_E1 for 
#define GPIO_Enable_POS()    (FUNC_GPIOC->BSRR.bit.BR13 = 1)   //
#define GPIO_Disable_POS()   (FUNC_GPIOC->BSRR.bit.BS13 = 1)  //


//PC8 GPIO_E1 for 
#define GPIO_Enable_Z()    (FUNC_GPIOC->BSRR.bit.BR8 = 1)   //
#define GPIO_Disable_Z()   (FUNC_GPIOC->BSRR.bit.BS8 = 1)  //

//PC12 GPIO_C12 for Relay_DSP 母线软启动用  输出量
#define GPIO_Disable_SoftStart()  (FUNC_GPIOC->BSRR.bit.BR12 = 1)  //清零 Bit Reset   继电器关闭短路限流电阻，软启动结束
#define GPIO_Enable_SoftStart()   (FUNC_GPIOC->BSRR.bit.BS12 = 1) //置1  Bit Set/Reset 继电器打开接入限流电阻，软启动使能

#if DRIVER_TYPE == SERVO_650N
#define GPIO_ReadMOTORPTCDSP()     (((FUNC_GPIOA->IDR.bit.IDR4)==1)? 0 : 1)
#endif

#if 0
/** @defgroup GPIO_Pin_sources 
  * @{
  */ 
#define GPIO_PinSource0            ((uint8_t)0x00)
#define GPIO_PinSource1            ((uint8_t)0x01)
#define GPIO_PinSource2            ((uint8_t)0x02)
#define GPIO_PinSource3            ((uint8_t)0x03)
#define GPIO_PinSource4            ((uint8_t)0x04)
#define GPIO_PinSource5            ((uint8_t)0x05)
#define GPIO_PinSource6            ((uint8_t)0x06)
#define GPIO_PinSource7            ((uint8_t)0x07)
#define GPIO_PinSource8            ((uint8_t)0x08)
#define GPIO_PinSource9            ((uint8_t)0x09)
#define GPIO_PinSource10           ((uint8_t)0x0A)
#define GPIO_PinSource11           ((uint8_t)0x0B)
#define GPIO_PinSource12           ((uint8_t)0x0C)
#define GPIO_PinSource13           ((uint8_t)0x0D)
#define GPIO_PinSource14           ((uint8_t)0x0E)
#define GPIO_PinSource15           ((uint8_t)0x0F)

//AF 0 selection   
#define GPIO_AF_RTC_50Hz      ((Uint8)0x00)  /* RTC_50Hz Alternate Function mapping */
#define GPIO_AF_MCO           ((Uint8)0x00)  /* MCO (MCO1 and MCO2) Alternate Function mapping */
#define GPIO_AF_TAMPER        ((Uint8)0x00)  /* TAMPER (TAMPER_1 and TAMPER_2) Alternate Function mapping */
#define GPIO_AF_SWJ           ((Uint8)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping */
#define GPIO_AF_TRACE         ((Uint8)0x00)  /* TRACE Alternate Function mapping */

//AF 1 selection  
#define GPIO_AF_TIM1          ((Uint8)0x01)  /* TIM1 Alternate Function mapping */
#define GPIO_AF_TIM2          ((Uint8)0x01)  /* TIM2 Alternate Function mapping */

//AF 2 selection   
#define GPIO_AF_TIM3          ((Uint8)0x02)  /* TIM3 Alternate Function mapping */
#define GPIO_AF_TIM4          ((Uint8)0x02)  /* TIM4 Alternate Function mapping */
#define GPIO_AF_TIM5          ((Uint8)0x02)  /* TIM5 Alternate Function mapping */

//AF 3 selection  
#define GPIO_AF_TIM8          ((Uint8)0x03)  /* TIM8 Alternate Function mapping */
#define GPIO_AF_TIM9          ((Uint8)0x03)  /* TIM9 Alternate Function mapping */
#define GPIO_AF_TIM10         ((Uint8)0x03)  /* TIM10 Alternate Function mapping */
#define GPIO_AF_TIM11         ((Uint8)0x03)  /* TIM11 Alternate Function mapping */

//AF 4 selection  
#define GPIO_AF_I2C1          ((Uint8)0x04)  /* I2C1 Alternate Function mapping */
#define GPIO_AF_I2C2          ((Uint8)0x04)  /* I2C2 Alternate Function mapping */
#define GPIO_AF_I2C3          ((Uint8)0x04)  /* I2C3 Alternate Function mapping */

//AF 5 selection  
#define GPIO_AF_SPI1          ((Uint8)0x05)  /* SPI1 Alternate Function mapping */
#define GPIO_AF_SPI2          ((Uint8)0x05)  /* SPI2/I2S2 Alternate Function mapping */

//AF 6 selection  
#define GPIO_AF_SPI3          ((Uint8)0x06)  /* SPI3/I2S3 Alternate Function mapping */

//AF 7 selection  
#define GPIO_AF_USART1        ((Uint8)0x07)  /* USART1 Alternate Function mapping */
#define GPIO_AF_USART2        ((Uint8)0x07)  /* USART2 Alternate Function mapping */
#define GPIO_AF_USART3        ((Uint8)0x07)  /* USART3 Alternate Function mapping */

//AF 8 selection  
#define GPIO_AF_UART4         ((Uint8)0x08)  /* UART4 Alternate Function mapping */
#define GPIO_AF_UART5         ((Uint8)0x08)  /* UART5 Alternate Function mapping */
#define GPIO_AF_USART6        ((Uint8)0x08)  /* USART6 Alternate Function mapping */

//AF 9 selection  
#define GPIO_AF_CAN1          ((Uint8)0x09)  /* CAN1 Alternate Function mapping */
#define GPIO_AF_CAN2          ((Uint8)0x09)  /* CAN2 Alternate Function mapping */
#define GPIO_AF_TIM12         ((Uint8)0x09)  /* TIM12 Alternate Function mapping */
#define GPIO_AF_TIM13         ((Uint8)0x09)  /* TIM13 Alternate Function mapping */
#define GPIO_AF_TIM14         ((Uint8)0x09)  /* TIM14 Alternate Function mapping */

//AF 10 selection   
#define GPIO_AF_OTG_FS         ((Uint8)0xA)  /* OTG_FS Alternate Function mapping */
#define GPIO_AF_OTG_HS         ((Uint8)0xA)  /* OTG_HS Alternate Function mapping */

//AF 11 selection  
#define GPIO_AF_ETH             ((Uint8)0x0B)  /* ETHERNET Alternate Function mapping */

//AF 12 selection  
#define GPIO_AF_FSMC            ((Uint8)0xC)  /* FSMC Alternate Function mapping */
#define GPIO_AF_OTG_HS_FS       ((Uint8)0xC)  /* OTG HS configured in FS, Alternate Function mapping */
#define GPIO_AF_SDIO            ((Uint8)0xC)  /* SDIO Alternate Function mapping */

//AF 13 selection   
#define GPIO_AF_DCMI          ((Uint8)0x0D)  /* DCMI Alternate Function mapping */

//AF 15 selection  
#define GPIO_AF_EVENTOUT      ((Uint8)0x0F)  /* EVENTOUT Alternate Function mapping */
#endif

//GPIOx_CRL端口配置结构体类型定义
typedef struct{
    Uint32  MODER0:2;        //端口模式位
    Uint32  MODER1:2;        //端口配置位
    Uint32  MODER2:2;        //端口模式位
    Uint32  MODER3:2;        //端口配置位
    Uint32  MODER4:2;        //端口模式位
    Uint32  MODER5:2;        //端口配置位
    Uint32  MODER6:2;        //端口模式位
    Uint32  MODER7:2;        //端口配置位
    Uint32  MODER8:2;        //端口模式位
    Uint32  MODER9:2;        //端口配置位
    Uint32  MODER10:2;       //端口模式位
    Uint32  MODER11:2;       //端口配置位
    Uint32  MODER12:2;       //端口模式位
    Uint32  MODER13:2;       //端口配置位
    Uint32  MODER14:2;       //端口模式位
    Uint32  MODER15:2;       //端口配置位
}STR_GPIOx_MODER_BIT; 
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_MODER_BIT     bit; 
}UNI_GPIOx_MODER_REG; 

//GPIOx_OTYPER端口配置结构体类型定义
typedef struct{
    Uint32  OT0:1;        //端口模式位
    Uint32  OT1:1;        //端口配置位
    Uint32  OT2:1;        //端口模式位
    Uint32  OT3:1;        //端口配置位
    Uint32  OT4:1;        //端口模式位
    Uint32  OT5:1;        //端口配置位
    Uint32  OT6:1;        //端口模式位
    Uint32  OT7:1;        //端口配置位
    Uint32  OT8:1;        //端口模式位
    Uint32  OT9:1;        //端口配置位
    Uint32  OT10:1;       //端口模式位
    Uint32  OT11:1;       //端口配置位
    Uint32  OT12:1;       //端口模式位
    Uint32  OT13:1;       //端口配置位
    Uint32  OT14:1;       //端口模式位
    Uint32  OT15:1;       //端口配置位
	Uint32  Reserved:16;	  //端口配置位
}STR_GPIOx_OTYPER_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_OTYPER_BIT    bit; 
}UNI_GPIOx_OTYPER_REG;

//GPIOx_OSPEEDR端口配置结构体类型定义
typedef struct{
    Uint32  OSPEEDR0:2;        //端口模式位
    Uint32  OSPEEDR1:2;        //端口配置位
    Uint32  OSPEEDR2:2;        //端口配置位
    Uint32  OSPEEDR3:2;        //端口模式位
    Uint32  OSPEEDR4:2;        //端口配置位
    Uint32  OSPEEDR5:2;        //端口模式位
    Uint32  OSPEEDR6:2;        //端口配置位
    Uint32  OSPEEDR7:2;        //端口配置位
    Uint32  OSPEEDR8:2;        //端口配置位
    Uint32  OSPEEDR9:2;        //端口配置位
    Uint32  OSPEEDR10:2;       //端口配置位
    Uint32  OSPEEDR11:2;       //端口配置位
    Uint32  OSPEEDR12:2;       //端口配置位
    Uint32  OSPEEDR13:2;       //端口配置位
    Uint32  OSPEEDR14:2;       //端口配置位
    Uint32  OSPEEDR15:2;       //端口配置位
}STR_GPIOx_OSPEEDR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_OSPEEDR_BIT   bit; 
}UNI_GPIOx_OSPEEDR_REG;

//GPIOx_PUPDR端口配置结构体类型定义
typedef struct{
    Uint32  PUPDR0:2;        //端口模式位
    Uint32  PUPDR1:2;        //端口配置位
    Uint32  PUPDR2:2;        //端口配置位
    Uint32  PUPDR3:2;        //端口模式位
    Uint32  PUPDR4:2;        //端口配置位
    Uint32  PUPDR5:2;        //端口模式位
    Uint32  PUPDR6:2;        //端口配置位
    Uint32  PUPDR7:2;        //端口配置位
    Uint32  PUPDR8:2;        //端口配置位
    Uint32  PUPDR9:2;        //端口配置位
    Uint32  PUPDR10:2;       //端口配置位
    Uint32  PUPDR11:2;       //端口配置位
    Uint32  PUPDR12:2;       //端口配置位
    Uint32  PUPDR13:2;       //端口配置位
    Uint32  PUPDR14:2;       //端口配置位
    Uint32  PUPDR15:2;       //端口配置位
}STR_GPIOx_PUPDR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_PUPDR_BIT     bit; 
}UNI_GPIOx_PUPDR_REG;

//GPIOx_IDR端口输入数据结构体类型定义
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
    volatile Uint32                  all;
    volatile STR_GPIOx_IDR_BIT       bit;
}UNI_GPIOx_IDR_REG;

//GPIOx_ODR端口输出数据结构体类型定义
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
    volatile Uint32                  all;
    volatile STR_GPIOx_ODR_BIT       bit;
}UNI_GPIOx_ODR_REG;

//GPIOx_BSRR端口输出数据结构体类型定义
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
    volatile Uint32                  all;
    volatile STR_GPIOx_BSRR_BIT       bit;
}UNI_GPIOx_BSRR_REG;

//GPIOx_BRR端口输出数据结构体类型定义
typedef struct{
    Uint32  LCK0:1;
    Uint32  LCK1:1;
    Uint32  LCK2:1;
    Uint32  LCK3:1;
    Uint32  LCK4:1;
    Uint32  LCK5:1;
    Uint32  LCK6:1;
    Uint32  LCK7:1;
    Uint32  LCK8:1;
    Uint32  LCK9:1;
    Uint32  LCK10:1;
    Uint32  LCK11:1;
    Uint32  LCK12:1;
    Uint32  LCK13:1;
    Uint32  LCK14:1;
    Uint32  LCK15:1;
    Uint32  Reserved:16;
}STR_GPIOx_LCKR_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_LCKR_BIT       bit;
}UNI_GPIOx_LCKR_REG;

//GPIOx_AFRL端口输出数据结构体类型定义
typedef struct{
    Uint32  AFR0:4;
    Uint32  AFR1:4;
    Uint32  AFR2:4;
    Uint32  AFR3:4;
    Uint32  AFR4:4;
    Uint32  AFR5:4;
    Uint32  AFR6:4;
    Uint32  AFR7:4;
}STR_GPIOx_AFRL_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_AFRL_BIT      bit;
}UNI_GPIOx_AFRL_REG;
//GPIOx_AFRH端口输出数据结构体类型定义
typedef struct{
    Uint32  AFR8:4;
    Uint32  AFR9:4;
    Uint32  AFR10:4;
    Uint32  AFR11:4;
    Uint32  AFR12:4;
    Uint32  AFR13:4;
    Uint32  AFR14:4;
    Uint32  AFR15:4;
}STR_GPIOx_AFRH_BIT;
typedef union{
    volatile Uint32                  all;
    volatile STR_GPIOx_AFRH_BIT      bit;
}UNI_GPIOx_AFRH_REG;

////GPIOx寄存器结构体类型定义
typedef struct
{
    volatile UNI_GPIOx_MODER_REG       MODER  ;
    volatile UNI_GPIOx_OTYPER_REG      OTYPER ;
    volatile UNI_GPIOx_OSPEEDR_REG     OSPEEDR;
    volatile UNI_GPIOx_PUPDR_REG       PUPDR  ;
	volatile UNI_GPIOx_IDR_REG         IDR    ;
	volatile UNI_GPIOx_ODR_REG         ODR    ;
    volatile UNI_GPIOx_BSRR_REG        BSRR   ;
    volatile UNI_GPIOx_LCKR_REG        LCKR   ;
	volatile UNI_GPIOx_AFRL_REG        AFRL   ;
	volatile UNI_GPIOx_AFRH_REG        AFRH   ;
}STR_FUNC_GPIO_Def;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern  STR_FUNC_GPIO_Def * FUNC_GPIOA;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOB;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOC;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOD;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOE;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOF;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOG;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOH;
extern  STR_FUNC_GPIO_Def * FUNC_GPIOI;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void InitFUNC_GPIO(void);

#ifdef __cplusplus
}
#endif

#endif /* __FUNC_GPIODRIVER_H */

/********************************* END OF FILE *********************************/
