/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: Ecatappl.h
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



#ifndef _ECATAPPL_H_
#define _ECATAPPL_H_

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/

#include "ecat_def.h"


/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/
/*Set to unsigned short to handle bit entries correct*/
#define     BoolTypeEAN(x)                  unsigned (x):1
#define     BIT1(x)                     unsigned short(x):1
#define     BIT2(x)                     unsigned short(x):2
#define     BIT3(x)                     unsigned short(x):3
#define     BIT4(x)                     unsigned short(x):4
#define     BIT5(x)                     unsigned short(x):5
#define     BIT6(x)                     unsigned short(x):6
#define     BIT7(x)                     unsigned short(x):7
#define     BIT8(x)                     unsigned short(x):8
#define     ALIGN0(x)
#define     ALIGN1(x)                   unsigned short(x):1;
#define     ALIGN2(x)                   unsigned short(x):2;
#define     ALIGN3(x)                   unsigned short(x):3;
#define     ALIGN4(x)                   unsigned short(x):4;
#define     ALIGN5(x)                   unsigned short(x):5;
#define     ALIGN6(x)                   unsigned short(x):6;
#define     ALIGN7(x)                   unsigned short(x):7;
#define     ALIGN8(x)                   unsigned short(x):8;
#define     ALIGN9(x)                   unsigned short x1:1; unsigned short(x):8;
#define     ALIGN10(x)                  unsigned short x1:2; unsigned short(x):8;
#define     ALIGN11(x)                  unsigned short x1:3; unsigned short(x):8;
#define     ALIGN12(x)                  unsigned short x1:4; unsigned short(x):8;
#define     ALIGN13(x)                  unsigned short x1:5; unsigned short(x):8;
#define     ALIGN14(x)                  unsigned short x1:6; unsigned short(x):8;
#define     ALIGN15(x)                  unsigned short x1:7; unsigned short(x):8;


/*-----------------------------------------------------------------------------------------
------
------    type definitions
------
-----------------------------------------------------------------------------------------*/

#endif //_ECATAPPL_H_

/*-----------------------------------------------------------------------------------------
------
------    global variables
------
-----------------------------------------------------------------------------------------*/

#if _ECATAPPL_
    #define PROTO
#else
    #define PROTO extern
#endif


PROTO BoolType bEcatWaitForInputUpdate;
PROTO BoolType bEtherCATRunLed;
PROTO BoolType bEtherCATErrorLed;
PROTO BoolType bRunApplication;

PROTO UINT32 cntIRQ;
PROTO UINT32 cntSYNC;
PROTO UINT8  cntFlag;


/*-----------------------------------------------------------------------------------------
------
------    global functions
------
-----------------------------------------------------------------------------------------*/

PROTO UINT16 MainInit(void);
//PROTO void EcatMainLoop(UINT16 SyncErrLimit, UINT16* StationAdd, UINT16  StationAlias );
//PROTO void EcatMainLoop(UINT16 SyncErrLimit,UINT16* StationAdd,UINT16 StationAlias,UINT8 Mode);
//PROTO void EcatMainLoop(UINT16 SyncErrLimit);

PROTO void EcatMainLoop(UINT16 SyncErrLimit,UINT16* StationAdd,UINT16 StationAlias,UINT16* pAlias,UINT8 Mode);


PROTO    void       ECAT_CheckTimer(void);
PROTO    void       PDI_Isr(void);
PROTO    void       Sync0_Isr(void);

PROTO    void       ECAT_Application(void);
PROTO    void       PDO_ResetOutputs(void);
PROTO    void       PDO_ReadInputs(void);
PROTO    void       PDO_InputMapping(void);

PROTO    void       CalcSMCycleTime(void);


#undef PROTO


