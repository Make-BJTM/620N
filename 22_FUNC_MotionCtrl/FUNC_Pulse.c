/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_Pulse.c
 创建人：高小峰                
 修改人：李浩               创建日期：11.09.23 
 描述： 1. 
        2.
		3.
 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_Pulse.h"
#include "FUNC_FunCode.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_GPIODriver.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define POS_PULSE_OVERFLOW_MAX  0xFFFF     //脉冲串计数器的最大溢出值

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 


/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
static int32 MaxPulsFreq = 1000;  //最大位置脉冲频率=设定最大频率/位置环调度频率
#else
static int32 MaxPulsFreq = 4000;  //最大位置脉冲频率=设定最大频率/位置环调度频率
#endif
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
int32 PosPulseCalc(void);
void  MaxPulsFreqSet(void);


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */


/*******************************************************************************
  函数名:  HandWheelSample()
  输入:   STR_FUNC_Gvar.DivarRegLw.bit.Hx1 ,STR_FUNC_Gvar.DivarRegLw.bit.Hx2,
          STR_FUNC_Gvar.DivarRegLw.bit.HxEn, (手轮Di信号)
  输出:  HandCount 手轮脉冲计数  
  子函数:  GPIO_ReadDI9DSP()，GPIO_ReadDI10DSP()       
  描述:  手轮脉冲串指令采样，在手轮刚开始启动和上次AB正交脉冲值和当前AB正交脉冲值相等时
         之后的两次执行过程中不进行手轮脉冲的加减操作，当上次AB正交脉冲值和当前AB正交脉冲值
         的时序不对时，应进行警告操作，当时序正确时解除警告。
********************************************************************************/ 
/*Uint32 HandWheelSample()
{
    static int32  HandCount = 0;    //手轮正交脉冲计数值
    static Uint16 ABLast = 0;	    //上一次AB正交脉冲对应数值
    static Uint16 ABPresent;		//当前AB正交脉冲对应数值
    static Uint16 ABFilterBefore;   //AB正交脉冲滤波前的值
    static Uint16 HRatio = 1;	    //手轮倍率  
    static Uint8  CountFlag = 0;    //滤波次数,滤出手轮动作时前几次的值
    Uint16 HRatioSel;			    //手轮倍率选择
   
    if (STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)    return 0;

    if  (STR_FUNC_Gvar.DivarRegLw.bit.HxEn == 0)  return 0;

    //if ( ((FunCodeUnion.code.DIFuncSel9 == 63)&& (FunCodeUnion.code.DIFuncSel10 == 64)) || 
    //((FunCodeUnion.code.DIFuncSel9 == 64)&& (FunCodeUnion.code.DIFuncSel10 == 63)) 
    //)
    {
        HRatioSel = (STR_FUNC_Gvar.DivarRegLw.bit.Hx1 + (STR_FUNC_Gvar.DivarRegLw.bit.Hx2<<1)); //HRatioSel代表手轮的倍率 可以为10倍，100倍 
        ABFilterBefore = GPIO_ReadDI9DSP() + (GPIO_ReadDI10DSP()<<1);  //也可以按位读取，不过要小心出现没读出数据的情况；
         
        switch(HRatioSel)               //根据X1,X2的输入选择倍率
        {
            case 1:
                HRatio = 10;
                break;
            case 2:
                HRatio = 100;
                break;
            default: 
                HRatio = 1;
                break;
        } 
        if(CountFlag > 2)    // 滤波次数达到规定的次数后，才进行手轮脉冲的加减操作
        {
            ABPresent = ABFilterBefore;
            if (ABPresent == ABLast)     //如果手轮AB正交输入无变化时，下一次动作变化前，需要再次滤波
            {
                CountFlag = 0;
            }
            else
            {
                if((ABPresent == 0 && ABLast == 1) || (ABPresent == 2 && ABLast == 0) ||  
                   (ABPresent == 3 && ABLast == 2) || (ABPresent == 1 && ABLast ==3))   //根据信号判断转速方向，该转速为正转，加上脉冲数
                { 
                    //WarnReg.bit.HANDWHEELWARNFLG = 0;  //解除警告
                    HandCount += HRatio;
                }
                else if((ABLast == 0 && ABPresent == 1) || (ABLast == 2 && ABPresent == 0) || 
                        (ABLast == 3 && ABPresent == 2) || (ABLast == 1 && ABPresent == 3)) //根据信号判断转速方向，该转速为反转，减去脉冲数
                {
                    //WarnReg.bit.HANDWHEELWARNFLG = 0;  //解除警告
                    HandCount -= HRatio;
                }
                else
                {
                    //WarnReg.bit.HANDWHEELWARNFLG = 1;  //置警告标志位
                    //PostErrMsg(HANDWHEELWARN);    //速度转得太快导致数值不是顺序变化，或者正反向切换速度过快；此时应进行警告处理
                                                    //注明此处加警告处理，警告信息会储存在Eepro，由于该警告事件可能发生，当发生次数较多时，会导致Eeprom参数故障
                }
                ABLast = ABPresent;
            }                              
        }
        else
        {
            CountFlag++;
            ABLast = ABFilterBefore;   
        }    

        return HandCount;
    }
    //else
    //{
        //return 0;
    //}
}
*/
/*******************************************************************************
  函数名: PosPulsCal()
  输入:    
  输出:   返回16位有符号位置指令  
  子函数: HandWheelSample()        
  描述:   脉冲串指令采样，在手轮和PLC位置脉冲指令增量的计算
********************************************************************************/ 
int32 PosPulseCalc()
{
    
    #if DRIVER_TYPE == SERVO_620P_407VG
	int32    DeltaPulse = 0;            //位置脉冲指令 增量式
    static int32  PulseCntLast = 0;
    static int8   OverFreqCntL = 0;

    int32    DeltaPulse2 = 0;            //位置脉冲指令 增量式
    static int32  PulseCntLast2 = 0;
	#endif

    #if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    int32    DeltaPulse3 = 0;            //位置脉冲指令 增量式
    static int32  PulseCntLast3 = 0;
	#endif

	//第一次进入该函数需对锁存变量赋值
    static Uint8 FirstGetPosRefFlag = 1;
    static int8   OverFreqCntH = 0;
    //没有选择脉冲指令输入情况下

    #if DRIVER_TYPE == SERVO_620P_407VG
    if(((FunCodeUnion.code.PL_PosSurcASel != 0) && (STR_FUNC_Gvar.DivarRegLw.bit.HxEn != 1)) ||(FirstGetPosRefFlag == 1))
    {
        PulseCntLast = STR_InnerGvarPosCtrl.PulsePosRef;          //绝对脉冲指令锁存
        PulseCntLast2 = STR_InnerGvarPosCtrl.PulsePosRef2;          //绝对脉冲指令锁存
        FirstGetPosRefFlag  = 0;
        return 0;
    }

    //低速位置脉冲指令	  
    DeltaPulse = STR_InnerGvarPosCtrl.PulsePosRef - PulseCntLast;      //计算增量式位置脉冲指令     
    PulseCntLast = STR_InnerGvarPosCtrl.PulsePosRef;          //绝对脉冲指令锁存
	
	if((DeltaPulse > MaxPulsFreq)
	  ||(DeltaPulse < -MaxPulsFreq))
	{
		 OverFreqCntL++;
         if(OverFreqCntL>3)
         {
             OverFreqCntL=4;
             PostErrMsg(PULSINERR);
         }
         
	}
    else
    {
        OverFreqCntL=0;
    }

    //高速位置脉冲指令	  
    DeltaPulse2 = STR_InnerGvarPosCtrl.PulsePosRef2 - PulseCntLast2;      //计算增量式位置脉冲指令   
    PulseCntLast2 = STR_InnerGvarPosCtrl.PulsePosRef2;          //绝对脉冲指令锁存
	
	if((DeltaPulse2 > MaxPulsFreq)
	  ||(DeltaPulse2 < -MaxPulsFreq))
	{
		 OverFreqCntH++;
         if(OverFreqCntH>3)
         {
            OverFreqCntH=4;
            PostErrMsg(PULSINERR);
         }
         
	}
    else
    {
        OverFreqCntH = 0;
	}

    if( (0 == FunCodeUnion.code.PL_PulsePosCmdSel) ||
        (15 > FunCodeUnion.code.OEM_FpgaVersion) )     //旧版本
    {
        //低速位置脉冲输入口
        STR_InnerGvarPosCtrl.DeltaPulse = DeltaPulse;	
        return DeltaPulse;
    }
    else
    {
        //高速位置脉冲口输入
        STR_InnerGvarPosCtrl.DeltaPulse = DeltaPulse2;	
        return DeltaPulse2;    
    }  
	#endif

    #if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))

    if(((FunCodeUnion.code.PL_PosSurcASel != 0) && (STR_FUNC_Gvar.DivarRegLw.bit.HxEn != 1)) ||(FirstGetPosRefFlag == 1))
    {
        PulseCntLast3 = STR_InnerGvarPosCtrl.PulsePosRef3;          //绝对脉冲指令锁存
        FirstGetPosRefFlag  = 0;
        return 0;
    }


    //高速位置脉冲指令

    DeltaPulse3 = STR_InnerGvarPosCtrl.PulsePosRef3 - PulseCntLast3;      //计算增量式位置脉冲指令   
    PulseCntLast3 = STR_InnerGvarPosCtrl.PulsePosRef3;          //绝对脉冲指令锁存
	
	if((DeltaPulse3 > MaxPulsFreq)
	  ||(DeltaPulse3 < -MaxPulsFreq))
	{
		 OverFreqCntH++;
         if(OverFreqCntH>3)
         {
            OverFreqCntH=4;
            PostErrMsg(PULSINERR);
         }
         
	}
    else
    {
        OverFreqCntH = 0;
	}

    //高速位置脉冲口输入
    STR_InnerGvarPosCtrl.DeltaPulse = DeltaPulse3;	
    return DeltaPulse3;    
#endif
}

/*******************************************************************************
  函数名: MaxPulsFreqSet(Uint16 PulsFreq，Uint16 PosFreq)
  输入:   设定的最大脉冲频率, 位置环频率   
  输出:   MaxPulsFreq
  子函数:         
  描述:   脉冲指令输入的最大频率换算  最大位置脉冲频率=设定最大频率/位置环调度频率
********************************************************************************/ 

void  MaxPulsFreqSet(void)
{
    if(FunCodeUnion.code.PL_PosPulsInMode==2)//4倍频
    {
        MaxPulsFreq = ((Uint32)FunCodeUnion.code.ER_MaxPulsFreq * 4000 / STR_FUNC_Gvar.System.PosFreq);
    }
    else
    {
        MaxPulsFreq = ((Uint32)FunCodeUnion.code.ER_MaxPulsFreq * 1000 / STR_FUNC_Gvar.System.PosFreq);
    }
    
}
/********************************* END OF FILE *********************************/

