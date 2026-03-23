/*
  ***************(C) COPYRIGHT 2014 Inovance Technology Co., Ltd*********************

  * @file	   		: StmEcatHw.h
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



#ifndef __StmEcatHW_H
#define __StmEcatHW_H



/*-------------------------------------------------------------------------------
------
------    Includes
------
-------------------------------------------------------------------------------*/

#include "stm32f4xx.h"


/*-------------------------------------------------------------------------------
------
------	  internal Types and Defines
------
-------------------------------------------------------------------------- ----*/

#define 	GetBaseTimer			 			TIM5->CNT
#define 	ClearBaseTimer						TIM5->CNT = 0

#define 	BaseTime_APB						RCC_APB1Periph_TIM5
#define 	BaseTime_TIMx						TIM5


/*-------------------------------------------------------------------------------
------
------    functions  state
------
-------------------------------------------------------------------------------*/


extern void C_ReadBuffer(uint16_t* pBuffer, uint16_t ReadAddr, uint16_t NumHalfwordToRead);
extern void C_WriteBuffer(uint16_t* pBuffer, uint16_t WriteAddr, uint16_t NumHalfwordToWrite);
extern void C_ReadBufferIsr(uint16_t* pBuffer, uint16_t ReadAddr, uint16_t NumHalfwordToRead);
extern void C_WriteBufferIsr(uint16_t* pBuffer, uint16_t WriteAddr, uint16_t NumHalfwordToWrite);

extern void ECAT_HW_Init(void); 


#if 0
extern uint32_t HW_GetTimer(void);
extern void HW_ClearTimer(void);
#endif



#endif /* __StmEcatHW_H  */

