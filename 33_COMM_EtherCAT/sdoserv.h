/*
 * ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************
 *
 * @file	                : Sdoserv.h
 * @author              :
 * @Description    :
 ******************************************************************************
 */


/*******************************************************************************
 * 修改                 ：ytl
 * 版本	                ：V1.00
 * 时间	                ：
 * 说明                         ：
 ********************************************************************************/

#ifndef _SDOSERV_H_
#define _SDOSERV_H_


/*-----------------------------------------------------------------------------------------
*  ------
*  ------    Includes
*  ------
*  -----------------------------------------------------------------------------------------*/

#include "ecatcoe.h"


/*-----------------------------------------------------------------------------------------
*  ------
*  ------    Defines and Types
*  ------
*  -----------------------------------------------------------------------------------------*/


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // Error Codes
 */

#define     ERROR_SDOINVALIDCOMMAND	0x1101
#define     ERROR_SDOINVALIDHEADER	0x1102
#define     ERROR_SDONOTSUPPORTED	0x1103


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // SDO services
 */

#define     SDOHEADER_SIZEINDICATOR		( (UINT8)0x01)
#define     SDOHEADER_TRANSFERTYPE		( (UINT8)0x02)
#define     SDOHEADER_DATASETSIZE		( (UINT8)0x0C)
#define     SDOHEADER_COMPLETEACCESS		( (UINT8)0x10)
#define     SDOHEADER_COMMAND			( (UINT8)0xE0)
#define     SDOHEADERSHIFT_SIZEINDICATOR	( (UINT8)0)
#define     SDOHEADERSHIFT_TRANSFERTYPE		( (UINT8)1)
#define     SDOHEADERSHIFT_DATASETSIZE		( (UINT8)2)
#define     SDOHEADERSHIFT_INDEXACCESS		( (UINT8)4)
#define     SDOHEADERSHIFT_COMMAND		( (UINT8)5)
#define     SDOSERVICE_SIZEINDICATOR		( (UINT8)0x01)
#define     SDOSERVICE_TRANSFERTYPE		( (UINT8)0x02)
#define     SDOSERVICE_DATASETSIZESHIFT		( (UINT8)2)
#define     SDOSERVICE_COMMANDSHIFT		( (UINT8)5)
#define     SDOSERVICE_INITIATEDOWNLOADREQ	( (UINT8)(0x01 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_INITIATEDOWNLOADRES	( (UINT8)(0x03 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_DOWNLOADSEGMENTREQ	( (UINT8)(0x00 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_DOWNLOADSEGMENTRES	( (UINT8)(0x01 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_INITIATEUPLOADREQ	( (UINT8)(0x02 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_INITIATEUPLOADRES	( (UINT8)(0x02 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_UPLOADSEGMENTREQ		( (UINT8)(0x03 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_UPLOADSEGMENTRES		( (UINT8)(0x00 << (SDOHEADERSHIFT_COMMAND) ) )
#define     SDOSERVICE_ABORTTRANSFER		( (UINT8)( ( (UINT8)0x04) << (SDOHEADERSHIFT_COMMAND) ) )

/* ///////////////////////////////////////////////////////////////////////////////////////// */


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // SDO structures
 */

/*
 * /////////////////////////////////////////////////////////////////////////////////////////
 * BASIC structures:
 */

/* initial service header: */

#define     SDOHEADER_COMMANDOFFSET	0
#define     SDOHEADER_INDEXLOOFFSET	1
#define     SDOHEADER_INDEXHIOFFSET	2
#define     SDOHEADER_SUBINDEXOFFSET	3
#define     SDOHEADER_COMMANDMASK	0xFF
#define     SDOHEADER_INDEXLOSHIFT	0
#define     SDOHEADER_INDEXHIMASK	0xFF
#define     SDOHEADER_SUBINDEXSHIFT	0

typedef struct MBX_STRUCT_PACKED_START
{
	UINT8 Sdo[4];
}MBX_STRUCT_PACKED_END
TINITSDOHEADER;

#define INITSDO_HEADER_SIZE SIZEOF(TINITSDOHEADER)

/* complete initial service header: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
}MBX_STRUCT_PACKED_END
TINITSDOMBX;

#define     MAX_EXPEDITED_DATA		4
#define     MIN_SEGMENTED_DATA		( (UINT16)7)
#define     EXPEDITED_FRAME_SIZE	(COE_HEADER_SIZE + INITSDO_HEADER_SIZE + (MAX_EXPEDITED_DATA) )
#define     DOWNLOAD_NORM_REQ_SIZE	(COE_HEADER_SIZE + INITSDO_HEADER_SIZE + 4)
/* HBu 06.02.06: names of defines changed */
/* HBu 21.03.06: the SDO_Download-Response has to have always 8 bytes */
#define     DOWNLOAD_NORM_RES_SIZE	(COE_HEADER_SIZE + INITSDO_HEADER_SIZE + 4)
#define     UPLOAD_NORM_RES_SIZE	(COE_HEADER_SIZE + INITSDO_HEADER_SIZE + 4)
#define     SEGMENT_NORM_HEADER_SIZE	(COE_HEADER_SIZE + 1)
#define     SEGMENT_NORM_RES_SIZE	( (SEGMENT_NORM_HEADER_SIZE) + (MIN_SEGMENTED_DATA) )
#define     SEGMENT_MBX_SIZE		(MBX_HEADER_SIZE + (SEGMENT_NORM_HEADER_SIZE) )

#define    SEGHEADER_MASK		( (UINT8)0xFF)
#define    SEGHEADER_NOMOREFOLLOWS	( (UINT8)0x01)
#define    SEGHEADER_SEGDATASIZE	( (UINT8)0x0E)
#define    SEGHEADER_TOGGLE		( (UINT8)0x10)
#define    SEGHEADER_COMMAND		( (UINT8)0xE0)
#define    SEGHEADERSHIFT_SEGDATASIZE	( (UINT8)1)
#define    SEGHEADERSHIFT_TOGGLE	( (UINT8)4)
#define    SEGHEADERSHIFT_COMMAND	( (UINT8)5)

/* segmented service header with data: */
typedef struct MBX_STRUCT_PACKED_START
{
	UINT8	SegHeader;
	UINT8	Data[(MAX_MBX_DATA_SIZE) - (SEGMENT_NORM_HEADER_SIZE)];
}MBX_STRUCT_PACKED_END
TSDOSEGHEADERDATA;

/*
 * /////////////////////////////////////////////////////////////////////////////////////////
 * DOWNLOAD service structures:
 */

/* expedited download request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
	UINT16		Data[(MAX_EXPEDITED_DATA) >> 1];
}MBX_STRUCT_PACKED_END
TINITSDODOWNLOADEXPREQMBX;

/* normal download request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
	UINT16		CompleteSize[2];
	UINT16		Data[( ( (MAX_MBX_DATA_SIZE) - (DOWNLOAD_NORM_REQ_SIZE) ) >> 1)];
}MBX_STRUCT_PACKED_END
TINITSDODOWNLOADNORMREQMBX;

/* expedited and normal download response: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
}MBX_STRUCT_PACKED_END
TINITSDODOWNLOADRESMBX;

/* segmented download request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER		MbxHeader;
	TCOEHEADER		CoeHeader;
	TSDOSEGHEADERDATA	SdoHeader; /* data is included in header ! */
}MBX_STRUCT_PACKED_END
TDOWNLOADSDOSEGREQMBX;

/* segmented download response: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	UINT8		SegHeader;
}MBX_STRUCT_PACKED_END
TDOWNLOADSDOSEGRESMBX;

/*
 * /////////////////////////////////////////////////////////////////////////////////////////
 * UPLOAD service structures:
 */

/* expedited and normal upload request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
}MBX_STRUCT_PACKED_END
TINITSDOUPLOADREQMBX;


/* expedited upload response: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;      /* 6 bytes */
	TCOEHEADER	CoeHeader;      /* 2 bytes */
	TINITSDOHEADER	SdoHeader;      /* 4 bytes */
	UINT16		Data[( (MAX_EXPEDITED_DATA) >> 1)];
}MBX_STRUCT_PACKED_END
TINITSDOUPLOADEXPRESMBX;

/* normal upload response: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
	UINT16		CompleteSize[2];
	UINT16		Data[( ( (MAX_MBX_DATA_SIZE) - (UPLOAD_NORM_RES_SIZE) ) >> 1)];
}MBX_STRUCT_PACKED_END
TINITSDOUPLOADNORMRESMBX;

/* segmented upload request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	UINT8		SegHeader;
}MBX_STRUCT_PACKED_END
TUPLOADSDOSEGREQMBX;

/* segmented upload response: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER		MbxHeader;
	TCOEHEADER		CoeHeader;
	TSDOSEGHEADERDATA	SdoHeader; /* data is included in header ! */
}MBX_STRUCT_PACKED_END
TUPLOADSDOSEGRESMBX;

/*
 * /////////////////////////////////////////////////////////////////////////////////////////
 * ABORT service structure and defines:
 */

/* abort request: */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TINITSDOHEADER	SdoHeader;
	UINT32		AbortCode;
}MBX_STRUCT_PACKED_END
TABORTSDOTRANSFERREQMBX;

#define ABORT_NORM_RES_SIZE (SIZEOF(TABORTSDOTRANSFERREQMBX) - SIZEOF(TMBXHEADER) )


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // Abort codes
 */

#if 0
#define     ABORTIDX_TOGGLE_BIT_NOT_CHANGED				0x01
#define     ABORTIDX_SDO_PROTOCOL_TIMEOUT				0x02
#define     ABORTIDX_COMMAND_SPECIFIER_UNKNOWN			0x03
#define     ABORTIDX_OUT_OF_MEMORY						0x04
#define     ABORTIDX_UNSUPPORTED_ACCESS					0x05
#define     ABORTIDX_WRITE_ONLY_ENTRY					0x06
#define     ABORTIDX_READ_ONLY_ENTRY					0x07
#define     ABORTIDX_OBJECT_NOT_EXISTING				0x08
#define     ABORTIDX_OBJECT_CANT_BE_PDOMAPPED			0x09
#define     ABORTIDX_MAPPED_OBJECTS_EXCEED_PDO			0x0A
#define     ABORTIDX_PARAM_IS_INCOMPATIBLE				0x0B
#define     ABORTIDX_INTERNAL_DEVICE_INCOMPATIBILITY	0x0C
#define     ABORTIDX_HARDWARE_ERROR						0x0D
#define     ABORTIDX_PARAM_LENGTH_ERROR					0x0E
#define     ABORTIDX_PARAM_LENGTH_TOO_LONG				0x0F
#define     ABORTIDX_PARAM_LENGTH_TOO_SHORT				0x10
#define     ABORTIDX_SUBINDEX_NOT_EXISTING				0x11
#define     ABORTIDX_VALUE_EXCEEDED						0x12
#define     ABORTIDX_VALUE_TOO_GREAT					0x13
#define     ABORTIDX_VALUE_TOO_SMALL					0x14
#define     ABORTIDX_MAX_VALUE_IS_LESS_THAN_MIN_VALUE	0x15
#define     ABORTIDX_GENERAL_ERROR						0x16
#define     ABORTIDX_DATA_CANNOT_BE_READ_OR_STORED		0x17
#define     ABORTIDX_DATA_CANNOT_BE_ACCESSED_BECAUSE_OF_LOCAL_CONTROL	0x18
#define     ABORTIDX_IN_THIS_STATE_DATA_CANNOT_BE_READ_OR_STORED	0x19
#define     ABORTIDX_NO_OBJECT_DICTIONARY_IS_PRESENT			0x1A
#define     ABORTIDX_ENTRY_CANT_BE_WRITTEN_SI0_NOT_0			0x1B

#endif

#define     ABORTIDX_IN_THIS_STATE_DATA_CANNOT_BE_READ_OR_STORED	1	//0x08000022  由于当前设备状态导致数据不能传送或者保存到应用（密码错误）
#define     ABORTIDX_OBJECT_CANT_BE_PDOMAPPED						2	//0x06040041 对象不能够映射到PDO
#define     ABORTIDX_HARDWARE_ERROR									3	//0x06060000 硬件错误导致对象访问失败（Modbus的从站设备故障）
#define     ABORTIDX_OBJECT_NOT_EXISTING							4	//0x06020000 对象字典中的对象不存在（无效地址）
#define     ABORTIDX_VALUE_EXCEEDED									5	//0x06090030 超出参数的值范围（无效参数）
#define     ABORTIDX_DATA_CANNOT_BE_READ_OR_STORED					6	//0x08000020 数据不能传送或保存到应用（参数更改无效）
#define     ABORTIDX_DATA_CANNOT_BE_READ_OR_STORED_BECAUSE_OF_LOCAL_CONTROL	  7	 //0x08000021 由于本地控制导致数据不能传送保存到应用（系统锁定）
#define     ABORTIDX_GENERAL_ERROR									8	//0x08000000 一般性错误
#define     ABORTIDX_UNSUPPORTED_ACCESS								9	//0x06010000 对象不支持访问
#define     ABORTIDX_WRITE_ONLY_ENTRY								10	//0x06010001 试图读只写对象
#define     ABORTIDX_READ_ONLY_ENTRY								11	//0x06010002 试图写只读对象
#define     ABORTIDX_ENTRY_CANT_BE_WRITTEN_SI0_NOT_0				12	//0x06010003 写对象子索引时，必须先清索引0
#define     ABORTIDX_PARAM_LENGTH_ERROR								13	//0x06070010 数据类型不匹配，服务参数长度不匹配
#define     ABORTIDX_PARAM_LENGTH_TOO_LONG							14	//0x06070012 数据类型不匹配，服务参数长度太大
#define     ABORTIDX_PARAM_LENGTH_TOO_SHORT							15	//0x06070013 数据类型不匹配，服务参数长度太短
#define     ABORTIDX_SUBINDEX_NOT_EXISTING							16	//0x06090011 子索引不存在
#define     ABORTIDX_VALUE_TOO_GREAT								17	//0x06090031 写入参数数值太大
#define     ABORTIDX_VALUE_TOO_SMALL								18	//0x06090032 写入参数值太小
#define     ABORTIDX_MAX_VALUE_IS_LESS_THAN_MIN_VALUE				19	//0x06090036 最大值小于最小值
#define     ABORTIDX_NO_OBJECT_DICTIONARY_IS_PRESENT				20	//0x08000023 对象字典动态产生错误或对象字典不存在

#define     ABORTIDX_TOGGLE_BIT_NOT_CHANGED							21	//0x05030000 触发位没有交替改变
#define     ABORTIDX_SDO_PROTOCOL_TIMEOUT							22	//0x05040000 SDO协议超时
#define     ABORTIDX_COMMAND_SPECIFIER_UNKNOWN						23	//0x05040001 非法或未知的Client/Server 命令字
#define     ABORTIDX_OUT_OF_MEMORY									24	//0x05040005 内存溢出
#define     ABORTIDX_MAPPED_OBJECTS_EXCEED_PDO						25	//0x06040042 映射的对象的数目和长度超出PDO长度
#define     ABORTIDX_PARAM_IS_INCOMPATIBLE							26	//0x06040043 一般性参数不兼容
#define     ABORTIDX_INTERNAL_DEVICE_INCOMPATIBILITY				27	//0x06040047 一般性设备内部不兼容
#define     ABORTIDX_WORKING						0xFF



#define     ABORT_NOERROR									0x00000000
#define     ABORT_TOGGLE_BIT_NOT_CHANGED					0x05030000
#define     ABORT_SDO_PROTOCOL_TIMEOUT						0x05040000
#define     ABORT_COMMAND_SPECIFIER_UNKNOWN					0x05040001
#define     ABORT_OUT_OF_MEMORY								0x05040005
#define     ABORT_UNSUPPORTED_ACCESS						0x06010000
#define     ABORT_WRITE_ONLY_ENTRY							0x06010001
#define     ABORT_READ_ONLY_ENTRY							0x06010002
#define     ABORT_ENTRY_CANT_BE_WRITTEN_SI0_NOT_0			0x06010003
#define     ABORT_OBJECT_NOT_EXISTING						0x06020000
#define     ABORT_OBJECT_CANT_BE_PDOMAPPED					0x06040041
#define     ABORT_MAPPED_OBJECTS_EXCEED_PDO					0x06040042
#define     ABORT_PARAM_IS_INCOMPATIBLE						0x06040043
#define     ABORT_INTERNAL_DEVICE_INCOMPATIBILITY			0x06040047
#define     ABORT_HARDWARE_ERROR							0x06060000
#define     ABORT_PARAM_LENGTH_ERROR						0x06070010
#define     ABORT_PARAM_LENGTH_TOO_LONG						0x06070012
#define     ABORT_PARAM_LENGTH_TOO_SHORT					0x06070013
#define     ABORT_SUBINDEX_NOT_EXISTING						0x06090011
#define     ABORT_VALUE_EXCEEDED							0x06090030
#define     ABORT_VALUE_TOO_GREAT							0x06090031
#define     ABORT_VALUE_TOO_SMALL							0x06090032
#define     ABORT_MAX_VALUE_IS_LESS_THAN_MIN_VALUE			0x06090036
#define     ABORT_GENERAL_ERROR								0x08000000
#define     ABORT_DATA_CANNOT_BE_READ_OR_STORED					0x08000020
#define     ABORT_DATA_CANNOT_BE_READ_OR_STORED_BECAUSE_OF_LOCAL_CONTROL	0x08000021
#define     ABORT_DATA_CANNOT_BE_READ_OR_STORED_IN_THIS_STATE			0x08000022
#define     ABORT_NO_OBJECT_DICTIONARY_IS_PRESENT				0x08000023


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // SDO Information services
 */

#define    SDOINFOSERVICE_OBJDICTIONARYLIST_Q	0x01
#define    SDOINFOSERVICE_OBJDICTIONARYLIST_S	0x02
#define    SDOINFOSERVICE_OBJDESCRIPTION_Q	0x03
#define    SDOINFOSERVICE_OBJDESCRIPTION_S	0x04
#define    SDOINFOSERVICE_ENTRYDESCRIPTION_Q	0x05
#define    SDOINFOSERVICE_ENTRYDESCRIPTION_S	0x06
#define    SDOINFOSERVICE_ERROR_Q		0x07
#define    SDOINFOSERVICE_INCOMPLETE		0x80


/*/////////////////////////////////////////////////////////////////////////////////////////
 * //
 * // SDO Information service structures:
 */

#define    INFO_LIST_TYPE_LENGTH	0
#define    INFO_LIST_TYPE_ALL		1
#define    INFO_LIST_TYPE_RXPDO		2
#define    INFO_LIST_TYPE_TXPDO		3
#define    INFO_LIST_TYPE_BACKUP	4
#define    INFO_LIST_TYPE_SET		5
#define    INFO_LIST_TYPE_MAX		5


/* SDO Information / Object Dictionary Lists: */
typedef struct MBX_STRUCT_PACKED_START
{
	UINT16 ListType;
}MBX_STRUCT_PACKED_END
TSDOINFOLIST;

#define    OBJFLAGS_MAXSUBINDEXMASK	0x00FF          /* max subindex mask */
#define    OBJFLAGS_MAXSUBINDEXSHIFT	0               /* max subindex shift */
#define    OBJFLAGS_OBJCODEMASK		0x0F00          /* obj  code mask */
#define    OBJFLAGS_OBJCODESHIFT	8               /* obj code shift */

#define    OBJCODE_VAR	0x07
#define    OBJCODE_ARR	0x08
#define    OBJCODE_REC	0x09

typedef struct MBX_STRUCT_PACKED_START
{
	UINT16	DataType;                  /* refer to data type index */
	UINT16	ObjFlags;
	/*Object Code and MaxSubindex will be set manually in the object dictionary. The value is always created in little endian format*/

/*        char                Name[];                // rest of mailbox data */
}MBX_STRUCT_PACKED_END
TSDOINFOOBJDESC;

#define SDO_INFO_OBJ_DESC_SIZE SIZEOF(TSDOINFOOBJDESC)

typedef struct MBX_STRUCT_PACKED_START
{
	UINT16		Index;
	TSDOINFOOBJDESC Res;
}MBX_STRUCT_PACKED_END
TSDOINFOOBJ;

#define SDO_INFO_OBJ_DESC_RES_SIZE SIZEOF(TSDOINFOOBJ)

#define    ACCESS_READWRITE		0x003F
#define    ACCESS_READ			0x0007
#define    ACCESS_READ_PREOP		0x0001
#define    ACCESS_READ_SAFEOP		0x0002
#define    ACCESS_READ_OP		0x0004
#define    ACCESS_WRITE			0x0038
#define    ACCESS_WRITE_PREOP		0x0008
#define    ACCESS_WRITE_SAFEOP		0x0010
#define    ACCESS_WRITE_OP		0x0020
#define    OBJACCESS_NOPDOMAPPING	0x0000
#define    OBJACCESS_RXPDOMAPPING	0x0040
#define    OBJACCESS_TXPDOMAPPING	0x0080
#define    OBJACCESS_BACKUP		0x0100
#define    OBJACCESS_SETTINGS		0x0200
#define    OBJACCESS_SAFEINPUTS		0x0400
#define    OBJACCESS_SAFEOUTPUTS	0x0800
#define    OBJACCESS_SAFEPARAMETER	0x1000

/* SDO Information / Entry Description: */
typedef struct MBX_STRUCT_PACKED_START
{
	UINT16	DataType;                               /* refer to data type index */
	UINT16	BitLength;
	UINT16	ObjAccess;                              /* Bit 0: Read Access in Pre-Op */
	/*
	 * Bit 1: Read Access in Safe-Op
	 * Bit 2: Read Access in Op
	 * Bit 3: Write Access in Pre-Op
	 * Bit 4: Write Access in Safe-Op
	 * Bit 5: Write Access in Op
	 * Bit 6: mappable in RxPDO
	 * Bit 7: mappable in TxPDO
	 * Bit 8: entry will be included in backup
	 * Bit 9: entry will be included in settings
	 */

/*
 *        UINT16            UnitType;                // optional if bit3 of valueInfo
 *        UINT8                DefaultValue[];        // optional if bit4 of valueInfo
 *        UINT8                MinValue[];                // optional if bit5 of valueInfo
 *        UINT8                MaxValue[];                // optional if bit6 of valueInfo
 *        char                Desc[];                    // rest of mailbox data
 */
}MBX_STRUCT_PACKED_END
TSDOINFOENTRYDESC;

#define     ENTRY_MASK_SUBINDEX		0x00FF
#define     ENTRY_SUBINDEX_SHIFT	0
#define     ENTRY_MASK_VALUEINFO	0xFF00
#define     ENTRY_VALUEINFO_SHIFT	8

typedef struct MBX_STRUCT_PACKED_START
{
	UINT16 Index;

	UINT16 Info;

	TSDOINFOENTRYDESC Res;
}MBX_STRUCT_PACKED_END
TSDOINFOENTRY;


/* SDO Information / Error: */
typedef struct MBX_STRUCT_PACKED_START
{
	UINT32 ErrorCode;
/*    char                    ErrorText[]; */
}MBX_STRUCT_PACKED_END
TSDOINFOERROR;

#define SDO_INFO_ERROR_SIZE SIZEOF(TSDOINFOERROR)

#define     INFOHEAD_OPCODE_MASK	0x007F
#define     INFOHEAD_OPCODE_SHIFT	0
#define     INFOHEADER_INCOMPLETE_MASK	0x0080
#define     INFOHEADER_INCOMPLETE_SHIFT 0           /* the incomplete value "SDOINFOSERVICE_INCOMPLETE" is defined as a UINT8 => no shift required */

/* SDO Information / Header */
typedef struct MBX_STRUCT_PACKED_START
{
	UINT16 InfoHead;

	UINT16 FragmentsLeft;

	union MBX_STRUCT_PACKED_START
	{
		TSDOINFOLIST	List;
		TSDOINFOOBJ	Obj;
		TSDOINFOENTRY	Entry;
		TSDOINFOERROR	Error;
		UINT16		Data[1];
	}MBX_STRUCT_PACKED_END
		Data;
}MBX_STRUCT_PACKED_END
TSDOINFOHEADER;

/* SDO Information / complete structure */
typedef struct MBX_STRUCT_PACKED_START
{
	TMBXHEADER	MbxHeader;
	TCOEHEADER	CoeHeader;
	TSDOINFOHEADER	SdoHeader;
}MBX_STRUCT_PACKED_END
TSDOINFORMATION;

#define     SIZEOF_SDOINFOHEAD			4
#define     SIZEOF_SDOINFO			( (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) )
#define     SIZEOF_SDOINFOSTRUCT		( (MBX_HEADER_SIZE) + (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) )
#define     SIZEOF_SDOINFOLISTHEAD		2
#define     SIZEOF_SDOINFOLISTSTRUCT		( (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) + (SIZEOF_SDOINFOLISTHEAD) )
#define     SIZEOF_SDOINFOOBJSTRUCT		( (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) + (SDO_INFO_OBJ_DESC_RES_SIZE) )
#define     SIZEOF_SDOINFOENTRYREQHEAD		4
#define     SIZEOF_SDOINFOENTRYREQSTRUCT	( (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) + (SIZEOF_SDOINFOENTRYREQHEAD) )
#define     SIZEOF_SDOINFOERRORSTRUCT		( (COE_HEADER_SIZE) + (SIZEOF_SDOINFOHEAD) + (SDO_INFO_ERROR_SIZE) )


/*-----------------------------------------------------------------------------------------
*  ------
*  ------    global variables
*  ------
*  -----------------------------------------------------------------------------------------*/

#if _SDOSERV_
#define PROTO
#else
#define PROTO extern
#endif

#define     SDO_PENDING_WRITE		0x1
#define     SDO_PENDING_SEG_WRITE	0x2
#define     SDO_PENDING_READ		0x3
#define     SDO_PENDING_SEG_READ	0x4
PROTO UINT8		u8PendingSdo;
PROTO BoolType		bStoreCompleteAccess;
PROTO UINT8		u8StoreSubindex;
PROTO UINT16		u16StoreIndex;
PROTO UINT32		u32StoreDataSize;
PROTO UINT16 MBXMEM	*pStoreData;
PROTO UINT8		(* pSdoPendFunc)(UINT16 Index, UINT8 Subindex, UINT32 Size, UINT16 MBXMEM * pData, UINT8 bCompleteAccess);

PROTO UINT16 VARMEM * VARMEM pSdoSegData;

#define SDO_INFO_HEADER_BYTE_SIZE ( (SIZEOF_SDOINFOSTRUCT) + (SIZEOF_SDOINFOLISTHEAD) )
PROTO MEM_ADDR VARMEM	aSdoInfoHeader[GET_MEM_SIZE(SDO_INFO_HEADER_BYTE_SIZE)];
PROTO UINT16 VARMEM	nSdoInfoFragmentsLeft;


/*-----------------------------------------------------------------------------------------
*  ------
*  ------    global functions
*  ------
*  -----------------------------------------------------------------------------------------*/

PROTO UINT8 SDOS_SdoInfoInd(TSDOINFORMATION MBXMEM *pSdoInfoInd);

PROTO UINT8 SDOS_SdoInd(TINITSDOMBX MBXMEM *pSdoMbx);

PROTO void SDOS_SdoRes(UINT8 abort, UINT32 objLength, UINT16 MBXMEM *pData);


#undef PROTO

#endif /* _SDOSRV_H_ */


