/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_StopProcess.h
 创建人：王军干
 修改人: 李浩                创建日期：11.12.08 
 描述：
       1.减少共享所有头文件的情况，避免数据随意更改,
       2.常规数据类型的声明，及其他模块或文件需要共享的函数的原型声明.      
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/
#ifndef FUNC_STOPPROCESS_H
#define FUNC_STOPPROCESS_H 

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
/* 常规类型定义 */   

 
/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
/*伺服停机控制标志位结构体*/
typedef struct{
    Uint32  Err1StopAck:1;         //1: 可复位故障1停机响应标志位      -》StatusFlag.bit.Err1AckER
    Uint32  Err2StopAck:1;         //2: 可复位故障2停机响应标志位      -》StatusFlag.bit.Err2EnAckER
    Uint32  OTStopAck:1;           //3: 超程停机响应标志位             -》StatusFlag.bit.OTAckER   OTAckER:1;        // 5 响应超程标志位
    Uint32  ServoOffAck:1;         //4: 伺服OFF响应标志位              -》StatusFlag.bit.SOffAckER SOffAckER
    Uint32  EmergencyStopAck:1;    //5: 紧急停机响应标志位             
    Uint32  CanQuickStopAck:1;     //6:CAnopen紧急停机响应标志位
    Uint32  CanHaltStopAck:1;      //7:CAnopen暂停响应标志位

    Uint32  Rsvd6:1;               //8: 伺服Off停机方式执行一次标志位    -》StatusFlag.bit.SOffFlg
    Uint32  FirErr1Stop:1;         //9: 故障1停机方式执行一次标志位      -》StatusFlag.bit.NO1StopFlg
    Uint32  FirErr2Stop:1;         //10: 故障2停机方式执行一次标志位      -》StatusFlag.bit.NO2StopFlg    
//    Uint16  FirOTStop:1;           //、: 超程停机方式执行一次标志位       -》OTStopFlg:1;
//    Uint16  FirEmergencyStop:1;    //、: 紧急停机方式执行一次标志位
    Uint32 FirCanQuickStop:1;        //11:CANopen快速停机执行一次标志位
    Uint32 FirCanHalt:1;             //12:CANopen暂停执行一次标志位
    
//    Uint16  GDStopEnable:1;        //11: GD停机方式方式使能标志位       -》StatusFlag.bit.RunEnGDFlg
    Uint32  OTClrReguFlg:1;        //13:超程停机时调节器清0标志位
    Uint32  EMSClrReguFlg:1;       //14:紧急停机时调节器清0标志位
    Uint32  CanQuickClrReguFlg:1;   //15:CAnopen紧急停机时调节器清零标志位
    
    Uint32  EMSClampFlg:1;         //16: 紧急停机嵌位标志位
    Uint32  OTClampFlg:1;          //17: 超程停机嵌位标志位
    Uint32  CanQuickClampFlg:1;    //18: CAnopen紧急停机嵌位标志位
    Uint32  CanHaltClampFlg:1;     //19: CAnopen暂停嵌位标志位

    Uint32  Rsvd:13;                //20~32: 保留 
}STR_STOP_CONTROL_FLAG;

/*伺服停机控制标志位公用体*/
typedef union{
    volatile Uint32                   all;
    volatile STR_STOP_CONTROL_FLAG    bit;
}UNI_STOP_CONTROL_FLAG;

/*伺服停机控制变量结构体*/
typedef struct{
    Uint16   Err1StopMode;       //NO1故障停机方式(与伺服Off共用相同的停机方式)      -》NO1StopMs
    Uint16   Err1StopState;      //NO1故障停机状态(与伺服Off共用相同的停机状态)      -》NO1StopStatusMs

    Uint16   Err2StopMode;       //NO2故障停机方式                                   -》NO2StopMs; 
    Uint16   Err2StopState;      //NO2故障停机状态                                   -》NO2StopStatusMs

    Uint16   OTStopMode;         //超程故障停机方式                                  -》OTStopMs    
    Uint16   OTStopState;        //超程故障停机状态                                  -》OTStopStatusMs

    Uint16   SoffStopMode;       //伺服Off停机方式  ----  新增加      
    Uint16   SoffStopState;      //伺服Off停机状态  ----  新增加    

    Uint16   EmergencyStopMode;  //紧急停机方式
    Uint16   EmergencyStopState; //紧急停机状态

    Uint16   CanQuickStopMode; //Canopen紧急停机方式
    Uint16   CanQuickStopState;//Canopen紧急停机状态

    Uint16   CanHaltStopMode; //Canopen暂停方式
    Uint16   CanHaltStopState;//Canopen暂停状态

    int32   StopModStateCutSpd; //停机方式和停机状态切换速度条件值
	int32   StopModStateCutSpd2;
}STR_STOP_CONTROL_VARIABLE;                                                                                  



/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void StopModeInit(void);
extern void StopProcess(void);
extern void WriteGDStop(Uint8 i);
extern void ServoStopStatus(Uint8 StopStateModSel);
extern void ServoStopRun(Uint8 StopModSel);

#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_StopProcess */

/********************************* END OF FILE *********************************/
