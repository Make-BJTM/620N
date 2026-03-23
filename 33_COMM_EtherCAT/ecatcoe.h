/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: Ecatcoe.h
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


  
#ifndef _ECATCOE_H_

#define _ECATCOE_H_

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/

#include "mailbox.h"

/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/

/*/////////////////////////////////////////////////////////////////////////////////////////
//
// Error Codes
*/

#define     ERROR_COEINVALIDSERVICE     0x01
#define     ERROR_COENOTSUPPORTED       0x02

/*/////////////////////////////////////////////////////////////////////////////////////////
//
// COE services
*/

#define     COESERVICE_EMERGENCY        0x01
#define     COESERVICE_SDOREQUEST       0x02
#define     COESERVICE_SDORESPONSE      0x03
#define     COESERVICE_TXPDO            0x04
#define     COESERVICE_RXPDO            0x05
#define     COESERVICE_TXPDOREMREQ      0x06
#define     COESERVICE_RXPDOREMREQ      0x07
#define     COESERVICE_SDOINFO          0x08

/*/////////////////////////////////////////////////////////////////////////////////////////
//
// COE Structures
*/

typedef UINT16 TCOEHEADER;
#define     COEHEADER_COESERVICESHIFT   12
#define     COEHEADER_COESERVICEMASK    0xF000

#define     COE_HEADER_SIZE             2

typedef struct MBX_STRUCT_PACKED_START
{
  TMBXHEADER        MbxHeader;
  TCOEHEADER        CoeHeader;
  UINT16            Data[((MAX_MBX_DATA_SIZE)-(COE_HEADER_SIZE)) >> 1];
}MBX_STRUCT_PACKED_END
TCOEMBX;

/*-----------------------------------------------------------------------------------------
------
------    global variables
------
-----------------------------------------------------------------------------------------*/

#if _ECATCOE_
    #define PROTO
#else
    #define PROTO extern
#endif

PROTO    TMBX MBXMEM * VARMEM pCoeSendStored;                /* 如果无法发送的邮箱服务(或者存),
                                                                COE服务将存储在这个变量
                                                                和将自动发送到的邮箱处理程序
                                                                (COE_ContinueInd)  当读下一个主站发来的邮件数据时调用*/

/*-----------------------------------------------------------------------------------------
------
------    global functions
------
-----------------------------------------------------------------------------------------*/

PROTO   void     COE_Init(void);
PROTO   UINT8    COE_ServiceInd(TCOEMBX MBXMEM *pCoeMbx);
PROTO   void     COE_ContinueInd(TMBX MBXMEM * pMbx);

#undef PROTO

#endif //_ECATCOE_H_


