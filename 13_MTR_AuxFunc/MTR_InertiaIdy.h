/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                   
 文件名: MTR_InertiaIdy.h 
 创建人：朱祥华            创建日期：2011.10.31 
 修改人：朱祥华            修改日期：2012.03.13                                                
 描述：惯量辨识程序变量及函数声明 
    1.
    2.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef MTR_INERTIAIDY_H
#define MTR_INERTIAIDY_H

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
//杜金明在线惯量辨识变量
typedef struct{

//******************在线式惯量辨识标志**************************    

    Uint16     Forward:1;             //正转标志 
    
    Uint16     SpeedUp:1;            //加速标志

    Uint16     Forward_SpeedUp_Mature:1;       //正向加速成熟标志

    Uint16     Forward_SpeedDown_Mature:1;     //正向减速成熟标志

    Uint16     Reverse_SpeedUp_Mature:1;       //反向加速成熟标志

    Uint16     Reverse_SpeedDown_Mature:1;     //反向减速成熟标志

    Uint16     Enable_Calculation:1;           //可计算惯量
    
    Uint16     SampleOk_Flag:1;                //采样数据完成标志

    Uint16     Status:2;                       //在线辨识状态

//***************************************************************    
    Uint16  Rsvd:6;
}STR_OnLnInertia_Bit;

typedef union{
    volatile Uint16                  all;
    volatile STR_OnLnInertia_Bit     bit;
}UNI_OnLnInertia_Reg;

//在线惯量辨识结构体
typedef struct{  //

               Uint64   CalCoef_Q30;        //惯量计算系数Q30格式
               Uint16    CurrentState;       //当次状态（依据加减速、正反转划分）
               Uint16   BeforeState;        //前次状态
               int32    Idntfy_DltaSpd;     //每次加速速度
            
//*****************************信息单元记录***************************************

               Uint16  SpdPeriodCnt;              //记录经历的速度环周期数量
               int32   DeltaIqFdbadd;              //每个单元量里面的电流和
               int32   OriginSpdFdb;              //起始速度
               int32   TerminationSpdFdb;         //终止速度

               int32   firstHalfSpdFdbAdd;          //前半程速度反馈和，用来判断速度变化方向
               int32   secondHalfSpdFdbAdd;          //后半程速度反馈和

               int32   firstHalfSpdRefAdd;          //前半程速度指令和，用来判断速度变化方向
               int32   secondHalfSpdRefAdd;          //后半程速度指令和

               int32   firstHalfPosRefAdd;          //前半程速度指令和，用来判断速度变化方向
               int32   secondHalfPosRefAdd;          //后半程速度指令和
               
         /*采样完200数据后锁存当前数据值用于主循环处理*/
               int32   SpdFdbLatch;               
               int32   DeltaIqFdbaddLatch;             //单元电流和清零
               int32   firstHalfSpdFdbAddLatch ;           //前半程速度和清零
               int32   secondHalfSpdFdbAddLatch;           //后半程速度和清零
               int32   secondHalfSpdRefAddLatch;
               int32   firstHalfSpdRefAddLatch ;
               int32   secondHalfPosRefAddLatch ;
               int32   firstHalfPosRefAddLatch ;
               int32   IqCompLatch;
               int32   SpdCompLatch;
               
//*****************************低速辨识补偿变量***********************************//    
                 
               int32   SpdComp;                      //速度补偿量
               int32   IqComp;                      //电流补偿
               int32   OriginIqComp;              //起始电流补偿
               int32   TerminationIqComp;          //终止电流补偿

//*******************************正向转动*****************************************

               int64   Forward_SpdUp_IqFdbadd;    //正向加速过程电流和
               int16   Forward_SpeedUpCnt;        //加速单元次数
               int16   Forward_SpeedUpCnt1;        //加速单元次数1
               int32   Forward_SpeedUpAdd;          //正向加速累积

               int64   Forward_SpdDown_IqFdbadd;  //正向减速过程电流和
               int16   Forward_SpeedDownCnt;      //减速单元次数
               int32   Forward_SpeedDownAdd;      //正向减速累计量

//*******************************反向转动***************************************** 

               int64   Reverse_SpdUp_IqFdbadd;       //加速过程电流和
               int16   Reverse_SpeedUpCnt;           //加速单元次数
               int32   Reverse_SpeedUpAdd;             //反向加速累积量

               int64   Reverse_SpdDown_IqFdbadd;  //减速过程电流和
               int16   Reverse_SpeedDownCnt;      //减速单元次数
               int32   Reverse_SpeedDownAdd;       //反向减速累积


//*******************************计算用变量****************************************

               int64   Compute_IqFdb1;                //计算用第一段电流
               int64   Compute_IqFdb2;                //计算用第二段电流
               int32   Compute_DeltaSpd1;            //计算用第一段速度变化量
               int32   Compute_DeltaSpd2;            //计算用第二段速度变化量
               int16   Compute_CellCnt1;            //计算用第一段单元个数   
               int16   Compute_CellCnt2;             //计算用第二段单元个数  
               int32   Compute_AverageIqFdb1;        //计算用第一段电流/单元格个数
               int32   Compute_AverageIqFdb2;        //计算用第二段电流/单元格个数
               int32   Compute_AverageDeltaSpd1;    //计算用第一段速度变化量/单元格个数
               int32   Compute_AverageDeltaSpd2;    //计算用第二段速度变化量/单元格个数

                Uint16     ForbidCnt;                           //禁止辨识惯量

//*********************************************************************************
               UNI_OnLnInertia_Reg  Flag;      //在线辨识状态标志

               int32  InertiaOut4;   //在线惯量辨识计算中间量
               int32  InertiaOut5;
               int32  InertiaOut6;

               Uint16  InertiaRatioNum;       //完整辨识次数计数
             
}STR_OnLnInertia;


/****************************************************************************************
****************************************************************************************/
/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */ 
//暂无

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void Init_OnLineInertiaIdy(void);             //在线惯量辨识上电初始化程序
extern void OnLnInertia_Sample(void);                //在线惯量辨识信息采集程序
extern void OnLnInertia_MainLoopSchedule(void);     //在线惯量辨识主循环调度处理程序

//extern 

#ifdef __cplusplus
}
#endif

#endif  /* MTR_InertiaIdy.h */
/********************************* END OF FILE *********************************/
