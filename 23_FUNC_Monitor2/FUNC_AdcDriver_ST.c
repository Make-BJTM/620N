/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_AdcDirver_ST.c                                                           
 创建人：陈培正               创建日期：2008.05
 修改人：童文邹 姚虹 熊飞     修改日期：2011.12
 描述： 
    1. AD底层驱动；
    2. 针对AD采样的DMA底层驱动；
 修改记录：  
    1. 2009.11.30      童文邹  
       变更内容： 修改了纠偏处理，初始化检测故障的报出,增加了电流采样处理程序,采样的获取函数等等，当PWM BLK为高时候检测0偏
    2. 2009.12.11      姚虹
       变更内容： 加入了UV相间校正功能
    3. 2011.12         熊飞
       变更内容： a)去掉增益、偏置计算函数void ADCCalibration(int32 *CalGain, int16 *CalOffset)，因为ST的AD没有这一用法,并且
                    112中实质上也没有让该函数起作用；
                  b)在AD初始化函数中，添加对采样值变量赋初值，以防止AI中初次使用该变量
                    时因调度关系问题出现的错误，如：AIx通道若初值为0，那么其电压将大于11.5V，造成伺服上电时误报AI故障；
                  c)更正函数void GetADCInitValue(Uint16 AdChannel, Uint16 *AdcSeqValue)，因为其对注入通道的配置有误，比如
                    触发配置，原来为对规则通道的配置，所以实际上注入通道的软件触发无效；
********************************************************************************/

//IS620N AD的分配通道如下
//PC0  Vref1.0   参考电压1.0V    CH10
//PC1  Vref2.5   参考电压2.5V    CH11
//PC2  AD-DC     母线电压        CH12
//PC4  AD-TEMP   驱动器温度      CH14
//PC5  AD-PWR    控制电电压      CH15

//IS650N AD的分配通道如下
//PC0  Vref1.0   参考电压1.0V    CH10
//PC1  Vref2.5   参考电压2.5V    CH11
//PC2  AD-IB     制动电流采样    CH12
//PC4  AD-TEMP   驱动器温度      CH14
//PC5  AD-DC     母线电压        CH15
 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_AdcDriver_ST.h"
#include "FUNC_Filter.h"

#include "FUNC_GPIODriver.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define    ADCREFL    1365
#define    ADCREFH    3413

// DSP的AD采样后得到数字量，对于AI1和AI2，AI输入电压为0V 时，AD采样数字量为 2048, 
// 即 0<==>2048，而 -12v<==>4096, +12V <==>0 ，是反向线性映射的关系
#define  ONCHIP_AD_ZERO     2048L 

//母线电压的范围：对应不同的驱动器电压等级
//#define DC220MAX 4640L  //0.1V  MD300S2.2  500V,
#define DC220MAX           5000L 

#if POWERDRIVER_TYPE==POWDRV_IS650
    #define DC380MAX_1000V_IS650        9868L
    #define DC380MAX_30KW_IS650         10000L 
	#define DC380MAX_18KW_IS650         9791L      
	#define DC380MAX_909V_IS650         9090L  
	    
    #define IB3VMAX_18KW                650    //65A 
	#define IB3VMAX_30KW                811    //81.1A
	#define IB3VMAX_37KW               1055    //105.5A
	#define IB3VMAX_45KW               1220    //122.0A
	#define IB3VMAX_55KW               1626    //1626A        
#else
    //AD采样时数字量4096对应 电压3V即母线电压1000V 
    #define DC380MAX_IS500     10000L 
#endif

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  




/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */     
volatile Uint32 ADC_RegularConvertedValueTab[3]; //存储AD转换的初始值,是DMA存储的目标

static int32  g_CalGain;
static int16  g_CalOffset;
static Uint16 Avg_AdcRefH = 0 ; //实际采样值
static Uint16 Avg_AdcRefL = 0 ; //实际采样值

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   
void InitAdc(void);                     
void Get_ADC_ChannelValueAIUdc_16k(void);   //AI和母线电压Udc采样16k获取
void Get_ADC_ChannelValueIPM_1k(void);      //IPM温度采样获取


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void ADSampConfig(void);
Static_Inline void ADCOfftrimInit(void);
Static_Inline void ADCCalibration(void);
Static_Inline void GetADCInitValue(void);

/*******************************************************************************
函数名: void InitAdc(void)   初始化ADC配置
输入  : 无  
输出  : 无  
子函数: void ADCOfftrimInit(void);
        void ADSampConfig(void);
        void ADC_SoftwareStartConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);     
描述  : 对AD的初始化操作
        1.对AD采样值自动纠偏；
        2.AD底层配置：通道分配、转换模式、中断、DMA配置等；
        3.对AD采样缓存赋初值：2048 
********************************************************************************/ 
void InitAdc(void)
{    
    //--------------AD采样外设配置----------------------------------------//  

	FUNC_GPIOC->MODER.bit.MODER0 = FUNC_GPIO_Mode_AN;      //VREF1.0 PC0  模拟输入
    FUNC_GPIOC->PUPDR.bit.PUPDR0 = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOC->MODER.bit.MODER1 = FUNC_GPIO_Mode_AN;      //VREF2.5 PC1 模拟输入
    FUNC_GPIOC->PUPDR.bit.PUPDR1 = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOC->MODER.bit.MODER2 = FUNC_GPIO_Mode_AN;      //AD-DC PC2   模拟输入
    FUNC_GPIOC->PUPDR.bit.PUPDR2 = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOC->MODER.bit.MODER4 = FUNC_GPIO_Mode_AN;      //AD-TEMP PC4 模拟输入
    FUNC_GPIOC->PUPDR.bit.PUPDR4 = FUNC_GPIO_PuPd_NOPULL;
    FUNC_GPIOC->MODER.bit.MODER5 = FUNC_GPIO_Mode_AN;      //AD-PWR PC5  模拟输入
    FUNC_GPIOC->PUPDR.bit.PUPDR5 = FUNC_GPIO_PuPd_NOPULL;    

    //调用纠偏函数
    ADCOfftrimInit();  

    //调用工作模式配置模式
    ADSampConfig();
   
    ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
    /*AD上电*/
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);  

    //初始化之后启动一次转换  调用stm32f20x_adc.c函数
    ADC_SoftwareStartConv(ADC1);
   
   //赋初值
    STR_FUNC_Gvar.ADC_Samp.Udc_Live  = 0;    
    STR_FUNC_Gvar.ADC_Samp.IPMT = 2048;    
    STR_FUNC_Gvar.ADC_Samp.Pwr  = 0;     
    STR_FUNC_Gvar.ADC_Samp.Ib  = 0;    
} 


/*******************************************************************************
函数名: void ADCOfftrimInit(void)   AD自动纠偏，ST的DSP自带此功能 在函数InitAdc()中被调用
输入  : 无    
输出  : 无  
子函数: void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState); 
        void ADC_ResetCalibration(ADC_TypeDef* ADCx); 
        FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef* ADCx);        
描述  : AD自动纠偏，ST的DSP自带此功能 
********************************************************************************/ 
Static_Inline void ADCOfftrimInit(void) 
{
    ADCCalibration();

    ADC_Cmd(ADC1, DISABLE);
    ADC_Cmd(ADC2, DISABLE);

    DELAY_US(50); 
}
/*******************************************************************************
函数名: ADCCalibration(void)   获取AD纠偏基准源的采样值
输入  : 无    
输出  : CalGain     Q12
        CalOffset   Q0 
子函数:  
         
               
描述  : 获取CalGain, CalOffset; 
********************************************************************************/ 
Static_Inline void ADCCalibration()
{
    Uint16 AdcRefLReaL = ADCREFL;     //理想值1V
    Uint16 AdcRefHReaL = ADCREFH;     //理想值2.5V

    //获取基准源采样数据
    GetADCInitValue();

    g_CalGain   = (((int32)AdcRefHReaL  - AdcRefLReaL)<<12) / (int32)(Avg_AdcRefH - Avg_AdcRefL);
   
    g_CalOffset = (int16)AdcRefLReaL  - (int16)((Avg_AdcRefL* (g_CalGain) + (1<<11))>>12);
}

/*******************************************************************************
函数名: GetADCInitValue(void)   获取AD纠偏基准源的采样值
输入  : 无    
输出  : 无  
子函数:  
         
               
描述  :  
********************************************************************************/ 
Static_Inline void GetADCInitValue()                   
{
    static Uint32 ADCChannelValue[8]; 
    Uint16 i = 0;
    Uint16 j = 0;
    Uint16 ADCInitA = 0;
    Uint16 ADCInitB = 0;
    Uint32 SumADCInitA = 0;
    Uint32 SumADCInitB = 0;

    //工作模式的配置

    DMA_InitTypeDef   DMA_InitStructure;
    ADC_InitTypeDef   ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    /*********************************DMA2 Channel0 配置**********************************************************/

    DMA_DeInit(DMA2_Stream0);
    DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
    DMA_InitStructure.DMA_Memory0BaseAddr = (Uint32)&ADCChannelValue;
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_BASE + 0x308;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 8;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;        
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);

    /* DMA2_Stream0 enable */
    DMA_Cmd(DMA2_Stream0, ENABLE);

    /*********************************ADC1 配置**********************************************************/
    ADC_DeInit();

     /* ADC Common Init **********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC1 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;  
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 8;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_DMACmd(ADC1,ENABLE);
    ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);
    ADC_EOCOnEachRegularChannelCmd(ADC1,ENABLE);

  /* ADC1 regular channel7 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);      //设channel 11( 2.5V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_3Cycles);      //设channel 10( 1.0V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 3, ADC_SampleTime_3Cycles);      //设channel 11( 2.5V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, ADC_SampleTime_3Cycles);      //设channel 10( 1.0V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_3Cycles);      //设channel 11( 2.5V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 6, ADC_SampleTime_3Cycles);      //设channel 10( 1.0V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 7, ADC_SampleTime_3Cycles);      //设channel 11( 2.5V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 8, ADC_SampleTime_3Cycles);      //设channel 10( 1.0V )为第2个转换，采样时间1.5个AD周期
    /* ADC2 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;  
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 8;
    ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC2 regular channel7 configuration *************************************/
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);    //设channel 10 ( 1.0V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_11,2, ADC_SampleTime_3Cycles);     //设channel 11 ( 2.5V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 3, ADC_SampleTime_3Cycles);    //设channel 10 ( 1.0V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_11,4, ADC_SampleTime_3Cycles);     //设channel 11 ( 2.5V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 5, ADC_SampleTime_3Cycles);    //设channel 10 ( 1.0V )为第1个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_11,6, ADC_SampleTime_3Cycles);     //设channel 11 ( 2.5V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 7, ADC_SampleTime_3Cycles);    //设channel 10 ( 1.0V )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_11,8, ADC_SampleTime_3Cycles);     //设channel 11 ( 2.5V )为第2个转换，采样时间1.5个AD周期
  
    /*AD上电*/
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE); 

    DELAY_US(50);
    
    //for (i = 0; i < 16; i++)
	for (i = 0; i < 8; i++)	//by huangxin201711_23 溢出bug
    {
        ADCChannelValue[i] = 0;
    }
    
    //前50次采样值误差大，不用
    for (i = 0; i < 50; i++)
    {
        ADC_SoftwareStartConv(ADC1);
        //等待AD转换完成    
        while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));  //ADC转换完成

        ADC_ClearFlag(ADC1,ADC_FLAG_EOC);            //清楚转换完成标志   
    }
    
    for (i = 0; i < INIT_AD_SAMPLETIMES; i++)
    {
        //重新赋值
        ADCInitA = 0; 
        ADCInitB = 0; 

        ADC_SoftwareStartConv(ADC1);
        //等待AD转换完成    
        while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));  //ADC转换完成

        ADC_ClearFlag(ADC1,ADC_FLAG_EOC);            //清楚转换完成标志   
    
        for (j = 0; j < 8; )
        {    
            ADCInitA += (ADCChannelValue[j]   & 0x0000ffff);  //2.5 通道;
            ADCInitB += (ADCChannelValue[j] >>16)          ;  //1V  
            j++;
            ADCInitA += (ADCChannelValue[j] >>16);  //2.5 通道;
            ADCInitB += (ADCChannelValue[j]   & 0x0000ffff);  //1V
            j++;  
        } 
        
        //此步求取可用于存储分析AD采样值
        ADCInitA = ADCInitA >> 3;   
        ADCInitB = ADCInitB >> 3;
        
        //获取采样值总和
        SumADCInitA += ADCInitA;
        SumADCInitB += ADCInitB;              
    }
    
    //获取AD多次采样的平均值   
    Avg_AdcRefH = (Uint16)(SumADCInitA >> INIT_AD_SAMPLETIMES_BIT);
    Avg_AdcRefL = (Uint16)(SumADCInitB >> INIT_AD_SAMPLETIMES_BIT);    
}


/*******************************************************************************
函数名:  void ADSampConfig(void)       在函数InitAdc()中被调用
输入  :  无  
输出  :  无  
子函数:  void DMA_DeInit(DMA_Channel_TypeDef* DMAy_Channelx)；
         void DMA_Init(DMA_Channel_TypeDef* DMAy_Channelx, DMA_InitTypeDef* DMA_InitStruct)；
         void DMA_ITConfig(DMA_Channel_TypeDef* DMAy_Channelx, uint32_t DMA_IT, FunctionalState NewState)；
         void DMA_Cmd(DMA_Channel_TypeDef* DMAy_Channelx, FunctionalState NewState)；
         void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct)；
         void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)；
         void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState)；
         void ADC_ExternalTrigConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)；
         void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState)；       
描述  :  1.配置DMA：外设AD地址、DMA存储器地址，从AD读取7组32位数据到DMA1_channel1存储区，循环模式，非存储器对存储器模式；
         2.配置AD ：双AD模式，同步规则通道采样，AD1与AD2同步采样，但转换顺序不一样，使能DMA请求，采样窗宽度，外部触发（TIM1CC1事件）；
********************************************************************************/
Static_Inline void ADSampConfig(void)
{
    DMA_InitTypeDef   DMA_InitStructure;
    ADC_InitTypeDef   ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    /*********************************DMA2 Channel0 配置**********************************************************/
    DMA_DeInit(DMA2_Stream0);
    DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
    DMA_InitStructure.DMA_Memory0BaseAddr = (Uint32)&ADC_RegularConvertedValueTab;
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_BASE + 0x308;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = 3; 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);

    /* DMA2_Stream0 enable */
    DMA_Cmd(DMA2_Stream0, ENABLE);

 
    /*********************************ADC1 配置**********************************************************/
    /*复位ADC1 & ADC2*/ 
    ADC_DeInit();

     /* ADC Common Init **********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC1 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;  
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 3;

    ADC_Init(ADC1, &ADC_InitStructure);


  /* ADC1 regular channel7 configuration *************************************/
#if DRIVER_TYPE == SERVO_650N
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_3Cycles);      //设channel 14(  Temp )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 2, ADC_SampleTime_3Cycles);      //设channel 15(  DC   )为第4个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_3Cycles);      //设channel 12(  IB   )为第5个转换，采样时间1.5个AD周期
#else
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_3Cycles);      //设channel 14(  Temp )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_3Cycles);      //设channel 12(  DC   )为第4个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 3, ADC_SampleTime_3Cycles);      //设channel 15(  Pwr  )为第4个转换，采样时间1.5个AD周期
#endif 

    /* ADC2 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;  
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
    ADC_InitStructure.ADC_NbrOfConversion = 3;
    ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC2 regular channel7 configuration *************************************/
#if DRIVER_TYPE == SERVO_650N
    ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 1, ADC_SampleTime_3Cycles);     //设channel 15(  DC   )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 2, ADC_SampleTime_3Cycles);     //设channel 12(  IB   )为第4个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 3, ADC_SampleTime_3Cycles);     //设channel 14(  Temp )为第4个转换，采样时间1.5个AD周期
#else
    ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);     //设channel 12(  DC   )为第2个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 2, ADC_SampleTime_3Cycles);     //设channel 15(  Pwr  )为第4个转换，采样时间1.5个AD周期
    ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 3, ADC_SampleTime_3Cycles);     //设channel 14(  Temp )为第4个转换，采样时间1.5个AD周期
#endif
} 
/*******************************************************************************
函数名:
输入  : 无
输出  : 无
描述  : 
********************************************************************************/ 
void Get_ADC_ChannelValueAIUdc_16k(void)
{
    static STR_MidFilter     UdcChanelFilter = MidFilter_Defaults ;

    int32 Udc_Temp = 0;
    int32 UdcCoef = 0;              //母线调节器系数更新值
    
	// 将DMA读取的数据按实际变量的数据宽度进行分离，存储区位32位，实际变量为16位      
    Udc_Temp  = (ADC_RegularConvertedValueTab[1] & 0x0000ffff) + (ADC_RegularConvertedValueTab[0] >>16); //DC  通道

    //Udc中值滤波 取最近三次采样中间值
    Udc_Temp  = (((int32)Udc_Temp * g_CalGain) >>13) + g_CalOffset;
    if(Udc_Temp < 0)
    {
        Udc_Temp = 0 ;
    }
    UdcChanelFilter.InPut = Udc_Temp; 

	AvgFilter(&UdcChanelFilter); 

#if DRIVER_TYPE == SERVO_650N
    if(FunCodeUnion.code.OEM_ServoSeri == 20005)
	{	 
	    UdcCoef = DC380MAX_30KW_IS650 * FunCodeUnion.code.OEM_UdcGain / 1000;
	}
	else if((FunCodeUnion.code.OEM_ServoSeri == 20007)
	      ||(FunCodeUnion.code.OEM_ServoSeri == 20008))
	{
	    UdcCoef = DC380MAX_1000V_IS650 * FunCodeUnion.code.OEM_UdcGain / 1000;
		
	}
	else if(FunCodeUnion.code.OEM_ServoSeri == 20015)
	{	 
	    UdcCoef = DC380MAX_909V_IS650 * FunCodeUnion.code.OEM_UdcGain / 1000;
	}
	else
	{
	    UdcCoef = DC380MAX_18KW_IS650 * FunCodeUnion.code.OEM_UdcGain / 1000;
	}

    STR_FUNC_Gvar.ADC_Samp.Udc_Live = (int32)((UdcCoef * (UdcChanelFilter.OutPut)) >> 12);
#else 
    //母线电压采样系数220V
    if(FunCodeUnion.code.OEM_VoltClass == 220)      //H01_04  OEM驱动器电压级
    {   
        //母线电压增益，3V对应母线电压的最高电压500v,单位0.1v  
        //#define DC220MAX  5000L   即ADC数字量4096对应实际母线电压5000（0.1V）
        UdcCoef = DC220MAX * FunCodeUnion.code.OEM_UdcGain / 1000;
    }
    else if(FunCodeUnion.code.OEM_VoltClass == 380)
    {   
        //3V对应1000V即ADC数字量4096对应实际母线电压10000（0.1V） #define DC380MAX_IS500 10000L    
        UdcCoef = DC380MAX_IS500 * FunCodeUnion.code.OEM_UdcGain / 1000;
    }

    //母线电压采样结果进行运算Udc_Live为的实时母线电压值，单位对应0.1v
    //220V时3V对应母线电压的最高电压500v，也对应ADC数字量4096
    //380V时3V对应母线电压的最高电压1000v，也对应ADC数字量4096
    STR_FUNC_Gvar.ADC_Samp.Udc_Live = (int32)((UdcCoef * (UdcChanelFilter.OutPut)) >> 12); 
#endif

}


/*******************************************************************************
函数名:
输入  : 无
输出  : 无
描述  : 
********************************************************************************/ 
void Get_ADC_ChannelValueIPM_1k(void)
{
    int32 IPMT_Temp = 0;

     /* 将DMA读取的数据按实际变量的数据宽度进行分离，存储区位32位，实际变量为16位 */
    IPMT_Temp = (ADC_RegularConvertedValueTab[0] & 0x0000ffff) + (ADC_RegularConvertedValueTab[2] >>16);//Temp 通道

    IPMT_Temp = (((int32)IPMT_Temp * g_CalGain) >>13) + g_CalOffset;

    if(IPMT_Temp < 0)
    {
        IPMT_Temp = 0;
    }
    STR_FUNC_Gvar.ADC_Samp.IPMT = IPMT_Temp;
}


/*******************************************************************************
函数名:
输入  : 无
输出  : 无
描述  : 
********************************************************************************/ 
void Get_ADC_ChannelValuePwr_1k(void)
{
    int32 Pwr_Temp = 0; 

    Pwr_Temp  = (ADC_RegularConvertedValueTab[2] & 0x0000ffff) + (ADC_RegularConvertedValueTab[1] >>16); //Pwr  通道 

    Pwr_Temp  = (((int32)Pwr_Temp * g_CalGain) >>13) + g_CalOffset;
    if(Pwr_Temp < 0)
    {
        Pwr_Temp = 0 ;
    }

    STR_FUNC_Gvar.ADC_Samp.Pwr = Pwr_Temp;
}


/*******************************************************************************
函数名:
输入  : 无
输出  : 无
描述  : 
********************************************************************************/
#if POWERDRIVER_TYPE==POWDRV_IS650
void Get_ADC_ChannelValueIB_16k(void)
{
	static STR_MidFilter     IBChanelFilter  = MidFilter_Defaults ;
	int32 IB_Temp=0; 

	//计算刹车电流
	IB_Temp = (ADC_RegularConvertedValueTab[2] & 0x0000ffff) + (ADC_RegularConvertedValueTab[1] >>16); //刹车电流检测
	IB_Temp = (((int32)IB_Temp * g_CalGain) >>13) + g_CalOffset;  
	if(IB_Temp < 0)
    {
        IB_Temp = 0 ;
    }
    IBChanelFilter.InPut = IB_Temp;

	MidFilter(&IBChanelFilter);

	switch(FunCodeUnion.code.OEM_ServoSeri)
	{
	    case 20000:  case 20001: case 20002:  case 20003: case 20004:
		     STR_FUNC_Gvar.ADC_Samp.Ib = IBChanelFilter.OutPut * IB3VMAX_18KW >>12;
		     break;
		case 20005:
		     STR_FUNC_Gvar.ADC_Samp.Ib = IBChanelFilter.OutPut * IB3VMAX_30KW >>12;
		     break;
		case 20006:
		     STR_FUNC_Gvar.ADC_Samp.Ib = IBChanelFilter.OutPut * IB3VMAX_37KW >>12;
		     break;
		case 20007:
		     STR_FUNC_Gvar.ADC_Samp.Ib = IBChanelFilter.OutPut * IB3VMAX_45KW >>12;
		     break;
		case 20008:
		     STR_FUNC_Gvar.ADC_Samp.Ib = IBChanelFilter.OutPut * IB3VMAX_55KW >>12;
        default:
             STR_FUNC_Gvar.ADC_Samp.Ib = 0;
		     break;
	}
}
#endif
/********************************* END OF FILE *********************************/
