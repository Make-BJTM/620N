/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    MTR_HDH_RomOper.c  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   王治国                 修改日期：2014.08.18

 描述： 
    1. 
    2. 
 修改记录：  
    2014.08.18
    1.基于HC编码器文件修改，支持海德汉编码器
    
********************************************************************************/ 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_Main.h"
#include "MTR_HDH_RomOper.h"
#include "MTR_AbsRomOper.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */ 

//编码器工作模式
#define HDHABSENC_FORBID           0       //编码器禁止
#define HDHABSENC_SQNCRD           1       //连续读数据(标准模式)
#define HDHABSENC_SETMRSCODE       2       //设定MrsCode
#define HDHABSENC_RANDRDROM        3       //随机读EEPROM
#define HDHABSENC_RANDWRTROM       4       //随机写EEPROM
#define HDHABSENC_RSTABSPOS        6       //复位编码器


//存入电机ROM中前2个是校验字,如果读出来的参数与FLASH不同,需要报错并重新写入电机参数
#define ABSROM_CHECK_WORD   0xAA55

#define ABSROM_BYTE_LEN     100
#define ABSROM_WORD_LEN     50

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

UNI_HDHMOTROMCTRL_REG       HDHMotRomCtrl_Reg = {0};        //绝对式编码器EEPROM访问控制
UNI_HDHMOTROMCTRL2_REG      HDHMotRomCtrl2_Reg = {0};       //绝对式编码器EEPROM访问控制2

UNI_HDHABSENCSTATE_REG      HDHAbsEncState_Reg;             //绝对式编码器运行状态

UNI_HDHABSERR_REG           HDHAbsErr_Reg = {0};            //编码器故障状态

UNI_HDHMOTROMSTATE_REG      HDHMotRomState_Reg;             //EEPROM访问状态
UNI_HDHMOTROMSTATE2_REG     HDHMotRomState2_Reg;            //EEPROM访问状态2

UNI_HDHABSENCTRL_REG        HDHAbsEncCtrl_Reg = {0};

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */    

//控制参数，即需初始化的参数
extern volatile Uint16 *AbsEncCtrl;         //绝对式编码器设置
extern volatile Uint16 *AbsEncCtrl2;        //绝对式编码器设置2
extern volatile Uint16 *ABSEncSel;          //旋转编码器控制
extern volatile Uint16 *LnrEncCtrl2;        //全闭环直线编码器控制2
extern volatile Uint16 *AbsCmdDelay;        //串行编码器命令延迟

//运行命令，即运行中DSP提供给FPGA的参数
extern volatile Uint16 *MotRomCtrl;         //绝对式编码器EEPROM访问控制
extern volatile Uint16 *MotRomCtrl2;        //绝对式编码器EEPROM访问控制2

//状态参数表，即DSP从FPGA中读取的参数
extern volatile Uint16 *AbsEncErr;          //绝对式编码器故障状态
extern volatile Uint16 *AbsAngLow;          //编码器绝对位置低16位
extern volatile Uint16 *AbsEncState;        //绝对式编码器运行状态
extern volatile Uint16 *HDHAbsTurn;         //绝对式编码器多圈数据
extern volatile Uint16 *MotRomState;        //绝对式编码器EEPROM访问状态 
extern volatile Uint16 *MotRomState2;       //绝对式编码器EEPROM访问状态

extern volatile Uint16 *HostSysCtrl;            //相电流平衡校正和绝对式初始复位操作
extern volatile Uint16 *TestData;               //测试用，读写
extern volatile Uint16 *HostAlarmClr;           //报警清除

extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;
extern volatile union_LNRENCTRL2_REG    LnrEnCtrl2_Reg;       //直线/全闭环外部编码器设置2
volatile UNI_FPGA_HDHABSENC_SEL_REG  UNI_FPGA_HDHAbsEncSel = {0}; 

//绝对编码器ROM中存储数据相关变量引用
extern UNI_ABSROMDATA  AbsRomData;          //绝对式编码器EEPROM数据结构体
extern Uint16 * const AbsRomDataAddr[50];
extern Uint16 AbsRom_CheckWord;             //校验字
extern Uint16 AbsRom_Null;                  //空变量     
extern Uint16 AbsRom_H0030; 
extern Uint16 AbsRom_H0031; 
extern Uint16 AbsRom_H0032;

static Uint16 HDHPrmRdFlag;                    //参数读取标志位，1～未读取成功 
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void HDHAbsRom_InitDeal(void);     //上电时需要读一下绝对式编码器中的电机参数,用以对FPGA进行参数配置
void HDHAbsRomProcess(void);       //H0D04编码器ROM区读写操作函数,在转矩中断函数调用
void HDHAbsRom_EncState(void);     //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
Uint8 HDHAbsReadErrReg(void);
Uint8 HDHAbsReset(void);
Uint8 HDHAbsResetAndMultiTurn(void);
Uint16 HDHAbsEnc_SaveThetaOffset(void);
void CalcHDHEncTransTime(void);

///* Private_Functions ---------------------------------------------------------*/
///* 该文件内部调用的函数的声明 */ 
Static_Inline Uint8 HDHAbsRom_GetComStatus(void);
Static_Inline void FPGA_HDHInitAbsEncCtrl(Uint8 AbsMode);
Static_Inline Uint8 HDHAbsRom_ReadByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 HDHAbsRom_CheckByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 HDHAbsRom_ReadByte(Uint8 ReadAddr,Uint8 Mode);
Static_Inline Uint8 HDHAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr);
Static_Inline Uint8 HDHAbsRom_WriteByte(Uint8 WriteAddr);
Static_Inline void InitSetMrsCode(Uint8 MrsCode);
Static_Inline void HDHAbsEncCommInit(void);


/*******************************************************************************
  函数名:  void HDHAbsRom_EncState(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    2.
********************************************************************************/
void HDHAbsRom_EncState(void)
{
    Uint8 AbsRomAddr = 0;
    Uint16 AbsPosTempH = 0;  
    int64 AbsPosTemp = 0; 
    int64 Temp = 0; 

    HDHAbsEncState_Reg.all = *AbsEncState;     //绝对式编码器运行状态

	if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 1)
	{ 	  
        //串行编码器反馈单圈位置 
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = ((Uint32)HDHAbsEncState_Reg.bit.AbsAngHigh << 16) + (*AbsAngLow);
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb =  STR_MTR_Gvar.AbsRom.SingleAbsPosFdb & 0x000FFFFF;

        AbsPosTempH = 4095 - (*HDHAbsTurn);  //逆时针旋转 反馈多圈数据是递减的
        if(AbsPosTempH >= FunCodeUnion.code.PL_HDHEncMultiTurnOffset)
        {
            AbsPosTempH = AbsPosTempH - FunCodeUnion.code.PL_HDHEncMultiTurnOffset;
        }
        else
        {
            AbsPosTempH = (Uint32)AbsPosTempH + 4096L - (Uint32)FunCodeUnion.code.PL_HDHEncMultiTurnOffset;
        }

        //串行编码器反馈多圈+单圈位置 
        AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 20) & 0xFFFFFFFFFFF00000; 
        AbsPosTemp = (Uint64)((Uint64)AbsPosTemp | (Uint64)STR_MTR_Gvar.AbsRom.SingleAbsPosFdb); 

        if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时
        {
            Temp = (Uint64)STR_MTR_Gvar.FPGA.EncRev << 12;

            AbsPosTemp = Temp - 1 - AbsPosTemp; 
            
            STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = STR_MTR_Gvar.FPGA.EncRev - 1 - STR_MTR_Gvar.AbsRom.SingleAbsPosFdb; 
            
            AbsPosTempH = 4095 - AbsPosTempH;         
        }
         
        AuxFunCodeUnion.code.DP_AbsExtData = AbsPosTempH;
         
        STR_MTR_Gvar.AbsRom.MultiAbsPosFdbL = (Uint32)AbsPosTemp;
        STR_MTR_Gvar.AbsRom.MultiAbsPosFdbH = (Uint32)((Uint64)AbsPosTemp >> 32);
	} 
    else
    {
        HDHAbsEncCommInit();     
    } 
    
    if(HDHPrmRdFlag == 1)         //电机参数读取不成功时，置所有值为0xFFFF；
    {
        AbsRomAddr = 3;

        while(AbsRomAddr < ABSROM_WORD_LEN)          
        {             
             *AbsRomDataAddr[AbsRomAddr] = 0xFFFF;
             AbsRomAddr ++;
        }

        HDHPrmRdFlag = 0;      //清除标志位
    } 
}


/*******************************************************************************
  函数名:  void HDHAbsPosInitDeal(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline void HDHAbsEncCommInit(void)
{
    static int8 delaycnt = 0;
    Uint32 AbsPosTempL = 0;
    int16 AbsPosTempH = 0;
    int64 AbsPosTemp = 0; 
    int64 Temp = 0;

    if(0 == delaycnt)
    {
        if(0 == HDHAbsRom_GetComStatus()) 
        {
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_SQNCRD);  //设置为连续读
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
            AbsPosTempL = (Uint32)((Uint32)HDHAbsEncState_Reg.bit.AbsAngHigh << 16) + (Uint32)(*AbsAngLow);
            AbsPosTempL =  AbsPosTempL & 0x000FFFFF;
            AbsPosTempH = 4095 - (*HDHAbsTurn);  //逆时针旋转 反馈多圈数据是递减的;
            if(AbsPosTempH >= FunCodeUnion.code.PL_HDHEncMultiTurnOffset)
            {
                AbsPosTempH -= FunCodeUnion.code.PL_HDHEncMultiTurnOffset;
            }
            else
            {
                AbsPosTempH = AbsPosTempH + 4096 - FunCodeUnion.code.PL_HDHEncMultiTurnOffset;
            }

            AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 20) & 0xFFFFFFFFFFF00000;
            AbsPosTemp = (int64)((Uint64)AbsPosTemp | (Uint64)AbsPosTempL);

            if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时
            {
                Temp = (Uint64)STR_MTR_Gvar.FPGA.EncRev << 12;
    
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
  函数名:  void HDHAbsRom_InitDeal(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 上电时需要读一下绝对式编码器中的电机参数,用以对FPGA进行参数配置
    2.
********************************************************************************/    
void HDHAbsRom_InitDeal(void)
{
    Uint8 AbsRomAddr = 0;
    Uint8 Temp = 0;
    Uint16 Temp_16 = 0;     

    FunCodeUnion.code.MT_EncoderSel = 0x11;         //H00_30 编码器类型
    FunCodeUnion.code.MT_EncoderPensL = 0;          //H00_31 编码器线数L
    FunCodeUnion.code.MT_EncoderPensH = 0x10;       //H00_32 编码器线数H 

    if(14140 == FunCodeUnion.code.MT_MotorModel)  UNI_FPGA_HDHAbsEncSel.bit.HDHAbsType = 1;
    else UNI_FPGA_HDHAbsEncSel.bit.HDHAbsType = 0;

    //说明：通讯频率=40MHz/2/(BaudSelect+1)
    //2M   需要添加功能码
    //UNI_FPGA_RotEncCtrl.bit.RotEncFiltTm = 9;
    //如果参数设置过小，恢复出厂值  
    if(FunCodeUnion.code.OEM_HDHEncBaud < 100) FunCodeUnion.code.OEM_HDHEncBaud = 2000;
    Temp_16 = (20000L / FunCodeUnion.code.OEM_HDHEncBaud) - 1;     
    UNI_FPGA_HDHAbsEncSel.bit.BaudSelect = 9;
    *ABSEncSel    = UNI_FPGA_HDHAbsEncSel.all;    //旋转编码器   

    //海德汉编码器恢复时间
    //AbsTstTime≥BaudSelect，默认值为39（即1us，对应50m线长；线长增加时，该值也要增大），建议通过功能码设置。
    Temp_16 =  (Uint16)((Uint32)1000000L / (Uint32)FunCodeUnion.code.OEM_HDHEncBaud);
    if(FunCodeUnion.code.OEM_HDHAbsTstTime < Temp_16) FunCodeUnion.code.OEM_HDHAbsTstTime = Temp_16;

    Temp_16 = (FunCodeUnion.code.OEM_HDHAbsTstTime / FPGA_PRD) - 1;
    LnrEnCtrl2_Reg.bit.HDHAbsTstTime = Temp_16;
    *LnrEncCtrl2 = LnrEnCtrl2_Reg.all;  

    DELAY_US(100000L);    //延时100ms以等待绝对式编码器完成上电待机
    
    //读取编码器存储EEPROM参数
    InitSetMrsCode(0xAD); 
    
    AbsRomAddr = 0;
    while(AbsRomAddr < 100)
    {   
        Temp = HDHAbsRom_ReadByte(AbsRomAddr,0);
        if(1 == Temp) 
        {
            AbsRomAddr++;
        }
        else if(2 == Temp) AbsRomAddr = 100 + 1;

        DELAY_US(62);                    //虚拟中断周期调用的情况

        HDHAbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态 
        if(0 != (HDHAbsErr_Reg.all & 0x0F))     //通讯错误（RX端）
        {
            AbsRomAddr = 100 + 1;
            HDHPrmRdFlag = 1;        //参数读取不成功

            FunCodeUnion.code.MT_ABSEncVer = 65535;            //H00_04 编码器软件版本号 
            FunCodeSaveInAbsRomInit();
            break;   
        }
    }

    if(AbsRomAddr == ABSROM_BYTE_LEN)
    {
        if(ABSROM_CHECK_WORD != AbsRomData.all_16Bits[0])
        {
            HDHPrmRdFlag = 1;              //参数校验不成功
            PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据  

            //获取校验字和版本号
            AbsRom_CheckWord = AbsRomData.all_16Bits[0];      
            FunCodeUnion.code.MT_ABSEncVer = AbsRomData.all_16Bits[2];   
            FunCodeSaveInAbsRomInit();
        }
        else
        {
            AbsRomAddr = 0;
            while(AbsRomAddr < ABSROM_WORD_LEN)
            {
                *AbsRomDataAddr[AbsRomAddr] = AbsRomData.all_16Bits[AbsRomAddr];
                AbsRomAddr ++;
            }
        }
    }
    
    FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID);     
}
/*******************************************************************************
  函数名: void SetMrsCode(Uint8 MrsCode)  
  输入:    
  输出:    
  子函数:         
  描述: 
    ★ 选择存储区域操作步骤如下：（存储器某个地址的读写操作过程与620P类似）
    ①查询编码器状态寄存器0x22，若AbsComStatus为0，则执行下一步，否则等待该位变为0。
    ②将控制参数寄存器0x14中的AbsMode位域值设为2，即"选择存储区域"模式。
    ③将控制参数寄存器0x3E中的MrsCode位域设置为所需的区域MRS值。
    ④在运行命令寄存器0x3B中，将EepromMode位置1。执行该寄存器则会启动对编码器内部存储区域的选择操作。
    ⑤不断查询编码器状态寄存器0x22，当AbsComStatus位变为0时，若AbsComErr亦为0，则操作成功，否则本次访问失败。
    ⑥将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式。    
********************************************************************************/
Static_Inline void InitSetMrsCode(Uint8 MrsCode)
{
    //Uint8 TimeDelay = 0;
	Uint16 TimeDelay = 0;//by huangxin201712_1 导入裴高科修改点，数据溢出bug
    DELAY_US(62);

    while((1 == HDHAbsRom_GetComStatus()) && (TimeDelay < 100))   //等到通讯状态不忙时再恢复设置,只延时100us
    {
        DELAY_US(1);                    //延时
        TimeDelay++;
    }

    FPGA_HDHInitAbsEncCtrl(HDHABSENC_SETMRSCODE);

    HDHMotRomCtrl2_Reg.bit.ByteToEeprom2 = 0;
    HDHMotRomCtrl2_Reg.bit.MrsCode = MrsCode;
    *MotRomCtrl2 = HDHMotRomCtrl2_Reg.all;


    HDHMotRomCtrl_Reg.bit.EepromMode = 1;
    *MotRomCtrl = HDHMotRomCtrl_Reg.all;

    DELAY_US(62);

    while((1 == HDHAbsRom_GetComStatus()) && (TimeDelay < 500))   //等到通讯状态不忙时再恢复设置,只延时100us
    {
        DELAY_US(1);                    //延时
        TimeDelay++;
    }
 
    FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID); 
    DELAY_US(62);   
}
/*******************************************************************************
  函数名: void SetMrsCode(Uint8 MrsCode)  
  输入:    
  输出: 1～完成，0～未完成   
  子函数:         
  描述: 
    ★ 选择存储区域操作步骤如下：（存储器某个地址的读写操作过程与620P类似）
    ①查询编码器状态寄存器0x22，若AbsComStatus为0，则执行下一步，否则等待该位变为0。
    ②将控制参数寄存器0x14中的AbsMode位域值设为2，即"选择存储区域"模式。
    ③将控制参数寄存器0x3E中的MrsCode位域设置为所需的区域MRS值。
    ④在运行命令寄存器0x3B中，将EepromMode位置1。执行该寄存器则会启动对编码器内部存储区域的选择操作。
    ⑤不断查询编码器状态寄存器0x22，当AbsComStatus位变为0时，若AbsComErr亦为0，则操作成功，否则本次访问失败。
    ⑥将控制参数寄存器0x14中的AbsMode位域值设为后续操作所需要的模式。    
********************************************************************************/
Static_Inline Uint8 SetMrsCode(Uint8 MrsCode)
{
	static Uint8 SetMrsCodeStep = 0; 
    static Uint8 DelayCnt = 0;

    if(1 == HDHAbsRom_GetComStatus()) return 0;

    switch(SetMrsCodeStep)
    {
        case 0:  
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID); 
            SetMrsCodeStep = 1;
            return 0;

        case 1:  
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_SETMRSCODE); 
            SetMrsCodeStep = 2;
            return 0;

        case 2: 
            HDHMotRomCtrl2_Reg.bit.ByteToEeprom2 = 0;        
            HDHMotRomCtrl2_Reg.bit.MrsCode = MrsCode;
            *MotRomCtrl2 = HDHMotRomCtrl2_Reg.all;        
        
            HDHMotRomCtrl_Reg.bit.EepromMode = 1;
            *MotRomCtrl = HDHMotRomCtrl_Reg.all;

            SetMrsCodeStep = 3;
            DelayCnt = 0;
            return 0;

        case 3:  
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID); 
            DelayCnt++;
            if(DelayCnt > 20) SetMrsCodeStep = 4;
            return 0;

        case 4:  
            SetMrsCodeStep = 0;
            DelayCnt = 0;
            return 1;

        default:
            SetMrsCodeStep = 0;
            return 0;
    } 
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Uint8 HDHAbsReadErrReg(void)
{
	static Uint8 ReadErrRegStep = 0;
    Uint16 Temp = 0;

    if(0 == ReadErrRegStep)
    {
        if(1 == SetMrsCode(0xB9)) ReadErrRegStep = 1;
    }
    else if(1 == ReadErrRegStep)
    {
        if(1 == HDHAbsRom_ReadByte(0,1))
        {
            //故障值
            AuxFunCodeUnion.code.DP_ABSEncErr = HDHMotRomState_Reg.bit.ByteFromEeprom;
            AuxFunCodeUnion.code.DP_ABSEncErr &= 0x00FF;
            ReadErrRegStep = 2;               
        }
    }
    else if(2 == ReadErrRegStep)
    {
        if(1 == HDHAbsRom_ReadByte(1,1))
        {
            //警告值
            Temp = HDHMotRomState_Reg.bit.ByteFromEeprom;
            Temp = (Temp << 8) & 0xFF00;        
            AuxFunCodeUnion.code.DP_ABSEncErr |= Temp;  
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_SQNCRD);
            ReadErrRegStep = 0;                 
            return 0;
        }
    }

    return 1;
}
/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Uint8 HDHAbsResetAndMultiTurn(void)
{
    static Uint8 Step = 0;
    static Uint16 WaitDelay = 0;

    if(0 == Step)
    {
        if(0 == HDHAbsReset()) 
        {
            Step = 1;
            WaitDelay = 0;
        } 
    }
    else if(1 == Step)
    {
        if(WaitDelay < 1600)					 
        {
            WaitDelay++;
        }
        else        
        {
             Step = 2; 
        }     
    }
    else
    {
        FunCodeUnion.code.PL_HDHEncMultiTurnOffset = 0;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_HDHEncMultiTurnOffset));
        Step = 0;
        return 0;
    }

    return 2;
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
Uint8 HDHAbsReset(void)
{
	static Uint8 ReadErrRegStep = 0;
    static Uint16 WaitDelay = 0; 

    if(0 == ReadErrRegStep)
    {
        if(1 == SetMrsCode(0xB9)) ReadErrRegStep = 1;
    }
    else if(1 == ReadErrRegStep)
    {
        if(0 == HDHAbsRom_GetComStatus())
        {
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_RANDWRTROM);     
            ReadErrRegStep = 2;
        }
    }
    else if(2 == ReadErrRegStep)
    {
        if(1 == HDHAbsRom_WriteByteCmd(0, 0))      
        {
            ReadErrRegStep = 3;                 
            WaitDelay = 0;
        }
    }
    else if(3 == ReadErrRegStep)
    {
        //延时1000ms
        if(WaitDelay < 16000)					     //需要把延迟时间加长
        {
            WaitDelay++;
        }
        else        
        {
             ReadErrRegStep = 4; 
        }    
    }
    else if(4 == ReadErrRegStep)
    {
        if(0 == HDHAbsRom_GetComStatus())
        {
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_RSTABSPOS);     
            ReadErrRegStep = 5;
        }        
    }
    else if(5 == ReadErrRegStep)
    {
        HDHMotRomCtrl_Reg.bit.ByteToEeprom = 0;
        HDHMotRomCtrl_Reg.bit.EepromAddr   = 0;
        HDHMotRomCtrl_Reg.bit.EepromMode = 1;

        *MotRomCtrl = HDHMotRomCtrl_Reg.all;        
        WaitDelay = 0;
        ReadErrRegStep = 6;
    }
    else if(6 == ReadErrRegStep)
    {
        //延时1000ms
        if(WaitDelay < 16000)					     //需要把延迟时间加长
        {
            WaitDelay++;
        }
        else        
        {	
			WaitDelay = 0;
            ReadErrRegStep = 7; 
        } 
    }
    else if(7 == ReadErrRegStep)
    {
        if(1 == HDHAbsRom_ReadByte(0,1))
        {
            //故障值
            AuxFunCodeUnion.code.DP_ABSEncErr = HDHMotRomState_Reg.bit.ByteFromEeprom;
            ReadErrRegStep = 8;
			WaitDelay = 0;  
        }
    }
    else if(8 == ReadErrRegStep)
    {         
        FPGA_HDHInitAbsEncCtrl(HDHABSENC_SQNCRD);
        ReadErrRegStep = 0;
        return 0; 
    }

    return 1;
}

/*******************************************************************************
  函数名:  void HDHAbsRomProcess(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. H0D04编码器ROM区读写操作函数,在转矩中断函数调用
    2.
********************************************************************************/
void HDHAbsRomProcess(void)
{
    static Uint16 State = 0;
    static Uint8 AbsRomAddr = 0; 
    Uint8 Temp = 0; 

    switch(State)
    {
        case 0:  //空闲
            if(AuxFunCodeUnion.code.MT_OperAbsROM == 2)
            {
                if(1 == SetMrsCode(0xAD))
                {
                    AbsRomAddr = 0;
                    while(AbsRomAddr < ABSROM_WORD_LEN)
                    {
                         AbsRomData.all_16Bits[AbsRomAddr] = 0;
                         AbsRomAddr ++;
                    }

                    //开始读AbsRom
                    State = 1;
                    AbsRomAddr = 0;
                }
            }
            else if(AuxFunCodeUnion.code.MT_OperAbsROM == 1)
            {
                if(1 == SetMrsCode(0xAD))
                {
                    //得到要保存到电机ROM中的参数
                    AbsRom_CheckWord = ABSROM_CHECK_WORD;
                    FunCodeUnion.code.MT_ABSEncVer = 0;
                    AbsRom_H0030 = 0x0011; 
                    AbsRom_H0031 = 0x0000; 
                    AbsRom_H0032 = 0x0010;                    
                    AbsRom_Null = 0;
                    AbsRomAddr = 0;
                    while(AbsRomAddr < ABSROM_WORD_LEN)
                    {
                         AbsRomData.all_16Bits[AbsRomAddr] = *AbsRomDataAddr[AbsRomAddr];
                         AbsRomAddr ++;
                    }
    
                    //开始写AbsRom
                    State = 3;
                    AbsRomAddr = 0;
                }
            }
            else
            {
                State = 0;
                AuxFunCodeUnion.code.MT_OperAbsROM = 0;
            }
            break;

        case 1: //读AbsRom中
            Temp = HDHAbsRom_ReadByte(AbsRomAddr,0);

            if(1 == Temp) 
            {
                AbsRomAddr++;
            }
            else if(2 == Temp)   //发生校验故障或通信故障
            {
                State = 5;    //出错
            }

            if(AbsRomAddr >= ABSROM_BYTE_LEN) State = 2;
            break;

        case 2: //读AbsRom完成 
            if(ABSROM_CHECK_WORD != AbsRomData.all_16Bits[0])
            {
                PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据
            }
            else
            {
                AbsRomAddr = 0;
                while(AbsRomAddr < ABSROM_WORD_LEN)
                {
                     *AbsRomDataAddr[AbsRomAddr] = AbsRomData.all_16Bits[AbsRomAddr];
                     AbsRomAddr ++;
                }
            }

            State = 4;
            AbsRomAddr = 0;
            break;

        case 3: //写AbsRom中
            Temp = HDHAbsRom_WriteByte(AbsRomAddr);

            if(1 == Temp)
            {
                AbsRomAddr++;
            }
            else if(2 == Temp)
            {  
                State = 5;    //出错
            }

            if(AbsRomAddr >= ABSROM_BYTE_LEN) State = 4;    //完成
            break;

        case 4: //写，读AbsRom完成
        case 5:
            if(0 == HDHAbsRom_GetComStatus())
            {
                if(State == 4)        //读写成功
                { 
                    AuxFunCodeUnion.code.MT_OperAbsROM = 0;
                }
                else                  //读写失败提示
                {
                    AuxFunCodeUnion.code.MT_OperAbsROM = 3;                      
                }

                FPGA_HDHInitAbsEncCtrl(HDHABSENC_SQNCRD);  //设置为连续读
                State = 0;                    
                AbsRomAddr = 0;                
            }
            break;
        default:
            break;
    }
} 

/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_GetComStatus(void)
  输入:   无 
  输出:   0～空闲,1～通讯忙 
  子函数: 无
  描述：
    1. 得到绝对式编码器当前的通讯状态，0～空闲,1～通讯忙
    2.
********************************************************************************/
Static_Inline Uint8 HDHAbsRom_GetComStatus(void)
{
    Uint16 DelayCnt = 0;      //延迟时间计算

    HDHAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态;

    //需要等到通信不忙的时候才进行写操作
    while((HDHAbsEncState_Reg.bit.AbsComStatus == 1) && (DelayCnt < 10))
    {
        DelayCnt++;
        DELAY_US(1);
        HDHAbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态
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
void CalcHDHEncTransTime(void)
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

    /*位置读取总通讯时间tenc = 30μs（按2MHz时钟、0.5us恢复时间设置，
      实测的总通讯时间为26us）。注意相邻两次通讯之间的总线空闲时间
      必须大于16us（编码器规格书中的tm时间）。*/
    Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 30000;
    Tabsenc   = Tabsenc / FPGA_PRD;
    STR_MTR_Gvar.AbsRom.AbsTransferTime = Tabsenc;

    //编码器通讯波特率换算  
    STR_MTR_Gvar.AbsRom.AbsCommBaud = (1000000L / FunCodeUnion.code.OEM_HDHEncBaud) / FPGA_PRD;

    //串行编码器的通讯启动延时参数，只做保存
    AbsTransferDelay = (Ts - Tspd - Tmcu  + Ts - Tabsenc) >> 1; 

    if(AbsTransferDelay > (Ts >> 1))
    {
        AbsTransferDelay -= (Ts >> 1);
    }

    HDHAbsEncCtrl_Reg.bit.AbsTransferDelay = AbsTransferDelay; 

    *AbsEncCtrl = HDHAbsEncCtrl_Reg.all;

    /*说明：AbsCmdDelay = (Ts + Δ - tenc - tspd) / Tclk。
      其中Ts为同步周期（FPGA中断周期），
      tenc为编码器的总通讯时间，
      Δ为编码器命令字的传输时间（必要的话还要考虑线延迟），
      tspd为速度计算时间，Tclk为系统主时钟周期。此处的Δ=0。*/ 
    CmdTransTime = 0;
    Temp = (FunCodeUnion.code.OEM_EncCmdTransLineDelay * 10L) / FPGA_PRD;
    Temp += Ts + CmdTransTime - Tspd  - Tabsenc;        
    *AbsCmdDelay = Temp; 

}
/*******************************************************************************
  函数名:  Static_Inline void FPGA_HDHInitAbsEncCtrl(Uint8 AbsMode)
  输入:   编码器工作模式设置 
  输出:   无
  子函数: 无
  描述：
    1. 绝对式编码器控制初始化
    2.
********************************************************************************/
Static_Inline void FPGA_HDHInitAbsEncCtrl(Uint8 AbsMode)
{
    HDHAbsEncCtrl_Reg.bit.AbsMode = AbsMode;   //编码器工作模式设置

    *AbsEncCtrl = HDHAbsEncCtrl_Reg.all;
}

/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_ReadByteCmd(Uint8 ReadAddr)
  输入:   读地址
  输出:   1～读取命令发送成功   0～通信忙,未读取,继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 HDHAbsRom_ReadByteCmd(Uint8 ReadAddr)
{
	static Uint16 MonitStep = 0;

	if(MonitStep == 0)
	{
        if(0 == HDHAbsRom_GetComStatus())
        {
            HDHMotRomCtrl_Reg.bit.EepromAddr   = ReadAddr;
            HDHMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = HDHMotRomCtrl_Reg.all;

			MonitStep = 1;						 //下一次检测
		}
	}
	else
	{
        if(0 == HDHAbsRom_GetComStatus())
        {
            HDHMotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态

            if(HDHMotRomState_Reg.bit.EepromBusy == 0)
            {
			    MonitStep = 0;
				return 1;
			}
		}
	}

    return 0;
}

/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_CheckByteCmd(Uint8 ReadAddr)
  输入:   读地址
  输出:   1～完成校验，将数据读出，0～未完成校验
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 HDHAbsRom_CheckByteCmd(Uint8 ReadAddr)
{
    if(0 == HDHAbsRom_GetComStatus())
    {
        HDHMotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态
        HDHMotRomState2_Reg.all  = *MotRomState2;      //EEPROM访问状态

        if(HDHMotRomState_Reg.bit.EepromBusy == 0)
        {
            return 1;
        }
    }

    return 0;    
}

/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_ReadByte(Uint8 ReadAddr,Uint8 Mode)
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
    备注：海德汉读数据至少需要400uss
********************************************************************************/
Static_Inline Uint8 HDHAbsRom_ReadByte(Uint8 ReadAddr,Uint8 Mode)
{
    static Uint16 TimeDelay = 0;
    static Uint8 ReadByteStep = 0;   //等待模式设置
    static Uint8 ReadByteDly = 0;  

    if(ReadByteStep == 0)
    {
        //完成第1、2步
        FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽避免FPGA一直处于忙碌状态

        if(0 == HDHAbsRom_GetComStatus())
        {
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_RANDRDROM);      //设置绝对式编码器模式为随机读EEPROM

            ReadByteStep = 1;
        }
    }
    else if(ReadByteStep == 1)
    {
        //完成第3、4、5步
        if(1 == HDHAbsRom_ReadByteCmd(ReadAddr))
        {
            ReadByteStep = 2;     
            ReadByteDly = 0;
        }
    }
    else if(ReadByteStep == 2)
    {
        //延时0.5ms
        TimeDelay = 0;

        ReadByteDly ++;
        if(ReadByteDly > 8) ReadByteStep = 3;
    }
    else if(ReadByteStep == 3)
    {
        //完成第5、6步
        if(1 == HDHAbsRom_CheckByteCmd(ReadAddr))
        {
            if(ReadAddr == HDHMotRomState_Reg.bit.EepromAddr)
            {
                if(0 == Mode)AbsRomData.all_8Bits[ReadAddr] = HDHMotRomState_Reg.bit.ByteFromEeprom;
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
    if((TimeDelay > 4000) || (0 != (HDHAbsErr_Reg.all & 0x0F)))          //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);      //延时过长警告  Er.740
        ReadByteStep = 0;   //等待模式设置
        TimeDelay = 0;
        return 2;                    //不再进入读
    }

    return 0;
}


/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_WriteByteCmd(Uint16 WriteByte, Uint16 WriteAddr)
  输入:   读地址
  输出:   1～写入命令发送成功， 0～通信忙，无法写入，继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 HDHAbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr)
{
	static Uint16 MonitStep = 0;

	if(MonitStep == 0)		        //第一次进入时检测
	{
        if(0 == HDHAbsRom_GetComStatus())
        {
            HDHMotRomCtrl_Reg.bit.ByteToEeprom = WriteByte;
            HDHMotRomCtrl_Reg.bit.EepromAddr   = WriteAddr;
            HDHMotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = HDHMotRomCtrl_Reg.all;                //启动Rom操作
			MonitStep = 1;     						  //Rom操作完后进入下个周期判定
		}
	}							   //第二步操作
	else
	{
        if(0 == HDHAbsRom_GetComStatus())
        {
            HDHMotRomState_Reg.all  = *MotRomState;               //EEPROM访问状态

            if(HDHMotRomState_Reg.bit.EepromBusy == 0)
            {
				MonitStep = 0;
				return 1;
			} 
		}
	}

    return 0;
}
/*******************************************************************************
  函数名:  Static_Inline Uint8 HDHAbsRom_WriteByte(Uint8 ReadAddr)
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
Static_Inline Uint8 HDHAbsRom_WriteByte(Uint8 WriteAddr)
{
    static Uint8 WriteByteStep = 0;         //等待模式设置
    static Uint16 TimeDelay = 0;            //延时时间计数器
    static Uint16 WaitDelay = 0;            //等待时间计数器

    if(WriteByteStep == 0)
    {
        //完成第1、2步
        FPGA_HDHInitAbsEncCtrl(HDHABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽再设置模式

        if(0 == HDHAbsRom_GetComStatus())
        {
            FPGA_HDHInitAbsEncCtrl(HDHABSENC_RANDWRTROM);      //设置绝对式编码器模式为随机写EEPROM

            WriteByteStep = 1;
        }
    }
    else if(WriteByteStep == 1)
    {
        //完成第3、4、5步
        if(1 == HDHAbsRom_WriteByteCmd(AbsRomData.all_8Bits[WriteAddr], WriteAddr))      //判定写命令是否发送完
        {
            WriteByteStep   = 2;                 //写命令完成后进入到等待阶段
            WaitDelay = 0;
        }
    }
    else if(WriteByteStep == 2)
    {
        //延时16ms
        if(WaitDelay < 160)					     //需要把延迟时间加长
        {
            WaitDelay++;
        }
        else        //累积了500个周期，31ms后进入读校验阶段
        {
            WaitDelay = 0;
            TimeDelay    = 0;
            WriteByteStep = 3;      
        }
    }
    else if(WriteByteStep == 3)
    {
        if(1 == HDHAbsRom_ReadByte(WriteAddr,0))
        {
            if((WriteAddr != HDHMotRomState_Reg.bit.EepromAddr) ||
               (AbsRomData.all_8Bits[WriteAddr] != HDHMotRomState_Reg.bit.ByteFromEeprom))
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
    if((TimeDelay > 6000) || (0 != (HDHAbsErr_Reg.all & 0x0F)))           //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);       //延时过长警告 Er.740
        WriteByteStep = 0;      //进入等待阶段
        TimeDelay = 0;
        return 2;                       //结束写
    }

    return 0;
}

/*******************************************************************************
  函数名:  Uint16 HDHAbsEnc_SaveThetaOffset(void)
  输入:   
  输出:   1～成功，0～不成功
  子函数: 无
  描述：

********************************************************************************/
Uint16 HDHAbsEnc_SaveThetaOffset(void)
{ 
    static Uint16 Step    = 0;     //初次复位 
    static Uint16 TimeDelay    = 0;             //延时周期数

    if(Step == 0)
    {
        if(1 == SetMrsCode(0xAD)) 
        {   
            TimeDelay = 0; 
            AbsRomData.all_16Bits[26] = FunCodeUnion.code.MT_ThetaOffsetL;      //H00_28 绝对式码盘位置偏置L 
            AbsRomData.all_16Bits[27] = FunCodeUnion.code.MT_ThetaOffsetH;      //H00_29 绝对式码盘位置偏置H                        
            Step = 1;
        }
    }         
    else if(Step == 1)       //将位置信息保存到相应功能码   
	{
	    if(1 == HDHAbsRom_WriteByte(52))	   //保存地址为52 
		{
		    TimeDelay = 0;
            Step = 2;
		}			    
	}
    else if(Step == 2)         
	{
	    if(1 == HDHAbsRom_WriteByte(53))	   //保存地址为53 
		{
		    TimeDelay = 0;
            Step = 3;
		}			    
	}
    else if(Step == 3)         
	{
	    if(1 == HDHAbsRom_WriteByte(54))	   //保存地址为54 
		{
		    TimeDelay = 0;
            Step = 4;
		}			    
	}
	else if(Step == 4)
	{
  		if(1 == HDHAbsRom_WriteByte(55))      //保存地址为55
		{
	    	TimeDelay = 0;
	    	Step = 0;
            return 1;			
		}	
    } 	
		
	TimeDelay++;
	
    if(TimeDelay > 4000)
    {
        TimeDelay = 0;
        Step = 0;        //回到第一步
        PostErrMsg(ENCDERR_Z8);        //发生了复位故障 Er.740
        return 2;
    }

    return 0;
}


/********************************* END OF FILE *********************************/
