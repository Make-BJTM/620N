/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_InterfaceProcess.h
 创建人:    李浩                  创建日期：2012.03.26
 描述:
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __MTR_INTERFACE_PROCESS_H
#define __MTR_INTERFACE_PROCESS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h" 
#include "FUNC_MTRInterface.h"


/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */


/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

/*================以下为FUNC传递过来，供MTR使用的接口变量结构体类型==================================================*/



/*================以下为FUNC传递过来，供MTR使用的接口变量结构体类型==================================================*/

//FUNCToMTR_LIST_4Hz_32Bits共用体类型定义
typedef struct{ 
    int32 PosKpCoef_Q15;           //位置环量纲系数STR_FUNC_Gvar.PosCtrl.KpCoef
    int32 SpdKf_Q12;               //速度环调节器PDFF前馈系数0-4096  wzg20120329   Disable时为4096
	int32 SpdDampingKf_Q12;        //速度环调节器Damping系数0-4096   Disable时为0，Max为4096
}STR_MTR_FUNCTOMTR_LIST_4Hz_32Bits;

typedef union{
    Uint32                              all[FUNCToMTR_LIST_4Hz_32Bits_NUM];
    STR_MTR_FUNCTOMTR_LIST_4Hz_32Bits   List;
}UNI_MTR_FUNCTOMTR_LIST_4Hz_32Bits;



//FUNC模块传递给MTR模块(传递频率为16kHz)位结构体类型
typedef struct{
    Uint32    RunMod:3;                     //运行模式
    Uint32    ServoRunStatus:2;             //指示伺服运行状态的变量,根据使能状态、故障状态合成的变量
    Uint32    PwmStatus:1;                  //PWM状态标志位
    Uint32    ToqStop:1;                    //急停减速使能标志
    Uint32    ZeroSpdStop:1;                //零速停机使能标志

    Uint32    OTClamp:1;                    //发生超程时零位置停机固定标志位
    Uint32    SpdReguDatClr:1;              //速度环偏差等参数清零标志位
    Uint32    AlmRst:1;                     //故障复位
    Uint32    Pcl:1;                        //正转外部转矩限制
    Uint32    Ncl:1;                        //反转外部转矩限制
    Uint32    UVAdjustRatioEn:1;            //UV相电流平衡校正使能
    Uint32    SpdToqLoopNact:1;             //速度转矩环程序屏蔽标志位
    Uint32    OffLnInertiaModeEn:1;         //离线惯量辨识使能

    Uint32    InnerServoSon:1;              //内部使能标志
    Uint32    ModSwitchPeriod:1;            //模式切换过度阶段标志位
    Uint32    BrkOut:1;                     //泄放输出信号
    Uint32    UdcOk:1;                      //母线电压Ok

    Uint32    OpenFSAEn:1;                  //开环扫频使能信号

    Uint32    Rsvd:11;                      //Rsvd
}STR_FUNCToMTR_STATUSFLAG_16kHz;


//FUNCToMTR_LIST_16kHz共用体类型定义
typedef struct{
    //32位变量
    volatile STR_FUNCToMTR_STATUSFLAG_16kHz   StatusFlag; //FUNC模块传递给MTR模块(传递频率为16kHz)位结构体
    Uint32  Spd_Kp;                               //经FUNC模块处理转换后的速度比例增益
    Uint32  Spd_KiQ10;                            //经FUNC模块处理转换后的速度积分增益 Q10
    int32   ToqRefFilterTc;                         //速度环调节器转矩前馈滤波时间单位us (0-655350)
    int32   SpdLmt;                               //由FUNC计算出的转矩模式下的速度限制值
    int32   ToqCmd;                               //转矩模式下的转矩指令
    int32   SpdRef;                               //非转矩模式下速度调节器输入
    int32   PosRef;                               //位置调节器输入,供摩擦力过零补偿使用
//    int32   AI1VoltOut;                           //AI1电压
//    int32   AI2VoltOut;                           //AI2电压
    int32   Udc_Live;  //DC   母线电压

    int32   FSAIqCmd;                             //开环频率特性分析电流激励指令
    int32   FSASpdCmd;                            //开环频率特性分析速度激励指令
    int32   SpdLmtPos;                            //
    int32   SpdLmtNeg;                            //
	
}STR_MTR_FUNCTOMTR_LIST_16kHz;

typedef union{
    Uint32                          all[FUNCToMTR_LIST_16kHz_NUM];
    STR_MTR_FUNCTOMTR_LIST_16kHz    List;
}UNI_MTR_FUNCTOMTR_LIST_16kHz;


/*================以上为FUNC传递过来，供MTR使用的接口变量结构体类型==================================================*/



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern UNI_MTR_FUNCTOMTR_LIST_4Hz_32Bits    UNI_MTR_FUNCToMTR_List_4Hz_32Bits;
extern UNI_MTR_FUNCTOMTR_LIST_16kHz         UNI_MTR_FUNCToMTR_List_16kHz;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


#ifdef __cplusplus
}
#endif

#endif /* __MTR_INTERFACE_PROCESS_H */

/********************************* END OF FILE *********************************/
