/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_Oscilloscope.c
 创建人：   邓开余                 创建日期：2009.06
 修改人：   王治国                 修改日期：2011.12.12
 描述： 
    1. 
    2. 
 缩写：
    1. osci -> oscillograph
    2. trig -> trigger
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.      
    2.    
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "Pub_main.h"
#include "FUNC_GlobalVariable.h" 
#include "FUNC_InterfaceProcess.h"
#include "FUNC_Oscilloscope.h"
#include "FUNC_FunCode.h"
#include "FUNC_COMMInterface.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//示波器缓冲区数组结构体
UNI_OSCILLOSCOPEBUFFER   UNI_OsciBuffer;

//示波器内部结构体
STR_OSCILLOSCOPE    STR_Osci;

//串口波特率
const  Uint32 UARTBaudrate[7] = {2400, 4800, 9600,19200, 38400,57600,115200};

//示波器通道空对象
int32  OsciNull_32Bits = 0;
/*
* const int a;int const a  这两个写法是等同的,表示a是一个int常量
*
* const int * a  表示a是一个指针,可以任意指向int常量或者int变量
*                它总是把它所指向的目标当作一个int常量
* int const* a   含义同const int * a
*
* int * const a  表示a是一个指针常量,初始化的时候必须固定指向一个int变量,
*                之后就不能再指向别的地方了
*/
int32* const pOscTargetAddr_DisplayBit[11] =
{
    &OsciNull_32Bits,                                           //0 空
    (int32*)&STR_FUNC_Gvar.DiDoOutput.DiPortState,              //1 DI
    (int32*)&STR_FUNC_Gvar.DiDoOutput.DoPortState,              //2 DO
    (int32*)&STR_FUNC_Gvar.DiDoOutput.VirtualDiPortState,       //3 虚拟DI
    (int32*)&STR_FUNC_Gvar.DiDoOutput.VirtualDoPortState,       //4 虚拟DO
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DIFunc0,               //5
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DIFunc1,               //6
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DIFunc2,               //7
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DIFunc3,               //8
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DOFunc0,               //9
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_DOFunc1,               //10
};

int32* const pOscTargetAddr[54] =
{
    (int32*)&STR_FUNC_Gvar.PosCtrl.PosRef,                      //0 位置指令
    (int32*)&STR_FUNC_Gvar.PosCtrl.PosFdb,                      //1 位置反馈
    (int32*)&STR_FUNC_Gvar.PosCtrl.PosAmplifErr,                //2 位置随动误差
    (int32*)&STR_FUNC_Gvar.OscTarget.SpdRefOld,                      //3 速度指令
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb,     //4 速度反馈
    (int32*)&STR_FUNC_Gvar.OscTarget.SpdFdbFlt,                 //5 速度反馈滤波
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef,      //6 转矩指令
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb,      //7 电流反馈
    (int32*)&STR_FUNC_Gvar.ADC_Samp.Udc_Live,                   //8 母线电压
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IuFdb,      //9 U相反馈电流
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IvFdb,      //10 V相反馈电流
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IwFdb,      //11 W相反馈电流
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb,      //12 d轴电流反馈
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_EncMultTurnData,       //13 总线编码器多圈数据
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SingleAbsPosFdb,    //14 总线编码器反馈单圈位置
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PulsePosRef,        //15 位置指令绝对值(FPGA)
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue,     //16 位置反馈绝对值(FPGA)
    (int32*)&STR_FUNC_Gvar.OscTarget.InputPulseCnt,             //17 输入指令脉冲计数器(H0B13)
    (int32*)&STR_FUNC_Gvar.OscTarget.CurrentAbsPos,             //18 当前绝对位置(H0B17)
    (int32*)&FunCodeUnion.code.AT_NotchFiltFreqDisp,            //19 共振频率辨识结果
    (int32*)&AuxFunCodeUnion.code.RatioNow,                     //20 惯量辨识结果
    (int32*)&STR_FUNC_Gvar.PosCtrl.ExInPosErrSum,               //21 混合控制脉冲偏差
    (int32*)&STR_FUNC_Gvar.OscTarget.Osc_AvrLoad,               //22 平均负载率 0.1%
    (int32*)&STR_FUNC_Gvar.OscTarget.PosAmpErr,                 //23 位置指令单位的随动偏差
    (int32*)&STR_FUNC_Gvar.OscTarget.InputPulsCntRT,            //24 实时输入指令脉冲计数器
    (int32*)&STR_FUNC_Gvar.OscTarget.InputTarPosRT,             //25 实时目标绝对位置显示  
    (int32*)&STR_FUNC_Gvar.OscTarget.InputTarSpd,               //26 实时目标速度显示   
    (int32*)&STR_FUNC_Gvar.OscTarget.InputTarToq,               //27 实时目标转矩显示 
    (int32*)&STR_FUNC_Gvar.OscTarget.CtrlWord,                  //28 控制字
    (int32*)&STR_FUNC_Gvar.OscTarget.StatusWord,                //29 状态字
    (int32*)&STR_FUNC_Gvar.OscTarget.SyncLength,                //30 SYNC0信号周期 单位12.5ns    
    (int32*)&STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr,                                           //31
    (int32*)&STR_FUNC_Gvar.OscTarget.CH1_Test,                  //32 功能模块示波器测试变量1
    (int32*)&STR_FUNC_Gvar.OscTarget.CH2_Test,                  //33 功能模块示波器测试变量2
    (int32*)&STR_FUNC_Gvar.OscTarget.CH3_Test,                  //34 功能模块示波器测试变量3
    (int32*)&STR_FUNC_Gvar.OscTarget.CH4_Test,                  //35 功能模块示波器测试变量4
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.CH1_Test,   //36 电机功能模块示波器测试变量1
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.CH2_Test,   //37 电机功能模块示波器测试变量2
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.CH3_Test,   //38 电机功能模块示波器测试变量3
    (int32*)&UNI_FUNC_MTRToFUNC_FastList_16kHz.List.CH4_Test,   //39 电机功能模块示波器测试变量4
    (int32*)&STR_PUB_Gvar.MainLoop_PRTime,                      //40 主循环程序执行时间
    (int32*)&STR_PUB_Gvar.ToqInterrupt_PRTime,                  //41 转矩环中断程序执行时间
    (int32*)&STR_PUB_Gvar.PosInterrupt_PRTime,                  //42 位置环中断程序执行时间
    (int32*)&STR_PUB_Gvar.MainLoop_PSTime,                      //43 主循环中程序的调度时间
    (int32*)&STR_PUB_Gvar.ToqInterrupt_PSTime,                  //44 转矩中断调度时间
    (int32*)&STR_PUB_Gvar.PosInterrupt_PSTime,                  //45 位置环软中断调度时间
    (int32*)&STR_PUB_Gvar.McuIqCalTime,                         //46 MCU转矩指令计算时间
    (int32*)&STR_PUB_Gvar.SYNCInterrupt_PRTime,                 //47 SYNC中断程序执行时间
    (int32*)&STR_PUB_Gvar.SYNCInterrupt_PSTime,                 //48 SYNC中断程序调度时间
    (int32*)&STR_PUB_Gvar.IRQInterrupt_PRTime,                  //49 IRQ中断程序执行时间
    (int32*)&STR_PUB_Gvar.IRQInterrupt_PSTime,                  //50 IRQ中断程序调度时间
    (int32*)&STR_PUB_Gvar.SYNC2IRQ_DeltaTime,                   //51 SYNC与IRQ相位
    &OsciNull_32Bits,                                           //52
    &OsciNull_32Bits,                                           //53
};

const int32 OscTargetCoeff[54] =
{
    1,            //0 位置指令
    1,            //1 位置反馈
    1,            //2 位置随动误差
    10000,        //3 速度指令
    10000,        //4 速度反馈
    1,            //5 速度反馈滤波
    1,            //6 转矩指令
    1,            //7 电流反馈
    1,            //8 母线电压
    1,            //9 U相反馈电流
    1,            //10 V相反馈电流
    1,            //11 W相反馈电流
    1,            //12 d轴电流反馈
    1,            //13 速度反馈0.0001rpm
    1,            //14 速度指令0.0001rpm
    1,            //15 位置指令绝对值
    1,            //16 位置反馈绝对值
    1,            //17 输入指令脉冲计数器(H0B13)
    1,            //18 反馈脉冲计数器(H0B17)
    1,            //19 共振频率辨识结果
    1,            //20 惯量辨识结果
    1,            //21
    1,            //22
    1,            //23
    1,            //24
    1,            //25
    1,            //26
    1,            //27
    1,            //28
    1,            //29
    1,            //30
    1,            //31
    1,            //32 功能模块示波器测试变量1
    1,            //33 功能模块示波器测试变量2
    1,            //34 功能模块示波器测试变量3
    1,            //35 功能模块示波器测试变量4
    1,            //36 电机功能模块示波器测试变量1
    1,            //37 电机功能模块示波器测试变量2
    1,            //38 电机功能模块示波器测试变量3
    1,            //39 电机功能模块示波器测试变量4
    1,            //40 主循环程序执行时间
    1,            //41 主循环中程序的调度时间
    1,            //42 主循环中程序的调度时间测试
    1,            //43 转矩环中断程序执行时间
    1,            //44 位置环中断程序执行时间
    1,            //45 辅助中断程序执行时间
    1,            //46 转矩中断调度时间
    1,            //47 位置环软中断调度时间
    1,            //48 辅助中断调度时间  
    1,            //49
    1,            //50
    1,            //51
    1,            //52
    1,            //53
};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
//while主循环程序调用函数,与通讯模块调度频率一致
void OscilloscopeProcess(void);

//电流环中断程序调用函数
void OscilloscopeSampling(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
//++单次采样预处理-while主循环中执行
Static_Inline void SingleModeProcess(STR_OSCILLOSCOPE * p);
//通道地址选择
Static_Inline void CH_AddrSel(STR_OSCILLOSCOPE * p);
//触发对象选择
Static_Inline void TrigTargetSel(STR_OSCILLOSCOPE * p);


//++单次采样触发-电流环中断中执行
Static_Inline void SingleModeTrig(STR_OSCILLOSCOPE * p);
//触发A
Static_Inline Uint8 TrigDeal_A(STR_OSCILLOSCOPE * p);
//触发B
Static_Inline Uint8 TrigDeal_B(STR_OSCILLOSCOPE * p);
//数据触发处理
Static_Inline Uint8 TrigDeal(Uint8 EdgeSel, int16 Level, int16 TargetLatch, int16 Target);
//位触发处理
Static_Inline Uint8 BitTrigDeal(Uint8 EdgeSel,Uint16 TargetLatch, Uint16 Target); 

//++单次采样数据采集-电流环中断中执行
Static_Inline void SingleModeSampling(STR_OSCILLOSCOPE * p);


//++连续采样预处理-while主循环中执行
Static_Inline void SeriesModeProcess(STR_OSCILLOSCOPE * p);

//++连续采样数据采集-电流环中断中执行
Static_Inline void SeriesModeSampling(STR_OSCILLOSCOPE * p);

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void OscilloscopeProcess(void)
{
    AuxFunCodeUnion.code.OS_SampleClk = STR_FUNC_Gvar.System.ToqFreq;

    //如果后台机械特性速度频谱分析功能使能时,示波器不工作
    if(1 == STR_FUNC_Gvar.MonitorFlag.bit.SpdFSAEn)
    {
        AuxFunCodeUnion.code.OS_SampleEnable = 2;
    }

    if((AuxFunCodeUnion.code.OS_Mode < 5) || (AuxFunCodeUnion.code.OS_Mode == 10))        //单次采样
    {
        SingleModeProcess(&STR_Osci);
    }
    else if((AuxFunCodeUnion.code.OS_Mode < 10) || (AuxFunCodeUnion.code.OS_Mode == 11))  //连续采样
    {
        SeriesModeProcess(&STR_Osci);
    }
}
/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void SingleModeProcess(STR_OSCILLOSCOPE * p)
{
    //复位示波器采样状态功能码
    AuxFunCodeUnion.code.OS_SampleStatus = 0;

    switch(p->OsciCtrl.bit.SampleStutas)
    {
        //当处于采样完成状态或空闲状态时,等待采样使能
        case 4:                     //采样完成
            AuxFunCodeUnion.code.OS_SampleStatus = 2;

        case 0:                     //空闲,未采样
            //扇区可读取
            AuxFunCodeUnion.code.OS_SectionCtrlA = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlB = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlC = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlD = 0;
            //停止采样
            if(AuxFunCodeUnion.code.OS_SampleEnable > 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
            }
            //开始采样,更新结构体变量
            else if(AuxFunCodeUnion.code.OS_SampleEnable == 1)
            {

                AuxFunCodeUnion.code.OS_SampleEnable = 0;
                p->OsciCtrl.bit.SampleStutas = 1; 

                //采样数据长度
                if(AuxFunCodeUnion.code.OS_Mode == 3)
                {
                    p->SampleDataLen = 4095;
                }
                else if(AuxFunCodeUnion.code.OS_Mode == 4)
                {
                    p->SampleDataLen = 2047;
                }
                else
                {
                    p->SampleDataLen = 1023;
                }

                p->SampleTime = AuxFunCodeUnion.code.OS_SampleClkPrescaler - 1;     //采样间隔时间

                //示波器控制位结构体
                p->OsciCtrl.bit.Mode = AuxFunCodeUnion.code.OS_Mode;                            //采样模式
                p->OsciCtrl.bit.TrigEdgeSel_A = AuxFunCodeUnion.code.OS_TrigEdge_A & 0x00FF;
                p->OsciCtrl.bit.TrigEdgeSel_B = AuxFunCodeUnion.code.OS_TrigEdge_B & 0x00FF;
                p->OsciCtrl.bit.TrigCtrl = AuxFunCodeUnion.code.OS_TrigSetup; 

                CH_AddrSel(p);                                //各个通道采样对象地址选择

                p->pCH1BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[0];              //通道1采样16位数据时缓冲区地址指针
                p->pCH2BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[1024];           //通道2采样16位数据时缓冲区地址指针
                p->pCH3BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[2048];           //通道3采样16位数据时缓冲区地址指针
                p->pCH4BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[3072];           //通道4采样16位数据时缓冲区地址指针
            
                p->pCH1BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[0];              //通道1采样32位数据时缓冲区地址指针
                p->pCH2BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[1024];           //通道2采样32位数据时缓冲区地址指针
                p->pCH3BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[2048];           //通道3采样32位数据时缓冲区地址指针
                p->pCH4BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[3072];           //通道4采样32位数据时缓冲区地址指针


                p->TrigLevel_A  = AuxFunCodeUnion.code.OS_TrigLevel_A;                              //触发水平A
                p->TrigLevel_B  = AuxFunCodeUnion.code.OS_TrigLevel_B;                              //触发水平B

                TrigTargetSel(p);                                                                   //触发对象选择

                //触发有效后的数据长度
                if((p->pTrigTarget_A == &OsciNull_32Bits) && (p->pTrigTarget_B == &OsciNull_32Bits))
                {
                    p->DataLenAfterTrig = p->SampleDataLen;
                }
                else
                {
                    p->DataLenAfterTrig = p->SampleDataLen - AuxFunCodeUnion.code.OS_TrigPosSetup;
                }
            }
            break;
        case 1:                     //触发前准备
        case 2:                     //触发前采样
        case 3:                     //触发后采样
            AuxFunCodeUnion.code.OS_SampleStatus = 1;

            //扇区不可读取
            AuxFunCodeUnion.code.OS_SectionCtrlA = 0xFFFF;
            AuxFunCodeUnion.code.OS_SectionCtrlB = 0xFFFF;
            AuxFunCodeUnion.code.OS_SectionCtrlC = 0xFFFF;
            AuxFunCodeUnion.code.OS_SectionCtrlD = 0xFFFF;
            //停止采样
            if(AuxFunCodeUnion.code.OS_SampleEnable > 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
                p->OsciCtrl.bit.SampleStutas = 0;
            }
            //开始采样
            else if(AuxFunCodeUnion.code.OS_SampleEnable == 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
            }
            break;
       default:                  //状态错误,复位
           p->OsciCtrl.bit.SampleStutas = 0;
           break;
    }
}
/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void CH_AddrSel(STR_OSCILLOSCOPE * p)
{
    Uint16 CH_Sel = 0; 

    //通道A
    CH_Sel = AuxFunCodeUnion.code.OS_CHSel12 >> 8;

    if(CH_Sel < 128)
    {
        p->pCH1Addr = pOscTargetAddr_DisplayBit[CH_Sel];
        p->CH1Coeff = 1;
    }
    else
    {
        CH_Sel = CH_Sel & 0x7F;
        p->pCH1Addr = pOscTargetAddr[CH_Sel];
        p->CH1Coeff = OscTargetCoeff[CH_Sel];
    }

    //通道B
    CH_Sel = AuxFunCodeUnion.code.OS_CHSel12 & 0x00FF;

    if(CH_Sel < 128)
    {
        p->pCH2Addr = pOscTargetAddr_DisplayBit[CH_Sel];
        p->CH2Coeff = 1;
    }
    else
    {
        CH_Sel = CH_Sel & 0x7F;
        p->pCH2Addr = pOscTargetAddr[CH_Sel];
        p->CH2Coeff = OscTargetCoeff[CH_Sel];
    }

    //通道C
    CH_Sel = AuxFunCodeUnion.code.OS_CHSel34 >> 8;

    if(CH_Sel < 128)
    {
        p->pCH3Addr = pOscTargetAddr_DisplayBit[CH_Sel];
        p->CH3Coeff = 1;
    }
    else
    {
        CH_Sel = CH_Sel & 0x7F;
        p->pCH3Addr = pOscTargetAddr[CH_Sel];
        p->CH3Coeff = OscTargetCoeff[CH_Sel];
    }

    //通道D
    CH_Sel = AuxFunCodeUnion.code.OS_CHSel34 & 0x00FF;

    if(CH_Sel < 128)
    {
        p->pCH4Addr =  pOscTargetAddr_DisplayBit[CH_Sel];
        p->CH4Coeff = 1;
    }
    else
    {
        CH_Sel = CH_Sel & 0x7F;
        p->pCH4Addr = pOscTargetAddr[CH_Sel];
        p->CH4Coeff = OscTargetCoeff[CH_Sel];
    }
}
/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void TrigTargetSel(STR_OSCILLOSCOPE * p)
{
    Uint16  TargetSel_Temp = 0;

    //通道A触发对象选择
    TargetSel_Temp = AuxFunCodeUnion.code.OS_TrigEdge_A >> 8;

    if(TargetSel_Temp == 0)
    {
        p->pTrigTarget_A = &OsciNull_32Bits;
        p->TrigTargetBit_A = 0xFFFF;
    }
    else if(TargetSel_Temp < 5)
    {
        p->pTrigTarget_A = *(&p->pCH1Addr + TargetSel_Temp - 1);
        p->TrigTargetCoeff_A = (int32)*(&p->CH1Coeff + TargetSel_Temp - 1);
        p->TrigTargetBit_A = 0xFFFF;
    }
    else if(TargetSel_Temp > 15)
    { 
        p->TrigTargetBit_A = 0x0001 << (TargetSel_Temp & 0x000F);
        TargetSel_Temp = TargetSel_Temp >> 4;
        p->pTrigTarget_A = pOscTargetAddr_DisplayBit[TargetSel_Temp];
    }

    //通道B触发对象选择
    TargetSel_Temp = AuxFunCodeUnion.code.OS_TrigEdge_B >> 8;

    if(TargetSel_Temp == 0)
    {
        p->pTrigTarget_B = &OsciNull_32Bits;
        p->TrigTargetBit_B = 0xFFFF;
    }
    else if(TargetSel_Temp < 5)
    {
        p->pTrigTarget_B = *(&p->pCH1Addr + TargetSel_Temp - 1);
        p->TrigTargetCoeff_B = (int32)*(&p->CH1Coeff + TargetSel_Temp - 1);
        p->TrigTargetBit_B = 0xFFFF;
    }
    else if(TargetSel_Temp > 15)
    {
        p->TrigTargetBit_B = 0x0001 << (TargetSel_Temp & 0x000F);
        TargetSel_Temp = TargetSel_Temp >> 4;
        p->pTrigTarget_B = pOscTargetAddr_DisplayBit[TargetSel_Temp];
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void SeriesModeProcess(STR_OSCILLOSCOPE * p)
{
    static Uint8 StartDelay = 0;   //启动延时
    Uint32 Temp = 0;

    switch(p->OsciCtrl.bit.SampleStutas)
    {
        case 0:                     //空闲,未采样
            AuxFunCodeUnion.code.OS_SampleStatus = 0;

            //扇区可读取
            AuxFunCodeUnion.code.OS_SectionCtrlA = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlB = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlC = 0;
            AuxFunCodeUnion.code.OS_SectionCtrlD = 0;

            //计算采样间隔时间最小值
            Temp = (Uint32)STR_FUNC_Gvar.System.ToqFreq * (Uint32)1551;
            Temp = (Uint32)Temp / (Uint32)UARTBaudrate[FunCodeUnion.code.CM_BodeRate];
            if(AuxFunCodeUnion.code.OS_Mode == 8)
            {
                Temp = Temp >> 6;
                p->SampleTimeMin = Temp + 4;
            }
            else if(AuxFunCodeUnion.code.OS_Mode == 9)
            {
                Temp = Temp >> 5;
                p->SampleTimeMin = Temp + 4;
            }
             else if(AuxFunCodeUnion.code.OS_Mode == 11)
            {
                Temp = Temp >> 3;
                p->SampleTimeMin = Temp + 4;
            }
            else
            {
                Temp = Temp >> 4;
                p->SampleTimeMin = Temp + 4;
            }

            //停止采样
            if(AuxFunCodeUnion.code.OS_SampleEnable > 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
                StartDelay = 0;
            }
            //开始采样
            else if(AuxFunCodeUnion.code.OS_SampleEnable == 1)
            {
                if(StartDelay < 100)  //开始采用前延时100ms
                {
                    StartDelay ++;
                    return;
                }  
                
                AuxFunCodeUnion.code.OS_SampleEnable = 0;

                //示波器控制位结构体
                p->OsciCtrl.bit.Mode = AuxFunCodeUnion.code.OS_Mode;
                p->OsciCtrl.bit.SampleStutas = 5;

                p->SampleCnt = 0;                                           //采样数据计数器
                //采样间隔时间
                p->SampleTime = AuxFunCodeUnion.code.OS_SampleClkPrescaler - 1;
                if(p->SampleTime < p->SampleTimeMin)
                {
                    p->SampleTime = p->SampleTimeMin;
                }

                //采样数据长度
                if(AuxFunCodeUnion.code.OS_Mode == 8)
                {
                    p->SampleDataLen = 63;
                }
                else if(AuxFunCodeUnion.code.OS_Mode == 9)
                {
                    p->SampleDataLen = 31;
                }
                else if(AuxFunCodeUnion.code.OS_Mode == 11)
                {
                    p->SampleDataLen = 7;
                }
                else
                {
                    p->SampleDataLen = 15;
                }

                CH_AddrSel(p);                                //各个通道采样对象地址选择

                p->SectionSel = 0;                                                  //从第0个扇区开始存储数据
                p->pCH1BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[0];              //通道1采样16位数据时缓冲区地址指针
                p->pCH2BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[16];           //通道2采样16位数据时缓冲区地址指针
                p->pCH3BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[32];           //通道3采样16位数据时缓冲区地址指针
                p->pCH4BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[48];           //通道4采样16位数据时缓冲区地址指针
            
                p->pCH1BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[0];              //通道1采样32位数据时缓冲区地址指针
                p->pCH2BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[8];           //通道2采样32位数据时缓冲区地址指针
                p->pCH3BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[16];              //通道3采样32位数据时缓冲区地址指针
                p->pCH4BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[24];           //通道4采样32位数据时缓冲区地址指针
            }
            else
            {
                StartDelay = 0;
            }
            break; 
        case 5:                     //连续采样中
            AuxFunCodeUnion.code.OS_SampleStatus = 3;
            StartDelay = 0;

            //停止采样
            if(AuxFunCodeUnion.code.OS_SampleEnable > 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
                p->OsciCtrl.bit.SampleStutas = 0;
            }
            //开始采样
            else if(AuxFunCodeUnion.code.OS_SampleEnable == 1)
            {
                AuxFunCodeUnion.code.OS_SampleEnable = 0;
            }
            break;
       default:                  //状态错误,复位
           p->OsciCtrl.bit.SampleStutas = 0;
           break;
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
void OscilloscopeSampling(void)
{
    static Uint16   Cnt = 0;

    STR_FUNC_Gvar.OscTarget.Osc_EncMultTurnData = (Uint32)AuxFunCodeUnion.code.DP_AbsExtData;

    //如果没有使能采样或者采样完成,计数器清零并退出
    if((STR_Osci.OsciCtrl.bit.SampleStutas == 0) || (STR_Osci.OsciCtrl.bit.SampleStutas == 4))
    {
        Cnt = 0;
        return;
    }

    //采样间隔时间判断,如果STR_Osci.SampleTime = 0,表示每次调度电流环时采样
    if(Cnt >= STR_Osci.SampleTime)
    {
        Cnt = 0;
    }
    else
    {
        Cnt ++;
        return;
    }

    if((STR_Osci.OsciCtrl.bit.Mode < 5) || (STR_Osci.OsciCtrl.bit.Mode == 10)) 
    {
        SingleModeTrig(&STR_Osci);
        SingleModeSampling(&STR_Osci);
    }
    else if((STR_Osci.OsciCtrl.bit.Mode < 10) || (STR_Osci.OsciCtrl.bit.Mode == 11))
    {
        SeriesModeSampling(&STR_Osci);
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void SingleModeTrig(STR_OSCILLOSCOPE * p)
{
    static Uint16   SampleCntBeforTrig = 0;
    static Uint16   SampleCntAfterTrig = 1;
    Uint8   TrigDealReturn = 0;

    //采样数据计数器累加
    p->SampleCnt ++;
    //如果采样数据是1024个, 那么p->SampleDataLen = 1023, p->SampleCnt范围0-1023
    if(p->SampleCnt > p->SampleDataLen)
    {
        p->SampleCnt = 0;
    }

    //触发前准备
    if(p->OsciCtrl.bit.SampleStutas == 1)
    {
        SampleCntBeforTrig ++ ;
        if(SampleCntBeforTrig < 10)
        {
            p->SampleCnt = 0;        //前9个数据不存储
        }
        else
        {
            if((SampleCntBeforTrig - 10) > (p->SampleDataLen - p->DataLenAfterTrig))
            {
                p->OsciCtrl.bit.SampleStutas = 2;
                //复位计数器
                SampleCntBeforTrig = 0;
				p->SampleCnt = 0;
            }
        }
        //锁存触发对象值
        p->TrigTargetLatch_A = * p->pTrigTarget_A;
        p->TrigTargetLatch_B = * p->pTrigTarget_B;
    }
    //触发前采样
    if(p->OsciCtrl.bit.SampleStutas == 2)
    {
        switch(p->OsciCtrl.bit.TrigCtrl)
        {
            case 0:
                TrigDealReturn = TrigDeal_A(p);
                break;
            case 1:
                TrigDealReturn = (TrigDeal_A(p)) & (TrigDeal_B(p));
                break;
            case 2:
                TrigDealReturn = (TrigDeal_A(p)) | (TrigDeal_B(p));
                break;
            default:
                TrigDealReturn = 1;  //其它值为无触发条件
                break;
        }

        if(TrigDealReturn == 1)
        {
            p->OsciCtrl.bit.SampleStutas = 3;
            //复位计数器
            SampleCntAfterTrig = 0;
            AuxFunCodeUnion.code.OS_TrigPosOfBuff = p->SampleCnt;
        }

        //锁存触发对象值
        p->TrigTargetLatch_A = * p->pTrigTarget_A;
        p->TrigTargetLatch_B = * p->pTrigTarget_B;
    }
    //触发后采样
    else if(p->OsciCtrl.bit.SampleStutas == 3)
    {
        //如果采样数据是1024个, 那么p->SampleDataLen = 1023, p->SampleCnt范围0-1023
        //触发位置是0时, 那么p->DataLenAfterTrig = 1023
        //触发位置是100时, 那么p->DataLenAfterTrig = 923
        //触发后采样数据计数器累加
        SampleCntAfterTrig ++;
        if(SampleCntAfterTrig >= p->DataLenAfterTrig) 
        {
            p->OsciCtrl.bit.SampleStutas = 4;
        }
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline Uint8 TrigDeal_A(STR_OSCILLOSCOPE * p)
{
    int32 Target = 0;
    int32 TargetLatch = 0;

    //无触发返回1
    if(p->pTrigTarget_A == &OsciNull_32Bits) return 1;

    if(p->TrigTargetBit_A == 0xFFFF)
    {
        //触发对象系数增益转换
        Target = (int32)(*p->pTrigTarget_A) / (int32)p->TrigTargetCoeff_A;

        //锁存触发对象系数增益转换
        TargetLatch = (int32)(p->TrigTargetLatch_A) / (int32)p->TrigTargetCoeff_A;

        return( TrigDeal(p->OsciCtrl.bit.TrigEdgeSel_A, (int16)p->TrigLevel_A, (int16)TargetLatch, (int16)Target) );
    }
    else
    {
        Target = (Uint16)(*p->pTrigTarget_A) & p->TrigTargetBit_A;
        TargetLatch = (Uint16)(p->TrigTargetLatch_A) & p->TrigTargetBit_A;
        return( BitTrigDeal( p->OsciCtrl.bit.TrigEdgeSel_A, (Uint16)TargetLatch, (Uint16)Target) );
    }
}
/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline Uint8 TrigDeal_B(STR_OSCILLOSCOPE * p)                           
{
    int32 Target = 0;
    int32 TargetLatch = 0;

    //无触发返回1
    if(p->pTrigTarget_B == &OsciNull_32Bits) return 1;

    if(p->TrigTargetBit_B == 0xFFFF)
    {
        //触发对象系数增益转换
        Target = (int32)(*p->pTrigTarget_B) / (int32)p->TrigTargetCoeff_B;

        //锁存触发对象系数增益转换
        TargetLatch = (int32)(p->TrigTargetLatch_B) / (int32)p->TrigTargetCoeff_B;

        return( TrigDeal(p->OsciCtrl.bit.TrigEdgeSel_B, (int16)p->TrigLevel_B, (int16)TargetLatch, (int16)Target) );
    }
    else
    {
        Target = (Uint16)(*p->pTrigTarget_B) & p->TrigTargetBit_B;
        TargetLatch = (Uint16)(p->TrigTargetLatch_B) & p->TrigTargetBit_B;
        return( BitTrigDeal( p->OsciCtrl.bit.TrigEdgeSel_B, (Uint16)TargetLatch, (Uint16)Target) );
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline Uint8 TrigDeal(Uint8 EdgeSel, int16 Level, int16 TargetLatch, int16 Target)
{
    switch(EdgeSel)
    {
        case 0:                 //上升沿
            if((Target > Level) && (TargetLatch <= Level)) return 1;
            else return 0;

        case 1:                 //下升沿
            if((Target < Level) && (TargetLatch >= Level)) return 1;
            else return 0;

        case 2:                 //沿变化
            if( ((Target > Level) && (TargetLatch <= Level)) ||
                ((Target < Level) && (TargetLatch >= Level)) ) return 1;
            else return 0;

        case 3:                 //水平之上
            if(Target > Level) return 1;
            else return 0;

        case 4:                 //水平之下
            if(Target < Level) return 1;
            else return 0;

       default:                 //无触发
            return 1;
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline Uint8 BitTrigDeal(Uint8 EdgeSel,Uint16 TargetLatch, Uint16 Target)
{
    switch(EdgeSel)
    {
        case 0:                 //上升沿
            if(Target > TargetLatch) return 1;
            else return 0;

        case 1:                 //下升沿
            if(Target < TargetLatch) return 1;
            else return 0;

        case 2:                 //沿变化
            if(Target != TargetLatch) return 1;
            else return 0;

        case 3:                 //水平之上
            if(Target > (int32)0) return 1;
            else return 0;

        case 4:                 //水平之下
            if(Target < (int32)1) return 1;
            else return 0;

       default:                 //无触发
            return 1;
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void SingleModeSampling(STR_OSCILLOSCOPE * p)
{
    int32  Temp = 0;

    switch(p->OsciCtrl.bit.Mode)
    {
        case 0:           //4个通道 16位
            //采集数据
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_16Bits + p->SampleCnt) = (int16)Temp;

            Temp = (int32)(*p->pCH2Addr) + Sign_NP(*p->pCH2Addr) * ((int32)p->CH2Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH2Coeff;
            *(p->pCH2BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;
            break;

        case 1:           //1通道 32位   3、4通道 16位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;
            break;

        case 2:           //2个通道 32位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH2BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;
            break;

        case 3:           //1个通道 16位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_16Bits + p->SampleCnt) = (int16)Temp;
            break;
        case 4:          //1个通道 32位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;
            break;
        case 10:           //4个通道 32位
            //采集数据
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH2Addr) + Sign_NP(*p->pCH2Addr) * ((int32)p->CH2Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH2Coeff;
            *(p->pCH2BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;
            break;
    }
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/
Static_Inline void SeriesModeSampling(STR_OSCILLOSCOPE * p)
{
    int32  Temp = 0;

    switch(p->OsciCtrl.bit.Mode)
    {
        case 5:           //4个通道 16位
            //采集数据
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_16Bits + p->SampleCnt) = (int16)Temp;

            Temp = (int32)(*p->pCH2Addr) + Sign_NP(*p->pCH2Addr) * ((int32)p->CH2Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH2Coeff;
            *(p->pCH2BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;
            break;

        case 6:           //1通道 32位   3、4通道 16位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_16Bits + p->SampleCnt) = (int16)Temp ;
            break;

        case 7:           //2个通道 32位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;
            break;

        case 8:           //1个通道 16位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_16Bits + p->SampleCnt) = (int16)Temp;
            break;
        case 9:          //1个通道 32位
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;
            break;
        case 11:           //4个通道 32位
            //采集数据
            Temp = (int32)(*p->pCH1Addr) + Sign_NP(*p->pCH1Addr) * ((int32)p->CH1Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH1Coeff;
            *(p->pCH1BuffAddr_32Bits + p->SampleCnt) = (int32)Temp;

            Temp = (int32)(*p->pCH2Addr) + Sign_NP(*p->pCH2Addr) * ((int32)p->CH2Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH2Coeff;
            *(p->pCH2BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;

            Temp = (int32)(*p->pCH3Addr) + Sign_NP(*p->pCH3Addr) * ((int32)p->CH3Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH3Coeff;
            *(p->pCH3BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;

            Temp = (int32)(*p->pCH4Addr) + Sign_NP(*p->pCH4Addr) * ((int32)p->CH4Coeff >> 1);
            Temp = (int32)Temp / (int32)p->CH4Coeff;
            *(p->pCH4BuffAddr_32Bits + p->SampleCnt) = (int32)Temp ;
            break;
    }

    //采样数据计数器累加
    //如果采样数据是1024个, 那么p->SampleDataLen = 1024, p->SampleCnt范围0-1023
    if(p->SampleCnt < p->SampleDataLen)
    {
        p->SampleCnt ++;
    }
    else
    {
        p->SampleCnt = 0;

        //如果串口空闲,更新H3202,COMM模块向上位机发送数据
        if((AuxFunCodeUnion.code.OS_UartStatus == 0)&& (AuxFunCodeUnion.code.OS_SeriesTxCtrl == 0))
        {
            AuxFunCodeUnion.code.OS_SeriesTxCtrl = 0x9000 + p->SectionSel;
        }

        //更新扇区号
        if(p->SectionSel >= 63)
        {
            p->SectionSel = 0;
        }
        else
        {
            p->SectionSel ++;
        }
        //更新存储地址
        p->pCH1BuffAddr_16Bits = &UNI_OsciBuffer.all_16Bits[p->SectionSel << 6];        //通道1采样16位数据时缓冲区地址指针
        p->pCH2BuffAddr_16Bits = p->pCH1BuffAddr_16Bits + 16;                            //通道2采样16位数据时缓冲区地址指针
        p->pCH3BuffAddr_16Bits = p->pCH1BuffAddr_16Bits + 32;                           //通道3采样16位数据时缓冲区地址指针
        p->pCH4BuffAddr_16Bits = p->pCH1BuffAddr_16Bits + 48;                           //通道4采样16位数据时缓冲区地址指针

        p->pCH1BuffAddr_32Bits = &UNI_OsciBuffer.all_32Bits[p->SectionSel << 5];        //通道1采样32位数据时缓冲区地址指针
        p->pCH2BuffAddr_32Bits = p->pCH1BuffAddr_32Bits + 8;                            //通道2采样32位数据时缓冲区地址指针
        p->pCH3BuffAddr_32Bits = p->pCH1BuffAddr_32Bits + 16;                            //通道3采样32位数据时缓冲区地址指针
        p->pCH4BuffAddr_32Bits = p->pCH1BuffAddr_32Bits + 24;                            //通道4采样32位数据时缓冲区地址指针
    }
}

/********************************* END OF FILE *********************************/
