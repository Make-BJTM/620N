/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_ServoMonitor.h
 创建人：王军干
 修改人：李浩                修改日期：11.12.09 
 描述：
       1.
       2.     
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/
#ifndef FUNC_SERVOMONITOR_H
#define FUNC_SERVOMONITOR_H 


#ifdef __cplusplus
extern "C" {
#endif   

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h"
#include "FUNC_StopProcess.h"
#include "FUNC_ServoError.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */
                                                                    
/* Exported_Types ------------------------------------------------------------*/ 
/* 常规类型定义 */   

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

/* 警告是否自动解除不显示及没解除显示标志位 */
//当该标志位置1时，表明该警告发生，可以面板显示；
//反之，若该标志位清零，表明该警告自动解除，面板不显示 
typedef struct {
    Uint32 IDENTIFYWARNFLG:1;       //0xEA40 参数辨识失败
    Uint32 ODVALUEERRFLG:1;         //0xE998 参数设置不合理
    Uint32 CANPASSIVEERRFLG:1;      //0xE996 总线被动错误警告
    Uint32 CANRECOVBUSOFFFLG:1;     //0xE995 总线恢复警告
    Uint32 COMMADDRCONFLICTFLG:1;   //0xE994 CANLINK地址冲突
    Uint32 POWPLWARNFLG:1;          //0xE990 功率电源缺相警告标志位
    Uint32 NOTWARNFLG:1;            //0xE952 负向超程状态异常标志位
    Uint32 PNOTWARNFLG:1;           //0xE950 正向超程状态异常标志位
    Uint32 PRAMSVWARNFLG:1;         //0xE942 参数存储频繁警告
    Uint32 MCHGDWARNFLG:1;          //0xE941 需重新接通电源的参数变更警告标志位
    Uint32 MTOFFLINEFLG:1;          //0xE939 电机动力线断线
    Uint32 RBTOOSMALLFLG:1;         //0xE922 外置再生泄放电阻阻值过低警告标志位
    Uint32 RBOVERLOADFLG:1;         //0xE920 再生制动电阻过载
	Uint32 BLKOVERCURRENTFLG:1;     //0xE921 再生制动电流过大
    Uint32 AIZEROBIASOVWARNFLG:1;   //0xE831 AI零偏过大
    Uint32 ORIGINOVERTIMEFLG:1;     //0xE601 原点复归回零超时错误
    Uint32 PULSOUTSETWARNFLG:1;     //0xE110 分频脉冲输出设定故障
    Uint32 EMSTOPWARNFLG:1;         //0xE900 紧急停机警告
    Uint32 MTOLWARNFLG:1;           //0xE909 电机过载警告
    Uint32 ENCDBATWARNFLG:1;        //0xE730 编码器电池报警
    Uint32 ENCDTXCOMMERRFLG:1;      //0xE732 编码器TX端通讯校验错误
    Uint32 ABSMOTALARMFLG:1;        //0xE980 编码器算法异常警告
    Uint32 ENCODEROTFLG:1;          //0xE760 编码器过热 
    Uint32 Rsvd:9;
}STR_WARN_RELEASE_FLAG;

typedef union{
    volatile Uint32                  all;
    volatile STR_WARN_RELEASE_FLAG   bit;
}UNI_WARN_RELEASE_FLAG;



/* 抱闸功能相关标志位位结构体定义 */
typedef struct{
    Uint8    ZeroSpdBrake:1;         //原状态为接近零速时的抱闸处理标志位 -》 gstr_Monitor.BlkFlag.bit.ZspdDelayEn    ZspdDelayEn
   
    Uint8    TravelBrake:1;          //原状态为旋转时的抱闸处理标志位 -》 gstr_Monitor.BlkFlag.bit.TspdEn

    Uint8    SoffToSonEn:1;

    Uint8    ErrZeroSpdBrake:1;      //故障2零速停机处理方式时的抱闸处理标志位 -》 ErrZstopFlg;
    Uint8    ErrZeroSpdBrakeOver:1;  //故障2零速停机处理方式时的抱闸处理结束标志位 -》gstr_Monitor.BlkFlag.bit.ErrZstopOver
  	Uint8    ErrResetPWMOn:1;		 //处理可复位故障复位后，立即使能不出力的问题  //by huangxin201804 _1
    Uint8    Rsvd:2;
}STR_BRAKE_FLAG;                      

/* 抱闸功能相关标志位共用体定义 */
typedef union{
    volatile Uint8             all;  
    volatile STR_BRAKE_FLAG    bit;
}UNI_BRAKE_FLAG;

/* 运行状态切换标志位位域结构体定义 */
typedef struct{
    Uint16     ServoEnable:2;    //伺服是否使能状态标志位
    Uint16     ZeroSpdFlag:1;    //伺服OFF后，零速停机标志位0～零速停机，1～自由停机
    Uint16     DisPwmFlag:1;     //PWM关断标志位	0～未关断，  1～已关断
    Uint16     ServoOn:1;        //监控模块内部使用的伺服On信息值 1-为Di输入了伺服On信号，0-为Di输入了伺服Off信号；  -》SvOn_m
    Uint16     ServoOnLast:1;    //监控模块内部使用的上一次伺服On信息值      -》SvEnDI
    Uint16     RefDir:1;         //当前指令方向标志位:0-正方向及静止，1-负方向  等价于原来的 SVSTFlag.bit.REFDIR 
    Uint16     InnerServoSon:1;  //内部伺服ON信号标志,需要伺服 ON的辅助功能使用
    Uint16     FstSonFlag:1;     //外部DI第一次使能
    Uint16     AngIntRdy:1;
    Uint16     CanopenServoSon:1;                 //内部使能标志

    Uint16     Rsvd:5;
}STR_RUNSTATE_FLAG;

/* 运行状态切换标志位共用体定义 */
typedef union{
    volatile Uint16               all;
    volatile STR_RUNSTATE_FLAG   bit;      
}UNI_RUNSTATE_FLAG;

/* 定义监控模块整体变量结构体类型 */
typedef struct{
 
    UNI_STOP_CONTROL_FLAG       StopCtrlFlag;  //停机控制(包含停机方式，停机状态)标志位
    STR_STOP_CONTROL_VARIABLE   StopCtrlVar;   //停机控制(包括停机方式，停机状态)变量

    UNI_SERVO_ERROR_FLAG        ErrorFlag;     //伺服故障标志位
    STR_SERVO_ERROR_VARIABLE    ErrorVar;      //伺服故障变量

    UNI_BRAKE_FLAG              BrakeFlag;     //抱闸控制标志位

    UNI_RUNSTATE_FLAG           RunStateFlag;  //伺服运行中是否使能状态标志位

    Uint8 SonDelayCnt;                     //直线电机 带无UVW省线编码器电机，上电SON角度辨识后延时用
}STR_SERVO_MONITOR;    


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用结构体类型声明 */


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_SERVO_MONITOR  STR_ServoMonitor;
extern UNI_WARN_RELEASE_FLAG  UNI_WarnReg;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void ServoMonitor(void);
extern void InitMonitorHardWare(void);
extern void ServoRunManage(void);
extern void MonitorShow(void);

#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_ServoMonitor.h */

/********************************* END OF FILE *********************************/


