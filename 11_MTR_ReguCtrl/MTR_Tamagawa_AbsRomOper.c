/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Tamagawa_AbsRomOper.c  
 创建人：   王治国                  创建日期：2014.11.19
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
#include "MTR_Tamagawa_AbsRomOper.h"
#include "MTR_AbsEncAngleInit.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"
#include "MTR_AbsRomOper.h"

#if TAMAGAWA_ENC_SW

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* 编码器工作模式
0-No action（编码器被禁止），1-连续读数据（同步于电流环周期），2-随机读数据，
3-随机读EEPROM，4-随机写EEPROM，5-复位绝对位置（即位置清零），
6-复位报警信号，7-复位多圈数据及报警信号
 */
#define TAMAG_ABSENC_FORBID             0       //绝对编码器禁止
#define TAMAG_ABSENC_SQNCRD             1       //连续读数据  
//#define TAMAG_ABSENC_RANDRD             2       //随机读数据
#define TAMAG_ABSENC_RANDRDROM          3       //随机读EEPROM
#define TAMAG_ABSENC_RANDWRTROM         4       //随机写EEPROM
#define TAMAG_ABSENC_RSTABSPOS          5       //复位绝对位置   
#define TAMAG_ABSENC_RSTALARM           6       //复位报警信号 
#define TAMAG_ABSENC_RSTMULTPOSALARM    7       //复位多圈数据及报警信号
       

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
UNI_TAMAG_ABSENCSTATE_REG     TAMAGAbsEncState_Reg;       //绝对式编码器运行状态

UNI_TAMAGABSERR_REG           TAMAGAbsErr_Reg = {0};      //编码器故障状态

UNI_TAMAG_ABSENCTRL_REG       TAMAGAbsEncCtrl_Reg = {0};   //绝对式编码器控制

UNI_TAMAG_MOTROMSTATE_REG     TAMAGMotRomState_Reg;         //EEPROM访问状态

UNI_ABSROMDATA                TAMAGAbsRomData;         //绝对式编码器EEPROM数据结构体

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//控制参数，即需初始化的参数
extern volatile Uint16 *AbsEncCtrl;     //绝对式编码器设置
extern volatile Uint16 *AbsCmdDelay;     //串行编码器命令延迟

//状态参数表，即DSP从FPGA中读取的参数
extern volatile Uint16 *AbsEncErr;      //绝对式编码器故障状态
extern volatile Uint16 *AbsAngLow;      //编码器绝对位置低16位
extern volatile Uint16 *AbsEncState;    //绝对式编码器运行状态
extern volatile Uint16 *MotRomCtrl;     //绝对式编码器EEPROM访问控制
extern volatile Uint16 *MotRomState;    //绝对式编码器EEPROM访问状态
extern volatile Uint16 *TAMAGAbsTurn;        //绝对式编码器多圈数据
extern volatile Uint16 *HostSysCtrl;    //相电流平衡校正和绝对式初始复位操作
extern volatile Uint16 *HostAlarmClr;   //报警清除
extern volatile Uint16 *HostPosRst;             //位置偏差计数器清零
extern volatile Uint16 *ABSEncSel;              //绝对编码器选择

extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;
extern volatile UNI_FPGA_ABSENC_SEL_REG   UNI_FPGA_AbsEncSel; //旋转编码器控制寄存器

extern Uint16 AbsRom_CheckWord;          //校验字
extern Uint16 AbsRom_Null;               //空变量

static Uint8 TAMAGPrmRdFlag;            //参数读取标志位，1～未读取成功

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void TAMAGAbsRom_InitDeal(void);
void TAMAGAbsRom_EncState(void);
void CalcTAMAGEncTransTime(void);
void TAMAGAbsRomProcess(void);
Uint8 TAMAGAbsEnc_SaveThetaOffset(void);
Uint8 TAMAGAbsRom_ClcErr(void);
Uint8 TAMAGAbsRom_ClcErrAndMultiTurn(Uint8 Mode);  

Uint16 * const TAMAGAbsRomDataAddr[TAMAGABS_ROM_WORDS_LEN] =
{
    (Uint16 *)&AbsRom_CheckWord,                          //校验字 应当为0xAA55
    (Uint16 *)&FunCodeUnion.code.MT_AbsEncType,           //H00_08 绝对编码器类型 0-单圈 1-多圈
    (Uint16 *)&FunCodeUnion.code.MT_RateVolt,             //H00_09 额定电压
    (Uint16 *)&FunCodeUnion.code.MT_RatePower,            //H00_10 额定功率
    (Uint16 *)&FunCodeUnion.code.MT_RateCurrent,          //H00_11 额定电流
    (Uint16 *)&FunCodeUnion.code.MT_RateToq,              //H00_12 额定转矩
    (Uint16 *)&FunCodeUnion.code.MT_MaxToqOrCur,          //H00_13 最大转矩或最大电流
    (Uint16 *)&FunCodeUnion.code.MT_RateSpd,              //H00_14 额定转速
    (Uint16 *)&FunCodeUnion.code.MT_MaxSpd,               //H00_15 最大转速
    (Uint16 *)&FunCodeUnion.code.MT_Inertia,              //H00_16 转动惯量
    (Uint16 *)&FunCodeUnion.code.MT_PolePair,             //H00_17 永磁同步电机极对数
    (Uint16 *)&FunCodeUnion.code.MT_StatResist,           //H00_18 定子电阻
    (Uint16 *)&FunCodeUnion.code.MT_StatInductQ,          //H00_19 定子电感Lq
    (Uint16 *)&FunCodeUnion.code.MT_StatInductD,          //H00_20 定子电感Ld
    (Uint16 *)&FunCodeUnion.code.MT_RevEleCoe,            //H00_21 反电势系数
    (Uint16 *)&FunCodeUnion.code.MT_ToqCoe,               //H00_22 转矩系数Kt
    (Uint16 *)&FunCodeUnion.code.MT_EleConst,             //H00_23 电气常数Te
    (Uint16 *)&FunCodeUnion.code.MT_MachConst,            //H00_24 机械常数Tm
    (Uint16 *)&FunCodeUnion.code.MT_ThetaOffsetL,         //H00_28 绝对式码盘位置偏置L 
    (Uint16 *)&FunCodeUnion.code.MT_ThetaOffsetH,         //H00_29 绝对式码盘位置偏置H 
    (Uint16 *)&FunCodeUnion.code.MT_AbsRomMotorModel,     //H00_35 总线电机型号
    (Uint16 *)&FunCodeUnion.code.OEM_KedGain,             //H01_22 D轴反电动势补偿系数
    (Uint16 *)&FunCodeUnion.code.OEM_KeqGain,             //H01_23 Q轴反电动势补偿系数
    (Uint16 *)&FunCodeUnion.code.OEM_CurIdKpSec,          //H01_24 D轴电流环比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CurIdKiSec,          //H01_25 D轴电流环积分补偿因子
    (Uint16 *)&FunCodeUnion.code.OEM_CurIqKpSec,          //H01_27 Q轴电流环比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CurIqKiSec,          //H01_28 Q轴电流环积分补偿因子                       
    (Uint16 *)&FunCodeUnion.code.OEM_CapIdKp,             //H01_52 性能优先模式D轴比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CapIdKi,             //H01_53 性能优先模式D轴积分补偿因子
    (Uint16 *)&FunCodeUnion.code.OEM_CapIqKp,             //H01_54 性能优先模式Q轴比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CapIqKi,             //H01_55 性能优先模式Q轴积分补偿因子
};

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void InitTAMAGAbsEncCtrl(Uint8 AbsMode);
Static_Inline void TAMAGAbsEncCommInit(void);
Static_Inline Uint8 TAMAGAbsRom_GetComStatus(void);
Static_Inline Uint8 TAMAGAbsRom_ReadByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 TAMAGAbsRom_ReadByte(Uint8 ReadAddr);
Static_Inline Uint8 TAMAGAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr);
Static_Inline Uint8 TAMAGAbsRom_WriteByte(Uint8 WriteAddr);

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void TAMAGAbsRom_InitDeal(void)
{
    Uint8 AbsRomAddr = 0;
    Uint8 Temp = 0;

    FunCodeUnion.code.MT_EncoderSel = 0x10;         //H00_30 编码器类型
    FunCodeUnion.code.MT_EncoderPensL = 0;          //H00_31 编码器线数L
    FunCodeUnion.code.MT_EncoderPensH = 0x10;       //H00_32 编码器线数H

    //配置*ABSEncSel寄存器
    UNI_FPGA_AbsEncSel.bit.NikType = 0;
    UNI_FPGA_AbsEncSel.bit.AbsType = 3;
    UNI_FPGA_AbsEncSel.bit.BaudSelect = 0;
    *ABSEncSel = UNI_FPGA_AbsEncSel.all; 

    DELAY_US(100000L);    //延时100ms以等待绝对式编码器完成上电待机

    if(1 == FunCodeUnion.code.DisRdEncEepromPwrOn) return;

    AbsRomAddr = 0;
    while(AbsRomAddr < TAMAGABS_ROM_BYTES_LEN)
    {
        Temp = TAMAGAbsRom_ReadByte(AbsRomAddr);
        if(1 == Temp) AbsRomAddr++;
        else if(2 == Temp) AbsRomAddr = TAMAGABS_ROM_BYTES_LEN + 1;

        DELAY_US(62);                    //虚拟中断周期调用的情况

        TAMAGAbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态 
        if(0 != (TAMAGAbsErr_Reg.all & 0x0F))     //通讯错误（RX端）
        {
            //编码器断线 或 通讯故障
            PostErrMsg(CHABSROMERR1); 
            AbsRomAddr = TAMAGABS_ROM_BYTES_LEN + 1;
            TAMAGPrmRdFlag = 1;        //参数读取不成功

            FunCodeUnion.code.MT_ABSEncVer = 65535;            //H00_04 编码器软件版本号
            FunCodeSaveInAbsRomInit(); 
            break;   
        }
    }

    if(AbsRomAddr != TAMAGABS_ROM_BYTES_LEN) 
    {
        InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);
        return;
    } 
   
    if(ABSROM_CHECK_WORD != TAMAGAbsRomData.all_16Bits[0])
    {
        TAMAGPrmRdFlag = 1;              //参数校验不成功
        PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据

        //获取校验字和版本号
        AbsRom_CheckWord = TAMAGAbsRomData.all_16Bits[0];      
        FunCodeUnion.code.MT_ABSEncVer = TAMAGAbsRomData.all_16Bits[2];              
        FunCodeSaveInAbsRomInit();
    }
    else
    {
        AbsRomAddr = 0;
        while(AbsRomAddr < TAMAGABS_ROM_WORDS_LEN)
        {
            *TAMAGAbsRomDataAddr[AbsRomAddr] = TAMAGAbsRomData.all_16Bits[AbsRomAddr];
            AbsRomAddr ++;
        } 

        while(0 == TAMAGAbsRom_ReadByte(78)) DELAY_US(62); 

        if(TAMAGAbsRomData.all_8Bits[78] < 90)
        {
            
            TAMAGAbsRomData.all_8Bits[78] = 95;
    
            while(0 == TAMAGAbsRom_WriteByte(78)) DELAY_US(62); 
            
        }
    }

    InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);
}


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void TAMAGAbsRom_EncState(void)
{
    Uint8 AbsRomAddr = 0;
    Uint16 AbsPosTempH = 0;  
    int64 AbsPosTemp = 0; 
    int64 Temp = 0;
            
    TAMAGAbsEncState_Reg.all = *AbsEncState;     //绝对式编码器运行状态 

	if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 1)
	{
        //串行编码器反馈单圈位置 
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = ((Uint32)TAMAGAbsEncState_Reg.bit.AbsAngHigh << 16) + (*AbsAngLow);
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb =  STR_MTR_Gvar.AbsRom.SingleAbsPosFdb & 0x000FFFFF;

        AbsPosTempH = *TAMAGAbsTurn; 
        if(AbsPosTempH >= FunCodeUnion.code.PL_EncMultiTurnOffset)
        {
            AbsPosTempH = AbsPosTempH - FunCodeUnion.code.PL_EncMultiTurnOffset;
        }
        else
        {
            AbsPosTempH = (Uint32)AbsPosTempH + 65536L - (Uint32)FunCodeUnion.code.PL_EncMultiTurnOffset;
        }

        //串行编码器反馈多圈+单圈位置 
        AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 20) & 0xFFFFFFFFFFF00000; 
        AbsPosTemp = (Uint64)((Uint64)AbsPosTemp | (Uint64)STR_MTR_Gvar.AbsRom.SingleAbsPosFdb); 

        if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时
        {
            Temp = (Uint64)STR_MTR_Gvar.FPGA.EncRev << 16;

            AbsPosTemp = Temp - 1 - AbsPosTemp; 
            
            STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = STR_MTR_Gvar.FPGA.EncRev - 1 - STR_MTR_Gvar.AbsRom.SingleAbsPosFdb; 
            
            AbsPosTempH = 65535 - AbsPosTempH;         
        }
         
        AuxFunCodeUnion.code.DP_AbsExtData = AbsPosTempH;
         
        STR_MTR_Gvar.AbsRom.MultiAbsPosFdbL = (Uint32)AbsPosTemp;
        STR_MTR_Gvar.AbsRom.MultiAbsPosFdbH = (Uint32)((Uint64)AbsPosTemp >> 32);
    }
    else
    {
        TAMAGAbsEncCommInit(); 
    }    
    
    if(TAMAGPrmRdFlag == 1)         //电机参数读取不成功时，置所有值为0xFFFF；
    {
        AbsRomAddr = 3;

        while(AbsRomAddr < TAMAGABS_ROM_WORDS_LEN)          
        {             
             *TAMAGAbsRomDataAddr[AbsRomAddr] = 0xFFFF;
             AbsRomAddr ++;
        }

        TAMAGPrmRdFlag = 0;      //清除标志位
    }     
}

/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline void TAMAGAbsEncCommInit(void)
{
    static int8 delaycnt = 0;
    Uint32 AbsPosTempL = 0;
    int16 AbsPosTempH = 0;
    int64 AbsPosTemp = 0; 
    int64 Temp = 0;

    if(0 == delaycnt)
    {
        if(0 == TAMAGAbsRom_GetComStatus()) 
        {
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_SQNCRD);  //设置为连续读
            *HostPosRst = 1;
            delaycnt = 1; 
        }
    }
    else
    {       
        if(delaycnt > 10)
        {
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;

            delaycnt = 0;
            *HostPosRst = 0; 

            if(0 == STR_MTR_Gvar.AbsRom.AbsPosDetection) return;

            //多圈绝对位置使能 得到有效数据
            AbsPosTempL = (Uint32)((Uint32)TAMAGAbsEncState_Reg.bit.AbsAngHigh << 16) + (Uint32)(*AbsAngLow);
            AbsPosTempL =  AbsPosTempL & 0x000FFFFF;
            AbsPosTempH = *TAMAGAbsTurn;
            if(AbsPosTempH >= FunCodeUnion.code.PL_EncMultiTurnOffset)
            {
                AbsPosTempH = AbsPosTempH - FunCodeUnion.code.PL_EncMultiTurnOffset;
            }
            else
            {
                AbsPosTempH = (Uint32)AbsPosTempH + 65536L - (Uint32)FunCodeUnion.code.PL_EncMultiTurnOffset;
            }

            AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 20) & 0xFFFFFFFFFFF00000;
            AbsPosTemp = (int64)((Uint64)AbsPosTemp | (Uint64)AbsPosTempL);

            if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时
            {
                Temp = (Uint64)STR_MTR_Gvar.FPGA.EncRev << 16;
    
                AbsPosTemp = Temp - 1 - AbsPosTemp;                             
            }
                             
            STR_MTR_Gvar.AbsRom.MultiAbsPosFdbL = (Uint32)AbsPosTemp;
            STR_MTR_Gvar.AbsRom.MultiAbsPosFdbH = (Uint32)((int64)AbsPosTemp >> 32); 
            STR_MTR_Gvar.AbsRom.FpgaPosFdbInit =  STR_MTR_Gvar.FPGA.PosFdbAbsValue;
        }
        else
        {
            delaycnt ++;
        }
    }
}

/*******************************************************************************
  函数名:   
  输入:   无 
  输出:   0～空闲,1～通讯忙 
  子函数: 无
  描述：
    1. 得到绝对式编码器当前的通讯状态，0～空闲,1～通讯忙
    2.
********************************************************************************/
Static_Inline Uint8 TAMAGAbsRom_GetComStatus(void)
{
    Uint16 DelayCnt = 0;      //延迟时间计算

    TAMAGAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态;

    //需要等到通信不忙的时候才进行写操作
    while((TAMAGAbsEncState_Reg.bit.AbsComStatus == 1) && (DelayCnt < 10))
    {
        DelayCnt++;
        DELAY_US(1);
        TAMAGAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态
    }

    if(DelayCnt < 10)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}



/*******************************************************************************
  函数名:  
  输入:   
  输出:   
  子函数: 
  描述：
    1. 
    2.
********************************************************************************/
void CalcTAMAGEncTransTime(void)
{
    int32 Ts = 0;          //电流环控制周期转换成的FPGA周期
    int32 Tmcu = 0;        //MCU转矩指令处理时间转换成的FPGA周期
    int32 Tspd = 0;        //速度计算时间转换成的FPGA周期
    int32 Tabsenc = 0;     //串行编码器总通讯时间转换成的FPGA周期   
    int32 AbsTransferDelay = 0;
    int32 Temp = 0;
    int32 CmdTransTime = 0;

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

    //时钟2.5M 共计12个数据 说明书上计算时间3us  共计51us
    Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 53000;
    
    Tabsenc   = Tabsenc / FPGA_PRD;

    STR_MTR_Gvar.AbsRom.AbsTransferTime = Tabsenc;  

    //编码器通讯波特率换算  2.5M 对应400ns
    STR_MTR_Gvar.AbsRom.AbsCommBaud = 400 / FPGA_PRD;

    //串行编码器的通讯启动延时参数
    AbsTransferDelay = (Ts - Tspd - Tmcu  + Ts - Tabsenc) >> 1;

    if(AbsTransferDelay > (Ts >> 1))
    {
        AbsTransferDelay -= (Ts >> 1);
    }

    TAMAGAbsEncCtrl_Reg.bit.AbsTransferDelay = AbsTransferDelay;

    *AbsEncCtrl = TAMAGAbsEncCtrl_Reg.all;

    /*说明：AbsCmdDelay = (Ts + Δ - tenc - tspd) / Tclk。其中Ts为同步周期（FPGA中断周期），
    tenc为编码器的总通讯时间，Δ为编码器命令字的传输时间（必要的话还要考虑线延迟），
    tspd为速度计算时间，Tclk为系统主时钟周期。参见620P的相关说明。
    Δ为编码器命令字的传输时间:
    尼康16位字长，（16+2）/波特率
    其它8位字长，（8+2）/波特率
    线延迟通过功能码H0166 设定 默认0 */    
       
    //编码器通讯频率 2.5M 对应400ns
    CmdTransTime = (10L * 400L) / FPGA_PRD; 
    Temp = (FunCodeUnion.code.OEM_EncCmdTransLineDelay * 10L) / FPGA_PRD;
    Temp += Ts + CmdTransTime - Tspd  - Tabsenc;       
    *AbsCmdDelay = Temp;    
}

/*******************************************************************************
  函数名:  
  输入:   编码器工作模式设置 
  输出:   无
  子函数: 无
  描述：
    1. 绝对式编码器控制初始化
    2.
********************************************************************************/
Static_Inline void InitTAMAGAbsEncCtrl(Uint8 AbsMode)
{
    TAMAGAbsEncCtrl_Reg.bit.AbsMode = AbsMode;   //编码器工作模式设置

    *AbsEncCtrl = TAMAGAbsEncCtrl_Reg.all;
}


/*******************************************************************************
  函数名:   
  输入:   读地址
  输出:   1～读取命令发送成功   0～通信忙,未读取,继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 TAMAGAbsRom_ReadByteCmd(Uint8 ReadAddr)
{
    UNI_TAMAG_MOTROMCTRL_REG TAMAGMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	static Uint16 MonitStep = 0;

	if(MonitStep == 0)
	{
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            TAMAGMotRomCtrl_Reg.bit.EepromAddr   = ReadAddr;
            TAMAGMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = TAMAGMotRomCtrl_Reg.all;

			MonitStep = 1;						 //下一次检测
		}
	}
	else
	{
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            TAMAGMotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态

            if(TAMAGMotRomState_Reg.bit.EepromBusy == 0)
            {
			    MonitStep = 0;
				return 1;
			}
		}
	}

    return 0;
}

/*******************************************************************************
  函数名:   
  输入:   读地址
  输出:   1～读完, 0～还未完成, 2错误
  子函数: 无
  描述：
    1、EEPROM读操作
    ① 查询编码器状态寄存器0x22，若AbsComStatus为0，则执行下一步，否则等待该位变为0。
    ② 将控制参数寄存器0x14中的AbsMode位域值设为3，即"随机读EEPROM"模式。
    ③ 在运行命令寄存器0x3B中，把需要访问的EEPROM地址设置到EepromAddr位域内（此时ByteToEeprom可为任意值），
    同时将EepromMode位置1。执行该寄存器则会启动对编码器内部EEPROM的读访问。
    ④ 不断查询编码器状态寄存器0x22，当AbsComStatus位变为0时，若AbsComErr亦为0，则执行下一步，
    否则本次访问失败、读出的数据无效。
    ⑤ 查询EEPROM状态寄存器0x3B，若EepromBusy为1，则本次访问失败、读出的数据无效，
    否则ByteFromEeprom即为所读出的值。
    ⑥ 将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式。
********************************************************************************/
Static_Inline Uint8 TAMAGAbsRom_ReadByte(Uint8 ReadAddr)
{
    static Uint16 TimeDelay = 0;
    static Uint8  ReadByteStep = 0;   //等待模式设置

    if(ReadByteStep == 0)
    {
        //完成第1、2步
        InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽避免FPGA一直处于忙碌状态

        if(0 == TAMAGAbsRom_GetComStatus())
        {
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_RANDRDROM);      //设置绝对式编码器模式为随机读EEPROM

            ReadByteStep = 1;
        }
    }
    else if(ReadByteStep == 1)
    {
        //完成第3、4、5步
        if(1 == TAMAGAbsRom_ReadByteCmd(ReadAddr))
        {
            ReadByteStep = 2;     //进入校验阶段
        }
    }
    else if(ReadByteStep == 2)
    {
        //完成第5、6步
        if(1 == TAMAGAbsRom_ReadByteCmd(ReadAddr))
        {
            if(ReadAddr == TAMAGMotRomState_Reg.bit.EepromAddr)
            {
                TAMAGAbsRomData.all_8Bits[ReadAddr] = TAMAGMotRomState_Reg.bit.ByteFromEeprom;
                ReadByteStep = 0;      //进入等待阶段
                TimeDelay = 0;
                return 1;           //无论读是否对错都结束读ROM状态
            }
            else
            {
                PostErrMsg(ENCDPMERR);     //参数校验错误
                ReadByteStep = 0;      //进入等待阶段
                TimeDelay = 0;
                return 2;           //无论读是否对错都结束读ROM状态
            }
        }
    }

    TimeDelay++;
    if((TimeDelay > 4000) || (0 != (TAMAGAbsErr_Reg.all & 0x0F)))          //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);      //延时过长警告  Er.740
        ReadByteStep = 0;   //等待模式设置
        TimeDelay = 0;
        return 2;                    //不再进入读
    }

    return 0;
}

/*******************************************************************************
  函数名: 
  输入:   读地址
  输出:   1～写入命令发送成功， 0～通信忙，无法写入，继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 TAMAGAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr)
{
	static Uint16 MonitStep = 0;
    UNI_TAMAG_MOTROMCTRL_REG TAMAGMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	if(MonitStep == 0)		        //第一次进入时检测
	{
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            TAMAGMotRomCtrl_Reg.bit.ByteToEeprom = WriteByte;
            TAMAGMotRomCtrl_Reg.bit.EepromAddr   = WriteAddr;
            TAMAGMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = TAMAGMotRomCtrl_Reg.all;                //启动Rom操作
			MonitStep = 1;     						  //Rom操作完后进入下个周期判定
		}
	}							   //第二步操作
	else
	{
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            TAMAGMotRomState_Reg.all  = *MotRomState;               //EEPROM访问状态

            if(TAMAGMotRomState_Reg.bit.EepromBusy == 0)
            {
				MonitStep = 0;
				return 1;
			} 
		}
	}

    return 0;
}

/*******************************************************************************
  函数名:   
  输入:   写数据
  输出:   1～写完，0～还未完成
  子函数: 无
  描述：
    ① 查询编码器状态寄存器0x22，若AbsComStatus为0，则执行下一步，否则等待该位变为0。
    ② 将控制参数寄存器0x14中的AbsMode位域值设为4，即"随机写EEPROM"模式。
       注意该寄存器的AbsTransferDelay位域必须始终保持某个预设值，不能随意更改（下同）。
    ③ 在运行命令寄存器0x3B中，把需要写入的数据字节和EEPROM地址分别设置到ByteToEeprom
       和EepromAddr两个位域内，同时将EepromMode位置1。执行该寄存器则会启动对编码器内部EEPROM的写访问。
    ④ 不断查询编码器状态寄存器0x22，当AbsComStatus位变为0时，若AbsComErr亦为0，
       则执行下一步，否则本次访问失败、写入的数据无效（注意在访问失败的情况下，也要执行第⑥步，下同）。
    ⑤ 查询EEPROM状态寄存器0x3B，若EepromBusy为1，则本次访问失败、写入的数据无效。
    ⑥ 将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式。
    注：由于EEPROM内部的写周期需要18ms左右，因此，MCU相邻的两次EEPROM写操作之间的间隔应大于18ms，
        否则会造成访问失败。另外，上述步骤不能保证数据被正确地写入EEPROM，如需确认，
        必须执行EEPROM读操作的过程，若读出的数据和写入的一致，则说明数据已被正确写入。

********************************************************************************/
Static_Inline Uint8 TAMAGAbsRom_WriteByte(Uint8 WriteAddr)
{
    static Uint8 WriteByteStep = 0;      //等待模式设置
    static Uint16 TimeDelay = 0;                    //延时时间计数器
    static Uint16 WaitDelay = 0;                    //等待时间计数器

    if(WriteByteStep == 0)
    {
        //完成第1、2步
        InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽再设置模式

        if(0 == TAMAGAbsRom_GetComStatus())
        {
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_RANDWRTROM);      //设置绝对式编码器模式为随机写EEPROM

            WriteByteStep = 1;
        }
    }
    else if(WriteByteStep == 1)
    {
        //完成第3、4、5步
        if(1 == TAMAGAbsRom_WriteByteCmd(TAMAGAbsRomData.all_8Bits[WriteAddr], WriteAddr))      //判定写命令是否发送完
        {
            WriteByteStep   = 2;                 //写命令完成后进入到等待阶段
            WaitDelay = 0;
        }
    }
    else if(WriteByteStep == 2)
    {
        //延时25ms
        if(WaitDelay < 800)					     //需要把延迟时间加长
        {
            WaitDelay++;
        }
        else        //累积了500个周期，31ms后进入读校验阶段
        {
            WaitDelay = 0;
            TimeDelay    = 0;
            WriteByteStep = 3;      //进入等待阶段//STEPREADBYTE;
        }
    }
    else if(WriteByteStep == 3)
    {
        if(1 == TAMAGAbsRom_ReadByte(WriteAddr))
        {
            if((WriteAddr != TAMAGMotRomState_Reg.bit.EepromAddr) ||
               (TAMAGAbsRomData.all_8Bits[WriteAddr] != TAMAGMotRomState_Reg.bit.ByteFromEeprom))
            {
                PostErrMsg(ENCDPMERR);              //参数校验错误
                WriteByteStep = 0;        //进入等待阶段
                TimeDelay    = 0;
                return 2;           //无论读是否对错都结束读ROM状态
            }

            WriteByteStep = 0;      //进入等待阶段
            TimeDelay    = 0;
            return 1;           //无论读是否对错都结束读ROM状态        
        }
    }    

    TimeDelay++;
    if((TimeDelay > 6000) || (0 != (TAMAGAbsErr_Reg.all & 0x0F)))          //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);       //延时过长警告 Er.740
        WriteByteStep = 0;      //进入等待阶段
        TimeDelay = 0;
        return 2;                       //结束写
    }

    return 0;
}

/*******************************************************************************
  函数名: 
  输入:    
  输出:   
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void TAMAGAbsRomProcess(void)
{
    static Uint16 State = 0;
    static Uint8 AbsRomAddr = 0;

    Uint8 Temp = 0;

    switch(State)
    {
        case 0:  //空闲
            if(AuxFunCodeUnion.code.MT_OperAbsROM == 2)
            {
                //开始读AbsRom
                State = 1;
                AbsRomAddr = 0;
            }
            else if(AuxFunCodeUnion.code.MT_OperAbsROM == 1)
            {  
                //得到要保存到电机ROM中的参数
                AbsRom_CheckWord = ABSROM_CHECK_WORD;
                AbsRomAddr = 0;
                while(AbsRomAddr < TAMAGABS_ROM_WORDS_LEN)
                {
                     TAMAGAbsRomData.all_16Bits[AbsRomAddr] = *TAMAGAbsRomDataAddr[AbsRomAddr];
                     AbsRomAddr ++;
                }

                //开始写AbsRom
                State = 3;
                AbsRomAddr = 0;
            }
            else
            {
                State = 0;
                AuxFunCodeUnion.code.MT_OperAbsROM = 0;
            } 
            break;

        case 1: //读AbsRom中
            Temp = TAMAGAbsRom_ReadByte(AbsRomAddr);

            if(1 == Temp) 
            {
                AbsRomAddr++;
            }
            else if((2 == Temp) || (0 != (TAMAGAbsErr_Reg.all & 0x0F)))   //发生校验故障或通信故障
            {
                State = 5;    //出错
            }

            if(AbsRomAddr >= TAMAGABS_ROM_BYTES_LEN) State = 2;
            break;

        case 2: //读AbsRom完成 
                if(ABSROM_CHECK_WORD != TAMAGAbsRomData.all_16Bits[0])
                {
                    PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据
                }
                else
                {
                    AbsRomAddr = 0;
                    while(AbsRomAddr < TAMAGABS_ROM_WORDS_LEN)
                    {
                         *TAMAGAbsRomDataAddr[AbsRomAddr] = TAMAGAbsRomData.all_16Bits[AbsRomAddr];
                         AbsRomAddr ++;
                    }
                }

                State = 4;
                AbsRomAddr = 0;
            break;

        case 3: //写AbsRom中
            Temp = TAMAGAbsRom_WriteByte(AbsRomAddr);

            if(1 == Temp)
            {
                AbsRomAddr++;
            }
            else if((2 == Temp) || (0 != (TAMAGAbsErr_Reg.all & 0x0F)))
            {  
                State = 5;    //出错
            }

            if(AbsRomAddr >= TAMAGABS_ROM_BYTES_LEN) State = 4;    //完成
            break;

        case 4: //写，读AbsRom完成
        case 5:
            if(0 == TAMAGAbsRom_GetComStatus())
            {
                if(State == 4)        //读写成功
                {
                    AuxFunCodeUnion.code.MT_OperAbsROM = 0;                         
                }
                else                  //读写失败提示
                {
                    AuxFunCodeUnion.code.MT_OperAbsROM = 3;                        
                }

                InitTAMAGAbsEncCtrl(TAMAG_ABSENC_SQNCRD);  //设置为随机读

                State = 0;
                AbsRomAddr = 0;                 
            }
            break;
        default:
            break;
    }
}

/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
Uint8 TAMAGAbsEnc_SaveThetaOffset(void)
{
    static Uint8 Step = 0; 
    Uint8 Temp = 0;

    Temp = 0;
    //将位置信息保存到相应功能码
    switch(Step)
    {
        case 0:
            TAMAGAbsRomData.all_16Bits[18] = FunCodeUnion.code.MT_ThetaOffsetL;      //H00_28 绝对式码盘位置偏置L 
            TAMAGAbsRomData.all_16Bits[19] = FunCodeUnion.code.MT_ThetaOffsetH;      //H00_29 绝对式码盘位置偏置H 
            Step = 1; 
            break;    
    
         case 1:
            Temp = TAMAGAbsRom_WriteByte(35); 
            break;
               
         case 2:
            Temp = TAMAGAbsRom_WriteByte(36);
            break;
                
         case 3:
            Temp = TAMAGAbsRom_WriteByte(37);
            break;
               
         case 4:
            Temp = TAMAGAbsRom_WriteByte(38); 
            break;

        default:
	    	Step = 0;
            break;    
    } 
    
    if(1 == Temp) 
    {
        if(4 == Step)
        {
	    	InitTAMAGAbsEncCtrl(TAMAG_ABSENC_SQNCRD);
	    	Step = 0;
            return 1; 
        }
        else
        {
            Step++;
        }         
    }          	
    else if(2 == Temp)
    {
        InitTAMAGAbsEncCtrl(TAMAG_ABSENC_SQNCRD);   
        Step = 0;          //回到第一步                           
        return 2;
    }

    return 0;
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Uint8 TAMAGAbsRom_ClcErrAndMultiTurn(Uint8 Mode)
{
	static Uint8 Step = 0;
	static Uint8 Cnt = 0;

    UNI_TAMAG_MOTROMCTRL_REG TAMAGMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义  

	if(Step == 0)		        //第一次进入时检测
	{
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);
            Step = 1;
            Cnt = 0; 
		}
	}
    else if(Step == 1)
    {
        //连续10次
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            if(Mode == 1)InitTAMAGAbsEncCtrl(TAMAG_ABSENC_RSTALARM);
            else if(Mode == 2)InitTAMAGAbsEncCtrl(TAMAG_ABSENC_RSTMULTPOSALARM);
            else InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);

            TAMAGMotRomCtrl_Reg.bit.ByteToEeprom = 0;
            TAMAGMotRomCtrl_Reg.bit.EepromAddr   = 0;
            TAMAGMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = TAMAGMotRomCtrl_Reg.all;  
            Cnt ++;
            if(Cnt >= 10) 
            {
                Step = 2;
                Cnt = 0;
            }
        }    
    } 
    else if(Step == 2)
    {
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_FORBID);

            Cnt ++;
            if(Cnt >= 16) 
            {
                Step = 3;
                Cnt = 0;
            }                						  
        }
    }
    else if(Step == 3)
    {
        if(0 == TAMAGAbsRom_GetComStatus())
        {
            FunCodeUnion.code.PL_EncMultiTurnOffset = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));
            InitTAMAGAbsEncCtrl(TAMAG_ABSENC_SQNCRD);
            Step = 0;
            return 0;   						  
        }
    }

    return Mode;
} 

#endif

/********************************* END OF FILE *********************************/
