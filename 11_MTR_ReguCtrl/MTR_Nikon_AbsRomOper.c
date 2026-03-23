/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_Nikon_AbsRomOper.h  
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
#include "MTR_Nikon_AbsRomOper.h"
#include "MTR_AbsEncAngleInit.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"
#include "MTR_AbsRomOper.h"

#if NOKIN_ENC_SW

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* 编码器工作模式
0-No action（编码器被禁止），1-连续读数据（同步于电流环周期），2-随机读报警状态，
3-随机读EEPROM，4-随机写EEPROM，5-复位单圈位置（即位置清零），
6-复位报警信号，7-复位多圈数据。 */
#define NIKON_ABSENC_FORBID           0       //绝对编码器禁止
#define NIKON_ABSENC_SQNCRD           1       //连续读数据（同步于电流环周期）
#define NIKON_ABSENC_RANDRDWARN       2       //随机读报警状态 
#define NIKON_ABSENC_RANDRDROM        3       //随机读EEPROM
#define NIKON_ABSENC_RANDWRTROM       4       //随机写EEPROM
#define NIKON_ABSENC_RSTSINGLEPOS     5       //复位单圈位置   
#define NIKON_ABSENC_RSTALARM         6       //复位编码器中报警信号 
#define NIKON_ABSENC_RSTMULTPOS       7       //复位多圈数据
       

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
UNI_NK_ABSENCSTATE_REG     NKAbsEncState_Reg;       //绝对式编码器运行状态

UNI_NKABSERR_REG           NKAbsErr_Reg = {0};      //编码器故障状态

UNI_NK_ABSENCTRL_REG       NKAbsEncCtrl_Reg = {0};

UNI_NK_MOTROMSTATE_REG     NKMotRomState_Reg;         //EEPROM访问状态


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//控制参数，即需初始化的参数
extern volatile Uint16 *AbsEncCtrl;     //绝对式编码器设置
extern volatile Uint16 *AbsCmdDelay;     //串行编码器命令延迟

//状态参数表，即DSP从FPGA中读取的参数
extern volatile Uint16 *AbsEncErr;      //绝对式编码器故障状态
extern volatile Uint16 *AbsAngLow;      //编码器绝对位置低16位
extern volatile Uint16 *AbsEncState;    //绝对式编码器运行状态
extern volatile Uint16 *NKAbsTurn;      //NIKON编码器多圈数据
extern volatile Uint16 *MotRomCtrl;     //绝对式编码器EEPROM访问控制
extern volatile Uint16 *MotRomState;    //绝对式编码器EEPROM访问状态

extern volatile Uint16 *HostSysCtrl;    //相电流平衡校正和绝对式初始复位操作
extern volatile Uint16 *HostAlarmClr;   //报警清除
extern volatile Uint16 *HostPosRst;             //位置偏差计数器清零

extern volatile Uint16 *ABSEncSel;      //绝对编码器选择      

extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;
extern volatile UNI_FPGA_ABSENC_SEL_REG   UNI_FPGA_AbsEncSel; //旋转编码器控制寄存器

extern UNI_ABSROMDATA          AbsRomData;         //绝对式编码器EEPROM数据结构体

extern Uint16 AbsRom_CheckWord;          //校验字
extern Uint16 AbsRom_Null;               //空变量


extern Uint16 AbsRom_H0030; 
extern Uint16 AbsRom_H0031; 
extern Uint16 AbsRom_H0032;   
extern Uint16 * const AbsRomDataAddr[50];


static Uint8  NKPrmRdFlag;            //参数读取标志位，1～未读取成功
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void NKAbsRom_InitDeal(void);
void NKAbsRom_EncState(void);
Uint8 NKAbsRom_RdErrStatus(void);
Uint8 NKAbsRom_ClcErrAndMultiTurn(Uint8 Mode);
void CalcNikonEncTransTime(void);
void NKAbsRomProcess(void);
Uint8 NKAbsEnc_SaveThetaOffset(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void InitNKAbsEncCtrl(Uint8 AbsMode);
Static_Inline void NKAbsEncCommInit(void);
Static_Inline Uint8 NKAbsRom_GetComStatus(void);
Static_Inline Uint8 NKAbsRom_ReadByteCmd(Uint8 ReadAddr,Uint8 ReadBusy);
Static_Inline Uint8 NKAbsRom_CheckByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 NKAbsRom_ReadByte(Uint8 ReadAddr);
Static_Inline Uint8 NKAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr);
Static_Inline Uint8 NKAbsRom_WriteByte(Uint8 WriteAddr);


/*******************************************************************************
  函数名:  void NikonAbsRom_InitDeal(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
void NKAbsRom_InitDeal(void)
{
    Uint8 AbsRomAddr = 0;
    Uint8 Temp = 0;
     
    FunCodeUnion.code.MT_EncoderSel = 0x12;         //H00_30 编码器类型
    FunCodeUnion.code.MT_EncoderPensL = 0;          //H00_31 编码器线数L
    FunCodeUnion.code.MT_EncoderPensH = 0x10;       //H00_32 编码器线数H 

    /*
    旋转编码器控制（电机轴端）
    参数位域	参数名称	参数符号	取值范围	参数属性	
    [1:0]	未定义	---	任意值	只写
    [2]	    尼康编码器分辨率	NikType	0~1	
    [4:3]	串行编码器类型	AbsType	0~3	
    [13:5]	未定义	---	任意值	
    [15:14]	串行编码器通讯波特率选择	BaudSelect	0~3	

    说明：BaudSelect：0-2MHz（默认兼容第一代编码器），1-2.5MHz，2-4MHz，3-未定义；
    AbsType：0-编码器接口复位，1-汇川编码器(20/23bit)，2-尼康编码器，3-多摩川编码器(17bit)；
    NikType：0-20bit，1-17bit（当BaudSelect=2时，可选择4M波特率，其它值默认2.5M）
    */

    //配置*ABSEncSel寄存器
    switch(FunCodeUnion.code.MT_MotorModel) 
    {
        case 14020:
        case 14120:
            UNI_FPGA_AbsEncSel.bit.NikType = 0;
            UNI_FPGA_AbsEncSel.bit.BaudSelect = 1;
            break;
        case 14021:
        case 14121:
            UNI_FPGA_AbsEncSel.bit.NikType = 1;
            UNI_FPGA_AbsEncSel.bit.BaudSelect = 1;
            break;
        case 14022:
        case 14122:
            UNI_FPGA_AbsEncSel.bit.NikType = 1;
            UNI_FPGA_AbsEncSel.bit.BaudSelect = 2;
            break;

        default:    
            break;
    }        
    UNI_FPGA_AbsEncSel.bit.AbsType = 2;
    *ABSEncSel = UNI_FPGA_AbsEncSel.all;     

    DELAY_US(500000L);    //延时500ms以等待绝对式编码器完成上电待机

    if(1 == FunCodeUnion.code.DisRdEncEepromPwrOn) return;

    AbsRomAddr = 0;
    while(AbsRomAddr < ABS_ROM_BYTES_LEN)
    {
        Temp = NKAbsRom_ReadByte(AbsRomAddr);
        if(1 == Temp) AbsRomAddr++;
        else if(2 == Temp) AbsRomAddr = ABS_ROM_BYTES_LEN + 1;

        DELAY_US(62);                    //虚拟中断周期调用的情况

        NKAbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态 
        if(0 != (NKAbsErr_Reg.all & 0x0F))     //通讯错误（RX端）
        {
            PostErrMsg(CHABSROMERR1); 
            AbsRomAddr = ABS_ROM_BYTES_LEN + 1;
            NKPrmRdFlag = 1;        //参数读取不成功

            FunCodeUnion.code.MT_ABSEncVer = 65535;            //H00_04 编码器软件版本号             
            FunCodeSaveInAbsRomInit(); 
            break;   
        }
    }

    InitNKAbsEncCtrl(NIKON_ABSENC_FORBID); 

    if(AbsRomAddr != ABS_ROM_BYTES_LEN) return;

   
    if(ABSROM_CHECK_WORD != AbsRomData.all_16Bits[0])
    {
        NKPrmRdFlag = 1;              //参数校验不成功
        PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据

        //获取校验字和版本号
        AbsRom_CheckWord = AbsRomData.all_16Bits[0];      
        FunCodeUnion.code.MT_ABSEncVer = AbsRomData.all_16Bits[2];   
        FunCodeSaveInAbsRomInit();
    }
    else
    {
        AbsRomAddr = 0;
        while(AbsRomAddr < ABS_ROM_WORDS_LEN)
        {
            *AbsRomDataAddr[AbsRomAddr] = AbsRomData.all_16Bits[AbsRomAddr];
            AbsRomAddr ++;
        }
    }
}


/*******************************************************************************
  函数名:  void NKAbsRom_EncState(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    2.
********************************************************************************/
void NKAbsRom_EncState(void)
{
    Uint8 AbsRomAddr = 0;
    Uint16 AbsPosTempH = 0;  
    int64 AbsPosTemp = 0; 
    int64 Temp = 0;
            
    NKAbsEncState_Reg.all = *AbsEncState;     //绝对式编码器运行状态

	if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 1)
	{
        //串行编码器反馈单圈位置 
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = ((Uint32)NKAbsEncState_Reg.bit.AbsAngHigh << 16) + (*AbsAngLow);
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb =  STR_MTR_Gvar.AbsRom.SingleAbsPosFdb & 0x000FFFFF;

        AbsPosTempH = *NKAbsTurn;
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
        NKAbsEncCommInit();         
    } 

    if(NKPrmRdFlag == 1)         //电机参数读取不成功时，置所有值为0xFFFF；
    {
        AbsRomAddr = 3;

        while(AbsRomAddr < ABS_ROM_WORDS_LEN)          
        {             
             *AbsRomDataAddr[AbsRomAddr] = 0xFFFF;
             AbsRomAddr ++;
        }

        NKPrmRdFlag = 0;      //清除标志位
    }    
      
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
Static_Inline void NKAbsEncCommInit(void)
{
    static int8 delaycnt = 0;
    static int8 ClcErrFlag = 0;
    Uint32 AbsPosTempL = 0;
    int16 AbsPosTempH = 0;
    int64 AbsPosTemp = 0;
    int64 Temp = 0;

    if((1 == FunCodeUnion.code.NKAbsEncRstPwrOn) && (0 == ClcErrFlag))
    {   
        //复位故障
        if(0 == NKAbsRom_ClcErrAndMultiTurn(1)) ClcErrFlag = 1;
        return;
    }

    //多圈绝对位置使能
    if(0 == delaycnt)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);
              
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
            
            //得到有效数据
            AbsPosTempL = (Uint32)((Uint32)NKAbsEncState_Reg.bit.AbsAngHigh << 16) + (Uint32)(*AbsAngLow);
            AbsPosTempL =  AbsPosTempL & 0x000FFFFF;
            AbsPosTempH = *NKAbsTurn;
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
  函数名:  Static_Inline Uint8 NikonAbsRom_GetComStatus(void)
  输入:   无 
  输出:   0～空闲,1～通讯忙 
  子函数: 无
  描述：
    1. 得到绝对式编码器当前的通讯状态，0～空闲,1～通讯忙
    2.
********************************************************************************/
Static_Inline Uint8 NKAbsRom_GetComStatus(void)
{
    Uint16 DelayCnt = 0;      //延迟时间计算

    NKAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态;

    //需要等到通信不忙的时候才进行写操作
    while((NKAbsEncState_Reg.bit.AbsComStatus == 1) && (DelayCnt < 10))
    {
        DelayCnt++;
        DELAY_US(1);
        NKAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态
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
void CalcNikonEncTransTime(void)
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

    //时钟2.5M 共计5个16bit数据 说明书上计算时间3us  共计39us
    Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 42000;
    
    Tabsenc   = Tabsenc / FPGA_PRD;

    STR_MTR_Gvar.AbsRom.AbsTransferTime = Tabsenc;  

    if(2 == UNI_FPGA_AbsEncSel.bit.BaudSelect)
    {
        //编码器通讯波特率换算  4M 对应250ns  
        STR_MTR_Gvar.AbsRom.AbsCommBaud = 250 / FPGA_PRD; 
    }
    else
    {
        //编码器通讯波特率换算  2.5M 对应400ns  
        STR_MTR_Gvar.AbsRom.AbsCommBaud = 400 / FPGA_PRD;     
    } 

    //串行编码器的通讯启动延时参数
    AbsTransferDelay = (Ts - Tspd - Tmcu  + Ts - Tabsenc) >> 1;

    if(AbsTransferDelay > (Ts >> 1))
    {
        AbsTransferDelay -= (Ts >> 1);
    }

    NKAbsEncCtrl_Reg.bit.AbsTransferDelay = AbsTransferDelay;

    *AbsEncCtrl = NKAbsEncCtrl_Reg.all;

    /*说明：AbsCmdDelay = (Ts + Δ - tenc - tspd) / Tclk。其中Ts为同步周期（FPGA中断周期），
    tenc为编码器的总通讯时间，Δ为编码器命令字的传输时间（必要的话还要考虑线延迟），
    tspd为速度计算时间，Tclk为系统主时钟周期。参见620P的相关说明。
    Δ为编码器命令字的传输时间:
    尼康16位字长，（16+2）/波特率
    其它8位字长，（8+2）/波特率
    线延迟通过功能码H0166 设定 默认0 */
    if(2 == UNI_FPGA_AbsEncSel.bit.BaudSelect)
    {
        //编码器通讯频率 4M 对应250ns
        CmdTransTime = (18L * 250L) / FPGA_PRD;  
    }
    else
    {
        //编码器通讯频率 2.5M 对应400ns
        CmdTransTime = (18L * 400L) / FPGA_PRD;        
    }
    Temp = (FunCodeUnion.code.OEM_EncCmdTransLineDelay * 10L) / FPGA_PRD;
    Temp += Ts + CmdTransTime - Tspd  - Tabsenc;        
    *AbsCmdDelay = Temp;
    
}

/*******************************************************************************
  函数名:  Static_Inline void InitNKAbsEncCtrl(Uint8 AbsMode)
  输入:   编码器工作模式设置 
  输出:   无
  子函数: 无
  描述：
    1. 绝对式编码器控制初始化
    2.
********************************************************************************/
Static_Inline void InitNKAbsEncCtrl(Uint8 AbsMode)
{
    NKAbsEncCtrl_Reg.bit.AbsMode = AbsMode;   //编码器工作模式设置

    *AbsEncCtrl = NKAbsEncCtrl_Reg.all;
}

/*******************************************************************************
  函数名: 
  输入:  
  输出:   无
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Uint8 NKAbsRom_RdErrStatus(void)
{
	static Uint8 Step = 0;
    UNI_NK_MOTROMCTRL_REG NKMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	if(Step == 0)		        //第一次进入时检测
	{
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);
            Step = 1;
		}
	}
    else if(Step == 1)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_RANDRDWARN);
            Step = 2;
        }    
    } 
    else if(Step == 2)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            NKMotRomCtrl_Reg.bit.ByteToEeprom = 0;
            NKMotRomCtrl_Reg.bit.EepromAddr   = 0;
            NKMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = NKMotRomCtrl_Reg.all;               
			Step = 3;     						  
        }
    }
    else if(Step == 3)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);
            Step = 0;
            
            PostErrMsg(PCHGDWARN);        //Er.941
            return 0;   						  
        }
    }

    return 1;
}

/*******************************************************************************
  函数名: 
  输入:  
  输出:   无
  子函数: 无
  描述：
    取消要求在个別传输模式、轴停止状态（250min―1 以下）下进行。
    请向编码器连续8次发送命令"CDF8～10"。
    注）连续8次发送期间，如果有別的命令输入，或因干扰等而使该命令被判断为异常，则不进行取消处理。请再次连续8次发送命令。
    根据下列命令，可取消如下状态或多旋转数据。
    CDF8　：　状态标识（超速、MT错误、存储访问异常）
    CDF9　：　多旋转数据
    CDF10：　状态标识＋多旋转数据
    从接收第8次取消要求命令，至实际完成取消，最大需要400μs的时间。在此期间，编码器数据为取消前的数据，因此，请在确实确认完成取消之后，再发出新的命令要求。

********************************************************************************/
Uint8 NKAbsRom_ClcErrAndMultiTurn(Uint8 Mode)
{
	static Uint8 Step = 0;
	static Uint8 Cnt = 0;

    UNI_NK_MOTROMCTRL_REG NKMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义  

	if(Step == 0)		        //第一次进入时检测
	{
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);
            Step = 1;
            Cnt = 0;
		}
	}
    else if(Step == 1)
    {
        //连续8次
        if(0 == NKAbsRom_GetComStatus())
        {
             InitNKAbsEncCtrl(NIKON_ABSENC_RSTALARM);

            NKMotRomCtrl_Reg.bit.ByteToEeprom = 0;
            NKMotRomCtrl_Reg.bit.EepromAddr   = 0;
            NKMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = NKMotRomCtrl_Reg.all;  
            Cnt ++;
            if(Cnt >= 8) 
            {
                Step = 2;
                Cnt = 0;
            }
        }    
    } 
    else if(Step == 2)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);

            Cnt ++;
            if(Cnt >= 16) 
            {
                if(Mode == 2)Step = 3;
                else Step = 5; 
                Cnt = 0;
            }                						  
        }
    }
    else if(Step == 3)
    {
        //连续8次
        if(0 == NKAbsRom_GetComStatus())
        {
             InitNKAbsEncCtrl(NIKON_ABSENC_RSTMULTPOS);

            NKMotRomCtrl_Reg.bit.ByteToEeprom = 0;
            NKMotRomCtrl_Reg.bit.EepromAddr   = 0;
            NKMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = NKMotRomCtrl_Reg.all;  
            Cnt ++;
            if(Cnt >= 8) 
            {
                Step = 4;
                Cnt = 0;
            }
        }    
    } 
    else if(Step == 4)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);

            Cnt ++;
            if(Cnt >= 16) 
            {
                Step = 5;
                Cnt = 0;
            }                						  
        }
    }
    else if(Step == 5)
    {
        if(0 == NKAbsRom_GetComStatus())
        {
            FunCodeUnion.code.PL_EncMultiTurnOffset = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));
            InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);
            Step = 0;
            return 0;   						  
        }
    }

    return Mode;
}

/*******************************************************************************
  函数名:  Static_Inline Uint8 NKAbsRom_CheckByteCmd(Uint8 ReadAddr)
  输入:   读地址
  输出:   1～完成校验，将数据读出，0～未完成校验
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 NKAbsRom_CheckByteCmd(Uint8 ReadAddr)
{
    if(0 == NKAbsRom_GetComStatus())
    {
        NKMotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态

        if(NKMotRomState_Reg.bit.EepromBusy == 0)
        {
            return 1;
        }
    }

    return 0;    
}


/*******************************************************************************
  函数名:  Static_Inline Uint8 NKNKAbsRom_ReadByteCmd(Uint8 ReadAddr,Uint8 ReadBusy)
  输入:   读地址
  输出:   1～读取命令发送成功   0～通信忙,未读取,继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 NKAbsRom_ReadByteCmd(Uint8 ReadAddr,Uint8 ReadBusy)
{
    UNI_NK_MOTROMCTRL_REG NKMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	static Uint16 MonitStep = 0;

	if(MonitStep == 0)
	{
        if(0 == NKAbsRom_GetComStatus())
        {
            NKMotRomCtrl_Reg.bit.EepromAddr   = ReadAddr;
            NKMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = NKMotRomCtrl_Reg.all;

			MonitStep = 1;						 //下一次检测
		}
	}
	else
	{
        if(0 == NKAbsRom_GetComStatus())
        {         
            if(0 == ReadBusy) 
            {
                MonitStep = 0;
                return 1;
            }
            else
            {                
                NKMotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态
    
                if(NKMotRomState_Reg.bit.EepromBusy == 0)
                {
    			    MonitStep = 0;
    				return 1;
    			}
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
关于EEPROM操作，尼康可以提供0~127的寻址范围。在具体操作方法上，与多摩川相比略有不同。
主要体现在读访问上，详细步骤如下：
①   首先设RotEncEnbl = 0。查询编码器状态寄存器0x22，若AbsComStatus为0，
     则执行下一步，否则等待该位变为0。
②   将控制参数寄存器0x14中的AbsMode位域值设为3，即"随机读EEPROM"模式。
③   在运行命令寄存器0x3B中，把需要访问的EEPROM地址设置到EepromAddr位域内（此时ByteToEeprom可为任意值），
     同时将EepromMode位置1。执行该寄存器则会启动对编码器内部EEPROM的读访问。
④   不断查询编码器状态寄存器0x22，当AbsComStatus位变为0时，若AbsComErr亦为0，
     则执行下一步，否则本次访问失败、读出的数据无效。
⑤   延时300us后，再执行一次③~④步（注意不要改变EEPROM地址）。然后继续执行下一步。
⑥   查询EEPROM状态寄存器0x3B，若EepromBusy为1，则本次访问失败、读出的数据无效，否则ByteFromEeprom即为所读出的值。
⑦   将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式。
********************************************************************************/
Static_Inline Uint8 NKAbsRom_ReadByte(Uint8 ReadAddr)
{
    static Uint16 TimeDelay = 0;
    static Uint8 ReadByteStep = 0;      //等待模式设置
    static Uint16 DlyCnt = 0;  
     
    if(ReadByteStep == 0)
    {
        //完成第1、2步
        InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽避免FPGA一直处于忙碌状态

        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_RANDRDROM);      //设置绝对式编码器模式为随机读EEPROM

            ReadByteStep = 1;
        }
    }
    else if(ReadByteStep == 1)
    {
        //完成第3、4步
        if(1 == NKAbsRom_ReadByteCmd(ReadAddr,0))
        {
            ReadByteStep = 2;     
            DlyCnt = 0;
        }
    }
    else if(ReadByteStep == 2)
    {
       DlyCnt++;
       if(DlyCnt >= 10)
       {
            ReadByteStep = 3;    
            DlyCnt = 0;       
       }
    }
    else if(ReadByteStep == 3)
    {
        //完成第3、4、5步
        if(1 == NKAbsRom_ReadByteCmd(ReadAddr,1))
        {
            ReadByteStep = 4;    
            DlyCnt = 0;
        }    
    }
    else if(ReadByteStep == 4)
    {
        //完成第5、6步
        if(1 == NKAbsRom_CheckByteCmd(ReadAddr))
        {
            if(ReadAddr == NKMotRomState_Reg.bit.EepromAddr)
            {
                AbsRomData.all_8Bits[ReadAddr] = NKMotRomState_Reg.bit.ByteFromEeprom;
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
    if((TimeDelay > 4000) || (0 != (NKAbsErr_Reg.all & 0x0F)))           //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);      //延时过长警告  Er.740
        ReadByteStep = 0;            //等待模式设置
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
Static_Inline Uint8 NKAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr)
{
	static Uint16 MonitStep = 0;
	static Uint16 Dly30ms = 0;

    UNI_NK_MOTROMCTRL_REG NKMotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

    if(1 == NKAbsRom_GetComStatus())  return 0;

	if(MonitStep == 0)		        //第一次进入时检测
	{
        NKMotRomCtrl_Reg.bit.ByteToEeprom = WriteByte;
        NKMotRomCtrl_Reg.bit.EepromAddr   = WriteAddr;
        NKMotRomCtrl_Reg.bit.EepromMode = 1;

        *MotRomCtrl = NKMotRomCtrl_Reg.all;                //启动Rom操作
		MonitStep = 1;     						  //Rom操作完后进入下个周期判定
        Dly30ms = 0;  		
	}
 	else if(MonitStep == 1)							   //第二步操作  延时30ms
    {
        Dly30ms++;
        if(Dly30ms > 1000)
        {
            MonitStep = 2;
        }
    }
	else if(MonitStep == 2)
	{      
        InitNKAbsEncCtrl(NIKON_ABSENC_RANDRDWARN);     
        MonitStep = 3;  	
	}
	else if(MonitStep == 3)
	{
        NKMotRomCtrl_Reg.bit.ByteToEeprom = WriteByte;
        NKMotRomCtrl_Reg.bit.EepromAddr   = WriteAddr;
        NKMotRomCtrl_Reg.bit.EepromMode = 1;

        *MotRomCtrl = NKMotRomCtrl_Reg.all;         //启动Rom操作
		MonitStep = 4;     						    //Rom操作完后进入下个周期判定
    }
	else if(MonitStep == 4)
	{
        NKMotRomCtrl_Reg.all  = *MotRomState;               //EEPROM访问状态

        if(NKMotRomState_Reg.bit.EepromBusy == 0)
        {
			MonitStep = 0;
			return 1;
		}
        else
        {
            MonitStep = 2;
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
    ⑥ 将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式
    。
    注：由于EEPROM内部的写周期需要 30ms 左右，因此，MCU相邻的两次EEPROM写操作之间的间隔应大于30ms，
        否则会造成访问失败。另外，上述步骤不能保证数据被正确地写入EEPROM，如需确认，
        必须执行EEPROM读操作的过程，若读出的数据和写入的一致，则说明数据已被正确写入。

注意：EEPROM的写访问，操作过程与多摩川的类似，但要注意一点：由于EEPROM内部的写周期至少需要30ms，
      因此，MCU相邻的两次EEPROM写操作之间的间隔应大于30ms，否则会造成访问失败。
********************************************************************************/
Static_Inline Uint8 NKAbsRom_WriteByte(Uint8 WriteAddr)
{
    static Uint8 WriteByteStep = 0;      //等待模式设置
    static Uint16 TimeDelay = 0;                    //延时时间计数器
    Uint8 Temp = 0;

    if(WriteByteStep == 0)
    {
        //完成第1、2步
        InitNKAbsEncCtrl(NIKON_ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽再设置模式

        if(0 == NKAbsRom_GetComStatus())
        {
            InitNKAbsEncCtrl(NIKON_ABSENC_RANDWRTROM);      //设置绝对式编码器模式为随机写EEPROM
            WriteByteStep = 1;
        }
    }
    else if(WriteByteStep == 1)
    {
        //完成第3、4、5步
        Temp = NKAbsRom_WriteByteCmd(AbsRomData.all_8Bits[WriteAddr], WriteAddr);     

        if(1 == Temp)     
        {
            WriteByteStep = 0;                 //写命令完成后进入到等待阶段        
            TimeDelay = 0;
            return 1;               //结束写ROM状态 
        }
    }

    TimeDelay++;
    if((TimeDelay > 6000) || (0 != (NKAbsErr_Reg.all & 0x0F)))          //延迟时间过长
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
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
void NKAbsRomProcess(void)
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
                AbsRom_H0030 = 0x0012; 
                AbsRom_H0031 = 0x0000; 
                AbsRom_H0032 = 0x0010;
                AbsRom_Null = 0;
                FunCodeUnion.code.MT_ABSEncVer = 0;

                //得到要保存到电机ROM中的参数
                AbsRom_CheckWord = ABSROM_CHECK_WORD;
                AbsRomAddr = 0;
                while(AbsRomAddr < ABS_ROM_WORDS_LEN)
                {
                     AbsRomData.all_16Bits[AbsRomAddr] = *AbsRomDataAddr[AbsRomAddr];
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
            Temp = NKAbsRom_ReadByte(AbsRomAddr);

            if(1 == Temp) 
            {
                AbsRomAddr++;
            }
            else if(2 == Temp)   //发生校验故障或通信故障
            {
                State = 5;    //出错
            }

            if(AbsRomAddr >= ABS_ROM_BYTES_LEN) State = 2;
            break;

        case 2: //读AbsRom完成 
                if(ABSROM_CHECK_WORD != AbsRomData.all_16Bits[0])
                {
                    PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据
                }
                else
                {
                    AbsRomAddr = 0;
                    while(AbsRomAddr < ABS_ROM_WORDS_LEN)
                    {
                         *AbsRomDataAddr[AbsRomAddr] = AbsRomData.all_16Bits[AbsRomAddr];
                         AbsRomAddr ++;
                    }
                }

                State = 4;
                AbsRomAddr = 0;
            break;

        case 3: //写AbsRom中 
            Temp = NKAbsRom_WriteByte(AbsRomAddr);

            if(1 == Temp)
            {
                AbsRomAddr++;
            }
            else if((2 == Temp) || (0 != (NKAbsErr_Reg.all & 0x0F)))
            {  
                State = 5;    //出错
            }

            if(AbsRomAddr >= ABS_ROM_BYTES_LEN) State = 4;    //完成
            break;

        case 4: //写，读AbsRom完成
        case 5:
            if(0 == NKAbsRom_GetComStatus())
            {
                if(State == 4)        //读写成功
                {
                    AuxFunCodeUnion.code.MT_OperAbsROM = 0;                         
                }
                else                  //读写失败提示
                {
                    AuxFunCodeUnion.code.MT_OperAbsROM = 3;                        
                }

                InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);  //设置为随机读

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
Uint8 NKAbsEnc_SaveThetaOffset(void)
{
    static Uint8 Step = 0; 
    Uint8 Temp = 0;

    Temp = 0;
    //将位置信息保存到相应功能码
    switch(Step)
    {
        case 0:
            AbsRomData.all_16Bits[26] = FunCodeUnion.code.MT_ThetaOffsetL;      //H00_28 绝对式码盘位置偏置L 
            AbsRomData.all_16Bits[27] = FunCodeUnion.code.MT_ThetaOffsetH;      //H00_29 绝对式码盘位置偏置H 
            Step = 1; 
            break;    
    
         case 1:
            Temp = NKAbsRom_WriteByte(52); 
            break;
               
         case 2:
            Temp = NKAbsRom_WriteByte(53);
            break;
                
         case 3:
            Temp = NKAbsRom_WriteByte(54);
            break;
               
         case 4:
            Temp = NKAbsRom_WriteByte(55); 
            break;

        default:
	    	Step = 0;
            break;    
    } 
    
    if(1 == Temp) 
    {
        if(4 == Step)
        {
	    	InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);
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
        InitNKAbsEncCtrl(NIKON_ABSENC_SQNCRD);   
        Step = 0;          //回到第一步                           
        return 2;
    }

    return 0;
}

#endif 

/********************************* END OF FILE *********************************/
