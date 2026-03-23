/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_AdcDriver_ST.h                                                           
 创建人：陈培正                创建日期：2008.06.18
 修改人：XX                    修改日期：XX.XX.XX 
 描述： 
    1.
    2.
 修改记录：  
    1. 2011.12       熊飞  
       变更内容：a)删去str_UTOV结构体变量类型定义，因为程序中没有用到；
                 b)删去str_ONCHIPADC结构体变量类型定义中的g_AdcState变量，因为程序中没有用到；
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef FUNC_ADCDRIVER_ST_H
#define FUNC_ADCDRIVER_ST_H

#ifdef __cplusplus
 extern "C" {
#endif    

/*******************u/v/w相电流由FPGA采样，剩余变量在AD上通道分配如下**********************/
//            1        2           3             4             5           6            7  
//ADC1******AD-AI1   AD-AI3     AD-Temp       AD-REF25       AD-AI2      AD-DC        AD-Pwr
//信号名称    AI1     AI3      驱动器温度   参考电压2.5V       AI2      母线电压    电源电压2.5V
//ADC2******AD-AI2   AD-DC      AD-Pwr        AD-AI1         AD-AI3     AD-TEMP       AD-REF25     
//信号名称    AI2   母线电压  电源电压2.5V      AI1            AI3      驱动器温度  参考电压2.5V 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define INIT_AD_SAMPLETIMES   64        //定义了用软件触发模式下ADC转换的次数
#define INIT_AD_SAMPLETIMES_BIT 6       //转换次数的幂（2^6 = 64）

typedef struct
{
    volatile Uint32 SR;
    volatile Uint32 CR1;
    volatile Uint32 CR2;
    volatile Uint32 SMPR1;
    volatile Uint32 SMPR2;
    volatile Uint32 JOFR1;
    volatile Uint32 JOFR2;
    volatile Uint32 JOFR3;
    volatile Uint32 JOFR4;
    volatile Uint32 HTR;
    volatile Uint32 LTR;
    volatile Uint32 SQR1;
    volatile Uint32 SQR2;
    volatile Uint32 SQR3;
    volatile Uint32 JSQR;
    volatile Uint32 JDR1;
    volatile Uint32 JDR2;
    volatile Uint32 JDR3;
    volatile Uint32 JDR4;
    volatile Uint32 DR;
} ADC_TypeDef_T;

#define PERIPH_BASE_T             ((Uint32)0x40000000) /*!< Peripheral base address in the bit-band region */
#define APB2PERIPH_BASE_T         (PERIPH_BASE_T + 0x10000)
#define ADC1_BASE_T               (APB2PERIPH_BASE_T + 0x2000)
#define ADC1_T                    ((ADC_TypeDef_T *) ADC1_BASE_T)

/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */    
#define ADC_START_ADC1()   ADC1_T->CR2 |=((Uint32)0x40000000)


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//暂无

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitAdc(void);
extern void Get_ADC_ChannelValueAIUdc_16k(void);
extern void Get_ADC_ChannelValueIPM_1k(void);
extern void Get_ADC_ChannelValuePwr_1k(void);


#ifdef __cplusplus
}
#endif

#endif /* FUNC_ADCDRIVER_ST_H */    

/********************************* END OF FILE *********************************/
