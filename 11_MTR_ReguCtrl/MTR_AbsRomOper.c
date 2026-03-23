/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    MTR_AbsRomOper.c  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   王治国                 修改日期：2012.06.28
            姚虹                             2012.11.20
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.多摩川绝对位置编码器
    2.将该文件移植MTR模块     
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_Main.h"
#include "MTR_AbsRomOper.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"
#include "MTR_FPGAInterface.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */
//字节读写步骤
#define    STEPWAITSET          0       //等待设置——以确定是读还是写
#define    STEPWRITEBYTE        1       //字节写阶段
#define    STEPWAITAFTERWRT     2       //字节写完等待阶段
#define    STEPREADBYTE         3       //字节读阶段
#define    STEPCHECKBYTE        4       //字节校验阶段

//复位操作步骤
#define    STEPRESET            0       //复位命令发送阶段
#define    STEPWAIT0            1       //等待复位完成
#define    STEPWAIT1            2       //等待复位完成
#define    STEPWAIT2            3       //等待复位完成
#define    STEPREVERT           4       //复位完成还原阶段   

//编码器工作模式
#define ABSENC_FORBID           0       //绝对编码器禁止
#define ABSENC_SQNCRD           1       //连续读数据
//#define ABSENC_RANDRD           2       //随机读数据
#define ABSENC_RANDRDROM        3       //随机读EEPROM
#define ABSENC_RANDWRTROM       4       //随机写EEPROM
#define ABSENC_SQNCRD_1         5       //连续读数据1
#define ABSENC_SQNCRD_2         6       //连续读数据2            

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
UNI_ABSENCSTATE_REG     AbsEncState_Reg;    //绝对式编码器运行状态
UNI_MOTROMSTATE_REG     MotRomState_Reg;    //EEPROM访问状态
UNI_ABSROMDATA          AbsRomData;         //绝对式编码器EEPROM数据结构体
UNI_ABSENCTRL_REG       AbsEncCtrl_Reg;     //绝对式编码器控制

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//控制参数，即需初始化的参数
extern volatile Uint16 *AbsEncCtrl;     //绝对式编码器设置
extern volatile Uint16 *AbsCmdDelay;     //串行编码器命令延迟

//运行命令，即运行中DSP提供给FPGA的参数
extern volatile Uint16 *MotRomCtrl;     //绝对式编码器EEPROM访问控制

//状态参数表，即DSP从FPGA中读取的参数
extern volatile Uint16 *AbsEncErr;      //绝对式编码器故障状态
extern volatile Uint16 *AbsAngLow;      //编码器绝对位置低16位
extern volatile Uint16 *AbsEncState;    //绝对式编码器运行状态
extern volatile Uint16 *AbsExtData;     //串行编码器扩展数据
extern volatile Uint16 *MotRomState;    //绝对式编码器EEPROM访问状态


extern volatile Uint16 *HostSysCtrl;            //相电流平衡校正和绝对式初始复位操作
extern volatile Uint16 *TestData;               //测试用，读写
extern volatile Uint16 *HostAlarmClr;           //报警清除
extern volatile Uint16 *ABSEncSel;              //绝对编码器选择
extern volatile Uint16 *PosFbkLow;              //位置响应低16位
extern volatile int16  *PosFbkHigh;             //位置响应高16位
extern volatile Uint16 *FPGA_RevCode;           //FPGA版本号
extern volatile Uint16 *HostPosRst;             //位置偏差计数器清零

extern volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr;   //报警命令寄存器
extern volatile UNI_FPGA_ABSENC_SEL_REG   UNI_FPGA_AbsEncSel; //旋转编码器控制寄存器

//绝对编码器ROM中存储数据地址表
Uint16 AbsRom_CheckWord = 0;          //校验字
Uint16 AbsRom_Null = 0;               //空变量

UNI_ABSERR_REG AbsErr_Reg = {0};

Uint16 AbsRom_H0030 = 0; 
Uint16 AbsRom_H0031 = 0; 
Uint16 AbsRom_H0032 = 0; 

static Uint8 PrmRdFlag = 0;            //参数读取标志位，1～未读取成功
static Uint8 EncFrameMode = 0;

Uint16 * const AbsRomDataAddr[50] =
{
    (Uint16 *)&AbsRom_CheckWord,                          //校验字 应当为0xAA55
    (Uint16 *)&AbsRom_Null,                               //保留 
    (Uint16 *)&FunCodeUnion.code.MT_ABSEncVer,            //H00_04 编码器软件版本号
    (Uint16 *)&AbsRom_Null,                               //保留
    (Uint16 *)&AbsRom_Null,                               //保留
    (Uint16 *)&AbsRom_Null,                               //保留
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
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd0025,             //H00_25
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd0026,             //H00_26
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd0027,             //H00_27
    (Uint16 *)&FunCodeUnion.code.MT_ThetaOffsetL,         //H00_28 绝对式码盘位置偏置L 
    (Uint16 *)&FunCodeUnion.code.MT_ThetaOffsetH,         //H00_29 绝对式码盘位置偏置H 
    (Uint16 *)&AbsRom_H0030, //FunCodeUnion.code.MT_EncoderSel,           //H00_30 编码器类型    新程序不在保存
    (Uint16 *)&AbsRom_H0031, //FunCodeUnion.code.MT_EncoderPensL,         //H00_31 编码器线数L   新程序不在保存
    (Uint16 *)&AbsRom_H0032, //FunCodeUnion.code.MT_EncoderPensH,         //H00_32 编码器线数H   新程序不在保存
    (Uint16 *)&FunCodeUnion.code.MT_AbsRomMotorModel,     //H00_35 总线电机型号
    (Uint16 *)&FunCodeUnion.code.MT_AbsEncFunBit,         //H00_37 绝对编码器功能设置位 
    (Uint16 *)&FunCodeUnion.code.OEM_KedGain,             //H01_22 D轴反电动势补偿系数
    (Uint16 *)&FunCodeUnion.code.OEM_KeqGain,             //H01_23 Q轴反电动势补偿系数
    (Uint16 *)&FunCodeUnion.code.OEM_CurIdKpSec,          //H01_24 D轴电流环比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CurIdKiSec,          //H01_25 D轴电流环积分补偿因子
    (Uint16 *)&FunCodeUnion.code.OEM_CurIqKpSec,          //H01_27 Q轴电流环比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CurIqKiSec,          //H01_28 Q轴电流环积分补偿因子
    (Uint16 *)&AbsRom_Null,                               //20位绝对编码器校准时校验使用                           
    (Uint16 *)&FunCodeUnion.code.OEM_CapIdKp,             //H01_52 性能优先模式D轴比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CapIdKi,             //H01_53 性能优先模式D轴积分补偿因子
    (Uint16 *)&FunCodeUnion.code.OEM_CapIqKp,             //H01_54 性能优先模式Q轴比例增益
    (Uint16 *)&FunCodeUnion.code.OEM_CapIqKi,             //H01_55 性能优先模式Q轴积分补偿因子
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd48,               //H00_48
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd49,               //H00_49
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd50,               //H00_50
    (Uint16 *)&FunCodeUnion.code.MT_Rsvd51,               //H00_51
    (Uint16 *)&AuxFunCodeUnion.code.DP_EncStBit1,         //H0B_31
    (Uint16 *)&AbsRom_Null,                               //20位绝对编码器写使能
};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void HCAbsRom_InitDeal(void);           //上电时需要读一下绝对式编码器中的电机参数,用以对FPGA进行参数配置
void AbsRomProcess(void);               //H0D04编码器ROM区读写操作函数,在转矩中断函数调用
void AbsRom_EncState(void);             //得到绝对式编码器运行状态，故障状态和EEPROM访问状态
Uint8 AbsRom_SoftRstProcess(void);      //软件复位时禁止绝对式编码器读写
Uint8 HC2ndAbsRom_ClcErrAndMultiTurn(Uint8 Mode);     //复位多圈编码器故障及多圈参数
void CalcHCEncTransTime(void);          //计算总通讯时间
Uint8 HCAbsEnc_SaveThetaOffset(void);   //存储角度辨识后的编码器位置偏置
void FunCodeSaveInAbsRomInit(void);     //初始化存储在编码器ROM中的功能码

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline Uint8 AbsRom_GetComStatus(void);
Static_Inline void FPGA_InitAbsEncCtrl(Uint8 AbsMode);
Static_Inline Uint8 AbsRom_ReadByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 AbsRom_CheckByteCmd(Uint8 ReadAddr);
Static_Inline Uint8 AbsRom_ReadByte(Uint8 ReadAddr,Uint8 ExtDataSel);
Static_Inline Uint8 AbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr);
Static_Inline Uint8 AbsRom_WriteByte(Uint8 WriteByte, Uint8 WriteAddr);
Static_Inline void HCAbsEncCommInit(void);
Static_Inline void SetAbsEncCtrl_SQNCRD(void);

/*******************************************************************************
  函数名:  void AbsRom_EncState(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 得到绝对式编码器运行状态，故障状态和EEPROM访问状态
    2.
********************************************************************************/
void AbsRom_EncState(void)
{
    Uint8 AbsRomAddr;
    Uint16 AbsPosTempH = 0;  
    int64 AbsPosTemp = 0; 
    int64 Temp = 0;

    AbsEncState_Reg.all = *AbsEncState;     //绝对式编码器运行状态
    
    if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit == 1)
    {
        //串行编码器反馈单圈位置 
        STR_MTR_Gvar.AbsRom.SingleAbsPosFdb = (Uint32)((Uint32)AbsEncState_Reg.bit.AbsAngHigh << 16) + (Uint32)(*AbsAngLow);
        if(0x800000 == STR_MTR_Gvar.FPGA.EncRev) STR_MTR_Gvar.AbsRom.SingleAbsPosFdb &= 0x007FFFFF;
        else STR_MTR_Gvar.AbsRom.SingleAbsPosFdb &= 0x000FFFFF; 

        AbsPosTempH = *AbsExtData;
        if(AbsPosTempH >= FunCodeUnion.code.PL_EncMultiTurnOffset)
        {
            AbsPosTempH = AbsPosTempH - FunCodeUnion.code.PL_EncMultiTurnOffset;
        }
        else
        {
            AbsPosTempH = (Uint32)AbsPosTempH + 65536L - (Uint32)FunCodeUnion.code.PL_EncMultiTurnOffset;
        }

        //串行编码器反馈多圈+单圈位置 
        if(0x800000 == STR_MTR_Gvar.FPGA.EncRev) AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 23) & 0xFFFFFFFFFF800000;
        else AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 20) & 0xFFFFFFFFFFF00000;

        AbsPosTemp = (int64)((Uint64)AbsPosTemp | (Uint64)STR_MTR_Gvar.AbsRom.SingleAbsPosFdb);

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
        HCAbsEncCommInit();
    }

    if(PrmRdFlag == 1)         //电机参数读取不成功时，置所有值为0xFFFF；
    {
        AbsRomAddr = 3;

        while(AbsRomAddr < ABS_ROM_WORDS_LEN)          
        {             
             *AbsRomDataAddr[AbsRomAddr] = 0xFFFF;
             AbsRomAddr ++;
        }

        PrmRdFlag = 0;      //清除标志位
    }
}

/*******************************************************************************
  函数名:  void AbsRom_SoftRstProcess(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 上电时需要读一下绝对式编码器中的电机参数,用以对FPGA进行参数配置
    2.
********************************************************************************/
Uint8 AbsRom_SoftRstProcess(void)
{
    static Uint8 Step = 0;       

    if((FunCodeUnion.code.MT_EncoderSel & 0xf0) != 0x10)    //非绝对式编码器
    {
        *TestData = 0xA5A5; 
        Step = 0;
        return 3;    
    }
       
    switch(Step)
    {
        case 0:
            FPGA_InitAbsEncCtrl(ABSENC_FORBID);    //先屏蔽编码器,避免编码器一直处于忙碌状态        
            *TestData = 0xA5A5; 
            Step = 1;
            return 2;

        case 1:
            if(0 == AbsRom_GetComStatus()) 
            {
                Step = 0;
                return 3;
            }
            else  
            {
                return 2; 
            } 

        default:
            Step = 0;
            return 2;
    }
}
    

/*******************************************************************************
  函数名:  void HCAbsRom_InitDeal(void);
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 上电时需要读一下绝对式编码器中的电机参数,用以对FPGA进行参数配置
    2.
********************************************************************************/
void HCAbsRom_InitDeal(void)
{
    Uint8 AbsRomAddr = 0;
    //Uint8 TimeDelay = 0;
    Uint16 TimeDelay = 0;  //by	huangxin201711_23
	Uint8 Temp = 0;

    FunCodeUnion.code.MT_EncoderSel = 0x13;

    //配置*ABSEncSel寄存器
    UNI_FPGA_AbsEncSel.bit.NikType = 0;
    UNI_FPGA_AbsEncSel.bit.AbsType = 1;
    UNI_FPGA_AbsEncSel.bit.BaudSelect = 0;
    *ABSEncSel = UNI_FPGA_AbsEncSel.all; 

    DELAY_US(100000L);    //延时100ms以等待绝对式编码器完成上电待机

    //汇川1代 2代编码器 上电时读取编码器存储参数操作是相同的
    AbsRomAddr = 0;
    while(AbsRomAddr < ABS_ROM_BYTES_LEN)
    {
        Temp = AbsRom_ReadByte(AbsRomAddr,0);
        if(1 == Temp) AbsRomAddr++;
        else if(2 == Temp) AbsRomAddr = ABS_ROM_BYTES_LEN + 1;

        DELAY_US(62);                    //虚拟中断周期调用的情况

        AbsErr_Reg.all      = *AbsEncErr;       //绝对式编码器故障状态 
        if(0 != (AbsErr_Reg.all & 0x0F))     //通讯错误（RX端）
        {
            //编码器断线 或 通讯故障
            PostErrMsg(CHABSROMERR1); 
            AbsRomAddr = ABS_ROM_BYTES_LEN + 1;
            PrmRdFlag = 1;        //参数读取不成功

            FunCodeUnion.code.MT_ABSEncVer = 65535;            //H00_04 编码器软件版本号
            FunCodeSaveInAbsRomInit(); 
            break;   
        }
    }

    FPGA_InitAbsEncCtrl(ABSENC_FORBID);

    //编码器通讯不正常
    if(AbsRomAddr != ABS_ROM_BYTES_LEN) return;  
   
    //编码器通讯正常
    if(ABSROM_CHECK_WORD != AbsRomData.all_16Bits[0])
    {
        PostErrMsg(CHABSROMERR);    //校验故障，电机ROM中未写入数据
        PrmRdFlag = 1;              //参数校验不成功

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

    //汇川1代编码器
    if(14000 == FunCodeUnion.code.MT_MotorModel)
    {
        //1代编码器
        FPGA_InitAbsEncCtrl(ABSENC_FORBID); 
        FunCodeUnion.code.MT_EncoderPensL = 0;      //H00_31 编码器线数L
        FunCodeUnion.code.MT_EncoderPensH = 0x10;   //H00_32 编码器线数H  
         
        UNI_FPGA_AbsEncSel.bit.BaudSelect = 0;
        *ABSEncSel = UNI_FPGA_AbsEncSel.all; 
        return;
    }

    //汇川2代单圈编码器 产品匹配 校验
    if(23 != (FunCodeUnion.code.MT_ABSEncVer / 1000))
    {
        
        FPGA_InitAbsEncCtrl(ABSENC_FORBID); 
        FunCodeUnion.code.MT_EncoderPensL = 0;      //H00_31 编码器线数L
        FunCodeUnion.code.MT_EncoderPensH = 0x10;   //H00_32 编码器线数H   
        PostErrMsg(HC2NDENCMATCHERR);               //产品匹配故障              
        return;                            
    } 

    //设置编码器线数
    FunCodeUnion.code.MT_EncoderPensL = 0;      //H00_31 编码器线数L
    FunCodeUnion.code.MT_EncoderPensH = 0x80;   //H00_32 编码器线数H

    //设置波特率 
    if(230 == (FunCodeUnion.code.MT_ABSEncVer / 100))
    {
        while (0 == AbsRom_WriteByte(0x01,0x66)) DELAY_US(62); 

        TimeDelay = 0;
        while((1 == AbsRom_GetComStatus()) && (TimeDelay < 100))   //等到通讯状态不忙时再恢复设置,只延时100us
        {
            DELAY_US(1);                    //延时
            TimeDelay++;
        }  
       
        FPGA_InitAbsEncCtrl(ABSENC_FORBID);

        UNI_FPGA_AbsEncSel.bit.BaudSelect = 1;
        *ABSEncSel = UNI_FPGA_AbsEncSel.all;
    }
    else if(231 == (FunCodeUnion.code.MT_ABSEncVer / 100))
    {
        while (0 == AbsRom_WriteByte(0x08,0x66)) DELAY_US(62); 

        TimeDelay = 0;
        while((1 == AbsRom_GetComStatus()) && (TimeDelay < 100))   //等到通讯状态不忙时再恢复设置,只延时100us
        {
            DELAY_US(1);                    //延时
            TimeDelay++;
        }  
       
        FPGA_InitAbsEncCtrl(ABSENC_FORBID);  

        UNI_FPGA_AbsEncSel.bit.BaudSelect = 2;
        *ABSEncSel = UNI_FPGA_AbsEncSel.all;       
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
void FunCodeSaveInAbsRomInit(void)
{
    FunCodeUnion.code.MT_RateVolt = 0;             //H00_09 额定电压
    FunCodeUnion.code.MT_RatePower = 75;            //H00_10 额定功率
    FunCodeUnion.code.MT_RateCurrent = 470;          //H00_11 额定电流
    FunCodeUnion.code.MT_RateToq = 239;              //H00_12 额定转矩
    FunCodeUnion.code.MT_MaxToqOrCur = 716;          //H00_13 最大转矩或最大电流
    FunCodeUnion.code.MT_RateSpd = 3000;              //H00_14 额定转速
    FunCodeUnion.code.MT_MaxSpd = 6000;               //H00_15 最大转速
    FunCodeUnion.code.MT_Inertia = 130;              //H00_16 转动惯量
    FunCodeUnion.code.MT_PolePair = 5;             //H00_17 永磁同步电机极对数
    FunCodeUnion.code.MT_StatResist = 500;           //H00_18 定子电阻
    FunCodeUnion.code.MT_StatInductQ = 327;          //H00_19 定子电感Lq
    FunCodeUnion.code.MT_StatInductD = 387;          //H00_20 定子电感Ld
    FunCodeUnion.code.MT_RevEleCoe = 3330;            //H00_21 反电势系数
    FunCodeUnion.code.MT_ToqCoe = 51;               //H00_22 转矩系数Kt
    FunCodeUnion.code.MT_EleConst = 654;             //H00_23 电气常数Te
    FunCodeUnion.code.MT_MachConst = 24;            //H00_24 机械常数Tm
    FunCodeUnion.code.MT_ThetaOffsetL = 8192;         //H00_28 绝对式码盘位置偏置L 
    FunCodeUnion.code.MT_ThetaOffsetH = 0;            //H00_29 绝对式码盘位置偏置H
    FunCodeUnion.code.MT_AbsRomMotorModel = 0;     //H00_35 总线电机型号
    FunCodeUnion.code.MT_AbsEncFunBit = 0;         //H00_37 绝对编码器功能设置位 
    FunCodeUnion.code.OEM_KedGain = 600;             //H01_22 D轴反电动势补偿系数
    FunCodeUnion.code.OEM_KeqGain = 1000;             //H01_23 Q轴反电动势补偿系数
    FunCodeUnion.code.OEM_CurIdKpSec = 1000;          //H01_24 D轴电流环比例增益
    FunCodeUnion.code.OEM_CurIdKiSec = 200;          //H01_25 D轴电流环积分补偿因子
    FunCodeUnion.code.OEM_CurIqKpSec = 1000;          //H01_27 Q轴电流环比例增益
    FunCodeUnion.code.OEM_CurIqKiSec = 100;          //H01_28 Q轴电流环积分补偿因子                          
    FunCodeUnion.code.OEM_CapIdKp = 2000;             //H01_52 性能优先模式D轴比例增益
    FunCodeUnion.code.OEM_CapIdKi = 200;             //H01_53 性能优先模式D轴积分补偿因子
    FunCodeUnion.code.OEM_CapIqKp = 2000;             //H01_54 性能优先模式Q轴比例增益
    FunCodeUnion.code.OEM_CapIqKi = 100;             //H01_55 性能优先模式Q轴积分补偿因子
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
Static_Inline void HCAbsEncCommInit(void)
{
    static Uint8 Step = 0;
    Uint32 AbsPosTempL = 0;
    int16 AbsPosTempH = 0;
    int64 AbsPosTemp = 0;
    int64 Temp = 0;

    if(14000 == FunCodeUnion.code.MT_MotorModel)
    {
        if(0 == Step) 
        { 
            if(0 == AbsRom_GetComStatus()) 
            {
                FPGA_InitAbsEncCtrl(ABSENC_SQNCRD);  //设置为连续读 
                EncFrameMode = 0; 
               *HostPosRst = 1;                                 
                Step = 1;
            }
        }
        else 
        {               
            if(Step >= 10)
            {
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;
                Step = 0; 
                *HostPosRst = 0; 
            }
            else
            {
                Step ++;
            }
        }
        return;    
    }
        
    if( ( (14101 == FunCodeUnion.code.MT_MotorModel) && (0 == STR_MTR_Gvar.AbsRom.AbsPosDetection) ) )     
    {  
        //未使能多圈绝对位置
        if(0 == Step) 
        {   
            if(0 == AbsRom_GetComStatus()) 
            {

                if(23 == (FunCodeUnion.code.MT_ABSEncVer / 1000))
                {
                    if(FunCodeUnion.code.EncFrameMode == 1)
                    {                    
                        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD_1);  //设置为连续读 1 
                        EncFrameMode = 1;                    
                    }
                    else if(FunCodeUnion.code.EncFrameMode == 2)
                    {
                        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD_2);  //设置为连续读2  
                        EncFrameMode = 2;                  
                    }
                    else
                    {
                        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD);  //设置为连续读
                        EncFrameMode = 0;
                    }
                }
                else
                {
                    FPGA_InitAbsEncCtrl(ABSENC_SQNCRD);  //设置为连续读
                    EncFrameMode = 0;
                }
    
                Step = 1;
                *HostPosRst = 1;
            }
        }
        else 
        {               
            if(Step >= 10)
            {
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;
                Step = 0;
                *HostPosRst = 0; 
            }
            else
            {
                Step ++;
            }
        }
        
        return;
    }          

    //使能多圈绝对位置功能
    EncFrameMode = 2;

    if(0 == AbsRom_GetComStatus()) 
    {
        if(0 == Step) 
        {   
            if(0 == AbsRom_GetComStatus())
            { 
                FPGA_InitAbsEncCtrl(ABSENC_SQNCRD_2);    //设置为连续读2 
                *HostPosRst = 1;               
                Step = 1;
            }
        }
        else
        {
            if(Step >= 10)
            {
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncCommInit = 1;
                Step = 0;
                *HostPosRst = 0; 

                //得到有效数据
                AbsPosTempL = (Uint32)((Uint32)AbsEncState_Reg.bit.AbsAngHigh << 16) + (Uint32)(*AbsAngLow);
                AbsPosTempL =  AbsPosTempL & 0x007FFFFF;

                AbsPosTempH = *AbsExtData;
                if(AbsPosTempH >= FunCodeUnion.code.PL_EncMultiTurnOffset)
                {
                    AbsPosTempH = AbsPosTempH - FunCodeUnion.code.PL_EncMultiTurnOffset;
                }
                else
                {
                    AbsPosTempH = (Uint32)AbsPosTempH + 65536L - (Uint32)FunCodeUnion.code.PL_EncMultiTurnOffset;
                }
                    
                AbsPosTemp = ((int64)(int32)(int16)AbsPosTempH << 23) & 0xFFFFFFFFFF800000;
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
                Step ++;
            }
        }
    } 
}
/*******************************************************************************
  函数名:  void AbsRomProcess(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. H0D04编码器ROM区读写操作函数,在转矩中断函数调用
    2.
********************************************************************************/
void AbsRomProcess(void)
{
    static Uint16 State = 0;
    static Uint8 AbsRomAddr = 0;
    static Uint8 DoneFlag = 0;
    static Uint32 DoneDelay = 0;
    Uint8 Temp = 0;

    //写完成后延时2s
    if((1 == DoneFlag) && (0 == AbsRom_GetComStatus()))
    {
        DoneDelay++;

        if(DoneDelay  > (2L * (Uint32)STR_MTR_Gvar.System.ToqFreq))
        {  
            AuxFunCodeUnion.code.MT_OperAbsROM = 0;            
            DoneFlag = 0;
            DoneDelay = 0;
            PostErrMsg(PCHGDWARN);
            SetAbsEncCtrl_SQNCRD();
        }
        return;
    }

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
                AbsRom_H0030 = 0x0013; 
                AbsRom_H0031 = 0x0000; 
                AbsRom_H0032 = 0x0010; 
                AbsRom_Null = 0;                    
               
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

            DoneFlag = 0;
            DoneDelay = 0;
            break;

        case 1: //读AbsRom中
            Temp = AbsRom_ReadByte(AbsRomAddr,0);

            if(1 == Temp) 
            {
                AbsRomAddr++;
            }
            else if(2 == Temp) 
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
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[AbsRomAddr],AbsRomAddr);
            if(1 == Temp)
            {
                AbsRomAddr++;
            }
            else if(2 == Temp) 
            {  
                State = 5;    //出错
            }

            if(AbsRomAddr >= 100) State = 4;    //完成
            break;

        case 4: //写，读AbsRom完成
        case 5:
            if(0 == AbsRom_GetComStatus())
            {
                if(State == 4)        //读写成功
                {
                    if(AuxFunCodeUnion.code.MT_OperAbsROM == 1)
                    {
                        DoneFlag = 1;                           
                        FPGA_InitAbsEncCtrl(ABSENC_FORBID);  
                    }
                    else
                    {
                        DoneFlag = 0;
                        AuxFunCodeUnion.code.MT_OperAbsROM = 0;
                        SetAbsEncCtrl_SQNCRD();
                    }
                }
                else       //读写失败提示
                {
                    DoneFlag = 0;
                    AuxFunCodeUnion.code.MT_OperAbsROM = 3;
                    SetAbsEncCtrl_SQNCRD();
                }

                State = 0;
                AbsRomAddr = 0; 
                DoneDelay = 0;               
            }
            break;
        default:
            break;
    }
}


/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_GetComStatus(void)
  输入:   无 
  输出:   0～空闲,1～通讯忙 
  子函数: 无
  描述：
    1. 得到绝对式编码器当前的通讯状态，0～空闲,1～通讯忙
    2.
********************************************************************************/
Static_Inline Uint8 AbsRom_GetComStatus(void)
{
    Uint16 DelayCnt = 0;      //延迟时间计算

    AbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态;

    //需要等到通信不忙的时候才进行写操作
    while((AbsEncState_Reg.bit.AbsComStatus == 1) && (DelayCnt < 10))
    {
        DelayCnt++;
        DELAY_US(1);
        AbsEncState_Reg.all = *AbsEncState;    //绝对式编码器运行状态
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
void CalcHCEncTransTime(void)
{
    int16   AbsTransferDelay = 0;    //绝对编码器数据传输延时

    int32 Ts = 0;          //电流环控制周期转换成的FPGA周期
    int32 Tmcu = 0;        //MCU转矩指令处理时间转换成的FPGA周期
    int32 Tspd = 0;        //速度计算时间转换成的FPGA周期
    int32 Tabsenc = 0;     //串行编码器总通讯时间转换成的FPGA周期 
    int32 Temp = 0;     
    int32 Temp2 = 0;
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

    if(14000 == FunCodeUnion.code.MT_MotorModel)
    {
        Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransDelay * 10) / FPGA_PRD;   //串行编码器总通讯时间，H0149 

        //编码器通讯波特率换算  2M 对应500ns
        STR_MTR_Gvar.AbsRom.AbsCommBaud = 500 / FPGA_PRD;

        CmdTransTime = (10L * 500L) / FPGA_PRD;
    }
    else if(23 != (FunCodeUnion.code.MT_ABSEncVer / 1000))
    {
        Tabsenc   = ((int32)(int16)FunCodeUnion.code.OEM_AbsTransDelay * 10) / FPGA_PRD;   //串行编码器总通讯时间，H0149 

        //编码器通讯波特率换算  2M 对应500ns
        STR_MTR_Gvar.AbsRom.AbsCommBaud = 500 / FPGA_PRD;

        CmdTransTime = (10L * 500L) / FPGA_PRD;
    } 
    else if((14101 == FunCodeUnion.code.MT_MotorModel) && (0 != STR_MTR_Gvar.AbsRom.AbsPosDetection))
    {
        Temp = (FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 12000;    //编码器计算时间10us + 2us余量

        if(230 == (FunCodeUnion.code.MT_ABSEncVer / 100))
        {
            //2.5M
            Temp += 400L * 80;  //8个数据传输时间

            //编码器通讯波特率换算  2.5M 对应400ns
            STR_MTR_Gvar.AbsRom.AbsCommBaud = 400 / FPGA_PRD;

            CmdTransTime = (10L * 400L) / FPGA_PRD;
        }
        else
        {
           //4M
            Temp += 250L * 80;  //8个数据传输时间

            //编码器通讯波特率换算  4M 对应250ns
            STR_MTR_Gvar.AbsRom.AbsCommBaud = 250 / FPGA_PRD;

            CmdTransTime = (10L * 250L) / FPGA_PRD;
        }

        Tabsenc   = Temp / FPGA_PRD;   
    }
    else 
    {
        Temp = (FunCodeUnion.code.OEM_AbsTransCompTime * 10) + 12000;    //编码器计算时间10us    + 2us余量

        if(EncFrameMode == 1)
        {
            Temp2 = 7;  //设置为连续读 1                     
        }
        else if(EncFrameMode == 2)
        {
            Temp2 = 8;  //设置为连续读2                    
        }
        else
        {
            Temp2 = 6;  //设置为连续读
        } 
       
        if(230 == (FunCodeUnion.code.MT_ABSEncVer / 100))
        {
            //2.5M
            Temp += Temp2 * 400L * 10L;  //8个数据传输时间

            //编码器通讯波特率换算  2.5M 对应400ns
            STR_MTR_Gvar.AbsRom.AbsCommBaud = 400 / FPGA_PRD;

            CmdTransTime = (10L * 400L) / FPGA_PRD;
        }
        else
        {
           //4M
            Temp += Temp2 * 250L * 10L;  //8个数据传输时间

            //编码器通讯波特率换算  4M 对应250ns
            STR_MTR_Gvar.AbsRom.AbsCommBaud = 250 / FPGA_PRD;

            CmdTransTime = (10L * 250L) / FPGA_PRD;
        }

        Tabsenc   = Temp / FPGA_PRD;    
    }

    STR_MTR_Gvar.AbsRom.AbsTransferTime = Tabsenc;

    //串行编码器的通讯启动延时参数，只做保存
    AbsTransferDelay = (Ts - Tspd - Tmcu  + Ts - Tabsenc) >> 1;
    //AbsCmdDelay = (Ts + Δ - tenc - tspd) / Tclk。

    if(AbsTransferDelay > (Ts >> 1))
    {
        AbsTransferDelay -= (Ts >> 1);
    } 

    //编码器数据传输延时设置,第一次上电配置时给一个初始值，避免由于初始化未计算完而配置成0
    AbsEncCtrl_Reg.bit.AbsTransferDelay = AbsTransferDelay;  


    /*说明：AbsCmdDelay = (Ts + Δ - tenc - tspd) / Tclk。其中Ts为同步周期（FPGA中断周期），
    tenc为编码器的总通讯时间，Δ为编码器命令字的传输时间（必要的话还要考虑线延迟），
    tspd为速度计算时间，Tclk为系统主时钟周期。参见620P的相关说明。
    Δ为编码器命令字的传输时间:
    尼康16位字长，（16+2）/波特率
    其它8位字长，（8+2）/波特率
    线延迟通过功能码H0166 设定 默认0 */ 
       
    Temp = (FunCodeUnion.code.OEM_EncCmdTransLineDelay * 10L) / FPGA_PRD;
    Temp += Ts + CmdTransTime - Tspd  - Tabsenc;
    *AbsCmdDelay = Temp;
}



/*******************************************************************************
  函数名:  Static_Inline void FPGA_InitAbsEncCtrl(Uint8 AbsMode)
  输入:   编码器工作模式设置 
  输出:   无
  子函数: 无
  描述：
    1. 绝对式编码器控制初始化
    2.
********************************************************************************/
Static_Inline void FPGA_InitAbsEncCtrl(Uint8 AbsMode)
{
    AbsEncCtrl_Reg.bit.AbsMode = AbsMode;   //编码器工作模式设置

    *AbsEncCtrl = AbsEncCtrl_Reg.all;
}

/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_ReadByteCmd(Uint8 ReadAddr)
  输入:   读地址
  输出:   1～读取命令发送成功   0～通信忙,未读取,继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 AbsRom_ReadByteCmd(Uint8 ReadAddr)
{
    UNI_MOTROMCTRL_REG MotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	static Uint16 MonitStep = 0;

	if(MonitStep == 0)
	{
        if(0 == AbsRom_GetComStatus())
        {
            MotRomCtrl_Reg.bit.EepromAddr   = ReadAddr;
            MotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = MotRomCtrl_Reg.all;

			MonitStep = 1;						 //下一次检测
		}
	}
	else
	{
        if(0 == AbsRom_GetComStatus())
        {
            MotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态

            if(MotRomState_Reg.bit.EepromBusy == 0)
            {
			    MonitStep = 0;
				return 1;
			}
		}
	}

    return 0;
}
/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_CheckByteCmd(Uint8 ReadAddr)
  输入:   读地址
  输出:   1～完成校验，将数据读出，0～未完成校验
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 AbsRom_CheckByteCmd(Uint8 ReadAddr)
{
    if(0 == AbsRom_GetComStatus())
    {
        MotRomState_Reg.all  = *MotRomState;      //EEPROM访问状态

        if(MotRomState_Reg.bit.EepromBusy == 0)
        {
            return 1;
        }
    }

    return 0;    
}
/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_ReadByte(Uint8 ReadAddr,Uint8 ExtDataSel)
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
Static_Inline Uint8 AbsRom_ReadByte(Uint8 ReadAddr,Uint8 ExtDataSel)
{
    static Uint16 TimeDelay = 0;
    static Uint8 ReadByteStep = STEPWAITSET;   //等待模式设置

    if((0 == ExtDataSel) && (ReadAddr > 99)) 
    {
        TimeDelay = 0;
        return 2;
    }

    if(ReadByteStep == STEPWAITSET)
    {
        //完成第1、2步
        FPGA_InitAbsEncCtrl(ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽避免FPGA一直处于忙碌状态

        if(0 == AbsRom_GetComStatus())
        {
            FPGA_InitAbsEncCtrl(ABSENC_RANDRDROM);      //设置绝对式编码器模式为随机读EEPROM

            ReadByteStep = STEPREADBYTE;
        }
    }
    else if(ReadByteStep == STEPREADBYTE)
    {
        //完成第3、4、5步
        if(1 == AbsRom_ReadByteCmd(ReadAddr))
        {
            ReadByteStep = STEPCHECKBYTE;     //进入校验阶段
        }
    }
    else if(ReadByteStep == STEPCHECKBYTE)
    {
        //完成第5、6步
        if(1 == AbsRom_CheckByteCmd(ReadAddr))
        {
            if(ReadAddr == MotRomState_Reg.bit.EepromAddr)
            {
                AbsRomData.all_8Bits[ReadAddr] = MotRomState_Reg.bit.ByteFromEeprom;
                ReadByteStep = STEPWAITSET;      //进入等待阶段
                TimeDelay = 0;
                return 1;           //无论读是否对错都结束读ROM状态
            }
            else
            {
                PostErrMsg(ENCDPMERR);     //参数校验错误
                ReadByteStep = STEPWAITSET;      //进入等待阶段
                TimeDelay = 0;
                return 2;           //无论读是否对错都结束读ROM状态
            }
        }
    }
      

    TimeDelay++;
    if((TimeDelay > 4000) || (0 != (AbsErr_Reg.all & 0x0F)))        //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);      //延时过长警告  Er.740
        ReadByteStep = STEPWAITSET;   //等待模式设置
        TimeDelay = 0;
        return 2;                    //不再进入读
    }

    return 0;
}
/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_WriteByteCmd(Uint16 WriteByte, Uint16 WriteAddr)
  输入:   读地址
  输出:   1～写入命令发送成功， 0～通信忙，无法写入，继续等待
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline Uint8 AbsRom_WriteByteCmd(Uint8 WriteByte, Uint8 WriteAddr)
{
	static Uint16 MonitStep = 0;
    UNI_MOTROMCTRL_REG MotRomCtrl_Reg = {0};     //用于向FPGA寄存器中写入的数据定义

	if(MonitStep == 0)		        //第一次进入时检测
	{
        if(0 == AbsRom_GetComStatus())
        {
            MotRomCtrl_Reg.bit.ByteToEeprom = WriteByte;
            MotRomCtrl_Reg.bit.EepromAddr   = WriteAddr;
            MotRomCtrl_Reg.bit.EepromMode = 1;

            *MotRomCtrl = MotRomCtrl_Reg.all;                //启动Rom操作
			MonitStep = 1;     						  //Rom操作完后进入下个周期判定
		}
	}							   //第二步操作
	else
	{
        if(0 == AbsRom_GetComStatus())
        {
            MotRomState_Reg.all  = *MotRomState;               //EEPROM访问状态

            if(MotRomState_Reg.bit.EepromBusy == 0)
            {
				MonitStep = 0;
				return 1;
			} 
		}
	}

    return 0;
}
/*******************************************************************************
  函数名:  Static_Inline Uint8 AbsRom_WriteByte(Uint8 WriteByte, Uint8 WriteAddr)
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
Static_Inline Uint8 AbsRom_WriteByte(Uint8 WriteByte, Uint8 WriteAddr)
{
    static Uint8 WriteByteStep = STEPWAITSET;      //等待模式设置
    static Uint16 TimeDelay = 0;                    //延时时间计数器
    static Uint16 WaitDelay = 0;                    //等待时间计数器

    if(WriteByteStep == STEPWAITSET)
    {
        //完成第1、2步
        FPGA_InitAbsEncCtrl(ABSENC_FORBID);       //进入随机读写时应先将编码器屏蔽再设置模式

        if(0 == AbsRom_GetComStatus())
        {
            FPGA_InitAbsEncCtrl(ABSENC_RANDWRTROM);      //设置绝对式编码器模式为随机写EEPROM

            WriteByteStep = STEPWRITEBYTE;
        }
    }
    else if(WriteByteStep == STEPWRITEBYTE)
    {
        //完成第3、4、5步
        if(1 == AbsRom_WriteByteCmd(WriteByte, WriteAddr))       //判定写命令是否发送完
        {
            WriteByteStep   = STEPWAITAFTERWRT;                 //写命令完成后进入到等待阶段
            WaitDelay = 0;
        }
    }
    else if(WriteByteStep == STEPWAITAFTERWRT)
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
            
            if(0x60 > WriteAddr)            
            {
                WriteByteStep = STEPREADBYTE;      //进入STEPREADBYTE;
            }
            else
            {
                WriteByteStep = STEPWAITSET;      //进入等待阶段
                return 1;           //无论读是否对错都结束读ROM状态               
            } 
        }
    }
    else if(WriteByteStep == STEPREADBYTE)
    {
        if(1 == AbsRom_ReadByte(WriteAddr,0))
        {
            if((WriteAddr != MotRomState_Reg.bit.EepromAddr) ||
               (WriteByte != MotRomState_Reg.bit.ByteFromEeprom))
            {
                PostErrMsg(ENCDPMERR);              //参数校验错误
                WriteByteStep = STEPWAITSET;        //进入等待阶段
                TimeDelay    = 0;
                return 2;           //无论读是否对错都结束读ROM状态
            }

            WriteByteStep = STEPWAITSET;      //进入等待阶段
            TimeDelay    = 0;
            return 1;           //无论读是否对错都结束读ROM状态        
        }
    }

    TimeDelay++;
    if((TimeDelay > 6000) || (0 != (AbsErr_Reg.all & 0x0F)))           //延迟时间过长
    {
        PostErrMsg(ENCDERR_Z7);       //延时过长警告 Er.740
        WriteByteStep = STEPWAITSET;      //进入等待阶段
        TimeDelay = 0;
        return 2;                       //结束写
    }

    return 0;
}

/*******************************************************************************
  函数名:   
  输入:   
  输出:   1～成功，0～不成功
  子函数: 无
  描述：

********************************************************************************/
Uint8 HCAbsEnc_SaveThetaOffset(void)
{
    static Uint8 Step = 0; 
    static Uint16 DelayT = 0; 
    Uint8 Temp = 0;

    Temp = 0;
    //将位置信息保存到相应功能码
    switch(Step)
    {
        case 0:
            AbsRomData.all_16Bits[26] = FunCodeUnion.code.MT_ThetaOffsetL;      //H00_28 绝对式码盘位置偏置L 
            AbsRomData.all_16Bits[27] = FunCodeUnion.code.MT_ThetaOffsetH;      //H00_29 绝对式码盘位置偏置H 
            Step = 1; 
            DelayT = 0;
            break;    
    
         case 1:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[52],52); 
            break;
               
         case 2:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[53],53);
            break;
                
         case 3:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[54],54);
            break;
               
         case 4:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[55],55);
            break;

         case 5:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[98],98);
            break;
               
         case 6:
            Temp = AbsRom_WriteByte(AbsRomData.all_8Bits[99],99);
            break;

         case 7:  
	    	if(DelayT < 16000)
            {
                FPGA_InitAbsEncCtrl(ABSENC_FORBID); 
                DelayT ++; 
                return 0;                                 
            }
            else
            {
                if(0 == AbsRom_GetComStatus())
                {
                    SetAbsEncCtrl_SQNCRD();    
                    Step = 0;
                    DelayT = 0; 
                    return 1;
                }
                return 0;
            }          
        default:
            Step = 0;
            break;    
    } 
  
    if(1 == Temp) 
    {
        Step++;
        DelayT = 0; 
    }          	
    else if(2 == Temp)
    {
        SetAbsEncCtrl_SQNCRD();   
        Step = 0;                                
        return 2;
    } 

    return 0;       
}


/*******************************************************************************
  函数名:   
  输入:   无                                                 
  输出:   
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Uint8 HC2ndAbsRom_ClcErrAndMultiTurn(Uint8 Mode)
{
	static Uint8 Step = 0;
	static Uint16 TimeDelay = 0;
    Uint8 Temp = 0; 

    switch(Step)
    { 
        case 0:
            if(23 != (FunCodeUnion.code.MT_ABSEncVer / 1000)) 
            {
                Step = 0;
                TimeDelay = 0; 
                return 0;
            }
            else
            {
                Step = Mode;   
                TimeDelay = 0;
            }
        
        case 1:
            //复位状态信息
            Temp = AbsRom_WriteByte(0x11,0x72);
            if(0 != Temp) Step = 3;            
            break;

        case 2:
            //多圈数据清零
            Temp = AbsRom_WriteByte(0x44,0x72);
            if(0 != Temp) Step = 3;
            break;

        case 3:
            TimeDelay ++;
            if(TimeDelay > 16000)
            {
                Step = 0;
                TimeDelay = 0; 
				if(Mode == 2)//by huangxin201803 _3 H0D20=1时只清除故障，不清除多圈数据H0544
	            {
				    FunCodeUnion.code.PL_EncMultiTurnOffset = 0;
	                SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));
				}
                SetAbsEncCtrl_SQNCRD();
                return 0;
            }
            break;            
        default:
            break;
    }

    return Mode;
} 
/*******************************************************************************
  函数名:   
  输入:   无                                                 
  输出:   
  子函数: 无
  描述：
    1. 
    2.
********************************************************************************/
Static_Inline void SetAbsEncCtrl_SQNCRD(void)
{
    if(EncFrameMode == 1)
    {
        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD_1);  //设置为连续读 1                  
    }
    else if(EncFrameMode == 2)
    {
        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD_2);  //设置为连续读2                  
    }
    else
    {
        FPGA_InitAbsEncCtrl(ABSENC_SQNCRD);  //设置为连续读
    }
}
void AbsRom_BaudSet_2MHZ(void)
{
	while (0 == AbsRom_WriteByte(0x0,0x66)) DELAY_US(62);
}
/********************************* END OF FILE *********************************/
