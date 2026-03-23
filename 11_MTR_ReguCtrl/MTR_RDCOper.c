/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    MTR_RDCOper.c  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.
    2.   
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_Main.h"
#include "MTR_RDCOper.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */


/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
UNI_RDCSTATE_REG     RDCState_Reg;    //绝对式编码器运行状态
UNI_RDCERR_REG       RDCErr_Reg;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

extern volatile Uint16 *AbsEncCtrl;     //绝对式编码器设置
extern volatile Uint16 *AbsEncErr;      //绝对式编码器故障状态
extern volatile Uint16 *AbsAngLow;      //编码器绝对位置低16位
extern volatile Uint16 *AbsEncState;    //绝对式编码器运行状态

extern volatile Uint16 *HostAlarmClr;   //报警清除   
extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;

extern volatile Uint16 *ABSEncSel;          //旋转编码器控制



/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void RDConverterState(void);    //得到旋变解码芯片运行状态
void CalcRDCTransTime(void);     //计算总通讯时间

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 

/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述：
    1. 
    2.
********************************************************************************/
void CalcRDCTransTime(void)
{
    int32 Ts;          //电流环控制周期转换成的FPGA周期
    int32 Tmcu;        //MCU转矩指令处理时间转换成的FPGA周期
    int32 Tspd;        //速度计算时间转换成的FPGA周期
    int32 Tabsenc;     //串行编码器总通讯时间转换成的FPGA周期   
    int32 AbsTransferDelay;

    if(FunCodeUnion.code.OEM_ToqLoopFreqSel == 1)
    {
        Ts = 1000000000L / (FunCodeUnion.code.OEM_CarrWaveFreq << 1);    //获取ns时间
        Ts = Ts / FPGA_PRD;
    }
    else
    {
        Ts = 1000000000L / FunCodeUnion.code.OEM_CarrWaveFreq;    //获取ns时间
        Ts = Ts / FPGA_PRD;            
    }
    
    Tspd      = 100 / FPGA_PRD;                         //速度计算时间0.1us
    Tmcu      = ((int32)(int16)FunCodeUnion.code.OEM_IqCalTm * 10) / FPGA_PRD;

    Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 23000;
    
    Tabsenc   = Tabsenc / FPGA_PRD;

    STR_MTR_Gvar.AbsRom.AbsTransferTime = Tabsenc;  

    //串行编码器的通讯启动延时参数，只做保存
    AbsTransferDelay = (Ts - Tspd - Tmcu  + Ts - Tabsenc) >> 1;

    if(AbsTransferDelay > (Ts >> 1))
    {
        AbsTransferDelay -= (Ts >> 1);
    }

    *AbsEncCtrl = ((AbsTransferDelay & 0x1FFF) | 0x2000); 

	//编码器通讯波特率换算  2M 对应500ns
    STR_MTR_Gvar.AbsRom.AbsCommBaud = 500 / FPGA_PRD;

    *ABSEncSel = 0; 
}




/*******************************************************************************
  函数名:  void RDConverterState(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 得到旋变解码芯片运行状态
    2.
********************************************************************************/
void RDConverterState(void)
{
    static Uint8 TxErrSum = 0;     //报错次数
    static Uint8 ErrDly = 0;       //延时
    static Uint8 PwrOnDly = 0;       //延时

    RDCState_Reg.all = *AbsEncState;     //绝对式编码器运行状态
    RDCErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态

    AuxFunCodeUnion.code.DP_FpgaAbsRomErr = RDCErr_Reg.all;     //H0b28，绝对编码器故障信息显示

    //绝对式编码器位置反馈
    STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = ((int32)RDCState_Reg.bit.AbsAngHigh << 16) + (*AbsAngLow);
    
    if(PwrOnDly < 10)
    {
        PwrOnDly ++;

        if(RDCErr_Reg.bit.ParityError == 1)     
        {
            PostErrMsg(ROTTRANERR_Z2);
        }        
        if(RDCErr_Reg.bit.DataError == 1)     
        {
            PostErrMsg(ROTTRANERR_Z3);
        }
    }
    else
    {
        STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;
    }

    //最低3位
    if(RDCErr_Reg.bit.AbsCntError == 1)     //计数增量异常
    {
        PostErrMsg(ROTTRANERR_Z6);
    }
    
    if((RDCErr_Reg.bit.AbsZeroError == 1) || (RDCErr_Reg.bit.AbsDivError == 1))//编码器算法异常 ER.980
    {
        PostErrMsg(ABSMOTALARM);
    } 

    // RDC信号畸变（TX端） RDC跟踪失效（TX端）  容错
    if((RDCErr_Reg.bit.DosFlag == 1) || (RDCErr_Reg.bit.LotFlag == 1))
    {
        TxErrSum ++;
    }

    if(0 == TxErrSum)
    {
        ErrDly = 0;
    }    
    else if(3 >= TxErrSum)   //已经报错 开始计时
    {
        ErrDly ++;
        if(ErrDly > 8)          // 累计8个周期大于3次标志位为1 报错
        {
            TxErrSum = 0;
            ErrDly = 0;
        } 
    } 
    else
    {
         //开始报错        
        if(RDCErr_Reg.bit.DosFlag == 1)     
        {
            PostErrMsg(ROTTRANERR_Z4);
        }        
        if(RDCErr_Reg.bit.LotFlag == 1)     
        {
            PostErrMsg(ROTTRANERR_Z5);
        }
    }  
}


/********************************* END OF FILE *********************************/
