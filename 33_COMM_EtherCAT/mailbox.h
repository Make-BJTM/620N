/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: Mailbox.h
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


#ifndef _MAILBOX_H_
#define _MAILBOX_H_

/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/

#include "esc.h"
#include "ecatslv.h"

/*-----------------------------------------------------------------------------------------
------
------    Defines and Types
------
-----------------------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////
//
//    General
//
#define     MBX_TYPE_AOE                        1
#define     MBX_TYPE_EOE                        2
#define     MBX_TYPE_COE                        3
#define     MBX_TYPE_FOE                        4
#define     MBX_TYPE_SOE                        5
#define     MBX_TYPE_VOE                        15

#define     EMCY_SERVICE                        ((UINT8) 0x0001)
#define     COE_SERVICE                         ((UINT8) 0x0002)
#define     SOE_SERVICE                         ((UINT8) 0x0004)
#define     EOE_SERVICE                         ((UINT8) 0x0008)
#define     AOE_SERVICE                         ((UINT8) 0x0010)
#define     VOE_SERVICE                         ((UINT8) 0x0020)
#define     FOE_SERVICE                         ((UINT8) 0x0040)
#define     FRAGMENTS_FOLLOW                    ((UINT8) 0x0080)

#ifndef DISABLE_MBX_INT
    #define    DISABLE_MBX_INT
#endif
#ifndef ENABLE_MBX_INT
    #define    ENABLE_MBX_INT
#endif

#ifndef    ENTER_MBX_CRITICAL
    #define    ENTER_MBX_CRITICAL
#endif

#ifndef    LEAVE_MBX_CRITICAL
    #define    LEAVE_MBX_CRITICAL
#endif

#ifndef    MAX_MBX_QUEUE_SIZE
    #define    MAX_MBX_QUEUE_SIZE    10
#endif

///////////////////////////////////////////////////////////////
//
//    Command Codes for the mailbox type 0
//

#define    MBXSERVICE_MBXERRORCMD          0x01

///////////////////////////////////////////////////////////////
//
//    Error Codes for a mailbox error response
//

#define    MBXERR_SYNTAX                   0x01
#define    MBXERR_UNSUPPORTEDPROTOCOL      0x02
#define    MBXERR_INVALIDCHANNEL           0x03
#define    MBXERR_SERVICENOTSUPPORTED      0x04
#define    MBXERR_INVALIDHEADER            0x05
#define    MBXERR_SIZETOOSHORT             0x06
#define    MBXERR_NOMOREMEMORY             0x07
#define    MBXERR_INVALIDSIZE              0x08
#define    MBXERR_SERVICEINWORK            0x09

/*---------------------------------------------------------------------------------
------
------    Data Types
------
---------------------------------------------------------------------------------*/
typedef struct MBX_STRUCT_PACKED_START
{
    UINT16                          Length;
    UINT16                          Address;

    UINT8                           Flags[2];
    #define    MBX_OFFS_TYPE        1
    #define    MBX_OFFS_COUNTER     1
    #define    MBX_MASK_TYPE        0x0F
    #define    MBX_MASK_COUNTER     0xF0
    #define    MBX_SHIFT_TYPE       0
    #define    MBX_SHIFT_COUNTER    4
}MBX_STRUCT_PACKED_END
TMBXHEADER;

#define     MBX_HEADER_SIZE         SIZEOF(TMBXHEADER)

#define     MAX_MBX_DATA_SIZE       (MAX_MBX_SIZE - MBX_HEADER_SIZE)

typedef struct MBX_STRUCT_PACKED_START
{
    TMBXHEADER                      MbxHeader;
    UINT16                          Data[(MAX_MBX_DATA_SIZE >> 1)];
}MBX_STRUCT_PACKED_END
TMBX;

#define MBX_BUFFER_SIZE     (MBX_HEADER_SIZE + MAX_MBX_DATA_SIZE )

typedef struct
{
    UINT16    firstInQueue;
    UINT16    lastInQueue;
    UINT16    maxQueueSize;
    TMBX MBXMEM * queue[(MAX_MBX_QUEUE_SIZE)+1];
} TMBXQUEUE;





#endif //_MAILBOX_H_

/*-----------------------------------------------------------------------------------------
------
------    global variables
------
-----------------------------------------------------------------------------------------*/

#if _MAILBOX_
    #define PROTO
#else
    #define PROTO extern
#endif

PROTO BoolType                    bReceiveMbxIsLocked;
PROTO BoolType                    bSendMbxIsFull;
PROTO BoolType                    bMbxRunning;
PROTO BoolType                    bMbxRepeatToggle;
PROTO UINT16                  u16SendMbxSize;
PROTO UINT16                  u16ReceiveMbxSize;
PROTO UINT16                  u16EscAddrReceiveMbx;
PROTO UINT16                  u16EscAddrSendMbx;
PROTO UINT8                   u8MbxWriteCounter;
PROTO UINT8                   u8MbxReadCounter;
PROTO TMBX MBXMEM             asMbx[2];
PROTO UINT8                   u8MailboxSendReqStored;
PROTO TMBX MBXMEM *           psWriteMbx;
PROTO TMBX MBXMEM *           psReadMbx;
PROTO TMBX MBXMEM *           psRepeatMbx;
PROTO TMBX MBXMEM *           psStoreMbx;
PROTO TMBXQUEUE MBXMEM        sMbxSendQueue;
PROTO TMBXQUEUE MBXMEM        sMbxReceiveQueue;

/*-----------------------------------------------------------------------------------------
------
------    global functions
------
-----------------------------------------------------------------------------------------*/

PROTO   void     MBX_Init(void);
PROTO   UINT8    MBX_StartMailboxHandler(void);
PROTO   void     MBX_StopMailboxHandler(void);
PROTO   void     MBX_MailboxWriteInd(TMBX MBXMEM *pMbx);
PROTO   void     MBX_MailboxReadInd(void);
PROTO   void     MBX_MailboxRepeatReq(void);
PROTO   UINT8    MBX_MailboxSendReq(TMBX MBXMEM * pMbx, UINT8 flags);
PROTO   void     MBX_CheckAndCopyMailbox(void);
PROTO   UINT8    MBX_CopyToSendMailbox(TMBX MBXMEM *pMbx);
PROTO   void     MBX_Main(void);

#undef PROTO



