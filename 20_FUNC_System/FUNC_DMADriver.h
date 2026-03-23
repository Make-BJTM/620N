/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_DMADriver.h  
 创建人：   李浩                   创建日期：2012.01.15                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1.
    2.
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_DMADRIVER_H
#define __FUNC_DMADRIVER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */ 
#define DMA1_Channel3_Enable()       (FUNC_DMA1->Channel3_SCR.bit.EN = 1)
#define DMA1_Channel3_Disable()      (FUNC_DMA1->Channel3_SCR.bit.EN = 0)

#define DMA1_Channel4_Enable()       (FUNC_DMA1->Channel4_SCR.bit.EN = 1)
#define DMA1_Channel4_Disable()      (FUNC_DMA1->Channel4_SCR.bit.EN = 0)
/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 

//此寄存器由硬件置位+++++++++++++++++++++++++++++++++++++++++
//DMA中断状态寄存器结构体类型定义
typedef struct{
    Uint32  FEIF0:1;         //bit0 
    Uint32  Reserved0:1;      //bit1 保留
	Uint32  DMEIF0:1;        //bit2
	Uint32  TEIF0:1;         //bit3 
    Uint32  HTIF0:1;         //bit4
	Uint32  TCIF0:1;         //bit5 
    Uint32  FEIF1:1;         //bit6 
    Uint32  Reserved1:1;      //bit7 保留
	Uint32  DMEIF1:1;        //bit8
	Uint32  TEIF1:1;         //bit9 
    Uint32  HTIF1:1;         //bit10
	Uint32  TCIF1:1;         //bit11
	Uint32  Reserved2:4;      //bit12-bit15 
  
    Uint32  FEIF2:1;         //bit16 
    Uint32  Reserved3:1;      //bit17 保留
	Uint32  DMEIF2:1;        //bit18
	Uint32  TEIF2:1;         //bit19 
    Uint32  HTIF2:1;         //bit20
	Uint32  TCIF2:1;         //bit21 
    Uint32  FEIF3:1;         //bit22 
    Uint32  Reserved4:1;      //bit23 保留
	Uint32  DMEIF3:1;        //bit24
	Uint32  TEIF3:1;         //bit25 
    Uint32  HTIF3:1;         //bit26
	Uint32  TCIF3:1;         //bit27
	Uint32  Reserved5:4;      //bit28-bit31 
}STR_DMA_LISR_BIT; 

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_LISR_BIT      bit; 
}UNI_DMA_LISR_REG;
 
typedef struct{
    Uint32  FEIF4:1;         //bit0 
    Uint32  Reserved0:1;      //bit1 保留
	Uint32  DMEIF4:1;        //bit2
	Uint32  TEIF4:1;         //bit3 
    Uint32  HTIF4:1;         //bit4
	Uint32  TCIF4:1;         //bit5 
    Uint32  FEIF5:1;         //bit6 
    Uint32  Reserved1:1;      //bit7 保留
	Uint32  DMEIF5:1;        //bit8
	Uint32  TEIF5:1;         //bit9 
    Uint32  HTIF5:1;         //bit10
	Uint32  TCIF5:1;         //bit11
	Uint32  Reserved2:4;      //bit12-bit15 
  
    Uint32  FEIF6:1;         //bit16 
    Uint32  Reserved3:1;      //bit17 保留
	Uint32  DMEIF6:1;        //bit18
	Uint32  TEIF6:1;         //bit19 
    Uint32  HTIF6:1;         //bit20
	Uint32  TCIF6:1;         //bit21 
    Uint32  FEIF7:1;         //bit22 
    Uint32  Reserved4:1;      //bit23 保留
	Uint32  DMEIF7:1;        //bit24
	Uint32  TEIF7:1;         //bit25 
    Uint32  HTIF7:1;         //bit26
	Uint32  TCIF7:1;         //bit27
	Uint32  Reserved5:4;      //bit28-bit31 
}STR_DMA_HISR_BIT; 

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_HISR_BIT      bit; 
}UNI_DMA_HISR_REG; 
//此寄存器由硬件置位+++++++++++++++++++++++++++++++++++++++++

//DMA中断标志清除寄存器结构体类型定义
typedef struct{
    Uint32  CFEIF0:1;         //bit0 
	Uint32  Reserved0:1;       //bit1
	Uint32  CDMEIF0:1;        //bit2
	Uint32  CTEIF0:1;         //bit3
	Uint32  CHTIF0:1;         //bit4
	Uint32  CTCIF0:1;         //bit5
    Uint32  CFEIF1:1;         //bit6 
	Uint32  Reserved1:1;       //bit7
	Uint32  CDMEIF1:1;        //bit8
	Uint32  CTEIF1:1;         //bit9
	Uint32  CHTIF1:1;         //bit10
	Uint32  CTCIF1:1;         //bit11
	Uint32  Reserved2:4;       //bit12-bit15

    Uint32  CFEIF2:1;         //bit16 
	Uint32  Reserved3:1;       //bit17
	Uint32  CDMEIF2:1;        //bit18
	Uint32  CTEIF2:1;         //bit19
	Uint32  CHTIF2:1;         //bit20
	Uint32  CTCIF2:1;         //bit21
    Uint32  CFEIF3:1;         //bit22 
	Uint32  Reserved4:1;       //bit23
	Uint32  CDMEIF3:1;        //bit24
	Uint32  CTEIF3:1;         //bit25
	Uint32  CHTIF3:1;         //bit26
	Uint32  CTCIF3:1;         //bit27
	Uint32  Reserved5:4;       //bit28-bit31
}STR_DMA_LIFCR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_LIFCR_BIT      bit;
}UNI_DMA_LIFCR_REG;

typedef struct{
    Uint32  CFEIF4:1;         //bit0 
	Uint32  Reserved0:1;       //bit1
	Uint32  CDMEIF4:1;        //bit2
	Uint32  CTEIF4:1;         //bit3
	Uint32  CHTIF4:1;         //bit4
	Uint32  CTCIF4:1;         //bit5
    Uint32  CFEIF5:1;         //bit6 
	Uint32  Reserved1:1;       //bit7
	Uint32  CDMEIF5:1;        //bit8
	Uint32  CTEIF5:1;         //bit9
	Uint32  CHTIF5:1;         //bit10
	Uint32  CTCIF5:1;         //bit11
	Uint32  Reserved2:4;       //bit12-bit15

    Uint32  CFEIF6:1;         //bit16 
	Uint32  Reserved3:1;       //bit17
	Uint32  CDMEIF6:1;        //bit18
	Uint32  CTEIF6:1;         //bit19
	Uint32  CHTIF6:1;         //bit20
	Uint32  CTCIF6:1;         //bit21
    Uint32  CFEIF7:1;         //bit22 
	Uint32  Reserved4:1;       //bit23
	Uint32  CDMEIF7:1;        //bit24
	Uint32  CTEIF7:1;         //bit25
	Uint32  CHTIF7:1;         //bit26
	Uint32  CTCIF7:1;         //bit27
	Uint32  Reserved5:4;       //bit28-bit31
}STR_DMA_HIFCR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_HIFCR_BIT     bit;
}UNI_DMA_HIFCR_REG;

//DMA中断相关寄存器结构体类型定义
typedef struct{
    volatile UNI_DMA_LISR_REG      LISR;
	volatile UNI_DMA_HISR_REG      HISR;
    volatile UNI_DMA_LIFCR_REG     LIFCR;
	volatile UNI_DMA_HIFCR_REG     HIFCR;
}STR_FUNC_DMA_IT_Def;



//DMA通道x配置寄存器结构体类型定义
typedef struct{
    Uint32  EN:1;            //bit0 通道开启
	Uint32  DMEIE:1;         //bit1
    Uint32  TEIE:1;          //bit2
    Uint32  HTIE:1;          //bit3 
    Uint32  TCIE:1;          //bit4
	Uint32  PFCTRL:1;        //bit5
    Uint32  DIR:2;           //bit6,bit7 
    Uint32  CIRC:1;          //bit8 
    Uint32  PINC:1;          //bit9 
    Uint32  MINC:1;          //bit10 存储器地址增量模式
    Uint32  PSIZE:2;         //bit11-12 外设数据宽度
    Uint32  MSIZE:2;         //bit13-14 存储器数据宽度
	Uint32  PINCOS:1;        //bit15
    Uint32  PL:2;            //bit16-17 通道优先级
	Uint32  DBM:1;           //bit18
	Uint32  CT:1;            //bit19
	Uint32  Reserved0:1;      //bit20
	Uint32  PBURST:2;        //bit21-22
	Uint32  MBURST:2;        //bit23-24
	Uint32  CHSEL:3;         //bit25-27
	Uint32  Reserved1:4;      //bit28-31
}STR_DMA_SCR_BIT;

typedef union{
    volatile Uint32               all;
    volatile STR_DMA_SCR_BIT      bit;
}UNI_DMA_SCR_REG;

//DMA通道x传输数量寄存器结构体类型定义
typedef struct{
    Uint32   NDT:16;          //bit0-15 数据传输数量
    Uint32   Rsvd:16;         //bit16-31 保留
}STR_DMA_SNDTR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_SNDTR_BIT     bit;    
}UNI_DMA_SNDTR_REG;

//DMA通道外设地址寄存器结构体类型定义
typedef struct{
    Uint32   PAR:32;          //bit0-31 数据传输数量
}STR_DMA_SPAR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_SPAR_BIT      bit;    
}UNI_DMA_SPAR_REG;

//DMA通道存储器地址寄存器结构体类型定义
typedef struct{
    Uint32   M0A:32;          //bit0-31 数据传输数量
}STR_DMA_SM0AR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_SM0AR_BIT     bit;    
}UNI_DMA_SM0AR_REG;

//DMA通道存储器地址寄存器结构体类型定义
typedef struct{
    Uint32   M1A:32;          //bit0-31 数据传输数量
}STR_DMA_SM1AR_BIT;

typedef union{
    volatile Uint32                all;
    volatile STR_DMA_SM1AR_BIT     bit;    
}UNI_DMA_SM1AR_REG;

//DMA通道FIFO控制寄存器
typedef struct{
    Uint32  FTH:2;	     //bit0-1
	Uint32  DMDIS:1;     //bit2
	Uint32  FS:3;        //bit3-5
	Uint32  Reserved0:1;  //bit6
	Uint32  FEIE:1;      //bit7
	Uint32  Reserved1:24; //bit8-31
}STR_DMA_SFCR_BIT;
typedef union{
    volatile Uint32                all;
    volatile STR_DMA_SFCR_BIT      bit;    
}UNI_DMA_SFCR_REG;
//DMA1寄存器结构体类型定义
typedef struct{
    //DMA 通道1 ++++++++++++++++++  
    volatile UNI_DMA_SCR_REG      Channel0_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel0_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel0_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel0_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel0_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel0_SFCR;

    //DMA 通道2 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel1_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel1_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel1_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel1_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel1_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel1_SFCR;

    //DMA 通道3 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel2_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel2_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel2_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel2_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel2_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel2_SFCR;

    //DMA 通道4 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel3_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel3_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel3_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel3_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel3_SM1AR;
    volatile UNI_DMA_SFCR_REG    Channel3_SFCR;

    //DMA 通道5 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel4_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel4_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel4_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel4_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel4_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel4_SFCR;

    //DMA 通道6 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel5_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel5_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel5_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel5_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel5_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel5_SFCR;

    //DMA 通道7 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel6_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel6_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel6_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel6_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel6_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel6_SFCR;

    //DMA 通道8 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel7_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel7_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel7_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel7_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel7_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel7_SFCR;
}STR_FUNC_DMA1_Def;

//DMA2寄存器结构体类型定义
typedef struct{
    //DMA 通道1 ++++++++++++++++++  
    volatile UNI_DMA_SCR_REG      Channel0_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel0_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel0_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel0_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel0_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel0_SFCR;

    //DMA 通道2 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel1_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel1_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel1_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel1_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel1_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel1_SFCR;

    //DMA 通道3 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel2_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel2_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel2_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel2_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel2_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel2_SFCR;

    //DMA 通道4 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel3_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel3_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel3_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel3_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel3_SM1AR;
    volatile UNI_DMA_SFCR_REG    Channel3_SFCR;

    //DMA 通道5 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel4_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel4_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel4_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel4_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel4_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel4_SFCR;

    //DMA 通道6 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel5_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel5_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel5_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel5_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel5_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel5_SFCR;

    //DMA 通道7 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel6_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel6_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel6_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel6_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel6_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel6_SFCR;

    //DMA 通道8 ++++++++++++++++++
    volatile UNI_DMA_SCR_REG      Channel7_SCR;
    volatile UNI_DMA_SNDTR_REG    Channel7_SNDTR;
    volatile UNI_DMA_SPAR_REG     Channel7_SPAR;
    volatile UNI_DMA_SM0AR_REG    Channel7_SM0AR;
	volatile UNI_DMA_SM1AR_REG    Channel7_SM1AR;
    volatile UNI_DMA_SFCR_REG     Channel7_SFCR;
}STR_FUNC_DMA2_Def;
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_FUNC_DMA_IT_Def * FUNC_DMA1_IT_REG; 
extern STR_FUNC_DMA_IT_Def * FUNC_DMA2_IT_REG;
extern STR_FUNC_DMA1_Def * FUNC_DMA1;
extern STR_FUNC_DMA2_Def * FUNC_DMA2;


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_DMADriver_H */

/********************************* END OF FILE *********************************/


