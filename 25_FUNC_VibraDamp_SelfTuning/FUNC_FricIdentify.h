 /********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_FricIdentify.h
 创建人：姚虹                创建日期：2012.09.17
 修改人：××××××                修改日期：××××××××××
 描述：
       1. 文件用于定义库仑摩擦辨识所用结构体
 缩写：
       1. 
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx 
********************************************************************************/


#ifndef SERVO_FRICIDENTIFY_H
#define SERVO_FRICIDENTIFY_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    Uint16 BP_ModeSelet;			 //模式选择
	Uint16 SL_SpdBCommandFrom;		 //速度指令来源
	Uint16 SL_SpdABSwitchFlag;		 //选择B速度来源
	Uint16 MTS_RefRunMode;
	Uint16 MTS_ExeSects;
	Uint16 MTS_RunTimeUnit;
	Uint16 MTS_RiseTime1;
	Uint16 MTS_FallTime1;
	Uint16 MTS_SEC1_Ref;
	Uint16 MTS_SEC1_RunTime;
	Uint16 MTS_SEC1_RiFaSel;
	Uint16 MTS_SEC2_Ref;
	Uint16 MTS_SEC2_RunTime;
	Uint16 MTS_SEC2_RiFaSel;
	Uint16 MTS_SEC3_Ref;
	Uint16 MTS_SEC3_RunTime;
	Uint16 MTS_SEC3_RiFaSel;
	Uint16 MTS_SEC4_Ref;
	Uint16 MTS_SEC4_RunTime;
	Uint16 MTS_SEC4_RiFaSel; 
	Uint16 MTS_SEC5_Ref;
	Uint16 MTS_SEC5_RunTime;
	Uint16 MTS_SEC5_RiFaSel;

	int16  EndIdenFlag;     //辨识结束标志位
	int32  Kslope_Q10;          //库仑摩擦斜率
	int32  Fbias;           //库仑摩擦偏置  
}STR_FRIC;

extern STR_FRIC  STR_Fric;     //结构体定义
extern void Fric_Iden(void);
extern void Fric_CoffCal(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif

