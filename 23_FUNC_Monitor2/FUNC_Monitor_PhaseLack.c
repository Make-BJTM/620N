/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_PhaseLack.c       缺相检测相关处理程序                                                 
 创建人：童文邹、肖明海      创建日期：2012.02.06
 修改人：朱祥华              修改日期：2012.02.06 
 描述： 缺相检测相关处理程序
     1.
 修改记录：  
    1. 朱祥华       2012.02.09
       变更内容：原有的缺相保护只根据硬件PL信号来判断，能保护在确实断了一相的情况下报出缺相，
       但会出现伺服运行，且重载急起停的时候出现误报情况。另实测缺一相时PL高电平信号从1ms至3ms不等
       因此将三相驱动电均断线和缺一相分开判断，其中三相驱动电均断线状态则根据主循环30ms监控PL高电平状态次数判断。
       缺一相状态则在电流环中监控，只当PL电平状态为0000，000，0000，1111才认为是有效高电平，
       然后每500ms监控有效高电平是否超过报错值如是则进行相关报错处理
       缺一相若1KW以上机型则直接报缺相，1kw以下220V允许两相输入且允许警告则报警告
       H0A00功能码ER_InPLProtectSel：0-使能故障禁止警告;1-使能故障和警告;2-禁止故障和警告
       另外三相驱动电均断线时不再进行判断是否缺一相
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_Monitor_PhaseLack.h"  
#include "FUNC_Monitor_DCBusVolt.h"
/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//暂无

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义*/  
STR_MONITOR_PHASELACK   STR_Monit_PhaseLack = {0}; //缺相结构体定义


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
extern STR_DCBUSVOLTAGEPROCESS STR_DCBusVolt;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitPL_Process(void);              //初始化PL信号滤波窗口宽度
void PL_1k_Monitor(void);                //输入电源缺主循环1K监控

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//暂无

/*******************************************************************************
  函数名:  InitPL_Process(void)     初始化PL信号滤波次数    ^_^
  输入:    电流环调度频率
  输出:    PL信号滤波窗口宽度
  子函数: 无
  描述:    初始化PL信号滤波窗口宽度，16K电流环调度频率时滤波8次，小于16K滤波6次
********************************************************************************/ 
void InitPL_Process(void)
{
    // 控制参数，即需初始化的参数
    Uint16 *PhslackConfig = (Uint16 *)(FPGA_BASE + (0x00000009 << 1));   //缺相检测配置

    #if POWERDRIVER_TYPE==POWDRV_IS650
	//低电平有效,滤波时间100us
    *PhslackConfig = 0x8FA1;
	#else
	//高电平有效,滤波时间100us
    *PhslackConfig = 0x0FA1;
    #endif
}

/*******************************************************************************
  函数名:  PL_1k_Monitor(void)     输入电源缺相监控程序（主循环1ms调度运算）
  输入:    有效缺相次数 SERVORUNSTATUS、H0A00、H0102驱动器型号
  输出:    驱动器缺相警告报警标志位
  子函数: 无
  描述:    原有的缺相保护只根据硬件PL信号来判断，能保护在确实断了一相的情况下报出缺相，
       但会出现伺服运行，且重载急起停的时候出现误报情况。另实测缺一相时PL高电平信号从1ms至3ms不等
       因此将三相驱动电均断线和缺一相分开判断，其中三相驱动电均断线状态则根据主循环30ms监控PL高电平状态次数判断。
       缺一相状态则在电流环中监控，只当PL电平状态为0000，000，0000，1111才认为是有效高电平，
       然后每500ms监控有效高电平是否超过报错值如是则进行相关报错处理
       缺一相若1KW以上机型则直接报缺相，1kw以下220V允许两相输入且允许警告则报警告
       H0A00功能码ER_InPLProtectSel：0-使能故障禁止警告;1-使能故障和警告;2-禁止故障和警告
       H0A00置2时直接认为三相己准备好InputRdyFlg = INPUT_RDY，且不进行缺相监控

       //IS650N
       全高正常供电
	   全缺为450HZ到800HZ内的脉冲
	   缺相间歇6ms高电平+脉冲，周期10ms，40ms检测周期内理论值4个周期
	   全为低:继电器故障且重载
	   低电平个数大于0且小于450HZ 继电器故障轻载
********************************************************************************/ 
#if POWERDRIVER_TYPE==POWDRV_IS650
void PL_1k_Monitor(void)
{
    //状态参数表，即DSP从FPGA中读取的参数
    Uint16 *PhslackState = (Uint16 *)(FPGA_BASE + (0x000000011 << 1));   //缺相检测状态

    static Uint16 PL_HighCnt = 30;              //缺相信号为高电平时计数
    static Uint16 PL_LowCnt = 0;                //缺相信号为低电平时计数
    static Uint16 PL_40msCnt = 0;               //40MS计数
    static Uint8  PLErr_DelayCnt =0;            //缺相报警报错延时次数
    static Uint8  PLErr_DelayCnt1 =0;           //缺相报警报错延时次数
    static int16  PhsLackCntLatch = 0;          //缺相信号上升沿计数器锁存
	static int16  PhsLackPLLatch  = 0;
    static Uint8  PL_HighLatch = 0;
    static Uint8  InputRdyFlgTemp = 0;          //中间变量
	static Uint8  PL_Highholdtim=0;
	static Uint8  LackphaseCnt=0;

    int16  ValidCnt = 0;                        //有效缺相信号累积计数
    Uint16 PhsLackStateTemp = 0;

    PhsLackStateTemp = *PhslackState;
    	
	if(FunCodeUnion.code.ER_InPLProtectSel == 2) //H0A.00==2 进入RDY状态
    {
        //若禁止缺相报错和报警则直接认为三相己准备好，且不进行缺相监控
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;
        InputRdyFlgTemp = INPUT_RDY; 

        PLErr_DelayCnt  = 0;
        PLErr_DelayCnt1 = 0;
        PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        PL_40msCnt = 0;
        PL_LowCnt  = 0;
		WarnAutoClr(POWERPLWARN); //清除电源缺相警告 
        return;
    }


    //电平判断
    if((PhsLackStateTemp & 0x8000) == 0x8000)     //若为高电平
    {
        PL_HighCnt ++;
		if(5>PL_HighLatch) PL_HighLatch++; 
        
		PL_LowCnt=0;
		
		PhsLackStateTemp = PhsLackStateTemp & 0x7FFF;
        ValidCnt = PhsLackStateTemp - PhsLackPLLatch ;
        ValidCnt = ValidCnt & 0x7FFF;
		
		if(ValidCnt==0)
		{
		    PL_Highholdtim++;
		}
		else
		{
		    PL_Highholdtim=0;
		}

		PhsLackPLLatch = PhsLackStateTemp & 0x7FFF;	
		
    }
    else
    {
        //STR_Monit_PhaseLack.PL_PinStatus = 0;
        PL_LowCnt ++;

	//缺相6ms高电平中，应检测无电平跳变
	if(PL_Highholdtim>=4)
    {
			LackphaseCnt++;
		    //PL_Highholdtim=0;
	    }
	
		PL_Highholdtim=0;
			
	    }


    //上电后一直没有检测到主回路有交流电源输入
    if(5 > PL_HighLatch)
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_NRD;
        PLErr_DelayCnt  = 0;
        PLErr_DelayCnt1 = 0;
        PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        PL_40msCnt      = 0;
        PL_HighCnt      = 0;
        return;
    }


    if(PL_40msCnt < 40)
    {
        PL_40msCnt ++;
    }
    else
    {
        PhsLackStateTemp = PhsLackStateTemp & 0x7FFF;
		
        ValidCnt = PhsLackStateTemp - PhsLackCntLatch;
        ValidCnt = ValidCnt & 0x7FFF;
        
		if((FunCodeUnion.code.OEM_VoltClass == 380) && (PL_HighCnt > 38)) //电源正常
        {
            InputRdyFlgTemp = INPUT_RDY;
			PLErr_DelayCnt1=0;
			PLErr_DelayCnt =0;
			STR_Monit_PhaseLack.PL_PinStatus = 0;
			
			
			WarnAutoClr(POWERPLWARN); //清除电源缺相警告
        }
		else if((ValidCnt >= 18)&&(ValidCnt<= 32)) //主回路断电,无交流电输入	450HZ以上到850 HZ以下
	    {
	        InputRdyFlgTemp = INPUT_NRD;
			PLErr_DelayCnt1=0;
			PLErr_DelayCnt =0;
			STR_Monit_PhaseLack.PL_PinStatus = 1;
			
			
			WarnAutoClr(POWERPLWARN); //清除电源缺相警告
	    }
		else if(LackphaseCnt>=3)
		{
			PLErr_DelayCnt1=0;
			PLErr_DelayCnt =0;
			
			STR_Monit_PhaseLack.PL_PinStatus = 0;
			if(FunCodeUnion.code.ER_InPLProtectSel == 0)
			{
			    PostErrMsg(POWERPL);       //电源线缺相
			}
			else
			{
			    PostErrMsg(POWERPLWARN);
			}
		}
	    else if((PL_LowCnt>= 38)&&(STR_DCBusVolt.SoftStartFlag == SS_Disable))                 //继电器故障重载
		{
	        //InputRdyFlgTemp = INPUT_NRD;
			PLErr_DelayCnt1++;
            if(PLErr_DelayCnt1 >= 3)
            {
                PostErrMsg(POWERPL3);   
            }
			WarnAutoClr(POWERPLWARN); //清除电源缺相警告
			STR_Monit_PhaseLack.PL_PinStatus = 0;
		}
		else if((STR_DCBusVolt.SoftStartFlag == SS_Disable)&&(ValidCnt >= 1)&&(ValidCnt< 18))//继电器轻载
		{
	        //InputRdyFlgTemp = INPUT_NRD;
			PLErr_DelayCnt++;
            if(PLErr_DelayCnt >= 3)
            {
                PostErrMsg(POWERPL3);  
				STR_Monit_PhaseLack.PL_PinStatus = 0;  
            }
			WarnAutoClr(POWERPLWARN); //清除电源缺相警告
			
		}
		else
		{
		   PLErr_DelayCnt1=0;
		   PLErr_DelayCnt =0;
		   WarnAutoClr(POWERPLWARN); //清除电源缺相警告

		}

        PL_40msCnt = 0;
        PL_HighCnt = 0;
		PL_LowCnt  = 0;
		LackphaseCnt=0;
		
		
		PhsLackCntLatch = PhsLackStateTemp;              
    }
	
    //掉电零速停机使能时
    //抱闸打开情况下掉电，由于抱闸函数已经进行了零速停机功能，此处不在使能
    if( (1 == FunCodeUnion.code.BP_PowOffZeroSpdStopEn)
      &&(RUN == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
      &&(INPUT_NRD == InputRdyFlgTemp)
      &&(INPUT_RDY == STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg)
      &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
    {        
        STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag = 1;
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;
    }
    else if((1 == STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag) &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
    {
        //让FUNC模块确定进入Nrd状态之后再对PowOffZeroSpdStopFlag标志位清零，否则停机过程有问题
        if (RUN != STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag = 0;
            STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = InputRdyFlgTemp;
        }
        else
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;                          
        }
    }
    else
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = InputRdyFlgTemp;                          
    }

    if(INPUT_NRD == InputRdyFlgTemp)
    {
        PLErr_DelayCnt = 0;
        PLErr_DelayCnt1 = 0;
        //PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        return;
    }

}

#else
void PL_1k_Monitor(void)
{
    //状态参数表，即DSP从FPGA中读取的参数
    Uint16 *PhslackState = (Uint16 *)(FPGA_BASE + (0x000000011 << 1));   //缺相检测状态

    static Uint16 PL_MonitCnt = 0;              //缺一相监控计数
    static Uint16 PL_HighCnt = 30;              //缺相信号为高电平时计数
    static Uint16 PL_LowCnt = 0;               //缺相信号为低电平时计数
    static Uint16 PL_30msCnt = 0;              //30MS计数
    static Uint8  PLErr_DelayCnt =0;            //缺相报警报错延时次数
    static Uint8  PLErr_DelayCnt1 =0;            //缺相报警报错延时次数
    static int16  PhsLackCntLatch = 0;          //缺相信号上升沿计数器锁存
    static Uint8  PL_LowLatch = 0;
    static Uint8  InputRdyFlgTemp = 0;                 //中间变量

    int16  ValidCnt = 0;                        //有效缺相信号累积计数
    Uint16 PhsLackStateTemp = 0;

    PhsLackStateTemp = *PhslackState;

    if(FunCodeUnion.code.ER_InPLProtectSel == 2) //H0A.00==2 进入RDY状态
    {
        //若禁止缺相报错和报警则直接认为三相己准备好，且不进行缺相监控
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;
        InputRdyFlgTemp = INPUT_RDY; 

        PL_MonitCnt = 0;
        PLErr_DelayCnt = 0;
        PLErr_DelayCnt1 = 0;
        PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        PL_30msCnt = 0;
        PL_LowCnt = 0; 
        return;
    }

    //电平判断
    if((PhsLackStateTemp & 0x8000) == 0x8000)     //若为高电平
    {
        STR_Monit_PhaseLack.PL_PinStatus = 1;

        PL_HighCnt ++;
        if(PL_HighCnt >= 30) PL_HighCnt = 30;
    }
    else
    {
        if(5 > PL_LowLatch) PL_LowLatch ++;

        STR_Monit_PhaseLack.PL_PinStatus = 0;
                
        PL_HighCnt =0;
        PL_LowCnt ++;
    }

    //上电后一直没有检测到主回路有交流电源输入
    if(5 > PL_LowLatch)
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_NRD;
        PL_MonitCnt = 0;
        PLErr_DelayCnt = 0;
        PLErr_DelayCnt1 = 0;
        PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        PL_30msCnt = 0;
        PL_LowCnt = 0;
        return;
    }

    if(PL_HighCnt >= 28) //主回路断电,无交流电输入
    {
        InputRdyFlgTemp = INPUT_NRD;
        PL_30msCnt = 0;
        PL_LowCnt = 0; 
    }
    else if(PL_30msCnt < 30)
    {
        PL_30msCnt ++;
    }
    else
    {
        if((FunCodeUnion.code.OEM_VoltClass == 380) && (PL_LowCnt > 28))
        {
            InputRdyFlgTemp = INPUT_RDY;
        }

        if((FunCodeUnion.code.OEM_VoltClass == 220) && (PL_LowCnt > 10)) 
        {
            //三相供电的时候PL_LowCnt应该为30
            //单项供电时缺相信号高电平占空比小于50%，10ms内PL_LowCnt > 4
            //主回供电
            InputRdyFlgTemp = INPUT_RDY;            
        }

        PL_30msCnt = 0;
        PL_LowCnt = 0;               
    }

    //掉电零速停机使能时
    //抱闸打开情况下掉电，由于抱闸函数已经进行了零速停机功能，此处不在使能
    if( (1 == FunCodeUnion.code.BP_PowOffZeroSpdStopEn)
      &&(RUN == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
      &&(INPUT_NRD == InputRdyFlgTemp)
      &&(INPUT_RDY == STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg)
      &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
    {        
        STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag = 1;
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;
    }
    else if((1 == STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag) &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
    {
        //让FUNC模块确定进入Nrd状态之后再对PowOffZeroSpdStopFlag标志位清零，否则停机过程有问题
        if (RUN != STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.PowOffZeroSpdStopFlag = 0;
            STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = InputRdyFlgTemp;
        }
        else
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = INPUT_RDY;                          
        }
    }
    else
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg = InputRdyFlgTemp;                          
    }

    if(INPUT_NRD == InputRdyFlgTemp)
    {
        PL_MonitCnt = 0;
        PLErr_DelayCnt = 0;
        PLErr_DelayCnt1 = 0;
        PhsLackCntLatch = PhsLackStateTemp & 0x7FFF;
        return;
    }

    //缺相判断
    PL_MonitCnt++; 
    if(PL_MonitCnt > 55) //每55ms判断缺相状态和根据功能码选择等条件进行故障处理，缺一相应有5次上升沿
    {
        //缺相信号上升沿计算
        PhsLackStateTemp = PhsLackStateTemp & 0x7FFF;
        ValidCnt = PhsLackStateTemp - PhsLackCntLatch;
        ValidCnt = ValidCnt & 0x7FFF;
        PhsLackCntLatch = PhsLackStateTemp;

        PL_MonitCnt = 0;   //计数清零

        //ER_InPLProtectSel功能码：0-使能故障禁止警告;1-使能故障和警告;2-禁止故障和警告
        if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != 0) 
         && (FunCodeUnion.code.ER_InPLProtectSel <2))
        {
            //55ms内缺一相有效信号次数大于4次小于等于6次时认为缺相有效
            if((ValidCnt >= 4)&&(ValidCnt <= 6))
            {   
                PLErr_DelayCnt1 = 0;
                //缺相处理滤波延时3个55ms                
                PLErr_DelayCnt++;
                if(PLErr_DelayCnt >=3)
                {
                    if(FunCodeUnion.code.OEM_ServoSeri >= 0x0006)
                    { 
                        //缺相情况，跟陈江彬沟通 S7R6以上机型均报电源线缺相故障20130214
                        PostErrMsg(POWERPL);       //电源线缺相
                    }  
                    //5R5允许两相输入且允许警告则报警告
                    else if((FunCodeUnion.code.OEM_ServoSeri == 0x0005) 
                         &&(FunCodeUnion.code.ER_InPLProtectSel == 1))
                    {
                        //报警告不满足条件时应该清除
                        PostErrMsg(POWERPLWARN);            //电源缺相警告
                    } 
               }
           }
           else if((FunCodeUnion.code.OEM_ServoSeri >= 0x0006) && (ValidCnt > 6))
           {
                PLErr_DelayCnt = 0;
                PLErr_DelayCnt1 ++;
                if(PLErr_DelayCnt1 >= 3)
                {
                    PostErrMsg(POWERPL2);  //由于继电器损坏引起
                }
           }
           else 
           {
               //若有一次缺相次未达标则清零延时计数和清除缺相警告
               PLErr_DelayCnt = 0;
               PLErr_DelayCnt1 = 0;
               WarnAutoClr(POWERPLWARN); //清除电源缺相警告
           }
        }
        else 
        {
            PLErr_DelayCnt = 0;
            PLErr_DelayCnt1 = 0;
        }
    }
}
#endif
/********************************* END OF FILE *********************************/
