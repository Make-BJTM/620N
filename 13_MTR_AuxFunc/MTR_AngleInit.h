 /********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_AngleInit.h
 创建人：姚虹                创建日期：2012.09.17
 修改人：××××××                修改日期：××××××××××
 描述：
       1. 文件用于定义初始角度辨识所用结构体
 缩写：
       1. 
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx 
********************************************************************************/


#ifndef SERVO_ANGLEINIT_H
#define SERVO_ANGLEINIT_H

#ifdef __cplusplus
extern "C" {
#endif

//初始角定位不找Z时定义的结构体
typedef struct
{
    Uint32 StepPulsCnt;    //不找Z时每步走的电角度对应的脉冲数
    Uint32 FxdAngPoint;    //不找Z时最后定位时电角度
	Uint16 MotRunPuls;     //判断电机是否运动阈值
    int16 DutyInc;      //占空比累加量
    int16 DutyU;        //得到U相测试时占空比
    int16 PwmPrd;       //PWM周期  
    int16 Tsamp;       //电流采样转换时间
}STR_ANGINIT;



extern void AngInt_ZPosLatch(void);
extern void AngInt_IncEncResetFromZ(void);
extern void AngInt_IncEncReset(void);
extern void AngInt_CurVct(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif

