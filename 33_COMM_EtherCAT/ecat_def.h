/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: Ecat_def.h
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


#ifndef _ECATDEF_H_
#define _ECATDEF_H_

/*-----------------------------------------------------------------------------------------
------	
------	Includes
------ 
-----------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------------------------
------	
------	Slave  Code Configuration Defines
------	
-----------------------------------------------------------------------------------------*/

/* PREOPTIMEOUT: 指定超时值Init 状态到PreOP/Boot.*/
#define PREOPTIMEOUT                              0x7D0

/* SAFEOP2OPTIMEOUT:  指定超时值 SafeOP 状态到OP. */
#define SAFEOP2OPTIMEOUT                          0x2328

/*-----------------------------------------------------------------------------------------
------	
------	Compiler Defines
------	
-----------------------------------------------------------------------------------------*/

#define FALSE                                     0

#define TRUE                                      1

#define BoolType                                  unsigned char

#define UINT8                                     unsigned char

#define UINT16                                    unsigned short

#define UINT32                                    unsigned int

#define USHORT                                    unsigned short

#define INT8                                      char

#define INT16                                     short

#define INT32                                     int

#define CHAR                                      char

#define UCHAR                                     unsigned char




/* SIZEOF(x): Used to calculate the size in Bytes */
#define SIZEOF(x)                                 sizeof(x)

/* HUGE: Should be adapted to the huge type of the microcontroller, if the microcontroller
does not support a huge type, HUGE shall be defined to nothing */
#define HUGE 

/* HMEMSET: Should be defined to the memset function for huge memory, if the microcontroller
does not support a huge type, HMEMSET shall be defined to a 'normal' memset function */
#define HMEMSET                                   memset

/* HMEMCPY: Should be defined to the memcpy function for huge memory, if the microcontroller
does not support a huge type, HMEMCPY shall be defined to a 'normal' memcpy function */
#define HMEMCPY                                   memcpy

/* HMEMCMP: Should be defined to the memcmp function for huge memory, if the microcontroller
does not support a huge type, HMEMCMP shall be defined to a 'normal' memcmp function */
#define HMEMCMP                                   memcmp

/* ESCMEM: Should be defined to select the memory type of the ESC memory (e.g. near, far or huge), if the microcontroller
does not support different memory types, ESCMEM shall be defined to nothing */
#define ESCMEM 

/* ESCMEMCPY: Should be defined to the memcpy function for ESCMEM memory, if the microcontroller
does not support different memory types, ESCMEMCPY shall be defined to a 'normal' memcpy function */
#define ESCMEMCPY                                 memcpy

/* ESCMEMSET: Should be defined to the memset function for ESCMEM memory, if the microcontroller
does not support different memory types, ESCMEMSET shall be defined to a 'normal' memset function */
#define ESCMEMSET                                 memset

/* ESCMBXMEMCPY: Should be defined to the memcpy function for copying ESCMEM memory to or from MBXMEM memory, if the microcontroller
does not support different memory types, ESCMBXMEMCPY shall be defined to a 'normal' memcpy function */
#define ESCMBXMEMCPY                              memcpy

/* MBXMEM: Should be defined to select the memory type of the memory used for mailbox communication (e.g. near, far or huge), 
if the microcontroller does not support different memory types, MBXMEM shall be defined to nothing */
#define MBXMEM 

/* MBXMEMCPY: Should be defined to the memcpy function for MBXMEM memory, if the microcontroller
does not support different memory types, MBXMEMCPY shall be defined to a 'normal' memcpy function */
#define MBXMEMCPY                                 memcpy

/* MBXMEMCMP: Should be defined to the memcmp function for MBXMEM memory, if the microcontroller
does not support different memory types, MBXMEMCMP shall be defined to a 'normal' memcmp function */
#define MBXMEMCMP                                 memcmp

/* MBXMEMSET: Should be defined to the memcpy function for MBXMEM memory, if the microcontroller
does not support different memory types, MBXMEMSET shall be defined to a 'normal' memset function */
#define MBXMEMSET                                 memset

/* MBXSTRLEN: Should be defined to the strlen function for MBXMEM memory, if the microcontroller
does not support different memory types, MBXSTRLEN shall be defined to a 'normal' strlen function */
#define MBXSTRLEN                                 strlen

/* MBXSTRCPY: Should be defined to the strcpy function for MBXMEM memory, if the microcontroller
does not support different memory types, MBXSTRCPY shall be defined to a 'normal' strcpy function */
#define MBXSTRCPY                                 memcpy

/* OBJCONST: Should be used to define the object dictionary in ROM (f.e. define OBJCONST const) or
in RAM (e.g. define OBJCONST) */
#define OBJCONST                               //   const

/* VARCONST: Should be used to define the constant variables in ROM (f.e. define VARCONST const) or
in RAM (e.g. define VARCONST) */
#define VARCONST 

/* VARVOLATILE: Should be used to prevent dummy variables to be deleted due to compiler optimization.  */
#define VARVOLATILE                               volatile

/* OBJMEM: Should be defined to select the memory type of the memory used for the object dictionary (e.g. near, far or huge), 
if the microcontroller does not support different memory types, OBJMEM shall be defined to nothing */
#define OBJMEM 

/* OBJTOMBXMEMCPY: Should be defined to the memcpy function for copying OBJMEM memory to MBXMEM memory, if the microcontroller
does not support different memory types, OBJTOMBXMEMCPY shall be defined to a 'normal' memcpy function */
#define OBJTOMBXMEMCPY                            memcpy

/* OBJTOMBXSTRCPY: Should be defined to the strcpy function for copying OBJMEM memory to MBXMEM memory, if the microcontroller
does not support different memory types, OBJTOMBXSTRCPY shall be defined to a 'normal' memcpy function */
#define OBJTOMBXSTRCPY                            memcpy

/* OBJMEMCPY: Should be defined to the memcpy function for OBJMEM memory, if the microcontroller
does not support different memory types, OBJMEMCPY shall be defined to a 'normal' memcpy function */
#define OBJMEMCPY                                 memcpy

/* OBJSTRLEN: Should be defined to the strlen function for OBJMEM memory, if the microcontroller
does not support different memory types, OBJSTRLEN shall be defined to a 'normal' strlen function */
#define OBJSTRLEN                                 strlen

/* OBJSTRCPY: Should be defined to the strcpy function for OBJMEM memory, if the microcontroller
does not support different memory types, OBJSTRCPY shall be defined to a 'normal' strcpy function */
#define OBJSTRCPY                                 memcpy

/* MAKE_HUGE_PTR: Should be defined to the initialize a pointer variable with an absolute address */
#define MAKE_HUGE_PTR 

/* MAKE_PTR_TO_ESC: Should be defined to the initialize the pointer to the ESC */
#define MAKE_PTR_TO_ESC 

/* EMCYMEMCPY: Should be defined to the memcpy function for EMCYMEM memory, if the microcontroller
does not support different memory types, EMCYMEMCPY shall be defined to a 'normal' memcpy function */
#define EMCYMEMCPY                                memcpy

/* EMCYMEMSET: Should be defined to the memset function for EMCYMEM memory, if the microcontroller
does not support different memory types, EMCYMEMSET shall be defined to a 'normal' memcset function */
#define EMCYMEMSET                                memset

/* EMCYMEM: Should be defined to select the memory type of the memory used for the emergencies (e.g. near, far or huge), 
if the microcontroller does not support different memory types, EMCYMEM shall be defined to nothing */
#define EMCYMEM 

/* MEMCPY: Should be defined to copy data within local memory. */
#define MEMCPY                                    memcpy

/* ALLOCMEM(size): Should be defined to the alloc function to get dynamic memory */
#define ALLOCMEM(size)                            malloc((size))

/* FREEMEM(pointer): Should be defined to the free function to put back dynamic memory */
#define FREEMEM(pointer)                          free((pointer))

/* VARMEMSET: Should be defined to the memset function for VARMEM memory, if the microcontroller
does not support different memory types, EMCYMEMSET shall be defined to a 'normal' memcset function */
#define VARMEMSET                                 memset

/* VARMEM: Should be defined to select the memory type of the memory used for dynamic memory (e.g. near, far or huge), 
if the microcontroller does not support different memory types, VARMEM shall be defined to nothing */
#define VARMEM 

/* MEM_ADDR: Type to access local memory addresses */
#define MEM_ADDR                                  UINT16

/* GET_MEM_SIZE(ByteSize): Round up the byte size to next matching memory boundary depending on "MEM_ADDR" */
#define GET_MEM_SIZE(ByteSize)                    (((ByteSize)+1) >> 1)

/* APPL_AllocMailboxBuffer(size): Should be defined to a function to get a buffer for a mailbox service,
this is only used if the switch MAILBOX_QUEUE is set */
#define APPL_AllocMailboxBuffer(size)             malloc((size))

/* APPL_FreeMailboxBuffer(pointer): Should be defined to a function to put back a buffer for a mailbox service,
this is only used if the switch MAILBOX_QUEUE is set */
#define APPL_FreeMailboxBuffer(pointer)           free((pointer))

/* STRUCT_PACKED_START: Is defined before the typedef struct construct to pack the generic structures if necessary */
#define STRUCT_PACKED_START 

/* STRUCT_PACKED_END: Is defined after the typedef struct {} construct to pack the generic structures if necessary */
#define STRUCT_PACKED_END 

/* MBX_STRUCT_PACKED_START: Is defined before the typedef struct construct to pack the MAILBOX structures if necessary */
#define MBX_STRUCT_PACKED_START 

/* MBX_STRUCT_PACKED_END: Is defined after the typedef struct {} construct to pack the MAILBOX structures if necessary */
#define MBX_STRUCT_PACKED_END 

/* OBJ_STRUCT_PACKED_START: Is defined before the typedef struct construct to pack the OBJECT structures if necessary */
#define OBJ_STRUCT_PACKED_START 

/* OBJ_STRUCT_PACKED_END: Is defined after the typedef struct {} construct to pack the OBJECT structures if necessary */
#define OBJ_STRUCT_PACKED_END 

/* OBJ_DWORD_ALIGN: Shall be set if the object structures are not Byte aligned and the Code is executed on an 32bit platform */
#define OBJ_DWORD_ALIGN                           1

/* OBJ_WORD_ALIGN: Shall be set if the object structures are not Byte aligned and the Code is executed on an 16bit platform */
#define OBJ_WORD_ALIGN                            0


/*-----------------------------------------------------------------------------------------
------	
------	Application Specific Defines
------	
-----------------------------------------------------------------------------------------*/

/* VENDOR_ID: Object 0x1018 SI1 (Vendor ID)	 */ 

#define VENDOR_ID                                 0x00100000

/* PRODUCT_CODE: Object 0x1018 SI2 (EtherCAT product code) */
#define PRODUCT_CODE                              0x000C0108

/* REVISION_NUMBER: Object 0x1018 SI3 (EtherCAT product revision number) */
#define REVISION_NUMBER                           0x00010001

/* SERIAL_NUMBER: Object 0x1018 SI4 (EtherCAT product serial number) */
#define SERIAL_NUMBER                             0x00000000

/* DEVICE_PROFILE_TYPE: 从站驱动类型 (Object 0x1000)  */
#define DEVICE_PROFILE_TYPE                     0x00020192    // 0x00001389

/* DEVICE_NAME: 从站驱动名称 (Object 0x1008) */
#define DEVICE_NAME                               "IS620-ECAT"

/* DEVICE_NAME_LEN:  DEVICE_NAME  长度*/
#define DEVICE_NAME_LEN                           0x0A

/* DEVICE_HW_VERSION: 硬件版本(Object 0x1009) */
#define DEVICE_HW_VERSION                         "V1.0"

/* DEVICE_HW_VERSION_LEN: 'DEVICE_HW_VERSION'  长度 */
#define DEVICE_HW_VERSION_LEN                     0x4

/* DEVICE_SW_VERSION: 软件版本(Object 0x100A) */
#define DEVICE_SW_VERSION                         "V1.0"

/* DEVICE_SW_VERSION_LEN: 'DEVICE_SW_VERSION' 长度*/
#define DEVICE_SW_VERSION_LEN                     0x4

/* MIN_PD_WRITE_ADDRESS: 过程数据输出缓冲区最小地址 (Sync Manager 2) */
#define MIN_PD_WRITE_ADDRESS                      0x1000

/* MAX_PD_WRITE_ADDRESS: 过程数据输出缓冲区最大地址 (Sync Manager 2) */
#define MAX_PD_WRITE_ADDRESS                      0x3000

/* MIN_PD_READ_ADDRESS: 过程数据输入缓冲区最小地址(Sync Manager 3) */
#define MIN_PD_READ_ADDRESS                       0x1000

/* MAX_PD_READ_ADDRESS: 过程数据输入缓冲区最大地址(Sync Manager 3) */
#define MAX_PD_READ_ADDRESS                       0x3000

/* MIN_MBX_SIZE: 最小的邮箱大小 (Sync Manager 0 and 1) 可以由主设置. */
#define MIN_MBX_SIZE                              0x0022

/* MAX_MBX_SIZE: 最大的邮箱大小 (Sync Manager 0 and 1) 可以由主设置. */
#define MAX_MBX_SIZE                              0x0100

/* MIN_MBX_WRITE_ADDRESS: 最小的接收邮箱缓冲区地址 (Sync Manager 0). */
#define MIN_MBX_WRITE_ADDRESS                     0x1000

/* MAX_MBX_WRITE_ADDRESS: 最大的接收邮箱缓冲区地址 (Sync Manager 0). */
#define MAX_MBX_WRITE_ADDRESS                     0x3000

/* MAX_PD_INPUT_SIZE: 最大的过程数据输入大小(Sync Manager 3)*/
#define MAX_PD_INPUT_SIZE                         0x0028

/* MIN_MBX_READ_ADDRESS: 最小的发送邮箱缓冲区地址(Sync Manager 1). */
#define MIN_MBX_READ_ADDRESS                      0x1000

/* MAX_MBX_READ_ADDRESS: 最大的发送邮箱缓冲区地址(Sync Manager 1). */
#define MAX_MBX_READ_ADDRESS                      0x3000

/* MAX_PD_OUTPUT_SIZE: 最大的过程数据输出大小(Sync Manager 2) */
#define MAX_PD_OUTPUT_SIZE                        0x0028

/* MIN_PD_CYCLE_TIME: 从站支持的最小周期时间 ns
(entry 0x1C32:05 or entry 0x1C33:05) */
//#define MIN_PD_CYCLE_TIME                         0x7A120

#define MIN_PD_CYCLE_TIME                         0x1E848

/* MAX_PD_CYCLE_TIME: 从站支持的最大周期时间 ns */
#define MAX_PD_CYCLE_TIME                         0xC3500000

/* PD_OUTPUT_DELAY_TIME: 从站支持的最小输出延时时间ns (entry 0x1C32:09) */
#define PD_OUTPUT_DELAY_TIME                      0x0

/* PD_OUTPUT_CALC_AND_COPY_TIME: 输出计算处理加写入发送缓冲区的最小时间ns (entry 0x1C32:06) */
#define PD_OUTPUT_CALC_AND_COPY_TIME              0x0

/* PD_INPUT_CALC_AND_COPY_TIME: 输入计算处理加读出接收缓冲区的最小时间 ns (entry 0x1C33:06) */
#define PD_INPUT_CALC_AND_COPY_TIME               0x0

/* PD_INPUT_DELAY_TIME: 输入延时时间 (entry 0x1C33:09) */
#define PD_INPUT_DELAY_TIME                       0x0



/*-----------------------------------------------------------------------------------------
------	
------	Test Configuration Defines
------	
-----------------------------------------------------------------------------------------*/


#endif // _ECATDEF_H_

