/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "stm32f4xx_tim.h"
#include "ECT_InterFace.h"
#include "ECT_CSP.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_System.h"
#include "MTR_System.h" 
#include "FUNC_FunCode.h"
#include "ECT_ESMDisplay.h"
#include "FUNC_GPIODriver.h"
#include "ECT_PP.h"
#include "PUB_Main.h"
#include "FUNC_PosCtrl.h"          

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
Uint16 * TIM7_CNT = (Uint16 *)(0x40000000 + 0x1400 + 0x24);
STR_ECATPOSSYNCVAR STR_EcatPosSync;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void Sync0Time7(void);


/*******************************************************************************
  函数名: void InitTim7(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void InitTim7(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; //定义TIM10同步帧周期计时器的结构体变量
    NVIC_InitTypeDef           NVIC_InitStructure_main;    //定义TIM1中断的中断控制器的结构体变量

    DINT;
    //定时器配置
    TIM_DeInit(TIM7);    //复位定时器5
    //系统时钟  1MHz
    TIM_TimeBaseStructure.TIM_Prescaler = 83;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = (STR_FUNC_Gvar.System.HalfPosPrd<<1) - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    TIM_Cmd(TIM7, ENABLE);   // TIM10 counter enable

	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);        // 使能定时器更新中断 

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   //优先组配置为1位主，可抢占性;3位亚优先级，非强占性

    //配置TIM2的中断优先级
    NVIC_InitStructure_main.NVIC_IRQChannel = TIM7_IRQn;       //更新事件中断
    NVIC_InitStructure_main.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure_main.NVIC_IRQChannelSubPriority = 2;       //0~7
    NVIC_InitStructure_main.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure_main);

    EINT;

    /*TIM_DeInit(TIM9);   //复位定时器11
    //函数运算时间计算用计数器  
    TIM_TimeBaseStructure.TIM_Prescaler =  167;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

    TIM_Cmd(TIM9, ENABLE);   // TIM9 counter enable*/


    
}
/*******************************************************************************
  函数名: void InitTim7IRQ(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void InitTim7IRQ(void)
{
    
	
}
/*******************************************************************************
  函数名: void Sync0Time7(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void Sync0Time7(void)
{
   TIM_SetCounter(TIM7,STR_FUNC_Gvar.System.HalfPosPrd);                        //使能定时器
   
}
/*******************************************************************************
  函数名: void TIM7_IRQHandler(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void TIM7_IRQHandler(void)
{
    Uint16 PosIntTimeTest = 0;                    //位置环软中断运行时间测试变量
    
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);       //Clear TIM2 update interrupt
        
	    if(STR_EcatPosSync.PosSyncModeConfigDone ==0)return;
                
        GPIO_Disable_POS();
        
        PosIntTimeTest = GetSysTime_1MHzClk();        //位置环软中断运行时间测试
    
        FUNC_PostionControl_PosInterrupt();
        MTR_PostionControl_PosInterrupt();
		GPIO_Enable_POS();

        /*位置环软中断时间测试*/
        STR_PUB_Gvar.PosInterrupt_PRTime = GetSysTime_1MHzClk() - PosIntTimeTest;   //位置环软中断运行时间测试
        STR_PUB_Gvar.PosInterrupt_PRTime = STR_PUB_Gvar.PosInterrupt_PRTime & 0xFFFF;
	}
}
/*******************************************************************************
  函数名: void InitTim7IRQ(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void Sync0PosControlSet(void)
{
    if(STR_ECTCSPVar.SYNCPeriodRatio>=1)//sync0中断周期小于位置环周期，则不接收位置指令
    {
	    WriteCSPFIFO();
    }
    
    ECTInteruptUpdate();

    if(STR_EcatPosSync.PosSyncModeConfigDone == 1)Sync0Time7();//启动定时器
}

/*******************************************************************************
  函数名: void InitTim7IRQ(void)
  输入:   无 
  输出:   
  子函数: 无       
  描述:   
********************************************************************************/ 
void PosSyncCheck(void)
{
    if((2 != FunCodeUnion.code.CM_FpgaSyncModeSel)
        ||(STR_ECTCSPVar.SYNCPeriodRatio==0)
        ||(EcatSync.CarFreqDivSyncFreq==0)
        ||(STR_FUNC_Gvar.MonitorFlag.bit.ESMState!=1)
        )
    {
        STR_EcatPosSync.PosSyncModeConfigDone = 0;
        return;//不用切换
    }
    
    STR_EcatPosSync.PosSyncModeConfigDone = 1;
}
/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
/*void PosErrCalibration(void)
{
    int32 Temp32 = 0;
    static Uint16 TIMlast = 0;
    
    Temp32 = (int32)(Uint32)TIM_GetCounter(TIM9);

    STR_FUNC_Gvar.PosCtrl.PosAmplifErrCalibration = STR_FUNC_Gvar.PosCtrl.PosFdb - STR_InnerGvarPosCtrl.PosFdb_ToqInt * Temp32 * 10/625L; 

    TIMlast = TIM_GetCounter(TIM9);

    TIM_SetCounter(TIM9,0);

}*/



/********************************* END OF FILE *********************************/
