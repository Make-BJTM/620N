/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_PosCmdFilter.c
 创建人：高小峰                
 修改人：李浩               创建日期：12.05.30 
         姚虹               2013-7-24
 描述： 1. 
        2.
		3.
 修改记录：  
     1）2013-7-24  
        加入位置时滞滤波器，用以抑制伺服低频摆动
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_PosCmdFilter.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 


/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void PosSmoothFilter(STR_POS_AVERAGE_FILTER *p);
void ResetPosSmoothFilter(STR_POS_AVERAGE_FILTER *p);


void InitPosLowPassFilter(STR_POS_LOWPASS_FILTER *p);
void ResetPosLowPassFilter(STR_POS_LOWPASS_FILTER *p);
void PosCmdLowPassFilter(STR_POS_LOWPASS_FILTER *p);



/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */


/*******************************************************************************
  函数名:  PosSmoothFilter()
  输入:    STR_PosCtrl.PosMultipleCmd(经过电子齿轮后的放大了的位置指令)
  输出:    STR_FUNC_Gvar.PosCtrl.PosRef (经过平滑滤波后的位置指令，该位置指令为全局位置指令)
  子函数:  无      
  描述:    对经过电子齿轮后的位置指令进行平均值平滑滤波
********************************************************************************/
void PosSmoothFilter(STR_POS_AVERAGE_FILTER *p)
{

    if(p->UnfullBuffCnt < p->FilterTime) 
    {
        p->UnfullBuffCnt++;    //刚上电时，滤波缓冲数组还未填满时，滤波计数器计数，并将当前位置指令存入缓冲数组
    }
    else
    {
        if(p->FilterBuffCnt >= p->FilterTime)  //当滤波缓冲数组中填满数据，应对滤波计数器清零
        {
            p->FilterBuffCnt = 0;
        }
        p->CmdBuffSum -= p->CmdBuffArr[p->FilterBuffCnt];    //当滤波缓冲数组中填满数据，缓冲数组的数据总和，应剔除相应的旧数据
    }
    p->CmdBuffArr[p->FilterBuffCnt] = p->Input;        //相应的滤波缓冲数组中存入当前位置指令 

    p->FilterBuffCnt++;

    p->CmdBuffSum += p->Input;                        //缓冲数组的数据总和加入当前新的位置指令数据

    p->Output = (int32)(((int64)(p->CmdRemainderSum + p->CmdBuffSum) * (int64)p->FltTmeDiv) >> 15);  //计算位置指令平滑滤波输出

    p->Remainder = p->CmdBuffSum - p->Output * p->FilterTime;                   //计算位置指令平滑滤波器余数

    p->CmdRemainderSum += p->Remainder;                                           //计算位置指令平滑滤波器余数总和

}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:  
  描述:    
********************************************************************************/
void ResetPosSmoothFilter(STR_POS_AVERAGE_FILTER *p)
{
    /*位置平滑滤波器清零*/
    p->CmdBuffSum = 0;                //平滑滤波器数组累加和清零
    p->CmdRemainderSum = 0;           //平滑滤波器余数累加和清零
    p->Remainder = 0;                 //平滑滤波器余数清零
    p->UnfullBuffCnt = 0;            
    p->FilterBuffCnt = 0;

}


/*******************************************************************************
  函数名: void InitPosLowPassFilter(STR_BILINEAR_POSLOWPASS_FILTER *p) 
  输入:   无
  输出:   无
  子函数:  无
  描述: 位置环调用的一阶低通滤波器，考虑定点除法余数补偿的情况 

********************************************************************************/ 
void InitPosLowPassFilter(STR_POS_LOWPASS_FILTER *p)
{
    if (p->Tc > p->Ts)
    {
        p->Ka = (p->Tc + p->Ts)/p->Ts;
    }
    else
    {
        p->Ka = 1;
    }
}

/*******************************************************************************
  函数名: void ResetPosLowPassFilter(STR_POS_LOWPASS_FILTER *p)
  输入:   无
  输出:   无
  子函数: 无
  描述:   位置环调用的一阶低通滤波器 后向差分变换
    1.  Y/X=1/(1+Tc*s)      s=(z-1)/(Ts*z)
        Y/X=Ts*z/(Ts*z+Tc*(z-1))
        Y*(Tc+Ts) - Y*Tc/z = X*Ts
        Y*(Tc+Ts) = X*Ts + Y*Tc/z
        y(n)*(Tc+Ts)=x(n)*Ts + y(n-1)*Tc
        y(n)=x(n)*Ts/(Tc+Ts) + y(n-1)*Tc/(Tc+Ts)
        y(n)=[x(n)-y(n-1)]*Ts/(Tc+Ts) + y(n-1)
        y(n)=[x(n)-y(n-1)]/ka + y(n-1)
        ka= (Tc+Ts)/Ts 
********************************************************************************/
void PosCmdLowPassFilter(STR_POS_LOWPASS_FILTER *p)
{
    int64  Temp = 0;
    int64  OutputLatch = 0;  

    OutputLatch = p->OutputQ14;

    Temp = ((int64)p->Input<<14) + p->RemainderQ14  - OutputLatch;
    

    p->OutputQ14 = (Temp / p->Ka) + OutputLatch;
    p->RemainderQ14 = Temp -(p->OutputQ14 - OutputLatch) *(int64)p->Ka;

    Temp = p->OutputQ14 + p->Remainder;
    p->Output = Temp>>14;
    p->Remainder = Temp - ((int64)p->Output<<14);

    //锁存低通滤波器余数,中断定长使用
    STR_InnerGvarPosCtrl.PosLPFilterRemainder = p->Output * p->Ka + p->Remainder - p->Output;
}

/*******************************************************************************
  函数名: void ResetPosLowPassFilter(STR_POS_LOWPASS_FILTER *p)
  输入:   无
  输出:   无
  子函数: 无
  描述:   位置低通滤波器复位

********************************************************************************/

void ResetPosLowPassFilter(STR_POS_LOWPASS_FILTER *p)
{
    p->Remainder = 0;
    p->Output = 0;
    p->RemainderQ14 = 0;
    p->OutputQ14    = 0;
    STR_InnerGvarPosCtrl.PosLPFilterRemainder = 0;
}





/********************************* END OF FILE *********************************/

