/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_Oscilloscope.h
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1.
    2.
 缩写：
    1. osci -> oscilloscope
    2. trig -> trigger
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.
********************************************************************************/ 
#ifndef __FUNC_OSCILLOSCOPE_H
#define __FUNC_OSCILLOSCOPE_H

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

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
//示波器位结构体类型定义
typedef struct{
    Uint16   Mode:4;                //示波器模式
    Uint16   SampleStutas:4;        //采样状态 0:未采样 1:触发前准备 2:触发前采样 3:触发后采样 4:单次采样完成 5:连续采样
    Uint16   TrigEdgeSel_A:3;       //触发沿选择A  0:上升沿 1:下降沿 2:沿变化 3:水平之上 4:水平之下
    Uint16   TrigEdgeSel_B:3;       //触发沿选择B  0:上升沿 1:下降沿 2:沿变化 3:水平之上 4:水平之下
    Uint16   TrigCtrl:2;            //触发控制     0:条件A 1:条件A或条件B 2:条件A与条件B
}STR_OSCILLOSCOPECTRL_BIT;

typedef union{
    volatile Uint16                      all;
    volatile STR_OSCILLOSCOPECTRL_BIT    bit;
}UNI_OSCILLOSCOPECTRL_REG;
//

//示波器变量结构体类型定义  
typedef struct{
    volatile UNI_OSCILLOSCOPECTRL_REG   OsciCtrl;    //示波器控制结构体

    Uint8   SectionSel;                     //连续采样时扇区选择

    Uint16  SampleTime;                     //采样间隔时间(单位:电流环调度周期)
    Uint16  SampleTimeMin;                  //连续采样时,采样间隔时间最小值(单位:电流环调度周期)
    Uint16  SampleDataLen;                  //采样数据长度
    Uint16  SampleCnt;                      //采样数据计数器

    int32 * pTrigTarget_A;                   //触发对象A地址指针
    int32 * pTrigTarget_B;                   //触发对象B地址指针
    int32   TrigTargetLatch_A;              //触发锁存值A
    int32   TrigTargetLatch_B;              //触发锁存值B
    int32   TrigTargetCoeff_A;              //触发对象系数A
    int32   TrigTargetCoeff_B;              //触发对象系数B

    Uint16  TrigTargetBit_A;                //触发对象位控制A
    Uint16  TrigTargetBit_B;                //触发对象位控制B
    int16   TrigLevel_A;                    //触发水平A
    int16   TrigLevel_B;                    //触发水平B
    Uint16  DataLenAfterTrig;               //触发有效后的数据长度

    //注意下面变量的顺序不能改变
    int32 * pCH1Addr;                        //通道1变量地址指针
    int32 * pCH2Addr;                        //通道2变量地址指针
    int32 * pCH3Addr;                        //通道3变量地址指针
    int32 * pCH4Addr;                        //通道4变量地址指针


    Uint16 * pCH1BuffAddr_16Bits;            //通道1采样16位数据时缓冲区地址指针
    Uint16 * pCH2BuffAddr_16Bits;            //通道2采样16位数据时缓冲区地址指针
    Uint16 * pCH3BuffAddr_16Bits;            //通道3采样16位数据时缓冲区地址指针
    Uint16 * pCH4BuffAddr_16Bits;            //通道4采样16位数据时缓冲区地址指针

    int32 * pCH1BuffAddr_32Bits;             //通道1采样32位数据时缓冲区地址指针
    int32 * pCH2BuffAddr_32Bits;             //通道2采样32位数据时缓冲区地址指针
    int32 * pCH3BuffAddr_32Bits;             //通道3采样32位数据时缓冲区地址指针
    int32 * pCH4BuffAddr_32Bits;             //通道4采样32位数据时缓冲区地址指针

    //注意下面变量的顺序不能改变
    Uint32   CH1Coeff;                   //通道1系数
    Uint32   CH2Coeff;                   //通道2系数
    Uint32   CH3Coeff;                   //通道3系数
    Uint32   CH4Coeff;                   //通道4系数

}STR_OSCILLOSCOPE;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_OSCILLOSCOPE_H */

/********************************* END OF FILE *********************************/
