/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: Ecathw.h
  * @author    		:
  * @Description    :
  ******************************************************************************
  */
/*******************************************************************************
  * 修改       		：ytl
  * 版本	  		：V1.00
  * 时间	   		：
  * 说明 	  		：
********************************************************************************/


/*-----------------------------------------------------------------------------------------
------
------    includes
------
-----------------------------------------------------------------------------------------*/

#include "ecat_def.h"

#include  "esc.h"

#include "StmEcatHW.h"

/*-----------------------------------------------------------------------------------------
------
------    global defines and types
------
---------------------------------------------------------------------------------------*/

#define ESC_RD                    0x02            ///< read access to ESC
#define ESC_WR                    0x04            ///< write access to ESC

///////////////////////////////////////////////////////////////
//
//    Microcontroller definitions
//


/*-----------------------------------------------------------------------------------------
------
------    hardware timer settings
------
-----------------------------------------------------------------------------------------*/

#define ECAT_TIMER_INC_P_MS               1000// 0x271    //625 ticks/ms



#if _EcatHW_
    #define PROTO
#else
    #define PROTO extern
#endif

/*-----------------------------------------------------------------------------------------
------
------    global functions/macros
------
-----------------------------------------------------------------------------------------*/


#define    DISABLE_AL_EVENT_INT     	 	NVIC->ICER[0] = 0x400;      
#define    ENABLE_AL_EVENT_INT     	 	  NVIC->ISER[0] = 0x400;

#define    DISABLE_SYNC_EVENT_INT     	   NVIC->ICER[0] = 0x100;     
#define    ENABLE_SYNC_EVENT_INT     	  NVIC->ISER[0] = 0x100;

#define    READ_EEPROM_LOADED			 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11);


PROTO UINT8 HW_Init(void);
PROTO void HW_Release(void);
PROTO UINT16 HW_GetALEventRegister(void);


PROTO void HW_ResetALEventMask(UINT16 intMask);
PROTO void HW_SetALEventMask(UINT16 intMask);



#define HW_EscReadWord(Data, Address)  C_ReadBuffer(((MEM_ADDR *)&(Data)),((UINT16)(Address)),1)
#define HW_EscReadDWord(DWordValue, Address) C_ReadBuffer(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),2)
#define HW_EscReadMbxMem(pData,Address,Len) C_ReadBuffer(((MEM_ADDR *)(pData)),((UINT16)(Address)),((UINT16)(Len)>>1))



#define HW_EscWriteWord(Data, Address)  C_WriteBuffer(((MEM_ADDR *)&(Data)),((UINT16)(Address)),1)
#define HW_EscWriteDWord(DWordValue, Address) C_WriteBuffer(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),2)
#define HW_EscWriteMbxMem(pData,Address,Len) C_WriteBuffer(((MEM_ADDR *)(pData)),((UINT16)(Address)),((UINT16)(Len)>>1))



PROTO void HW_DisableSyncManChannel(UINT8 channel);
PROTO void HW_EnableSyncManChannel(UINT8 channel);
PROTO TSYNCMAN ESCMEM *HW_GetSyncMan(UINT8 channel);

#undef    PROTO




