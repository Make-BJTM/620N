/*
  ***************(C) COPYRIGHT 2014 Inovance Technology Co., Ltd*********************

  * @file	   		: StmEcatHw.c
  * @author    		:
  * @Description    :
  *******************************************************************************
  */
/*******************************************************************************
  * 修改       		：ytl
  * 版本	  		：V1.00
  * 时间	   		：
  * 说明 	  		：
********************************************************************************/

/*-------------------------------------------------------------------------------
------
------    Includes
------
-------------------------------------------------------------------------------*/

#include "StmEcatHW.h"
//#include "ecatappl.h"
#include "ECT_InterFace.h"
#include "PUB_GlobalPrototypes.h"
#include "PUB_Main.h"
#include "ECT_PP.h"
#include "FUNC_CSPCmdBuffer.h"

/*-------------------------------------------------------------------------------
------
------    functions  state
------
-------------------------------------------------------------------------------*/
void SYNC0_Cfg(void);
void IRQ_Cfg(void);
void BaseTime_Cfg(void);


volatile unsigned short *ECAT_ADDR    = (unsigned short *)(0x60000000 + (0x8000 << 1));


void C_WriteBuffer(uint16_t* pBuffer, uint16_t WriteAddr, uint16_t NumHalfwordToWrite)
{
	for(; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /*!< while there is data to write */
	{

		/*!< Transfer data to the memory */
		
		*(ECAT_ADDR + WriteAddr) = *pBuffer++;

		/*!< Increment the address*/	
		WriteAddr += 2;
	}  
}


void C_ReadBuffer(uint16_t* pBuffer, uint16_t ReadAddr, uint16_t NumHalfwordToRead)
{
  for(; NumHalfwordToRead != 0; NumHalfwordToRead--) /*!< while there is data to read */
  {
	/*!< Read a half-word from the memory */
	*pBuffer++ = *(ECAT_ADDR + ReadAddr);

	  /*!< Increment the address*/	
	 ReadAddr += 2;
  }  
}

void C_WriteBufferIsr(uint16_t* pBuffer, uint16_t WriteAddr, uint16_t NumHalfwordToWrite)
{
	for(; NumHalfwordToWrite != 0; NumHalfwordToWrite--) /*!< while there is data to write */
	{

		/*!< Transfer data to the memory */
		
		*(ECAT_ADDR + WriteAddr) = *pBuffer++;

		/*!< Increment the address*/	
		WriteAddr += 2;
	}  
}


void C_ReadBufferIsr(uint16_t* pBuffer, uint16_t ReadAddr, uint16_t NumHalfwordToRead)
{
  for(; NumHalfwordToRead != 0; NumHalfwordToRead--) /*!< while there is data to read */
  {
	/*!< Read a half-word from the memory */
	*pBuffer++ = *(ECAT_ADDR + ReadAddr);

	  /*!< Increment the address*/	
	 ReadAddr += 2;
  }  
}


void SYNC0_Cfg(void)
{
	EXTI_InitTypeDef	 EXTI_InitStructure;
	GPIO_InitTypeDef	 GPIO_InitStructure;
	NVIC_InitTypeDef	 NVIC_InitStructure;

	/* Enable GPIOA clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure PA0 pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;//
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line0 to PA0 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);

	/* Configure EXTI Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI2_IRQ_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTI2_IRQ_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void IRQ_Cfg(void)
{
	EXTI_InitTypeDef	 EXTI_InitStructure;
	GPIO_InitTypeDef	 GPIO_InitStructure;
	NVIC_InitTypeDef	 NVIC_InitStructure;

	/* Enable GPIOA clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure PA0 pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line0 to PA0 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);

	/* Configure EXTI Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI4_IRQ_PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EXTI4_IRQ_SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}




void BaseTime_Cfg(void)
{

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd(BaseTime_APB, ENABLE);

	/* ---------------------------------------------------------------
	TIM5 counter clock at 1 MHz
	--------------------------------------------------------------- */
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(BaseTime_TIMx, &TIM_TimeBaseStructure);
	
	/* Prescaler configuration */
	TIM_PrescalerConfig(BaseTime_TIMx, 83, TIM_PSCReloadMode_Immediate);


	/* TIM5 enable counter */
	TIM_Cmd(BaseTime_TIMx, ENABLE);


}



void TestGPIO(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	/* Configure PG6 and PG8 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//GPIO_SetBits(GPIOC,GPIO_Pin_11);

}

void ECAT_HW_Init(void)
{
	//BaseTime_Cfg();
	TestGPIO();
	SYNC0_Cfg();
	IRQ_Cfg();
}

/**
  * @brief  This function handles EXTI2 IRQ Handler.
  * @param  None
  * @retval None
  */

void EXTI2_IRQHandler(void)
{
    static Uint16 SYNCIntScheduleTime = 0;     //SYNC中断调度时间测试变量
    Uint16 SYNCIntTimeTest = 0;                    //SYNC中断运行时间测试变量
    
    /*SYNC中断时间测试*/
    SYNCIntTimeTest = GetSysTime_1MHzClk();        //SYNC中断调度时间测试
    STR_PUB_Gvar.SYNCInterrupt_PSTime = SYNCIntTimeTest - SYNCIntScheduleTime;
    STR_PUB_Gvar.SYNCInterrupt_PSTime = STR_PUB_Gvar.SYNCInterrupt_PSTime & 0xFFFF;
	
    STR_PUB_Gvar.SYNC2IRQ_DeltaTime = (int32)SYNCIntTimeTest - (int32)STR_PUB_Gvar.IRQ_TriggerTime;
	
	if(STR_PUB_Gvar.SYNC2IRQ_DeltaTime>32768)
	{
		STR_PUB_Gvar.SYNC2IRQ_DeltaTime = STR_PUB_Gvar.SYNC2IRQ_DeltaTime-65536;
	}
	else if(STR_PUB_Gvar.SYNC2IRQ_DeltaTime<-32768)
	{
		STR_PUB_Gvar.SYNC2IRQ_DeltaTime = STR_PUB_Gvar.SYNC2IRQ_DeltaTime+65536;
	}
    SYNCIntScheduleTime = SYNCIntTimeTest;

    if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		// Clear the EXTI line 4 pending bit
		EXTI_ClearITPendingBit(EXTI_Line2);
		
		RdCSPBufferSync();

		Sync0PosControlSet();
		Sync0_Isr();//SAFEOP时也可进行数据反馈
    }

    /*SYNC中断时间测试*/
    STR_PUB_Gvar.SYNCInterrupt_PRTime = GetSysTime_1MHzClk() - SYNCIntTimeTest;   //SYNC中断运行时间测试
    STR_PUB_Gvar.SYNCInterrupt_PRTime = STR_PUB_Gvar.SYNCInterrupt_PRTime & 0xFFFF;
}
/**
  * @brief  This function handles EXTI4 IRQ Handler.
  * @param  None
  * @retval None
  */
	

void EXTI4_IRQHandler(void)
{
    static Uint16 IRQIntScheduleTime = 0;     //IRQ中断调度时间测试变量
    Uint16 IRQIntTimeTest = 0;                    //IRQ中断运行时间测试变量
    
    /*IRQ中断时间测试*/
    IRQIntTimeTest = GetSysTime_1MHzClk();        //IRQ中断调度时间测试
    STR_PUB_Gvar.IRQInterrupt_PSTime = IRQIntTimeTest - IRQIntScheduleTime;
    STR_PUB_Gvar.IRQ_TriggerTime = IRQIntTimeTest;
    STR_PUB_Gvar.IRQInterrupt_PSTime = STR_PUB_Gvar.IRQInterrupt_PSTime & 0xFFFF;
    IRQIntScheduleTime = IRQIntTimeTest;

    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		/* Clear the EXTI line 4 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line4);
		
		PDI_Isr();
		
		WrCSPBufferIrq();
	}
    
    /*IRQ中断时间测试*/
    STR_PUB_Gvar.IRQInterrupt_PRTime = GetSysTime_1MHzClk() - IRQIntTimeTest;   //IRQ中断运行时间测试
    STR_PUB_Gvar.IRQInterrupt_PRTime = STR_PUB_Gvar.IRQInterrupt_PRTime & 0xFFFF;

    
}
