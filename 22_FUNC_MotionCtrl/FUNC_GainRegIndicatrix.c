/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_Curve.c
 创建人：张小华					   创建日期；2012.09.05
 修订人：姚虹                      修订日期：2014.09.25
 描述： 
     1.JOG与定位试运行伺服实现


 修改记录：  
     1.根据一键式调整内容加入了辨识过程处理流程
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h" 
#include "FUNC_FunCode.h"
#include "FUNC_GainRegIndicatrix.h"
#include "FUNC_OperEeprom.h"
#include "FUNC_InterfaceProcess.h"
#include "PUB_Main.h"
#include "FUNC_PosCtrl.h"
#include "FUNC_Curve.h"
#include "FUNC_LineInterplt.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define    RATIOLENGTH     8   	    //用于保存的惯量比数据长度
#define    MIDCOFF_Q10     921      //由高响应到中响应切换系数0.9
#define    LOWCOFF_Q10     819      //由高响应到低响应切换系数0.8
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
STR_PERINDEX_DETECTION    STR_PerfIndexDet;
extern  STR_LOCAL_POSCONTROL     STR_PosCtrl;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */ 
Uint32 AutoTuneStatus  = 0;		 //自调整状态位，用以指示是否调整成功完成

extern STR_LINE_INTERPLT_ATTRIB  STR_CbcLnIntplt;
extern STR_CURVE_BLOCK_POS       STR_CurvePCMD;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   
void PerformanceIndexCal(void);
void PerformanceIndexInit(void);
void IndicatrixStopUpData(void);
void IndicatrixDataUp(void);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

//记录时间
Static_Inline void TimerCal(void);

//脉冲发送检测
Static_Inline void PulsCheck(void);
//平均负载率计算
Static_Inline void LoadRatioAvCal(void);

//计算定位时间
Static_Inline void PositioningTimeCal(void);

//计算过冲脉冲数
Static_Inline void OvershootPulseCal(void);
//计算振动等级
Static_Inline void OScillateLeveCal(void);

//自调整处理流程
void GainAutoTuneProcess(void);

Static_Inline void GainAutoTuneStepA(void);	    //自调整的步骤
Static_Inline void GainAutoTuneStepB(void);
Static_Inline void OscillateDeal(void);			//发生振动时的处理
Static_Inline void MaxRigidityCal(void);        //根据惯量比获取伺服系统允许设置的最大刚性
Static_Inline void GainParamSet(Uint16 Step);   //增益参数设置

//自调整过程调度函数
void GainAutoTuneProcess(void)     //增益自调整过程
{ 			   
	static Uint16 PosLagTimeStatusLast = 0;
    static Uint16 TiTuneCnt = 0;
	Uint16 i;
    Uint32 Temp;
	
	//当本周期开始启动运动Jog功能时，即开启自调整过程
	if((FunCodeUnion.code.AT_LoadMode != 0))     //打开允许辨识功能码，已经设置外部负载需求的模式
	{
		if((STR_PerfIndexDet.AutoTuneLast == 0) && (AuxFunCodeUnion.code.H2F_Rsvd33 == 1))
	    {
	        AutoTuneStatus = 1;     //开启调整
	    }
		else if((STR_PerfIndexDet.AutoTuneLast == 1) && (AuxFunCodeUnion.code.H2F_Rsvd33 == 0))	//当退出辨识时
		{
		    if(AutoTuneStatus != 0)   //如果此时辨识未结束，则直接认为辨识失败退出
			{
			    AutoTuneStatus = 5;
			}
		}
		else     //辨识状态不变时
		{		    
		}

		if((AutoTuneStatus != 0)&&(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN))   //断使能时直接认为辨识失败退出
		{
		    AutoTuneStatus = 5;
		}
	}
	else
	{
	    AutoTuneStatus = 0;	    //不许调整
	}
	
	if(STR_PerfIndexDet.HandAutoTuneFlag == 1)    //当处于手动自调整时
	{
	    STR_FUNC_Gvar.RigidityLvlTab.AutoTuneFlag = 1;    //置手动自调整标志位

        if(AuxFunCodeUnion.code.FA_AutoTune == 0)	   //当手动修改H0d09的值时，辨识失败，直接退出
		{
		    AutoTuneStatus = 5; 
		}
	}  

	switch(AutoTuneStatus)
	{
        case 1:       //开始辨识，先配置初始相关参数
        	if((STR_PerfIndexDet.PosLagTimeStatus == 1)&& (PosLagTimeStatusLast == 0))
            {
				STR_PerfIndexDet.NotchSetStep            = 1;     //振动检测步骤为第一步
				STR_PerfIndexDet.MaxOscRigidity			= 20;     //初始允许最高刚性为20级
			    FunCodeUnion.code.InertiaIdyCountModeSel = 1;    //开启在线惯量辨识
                FunCodeUnion.code.AutoTuningModeSel      = 1;    //自调整从标准刚性表模式开始
				FunCodeUnion.code.CM_WriteEepromEnable	 = 0;    //通信写功能码不更新Eeprom
			    FunCodeUnion.code.Rigidity_Level1st      = 12;  //初始刚性从默认刚性开始
                AuxFunCodeUnion.code.H2F_AutoTuneStats   = 0;    //清除给上位机的标志位

                GainParamSet(1);      //保存初始增益参数
				
                AutoTuneStatus = 2;
            }
        break;		
        
        case 2:	 //伺服处于辨识状态   
		     //在计算完成定位时间的瞬间开始进行相关判定，即一个定位周期只算一次   
		    if((STR_PerfIndexDet.PosLagTimeStatus == 1) && (PosLagTimeStatusLast == 0))
			{				
				GainAutoTuneStepA();   //判定惯量是否稳定过程

		        //惯量比辨识稳定时开始根据振动情况来调整刚性
				if(STR_PerfIndexDet.SteadyFlag == 1)
				{
				    GainAutoTuneStepB();   //开始判定振动情况
				}
			}
		break;

        case 3:      //刚性表调整完毕，开始手动调整相关增益
            //定位完成时根据是否超调来进行调整
            if((STR_PerfIndexDet.PosLagTimeStatus == 1) && (PosLagTimeStatusLast == 0))
            {   
                //自调整成功时退出自调整模式
				if(FunCodeUnion.code.AutoTuningModeSel != 0)
				{
				    FunCodeUnion.code.AutoTuningModeSel = 0;       //调整结束时均设置模式为手动调整
					SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AutoTuningModeSel));
				}

                //重新设置转矩滤波时间，2pi取为6
                FunCodeUnion.code.TL_ToqFiltTime  = 1000000L / ((Uint32)FunCodeUnion.code.GN_Spd_Kp*6*3);
				FunCodeUnion.code.TL_ToqFiltTime2 = FunCodeUnion.code.TL_ToqFiltTime;
                //不允许发生超调，且实际运行过程中发生了超调
                if(((AuxFunCodeUnion.code.H2F_LctCondition & 0x01) != 0) && ((AuxFunCodeUnion.code.H2F_AutoTuneStats & 0x02) != 0))
                {
                    Temp = (Uint32)FunCodeUnion.code.GN_Spd_Ti<<1;    //主要调整积分时间

                    if(Temp > 20000)    //最大为200ms
                    {
                        Temp = 20000;
                    }
                    
					FunCodeUnion.code.GN_Spd_Ti = Temp;

                    TiTuneCnt++;

                    if(TiTuneCnt >= 5)         //积分时间调整最多允许5次
                    {
                        AutoTuneStatus = 4;					
						TiTuneCnt      = 0;				
                        AuxFunCodeUnion.code.H2F_AutoTuneStats  &= 0xFFFD;  //清除超调标志位
                    }					
                }
                else                          //如果没有发生超调或者允许超调，则直接成功退出
                {
                    AutoTuneStatus = 4;					
                    AuxFunCodeUnion.code.H2F_AutoTuneStats  &= 0xFFFD;  //清除超调标志位
                }

				if(AutoTuneStatus == 4)	    //保存参数到Eeprom中
				{                   
            		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime));                                     
					SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime2));				    
				}
            }
        break;

		case 4:	//当调整成功且上位机提示可以退出时，保存辨识得到的刚性,对相应参数复位
		case 5: //当调整失败时，需要将参数复位成原始参数

			FunCodeUnion.code.CM_WriteEepromEnable  = 1;      //允许Eeprom更新			
			FunCodeUnion.code.AT_AdaptiveFilterMode = 0;	   //退出时不再检测共振频率
            STR_FUNC_Gvar.Fft.CalAfterNotchFlag     = 0;       //陷波器设置标志位复位

            //避免在报警故障状态直接进入第五步时，没有将模式置为0
            if(FunCodeUnion.code.AutoTuningModeSel != 0)
			{
				FunCodeUnion.code.AutoTuningModeSel = 0;       //调整结束时均设置模式为手动调整
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AutoTuningModeSel));
			}
	        
			//当定位时间检测完成，且不再发送脉冲，且停止前发送的脉冲为反方向脉冲，或者断使能时结束辨识
			if(((STR_PerfIndexDet.PosLagTimeStatus==1)&&(STR_PerfIndexDet.PulseStop==1) &&(STR_PerfIndexDet.PulseDir==-1))
               || (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN))
			{			                    
                if(AutoTuneStatus == 4)
		    	{
			    	AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x40;        //辨识成功
				}
				else      //参数复位
				{
			    	AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x80;		//辨识失败
                    
                    GainParamSet(2);    //增益参数复位       
				}			    
												
                STR_PerfIndexDet.NotchSetStep		     = 0;		//清除振动检测步骤
			    STR_PerfIndexDet.SteadyFlag              = 0;        //负载辨识稳定标志位清零
				FunCodeUnion.code.InertiaIdyCountModeSel = 0;       //关闭在线惯量辨识              							    
                
                //涉及惯量比计算的参数复位
	            STR_PerfIndexDet.RatioIndex   = 0;
		        STR_PerfIndexDet.RatioSum     = 0;
		        STR_PerfIndexDet.InerRatioCnt = 0;

		        i = 0;
		        while(i<8)
		        {
		            STR_PerfIndexDet.RatioArray[i] = 0;
			        i++; 
		        }
		        				
				//当手动设置辨识成功后，将功能取消
                if(STR_PerfIndexDet.HandAutoTuneFlag == 1)   
		        {
		            STR_PerfIndexDet.HandAutoTuneFlag = 0;
				    AuxFunCodeUnion.code.FA_AutoTune    = 0;

			        AuxFunCodeUnion.code.CurveMoveDisSetLow  = 0;	 //电机转动圈数
			        AuxFunCodeUnion.code.CurveMoveDisSetHigh = 0;
				    AuxFunCodeUnion.code.CurveMoveSpeedMax	 = 400;   //定位运行速度
				    AuxFunCodeUnion.code.CurveMoveRiseDownTime = 100;  //加减速时间
			        AuxFunCodeUnion.code.GUIModeSelet		   = 1;    //后台位置控制模式，宏定义在FUNC_ModeSelect.c中
			        AuxFunCodeUnion.code.GUIWorkMode		   = 40;    //后台模式复位

			        AuxFunCodeUnion.code.CurveSel        = 0;    // 关闭运动JOG功能
				    AuxFunCodeUnion.code.CurveRunDirSet	 = 0;   //正向运行
				    AuxFunCodeUnion.code.CurveRunMode    = 0;   //连续运行
					AuxFunCodeUnion.code.H2F_Rsvd33      = 0;	//关闭一键式调整功能
		        }	    
				AutoTuneStatus = 0;     //直接退出辨识	    
			}		    
		break;

		default:
		break;
	}
	
    PosLagTimeStatusLast           = STR_PerfIndexDet.PosLagTimeStatus;    //保存上周期的定位时间计算完成标志
	STR_PerfIndexDet.PulseStopLast = STR_PerfIndexDet.PulseStop;
	STR_PerfIndexDet.AutoTuneLast  = AuxFunCodeUnion.code.H2F_Rsvd33;	  
}

//自调整第二步，根据振动情况调整刚性，并根据外部负载模式决定
Static_Inline void GainAutoTuneStepB(void)
{
    static Uint16 PosLagTime = 0;    //用于比较的定位时间
    static Uint16 ModSel = 0;        //最终的模式选择
        
    //如果初始仅检测振动阶段则加增益
	if(STR_PerfIndexDet.NotchSetStep == 1)
	{
	    FunCodeUnion.code.Rigidity_Level1st++;

		if(FunCodeUnion.code.Rigidity_Level1st >= STR_PerfIndexDet.MaxOscRigidity)    //超出最大刚性等级，准备检测振动及退出
		{
            FunCodeUnion.code.Rigidity_Level1st = STR_PerfIndexDet.MaxOscRigidity;
		}
	}
	else if(STR_PerfIndexDet.NotchSetStep == 3)		//开启自适应滤波器后增益提高
	{
	    FunCodeUnion.code.Rigidity_Level1st++;
		 
		if(FunCodeUnion.code.Rigidity_Level1st >= STR_PerfIndexDet.MaxOscRigidity)    //超出最大刚性等级，准备检测振动及退出
		{
            FunCodeUnion.code.Rigidity_Level1st = STR_PerfIndexDet.MaxOscRigidity;
		}		       
	}
    else if(STR_PerfIndexDet.NotchSetStep == 6)   //找到最高刚性后，根据外部负载运行模式来设置增益模式
    {
        if(FunCodeUnion.code.AT_RespnLevel == 2)    //高响应模式
        {
            //刚性不变，也不考虑加入增益切换或者摩擦补偿
        }
        else if(FunCodeUnion.code.AT_RespnLevel == 1)  //中响应模式，直接改变刚性
        {                
            FunCodeUnion.code.Rigidity_Level1st = ((Uint32)STR_PerfIndexDet.SetRigidity * MIDCOFF_Q10)>>10;
        }
        else    //低响应模式，直接改变刚性 
        {               
            FunCodeUnion.code.Rigidity_Level1st = ((Uint32)STR_PerfIndexDet.SetRigidity * LOWCOFF_Q10)>>10;
        }
        
        STR_PerfIndexDet.NotchSetStep = 7;     //找到最高刚性后第一步只是设置适合刚性，下一步再设定运行模式       
    }
    else if(STR_PerfIndexDet.NotchSetStep == 7)
    {
        if(FunCodeUnion.code.AT_LoadMode == 1) //轨迹模式
        {           
            AutoTuneStatus = 3;     //直接判定是否有超调，开始微调增益

            AuxFunCodeUnion.code.H2F_AutoTuneStats |= (1<<3); 
        }
        else if(FunCodeUnion.code.AT_LoadMode == 2)  //定位模式
        {
            if(FunCodeUnion.code.AT_RespnLevel == 2)    //高响应模式
            {
                if(FunCodeUnion.code.AutoTuningModeSel == 1)       //刚性表模式
                {
                    PosLagTime = STR_PerfIndexDet.PosLagTime;    //锁存刚性表模式时的定位时间
                    
                    if((AuxFunCodeUnion.code.H2F_AutoTuneStats & 0x02) != 0)   //有超调则进入增益切换
                    {
                        ModSel     = 2;                           //有超调则直接保存增益切换模式
                        FunCodeUnion.code.AutoTuningModeSel = 2;      //进入刚性表加补偿模式
                    }
                    else
                    {                                                
                        ModSel     = 1;                              //无超调保存当前模式
                        FunCodeUnion.code.AutoTuningModeSel = 3;      //进入刚性表加补偿模式
                    }                        
                }
                else if(FunCodeUnion.code.AutoTuningModeSel == 3)
                {
                    if((AuxFunCodeUnion.code.H2F_AutoTuneStats & 0x02) != 0)    //发生超调直接使用增益切换
                    {
                        ModSel     = 2;
                    }
                    else if(STR_PerfIndexDet.PosLagTime < PosLagTime)    //无超调时判定当前定位时间是否小于之前模式值
                    {
                        ModSel     = 3;
                        PosLagTime = STR_PerfIndexDet.PosLagTime;     //保存定位时间
                    }

                    FunCodeUnion.code.AutoTuningModeSel = 2;      //进入定位模式                     
                }
                else if(FunCodeUnion.code.AutoTuningModeSel == 2)
                {
                    if(STR_PerfIndexDet.PosLagTime < PosLagTime)   //判定当前定位时间是否小于之前模式值
                    {
                        ModSel     = 2;
                        PosLagTime = STR_PerfIndexDet.PosLagTime;
                    }

                    FunCodeUnion.code.AutoTuningModeSel = 4;      //进入定位加补偿模式 
                }
                else
                {
                    if(STR_PerfIndexDet.PosLagTime < PosLagTime)   //判定当前定位时间是否小于之前模式值
                    {
                        ModSel     = 4;
                        PosLagTime = STR_PerfIndexDet.PosLagTime;
                    }
                     
                    FunCodeUnion.code.AutoTuningModeSel     = ModSel;     //恢复到定位时间最小的模式
                    AuxFunCodeUnion.code.H2F_AutoTuneStats |= (ModSel<<3);   //给上位机提示最终选定的模式
                    AutoTuneStatus = 3;     //直接判定是否有超调，开始微调增益
                }                                                   
            }
            else    //中响应和低响应模式均不作补偿 
            {
                if(FunCodeUnion.code.AutoTuningModeSel == 1)       //刚性表模式
                {
                    PosLagTime = STR_PerfIndexDet.PosLagTime;    //锁存刚性表模式时的定位时间

                    if((AuxFunCodeUnion.code.H2F_AutoTuneStats & 0x02) != 0)
                    {
                        ModSel  = 2;     //有超调则直接设置成增益切换模式
                    }
                    else
                    {
                        ModSel  = 1;     //无超调则使用本次调节的模式
                    }
                                                  
                    FunCodeUnion.code.AutoTuningModeSel = 2;      //进入定位模式
                }
                else if(FunCodeUnion.code.AutoTuningModeSel == 2)
                {
                    if(STR_PerfIndexDet.PosLagTime < PosLagTime)      //判定当前定位时间是否小于之前模式值
                    {
                        ModSel     = 2;
                        PosLagTime = STR_PerfIndexDet.PosLagTime;
                    }
                    
                    FunCodeUnion.code.AutoTuningModeSel    = ModSel;   //恢复到定位时间最小的模式
                    AuxFunCodeUnion.code.H2F_AutoTuneStats |= (ModSel<<3);   //给上位机提示最终选定的模式
                    
                    AutoTuneStatus = 3;     //直接判定是否有超调，开始微调增益
                }            
            }            
        }        
    }	 
}

//根据惯量比来设置最高允许刚性，此设置值根据经验来设计，还需要继续完善
Static_Inline void MaxRigidityCal(void)
{
    if(FunCodeUnion.code.GN_InertiaRatio < 100)
    {
        STR_PerfIndexDet.MaxOscRigidity = 26;
    }
    else if(FunCodeUnion.code.GN_InertiaRatio < 300)
    {
        STR_PerfIndexDet.MaxOscRigidity = 24;
    }
    else if(FunCodeUnion.code.GN_InertiaRatio < 500)
    {
        STR_PerfIndexDet.MaxOscRigidity = 22;
    }
    else
    {
        STR_PerfIndexDet.MaxOscRigidity = 20;
    }       
}

//自调整过程第一步，判定惯量辨识结果是否稳定，计算重力负载，摩擦力等变量
Static_Inline void GainAutoTuneStepA(void)
{
	static Uint16 MaxRatio = 0;	       //保存的最大和最小惯量比值，用于判定辨识是否稳定
	static Uint16 MinRatio = 0; 
	static Uint16 OldRatioIdenCnt = 0;
    static Uint16 WaitCnt = 0;       //未进行惯量辨识次数 

    if(OldRatioIdenCnt != AuxFunCodeUnion.code.H2F_Rsvd43)	 //使能后出现辨识值时保存结果并做相应运算
    {	
	    WaitCnt         = 0;             
		OldRatioIdenCnt = AuxFunCodeUnion.code.H2F_Rsvd43;
	    STR_PerfIndexDet.InerRatioCnt++;		   //辨识累加计算次数
		
		//求取惯量比的滑动平均值
		STR_PerfIndexDet.RatioSum 	-= STR_PerfIndexDet.RatioArray[STR_PerfIndexDet.RatioIndex];
		STR_PerfIndexDet.RatioSum   += AuxFunCodeUnion.code.DP_JRatioOnline;
		STR_PerfIndexDet.RatioArray[STR_PerfIndexDet.RatioIndex] = AuxFunCodeUnion.code.DP_JRatioOnline; 
		STR_PerfIndexDet.RatioIndex++;

		if(STR_PerfIndexDet.RatioIndex == 4)
		{
		    STR_PerfIndexDet.RatioIndex = 0;
		}

		FunCodeUnion.code.GN_Rsvd16 = STR_PerfIndexDet.RatioSum>>2;  //获得滤波后的惯量比,(JL+Jm)/Jm

		if(FunCodeUnion.code.GN_Rsvd16 > 100)
		{
			FunCodeUnion.code.GN_Rsvd16 -= 100;
		}
		
		//判断是否处于稳定状态
		if(STR_PerfIndexDet.InerRatioCnt < 4)
		{
		    MaxRatio = 0;
			MinRatio = 0;
		}
		else if(STR_PerfIndexDet.InerRatioCnt == 4)	 //设定最大最小初始值
		{
		    MaxRatio = FunCodeUnion.code.GN_Rsvd16;
			MinRatio = MaxRatio;
		}
		else    	 //获取保存的最大最小惯量比
		{
		    MaxRatio = MAX(MaxRatio, FunCodeUnion.code.GN_Rsvd16);	   
            MinRatio = MIN(MinRatio, FunCodeUnion.code.GN_Rsvd16);

			if(STR_PerfIndexDet.InerRatioCnt >= 10)    //只有在最大最小值计算次数到达6次或以上时才判定
			{
			    if(STR_PerfIndexDet.SteadyFlag == 0)  //是否稳定只判定一次
				{
				    if((MaxRatio - MinRatio) < 200)	 //如果最大最小差值小于2，则认为惯量比稳定
			    	{			            					
					    STR_PerfIndexDet.SteadyFlag = 1;

						//惯量比在5倍以内更新H0815，并获取惯量比下对应的允许最高刚性
						if(FunCodeUnion.code.GN_Rsvd16 < 500)   
						{
						    FunCodeUnion.code.GN_InertiaRatio = FunCodeUnion.code.GN_Rsvd16;   //更新H0815

							if(STR_PerfIndexDet.HandAutoTuneFlag == 1) //手动启动调整时才写入Eeprom
							{
							    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_InertiaRatio));
							}

                            MaxRigidityCal();     //计算惯量比下对应的最大刚性
						}
					}
					else   //惯量比不稳定   
					{
			        	AutoTuneStatus = 5;     //辨识失败			
		            
			        	AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x01;      //提示上位机不稳定				    
					}					
			    }
			} 													
		}
	        
        //将辨识得到的结果写入到补偿功能码中，由H0900运行模式决定其是否使用
		if((FunCodeUnion.code.AutoTuningModeSel == 3) || (FunCodeUnion.code.AutoTuningModeSel == 4))
		{
            FunCodeUnion.code.AT_ConstToqComp = AuxFunCodeUnion.code.H2F_Rsvd40;    
            FunCodeUnion.code.AT_ToqPlusComp  = AuxFunCodeUnion.code.H2F_Rsvd41;
            FunCodeUnion.code.AT_ToqMinusComp = AuxFunCodeUnion.code.H2F_Rsvd42;
		}
		else
		{
            FunCodeUnion.code.AT_ConstToqComp  = 0;    
            FunCodeUnion.code.AT_ToqPlusComp  = 0;
            FunCodeUnion.code.AT_ToqMinusComp = 0;		    
		}        		
    }   
    else				 //没有检测到惯量比更新
    {
        if(STR_PerfIndexDet.SteadyFlag == 0)  //只在初始刚性下判定是否有连续检测不出惯量比的情况
        {
            WaitCnt++;

            if(WaitCnt > 4)     //连续4次加减速未辨识出惯量，则提示失败
            {
                WaitCnt        = 0;
                AutoTuneStatus = 5;     //辨识失败					            
			    AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x01;      //提示上位机不稳定
            }
        }
        else
        {
            WaitCnt = 0;
        }
    }
}

/*******************************************************************************
  函数名: GainParamSet(Step) 
  输  入: Step步骤，1表示保存增益参数，2表示恢复增益参数          
  输  出:   
  子函数:                                      
  描  述: 根据函数参数设置来确定是保存初始增益参数还是将增益参数复位
********************************************************************************/
Static_Inline void GainParamSet(Uint16 Step)
{
    static Uint16 H0705Temp = 79;   //自调整中需要保存及复位的功能码
    static Uint16 H0706Temp = 79; 
    static Uint16 H0800Temp = 250;
    static Uint16 H0801Temp = 3183;
    static Uint16 H0802Temp = 400;
    static Uint16 H0803Temp = 400;
    static Uint16 H0804Temp = 2000;
    static Uint16 H0805Temp = 640;
    static Uint16 H0815Temp = 100;
    static Uint16 H0819Temp = 0;
    static Uint16 H0932Temp = 0;
    static Uint16 H0933Temp = 0;
    static Uint16 H0934Temp = 0;

    if(Step == 1)     //保存功能码
    {
        H0705Temp = FunCodeUnion.code.TL_ToqFiltTime;
        H0706Temp = FunCodeUnion.code.TL_ToqFiltTime2;
        H0800Temp = FunCodeUnion.code.GN_Spd_Kp;
        H0801Temp = FunCodeUnion.code.GN_Spd_Ti;
        H0802Temp = FunCodeUnion.code.GN_Pos_Kp;
        H0803Temp = FunCodeUnion.code.GN_Spd_Kp2;
        H0804Temp = FunCodeUnion.code.GN_Spd_Ti2;
        H0805Temp = FunCodeUnion.code.GN_Pos_Kp2;
        H0815Temp = FunCodeUnion.code.GN_InertiaRatio;
        H0819Temp = FunCodeUnion.code.GN_SpdFb_Kp;
        H0932Temp = FunCodeUnion.code.AT_ConstToqComp;
        H0933Temp = FunCodeUnion.code.AT_ToqPlusComp;
        H0934Temp = FunCodeUnion.code.AT_ToqMinusComp; 
    }
    else if(Step == 2)  //复位功能码
    {
        FunCodeUnion.code.TL_ToqFiltTime = H0705Temp;
        FunCodeUnion.code.TL_ToqFiltTime2 = H0706Temp;
        FunCodeUnion.code.GN_Spd_Kp = H0800Temp;
        FunCodeUnion.code.GN_Spd_Ti = H0801Temp;
        FunCodeUnion.code.GN_Pos_Kp = H0802Temp;
        FunCodeUnion.code.GN_Spd_Kp2 = H0803Temp; 
        FunCodeUnion.code.GN_Spd_Ti2 = H0804Temp;
        FunCodeUnion.code.GN_Pos_Kp2 = H0805Temp;
        FunCodeUnion.code.GN_InertiaRatio = H0815Temp;
        FunCodeUnion.code.GN_SpdFb_Kp     = H0819Temp;
        FunCodeUnion.code.AT_ConstToqComp = H0932Temp;
        FunCodeUnion.code.AT_ToqPlusComp  = H0933Temp;
        FunCodeUnion.code.AT_ToqMinusComp = H0934Temp;        
    }
       
}

/*******************************************************************************
  函数名: OscillateDeal() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 增益自调整过程中发生振动时的处理，跳转标志位的变化
********************************************************************************/
Static_Inline void OscillateDeal(void)
{
	static Uint16 OldFftCnt = 0;
    static Uint16 MaxRigitCnt = 0;
		
	if(OldFftCnt != STR_FUNC_Gvar.Fft.FftCalCnt)     //计算出新的振动幅值
	{		
		//检测到振动时的情况，如果未发生振动则在每次定位完成后调整刚性
		if(STR_FUNC_Gvar.Fft.OscillateValue > STR_FUNC_Gvar.Fft.OscillateLevel)
		{
		    if(STR_PerfIndexDet.NotchSetStep == 1)	 //第一次检测到振动时准备开启自适应陷波器
		    {
		        STR_PerfIndexDet.NotchSetStep = 2;
                STR_PerfIndexDet.FirOscRigidity  = FunCodeUnion.code.Rigidity_Level1st;   //保存第一次振动时的刚性
		    	STR_PerfIndexDet.MaxOscRigidity  = MIN(STR_PerfIndexDet.FirOscRigidity+2, STR_PerfIndexDet.MaxOscRigidity);   //加入陷波器后最多提高两级刚性
			}			
			else if(STR_PerfIndexDet.NotchSetStep == 3)	//如果已经设置了陷波器且检测到振动则准备直接退出 			
			{
			    //设置陷波器后再次振动时，如果刚性比未设置陷波器还要小，则复位陷波器
				if(FunCodeUnion.code.Rigidity_Level1st > STR_PerfIndexDet.FirOscRigidity)
                {
                    STR_PerfIndexDet.NotchSetStep   = 4;                       
                }                    
                else
                {
                    STR_PerfIndexDet.NotchSetStep   = 5;
                }
			}

            //如果处于最高刚性，则要直接退出
            if(FunCodeUnion.code.Rigidity_Level1st == STR_PerfIndexDet.MaxOscRigidity)
            {
	            FunCodeUnion.code.Rigidity_Level1st -= 2;
                STR_PerfIndexDet.SetRigidity        = FunCodeUnion.code.Rigidity_Level1st;   //最高刚性		
                STR_PerfIndexDet.NotchSetStep = 6;                
            }

            MaxRigitCnt = 0;
		}
        else       //如果没有震动就判断是否处于最高刚性
        {
             if(FunCodeUnion.code.Rigidity_Level1st == STR_PerfIndexDet.MaxOscRigidity)
             {
                  MaxRigitCnt++;

                  if(MaxRigitCnt > 10)      //如果连续10次检测都没有进入共振点,保持原刚性
                  {
                      STR_PerfIndexDet.SetRigidity  = FunCodeUnion.code.Rigidity_Level1st;   //最高刚性		
                      STR_PerfIndexDet.NotchSetStep = 6;
                      MaxRigitCnt                   = 0;        //计数值清零
                  }
             }
             else
             {
                  MaxRigitCnt = 0;
             }

        }
	}

	OldFftCnt = STR_FUNC_Gvar.Fft.FftCalCnt;

	//只处于振动检测
	if(STR_PerfIndexDet.NotchSetStep == 1)
	{
	    FunCodeUnion.code.AT_AdaptiveFilterMode	= 1;    //直接开启自适应陷波器
	}
	else if(STR_PerfIndexDet.NotchSetStep == 2)    //如果开启了自适应陷波器，先降低两个刚性等级
	{
		if(STR_FUNC_Gvar.Fft.CalAfterNotchFlag == 1)
		{
		    if(FunCodeUnion.code.Rigidity_Level1st > 13)   //初次碰到共振时需要保证刚性不低于12级
            {
                FunCodeUnion.code.Rigidity_Level1st -= 2;
            }

			FunCodeUnion.code.AT_AdaptiveFilterMode = 3;    //设置陷波器后只进行检测振动		    
		    STR_PerfIndexDet.NotchSetStep = 3;			//进入下一步增益提高阶段
		}
	}
	else if(STR_PerfIndexDet.NotchSetStep == 4)	     //加入陷波器后再次碰到共振，且刚性比未设置陷波器高
	{
	    FunCodeUnion.code.Rigidity_Level1st -= 1;
        STR_PerfIndexDet.SetRigidity         = FunCodeUnion.code.Rigidity_Level1st;   //最高刚性		
        STR_PerfIndexDet.NotchSetStep = 6;
		
		FunCodeUnion.code.AT_AdaptiveFilterMode = 0;   //不再做自调整 
	}
	else if(STR_PerfIndexDet.NotchSetStep == 5)		//加入陷波器后未提高刚性,结束调整并复位陷波器
	{
        FunCodeUnion.code.Rigidity_Level1st -= 1;        
        FunCodeUnion.code.AT_NotchFiltFreqC  = 4000;
		FunCodeUnion.code.AT_NotchFiltBandWidthC = 2;
        FunCodeUnion.code.AT_NotchFiltAttenuatLvlC = 0;        
        STR_PerfIndexDet.SetRigidity  = FunCodeUnion.code.Rigidity_Level1st;   //最终设置的刚性         
        STR_PerfIndexDet.NotchSetStep = 6;

		FunCodeUnion.code.AT_AdaptiveFilterMode = 0;   //不再做自调整
	} 
}


/*******************************************************************************
  函数名: PerformanceIndexCal() 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 
********************************************************************************/
void  PerformanceIndexCal(void)
{   			
	if(1 == AuxFunCodeUnion.code.CurveSel)     //启动运动Jog
	{
	    //计时
	    TimerCal();
	
        //检测脉冲开始发送和停止的时刻
		PulsCheck();
 		
		//惯量辨识部分在MTR模块中完成
	    
		//计算平均负载率
	    LoadRatioAvCal();
   
        //计算定位时间
        PositioningTimeCal();
   
        //过冲脉冲数计算
        OvershootPulseCal();

		//计算振动等级
		OScillateLeveCal();

		//发生振动时的处理
		OscillateDeal();

		//增益变换过程
		GainAutoTuneProcess();
	}
	else
	{
	    STR_PerfIndexDet.SystemTimerLast = GetSysTime_1MHzClk();       //取消运动Jog时需要重置时间

		//使用辅助功能码启动运动JOG和自调整功能
	    if(AuxFunCodeUnion.code.FA_AutoTune == 1)   //利用辅助功能码启动增益自调整
	    {
			STR_PerfIndexDet.HandAutoTuneFlag		 = 1;    //手动触发标志位
			AuxFunCodeUnion.code.CurveServoON		 = 1;   //内部使能
			AuxFunCodeUnion.code.GUIModeSelet		 = 1;    //后台位置控制模式，宏定义在FUNC_ModeSelect.c中
			AuxFunCodeUnion.code.GUIWorkMode		 = 60;    //后台运动JOG模式
			//设置运行位移
			AuxFunCodeUnion.code.CurveMoveDisSetLow  = FunCodeUnion.code.AT_Rsvd42;	 //电机转动4圈
			AuxFunCodeUnion.code.CurveMoveDisSetHigh = FunCodeUnion.code.AT_Rsvd43;
			AuxFunCodeUnion.code.CurveMoveSpeedMax	 = FunCodeUnion.code.AT_Rsvd44;   //定位运行速度
			AuxFunCodeUnion.code.CurveMoveRiseDownTime = FunCodeUnion.code.AT_Rsvd45;  //加减速时间
			AuxFunCodeUnion.code.WaitTime			   = FunCodeUnion.code.AT_Rsvd46;  //等待时间
			//设置初始刚性
			FunCodeUnion.code.AutoTuningModeSel      = 1;    //自调整从标准刚性表模式开始
			FunCodeUnion.code.Rigidity_Level1st      = 12;  //初始刚性从默认刚性开始

			if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == 2)	//当伺服处于使能状态时再开启运动Jog功能
			{
			    AuxFunCodeUnion.code.CurveSel        = 1;   // 启动运动JOG功能
				AuxFunCodeUnion.code.CurveRunDirSet	 = 1;   //正向运行
				AuxFunCodeUnion.code.CurveRunMode    = 1;   //连续运行
				AuxFunCodeUnion.code.H2F_Rsvd33      = 1;   //启动一键式调整功能
			}
	    }
	}
}

//位置脉冲检测及其相关标识位设置
Static_Inline void PulsCheck(void)
{
    if(STR_PosCtrl.PosMultipleCmd==0)
    {
        STR_PerfIndexDet.PulseStop = 1;   //脉冲发送结束
    
	    if(0 == STR_PerfIndexDet.PulseStopLast)	           //脉冲发送停止跳变沿
        {
            STR_PerfIndexDet.Timer1msLatch1 = STR_PerfIndexDet.Timer1ms;//锁存计数时间
	  
	        STR_PerfIndexDet.PulseMax         = 0;  //过冲脉冲数最大值清零
            STR_PerfIndexDet.PosLagTimeStatus = 0;  //标记需要计算定位时间
        }
    }
    else
    {
        STR_PerfIndexDet.PulseStop = 0;            //脉冲发送中

	    if(STR_PosCtrl.PosMultipleCmd > 0)
	    {
	        STR_PerfIndexDet.PulseDir = 1;	 //正方向脉冲发送
	    }
	    else
	    {
	        STR_PerfIndexDet.PulseDir = -1;	 //负方向脉冲发送
	    }
    }    
}
/*******************************************************************************
  函数名: PerformanceIndexInit() 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 增益指标计算初始
********************************************************************************/
void PerformanceIndexInit(void)
{
    STR_PerfIndexDet.Timer1us        = 0;
	STR_PerfIndexDet.Timer1ms   	 = 0;
	STR_PerfIndexDet.Timer1msLatch1  = 0;
	STR_PerfIndexDet.PulseMax        = 0;	
	STR_PerfIndexDet.OscillateLeveSum = 0;
	STR_PerfIndexDet.DataUpEn        = 0;
	STR_PerfIndexDet.LoadRatSum      = 0;
	STR_PerfIndexDet.PulseStop		 = 0;
	STR_PerfIndexDet.PulseStopLast	 = 0;
	STR_PerfIndexDet.PulseMnStopLast = 0;
}
/*******************************************************************************
  函数名: TimerCal() 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 记录时间，获取以us、ms为单位的计时值
********************************************************************************/
Static_Inline void TimerCal(void)
{
    Uint16  temp12;
	static Uint16 InitDone = 0;			  //上电第一次需要取时钟

	if(InitDone == 0)
	{
	    STR_PerfIndexDet.SystemTimerLast = GetSysTime_1MHzClk();
		InitDone = 1;
	}
   
    //计算以us，ms为单位的时间
    temp12 = GetSysTime_1MHzClk() - STR_PerfIndexDet.SystemTimerLast;

    STR_PerfIndexDet.Timer1us += temp12;      //计算从上周期调用到本周期调用所消耗的时间

    if(STR_PerfIndexDet.Timer1us >= MHZ1MS)		//计时达到1ms时的处理
    {
        STR_PerfIndexDet.Timer1us -= MHZ1MS;
	   
	    STR_PerfIndexDet.Timer1ms++;		
    }

	STR_PerfIndexDet.SystemTimerLast = GetSysTime_1MHzClk();
}
/*******************************************************************************
  函数名: LoadRatioAvCal() 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 平均负载率计算
********************************************************************************/
Static_Inline void LoadRatioAvCal(void)
{	
	static Uint16 DeltaCnt = 0;        //上次进入和和本次进入时的时间差，用于计算平均负载率

	if(STR_PerfIndexDet.PulseStop == 1)	   //脉冲发送一旦停止则重新设置锁存基准时间
	{		
		if(DeltaCnt > 10)		 //根据保存的转矩和计算平均转矩，只在采样大于10次时计算一次,其余保持不变以避免干扰
		{		    
			STR_PerfIndexDet.LoadRatAV    = STR_PerfIndexDet.LoadRatSum / DeltaCnt;
		    STR_PerfIndexDet.LoadRatSum   = 0;
			DeltaCnt					  = 0;
		}		  
	}
	else			//处于脉冲发送状态时
	{
	    if(STR_PerfIndexDet.PulseStopLast == 1)	   //刚启动时清零
		{
			STR_PerfIndexDet.LoadRatSum  = 0;
			DeltaCnt 					 = 0;
		}		

	    //累加时间不超过4s
	    if(DeltaCnt < 4096)
	    {
	        DeltaCnt++;
			STR_PerfIndexDet.LoadRatSum += ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef);
	    }  
	}
}

/*******************************************************************************
  函数名: PositioningTimeCal() 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 定位时间计算
********************************************************************************/
Static_Inline void PositioningTimeCal(void)
{
    static Uint16 FirstLagTime = 0;	   
	Uint16 Temp1;
	Uint16 Temp2;      //允许等待的时间，必须小于实际定位运行时的等待时间，取为1/2的实际等待时间 
		  
	if(0 == STR_PerfIndexDet.PosLagTimeStatus)         //确保一个矩形波定位时间只计算一次，不使用沿判断
    {	   
	    //计算脉冲停止发送到定位完成信号输出时的差值
        Temp1 = STR_PerfIndexDet.Timer1ms - STR_PerfIndexDet.Timer1msLatch1;
		Temp2 = (AuxFunCodeUnion.code.WaitTime)>>1;		  

	    //锁存第一次的定位完成时间
		if(FirstLagTime == 0)
		{
		    if(1 == STR_FUNC_Gvar.PosCtrl.DovarReg_Coin)
			{
			    FirstLagTime = Temp1; 
			}

			AuxFunCodeUnion.code.H2F_AutoTuneStats &= 0xF9;     //清除抖动和超调标志位
		}
		else   //保存完第一次定位完成时间后，如果完成信号再为0则认为出现了抖动
		{
		    if(0 == STR_FUNC_Gvar.PosCtrl.DovarReg_Coin)
			{
			    AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x04;    //定位抖动
			}

			if(ABS(STR_PerfIndexDet.PulseMax) > FunCodeUnion.code.PL_PosReachValue) //过冲脉冲大于设置值就认为超调
			{
			    AuxFunCodeUnion.code.H2F_AutoTuneStats |= 0x02;    //定位超调
			}
		}

		//在大于设定的等待时间后，根据条件设定相应定位时间
		if(Temp1 >= Temp2)
		{
			STR_PerfIndexDet.PosLagTime = (FirstLagTime>0) ? FirstLagTime : Temp2;
			
			STR_PerfIndexDet.PosLagTimeStatus = 1;      //不再计算定位时间
		}
    }
	else
	{
	    FirstLagTime = 0;
	}
}
/*******************************************************************************
  函数名: OvershootPulseCal()
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 计算最大的过冲脉冲数
********************************************************************************/
Static_Inline void OvershootPulseCal(void)
{
    if(1==STR_PerfIndexDet.PulseStop)      //脉冲发送停止
    {
        if(STR_PerfIndexDet.PulseDir == 1)  //正向脉冲，过冲脉冲数为负
	    {
	        if(STR_PerfIndexDet.PulseMax > STR_FUNC_Gvar.PosCtrl.PosAmplifErr) //寻找最大负值
	        { 
		        STR_PerfIndexDet.PulseMax = STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
		    }
	    }
	    else											  //   负向脉冲，过冲脉冲数为正
	    {
	        if(STR_PerfIndexDet.PulseMax < STR_FUNC_Gvar.PosCtrl.PosAmplifErr) //寻找最大正值
	        { 
		        STR_PerfIndexDet.PulseMax = STR_FUNC_Gvar.PosCtrl.PosAmplifErr;
		    }
	    }
    }
}
/*******************************************************************************
  函数名: OScillateLeveCal()
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 计算振动等级
********************************************************************************/
Static_Inline void OScillateLeveCal(void)
{
    static Uint16 SumCnt = 0;			//计算振动累加和的次数
	static Uint16 OldFftCalCnt = 0;	   //保存上周期FFt计算成功的次数
	
	//只要计算结果发生改变，就将结果进行累加
	if(OldFftCalCnt != STR_FUNC_Gvar.Fft.FftCalCnt)
	{
		SumCnt++;	    
		STR_PerfIndexDet.OscillateLeveSum += STR_FUNC_Gvar.Fft.OscillateValue;

		if(STR_PerfIndexDet.PulseStop==1)	  //脉冲停止时计算平均震动等级
		{
		    STR_PerfIndexDet.OscillateLevelAv = STR_PerfIndexDet.OscillateLeveSum / SumCnt;
			STR_PerfIndexDet.OscillateLevelAv = ((Uint32)STR_PerfIndexDet.OscillateLevelAv*1000)>>12;	 //转化为百分比		    
			STR_PerfIndexDet.OscillateLeveSum = 0;
			SumCnt							  = 0;
		}
	}

	OldFftCalCnt = STR_FUNC_Gvar.Fft.FftCalCnt;
}
/*******************************************************************************
  函数名: IndicatrixDataUp 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 将计算得到的过冲脉冲数、定位时间、平均负载率、振动等级数据更新，在
          位置指令开始发送时进行更新
********************************************************************************/
void IndicatrixDataUp(void)
{  
    if((0 == STR_PerfIndexDet.PulseStop)&&(1 == STR_PerfIndexDet.PulseMnStopLast))	 //下轮脉冲上升沿更新过冲脉冲数      	 
	{
       	STR_PerfIndexDet.DataUpEn = 1;   
    }

	if(1==STR_PerfIndexDet.DataUpEn)		 //只在上升沿时更新一次
	{
	    STR_PerfIndexDet.DataUpEn = 0;
		
	    //最大过冲量设置为指令脉冲形式
		AuxFunCodeUnion.code.GR_OvershootPulse =
		(Uint16)((int64)STR_PerfIndexDet.PulseMax * STR_FUNC_Gvar.PosCtrl.Denominator / STR_FUNC_Gvar.PosCtrl.Numerator);

		AuxFunCodeUnion.code.GR_oscillateLevel = STR_PerfIndexDet.OscillateLevelAv;
	    AuxFunCodeUnion.code.GR_PosLagTime     = STR_PerfIndexDet.PosLagTime;
	    AuxFunCodeUnion.code.GR_LoadRatioAv    = STR_PerfIndexDet.LoadRatAV;
	    AuxFunCodeUnion.code.GR_DataReady++; 
	}

    STR_PerfIndexDet.PulseMnStopLast = STR_PerfIndexDet.PulseStop;
}


/*******************************************************************************
  函数名: IndicatrixStopUpData(void) 
  输  入:          
  输  出:   
  子函数:                                       
  描  述: 在ServoOFF后将此次计算指标清零
********************************************************************************/
void IndicatrixStopUpData(void)
{
    AuxFunCodeUnion.code.H2F_AutoTuneStats = 0;
	AuxFunCodeUnion.code.GR_OvershootPulse = 0;
	AuxFunCodeUnion.code.GR_oscillateLevel = 0;
	AuxFunCodeUnion.code.GR_PosLagTime     = 0;
	AuxFunCodeUnion.code.GR_LoadRatioAv    = 0;
	AuxFunCodeUnion.code.GR_DataReady      = 0;
    STR_PerfIndexDet.PulseMax              = 0;
	STR_PerfIndexDet.PosLagTime            = 0;
	STR_PerfIndexDet.LoadRatAV             = 0;
	STR_PerfIndexDet.OscillateLevelAv      = 0;
	STR_PerfIndexDet.SteadyFlag            = 0;	    //惯量稳定标识清零
	STR_PerfIndexDet.PosLagTimeStatus      = 1;		//初始不允许计算定位时间
}


