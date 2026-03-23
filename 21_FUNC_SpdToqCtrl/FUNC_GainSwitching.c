/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_GainSwitching.c  
 创建人：   姚虹                   创建日期：2011.11.31
 修改人：   王治国                 修改日期：2012.02.05
 描述：     
    1. 获取位置环增益和速度环增益

 修改记录：
    2012.02.05      王治国
    1.功能码H0810更改如下：
      //0 禁止增益切换，固定到第1增益
      //1 禁止增益切换，固定到第2增益
      //2 根据速度指令对速度环比例、积分增益切换
    2.加入位置环增益切换方式    姚虹  2012.12.14
    2013.05.20      王治国
    1.参考松下，修改增益切换功能
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_GlobalVariable.h" 
#include "FUNC_InterfaceProcess.h"  
#include "FUNC_FunCode.h"
#include "FUNC_GainSwitching.h"
#include "FUNC_ErrorCode.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

//定义增益切换结构体变量,本文件内部使用
STR_GAINSWITCHINGLOCALVARIABLE      GnSWLocalVar;


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
static Uint8 LatchFlag = 0;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void GainSwitchingUpdate(void);
void GainSwitching(void);


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void GnSw_SpdRef(void);
Static_Inline void GetGnFir(void);
Static_Inline void GetGnSec(void);
Static_Inline void GnFirToSec(void);
Static_Inline void GnSecToFir(void);
/*******************************************************************************
  函数名: void GainSwitchingUpdate(void)
  输入:    
  输出:    
  子函数:         
  描述:   

********************************************************************************/ 
void GainSwitchingUpdate(void)
{
    static Uint8 FirOnPower = 0;
    int64 DeltaSpd = 0;
    int32 Temp = 0; 
    int32 GnSwLvl = 0;
    int32 GnSwHysteresis = 0;

    STR_FUNC_Gvar.GainSW.SpdKf_Q12 = (int16)(((Uint32)FunCodeUnion.code.GN_Pdff_Kf << 12) /1000L); // Disable时为4096
	// Disable时为0，Max为4096
	STR_FUNC_Gvar.GainSW.SpdDampingKf_Q12 = (int16)(((Uint32)FunCodeUnion.code.GN_Damping_Kf << 12) /1000L);  

    //=================更新增益切换模式=================
    if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod == TOQMOD)
    {
        //转矩模式不支持增益切换
        GnSWLocalVar.GnSwModeSel = 0;
    }
    else if(0 == FunCodeUnion.code.GN_Gn2ndSetup)
    {
        GnSWLocalVar.GnSwModeSel = 1;
    }
    else
    {
        if(0 == FunCodeUnion.code.GN_GnSwModeSel) GnSWLocalVar.GnSwModeSel = 0;
        else GnSWLocalVar.GnSwModeSel = FunCodeUnion.code.GN_GnSwModeSel + 1;
    }

    //等级、时滞处理，等级＜迟滞的情况时，在内部重新设定为迟滞＝等级
    GnSwLvl = FunCodeUnion.code.GN_GnSwLvl;
    GnSwHysteresis = FunCodeUnion.code.GN_GnSwHysteresis;
    if(GnSwLvl < GnSwHysteresis)  GnSwHysteresis = GnSwLvl;

    //=================增益根据速度指令高低阈值切换参数更新=================
    //锁存设定的速度阈值
    Temp = (int32)GnSwLvl - (int32)GnSwHysteresis;    

    GnSWLocalVar.SpdRefLvlMin = 10000L * Temp;

    Temp = (int32)GnSwLvl + (int32)GnSwHysteresis;
    GnSWLocalVar.SpdRefLvlMax = 10000L * Temp;

    //求速度比例增益
    GnSWLocalVar.LowSpdKp_Q30 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st) << 30;
    GnSWLocalVar.HighSpdKp_Q30 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.SpdKp2nd) << 30;

    //求速度积分增益
    GnSWLocalVar.LowSpdKi_Q40 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10) << 30;
    GnSWLocalVar.HighSpdKi_Q40 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.SpdKi2nd_Q10) << 30;

    //求位置比例增益
    GnSWLocalVar.LowPosKp_Q30 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.PosKp1st) << 30;
    GnSWLocalVar.HighPosKp_Q30 = (int64)(STR_FUNC_Gvar.RigidityLvlTab.PosKp2nd) << 30;

    //求增益切换速度阈值差
    DeltaSpd = GnSWLocalVar.SpdRefLvlMax - GnSWLocalVar.SpdRefLvlMin;

    //求两个速度比例增益切换斜率系数
    GnSWLocalVar.SpdKpCoef_Q30 = (int64)((GnSWLocalVar.HighSpdKp_Q30 - GnSWLocalVar.LowSpdKp_Q30) / DeltaSpd);

    //求两个速度积分增益切换斜率系数
    GnSWLocalVar.SpdKiCoef_Q40 = (int64)((GnSWLocalVar.HighSpdKi_Q40 - GnSWLocalVar.LowSpdKi_Q40) / DeltaSpd);

    //求两个位置比例增益切换斜率系数
    GnSWLocalVar.PosKpCoef_Q30 = (int64)((GnSWLocalVar.HighPosKp_Q30 - GnSWLocalVar.LowPosKp_Q30) / DeltaSpd);

    //=================参考松下切换方法参数更新=================
    //获取位置增益切换时间和增益切换延迟时间
    GnSWLocalVar.PosGnSwTmCnt    = ((Uint32)FunCodeUnion.code.GN_PosGnSwTm * STR_FUNC_Gvar.System.PosFreq) / (1000 * 10);
    GnSWLocalVar.GnSwDlyTmCnt = ((Uint32)FunCodeUnion.code.GN_GnSwDlyTm * STR_FUNC_Gvar.System.PosFreq) / (1000 * 10);


   //获取增益切换时位置增益递增量
    GnSWLocalVar.DelPosKp_Q10  = ((int32)((int16)FunCodeUnion.code.GN_Pos_Kp2 - (int16)FunCodeUnion.code.GN_Pos_Kp) << 10) / GnSWLocalVar.PosGnSwTmCnt;

     //需要保证切换水平大于滞环大小
    if(GnSwLvl < GnSwHysteresis)   
    {
        GnSwHysteresis = GnSwLvl;
    }

    //获取增益切换时转矩切换点 1%
    GnSWLocalVar.ToqSwPointH = (int16)(GnSwLvl + GnSwHysteresis) * 10L;
    GnSWLocalVar.ToqSwPointL = (int16)(GnSwLvl - GnSwHysteresis) * 10L;

    //获取增益切换时速度指令切换点  rpm
    GnSWLocalVar.SpdRefSwPointH = (int32)(int16)(GnSwLvl + GnSwHysteresis) * 10000L;
    GnSWLocalVar.SpdRefSwPointL = (int32)(int16)(GnSwLvl - GnSwHysteresis) * 10000L;

    //获取增益切换时速度指令变化率切换点  rpm / ms
    GnSWLocalVar.DelSpdRefSwPointH = GnSWLocalVar.SpdRefSwPointH * 10L / 1000L;
    GnSWLocalVar.DelSpdRefSwPointL = GnSWLocalVar.SpdRefSwPointL * 10L / 1000L;
    GnSWLocalVar.DelSpdRefSwTmCnt = STR_FUNC_Gvar.System.PosFreq / 1000L;

    //获取增益切换时速度反馈切换点  rpm
    GnSWLocalVar.SpdFdbSwPointH = GnSWLocalVar.SpdRefSwPointH;
    GnSWLocalVar.SpdFdbSwPointL = GnSWLocalVar.SpdRefSwPointL;
    GnSWLocalVar.SpdFdbSwPointM = (GnSWLocalVar.SpdFdbSwPointH + GnSWLocalVar.SpdFdbSwPointL) >> 1;

    //获取增益切换换时位置偏差切换点 编码器单位
    GnSWLocalVar.PosErrPointH = (int32)GnSwLvl + GnSwHysteresis;
    GnSWLocalVar.PosErrPointL = (int32)GnSwLvl - GnSwHysteresis;

    //增益切换时定位完成幅度，需要考虑编码器分辨率因素
    GnSWLocalVar.PosReachPoint = (int32)FunCodeUnion.code.PL_PosReachValue;


    //上电初始化时增益先固定在第一组
    if(0 == FirOnPower)
    {
        FirOnPower = 1;
        GetGnFir();
    }
}

/*******************************************************************************
  函数名: void GainSwitching(void)
  输入:    
  输出:    
  子函数:         
  描述:   

********************************************************************************/
void GainSwitching(void)
{
    static int32 DelSpdRef = 0;

    //求得每1ms速度指令变化率
    GnSWLocalVar.DelSpdRefSwRealTmCnt ++;
    if(GnSWLocalVar.DelSpdRefSwRealTmCnt == GnSWLocalVar.DelSpdRefSwTmCnt)
    {
        DelSpdRef = STR_FUNC_Gvar.SpdCtrl.SpdRef - GnSWLocalVar.DelSpdRefSwRefLatch;

        GnSWLocalVar.DelSpdRefSwRealTmCnt = 0;
        GnSWLocalVar.DelSpdRefSwRefLatch = STR_FUNC_Gvar.SpdCtrl.SpdRef;
    }

    //第一组向第二组增益切换
    if(1 == GnSWLocalVar.GnFirToSecFlag)
    {
        GnFirToSec();
        return;
    }

    // 获取增益
    switch(GnSWLocalVar.GnSwModeSel)   
    {
        case 0: //第一组增益固定
            GetGnFir();
			LatchFlag = 0;
            break;
                    
        case 1: //P-PI切换 
            STR_FUNC_Gvar.GainSW.SpdKp = STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st;
            STR_FUNC_Gvar.GainSW.PosKp = STR_FUNC_Gvar.RigidityLvlTab.PosKp1st;
            STR_FUNC_Gvar.GainSW.ToqRefFilterTc = STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc1st;  //单位us        

            if(0 == STR_FUNC_Gvar.DivarRegLw.bit.GainSel)   //PI
            {
                STR_FUNC_Gvar.GainSW.SpdKi_Q10 = STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10; 
            }
            else   //P
            {
                STR_FUNC_Gvar.GainSW.SpdKi_Q10 = 0;
            }
			LatchFlag = 0;
            break;

        case 2: //使用DI切换
            if(0 == STR_FUNC_Gvar.DivarRegLw.bit.GainSel)   //第一组
            {
                GetGnFir();
				LatchFlag = 0;
            }
            else   //第二组
            {
                GetGnSec();
				LatchFlag = 1;
            }
			
            break;

        case 3:   //根据转矩指令切换
            if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) > GnSWLocalVar.ToqSwPointH) //滞环处理，大于高点时使用第二增益
            {
                GnFirToSec();
            }
            else if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) <= GnSWLocalVar.ToqSwPointL)
            {
                GnSecToFir();
            }
			LatchFlag = 1;
            break;

        case 4:   //根据速度指令切换
            if(ABS(STR_FUNC_Gvar.SpdCtrl.SpdRef) > GnSWLocalVar.SpdRefSwPointH) //滞环处理，大于高点时使用第二增益
            {
                GnFirToSec();
            }
            else if(ABS(STR_FUNC_Gvar.SpdCtrl.SpdRef) <= GnSWLocalVar.SpdRefSwPointL)
            {
                GnSecToFir();
            }
			LatchFlag = 1;
            break;

        case 5:   //根据速度指令变化率切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != SPDMOD)
            {
                //第一组增益固定
                GetGnFir();
                LatchFlag = 0;
            }
            else
            {
                if(ABS(DelSpdRef) > GnSWLocalVar.DelSpdRefSwPointH) 
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else if(ABS(DelSpdRef) <= GnSWLocalVar.DelSpdRefSwPointL)
                {
                    GnSecToFir();
					LatchFlag = 1;
                }
            }
			
            break;

        case 6:   //根据速度指令高低阈值切换
                GnSw_SpdRef();
            break;

        case 7:   //根据位置偏差切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)
            {
                //第一组增益固定
                GetGnFir();
				LatchFlag = 0;
            }
            else
            {
                if(ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) > GnSWLocalVar.PosErrPointH) 
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else if(ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) <= GnSWLocalVar.PosErrPointL)
                {
                    GnSecToFir();
					LatchFlag = 1;
                }
            }
			
            break;

        case 8:   //根据有无位置指令切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)
            {
                //第一组增益固定
                GetGnFir();
				LatchFlag = 0;
            }
            else
            {
                if(STR_FUNC_Gvar.PosCtrl.PosRef != 0)
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else
                {
                    GnSecToFir();
					LatchFlag = 1;
                }
            }
            break;

        case 9:   //根据定位完成信号切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)
            {
                //第一组增益固定
                GetGnFir();
				LatchFlag = 0;
            }
            else
            {
                if(ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) > GnSWLocalVar.PosReachPoint)
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else
                {
                    GnSecToFir();
					LatchFlag = 1;
                }
            }
            break;

        case 10:  //根据速度反馈指令切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)
            {
                //第一组增益固定
                GetGnFir();
				LatchFlag = 0;
            }
            else
            {
                if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) > GnSWLocalVar.SpdFdbSwPointH)
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) <= GnSWLocalVar.SpdFdbSwPointL)
                {
                    GnSecToFir();
					LatchFlag = 1;
                }
            }
            break;

        case 11:  //根据位置指令+速度反馈指令切换
            if(STR_FUNC_Gvar.MonitorFlag.bit.RunMod != POSMOD)
            {
                //第一组增益固定
                GetGnFir();
				LatchFlag = 0;
            }
            else
            {
                if(STR_FUNC_Gvar.PosCtrl.PosRef != 0)
                {
                    GnFirToSec();
					LatchFlag = 1;
                }
                else    //当位置指令为零且速度反馈小于低阈值时
                {
                    GnSecToFir();                                      
					LatchFlag = 1;                                      
                }
            }
            break;
        default:
            //第一组增益固定
            GnSWLocalVar.GnSwModeSel = 0;
            GetGnFir();
			LatchFlag = 0;
            break;
    }      
}

/*******************************************************************************
  函数名: void GainSwitching_SpdRef(void)
  输入:    
  输出:    
  子函数:         
  描述: 依据速度指令进行速度环增益切换
********************************************************************************/
Static_Inline void GnSw_SpdRef(void)
{
    int64 DeltaSpd = 0;
    int64 Temp = 0;

    if(ABS(STR_FUNC_Gvar.SpdCtrl.SpdRef) <= GnSWLocalVar.SpdRefLvlMin)
    {
        GetGnFir();
		LatchFlag = 0;
    }
    else if(ABS(STR_FUNC_Gvar.SpdCtrl.SpdRef) >= GnSWLocalVar.SpdRefLvlMax)
    {
        GetGnSec();
		LatchFlag = 1;
    }
    else
    {
        DeltaSpd = ABS(STR_FUNC_Gvar.SpdCtrl.SpdRef) - GnSWLocalVar.SpdRefLvlMin;

        Temp = (int64)DeltaSpd * GnSWLocalVar.SpdKpCoef_Q30;
        STR_FUNC_Gvar.GainSW.SpdKp = (int32)((GnSWLocalVar.LowSpdKp_Q30 + Temp) >> 30);

        Temp = (int64)DeltaSpd * GnSWLocalVar.SpdKiCoef_Q40;
        STR_FUNC_Gvar.GainSW.SpdKi_Q10 = (int32)((GnSWLocalVar.LowSpdKi_Q40 + Temp) >> 30);

        Temp = (int64)DeltaSpd * GnSWLocalVar.PosKpCoef_Q30;
        STR_FUNC_Gvar.GainSW.PosKp = (int32)((GnSWLocalVar.LowPosKp_Q30 + Temp) >> 30); 
		LatchFlag = 1; 
    }
}

/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  描述: 
********************************************************************************/
Static_Inline void GetGnFir(void)
{
    //第一组增益
    STR_FUNC_Gvar.GainSW.SpdKp = STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st;
    STR_FUNC_Gvar.GainSW.PosKp = STR_FUNC_Gvar.RigidityLvlTab.PosKp1st;
    STR_FUNC_Gvar.GainSW.ToqRefFilterTc = STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc1st;  //单位us        
    STR_FUNC_Gvar.GainSW.SpdKi_Q10 = STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10; 
}

/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  描述: 
********************************************************************************/
Static_Inline void GetGnSec(void)
{
    //第二组增益
    STR_FUNC_Gvar.GainSW.SpdKp = STR_FUNC_Gvar.RigidityLvlTab.SpdKp2nd;
    STR_FUNC_Gvar.GainSW.PosKp = STR_FUNC_Gvar.RigidityLvlTab.PosKp2nd;
    STR_FUNC_Gvar.GainSW.ToqRefFilterTc = STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc2nd;  //单位us        
    STR_FUNC_Gvar.GainSW.SpdKi_Q10 = STR_FUNC_Gvar.RigidityLvlTab.SpdKi2nd_Q10;
}



/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  描述:  第一增益向第二增益的切换,或者保持为第二增益
********************************************************************************/
Static_Inline void GnFirToSec(void)
{

    GnSWLocalVar.RealDlyTmCnt = 0;

    if(GnSWLocalVar.RealTmCnt < GnSWLocalVar.PosGnSwTmCnt)    //增益切换时间段内，增益平滑变换
    {
        GnSWLocalVar.RealTmCnt++;
        STR_FUNC_Gvar.GainSW.PosKp = (int16)FunCodeUnion.code.GN_Pos_Kp + (int32)((GnSWLocalVar.RealTmCnt * GnSWLocalVar.DelPosKp_Q10) >> 10);
        GnSWLocalVar.GnFirToSecFlag = 1;
    }
    else                                      //切换时间走完或者从来没进行过切换，直接变成第二增益
    {
        STR_FUNC_Gvar.GainSW.PosKp  = FunCodeUnion.code.GN_Pos_Kp2;
        GnSWLocalVar.GnFirToSecFlag = 0;        
    }   

    STR_FUNC_Gvar.GainSW.SpdKp = STR_FUNC_Gvar.RigidityLvlTab.SpdKp2nd;
    STR_FUNC_Gvar.GainSW.ToqRefFilterTc = STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc2nd;  //单位us        
    STR_FUNC_Gvar.GainSW.SpdKi_Q10 = STR_FUNC_Gvar.RigidityLvlTab.SpdKi2nd_Q10;
}

/*******************************************************************************
  函数名: 
  输入:    
  输出:    
  描述: 第二增益向第一增益的切换，或者保持为第一增益
********************************************************************************/
Static_Inline void GnSecToFir(void)
{     
    if(LatchFlag==0)
    {
        GnSWLocalVar.RealDlyTmCnt = GnSWLocalVar.GnSwDlyTmCnt;
    }

    if(GnSWLocalVar.RealDlyTmCnt < GnSWLocalVar.GnSwDlyTmCnt)      //延时时间未到时
    {
        GetGnSec();
        GnSWLocalVar.RealDlyTmCnt++; 
    }
    else                                            //延时时间到或者没切换过，直接赋值为第一增益
    {
        if(11 != GnSWLocalVar.GnSwModeSel)
        {
            GetGnFir();
        }
        else
        {
            //第一组增益
            if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) < GnSWLocalVar.SpdFdbSwPointM)
            {
                STR_FUNC_Gvar.GainSW.SpdKp = STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st;
                STR_FUNC_Gvar.GainSW.PosKp = STR_FUNC_Gvar.RigidityLvlTab.PosKp1st;
                STR_FUNC_Gvar.GainSW.ToqRefFilterTc = STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc1st;  //单位us
            }
                     
            if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) <= GnSWLocalVar.SpdFdbSwPointL)
            {
                STR_FUNC_Gvar.GainSW.SpdKi_Q10 = STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10;
            }
        }

        GnSWLocalVar.RealTmCnt = 0;                    
    }    
}
/********************************* END OF FILE *********************************/
