/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名: FUNC_AI.c
 创建人：童文邹  姚虹   朱祥华   创建日期：2008.11
 修改人：熊飞                    修改日期：2011.11.24
 描述  ：
         1.依据3路AI的采样值，计算其所对应转矩或速度的PU值；
         2.AI死区处理；
         3.AI零偏手动、自动调整；
         4.AI滤波处理
 修改记录：
         1. 2011.12.5      熊飞
         变更内容： 1) 引进TI平台中采样电压超限(>11.5V)报警Er834，但只针对AI1和AI2,同时将112平台中的Er834更改为Er835,但只针对AI3;
                        需要在servo_errocode.h文件中添加更改成以下错误码：
                        #define  AI_SAMPLE_OV_ERR     0x0834     //采样电压过大故障
                        #define  FPGA_AD_SAMPLE_ERR   0x0835     // FPGA报出的AD采样故障
                    2) 将AiProcess()函数拆分为三个：Ai1Process()、Ai2Process()、Ai3Process()，系统将会分时段调用;
                    3) 引进TI平台中关于AI通道选用标志位的部分，未选中的通道将尽量减少处理，以节省CPU资源;但是对于TI中关于判断通道是否选用
                       的部分程序，做了简化，直接以AISrcFlag.bit.AIx为选用标志，为0代表未选，非零则被选；
                    4) 采用TI的处理方法，某通道AI的零偏自动校正功能只会在该通道被选用时起作用；
                    5) 更正死区处理函数中Uint16 Flag[3]为Uint16 Flag[4],因为当AI3调用死区函数时出现Flag[3]这一组员不存在的现象，而导致
                       AI3并未进行死区处理；
                    6) 原112#中ADC模块里对变量gstr_OnChipAdc.g_ADCChannelValue[ch]没有初始化赋值，会使ADC采样的第一个数字值是0,对应AI的输入
                       电压是12V，所以会在上电时报Er834（使用AI时）；但是通常都设有AI滤波时间2ms，所以这一错误会在上电时被滤掉，隐藏了这一问题；
                       现在需在ADC模块中添加初始化赋值：2048；
        2.2013.4.10 王治国
                参考台达和松下伺服，简化AI处理方法
                STR_FUNC_Gvar.AI.Ai1SampVolt表示模拟电压对应的数字量 32768 对应12V
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_AI.h"
#include "FUNC_FunCode.h"
//#include "FUNC_OperEeprom.h"
#include "FUNC_ManageFunCode.h"
#include "FUNC_ErrorCode.h" //为监控WarnReg.bit.PERRWARNFLG及PostErrMsg()而引入的，在修改监控程序后，应将其删除
#include "FUNC_ServoError.h" //G2_LH_12.31 主要是提供PstErrMsg()函数
#include "FUNC_Filter.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

   
//当AI采样的值大于11.5V 时（12V<==>32767）， 就报警Er834；AI采样电压过大，防止飞车!  此处为临界值
#define  AI_RunAwayV     31402L 

//AI采样10V对应的数字量
#define  A1_10V      27307L

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无


/* Private_TypesDefinitions --------------------------------------------------*/
/* 结构体变量定义 枚举变量定义 */
STR_AI_VAR  STR_AI;

STR_BILINEAR_LOWPASS_FILTER      AiChanel1Filter = BILINEAR_LOWPASS_FILTER_Defaults ;
STR_BILINEAR_LOWPASS_FILTER      AiChanel2Filter = BILINEAR_LOWPASS_FILTER_Defaults ;
STR_BILINEAR_LOWPASS_FILTER      AiChanel3Filter = BILINEAR_LOWPASS_FILTER_Defaults ;


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void AiConstInit(void);            //AI常数初始化函数，停机更新
void Ai1Process(void);             //AI1变量计算，实时调用
void Ai2Process(void);             //AI2，变量计算，实时调用
//void Ai3Process(void);             //AI3变量计算，实时调用
void AiShow(void);                 //H0B组 AI 采样电压值显示

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
//Static_Inline void AiZeroDriftAdjust(Uint16 AiChannel,int32 AI_Value);


/*******************************************************************************
  函数名:  void AiConstInit()
  输入  : 无
  输出  : 无 
  子函数: 
  描述  : AI常量初始化计算
********************************************************************************/
void AiConstInit(void)
{
#if DRIVER_TYPE == SERVO_620P_407VG

    
//    static Uint8 FirOnPower = 0;
//
//    if(0 == FirOnPower)
//    {
//        STR_FUNC_Gvar.AI.HighPrecisionAIEn = FunCodeUnion.code.OEM_HighPrecisionAIEn;
//        FirOnPower = 1;
//    }
//
//    //滤波器参数配置
//    AiChanel1Filter.Ts = (Uint16)(STR_FUNC_Gvar.System.PosPrd_Q10 >> 10);            //AI1
//    AiChanel1Filter.Tc = (int32)FunCodeUnion.code.AI1_InFltTime * 10L;     
//    AiChanel1Filter.InitLowPassFilter(&AiChanel1Filter);
//
//    AiChanel2Filter.Ts = (Uint16)(STR_FUNC_Gvar.System.PosPrd_Q10 >> 10);            //AI2
//    AiChanel2Filter.Tc = (int32)FunCodeUnion.code.AI2_InFltTime * 10L;     
//    AiChanel2Filter.InitLowPassFilter(&AiChanel2Filter);
//
////    AiChanel3Filter.Ts = (Uint16)(STR_FUNC_Gvar.System.PosPrd_Q10 >> 10);            //AI3
////    AiChanel3Filter.Tc = (int32)FunCodeUnion.code.AI3InFltTime * 10L;     
////    AiChanel3Filter.InitLowPassFilter(&AiChanel3Filter); 
//
//    //AI零漂单位由0.1mV换算到数字量
//    STR_AI.ZeroDrift1 = ((int32)((int16)FunCodeUnion.code.Ai1_ZeroDrift) << 15) /120000L;
//    STR_AI.ZeroDrift2 = ((int32)((int16)FunCodeUnion.code.Ai2_ZeroDrift) << 15) /120000L;
////    STR_AI.ZeroDrift3 = ((int32)((int16)FunCodeUnion.code.Ai3_ZeroDrift) << 15) /120000L;
//
//    //AI零漂单位由0.1mV换算到数字量
//    STR_AI.DeadT1 = ((int32)FunCodeUnion.code.AI1_DeadT <<15) /120000L;
//    STR_AI.DeadT2 = ((int32)FunCodeUnion.code.AI2_DeadT <<15) /120000L;
////    STR_AI.DeadT3 = ((int32)FunCodeUnion.code.AI3_DeadT << 15) /120000L;
//
//    //AI偏置单位由1mV换算到数字量
//    STR_AI.Offset1 = ((int32)((int16)FunCodeUnion.code.AI1_Offset)<<15) /12000L;
//    STR_AI.Offset2 = ((int32)((int16)FunCodeUnion.code.AI2_Offset)<<15) /12000L;
////    STR_AI.Offset3 = ((int32)((int16)FunCodeUnion.code.AI3_Offset) << 15) /12000L;

#endif
}

/*******************************************************************************
  函数名: void Ai1Process()
  输入  : 无  
  输出  : 无
  子函数: 
  描述  : 将AI1通道电压采样值处理
********************************************************************************/
void Ai1Process(void)
{
#if DRIVER_TYPE == SERVO_620P_407VG

//    int32   TempV1 = 0;
//
//    //滤波
//    AiChanel1Filter.Input = STR_FUNC_Gvar.ADC_Samp.AI1;
//    AiChanel1Filter.LowPassFilter(&AiChanel1Filter);
//    TempV1  = AiChanel1Filter.Output;
//
//    //对模拟量零漂进行调整
//    if(1 == AuxFunCodeUnion.code.FA_AIAdjustMode) AiZeroDriftAdjust(1,TempV1);
//
//    TempV1 -= STR_AI.ZeroDrift1;   //AI调零后的采样值
//
//    STR_AI.DispVolt1 = TempV1;
//
//    //防止AI输入电压过大，导致飞车
//    if(ABS(TempV1) > AI_RunAwayV)
//    {
//        TempV1 = 0;
//        PostErrMsg(AI_SAMPLE_OV_ERR);           //AI采样电压正向过压报警
//    }
//
//    //对模拟量偏置进行调整
//    TempV1 -= STR_AI.Offset1;
//
//    //死区处理
//    if(ABS(TempV1) < STR_AI.DeadT1) TempV1 = 0;
//
//    //10V遇限处理
//    if(TempV1 > A1_10V) TempV1 = A1_10V;
//    else if(TempV1 < (-A1_10V)) TempV1 = -A1_10V;
//
//    //接口
//    STR_FUNC_Gvar.AI.AI1VoltOut = TempV1;
#endif
}

/*******************************************************************************
  函数名: void Ai2Process()
  输入  : 无  
  输出  : 无
  子函数: 
  描述  : 将AI2通道电压采样值处理
********************************************************************************/
void Ai2Process(void)
{
#if DRIVER_TYPE == SERVO_620P_407VG

//    int32   TempV2 = 0;
//
//    //滤波
//    if(0 == STR_FUNC_Gvar.AI.HighPrecisionAIEn)
//    {
//        AiChanel2Filter.Input = STR_FUNC_Gvar.ADC_Samp.AI2;    
//    }
//    else
//    {
//        AiChanel2Filter.Input = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.HighPrecisionAI;     
//    }
//    AiChanel2Filter.LowPassFilter(&AiChanel2Filter);
//    TempV2 = AiChanel2Filter.Output;
//
//    //对模拟量零漂进行调整
//    if(2 == AuxFunCodeUnion.code.FA_AIAdjustMode) AiZeroDriftAdjust(2,TempV2);
//
//    TempV2 -= STR_AI.ZeroDrift2;   //AI调零后的采样值
//
//    STR_AI.DispVolt2 = TempV2;
//
//    //防止AI输入电压过大，导致飞车
//    if(ABS(TempV2) > AI_RunAwayV)
//    {
//        TempV2 = 0;
//        PostErrMsg(AI_SAMPLE_OV_ERR);           //AI采样电压正向过压报警
//    }
//
//    //对模拟量偏置进行调整
//    TempV2 -= STR_AI.Offset2;
//
//    //死区处理
//    if(ABS(TempV2) < STR_AI.DeadT2) TempV2 = 0;
//
//    //10V遇限处理
//    if(TempV2 > A1_10V) TempV2 = A1_10V;
//    else if(TempV2 < (-A1_10V)) TempV2 = -A1_10V;
//
//    //接口
//    STR_FUNC_Gvar.AI.AI2VoltOut = TempV2;
#endif
}

///*******************************************************************************
//  函数名: void Ai3Process()
//  输入  : 无  
//  输出  : 无
//  子函数:
//  描述  : AI3通道电压采样值处理
//********************************************************************************/
//void Ai2Process(void)
//{
//}

/*******************************************************************************
  函数名:  void AiAdjust(int32 TempV1, int32 TempV2, int32 TempV3)
  输入  :  
  子函数:  
  描述  :  对输入电压调整零位，监控零偏量以及输入电压:
********************************************************************************/
//Static_Inline void AiZeroDriftAdjust(Uint16 AiChannel,int32 AI_Value)
//{
#if DRIVER_TYPE == SERVO_620P_407VG
//
//#else
//    static int16 Count = 0;   //采样累加次数
//    int32 AvrV  = 0;   //模拟量采样的平均值
//    static int64 SumV  = 0;   //模拟量采样的累加值
//    static int8  AI1_SampleErr = 0; //0正常; 1AI1采样警告
//    static int8  AI2_SampleErr = 0; //0正常; 1AI1采样警告
////    static int8  AI3_SampleErr = 0; //0正常; 1AI1采样警告
//
//    if(Count < 4096)                         //连续采样4096次计算平均值
//    {
//        SumV += AI_Value;
//        Count++;
//    }
//    else
//    {
//        AvrV = SumV >> 12;
//        if((AvrV > 1366) || (AvrV < -1366))  //偏差过大 大于 500mV偏差
//        {
//            AvrV = 0;
//            switch(AiChannel)
//            {
//                case 1:
//                    AI1_SampleErr = 1;
//                    break;
//                case 2:
//                    AI2_SampleErr = 1;
//                    break;
//                //case 3:
//                    //AI3_SampleErr = 1;
//                    //break;
//            }
//        }
//        else
//        {
//            switch(AiChannel)
//            {
//                case 1:
//                    AI1_SampleErr = 0;
//                    break;
//                case 2:
//                    AI2_SampleErr = 0;
//                    break;
////                    case 3:
////                        AI3_SampleErr = 0;
////                        break;
//            }
//        }
//
//        if((AI1_SampleErr == 1) || (AI2_SampleErr == 1))
//        {
//            PostErrMsg(AIZEROBIASOVWARN);
//        }
//        else
//        {
//            WarnAutoClr(AIZEROBIASOVWARN);
//        }
//
//        Count = 0;
//        SumV  = 0;                                                      //允许继续调整
//        switch(AuxFunCodeUnion.code.FA_AIAdjustMode)
//        {
//            case 1:
//                //进入手动调整模式，要再次自动调整则需重新设置模式
//                AuxFunCodeUnion.code.FA_AIAdjustMode = 0;
//                STR_AI.ZeroDrift1 = AvrV;
//                FunCodeUnion.code.Ai1_ZeroDrift = (AvrV * 120000L) >> 15;
//                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.Ai1_ZeroDrift));
//                break;
//            case 2:
//                //进入手动调整模式，要再次自动调整则需重新设置模式
//                AuxFunCodeUnion.code.FA_AIAdjustMode = 0;
//                STR_AI.ZeroDrift2 = AvrV;
//                FunCodeUnion.code.Ai2_ZeroDrift = (AvrV * 120000L) >> 15;
//                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.Ai2_ZeroDrift));
//                break;
//            case 3:
////                //进入手动调整模式，要再次自动调整则需重新设置模式
////                AuxFunCodeUnion.code.FA_AIAdjustMode = 0;
////                STR_AI.ZeroDrift3 = AvrV * 10;
////                FunCodeUnion.code.Ai3ZeroDrift = (AvrV * 120000L) >> 15;
////                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.Ai3ZeroDrift));
////                break;
//            default:
//               break;
//       }
//   }
#endif
//}

/*******************************************************************************
  函数名: 
  输入  :  
  输出  :  
  子函数:  
  描述  :  
********************************************************************************/
void AiShow(void)
{
#if DRIVER_TYPE == SERVO_620P_407VG

//    AuxFunCodeUnion.code.DP_Ai1SampVolt  =  ((int32)STR_AI.DispVolt1 * 1200L)>>15;      //H0B21 Ai1采样电压值显示
//    AuxFunCodeUnion.code.DP_Ai2SampVolt  =  ((int32)STR_AI.DispVolt2 * 1200L)>>15;      //H0B22 Ai2采样电压值显示
////    AuxFunCodeUnion.code.DP_Ai3SampVolt  =  ((int32)STR_AI.DispVolt3 * 1200L  + (1 << 14))>>15;   //H0B23 Ai3采样电压值显示
#endif

}
/********************************* END OF FILE *********************************/
