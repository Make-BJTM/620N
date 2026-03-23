/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    MTR_AngleInit.c
 创建人：  姚虹          创建日期：12.9.19      
 修改人：                
 描述： 1. 完成初始角度的辨识，包含检测Z脉冲的方式和不检测Z脉冲的方式，
        2.
        3.
 修改记录：  
     1）12.11.15  将程序移到MTR模块
     2）13.03.20  加入电压注入电角度辨识程序

********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "PUB_Table.h"
#include "PUB_Library_Function.h"
#include "MTR_GlobalVariable.h"
#include "MTR_AngleInit.h"
#include "MTR_InterfaceProcess.h"
#include "MTR_FPGAInterface.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//#define DIVANGCNT       10       //初始角辨识不找Z时将电周期分成的份数
#define SQRT3_Q10       1773   
#define DEGREE10        100
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

STR_ANGINIT   STR_AngInit;

UNI_FPGA_SYSCTRL_REG  SysCtrl_Reg = {0};      //系统控制寄存器
/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

extern volatile Uint16 *HostDutyU;   //主机给定的U相PWM占空比
extern volatile Uint16 *HostDutyV;   //主机给定的V相PWM占空比
extern volatile Uint16 *HostDutyW;   //主机给定的W相PWM占空比

extern volatile Uint16 *PWMPrd_Mode;     //使用对称模式
extern volatile Uint16 *DeadTmCtrl;      //死区时间设置
extern volatile Uint16 *DeadTmComp;      //死区补偿时间设置
extern volatile Uint16 *CurSampCtrl;     //用以确定采样点

extern volatile Uint16 *HostSysCtrl;     //用以做初始角度辨识处理
extern volatile Uint16 *HostAlarmClr;    //报警清除
extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;

static int32  InitPos;              //记录初始位置
static int16  ZErrFlag = 0;          //1～找Z时出现故障，0～找Z时正常
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

void AngInt_ZPosLatch(void);
void AngInt_IncEncReset(void);
void AngInt_IncEncResetFromZ(void);
void AngInt_CurVct(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

Static_Inline Uint16 AngInt_SearchElecAngle(void);
Static_Inline Uint16 AngInt_DutySearch(void);


//用于省线式电机的初始角辨识，需要检测Z脉冲
void AngInt_IncEncResetFromZ(void)
{
	static Uint16 EncResetStep = 0;    //角度辨识步骤
    static Uint32 CurAng = 0;           //当前定位的电角度
    static Uint16 LoopCnt = 0;         //电机旋转电周期个数
	static Uint32 TimeDelay = 0;        //时间延时
    static int32  UVWPos1 = 0;         //判断UVW接线是否准确得到的位置
    static int32  UVWPos2 = 0;  
    static int32  UVWPos3 = 0;
	int32  Temp1;     //用于计算的中间变量	
	
	if(EncResetStep == 0)
	{
		//只有在还没有内部使能的情况下才进入角度辨识
        if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.InnerServoSon == 0)
        {
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.IncEncAngInitEn = 1;           //提示MTR模块开始角度辨识
            EncResetStep = 1;
        }
        else
        {
            AuxFunCodeUnion.code.OEM_ResetABSTheta = 0;
            EncResetStep = 0;
        }					
	}
	else if(EncResetStep == 1)
	{
	    *HostSysCtrl = 0x001E;        //初始角辨识使能

        if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == RUN)    //内部使能打开时进入下一步
        {
		    STR_MTR_Gvar.GetIqRef.IdRef = 0;     // 给D轴电流一个初始值
            STR_MTR_Gvar.GetIqRef.IqRef = 0;     // Q轴电流为0
            EncResetStep                = 2;
            UVWPos1 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;   //保存初始位置，用以判定电机是否转动
        }
	}
	else if(EncResetStep == 2)		  //定位到零度，保证定位过程中平滑
	{
		TimeDelay++;

        if(((TimeDelay & 0x0f) == 0) 
            && (STR_MTR_Gvar.GetIqRef.IdRef < STR_MTR_Gvar.GetIqRef.IqRate_MT))   //连续累加并小于额定电流
        {
            STR_MTR_Gvar.GetIqRef.IdRef++;
        }

        if(TimeDelay < (3 * STR_MTR_Gvar.System.ToqFreq))           //先给20度再给0度，避免转子初始位置在180度上
        {
            *HostAng  = (30 * STR_MTR_Gvar.FPGA.AngToCnt) / 360;
        }
        else if(TimeDelay < (6 * STR_MTR_Gvar.System.ToqFreq))    //延时6秒
        {
            *HostAng  = 0;
        }
		else
		{							
			EncResetStep  = 3;
			TimeDelay     = 0;
			InitPos       = STR_MTR_Gvar.FPGA.PosFdbAbsValue;  //保存初始位置						
		}
	}
	else if(EncResetStep == 3)		//给D轴不同角度以检测Z脉冲，在找Z之前需要确定UVW线未接反
	{
		TimeDelay++;
        STR_MTR_Gvar.GetIqRef.IdRef = STR_MTR_Gvar.GetIqRef.IqRate_MT;
        *HostAng                    = CurAng;
        
        //在需要辨别UVW信号时才增加辨识的步骤
        if((STR_MTR_Gvar.AngInt.StartFindZ == 0) && (FunCodeUnion.code.ER_UVWIdenEn == 1)) 
        {
            if(CurAng == 0)             //当电角度为0度时的转子位置
            {
                UVWPos1 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            }
            else if(CurAng == (STR_MTR_Gvar.FPGA.AngToCnt >> 3))   //当为45度角时的转子位置
            {
                UVWPos2 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            }
            else if(CurAng == (STR_MTR_Gvar.FPGA.AngToCnt >> 2))   //当为90度角时的转子位置
            {
                UVWPos3 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            }
            
            //不断累加电角度以使电机转动，初始只用转180度以判定UVW接线是否正常
            if((CurAng < (STR_MTR_Gvar.FPGA.AngToCnt >> 1)))
            {
                if((TimeDelay & 0x0f) == 0)    //每隔16个周期则电角度递增
                {
                    CurAng++;
                }
            }
            else
            {   //位置必须正向变化UVW相序才能判定为正确的
                if((((UVWPos2 - UVWPos1) > 0) && ((UVWPos3 - UVWPos2) > 0) && (STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 0)) 
                   || (((UVWPos2 - UVWPos1) < 0) && ((UVWPos3 - UVWPos2) < 0) && (STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 1)))
                {
                    STR_MTR_Gvar.AngInt.StartFindZ = 1;    //如UVW未错误，则进入找Z阶段
                }
                else 
                {
                    PostErrMsg(UVWLINKERR);        //UVW接线错误，需要任意调换两相
                }
            }
        }
        else      //在判定完UVW接线正确后开始不断运行找Z信号
        {
            STR_MTR_Gvar.AngInt.StartFindZ = 1;   //如不辨识UVW，则直接进入找Z阶段
            
            if(STR_MTR_Gvar.AngInt.AngInitZFlag == 0)
            {
                if((TimeDelay & 0x0f) == 0)
                {
                    CurAng++;

                    if(CurAng >= STR_MTR_Gvar.FPGA.AngToCnt)      //转过一个电周期后重新赋值
                    {
                        CurAng -= STR_MTR_Gvar.FPGA.AngToCnt;
                        LoopCnt ++;  
                    }

                    *HostAng =  CurAng;
                }
            }
        }

        if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN)   //找Z过程中发生故障
           || (LoopCnt > (2 * FunCodeUnion.code.MT_PolePair)))     //转了2圈都没有找到Z信号
        {
            ZErrFlag  = 1;
        }

        if((STR_MTR_Gvar.AngInt.AngInitZFlag == 1) || (ZErrFlag == 1))   //成功找到Z或发生故障都进入下一步
        {
            LoopCnt      = 0;
            CurAng       = 0;
            TimeDelay    = 0;
            EncResetStep = 4;		//进入下一步           
        }
	}
    else if(EncResetStep == 4)           //结束辨识
    {
		STR_MTR_Gvar.AngInt.StartFindZ   = 0;    //不再找Z信号
	    STR_MTR_Gvar.AngInt.AngInitZFlag = 0;    //清除Z检测标志位
			
		//ZIntDrvClear();           //屏蔽Z中断使能        

        EncResetStep = 5;      //进入下一步
    }
	else if(EncResetStep == 5)
	{
		STR_MTR_Gvar.GetIqRef.IqRef = 0;

        if(STR_MTR_Gvar.GetIqRef.IdRef > 0)       //电流依次递减，避免冲击
        {
            STR_MTR_Gvar.GetIqRef.IdRef -= 2;
        }
        else
        {
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.IncEncAngInitEn = 0;   //提示MTR模块结束角度辨识并不再内部使能
            STR_MTR_Gvar.GetIqRef.IdRef = 0;
		
		    if(0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdToqLoopNact)  //延时确认完成辨识后结束
		    {
			    EncResetStep = 6;
		    }
            *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;         //结束辨识
        }
	}
	else if(EncResetStep == 6)		//参数还原
    {
		EncResetStep = 0;
		
		AuxFunCodeUnion.code.OEM_ResetABSTheta = 0;              //结束调整

		 //得到零度和Z相脉冲的位置偏差
        if(STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 0)   //正向计数时
        {
            Temp1 = (STR_MTR_Gvar.AngInt.AbsZLatchPos - InitPos) % STR_MTR_Gvar.FPGA.AngToCnt;	
		}
        else                           //反向计数时
        {
            Temp1 = (InitPos - STR_MTR_Gvar.AngInt.AbsZLatchPos) % STR_MTR_Gvar.FPGA.AngToCnt;
        }
        
        		
        if(ZErrFlag == 0)    //只有在正确找到Z时才进行功能码更新
        {
            FunCodeUnion.code.MT_InitTheta = (Temp1 * 3600) / STR_MTR_Gvar.FPGA.AngToCnt; 	//得到Z信号对应的电角度
		
    		FunCodeUnion.code.MT_UposedgeTheta = FunCodeUnion.code.MT_InitTheta;
		
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_InitTheta)); 		    //保存辨识出的电角度
    		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_UposedgeTheta));

    		PostErrMsg(PCHGDWARN);			   //需要重新上电生效
        }
        else
        {
            ZErrFlag = 0;
            PostErrMsg(ANGINITFAIL);		     //无法找到Z而报警，辨识失败
        }
    }
}


/*
功能：得到角度辨识时Z信号锁存的位置信息
*/
void AngInt_ZPosLatch(void)
{
    if(STR_MTR_Gvar.AngInt.StartFindZ == 1)	   //如果开始电角度辨识, 得到当前绝对位置
    {
    	STR_MTR_Gvar.AngInt.AbsZLatchPos = STR_MTR_Gvar.FPGA.PosFdbAbsValue;   
    	STR_MTR_Gvar.AngInt.AngInitZFlag = 1;      //Z相检测标志位
    }
}

/*
功能：对未知的省线式编码器进行初始定位,每次上电都执行，不用找Z信号
*/
void AngInt_IncEncReset(void)
{
	static Uint16 EncResetStep = 0;		//初始定位步骤
	static Uint32 TimeDelay = 0;        //时间延时
    static int32  UVWPos1 = 0;         //判断UVW接线是否准确得到的位置
    static int32  UVWPos2 = 0;
    static int32  UVWPos3 = 0;  
    static Uint32 CurAng  = 0;        //不断变化的电角度以检测UVW相序
    static Uint32 PulsCnt = 0;        //检测UVW相序电角度累加时转动的脉冲数	
	
	if(EncResetStep == 0)
	{
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.IncEncAngInitEn = 1;      //提示MTR模块开始角度辨识并内部使能		

		EncResetStep = 1;		
	}
	else if(EncResetStep == 1)
	{
	    *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl | 0x02;        //初始角辨识使能
	
		if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == RUN)
		{
			EncResetStep = 2;
		}
	}
	else if(EncResetStep == 2)
	{
        if(1 == AngInt_SearchElecAngle())	  //定位完成，找到了角度的临界点
		{
			EncResetStep = 3;
			TimeDelay    = 0;
		}
	}
    else if(EncResetStep == 3)     //找到位置后进行UVW相序的辨识
    {
        if(FunCodeUnion.code.ER_UVWIdenEn == 1)   //UVW相序辨识使能
        {
            if(TimeDelay == 0)
            {
                UVWPos1 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
                CurAng  = STR_AngInit.FxdAngPoint;        //获得最后定位时的电角度
                STR_MTR_Gvar.GetIqRef.IdRef = STR_MTR_Gvar.GetIqRef.IqRate_MT;
            }
            else if(TimeDelay < (3 * STR_MTR_Gvar.System.ToqFreq)          //第一次和第二次定位都用3s
                   || ((TimeDelay > (3 * STR_MTR_Gvar.System.ToqFreq)) 
                      && (TimeDelay < (6 * STR_MTR_Gvar.System.ToqFreq))))    
            {
                if((PulsCnt < STR_AngInit.StepPulsCnt)
                   && ((TimeDelay & 0x0f) == 0))     //避免电机动作过大
                {
                    CurAng++;
                    PulsCnt++;

                    if(CurAng > STR_MTR_Gvar.FPGA.AngToCnt)
                    {
                        CurAng -= STR_MTR_Gvar.FPGA.AngToCnt;
                    }
                }
            }
            else if(TimeDelay == (3 * STR_MTR_Gvar.System.ToqFreq))
            {
                PulsCnt = 0;              //累加脉冲数清零
                UVWPos2 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            }
            else
            {
                UVWPos3 = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
                //位置必须跟随电角度变化，UVW相序才能判定为正确的
                if(STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 0)
                {
                    if(((UVWPos3 - UVWPos2) < 0) || ((UVWPos2 - UVWPos1) < 0))
                    {
                        PostErrMsg(UVWLINKERR);        //UVW接线错误，需要任意调换两相
                    }
                }
                else
                {
                    if(((UVWPos3 - UVWPos2) > 0) || ((UVWPos2 - UVWPos1) > 0))
                    {
                        PostErrMsg(UVWLINKERR);        //UVW接线错误，需要任意调换两相
                    }                
                }

                EncResetStep = 4;        //结束辨识UVW相序，进入第四步
                PulsCnt      = 0;
            }

            *HostAng = CurAng;
            TimeDelay++;
        }
        else
        {
            EncResetStep = 4;       //不辨识UVW直接进入下一步
            TimeDelay    = 0;
        }
    }
	else if(EncResetStep == 4)
	{
        STR_MTR_Gvar.GetIqRef.IqRef = 0;

        if(STR_MTR_Gvar.GetIqRef.IdRef > 0)        //电流依次递减，避免冲击
        {
            STR_MTR_Gvar.GetIqRef.IdRef -= 2;
        }
        else
        {
            STR_MTR_Gvar.GetIqRef.IdRef = 0;

            *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;      //初始角辨识结束,D轴电流不为0时不能配置结束
		
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.IncEncAngInitEn = 0;   //提示MTR模块结束角度辨识

            if(0 == UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.SpdToqLoopNact)       //内部控制恢复时才结束辨识
            {		   			
			    EncResetStep = 5;
			    TimeDelay    = 0;
		    }
        }
	}
	else if(EncResetStep == 5)
	{			
		AuxFunCodeUnion.code.OEM_ResetABSTheta = 0;              //结束调整

        EncResetStep  = 0;                      //允许重复定位 		
	} 
}


/*
功能：不断给不同的电角度D轴电流，根据电机是否转动，判断所给的电角度是否恰好是
      电机的电角度，一旦电机发生转动，则根据转动方向判定给定电角度是逐步变大还是
      逐步变小，避免最终转子和给定电角度相差180度，如果电机转动方向和第一次转动方
      向相反，则进入二分法查找电角度过程.
*/
Static_Inline Uint16 AngInt_SearchElecAngle(void)
{	
	static Uint32 StepPulsCnt = 0;          //每步转动的位置脉冲数
    static int32 CurAnglePoint = 0;
    static int32 CurAnglePoint_Q12 = 0;
	static Uint16 FirTurnDir = 0;       //第一次定位时电机转动方向；1～逆时针转动，2～顺时针转动
	static Uint16 CurTurnDir = 0;      //本周期的电机方向；
    static Uint32 TimeDelay = 0;       //延时计数器
    static int32  CurPos;		       //保存的当前位置变量
    static int32  NewPos;              //辨识时每周期获取的新的位置
    static int32  DeltPos = 0;         //每步辨识时的位置变化量
	static Uint16 Step = 0;            //辨识的步骤
    static Uint16 StopCnt = 0;         //等待持续时间
    static Uint16 DichFlag = 0;        //二分差值标志位：0～顺序累加过程，1～二分查找过程
    static Uint16 FirstFlag = 0;       //得到初始电机转动方向标志位：1～得到了电机初始转动方向
	static Uint16 DivAngCnt = 0;      //辨识时一个电角度周期分成的份数

	DivAngCnt = FunCodeUnion.code.ER_AngCnt;

    StepPulsCnt = STR_MTR_Gvar.FPGA.AngToCnt / DivAngCnt;  //每步转动的位置脉冲数
    STR_AngInit.StepPulsCnt = StepPulsCnt;
	STR_AngInit.MotRunPuls  = (Uint16)((STR_MTR_Gvar.FPGA.AngToCnt * FunCodeUnion.code.ER_MotRunPuls) / 10000); 
    
    switch(Step)
    {
        case 0:             //在某一电角度处给定初始的D轴电流
            CurPos                      = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            NewPos                      = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            STR_MTR_Gvar.GetIqRef.IdRef = 0;                                     //给初始电流值
            STR_AngInit.FxdAngPoint     = ((StepPulsCnt * CurAnglePoint_Q12) >> 12);
            *HostAng                    = STR_AngInit.FxdAngPoint;      //给定初始电角度值 
            
            Step = 1;        //进入下一步

            break;

        case 1:          //调节D轴电流大小
            if((STR_MTR_Gvar.GetIqRef.IdRef < STR_MTR_Gvar.GetIqRef.IqRate_MT)
			&& ((TimeDelay & 0x0F) == 0x0F))  //连续累加并小于额定电流，注意不能累加太快
            {
                STR_MTR_Gvar.GetIqRef.IdRef ++;
            }

            if(DichFlag == 0)
            {
                //判断电机是否转动，数值可调整，避免电机自由滑动，用以判定旋转方向
                if((ABS(STR_MTR_Gvar.FPGA.PosFdbAbsValue - CurPos) > STR_AngInit.MotRunPuls))
                {
                     //逆时针旋转，正转
                    if(((STR_MTR_Gvar.FPGA.PosFdbAbsValue > CurPos) && (STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 0)) 
                       || ((STR_MTR_Gvar.FPGA.PosFdbAbsValue < CurPos) && (STR_MTR_Gvar.GlobalFlag.bit.RevlDir == 1))) 
                    {
                        CurTurnDir = 1;
                    }
                    else
                    {
                        CurTurnDir = 2;           //顺时针旋转，反转
                    }

                    if(FirstFlag == 0)         //第一次定位时电机转动方向
                    {
                        FirstFlag  = 1;
                        FirTurnDir = CurTurnDir;
                    }

                    TimeDelay = 0;
                    Step = 2;       //电机转动后停机过程
                }
                else
                {
                    TimeDelay++;
					//第一次找方向时如果到达一定电流则换一个电角度
					if((FirstFlag == 0) && (STR_MTR_Gvar.GetIqRef.IdRef == (STR_MTR_Gvar.GetIqRef.IqRate_MT>>1)))  
					{
					     if((CurAnglePoint > 1) || (CurAnglePoint < -1)) //两次更换电角度都没动，说明可能断线或堵死了
                         {
                              Step  = 4;                //结束辨识并报警

                              PostErrMsg(ANGINITFAIL);    //辨识失败
                         }
                         else
                         {
                            TimeDelay  = 0;
						    CurTurnDir = 0;
						    Step       = 2;
                         }
					}
                }
            }
            else        //电机已经发生转向
            {
                TimeDelay++;
            }

            if(TimeDelay > (STR_MTR_Gvar.System.ToqFreq * 4))     //延迟时间到，结束辨识
            {
                Step = 4;
            }

            break;

        case 2:                    //电机停机过程
            STR_MTR_Gvar.GetIqRef.IdRef = 0;

            if(NewPos == STR_MTR_Gvar.FPGA.PosFdbAbsValue)
            {
                StopCnt++;
            }
            else
            {
                StopCnt = 0;       //只要出现一次变化则认为电机动了
                NewPos  = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            }

            if(StopCnt >= 30)    //连续30个周期位置信息不变，认为电机不再转动，开始下一个扇区定位
            {
                Step    = 3;
                StopCnt = 0;
                DeltPos = NewPos - CurPos;    //获取每步辨识时的位置增量
                TimeDelay = 0;            //计数器复位保证第一步时正确给电角度

            }
            break;

        case 3:         //根据当前电机运行方向决定采用累加扇区方法，还是采用二分法的方法
            if(((CurTurnDir ^ FirTurnDir) & 0x03) == 3)      //当电机转动方向和初次转动方向相反时
            {
                DichFlag  = 1;        //只要出现一次方向相反即进入二分插值过程
            }

            if(DichFlag == 0)
            {
                if(CurTurnDir == 1)       //逆时针转动时扇区递减
                {
                    CurAnglePoint--;      
                }
                else                     //顺时针转动或不转动时扇区递增
                {
                    CurAnglePoint++;      
                }

                if(CurAnglePoint < 0)             //保证扇区在0～DivAngCnt之间
                {
                    CurAnglePoint += DivAngCnt;
                }
                else if(CurAnglePoint >= DivAngCnt)
                {
                    CurAnglePoint -= DivAngCnt;
                }

				CurAnglePoint_Q12 = (CurAnglePoint<<12) + (((int64)DeltPos<<12) / (int32)StepPulsCnt);
            }
            else
            {
                if(CurTurnDir == 1)       //逆时针转动时扇区递减,考虑辨识时转子动作范围
                {
                    CurAnglePoint_Q12 = (((2*CurAnglePoint)-1) << 11) + (((int64)DeltPos<<12) / (int32)StepPulsCnt);  //取中值
                }
                else if(CurTurnDir == 2)   //顺时针转动时扇区递增,考虑辨识时转子动作范围
                {
                    CurAnglePoint_Q12 = (((2*CurAnglePoint)+1) << 11) + (((int64)DeltPos<<12) / (int32)StepPulsCnt);  //取中值
                }
            }

            if(CurAnglePoint_Q12 < 0)
            {
                CurAnglePoint_Q12 += (DivAngCnt << 12);
            }
            else if(CurAnglePoint_Q12 >= (DivAngCnt << 12))
            {
                CurAnglePoint_Q12 -= (DivAngCnt << 12);
            }

            Step = 0;             //重新回到第一步注入电流
            break;
        
        case 4:                  //结束辨识，参数复位
            Step = 0;
            DeltPos   = 0;
            StopCnt   = 0;
            DichFlag  = 0;
            FirstFlag = 0;
            CurTurnDir = 0;
            FirTurnDir = 0;
            TimeDelay = 0; 
            CurAnglePoint = 0;
            CurAnglePoint_Q12 = 0;

            return 1;

        default:
           break;
    }

    return 0;
}


//搜索找到正确的占空比命令,1～找到合适的占空比，2～UVW相序未接
Uint16 AngInt_DutySearch(void)
{
    static Uint16 SearchStep = 0;      //查找占空比步骤
    static Uint16 TimeDelay = 0;       //延时计数器
    static Uint16 TunCarFrqFlg = 0;    //载频调整标志位，0～调整占空比，1～调整载频
    static Uint16 CurCarFreq = 0;      //当前载频设置
    static int32  Iu = 0;

    switch(SearchStep)
    {
        case 0:
            if(TunCarFrqFlg == 1)         //如果当前载频宽度不足以产生足够大的电流时，调整载频
            {
                CurCarFreq          = CurCarFreq - 200;        //载频变化
                STR_AngInit.PwmPrd  = FPGA_CLK / CurCarFreq;
                STR_AngInit.DutyInc = 0;                      //桥臂全开
                STR_AngInit.DutyU   = 0;
                *PWMPrd_Mode        = (1<<15) + STR_AngInit.PwmPrd - 1;  // 使用PWM对称模式，电流环采样=开关频率
                *CurSampCtrl        =  STR_AngInit.PwmPrd - (STR_AngInit.Tsamp>>1);   //重新配置采样点
            }
            else
            {
                CurCarFreq = STR_MTR_Gvar.System.CarFreq;     //初始载频设置
            }

            SearchStep = 1;

            break;

        case 1:
            SysCtrl_Reg.bit.PwmDisUH = 0;     //开启U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 1;     //关断U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 1;     //关断V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 0;     //开启V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥
            
            if(TimeDelay < 2)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //关闭使能
            }
            else if(TimeDelay == 2)     //给定不断调整的占空比
            {
                TimeDelay++;
                *HostDutyU   = STR_AngInit.DutyInc;      //U相上桥设置占空比
                *HostDutyV   = STR_AngInit.PwmPrd;       //V相下桥常开
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else if(TimeDelay == 3)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //关闭使能
            }
            else if(TimeDelay == 4)
            {
                Iu       = STR_MTR_Gvar.FPGA.IuFdb;  //获取电流大小
                *HostDutyU  = STR_AngInit.PwmPrd;  //U相上桥全关
                *HostDutyV  = 0;                   //V相下桥全关
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else                        //当全部桥臂关断后开始判定占空比是否已经满足要求
            {
                TimeDelay = 0;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //关闭使能命令

                if(Iu > FunCodeUnion.code.MT_RateCurrent)    //取额定电流时的占空比
                {
                    STR_AngInit.DutyU = STR_AngInit.DutyInc;   //得到最终的占空比
                    SearchStep        = 3;   //得到正确的占空比，参数复位
                }
                else
                {
                    if(STR_AngInit.DutyInc > 0)
                    {
                        SearchStep = 2;   //给VU相电流，避免一直给同一个方向的电流导致电机出现转动
                    }
                    else
                    {
                        if(Iu > 100)        //当电流大于1A且上下桥臂全开时，调整载频
                        {
                            TunCarFrqFlg = 1;
                            SearchStep   = 2;   //调整载频以获取足够大的电流,先给一个负向电流
                        }
                        else               //电流小于1A则认为UVW未接线，报警
                        {
                            TunCarFrqFlg = 0;
                            SearchStep   = 0;   //返回错误处理，报警
                            return 2;
                        }
                    }
                }
            }

            break;

        case 2:     //给V相上桥开通，U相下桥开通，给一个反向电流，避免一直给同向电流造成电机转动
            SysCtrl_Reg.bit.PwmDisUH = 1;     //关闭U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 0;     //开启U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 0;     //开启V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 1;     //关闭V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相下桥和V相上桥

            if(TimeDelay < 2)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //关闭使能
            }
            else if(TimeDelay  == 2)     //给定不断调整的占空比
            {
                TimeDelay++;
                *HostDutyU   = STR_AngInit.PwmPrd;       //U相下桥全开
                *HostDutyV   = STR_AngInit.DutyInc;       //V相上桥开同样的占空比
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else if(TimeDelay  == 3)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //关闭使能
            }
            else if(TimeDelay  == 4)
            {
                TimeDelay++;
                *HostDutyU   = 0;                  //U相下桥关闭
                *HostDutyV   = STR_AngInit.PwmPrd;  //V相上桥关闭
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能
            }
            else
            {
                TimeDelay  = 0;
                SearchStep = 0;          //重新给U相电流
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能

                if(STR_AngInit.DutyInc > 0)
                {
                    STR_AngInit.DutyInc--;   //加大占空比设置
                }
            }

            break;

        case 3:
            SearchStep = 0;
            TimeDelay = 0;
            TunCarFrqFlg = 0;
           
            FunCodeUnion.code.OEM_DutyU        = STR_AngInit.DutyU;  //将得到的U相占空比和载频保存
            FunCodeUnion.code.OEM_SrchCurFreq  = CurCarFreq;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_DutyU));
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OEM_SrchCurFreq));

            return 1;           //返回正确值

        default:
            break;
    }

    return 0;
}

//电流注入方向顺序为U进V出，V进U出，V进W出，W进V出，W进U出，U进W出
void AngInt_CurVct(void)
{
    static int16 CurVctStep = 0;      //相电流矢量辨识时步骤
    static int32 TimeDelay = 0;       //延时周期
    static int16 Iuv1 = 0;           //UV相的线电流,零点
    static int16 Iuv2 = 0;           
    static int16 Ivu1 = 0;           //VU的相电流,零点
    static int16 Ivu2 = 0;            
    static int16 Iuw1 = 0;           //UW相的线电流,零点
    static int16 Iuw2 = 0;           
    static int16 Iwu1 = 0;           //WU相的线电流,零点
    static int16 Iwu2 = 0;
    static int16 Ivw1 = 0;           //VW相的线电流,零点
    static int16 Ivw2 = 0;
    static int16 Iwv1 = 0;           //WV相的线电流，零点
    static int16 Iwv2 = 0;
    static int16 CosValue = 0;             //最终进行查表计算的余弦值
    static int32 Iy = 0;   //U,V,W三相电流值
    Uint16 i = 0;
    int32 temp11=0;
    int32 temp12= 0 ;
    int32  Temp1; 
//    int16 Tdly;                //电流硬件延时
    int16 Tsck = 0;               //Σ-Δ调制器时钟周期,100ns,对应AMC1203或HCPL-7860的时钟频率为10MHz
    int32 Iu = 0, Iv = 0, Iw = 0, Ix = 0; 
    Uint64 Temp = 0;
    Uint32 Ixy = 0;                 //三相电流值投影后得到的角度值

    Uint32 Theta_Q8 = 0;            //电角度值对应的数字量
    Uint16 Theta = 0;               //实际的电角度值

    //系统控制寄存器配置为主机给定占空比
    SysCtrl_Reg.bit.EncZDsbl    = 0;   //Z相脉冲被屏蔽
    SysCtrl_Reg.bit.ZPlsMonDsbl = 0;   //禁止Z相脉冲缺失监控
    SysCtrl_Reg.bit.ZCntMonDsbl = 0;   //禁止ABZ相计数监控
    SysCtrl_Reg.bit.PwmDutySrc  = 1;   //PWM占空比选择为主机给定

    
    //电压注入法防止电机此时转动
    if((CurVctStep>=1)&&(CurVctStep<10))
    {
        temp11 = STR_MTR_Gvar.FPGA.PosFdbAbsValue - InitPos;
        temp12 = (STR_MTR_Gvar.FPGA.AngToCnt * DEGREE10)/3600;
        if(ABS(temp11)>temp12)
        {
            PostErrMsg(ANGINITFAIL);
            ZErrFlag = 1;
            CurVctStep = 10;
        }
        
    }

    
    switch(CurVctStep)
    {
        case 0:
            if(FunCodeUnion.code.ER_DutySearchEn == 1)        //根据是否搜索占空比确定使用哪个载频
            {
                STR_AngInit.PwmPrd  = FPGA_CLK / STR_MTR_Gvar.System.CarFreq;
            }
            else
            {
                STR_AngInit.PwmPrd  = FPGA_CLK / FunCodeUnion.code.OEM_SrchCurFreq;
            } 

            STR_AngInit.DutyInc = STR_AngInit.PwmPrd - 200;                     //初始占空比设置
            //Tdly              = 1000 / FPGA_PRD;               //电流硬件延时,设置为1us
            Tsck                = 100 / FPGA_PRD;
            STR_AngInit.Tsamp   = 3 * 32 * Tsck;                    //得到AD采样时间，采用32抽取率
            *PWMPrd_Mode        = (1<<15) + STR_AngInit.PwmPrd - 1;     //使用PWM对称模式，电流环采样=开关频率
            *DeadTmCtrl         = 0;                              //死区时间设置为0
            *DeadTmComp         = 0x8000;                        //死区补偿时间设置为0；
            *CurSampCtrl        = STR_AngInit.PwmPrd - (STR_AngInit.Tsamp>>1);   //设置电流采样点在全通桥臂关断处
            CurVctStep          = 1;                            //进入下一步

            UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            InitPos   =   STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            break;

        case 1:          //通过检测电流来得到正确的占空比
            
            if(FunCodeUnion.code.ER_DutySearchEn == 1)    //占空比搜索使能的情况才进行搜索，否则直接用占空比设置
            {
                i = AngInt_DutySearch();
            
                if(i == 1)           //找到设置的占空比后进入下一步
                {
                    *CurSampCtrl = STR_AngInit.PwmPrd - (STR_AngInit.Tsamp>>1) - (STR_AngInit.DutyU>>1);  //设置采样点
                    CurVctStep   = 2;
                }
                else if(i == 2)           //辨识失败，参数复位
                {
                    CurVctStep = 10;

                    PostErrMsg(ANGINITFAIL);
                }
            }
            else
            {
				STR_AngInit.DutyU = FunCodeUnion.code.OEM_DutyU;      //直接给定占空比

                *CurSampCtrl = STR_AngInit.PwmPrd - (STR_AngInit.Tsamp>>1) - (STR_AngInit.DutyU>>1);  //设置采样点
                
                CurVctStep = 2;                 //直接进入下一步
            }

            break;

        case 2:                   //开启U相上桥，V相下桥
            SysCtrl_Reg.bit.PwmDisUH = 0;     //开启U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 1;     //关断U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 1;     //关断V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 0;     //开启V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyU   = STR_AngInit.DutyU;    //U相给一定占空比
                *HostDutyV   = STR_AngInit.PwmPrd;   //V相下桥常开
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令                
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Iuv1  = STR_MTR_Gvar.FPGA.IuFdb;   //得到UV相电流的零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Iuv2         = STR_MTR_Gvar.FPGA.IuFdb;   //得到UV相电流
                *HostDutyU   = STR_AngInit.PwmPrd;    
                *HostDutyV   = 0;
                TimeDelay++;
                
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else
            {
                TimeDelay    = 0;
                CurVctStep   = 3;               //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }

            break;

        case 3:        //开V相上桥，U相下桥
            SysCtrl_Reg.bit.PwmDisUH = 1;     //关断U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 0;     //开启U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 0;     //开启V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 1;     //关断V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyU   = STR_AngInit.PwmPrd;   //U相下桥全开
                *HostDutyV   = STR_AngInit.DutyU;    //V相上桥给一定占空比
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Ivu1         = STR_MTR_Gvar.FPGA.IvFdb;   //得到VU相电流，零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Ivu2         = STR_MTR_Gvar.FPGA.IvFdb;   //得到VU相电流
                *HostDutyU   = 0;    
                *HostDutyV   = STR_AngInit.PwmPrd;
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
			else
            {
                TimeDelay    = 0;
                CurVctStep   = 4;               //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            break;

        case 4:          //VW桥通断及占空比设置，开V相上桥，W相下桥
            
            SysCtrl_Reg.bit.PwmDisUH = 1;     //开启U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 1;     //关断U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 0;     //关断V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 1;     //开启V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 0;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyV   = STR_AngInit.DutyU;     //V相上桥给一定占空比
                *HostDutyW   = STR_AngInit.PwmPrd;    //W相下桥常开
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令                
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Ivw1          = STR_MTR_Gvar.FPGA.IvFdb;   //得到vw电流，零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Ivw2         = STR_MTR_Gvar.FPGA.IvFdb;   //得到vw电流
                *HostDutyV   = STR_AngInit.PwmPrd;           
                *HostDutyW   = 0;       
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
			else
            {
                TimeDelay    = 0;
                CurVctStep   = 5;               //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }

            break;

        case 5:         //VW桥通断及占空比设置，开W相上桥，V相下桥
            SysCtrl_Reg.bit.PwmDisUH = 1;     //关断U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 1;     //开启U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 1;     //开启V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 0;     //关断V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 0;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyV   = STR_AngInit.PwmPrd;  //V相下桥全开
                *HostDutyW   = STR_AngInit.DutyU;  //W相上桥给一定占空比
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令                
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Iwv1          = -STR_MTR_Gvar.FPGA.IvFdb;   //得到WV相电流，零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Iwv2         = -STR_MTR_Gvar.FPGA.IvFdb;   //得到WV相电流
                *HostDutyV   = 0;           
                *HostDutyW   = STR_AngInit.PwmPrd;       
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
			else
            {
                TimeDelay    = 0;
                CurVctStep   = 6;               //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            break;

        case 6:          //UW桥通断及占空比设置，开W相上桥，U相下桥
            SysCtrl_Reg.bit.PwmDisUH = 1;     //关断U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 0;     //开启U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 1;     //开启V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 1;     //关断V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 0;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 1;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和V相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyU   = STR_AngInit.PwmPrd;         //U相下桥全开
                *HostDutyW   = STR_AngInit.DutyU;          //W相上桥给一定占空比
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Iwu1          = -STR_MTR_Gvar.FPGA.IuFdb;   //得到WU相电流，零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Iwu2          = -STR_MTR_Gvar.FPGA.IuFdb;   //得到WU相电流
                *HostDutyU   = 0;           
                *HostDutyW   = STR_AngInit.PwmPrd;       
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
			else
            {
                TimeDelay    = 0;
                CurVctStep   = 7;            //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            break;

        case 7:              //UW桥通断及占空比设置，开U相上桥，W相下桥
            SysCtrl_Reg.bit.PwmDisUH = 0;     //开启U相上桥
            SysCtrl_Reg.bit.PwmDisUL = 1;     //关断U相下桥
            SysCtrl_Reg.bit.PwmDisVH = 1;     //关断V相上桥
            SysCtrl_Reg.bit.PwmDisVL = 1;     //开启V相下桥
            SysCtrl_Reg.bit.PwmDisWH = 1;     //关闭W相上桥
            SysCtrl_Reg.bit.PwmDisWL = 0;     //关闭W相下桥

            *HostSysCtrl = SysCtrl_Reg.all;     //只开通U相上桥和W相下桥

            if(TimeDelay < 4)
            {
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
            }
            else if((TimeDelay & 0x3) == 0)
            {
                *HostDutyU   = STR_AngInit.DutyU;        //U相上桥给一定占空比
                *HostDutyW   = STR_AngInit.PwmPrd;       //W相下桥常开
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令                
            }
            else if((TimeDelay & 0x3) == 1)
            {
                Iuw1         = STR_MTR_Gvar.FPGA.IuFdb;   //得到UW相电流，零点
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }
            else if((TimeDelay & 0x3) == 2)     //关闭桥臂
            {
                Iuw2         = STR_MTR_Gvar.FPGA.IuFdb;   //得到UW相电流
                *HostDutyU   = STR_AngInit.PwmPrd;           
                *HostDutyW   = 0;       
                TimeDelay++;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;    //开启使能命令
            }
			else
            {
                TimeDelay    = 0;
                CurVctStep   = 8;               //进入下一步
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;    //开启使能命令
            }

            break;

        case 8:                 //进行角度计算

            Iu = (Iuv2 - Iuv1) - (Ivu2 - Ivu1);      //获取电流矢量大小，U‘相
            Iv = (Ivw2 - Ivw1) - (Iwv2 - Iwv1);      //V'相
            Iw = (Iwu2 - Iwu1) - (Iuw2 - Iuw1);      //W’相

            Ix = ((Iu - Iw) * SQRT3_Q10) >> 1;     //X轴的分量大小,Q10格式
            Iy = (Iv << 10) - ((Iu + Iw) << 9);    //Y轴的分量大小，Q10格式
            Temp = (Uint64)((int64)Ix*Ix + (int64)Iy*Iy);
            Ixy  = qsqrt64(Temp);                 //获得直角三角形斜边长
            CosValue = ((int64)Ix << 15) / Ixy;

            CurVctStep   = 9;               //进入下一步

            break;
        
        case 9:       //计算角度拆分成两步

            i = 64;                          //注意从正弦表64开始，值是单调递减的
            while(CosValue < SinTable[i])
            {
                i++;

                if(i > 192)         //值超出范围
                {
                    i = 0;
                    break;
                }
            }

            if(CosValue == SinTable[i])         //查表定位角度值
            {
                Theta_Q8 = (i - 64) << 8;
            }
            else
            {
                Theta_Q8 = ((SinTable[i-1] - CosValue) << 8) / (SinTable[i-1] - SinTable[i]); //线性插值获取角度增量
                Theta_Q8 += (i-1-64)<<8;
            }

            Theta = (Theta_Q8 * 1800) >> 15;     //注意此时0～128的数字量对应角度值为0～180度，这个值带个小数点
			
            if(Iy < 0)                    //角度在第三或第四象限
            {
                Theta = 3600 - Theta;
            }
            AuxFunCodeUnion.code.DP_IdenTheta = Theta;

            *HostAng = (STR_MTR_Gvar.FPGA.AngToCnt * Theta) / 3600;     //将实际电角度给入FPGA中
            CurVctStep = 10;              //参数复位

            break;

        case 10:       //计算完电角度后进行参数复位

            InitPos   =   STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            
            if(TimeDelay == 0)        //参数复位
            {                
                *CurSampCtrl = STR_MTR_Gvar.FPGA.CurSampCtrl;
                *HostDutyU   = 0;
                *HostDutyV   = 0;
                *HostDutyW   = 0;
                *PWMPrd_Mode = STR_MTR_Gvar.FPGA.PWMPrdMode.all;
                *DeadTmCtrl  = STR_MTR_Gvar.FPGA.DeadTmCtrl;
                *DeadTmComp  = STR_MTR_Gvar.FPGA.DeadTmComp;

                TimeDelay ++;
            }
            else if(TimeDelay < 3)       //清除复位过程中FPGA可能产生的故障
            {
                TimeDelay ++;
            }
            else                         //结束辨识
            {
                TimeDelay  = 0;
                

                *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;
                
            if((FunCodeUnion.code.ER_AngIntSel == 2)||(ZErrFlag==1))
                {
                    AuxFunCodeUnion.code.OEM_ResetABSTheta = 0;       //结束辨识
                    CurVctStep = 0;

                }
                else
                {
                    CurVctStep = 11;
                     
                }
                
            }

            *HostAlarmClr = 1;
            *HostAlarmClr = 0;

            break;
        case 11: //寻找Z信号
           AuxFunCodeUnion.code.ResZeroIndexEn=0x10;
           STR_MTR_Gvar.AngInt.StartFindZ   = 1; //开始寻找Z信号
           STR_MTR_Gvar.AngInt.AngInitZFlag = 0;
           TimeDelay  = 0;
           
           CurVctStep = 12;
           
           break;
        case 12: //寻找Z信号
           if(AuxFunCodeUnion.code.ResZeroIndexEn==0x11)//保存好初始值
           {
               AuxFunCodeUnion.code.ResZeroIndexEn = 0x12 ;   
           }
           
           //检测Z信号
           if((STR_MTR_Gvar.AngInt.AngInitZFlag == 0)
           &&(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN))
           {
               temp11 = (STR_MTR_Gvar.FPGA.AngToCnt * (Uint32)FunCodeUnion.code.MT_PolePair * 5>>2);
               if((STR_MTR_Gvar.FPGA.PosFdbAbsValue-InitPos)>temp11)      //转过一个电周期后重新赋值
               {
                   ZErrFlag  = 1;       
               }
           }
           if(TimeDelay++>40000)
           {
               ZErrFlag  = 1;
           }
           
           if((1==STR_MTR_Gvar.AngInt.AngInitZFlag)||(1==ZErrFlag))
           {
               CurVctStep = 13;
           }
           
           break;
        case 13: 
           AuxFunCodeUnion.code.ResZeroIndexEn=0x20;
           TimeDelay = 0;
           //得到零度和Z相脉冲的位置偏差
           temp11 = STR_MTR_Gvar.AngInt.AbsZLatchPos - InitPos ;
           
           if(temp11> 0)   //正向计数时
           {
               Temp1 = ( temp11 + AuxFunCodeUnion.code.DP_IdenTheta * STR_MTR_Gvar.FPGA.AngToCnt /3600) % STR_MTR_Gvar.FPGA.AngToCnt;	
		   }
           else                           //反向计数时
           {
               Temp1 = (-temp11 + AuxFunCodeUnion.code.DP_IdenTheta * STR_MTR_Gvar.FPGA.AngToCnt /3600) % STR_MTR_Gvar.FPGA.AngToCnt;
           }
        
        		
           if(ZErrFlag == 0)    //只有在正确找到Z时才进行功能码更新
           {
                FunCodeUnion.code.MT_InitTheta = (Temp1 * 3600) / STR_MTR_Gvar.FPGA.AngToCnt; 	//得到Z信号对应的电角度
		
    		    FunCodeUnion.code.MT_UposedgeTheta = FunCodeUnion.code.MT_InitTheta;
		
                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_InitTheta)); 		    //保存辨识出的电角度
    		    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_UposedgeTheta));

    		    PostErrMsg(PCHGDWARN);			   //需要重新上电生效
          }
          else
          {
               ZErrFlag = 0;
               PostErrMsg(ANGINITFAIL);		     //无法找到Z而报警，辨识失败
          }

           STR_MTR_Gvar.AngInt.StartFindZ = 0;
           STR_MTR_Gvar.AngInt.AngInitZFlag = 0;
           CurVctStep = 14;

           break;
        case 14: //寻找Z信号结束
		   if(AuxFunCodeUnion.code.ResZeroIndexEn==0)
           {
               CurVctStep = 0;
               AuxFunCodeUnion.code.OEM_ResetABSTheta = 0; 
           }
         
           break;
        default:
            break;
    }
}
