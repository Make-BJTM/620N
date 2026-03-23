/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_AoDriver_ST.c  
 创建人：   童文邹                 创建日期：2008.11.01
 修改人：   王治国                 修改日期：2012.03.20 
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
#include "FUNC_AoDriver_ST.h"
#include "FUNC_FunCode.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */
#define AO_ACT_SEL   1
#define AOPWMFREQ  50000   // AO输出的PWM频率；

#define AO_CHA              0
#define AO_CHB              1
#define AO_CH_NUM           2
#define AOSCALENUM          6

#define AO_NOSCALE              0        //无定标
#define AO_TOQSCALE_Q15         1        //转矩定标Q15
#define AO_SPDSCALE_Q20         2        //速度定标Q15
#define AO_POSPULSESCALE_Q4     3        //位置脉冲定标Q4
#define AO_DOSCALE              4        //DO信号定标
#define AO_AISCALE_Q15          5        //AI定标

//先去把下面两个宏定义置为0 和 1 测试到的数据计算后写入即可
#define CURRENT_OFFSET      100      //单位 mv   :when theretical output=0,the real output is equal to this value
#define CURRENT_GAIN        985L    //单位0.1%  : Delta(Theretical) /Delta(real)
#define PULSEMAX            45000L    //略大于100*（20V/0.05） (考虑最小倍率)
#define AOCMPLIMITL         20      //补偿硬件负电压高于10V
#define AOCMPLIMITH         1199

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
int32   TimPeriod = 0;
int32   AoConst[AOSCALENUM][AO_CH_NUM];
int16   AoOffset[AO_CH_NUM];

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
//void InitTim3(void);
void AoConst_Update(void);
void AoProcess(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
int32 AoCal(Uint16 SignalSel, Uint16 Ao_Ch);

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void InitTim3(void)
{
    GPIO_InitTypeDef    GPIO_InitStruct;           //定义FPGA中断的GPIO管脚的初始化结构体变量
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OC2InitStructure;  //TIM3 通道2输出配置 
    TIM_OCInitTypeDef       TIM_OC4InitStructure;  //TIM3 通道4输出配置

#if AO_ACT_SEL 

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;         // AO1 PB5 TIM3 通道2
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_TIM3);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;         // AO2 PC9 TIM3 通道4
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM3);  //LH modify TIM

    //定时器配置
    TIM_DeInit(TIM3);
    //TIM_DeInit(TIM8);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TimPeriod = (PUB_APB1_TIM_FREQ_84MHz / AOPWMFREQ) - 1;   // //LH modify TIM 得到定时器单周期的计数最大值

    TIM_TimeBaseStructure.TIM_Period = TimPeriod;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);     //配置TIM3时基

    //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    //TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);     //配置TIM8时基

    TIM_OCStructInit(&TIM_OC2InitStructure);

    TIM_OC2InitStructure.TIM_OCMode = TIM_OCMode_PWM2;               //计数器值大于比较值时输出有效电平高，反之为低！
    TIM_OC2InitStructure.TIM_OutputState = TIM_OutputState_Enable;   //开启相应的输出通道
    TIM_OC2InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出有效电平为高
    TIM_OC2InitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OC2InitStructure.TIM_Pulse = 0;
    TIM_OC2Init(TIM3, &TIM_OC2InitStructure);                        //只有TIM3的Ch2通道

    //TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Enable;
    //TIM_OCInitStructure.TIM_OCNPolarity  = TIM_OCPolarity_Low;
    //TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    //TIM_OC4Init(TIM8, &TIM_OCInitStructure);                        //只有TIM8的Ch4通道

    TIM_OCStructInit(&TIM_OC4InitStructure);

    TIM_OC4InitStructure.TIM_OCMode = TIM_OCMode_PWM2;               //计数器值大于比较值时输出有效电平高，反之为低！
    TIM_OC4InitStructure.TIM_OutputState = TIM_OutputState_Enable;   //开启相应的输出通道
    TIM_OC4InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出有效电平为高
    TIM_OC4InitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OC4InitStructure.TIM_Pulse = 0;
    TIM_OC4Init(TIM3, &TIM_OC4InitStructure);                        //只有TIM3的Ch4通道

    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);    //使能预装载寄存器
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_Cmd(TIM3, ENABLE);                              //使能定时器
    TIM3->CCR2 = (TimPeriod>>1);   //AO1 PB5 TIM3 
    TIM3->CCR4 = (TimPeriod>>1);   //AO2 PC9 TIM8

    //TIM_Cmd(TIM8, ENABLE);

    //高级定时器比较输出需要的设置
    //TIM_CtrlPWMOutputs(TIM8,ENABLE);

#endif
}  

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void AoConst_Update(void)
{
#if AO_ACT_SEL 
    int32 AO1Gain_Temp = 0;
    int32 AO2Gain_Temp = 0;
    int32 Temp = 0;

    if(FunCodeUnion.code.AO1Gain & 0x8000)
    {
        AO1Gain_Temp = FunCodeUnion.code.AO1Gain | 0xFFFF0000;
    }
    else
    {
        AO1Gain_Temp = FunCodeUnion.code.AO1Gain & 0x0000FFFF;    
    }

    if(FunCodeUnion.code.AO2Gain & 0x8000)
    {
        AO2Gain_Temp = FunCodeUnion.code.AO2Gain | 0xFFFF0000;
    }
    else
    {
        AO2Gain_Temp = FunCodeUnion.code.AO2Gain & 0x0000FFFF;     
    }
 
              
    //无定标
    AoConst[AO_NOSCALE][AO_CHA] = 0;
    AoConst[AO_NOSCALE][AO_CHB] = 0;  

    //转矩Q15, 功能模块转矩量纲0.1%, FunCodeUnion.code.AO1Gain量纲0.01, CURRENT_GAIN量纲0.001
    //AoConst[AO_TOQSCALE_Q15][]量纲 Q15_mv/0.1%
    AoConst[AO_TOQSCALE_Q15][AO_CHA] = (AO1Gain_Temp << 14) / 100;
    AoConst[AO_TOQSCALE_Q15][AO_CHA] = (int64)AoConst[AO_TOQSCALE_Q15][AO_CHA] * CURRENT_GAIN / 1000L;
    AoConst[AO_TOQSCALE_Q15][AO_CHB] = (AO2Gain_Temp << 14) / 100;
    AoConst[AO_TOQSCALE_Q15][AO_CHB] = (int64)AoConst[AO_TOQSCALE_Q15][AO_CHB] * CURRENT_GAIN / 1000L;

    //转速Q15, 功能模块速度量纲0.0001rpm, FunCodeUnion.code.AO1Gain量纲0.01, CURRENT_GAIN量纲0.001
    //AoConst[AO_SPDSCALE_Q15][]量纲 Q15_mv/rpm
    AoConst[AO_SPDSCALE_Q20][AO_CHA] = ((int64)AO1Gain_Temp << 19) / 10000L;
    AoConst[AO_SPDSCALE_Q20][AO_CHA] = (int64)AoConst[AO_SPDSCALE_Q20][AO_CHA] * CURRENT_GAIN / 100000L;
    AoConst[AO_SPDSCALE_Q20][AO_CHB] = ((int64)AO2Gain_Temp << 19) / 10000L;
    AoConst[AO_SPDSCALE_Q20][AO_CHB] = (int64)AoConst[AO_SPDSCALE_Q20][AO_CHB] * CURRENT_GAIN / 100000L;
    
    //位置脉冲Q4, 位置脉冲量纲1p, FunCodeUnion.code.AO1Gain量纲0.01, CURRENT_GAIN量纲0.001
    //AoConst[AO_POSPULSESCALE_Q4][]量纲 Q4_50mv/p
    AoConst[AO_POSPULSESCALE_Q4][AO_CHA] = ((AO1Gain_Temp << 3) * 50L)/100;
    AoConst[AO_POSPULSESCALE_Q4][AO_CHA] = (int64)AoConst[AO_POSPULSESCALE_Q4][AO_CHA] * CURRENT_GAIN / 1000L;
    AoConst[AO_POSPULSESCALE_Q4][AO_CHB] = ((AO2Gain_Temp << 3) * 50L)/100;
    AoConst[AO_POSPULSESCALE_Q4][AO_CHB] = (int64)AoConst[AO_POSPULSESCALE_Q4][AO_CHB] * CURRENT_GAIN / 1000L;

    //定位完成Q0, DO信号量纲1数字量, FunCodeUnion.code.AO1Gain量纲0.01, CURRENT_GAIN量纲0.001
    //AoConst[AO_DOSCALE][]量纲 5000mv/数字量
    AoConst[AO_DOSCALE][AO_CHA] = ((AO1Gain_Temp * 5000L) / 100)>>1;
    AoConst[AO_DOSCALE][AO_CHA] = (int64)AoConst[AO_DOSCALE][AO_CHA] * CURRENT_GAIN / 1000L;
    AoConst[AO_DOSCALE][AO_CHB] = ((AO2Gain_Temp * 5000L) / 100)>>1;
    AoConst[AO_DOSCALE][AO_CHB] = (int64)AoConst[AO_DOSCALE][AO_CHB] * CURRENT_GAIN / 1000L;

    //AI信号量纲32768->12V, FunCodeUnion.code.AO1Gain量纲0.01, CURRENT_GAIN量纲0.001
    //AoConst[AO_DOSCALE][]量纲 5000mv/数字量
    Temp = ((AO1Gain_Temp * 12000L) / 100)>>1;
    AoConst[AO_AISCALE_Q15][AO_CHA] = Temp * CURRENT_GAIN / 1000L;
    Temp = ((AO2Gain_Temp * 12000L) / 100)>>1;
    AoConst[AO_AISCALE_Q15][AO_CHB] = Temp * CURRENT_GAIN / 1000L;

    //FunCodeUnion.code.AO1Offset量纲mv, CURRENT_GAIN量纲0.001, CURRENT_OFFSET量纲mv
    //AoOffset[]量纲mv
    AoOffset[AO_CHA] =  (int16)FunCodeUnion.code.AO1Offset;
    AoOffset[AO_CHA] =  ((int64)AoOffset[AO_CHA] * CURRENT_GAIN / 1000L>>1)  + CURRENT_OFFSET;
    AoOffset[AO_CHB] =  (int16)FunCodeUnion.code.AO2Offset;
    AoOffset[AO_CHB] =  ((int64)AoOffset[AO_CHB]* CURRENT_GAIN / 1000L>>1)  + CURRENT_OFFSET;

#endif
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void AoProcess(void)
{
#if AO_ACT_SEL

    Uint16  ChAComp = 0;
    Uint16  ChBComp = 0;

    ChAComp = (Uint16)AoCal(FunCodeUnion.code.AO1SignalSel, AO_CHA); 
    ChBComp = (Uint16)AoCal(FunCodeUnion.code.AO2SignalSel, AO_CHB);

    //TimPeriod - > +10V;  0 -> -10V
    TIM3->CCR2 = ChAComp;   //AO1 PB5 TIM3 
    TIM3->CCR4 = ChBComp;   //AO2 PC9 TIM8
#endif
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
int32 AoCal(Uint16 SignalSel, Uint16 Ao_Ch)
{
#if AO_ACT_SEL
	int32 VoltCoe_Q15 = 0;
    int32 Volt = 0;
    int32 Comp = 0;
    int32 PosErrCntTemp = 0;


    //10000mv->TimPeriod
    VoltCoe_Q15 = ((int32)TimPeriod << 15) / 10000L;

    switch(SignalSel)
    {              
        //转速类Q15, 偏置应该换算后加在这里, 注意这里是减, CMP=0 ->100%占空比
        case 0:   
            Volt = (((int64)AoConst[AO_SPDSCALE_Q20][Ao_Ch] * UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >> 20) + AoOffset[Ao_Ch];
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15); //TimPeriod-((TimPeriod>>1) - (VoltCoe_Q15 * Volt >> 15))
            break;

        case 1:
            //MTR模块速度调节器输入,转矩模式或Soff时该值等于0
            Volt = (((int64)AoConst[AO_SPDSCALE_Q20][Ao_Ch] * STR_FUNC_Gvar.SpdCtrl.SpdRef) >> 20) + AoOffset[Ao_Ch];
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15); //TimPeriod-((TimPeriod>>1) - (VoltCoe_Q15 * Volt >> 15))
            break;

        case 5:
            //AuxFunCodeUnion.code.DP_SpdRef_Puse量纲 1rpm
            Volt = (int64)AoConst[AO_SPDSCALE_Q20][Ao_Ch] * (int16)AuxFunCodeUnion.code.DP_SpdRef_Puse * 10000L; 
            Volt = (Volt >> 20) + AoOffset[Ao_Ch];
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15); //TimPeriod-((TimPeriod>>1) - (VoltCoe_Q15 * Volt >> 15))
            break;

        case 7:
            Volt = (((int64)AoConst[AO_SPDSCALE_Q20][Ao_Ch] * STR_FUNC_Gvar.PosCtrl.FdFwdOut) >> 20) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15); //TimPeriod-((TimPeriod>>1) - (VoltCoe_Q15 * Volt >> 15))
            break;

        //转矩类Q15, 注意这里是减, CMP=0 ->100%占空比
        case 2:   
            Volt = ((AoConst[AO_TOQSCALE_Q15][Ao_Ch] * UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) >> 15) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15);
            break;

        //位置脉冲类Q4, 注意这里是减, CMP=0 ->100%占空比
        case 3:
            
            PosErrCntTemp = STR_FUNC_Gvar.OscTarget.PosAmpErr;

            if(PosErrCntTemp>PULSEMAX)
            {
                PosErrCntTemp=PULSEMAX;
            }
            else if(PosErrCntTemp<(-PULSEMAX))
            {
                PosErrCntTemp=-PULSEMAX;
            }
            Volt = ((AoConst[AO_POSPULSESCALE_Q4][Ao_Ch] * (int32)PosErrCntTemp) >> 4) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + ((int64)VoltCoe_Q15 * Volt >> 15);
            break;

        case 4:
            PosErrCntTemp=STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
            if(PosErrCntTemp>PULSEMAX)
            {
                PosErrCntTemp=PULSEMAX;
            }
            else if(PosErrCntTemp<(-PULSEMAX))
            {
                PosErrCntTemp=-PULSEMAX;
            }
            Volt = ((AoConst[AO_POSPULSESCALE_Q4][Ao_Ch] * PosErrCntTemp) >> 4) + AoOffset[Ao_Ch];  
            Comp = (TimPeriod>>1) + ((int64)VoltCoe_Q15 * Volt >> 15);
            break;

        //DO定位完成
        case 6:
            Volt = (AoConst[AO_DOSCALE][Ao_Ch] * (int16)STR_FUNC_Gvar.PosCtrl.DovarReg_Coin) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15);
            break;

        //AI1
        case 8:
            Volt = ((STR_FUNC_Gvar.AI.AI1VoltOut * AoConst[AO_AISCALE_Q15][Ao_Ch]) >> 15) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15);
            break;

        //AI2
        case 9:
            Volt = ((STR_FUNC_Gvar.AI.AI2VoltOut * AoConst[AO_AISCALE_Q15][Ao_Ch]) >> 15) + AoOffset[Ao_Ch]; 
            Comp = (TimPeriod>>1) + (VoltCoe_Q15 * Volt >> 15);
            break;

        default:
            break;
    }

    //限幅
    if(Comp > TimPeriod)  Comp = TimPeriod;
    if(Comp < AOCMPLIMITL)  Comp = AOCMPLIMITL;  

    return Comp;
#else
    return 0;
#endif
}



/********************************* END OF FILE *********************************/
