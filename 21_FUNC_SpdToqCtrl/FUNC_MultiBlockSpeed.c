/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_MultiBlockSpeed.c
 创建人：何俊辉                创建日期：08.11.03
 修改人：姚虹                  修改时间：10.02.05 
 修改人：李浩                  修改时间：12.03.13
 描述： 

 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h" 
#include "FUNC_MultiBlockSpeed.h"
#include "FUNC_FricIdentify.h"



/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


#define   MULTI_BLOCK_SPD_UP_DOWM_MAX_REF         1000


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */





/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

STR_MULTI_BLOCK_SPEED     STR_MultiBlockSpeed;




/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

int64   MultiBlockDeltaSpdRise_Q16 = 0;
int64   MultiBlockDeltaSpdDown_Q16 = 0;


	
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void MultiBlockSpeedInit(void);
int32 MultiBlockSpeedDeal(void);




/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void MultiBlockGetNewPara(void);
Static_Inline void MultiBlockRunTimeCheck(void);	



/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:       
  描述:  
********************************************************************************/
void MultiBlockSpeedInit()
{
    Uint16   TimeUnit = 0;
    Uint32   BlockRiseTime[5];   //相应段对应的加速时间变量
    Uint32   BlockDownTime[5];   //相应段对应的减速时间变量
    Uint32   Temp = 0;           //加减速速度增量计算临时变量
    Uint16    i = 0;

    TimeUnit = (FunCodeUnion.code.MTS_RunTimeUnit==1)?  60 : 1 ;   

    //基准时间 0.1s或者基准时间 0.1min ,通过变量TIMESTAMP进行选择 =1 or 60
    STR_MultiBlockSpeed.BaseCounter = ((Uint32)STR_FUNC_Gvar.System.SpdFreq / 10) * TimeUnit;//隐含取整 

    /* 多段速度的16段参数存储到参数列表中去 */
    STR_MultiBlockSpeed.BlockCommand[0]=((int16)FunCodeUnion.code.MTS_SEC1_Ref);
    STR_MultiBlockSpeed.RiseDownMode[0]=FunCodeUnion.code.MTS_SEC1_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[0]=FunCodeUnion.code.MTS_SEC1_RunTime;

    STR_MultiBlockSpeed.BlockCommand[1]=((int16)FunCodeUnion.code.MTS_SEC2_Ref);
    STR_MultiBlockSpeed.RiseDownMode[1]=FunCodeUnion.code.MTS_SEC2_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[1]=FunCodeUnion.code.MTS_SEC2_RunTime; 

    STR_MultiBlockSpeed.BlockCommand[2]=((int16)FunCodeUnion.code.MTS_SEC3_Ref);
    STR_MultiBlockSpeed.RiseDownMode[2]=FunCodeUnion.code.MTS_SEC3_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[2]=FunCodeUnion.code.MTS_SEC3_RunTime;

    STR_MultiBlockSpeed.BlockCommand[3]=((int16)FunCodeUnion.code.MTS_SEC4_Ref);
    STR_MultiBlockSpeed.RiseDownMode[3]=FunCodeUnion.code.MTS_SEC4_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[3]=FunCodeUnion.code.MTS_SEC4_RunTime;

    STR_MultiBlockSpeed.BlockCommand[4]=((int16)FunCodeUnion.code.MTS_SEC5_Ref);
    STR_MultiBlockSpeed.RiseDownMode[4]=FunCodeUnion.code.MTS_SEC5_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[4]=FunCodeUnion.code.MTS_SEC5_RunTime;

    STR_MultiBlockSpeed.BlockCommand[5]=((int16)FunCodeUnion.code.MTS_SEC6_Ref);
    STR_MultiBlockSpeed.RiseDownMode[5]=FunCodeUnion.code.MTS_SEC6_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[5]=FunCodeUnion.code.MTS_SEC6_RunTime;

    STR_MultiBlockSpeed.BlockCommand[6]=((int16)FunCodeUnion.code.MTS_SEC7_Ref);
    STR_MultiBlockSpeed.RiseDownMode[6]=FunCodeUnion.code.MTS_SEC7_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[6]=FunCodeUnion.code.MTS_SEC7_RunTime;		

    STR_MultiBlockSpeed.BlockCommand[7]=((int16)FunCodeUnion.code.MTS_SEC8_Ref);
    STR_MultiBlockSpeed.RiseDownMode[7]=FunCodeUnion.code.MTS_SEC8_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[7]=FunCodeUnion.code.MTS_SEC8_RunTime;

    STR_MultiBlockSpeed.BlockCommand[8]=((int16)FunCodeUnion.code.MTS_SEC9_Ref);
    STR_MultiBlockSpeed.RiseDownMode[8]=FunCodeUnion.code.MTS_SEC9_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[8]=FunCodeUnion.code.MTS_SEC9_RunTime;

    STR_MultiBlockSpeed.BlockCommand[9]=((int16)FunCodeUnion.code.MTS_SEC10_Ref);
    STR_MultiBlockSpeed.RiseDownMode[9]=FunCodeUnion.code.MTS_SEC10_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[9]=FunCodeUnion.code.MTS_SEC10_RunTime;

    STR_MultiBlockSpeed.BlockCommand[10]=((int16)FunCodeUnion.code.MTS_SEC11_Ref);
    STR_MultiBlockSpeed.RiseDownMode[10]=FunCodeUnion.code.MTS_SEC11_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[10]=FunCodeUnion.code.MTS_SEC11_RunTime;

    STR_MultiBlockSpeed.BlockCommand[11]=((int16)FunCodeUnion.code.MTS_SEC12_Ref);
    STR_MultiBlockSpeed.RiseDownMode[11]=FunCodeUnion.code.MTS_SEC12_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[11]=FunCodeUnion.code.MTS_SEC12_RunTime;

    STR_MultiBlockSpeed.BlockCommand[12]=((int16)FunCodeUnion.code.MTS_SEC13_Ref);
    STR_MultiBlockSpeed.RiseDownMode[12]=FunCodeUnion.code.MTS_SEC13_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[12]=FunCodeUnion.code.MTS_SEC13_RunTime;

    STR_MultiBlockSpeed.BlockCommand[13]=((int16)FunCodeUnion.code.MTS_SEC14_Ref);
    STR_MultiBlockSpeed.RiseDownMode[13]=FunCodeUnion.code.MTS_SEC14_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[13]=FunCodeUnion.code.MTS_SEC14_RunTime;

    STR_MultiBlockSpeed.BlockCommand[14]=((int16)FunCodeUnion.code.MTS_SEC15_Ref);
    STR_MultiBlockSpeed.RiseDownMode[14]=FunCodeUnion.code.MTS_SEC15_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[14]=FunCodeUnion.code.MTS_SEC15_RunTime;

    STR_MultiBlockSpeed.BlockCommand[15]=((int16)FunCodeUnion.code.MTS_SEC16_Ref);
    STR_MultiBlockSpeed.RiseDownMode[15]=FunCodeUnion.code.MTS_SEC16_RiFaSel;
    STR_MultiBlockSpeed.RunTimeGived[15]=FunCodeUnion.code.MTS_SEC16_RunTime;

    /* 多段速度相关参数初始化 */
    STR_MultiBlockSpeed.RunFlag = 1;              //默认执行一次
    STR_MultiBlockSpeed.DataRefreshFlag = 1;      //默认进入后进行参数更新

    STR_MultiBlockSpeed.StepInc = 1;            //默认一段一段往后运行；
    STR_MultiBlockSpeed.CurrentBlockNum = 0;      //从第一段开始
    STR_MultiBlockSpeed.CurrentCommand = 0;       //初始给定速度

    STR_MultiBlockSpeed.Counter1 = 0;
    STR_MultiBlockSpeed.Counter2 = 0;

    /* 以多段速度作为指令输入时，加减速时间初始化 */ 
    BlockRiseTime[0] = 0;                                 //零上升时间0
    BlockDownTime[0] = 0;                                 //零下降时间0
          
    BlockRiseTime[1] = (Uint32)FunCodeUnion.code.MTS_RiseTime1;   //上升时间1
    BlockDownTime[1] = (Uint32)FunCodeUnion.code.MTS_FallTime1;   //下降时间1

    BlockRiseTime[2] = (Uint32)FunCodeUnion.code.MTS_RiseTime2;   //上升时间2
    BlockDownTime[2] = (Uint32)FunCodeUnion.code.MTS_FallTime2;   //下降时间2

    BlockRiseTime[3] = (Uint32)FunCodeUnion.code.MTS_RiseTime3;   //上升时间3
    BlockDownTime[3] = (Uint32)FunCodeUnion.code.MTS_FallTime3;   //下降时间3

    BlockRiseTime[4] = (Uint32)FunCodeUnion.code.MTS_RiseTime4;   //上升时间4
    BlockDownTime[4] = (Uint32)FunCodeUnion.code.MTS_FallTime4;   //下降时间4

    /* 存储多段速度功能使能时的，加减速时间对应的加减速增量列表 */
    for(i = 0; i < 5; i++)
    {
        if (i == 0)   // 0- 零加减速时间及其相应的加减速增量的处理
        {
            STR_MultiBlockSpeed.DeltaRiseArray_Q16[0] =  ((int64)(10000L * FunCodeUnion.code.SL_SpdMaxLimit)<<16) << 1;
            STR_MultiBlockSpeed.DeltaDownArray_Q16[0] =  ((int64)(10000L * FunCodeUnion.code.SL_SpdMaxLimit)<<16) << 1;   
        }
        else          // 1~4 段加减速时间及其加减速增量处理
        {
            /* 加速时的速度增量的计算 */
            if(BlockRiseTime[i] == 0)
            {
                STR_MultiBlockSpeed.DeltaRiseArray_Q16[i] = ((int64)(10000L * FunCodeUnion.code.SL_SpdMaxLimit)<<16) << 1;    //0加减速时间处理
            }
            else
            {
                Temp = (BlockRiseTime[i] * (Uint32)STR_FUNC_Gvar.System.SpdFreq);
                BlockRiseTime[i] = (Temp / 1000);                      //除于ms时间单位,等于1s内减速的次数
                STR_MultiBlockSpeed.DeltaRiseArray_Q16[i] = ((int64)(10000L * SPDUPDOWM_MAXREF)<<16) / BlockRiseTime[i];	 //每一次加速时的，加速的增量       
            }
            /* 减速时的速度增量的计算 */
            if(BlockDownTime[i] == 0)
            {
                STR_MultiBlockSpeed.DeltaDownArray_Q16[i] = ((int64)(10000L * FunCodeUnion.code.SL_SpdMaxLimit)<<16) << 1;   //0加减速时间处理
            }
            else
            {
                Temp = (BlockDownTime[i] * (Uint32)STR_FUNC_Gvar.System.SpdFreq);
                BlockDownTime[i] = (Temp / 1000);                   //除于ms时间单位,等于1s内减速的次数
                STR_MultiBlockSpeed.DeltaDownArray_Q16[i] = ((int64)(10000L * SPDUPDOWM_MAXREF)<<16) / BlockDownTime[i];  //每一次加速时的，加速的增量         
            }
        }
    }

    MultiBlockDeltaSpdRise_Q16 = STR_MultiBlockSpeed.DeltaRiseArray_Q16[0];
    MultiBlockDeltaSpdDown_Q16 = STR_MultiBlockSpeed.DeltaDownArray_Q16[0];
	if(STR_FUNC_Gvar.FricIdentify.EndIdenFlag == 1)    //结束辨识时计算摩擦力
	{
	    Fric_CoffCal();                                  //库仑摩擦系数计算
		STR_FUNC_Gvar.FricIdentify.EndIdenFlag = 0;	  //不再计算
	}
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:       
  描述:  
********************************************************************************/  
int32 MultiBlockSpeedDeal()
{
    Uint16 DIStatus = 0;
    Uint16 SoftStartTimeMode = 0; 

    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN) //当伺服off时，多段指令给出的是零；即多段指令只有在伺服on时才有效；
    {
        STR_MultiBlockSpeed.CurrentCommand = 0;
        return STR_MultiBlockSpeed.CurrentCommand;
    }

    if(FunCodeUnion.code.MTS_RefRunMode == 2)    //外部DI切换,进行当前段指令的选择；
    {
        /* 四个DI决定在哪一段运行 */
        DIStatus = (STR_FUNC_Gvar.DivarRegLw.bit.Cmd4<<3) + (STR_FUNC_Gvar.DivarRegLw.bit.Cmd3<<2) + 
                   (STR_FUNC_Gvar.DivarRegLw.bit.Cmd2<<1) + (STR_FUNC_Gvar.DivarRegLw.bit.Cmd1);

        if( STR_FUNC_Gvar.DivarRegLw.bit.DirSel == 0)   //方向切换；
        {
            STR_MultiBlockSpeed.CurrentCommand = (int32)(10000L * (int16)STR_MultiBlockSpeed.BlockCommand[DIStatus]);
        }
        else
        {
            STR_MultiBlockSpeed.CurrentCommand = (int32)(-10000L * (int16)STR_MultiBlockSpeed.BlockCommand[DIStatus]);
        }

        /* LH 修改：在使用Di选择当前多段速度的段数以及更新相应的段指令的同时，也得及时更新当前段所对应的软启动相应的加减速速度增量*/
        SoftStartTimeMode = STR_MultiBlockSpeed.RiseDownMode[DIStatus];   //根据当前运行的段数，选择当前段所对应的软启动的加减速时间段

        MultiBlockDeltaSpdRise_Q16 = STR_MultiBlockSpeed.DeltaRiseArray_Q16[SoftStartTimeMode];   //根据当前所选择的软启动加减速时间段，来获取相应的软启动加速时的速度增量
        MultiBlockDeltaSpdDown_Q16 = STR_MultiBlockSpeed.DeltaDownArray_Q16[SoftStartTimeMode];   //根据当前所选择的软启动加减速时间段，来获取相应的软启动减速时的速度增量
        /* LH 修改完 */
    }
    else  //没有用到Di切换选择当前指令，即单次或循环运行模式下的多段指令的选择
    {
        if(STR_MultiBlockSpeed.DataRefreshFlag && STR_MultiBlockSpeed.RunFlag) //已经进入下一段且未停机，需要更新指令；其中RunFlag决定单词运行模式运行完一轮后停机，而DataRefreshFlag则决定在当前段指令运行时间没完成时，不进行段指令的更新
        {
            MultiBlockGetNewPara();  //更新当前段的转速值，以及软启动所需要加减速速度增量值；
        }
        MultiBlockRunTimeCheck();   //查看是否走完所在段规定的运行时间； 
    }

    return STR_MultiBlockSpeed.CurrentCommand;
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述: 
********************************************************************************/ 
Static_Inline void MultiBlockGetNewPara()
{
    Uint16 RiseDownTimeMode = 0;
    Uint16 BlockRunTimeZeroCnt = 0;
    
    if(FunCodeUnion.code.MTS_RefRunMode == 0)  //多段速度运行一轮后停机模式
    {
        while( (STR_MultiBlockSpeed.RunTimeGived[STR_MultiBlockSpeed.CurrentBlockNum] == 0) && 
               (STR_MultiBlockSpeed.CurrentBlockNum < FunCodeUnion.code.MTS_ExeSects)
             )  //排除当前段指令运行时间为零的情况，若为零，则跳过当前段，执行下一段；    
        {
           STR_MultiBlockSpeed.CurrentBlockNum += STR_MultiBlockSpeed.StepInc;
        }
        
        if(STR_MultiBlockSpeed.CurrentBlockNum >= FunCodeUnion.code.MTS_ExeSects)
        {
            STR_MultiBlockSpeed.CurrentBlockNum = 0;    //停机时回到第一段；
            STR_MultiBlockSpeed.RunFlag = 0;            //多段速停机标志；
            STR_MultiBlockSpeed.CurrentCommand = 0;     //停机时给定速度为0；
        }
        else
        {
            STR_MultiBlockSpeed.CurrentCommand = (int32)(10000L * (int16)STR_MultiBlockSpeed.BlockCommand[STR_MultiBlockSpeed.CurrentBlockNum]);
        }

    }
    else     //多段速度循环运行模式
    {
        STR_MultiBlockSpeed.CurrentBlockNum = STR_MultiBlockSpeed.CurrentBlockNum % FunCodeUnion.code.MTS_ExeSects;  //不能放后面，避免while判断时已经超出范围；

        /* LH 修改 排除当前段指令运行时间为零的情况，若为零，则跳过当前段，执行下一段，连续16次都为零时，退出循环，防止成为死循环，但unTimeGiven最小时间不可能为0，由功能码属性决定 */
        while((STR_MultiBlockSpeed.RunTimeGived[STR_MultiBlockSpeed.CurrentBlockNum] == 0)&&(BlockRunTimeZeroCnt<16))
        {
            BlockRunTimeZeroCnt++;
            STR_MultiBlockSpeed.CurrentBlockNum += STR_MultiBlockSpeed.StepInc;
            STR_MultiBlockSpeed.CurrentBlockNum = STR_MultiBlockSpeed.CurrentBlockNum % FunCodeUnion.code.MTS_ExeSects;
        }
        if(BlockRunTimeZeroCnt>=16)     //LH 修改 防止上面while 成为死循环，可以有退出死循环的机制，但RunTimeGiven最小时间不可能为0，由功能码属性决定
        {
            BlockRunTimeZeroCnt = 0;
            STR_MultiBlockSpeed.CurrentCommand = 0;
        }
        else
        {
            STR_MultiBlockSpeed.CurrentCommand = (int32)(10000L * (int16)STR_MultiBlockSpeed.BlockCommand[STR_MultiBlockSpeed.CurrentBlockNum]);
        }
    }

    RiseDownTimeMode = STR_MultiBlockSpeed.RiseDownMode[STR_MultiBlockSpeed.CurrentBlockNum];   //根据当前运行的段数，选择当前段所对应的软启动的加减速时间段

    MultiBlockDeltaSpdRise_Q16 = STR_MultiBlockSpeed.DeltaRiseArray_Q16[RiseDownTimeMode];   //根据当前所选择的软启动加减速时间段，来获取相应的软启动加速时的速度增量
    MultiBlockDeltaSpdDown_Q16 = STR_MultiBlockSpeed.DeltaDownArray_Q16[RiseDownTimeMode];   //根据当前所选择的软启动加减速时间段，来获取相应的软启动减速时的速度增量

    STR_MultiBlockSpeed.DataRefreshFlag = 0;    //更新完成
	STR_FUNC_Gvar.FricIdentify.MultiBlockNum = STR_MultiBlockSpeed.CurrentBlockNum;   //赋值给全局变量
}


/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数:        
  描述: 
********************************************************************************/  
Static_Inline void MultiBlockRunTimeCheck()	
{
    //确定该段运行时间；
    if(STR_MultiBlockSpeed.Counter1 < STR_MultiBlockSpeed.RunTimeGived[STR_MultiBlockSpeed.CurrentBlockNum]) //当前段运行时间；
    {
        if(STR_MultiBlockSpeed.Counter2 < STR_MultiBlockSpeed.BaseCounter)//每一秒所用的调度周期数；
        {
            STR_MultiBlockSpeed.Counter2 ++;
        }
        else
        {
            STR_MultiBlockSpeed.Counter2 = 0;
            STR_MultiBlockSpeed.Counter1 ++;
        }
    }
    else
    {
        STR_MultiBlockSpeed.Counter1 = 0;

        STR_MultiBlockSpeed.DataRefreshFlag = 1;   //多段速更新标志

        STR_MultiBlockSpeed.CurrentBlockNum = STR_MultiBlockSpeed.CurrentBlockNum + STR_MultiBlockSpeed.StepInc;  //到下一段；
    }
}

/********************************* END OF FILE *********************************/

