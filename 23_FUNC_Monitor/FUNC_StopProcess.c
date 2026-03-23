/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_StopProcess.c
 创建人：王军干
 修改人：李浩                创建日期：11.12.08 
 描述： 
     1.
     2.

 修改记录：  
     1.xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_StopProcess.h"
#include "FUNC_ServoMonitor.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_FunCode.h"
#include "FUNC_GPIODriver.h"
#include "PUB_Main.h"
#include "CANopen_DeviceCtrl.h"
#include "CANopen_PP.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//-----------动态制动--------------------
#define   ENGD           0  
#define   DISGD          1  
/*wzg
//-----------使能与关PWM----------------
#define   ENPWM          0
#define   DISPWM         1
*/
#define  GDCHECK 0              //使能GD是否发生检测功能

#define  GD_STOP_SPD     15000000      //GD停机处理速度
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


//临时调用外部函数++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//extern void ServoContrDatClr(void);
//extern void EnableDriver(Uint16 a);
//临时调用外部函数++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void StopModeInit(void);
void StopProcess(void);
void WriteGDStop(Uint8 i);
void ServoStopStatus(Uint8 StopStateModSel);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
void ServoOffStopDeal(STR_SERVO_MONITOR *p);
void No1ErrStopDeal(STR_SERVO_MONITOR *p);
void No2ErrStopDeal(STR_SERVO_MONITOR *p);
void OverTravelStopDeal(STR_SERVO_MONITOR *p);
void EmergencyStopDeal(STR_SERVO_MONITOR *p);
void ServoStopStatus(Uint8 StopStateModSel);

/*******************************************************************************
  函数名: void WriteGDStop(Uint8 i)
  输入:   形参i 可取DISGD 或 ENGD；
  输出:   ST芯片的GPIOE0端口高低电平信号
  子函数: GPIO_WRITE2GD(i)                       
  描述:   进行GD/DB停机控制信号的使能或不使能。 
********************************************************************************/ 
/* 确保电流过流时不会对DB电路造成影响，即硬件过流时屏蔽此项功能，软件过流不影响,先不管这个GD功能 2010-08-03 童文邹 */
void WriteGDStop(Uint8 i) 
{
    if(/*(STR_FUNC_Gvar.Monitor.ErrCode == HWOVERCURRENT)||
       (STR_FUNC_Gvar.Monitor.ErrCode == OVERCURRENT)  ||
       (STR_FUNC_Gvar.Monitor.ErrCode == OUTSHORT2GND) || */  
       (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 0)
      ) // 以上情况下时不允许DB制动，伺服准备好之后才去响应GD处理
    {
        i = DISGD;      //DISD = 1
    }
    if (i == DISGD)
    {
        //GPIO_Disable_GD(); 
    }  
    else
    {
        //GPIO_Enable_GD();
    }
}


/*******************************************************************************
  函数名: void StopModeInit(STR_SERVO_MONITOR *p)
  输入:   功能码
  输出:   故障1停机方式和停机状态，故障2停机方式和停机状态，超程停机方式和停机状态
  子函数: PostErrMsg()             
  描述:   停机模式初始化，主要是根据功能码对如下变量进行初始化
          p->StopCtrlVar.Err1StopMode ；p->StopCtrlVar.Err1StopState ；
          p->StopCtrlVar.Err2StopMode ；p->StopCtrlVar.Err2StopState ；
          p->StopCtrlVar.OTStopMode   ；p->StopCtrlVar.OTStopState   ；
********************************************************************************/ 
void StopModeInit()
{
    //停机方式和停机状态切换速度条件
    STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd = (int32)(10000L * FunCodeUnion.code.BP_StopModStateCutSpd);
#if ECT_ENABLE_SWITCH
	STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd2 = (int32)(10000L * FunCodeUnion.code.BP_StopModStateCutSpd2);
#endif
    /* 伺服off时的停机方式和停机状态初始化 */	
    switch(FunCodeUnion.code.BP_StopSoff)
    {
        case 0:  STR_ServoMonitor.StopCtrlVar.SoffStopMode  = 0;  //停机方式: 0 自由停车
                 STR_ServoMonitor.StopCtrlVar.SoffStopState = 0;  //停机状态: 0 保持自由运行状态  
                 break;

        case 1:  STR_ServoMonitor.StopCtrlVar.SoffStopMode  = 2;  //停机方式: 1 零速停机
                 STR_ServoMonitor.StopCtrlVar.SoffStopState = 0;  //停机状态: 0 保持自由运行状态
                 break;
  
        default: PostErrMsg(DEFUALTERR);            //系统参数出故障了
                 break;
    }

    /* 故障1停机方式和停机状态初始化 */	
    switch(FunCodeUnion.code.BP_StopNo1)
    {
        case 0:  STR_ServoMonitor.StopCtrlVar.Err1StopMode  = 0;  //停机方式: 0 自由停车
                 STR_ServoMonitor.StopCtrlVar.Err1StopState = 0;  //停机状态: 0 保持自由运行状态  
                 break;

        case 1:  STR_ServoMonitor.StopCtrlVar.Err1StopMode  = 1;  //停机方式: 1 DB停车
                 STR_ServoMonitor.StopCtrlVar.Err1StopState = 0;  //停机状态: 0 保持自由运行状态
                 break;

        case 2:  STR_ServoMonitor.StopCtrlVar.Err1StopMode  = 1;  //停机方式: 1 DB停车
                 STR_ServoMonitor.StopCtrlVar.Err1StopState = 1;  //停机状态: 1 DB停机状态 
                 break;

        default: PostErrMsg(DEFUALTERR);            //系统参数出故障了
                 break;
    }

                           
    /* 超程时的停机方式和停机状态初始化 */
    switch(FunCodeUnion.code.BP_OPStop)
    {
        case 0:  STR_ServoMonitor.StopCtrlVar.OTStopMode  = STR_ServoMonitor.StopCtrlVar.Err1StopMode; //在该功能码=0，则与故意1或伺服Off时的停机方式和停机状态一样
                 STR_ServoMonitor.StopCtrlVar.OTStopState = STR_ServoMonitor.StopCtrlVar.Err1StopState;
                 break;
        
        case 1:  STR_ServoMonitor.StopCtrlVar.OTStopMode  = 2;    //停机方式: 3 零速停车
                 STR_ServoMonitor.StopCtrlVar.OTStopState = 2;    //停机状态: 0 超程位置锁定状态
                 break;

        case 2:  STR_ServoMonitor.StopCtrlVar.OTStopMode  = 2;    //停机方式: 3 零速停车
                 STR_ServoMonitor.StopCtrlVar.OTStopState = 0;    //停机状态: 0 保持自由运行状态
                break;

        default: PostErrMsg(DEFUALTERR);            //系统参数出故障了
                 break;
	}
    
    /* 故障2时的停机方式和停机状态初始化 */
    switch(FunCodeUnion.code.BP_StopNo2)
    {
        case 0:  STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 0;   //停机方式: 0 自由停车
                 STR_ServoMonitor.StopCtrlVar.Err2StopState = 0;   //停机状态: 0 保持自由运行状态
                 break;

        case 1:  STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 2;   //停机方式: 2 零速停车
                 STR_ServoMonitor.StopCtrlVar.Err2StopState = 0;   //停机状态: 0 保持自由运行状态  
                 break;

        case 2:  STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 2;   //停机方式: 2 零速停车
                 STR_ServoMonitor.StopCtrlVar.Err2StopState = 1;   //停机状态: 1 DB停机状态  
                 break;

        case 3:  STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 1;   //停机方式: 1 DB停车
                 STR_ServoMonitor.StopCtrlVar.Err2StopState = 1;   //停机状态: 1 DB停机状态 
                 break;

        case 4:  STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 1;   //停机方式: 1 DB停车
                 STR_ServoMonitor.StopCtrlVar.Err2StopState = 0;   //停机状态: 0 保持自由运行状态 
                 break;

        default: PostErrMsg(DEFUALTERR);             //系统参数出故障了 
                 break;
    }

    /* 紧急停机时的停机方式和停机状态初始化 */
    switch(0)
    {
        case 0:  STR_ServoMonitor.StopCtrlVar.EmergencyStopMode  = 2;  //停机方式：2 零速停车
                 STR_ServoMonitor.StopCtrlVar.EmergencyStopState = 2;  //停机状态：2 位置锁定状态
                 break;

        default: PostErrMsg(DEFUALTERR);             //系统参数出故障了 
                 break;
    }

    if (STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 1)  //抱闸情况下，故障2的停机方式强制为 零速停机，自由状态
    {
         STR_ServoMonitor.StopCtrlVar.Err2StopMode  = 2;   //停机方式: 2 零速停车
         STR_ServoMonitor.StopCtrlVar.Err2StopState = 0;   //停机状态: 0 保持自由运行状态  
    }

	#if ECT_ENABLE_SWITCH
    CanopenQuickStopProcess();//CANopen快速停机初始化
    CanopenHalt();//CANopen暂停初始化
	#endif
}


/*******************************************************************************
  函数名: void StopProcess(STR_SERVO_MONITOR *p)
  输入:   STR_Montior 结构体变量中的四种停机响应标志位
  输出:   伺服Off，故障1. 故障2, 超程停机处理标志位
  子函数: ServoOffStopDeal()伺服Off停机处理，No1ErrStopDeal()故障1停机处理 
          No2ErrStopDeal()故障2停机处理， OTDeal()超程停机处理      
  描述:   根据停机响应，作相应停机处理
********************************************************************************/ 
void StopProcess()
{
    //这块的优先级还需要讨论一下，现在的情况是  WZG1470
    //1. 超程后，如果发生故障1和故障2，虽然PostErrMsg()函数中OTStopAck标志位清零
    //   但是代码运行到OverTravelMonitor()函数后OTStopAck标志位又被置1
    //   这样超程后，No1ErrStopDeal(),No2ErrStopDeal()在超程解除前不会被执行
    //2. 超程后,Soff的处理
    //   超程后如果Soff,在OverTravelMonitor()函数中判断ServoRunStatus标志位和转速
    //   如果 ServoRunStatus=RDY且转速小于30rpm，OTStopAck无效，本函数中响应函数ServoOffStopDeal()
    //3. 多个停机功能间的优先级由高到低排列如下：
    //   紧急停机>超程停机>故障1停机>故障2停机>伺服off停机

    if(STR_ServoMonitor.StopCtrlFlag.bit.Err1StopAck == 1)    //根据故障1停机响应标志位，进行故障1停机处理
    {
        //假如有其它故障或警告停机标志位同时有效时，对其进行清零
        if (STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck == 1)
        {
            STR_ServoMonitor.StopCtrlFlag.bit.OTClrReguFlg = 1;
            STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;
        }

		//故障2
        if(STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck == 1)
        {
            STR_ServoMonitor.StopCtrlFlag.bit.FirErr2Stop = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck = 0;
        }

		#if ECT_ENABLE_SWITCH
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClrReguFlg = 1;
        }
	
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }
        #endif
        //执行故障1停机
        No1ErrStopDeal(&STR_ServoMonitor);
    }
    else if(STR_ServoMonitor.StopCtrlFlag.bit.Err2StopAck == 1)    //根据故障2停机响应标志位，进行故障2停机处理
    {
        //假如有其它故障或警告停机标志位同时有效时，对其进行清零
        if (STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck == 1)
        {
            STR_ServoMonitor.StopCtrlFlag.bit.OTClrReguFlg = 1;
            STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;
        }

		#if ECT_ENABLE_SWITCH
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClrReguFlg = 1;
        }
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }
		#endif
        No2ErrStopDeal(&STR_ServoMonitor);
    }
/*	
	//by huangxin201711_16 quickstop移到OTstop前面，并加入对OTStop的屏蔽等
	#if ECT_ENABLE_SWITCH
    else if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
    {
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }
		if (STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck == 1)
        {
            STR_ServoMonitor.StopCtrlFlag.bit.OTClrReguFlg = 1;
            STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck = 0;
        }
		if((STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck == 1) && 
           (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 1) &&
           (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) <= STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd)
          ) //此种情况是为了解决在快速停机嵌位有效时，又断使能时，能关闭PWM
        {
            STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
        }
		else
		{
			CanopenQuickStopDeal(&STR_ServoMonitor);
		}
	}
	#endif 
	//by huangxin201711_16 quickstop移到OTstop前面，并加入对OTStop的屏蔽等 	
*/	 

	else if(STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck == 1)        //根据超程停机响应标志位，进行超程停机处理
    {        
		#if ECT_ENABLE_SWITCH
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClrReguFlg = 1;
			DeviceControlVar.QuickStopFinishFlag =1;  //by huangxin20171211 发生超程，让quickstop结束，以退出状态机
        }
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }
		#endif
        if((STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck == 1) && 
           (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 1) &&
           (ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) <= STR_ServoMonitor.StopCtrlVar.StopModStateCutSpd)
          ) //此种情况是为了解决在超程停机嵌位有效时，又断使能时，能关闭PWM
        {
            STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
            STR_ServoMonitor.StopCtrlFlag.bit.OTClrReguFlg = 0;
        }
        else
        {
            OverTravelStopDeal(&STR_ServoMonitor);
        }
    }
    else if((STR_ServoMonitor.StopCtrlFlag.bit.ServoOffAck == 1) && 
            (STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == 1)
           )  //根据伺服OFF停机响应标志位进行，伺服OFF停机处理
    {    
		#if ECT_ENABLE_SWITCH
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck = 0;
            STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClrReguFlg = 1;
        }
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }
		#endif
        ServoOffStopDeal(&STR_ServoMonitor);
    }

	#if ECT_ENABLE_SWITCH
    else if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck == 1)
    {
        if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
        {
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 0;
		    STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck = 0;
        }

        CanopenQuickStopDeal(&STR_ServoMonitor);
    }
	#endif
  	
	#if	 ECT_ENABLE_SWITCH
    
    else if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck == 1)
    {
        CanopenHaltDeal(&STR_ServoMonitor);
    }
	#endif
    //进行嵌位判断处理

	#if ECT_ENABLE_SWITCH
    STR_FUNC_Gvar.MonitorFlag.bit.OTClamp = STR_ServoMonitor.StopCtrlFlag.bit.EMSClampFlg | 
                                           STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg|
                                           STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg|
                                           STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg;
	#else
    STR_FUNC_Gvar.MonitorFlag.bit.OTClamp = STR_ServoMonitor.StopCtrlFlag.bit.EMSClampFlg | 
                                           STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg;
    #endif
}


/*******************************************************************************
  函数名: void ServoOffStopDeal(STR_SERVO_MONITOR *p)
  输入:   STR_Montior 结构体相应的变量以及标志位
  输出:   STR_Montior 结构体相应的变量及标志位 
  子函数: ServoStopRun()伺服Off停机方式选择，ServoStopStatus()伺服Off停机状态选择 
          WriteGDStop()GD停机控制，       
  描述:   伺服Off停机处理，根据标志位，相应的执行ServoStopMode()伺服Off停机方式和
          ServoStopStatus()伺服Off停机状态两函数
********************************************************************************/ 
void ServoOffStopDeal(STR_SERVO_MONITOR *p)
{  
    if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= p->StopCtrlVar.StopModStateCutSpd)
    {
        if(1 == STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag)
        {
            p->StopCtrlVar.SoffStopMode = 2;
        }

        ServoStopRun(p->StopCtrlVar.SoffStopMode);    //伺服OFF的停机方式和No1故障停机的方式选择相同 
    }
    else   //停机状态处理 ，接近零速作用
    {
        ServoStopStatus(p->StopCtrlVar.SoffStopState);  //执行ServoOff停机状态
        STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

        if(0 == STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag)
        {
            p->StopCtrlFlag.bit.ServoOffAck = INVALID;      //ServoOff停机响应完毕，清响应标志位
        }
    }
}

/*******************************************************************************
  函数名: void No1ErrStopDeal(STR_SERVO_MONITOR *p)
  输入:   STR_Montior 结构体相应的变量以及标志位
  输出:   STR_Montior 结构体相应的变量及标志位 
  子函数: ServoStopRun()故障1停机方式选择，ServoStopStatus()故障1停机状态选择 
          WriteGDStop()GD停机控制，       
  描述:   故障1停机处理，根据标志位，相应的执行ServoStopRun()故障1停机方式和
          ServoStopStatus()故障1停机状态两函数
********************************************************************************/ 
void No1ErrStopDeal(STR_SERVO_MONITOR *p)
{
    if(p->StopCtrlFlag.bit.FirErr1Stop == 0)       //故障1停机，ServoStopRun()执行一次,并置相应的标志位，不需要控制模式改变 
    {
        p->StopCtrlFlag.bit.FirErr1Stop = 1;       //置ServoStopRun()执行一次标志位
        if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= p->StopCtrlVar.StopModStateCutSpd)
        {
            ServoStopRun(p->StopCtrlVar.Err1StopMode); //故障1停机过程处理
        }

        if(p->StopCtrlVar.Err1StopMode != 2) STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
        else if(p->StopCtrlVar.Err1StopMode != 3) STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
    }

    /*接近零速时，借助停机状态标志位StopStateArrive ，转到停机状态处理方式*/
    if((UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb < p->StopCtrlVar.StopModStateCutSpd) && 
       (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb > -(p->StopCtrlVar.StopModStateCutSpd))
      )
    {
        ServoStopStatus(p->StopCtrlVar.Err1StopState);  //执行故障1停机状态       
        p->StopCtrlFlag.bit.Err1StopAck = INVALID;      //故障1处理完，清故障1响应标志位
        p->StopCtrlFlag.bit.FirErr1Stop = 0;            //清故障1执行一次停机标志位
        
        STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
        STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0; 
    }
}


/*******************************************************************************
  函数名: void No2ErrStopDeal(STR_SERVO_MONITOR *p)
  输入:   STR_Montior 结构体相应的变量以及标志位
  输出:   STR_Montior 结构体相应的变量及标志位 
  子函数: ServoStopRun()故障2停机方式选择，ServoStopStatus()故障2停机状态选择 
          WriteGDStop()GD停机控制，       
  描述:   故障2停机处理，根据标志位，相应的执行ServoStopRun()故障2停机方式和
          ServoStopStatus()故障2停机状态两函数
********************************************************************************/ 
void No2ErrStopDeal(STR_SERVO_MONITOR *p)
{
    if(p->StopCtrlFlag.bit.FirErr2Stop == 0)       //故障2停机，ServoStopRun()执行一次,并置相应的标志位，在没有超程时清除
    {
        p->StopCtrlFlag.bit.FirErr2Stop = 1;       //置ServoStopRun()执行一次标志位

        ServoStopRun(p->StopCtrlVar.Err2StopMode); //故障2停机过程处理

        if(p->StopCtrlVar.Err2StopMode != 2) STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
        else if(p->StopCtrlVar.Err2StopMode != 3) STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
    }    

    if((UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb < p->StopCtrlVar.StopModStateCutSpd) && 
       (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb > -(p->StopCtrlVar.StopModStateCutSpd))
      )
    {
        if( (p->StopCtrlVar.Err2StopMode == 2) && 
            (STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 0)
          )                                                 //若为零速停机 且 不使用抱闸功能，则正常作停机状态处理
        {
            ServoStopStatus(p->StopCtrlVar.Err2StopState);
            STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;                          
        }

        if(STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn == 1)
        {
            p->BrakeFlag.bit.ErrZeroSpdBrake = 1;         
        }
                     
        p->StopCtrlFlag.bit.Err2StopAck = INVALID;     //执行完故障2响应，响应清零 
        p->StopCtrlFlag.bit.FirErr2Stop = 0;           //故障2执行一次的标志位清零
         
        STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0; 
    }
	if((FunCodeUnion.code.ER_RunAwaySel==0)//关闭飞车监测且无抱闸情况下，速度大于额定转速则关闭PWM，防止飞车时报警停不下来
		&&(STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn==0)
		&&(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb)>((Uint32)FunCodeUnion.code.MT_RateSpd*10000)))
	{
			ServoStopRun(0);
	}
}


/*******************************************************************************
  函数名: void OverTravelStopDeal(STR_SERVO_MONITOR *p)
  输入:   StopModeSel 停机模式选择
  输出:   关PWM标志位，零速停机标志位，GD停机方式标志位，置急停转矩标志位， 
  子函数: EnableDriver()开关PWM函数，PostErrMsg()故障处理函数        
  描述:   超程时停机的处理
********************************************************************************/ 
void OverTravelStopDeal(STR_SERVO_MONITOR *p)
{
#if ECT_ENABLE_SWITCH


    CanopenPPReset();
	CanopenPPPosBuffReset();

	if(FunCodeUnion.code.CM_ECATHost==2)
	{
		if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= p->StopCtrlVar.StopModStateCutSpd2)
	    {
	        ServoStopRun(p->StopCtrlVar.OTStopMode); //执行超程停机过程
	
	        if(p->StopCtrlVar.OTStopMode != 2) STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
	        else if(p->StopCtrlVar.OTStopMode != 3) STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
	
	        p->StopCtrlFlag.bit.OTClrReguFlg = 1;    //调节器变量清除一次标志位置位
	    }
	    //停机已接近零速，置停机状态标志 该标志位变量在速度采样处赋值，以保证实时性？？
	    else
	    {
	        /* 在正超程有效且指令(位置、速度或转矩)方向为正,
	           或者反超程有效且指令(位置、速度或转矩)方向为反时执行停机状态 */
	        if( ((STR_FUNC_Gvar.DivarRegLw.bit.Pot==1) && (STR_ServoMonitor.RunStateFlag.bit.RefDir == 0))||
	            ((STR_FUNC_Gvar.DivarRegLw.bit.Not==1) && (STR_ServoMonitor.RunStateFlag.bit.RefDir == 1))
	          )
	        {
	            ServoStopStatus(p->StopCtrlVar.OTStopState);     //执行超程停机状态处理
	
	            if(p->StopCtrlFlag.bit.OTClrReguFlg) //清除调节器变量，仅且清除一次 //是否有必要清除值得探讨!  
	            {
	                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1; //防止位置环下位置偏差不清零出现执行累计脉冲的函数，一直清除脉冲偏差，防止OT后报ER.b00
	                p->StopCtrlFlag.bit.OTClrReguFlg = 0; //防止位置环下位置偏差不清零出现执行累计脉冲的函数。
	            }
	        }
	
	        /* 在停机过程中有可能已置转矩停机标志位 ，在此处清除给转矩控制的标志位 */
	        if (p->StopCtrlVar.OTStopMode == 3 )
	        {
	            STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
	        }
	        else if (p->StopCtrlVar.OTStopMode == 2)
	        {
	            STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
	        }
	    }
	}
	else
#endif
	{
		if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= p->StopCtrlVar.StopModStateCutSpd)
	    {
	        ServoStopRun(p->StopCtrlVar.OTStopMode); //执行超程停机过程
	
	        if(p->StopCtrlVar.OTStopMode != 2) STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
	        else if(p->StopCtrlVar.OTStopMode != 3) STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
	
	        p->StopCtrlFlag.bit.OTClrReguFlg = 1;    //调节器变量清除一次标志位置位
	    }
	    //停机已接近零速，置停机状态标志 该标志位变量在速度采样处赋值，以保证实时性？？
	    else
	    {
	        /* 在正超程有效且指令(位置、速度或转矩)方向为正,
	           或者反超程有效且指令(位置、速度或转矩)方向为反时执行停机状态 */
	        if( ((STR_FUNC_Gvar.DivarRegLw.bit.Pot==1) && (STR_ServoMonitor.RunStateFlag.bit.RefDir == 0))||
	            ((STR_FUNC_Gvar.DivarRegLw.bit.Not==1) && (STR_ServoMonitor.RunStateFlag.bit.RefDir == 1))
	          )
	        {
	            ServoStopStatus(p->StopCtrlVar.OTStopState);     //执行超程停机状态处理
	
	            if(p->StopCtrlFlag.bit.OTClrReguFlg) //清除调节器变量，仅且清除一次 //是否有必要清除值得探讨!  
	            {
	                STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1; //防止位置环下位置偏差不清零出现执行累计脉冲的函数，一直清除脉冲偏差，防止OT后报ER.b00
	                p->StopCtrlFlag.bit.OTClrReguFlg = 0; //防止位置环下位置偏差不清零出现执行累计脉冲的函数。
	            }
	        }
	
	        /* 在停机过程中有可能已置转矩停机标志位 ，在此处清除给转矩控制的标志位 */
	        if (p->StopCtrlVar.OTStopMode == 3 )
	        {
	            STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;
	        }
	        else if (p->StopCtrlVar.OTStopMode == 2)
	        {
	            STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
	        }
	    }
	}
}

/*******************************************************************************
  函数名: void EmergencyStopDeal(STR_SERVO_MONITOR *p)
  输入:   
  输出:    
  子函数: 
  描述:   紧急停机的处理
********************************************************************************/ 
void EmergencyStopDeal(STR_SERVO_MONITOR *p)
{
    if(ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) >= p->StopCtrlVar.StopModStateCutSpd)
    {
        ServoStopRun(p->StopCtrlVar.EmergencyStopMode); //执行超程停机过程

        if(p->StopCtrlVar.EmergencyStopMode != 2) STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;
        else if(p->StopCtrlVar.EmergencyStopMode != 3) STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 0;

		p->StopCtrlFlag.bit.EMSClrReguFlg = 1;
    }
    //停机已接近零速，置停机状态标志     
	else
	{
        /* 在停机过程中已置零速停机标志位 ，此处应清除零速停机的标志位，便于停机状态的执行 */
        ServoStopStatus(p->StopCtrlVar.EmergencyStopState);   //执行紧急停机状态处理
        STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 0;

        if(p->StopCtrlFlag.bit.EMSClrReguFlg) //清除调节器变量，仅且清除一次 
        {
            STR_FUNC_Gvar.MonitorFlag.bit.PosReguDatClr = 1; //防止位置环下位置偏差不清零出现执行累计脉冲报ER.b00
            p->StopCtrlFlag.bit.EMSClrReguFlg = 0;
        }
    }
} 


/*******************************************************************************
  函数名: void ServoStopRun(Uint8 StopModSel) 
  输入:   StopModeSel 停机模式选择
  输出:   关PWM标志位，零速停机标志位，GD停机方式标志位，置急停转矩标志位， 
  子函数: EnableDriver()开关PWM函数，PostErrMsg()故障处理函数        
  描述:  伺服停机方式选择，即伺服在旋转时，遇到停机指令后，停机方式的选择
********************************************************************************/ 
void ServoStopRun(Uint8 StopModSel)
{
    switch(StopModSel)
    {
        case 0:   STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;     //自由停机方式
                  break;
        
        case 1:   STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;     //使能GD停机方式，至相应的标志位   
                  break;

        case 2:   STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop = 1;        //零速停机方式标志位 
                  break;

        case 3:   STR_FUNC_Gvar.MonitorFlag.bit.ToqStop = 1;            //置急停转矩停机方式标志位
                  break;

#if ECT_ENABLE_SWITCH

        case 4:   STR_FUNC_Gvar.MonitorFlag.bit.SlopeStop = 1;     //停机方式: 4 不同模式下的斜坡停车
                  break;

        case 5:   STR_FUNC_Gvar.MonitorFlag.bit.SlopeStop = 1;     //停机方式: 5 急停减速度的斜坡停车
                  break;

#endif
        
        default:  STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                  PostErrMsg(DEFUALTERR);  //系统参数出故障了，关断PWM，并进行相应的故障处理   
                  break;
    }
}

/*******************************************************************************
  函数名: void ServoStopStatus(Uint8 StopStateModSel)
  输入:   StopStateModSel 停机状态模式选择
  输出:   关PWM标志位，零位固定标志位， 
  子函数: EnableDriver()开关PWM函数，PostErrMsg()故障处理函数，
          WriteGDStop()GD停机控制， DELAY_US()延迟子函数               
  描述:   伺服停机状态选择，即伺服在即将停止时，选择以哪种停机状态方式进行停机
********************************************************************************/ 
void ServoStopStatus(Uint8 StopStateModSel)
{
    switch(StopStateModSel)
    {
        case 0:   //关闭PWM
                 STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                 #if GDCHECK
                 DELAY_US(1);
                 #endif
                 WriteGDStop(DISGD);       //自由状态 //0-表示使能状态，1-表示关闭状态
                 break;

        case 1:  STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                 #if GDCHECK
                 DELAY_US(1);
                 #endif
                 WriteGDStop(ENGD);        //DB锁定状态,添加判断故障条件，当硬件过流禁止DB输出
                 break;

        case 2:  //嵌位标志位 
                 if (STR_ServoMonitor.StopCtrlFlag.bit.OTStopAck == 1)
                 {
                     STR_ServoMonitor.StopCtrlFlag.bit.OTClampFlg = 1;
                 }

        		 #if ECT_ENABLE_SWITCH
                 else if(STR_ServoMonitor.StopCtrlFlag.bit.CanQuickStopAck==1)
                 {
                     STR_ServoMonitor.StopCtrlFlag.bit.CanQuickClampFlg = 1;
                     
                     if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
                     {
                          STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;
 
                          #if GDCHECK
                          DELAY_US(1);
                          #endif                            
                          WriteGDStop(DISGD);   //解除动态制动
                     }
                 }
			
                 else if(STR_ServoMonitor.StopCtrlFlag.bit.CanHaltStopAck==1)
                 {
                     STR_ServoMonitor.StopCtrlFlag.bit.CanHaltClampFlg = 1;
                     
                     if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
                     {
                          STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = ENPWM;
 
                          #if GDCHECK
                          DELAY_US(1);
                          #endif                            
                          WriteGDStop(DISGD);   //解除动态制动
                     }
                 }
				 #endif
                 break;
        
        default: STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus = DISPWM;
                 PostErrMsg(DEFUALTERR); //系统参数出故障了，关断PWM，并进行相应的故障处理    
                 break;
    }
}


/********************************* END OF FILE *********************************/
