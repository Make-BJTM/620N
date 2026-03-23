/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_MTRInterface.c
 创建人:    王治国                  创建日期：2012.03.26
 描述:
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_MTRInterface.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_GlobalVariable.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义*/


/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
// FUNC模块传递给MTR模块32位变量(传递频率为4Hz)地址数组
Uint32 * const pFUNCToMTR_List_4Hz_32Bits_Addr[FUNCToMTR_LIST_4Hz_32Bits_NUM] =
{
    (Uint32 *) &STR_FUNC_Gvar.PosCtrl.KpCoef_Q15,                    //位置调节器量纲
    (Uint32 *) &STR_FUNC_Gvar.GainSW.SpdKf_Q12,                 //速度环调节器PDFF前馈系数0-4096 Disable时为4096
	(Uint32 *) &STR_FUNC_Gvar.GainSW.SpdDampingKf_Q12,          //速度环调节器Damping系数0-4096  Disable时为0，Max为4096

};

//FUNC模块传递给MTR模块(传递频率为16kHz)缓冲位结构体类型
static STR_FUNCToMTR_FLAGBUFF_16kHz    STR_FUNCToMTR_FlagBuff_16kHz;

// FUNC模块传递给MTR模块变量(传递频率为16kHz)地址数组
Uint32 * const pFUNCToMTR_List_16kHz_Addr[FUNCToMTR_LIST_16kHz_NUM] =
{
    //32位变量
    (Uint32 *) &STR_FUNCToMTR_FlagBuff_16kHz,                       //FUNC模块传递给MTR模块缓冲位结构体
    (Uint32 *) &STR_FUNC_Gvar.GainSW.SpdKp,                         //速度环调节器比例系数
    (Uint32 *) &STR_FUNC_Gvar.GainSW.SpdKi_Q10,                     //速度环调节器积分系数
    (Uint32 *) &STR_FUNC_Gvar.GainSW.ToqRefFilterTc,                //速度环调节器转矩前馈滤波时间参数
    (Uint32 *) &STR_FUNC_Gvar.ToqCtrl.SpdLmt,                       //转矩模式下的速度限制值
    (Uint32 *) &STR_FUNC_Gvar.ToqCtrl.ToqCmd,                       //转矩模式下转矩指令
    (Uint32 *) &STR_FUNC_Gvar.SpdCtrl.SpdRef,                       //速度调节器输入
    (Uint32 *) &STR_FUNC_Gvar.PosCtrl.PosRef,                       //位置调节器输入,供摩擦力过零补偿使用
    (Uint32 *) &STR_FUNC_Gvar.ADC_Samp.Udc_Live,                    //母线电压
    (Uint32 *) &STR_FUNC_Gvar.FsaCmd.FSAIqCmd,                        //开环频率特性分析电流指令输出
    (Uint32 *) &STR_FUNC_Gvar.FsaCmd.FSASpdCmd,                       //开环频率特性分析速度指令输出
    (Uint32 *) &STR_FUNC_Gvar.ToqCtrl.SpdLmtPos,
    (Uint32 *) &STR_FUNC_Gvar.ToqCtrl.SpdLmtNeg,
};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void G_FUNC_MTRGetList_4Hz_32Bits(Uint32 * pMTR_List_4Hz_32Bits_HeadAddr);
void G_FUNC_MTRGetList_16kHz(Uint32 * pMTR_List_16kHz_HeadAddr);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 

/*******************************************************************************
  函数名:
  输入:   被写入数组首地址 
  输出:   无 
  子函数: 无
  描述：
    1. 在初始化和主循环中调用，主循环中调用为频率4Hz
    2.
********************************************************************************/
void G_FUNC_MTRGetList_4Hz_32Bits(Uint32 * pMTR_List_4Hz_32Bits_HeadAddr)
{
    Uint16 i = 0;

    for(i=0;i<FUNCToMTR_LIST_4Hz_32Bits_NUM;i++)
    {
        *(pMTR_List_4Hz_32Bits_HeadAddr + i) = *pFUNCToMTR_List_4Hz_32Bits_Addr[i];
    }
}

/*******************************************************************************
  函数名: void G_FUNC_MTRGetList_16kHz(Uint32 * pMTR_List_16kHz_HeadAddr)
  输入:   被写入数组首地址 
  输出:   无 
  子函数: 无
  描述：
    1. 在FPGA触发外部中断中调用，调用为频率16kHz
    2.
********************************************************************************/
void G_FUNC_MTRGetList_16kHz(Uint32 * pMTR_List_16kHz_HeadAddr)
{

    //搜集FUNC传递给MTR的位变量
    STR_FUNCToMTR_FlagBuff_16kHz.RunMod             = STR_FUNC_Gvar.MonitorFlag.bit.RunMod;             //运行模式
    STR_FUNCToMTR_FlagBuff_16kHz.ServoRunStatus     = STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus;     //指示伺服运行状态的变量,根据使能状态、故障状态合成的变量
    STR_FUNCToMTR_FlagBuff_16kHz.PwmStatus          = STR_FUNC_Gvar.MonitorFlag.bit.PwmStatus;          //PWM状态标志位
    STR_FUNCToMTR_FlagBuff_16kHz.ToqStop            = STR_FUNC_Gvar.MonitorFlag.bit.ToqStop;            //急停减速使能标志
    STR_FUNCToMTR_FlagBuff_16kHz.ZeroSpdStop        = STR_FUNC_Gvar.MonitorFlag.bit.ZeroSpdStop;        //零速停机使能标志
    STR_FUNCToMTR_FlagBuff_16kHz.OTClamp            = STR_FUNC_Gvar.MonitorFlag.bit.OTClamp;            //发生超程时零位置停机固定标志位
    STR_FUNCToMTR_FlagBuff_16kHz.SpdReguDatClr      = STR_FUNC_Gvar.MonitorFlag.bit.SpdReguDatClr;      //速度环偏差等参数清零标志位
    STR_FUNCToMTR_FlagBuff_16kHz.AlmRst             = STR_FUNC_Gvar.DivarRegLw.bit.AlmRst;                //故障复位
    STR_FUNCToMTR_FlagBuff_16kHz.Pcl                = STR_FUNC_Gvar.DivarRegLw.bit.Pcl;                   //正转外部转矩限制
    STR_FUNCToMTR_FlagBuff_16kHz.Ncl                = STR_FUNC_Gvar.DivarRegLw.bit.Ncl;                   //反转外部转矩限制
    STR_FUNCToMTR_FlagBuff_16kHz.UVAdjustRatioEn    = STR_FUNC_Gvar.MonitorFlag.bit.UVAdjustRatioEn;      //UV相电流平衡校正使能
    STR_FUNCToMTR_FlagBuff_16kHz.SpdToqLoopNact     = STR_FUNC_Gvar.MonitorFlag.bit.SpdToqLoopNact;       //速度转矩环程序屏蔽标志位
    STR_FUNCToMTR_FlagBuff_16kHz.OffLnInertiaModeEn = STR_FUNC_Gvar.MonitorFlag.bit.OffLnInertiaModeEn;   //离线惯量辨识使能
    STR_FUNCToMTR_FlagBuff_16kHz.InnerServoSon      = STR_FUNC_Gvar.MonitorFlag.bit.InnerServoSon;        //内部使能标志
    STR_FUNCToMTR_FlagBuff_16kHz.ModSwitchPeriod    = STR_FUNC_Gvar.MonitorFlag.bit.ModSwitchPeriod;      //模式切换过度阶段标志位
    STR_FUNCToMTR_FlagBuff_16kHz.BrkOut             = STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut;              //泄放输出信号
    STR_FUNCToMTR_FlagBuff_16kHz.UdcOk              = STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk;               //母线电压Ok
    STR_FUNCToMTR_FlagBuff_16kHz.OpenFSAEn          = STR_FUNC_Gvar.Monitor2Flag.bit.OpenFSAEn;             //运行模式

    //传递变量
    //14个变量，如果用FOR循环赋值，赋值代码执行时间268/72  us
    //直接赋值，赋值代码执行时间50/72  us
    *(pMTR_List_16kHz_HeadAddr + 0) = *pFUNCToMTR_List_16kHz_Addr[0];
    *(pMTR_List_16kHz_HeadAddr + 1) = *pFUNCToMTR_List_16kHz_Addr[1];
    *(pMTR_List_16kHz_HeadAddr + 2) = *pFUNCToMTR_List_16kHz_Addr[2];
    *(pMTR_List_16kHz_HeadAddr + 3) = *pFUNCToMTR_List_16kHz_Addr[3];
    *(pMTR_List_16kHz_HeadAddr + 4) = *pFUNCToMTR_List_16kHz_Addr[4];
    *(pMTR_List_16kHz_HeadAddr + 5) = *pFUNCToMTR_List_16kHz_Addr[5];
    *(pMTR_List_16kHz_HeadAddr + 6) = *pFUNCToMTR_List_16kHz_Addr[6];
    *(pMTR_List_16kHz_HeadAddr + 7) = *pFUNCToMTR_List_16kHz_Addr[7]; 
    *(pMTR_List_16kHz_HeadAddr + 8) = *pFUNCToMTR_List_16kHz_Addr[8];
    *(pMTR_List_16kHz_HeadAddr + 9) = *pFUNCToMTR_List_16kHz_Addr[9];
    *(pMTR_List_16kHz_HeadAddr + 10) = *pFUNCToMTR_List_16kHz_Addr[10];
    *(pMTR_List_16kHz_HeadAddr + 11) = *pFUNCToMTR_List_16kHz_Addr[11];
    *(pMTR_List_16kHz_HeadAddr + 12) = *pFUNCToMTR_List_16kHz_Addr[12];

}

/********************************* END OF FILE *********************************/
