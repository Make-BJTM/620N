/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:  MTR_FPGAInterface.c                                                           
 创建人：姚虹                创建日期：2010.09.25
 修改人：朱祥华              修改日期：2011.11.13 
 描述：DSP和FPGA之间的相关操作
     1.在上电初始化的时候配置好FPGA的相应寄存器，使FPGA能正常工作并发出中断！
     2.在程序运行过程中对涉及到电流环计算的一些参数进行更改，也是通过设置FPGA的寄存器达到！
     3.在主中断程序中读取FPGA处理电流环时得到的数据，如Q轴反馈电流等并进行相应处理如报警等。
     4.其余的功能都是为了完成以上功能而做
     FPGA的角度基本都是以分辨率为基值，如*RotEnc_ZCnt（增量式编码器脉冲电角度）范围为0~10000
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "MTR_FPGAInterface.h" 
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h" 
#include "MTR_Global_Filter.h"
#include "PUB_Main.h"
#include "MTR_GPIODriver_ST.h"
#include "FUNC_Monitor_PhaseLack.h"
#include "MTR_AbsRomOper.h"
#include "MTR_Nikon_AbsRomOper.h"
#include "MTR_Tamagawa_AbsRomOper.h"
#include "MTR_HDH_RomOper.h"
#include "MTR_RDCOper.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//暂无

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义*/  
volatile STR_FPGA_CALCULATE_VAR  STR_FPGA_CalcVar;              //用于计算FPGA系数的数据结构体
volatile UNI_FPGA_INCENC_CTRL_REG   UNI_FPGA_IncEncCtrl = {0};     //③旋转编码器控制寄存器
volatile UNI_FPGA_ABSENC_SEL_REG   UNI_FPGA_AbsEncSel = {0}; 
volatile UNI_FPGA_ENC_TYPE_REG   UNI_FPGA_EncType    = {0};     //④编码器类型设置寄存器
volatile UNI_FPGA_ALARMCLR_REG   UNI_FPGA_AlarmClr   = {0};     //报警命令寄存器
volatile UNI_FPGA_ENC_DIV_REG    UNI_FPGA_EncDivCtrl = {0};     //⑤分频输出控制寄存器
volatile UNI_FPGA_POSCMD_REG     UNI_FPGA_PosCmd     = {0};     //②位置脉冲指令寄存器控制寄存器 

volatile union_LNRENCTRL2_REG       LnrEnCtrl2_Reg = {0};       //直线/全闭环外部编码器设置2

volatile UNI_FPGA_PERIODCTRL_REG  UNI_FPGA_PeriodCtrl = {0};     //速度启动延时及同步设置寄存器

//WZG1470  将UNI_FPGA_SYSCONFIG_REG  UNI_FPGA_SysConFig变为局部变量
//UNI_FPGA_SYSCONFIG_REG  UNI_FPGA_SysConFig  = {0};     //①FPGA硬件系统设置寄存器
//WZG1470  将UNI_FPGA_ENC_DIV_REG    UNI_FPGA_EncDivCtrl变为局部变量
//WZG1470  将UNI_FPGA_SYSSTATE_REG   UNI_FPGA_SysState变为局部变量
//UNI_FPGA_SYSSTATE_REG   UNI_FPGA_SysState   = {0};     //⑥系统状态寄存器
//wzg 移动到头文件中
//UNI_FPGA_SYSERR_REG     UNI_FPGA_SysErr     = {0};     //⑦系统故障状态寄存器

volatile UNI_FPGA_ZSVMINCTRL_REG  UNI_FPGA_ZsvMinCtrl = {0};     //最小零矢量设置寄存器 

//速度反馈低通滤波器
STR_MTR_NEW_LOWPASS_FILTER   NewSpdFdbLowpassFilter={0};
STR_MTR_NEW_LOWPASS_FILTER   NewSpdFdbLowpassFilter1={0};

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
//Uint16 FpgaSoftVersion;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
Uint16 FPGA_IsFPGARdy(void);            //上电初始化判断与FPGA读写是否正常
void InitFPGA(void);                    //FPGA上电配置初始化
void FPGA_UToVCoff(void);               //UV相电流的校正,使相电流采样值在通同一直流时能保持平衡
void SetFPGAParam(void);                //发送DSP相关数据至FPGA
void GetFPGAParam(void);                //得到FPGA中的状态参数    
void MainLoopStopUpdateFPGA(void);      //主循环更新FPGA相关控制参数
void MainLoopFPGAUpdate(void);          //主循环运行更新参数如PI电流环参数
void UpdateH0B_FPGA_State(void);        //H0B组FPGA状态信息更新

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//以下函数在InitFPGA()中调用
Static_Inline void FPGA_VerMatch(void);              //查看FPGA版本和ST版本之间是否匹配,
Static_Inline void FPGA_ClcAbsEncTime(void);         //计算串行编码器总通讯时间
Static_Inline void FPGA_InitPWM_AD_SYS_Config(void); //上电初始化PWM发波、电流采样、系统相关配置
Static_Inline void FPGA_InitPosCmd(void);            //上电初始化位置脉冲指令控制寄存器
Static_Inline void FPGA_InitEncCtrl_CalcVar(void);   //上电初始化旋转编码器、结构体STR_FPGA_CalcVar变量,如编码器线数，分频输出，电压等级和系数等
Static_Inline void FPGA_InitEncDivCtrl(void);        //上电初始化编码器分频输出控制 
Static_Inline void FPGA_InitCurSpdCtrlData(void);    //上电初始化电流、速度环相关控制参数
Static_Inline void FPGA_InitHostAng(void);           //得到上电后的电角度
//该函数在SetFPGAParam()初始化中调用一次
Static_Inline Uint16 FPGA_InitIfbOffset(void);       //函数返回值（0或1）判断是否完成电流偏置较正初始化
//运行中函数
Static_Inline void FPGA_SpdFbCal(void);              //得到速度反馈值，并根据功能码判断是否使用均值滤波
Static_Inline void M_SpdFdbCal(void);                //用于速度调节器积分部分的速度反馈运算
Static_Inline void FPGA_MechToElecAng(void);         //由机械角到电角度的转换
Static_Inline void FPGA_CtrlParaUpdate(void);        //更新FPGA控制传送至ST的参数
Static_Inline void FPGA_CurGainSwitch(void);         //电流环性能有限模式增益切换
// wzg20120225
Static_Inline void OvSpdFdbErrMonitor(void);         //速度反馈超速报错监控  

Static_Inline void RunCaseShortGnd(void);            //对地短路检测 20121216

Static_Inline void FPGA_InitLnrEncCtrl(void);         //全闭环接口寄存器初始化
/* FPGA地址分配---------------------------------------------------------*/
//注释部分是在别的文件中定义,在这里列出来是为了方便检索

// 控制参数，即需初始化的参数，写
volatile Uint16 *PWMPrd_Mode      = (Uint16 *)(FPGA_BASE + (0x00000000 << 1));   //PWM周期和模式；
volatile Uint16 *CurSampCtrl      = (Uint16 *)(FPGA_BASE + (0x00000001 << 1));   //AD采样启动延时设置1
volatile Uint16 *CurSampCtrlSec   = (Uint16 *)(FPGA_BASE + (0x00000002 << 1));   //AD采样启动延时设置2
volatile Uint16 *SpeedStartDelay  = (Uint16 *)(FPGA_BASE + (0x00000003 << 1));   //速度计算启动延时
volatile Uint16 *ZsvMinCtrl       = (Uint16 *)(FPGA_BASE + (0x00000005 << 1));   //最小零矢量控制
volatile Uint16 *DeadTmCtrl       = (Uint16 *)(FPGA_BASE + (0x00000006 << 1));   //IGBT死区时间
volatile Uint16 *DeadTmComp       = (Uint16 *)(FPGA_BASE + (0x00000007 << 1));   //死区补偿时间
volatile Uint16 *SysConfig        = (Uint16 *)(FPGA_BASE + (0x00000008 << 1));   //系统配置
//volatile Uint16 *PhslackConfig    = (Uint16 *)(FPGA_BASE + (0x00000009 << 1));   //缺相检测配置
volatile Uint16 *PosCmd           = (Uint16 *)(FPGA_BASE + (0x0000000A << 1));   //位置脉冲指令控制（低速脉冲命令，光耦隔离）
volatile Uint16 *PosCmd2          = (Uint16 *)(FPGA_BASE + (0x0000003D << 1));   //位置脉冲指令控制（高速脉冲命令，内部无隔离）
//volatile Uint16 *XintTrigConfig   = (Uint16 *)(FPGA_BASE + (0x0000000B << 1));   //中断定长触发配置
//volatile Uint16 *ExtDiFiltTm      = (Uint16 *)(FPGA_BASE + (0x0000000C << 1));   //外部Di滤波配置
volatile Uint16 *LnrEncCtrl       = (Uint16 *)(FPGA_BASE + (0x0000000D << 1));   //全闭环直线编码器控制
volatile Uint16 *LnrEncCtrl2      = (Uint16 *)(FPGA_BASE + (0x00000004 << 1));   //全闭环直线编码器控制2
volatile Uint16 *INCEncCtrl       = (Uint16 *)(FPGA_BASE + (0x00000010 << 1));   //旋转编码器控制
volatile Uint16 *ABSEncSel        = (Uint16 *)(FPGA_BASE + (0x00000010 << 1));   //绝对编码器选择
volatile Uint16 *RotEnc_MaxCnt    = (Uint16 *)(FPGA_BASE + (0x00000011 << 1));   //增量式编码器线数
volatile Uint16 *AbsCmdDelay      = (Uint16 *)(FPGA_BASE + (0x00000011 << 1));   //串行编码器命令延迟
volatile Uint16 *RotEnc_ZCnt      = (Uint16 *)(FPGA_BASE + (0x00000013 << 1));   //增量式编码器Z脉冲电角度
volatile Uint16 *AbsEncCtrl       = (Uint16 *)(FPGA_BASE + (0x00000014 << 1));   //绝对式编码器设置
volatile Uint16 *EncDivCtrl       = (Uint16 *)(FPGA_BASE + (0x00000015 << 1));   //编码器分频输出控制
volatile Uint16 *EncDiv_Num       = (Uint16 *)(FPGA_BASE + (0x00000016 << 1));   //编码器输出脉冲分频比之分子
volatile Uint16 *EncDiv_Den       = (Uint16 *)(FPGA_BASE + (0x00000017 << 1));   //编码器输出脉冲分频比之分母
volatile Uint16 *DivCntMaxHigh    = (Uint16 *)(FPGA_BASE + (0x00000016 << 1));   //绝对编码器分频输出脉冲数高4位
volatile Uint16 *DivCntMaxLow     = (Uint16 *)(FPGA_BASE + (0x00000017 << 1));   //绝对编码器分频输出脉冲数低16位
volatile Uint16 *ElecAngAcl       = (Uint16 *)(FPGA_BASE + (0x00000018 << 1));   //电角度定标因子
volatile Uint16 *CompAngScl       = (Uint16 *)(FPGA_BASE + (0x00000019 << 1));   //相角超前补偿因子
volatile Uint16 *SpdScl           = (Uint16 *)(FPGA_BASE + (0x0000001A << 1));   //速度定标因子
volatile Uint16 *MinSpd           = (Uint16 *)(FPGA_BASE + (0x0000001C << 1));   //最低可测量转速
volatile Uint16 *NilMtrSpdDly     = (Uint16 *)(FPGA_BASE + (0x0000001D << 1));   //电机零速检测延时
volatile Uint16 *SpdCmp           = (Uint16 *)(FPGA_BASE + (0x0000001E << 1));   //内部转速测量比较及切换门限
volatile int16  *IfbU_Offset      = (int16  *)(FPGA_BASE + (0x00000020 << 1));   //U相电流偏移量
volatile int16  *IfbV_Offset      = (int16  *)(FPGA_BASE + (0x00000021 << 1));   //V相电流偏移量
volatile Uint16 *IfbScl           = (Uint16 *)(FPGA_BASE + (0x00000022 << 1));   //电流定标因子
volatile int16  *ToqLimP          = (int16  *)(FPGA_BASE + (0x00000023 << 1));   //正向转矩限幅，有符号数
volatile int16  *ToqLimN          = (int16  *)(FPGA_BASE + (0x00000024 << 1));   //反向转矩限幅,有符号数
volatile Uint16 *IErrLim          = (Uint16 *)(FPGA_BASE + (0x00000025 << 1));   //电流环控制器积分误差限幅值
volatile Uint16 *IErrLimSec       = (Uint16 *)(FPGA_BASE + (0x0000000E << 1));   //第二组电流环控制器积分误差限幅值
volatile Uint16 *VdLim            = (Uint16 *)(FPGA_BASE + (0x00000026 << 1));   //D轴电压调制度限幅值
volatile Uint16 *VqLim            = (Uint16 *)(FPGA_BASE + (0x00000027 << 1));   //Q轴电压调制度限幅值
volatile Uint16 *Ked              = (Uint16 *)(FPGA_BASE + (0x00000028 << 1));   //D轴反电动势常数
volatile Uint16 *Keq              = (Uint16 *)(FPGA_BASE + (0x00000029 << 1));   //Q轴反电动势常数
volatile Uint16 *KpId             = (Uint16 *)(FPGA_BASE + (0x0000002A << 1));   //D轴电流环控制器比例增益
volatile Uint16 *KiId             = (Uint16 *)(FPGA_BASE + (0x0000002B << 1));   //D轴电流环控制器积分增益
volatile Uint16 *KpIq             = (Uint16 *)(FPGA_BASE + (0x0000002C << 1));   //Q轴电流环控制器比例增益
volatile Uint16 *KiIq             = (Uint16 *)(FPGA_BASE + (0x0000002D << 1));   //Q轴电流环控制器积分增益
volatile Uint16 *KpIdSec          = (Uint16 *)(FPGA_BASE + (0x0000000F << 1));   //D轴电流环控制器比例增益
volatile Uint16 *KiIdSec          = (Uint16 *)(FPGA_BASE + (0x0000001B << 1));   //D轴电流环控制器积分增益
volatile Uint16 *SyncLengthL      = (Uint16 *)(FPGA_BASE + (0x0000001D << 1));   //SYNC0周期测量值
volatile Uint16 *SyncLengthH      = (Uint16 *)(FPGA_BASE + (0x0000001E << 1));   //SYNC0周期测量值
volatile Uint16 *KpIqSec          = (Uint16 *)(FPGA_BASE + (0x0000001F << 1));   //Q轴电流环控制器比例增益
volatile Uint16 *KiIqSec          = (Uint16 *)(FPGA_BASE + (0x0000002E << 1));   //Q轴电流环控制器积分增益
volatile Uint16 *IfbCompGain      = (Uint16 *)(FPGA_BASE + (0x0000002F << 1));   //相电流平衡因子      20120607wzg1470


//运行命令，即运行中DSP提供给FPGA的参数，写
volatile Uint16 *HostSon          = (Uint16 *)(FPGA_BASE + (0x00000030 << 1));   //伺服ON/OFF
volatile Uint16 *HostPosRst       = (Uint16 *)(FPGA_BASE + (0x00000031 << 1));   //位置偏差计数器清零
volatile Uint16 *HostAlarmClr     = (Uint16 *)(FPGA_BASE + (0x00000032 << 1));   //报警清除命令
volatile int16  *HostToqRef       = (int16  *)(FPGA_BASE + (0x00000033 << 1));   //转矩数字给定命令
volatile int16  *HostIdRef        = (int16 *)(FPGA_BASE + (0x00000034 << 1));    //D轴电流给定命令   20120607wzg1470
volatile Uint16 *HostBusGain      = (Uint16 *)(FPGA_BASE + (0x00000036 << 1));   //母线电压补偿增益
volatile Uint16 *HostDutyU        = (Uint16 *)(FPGA_BASE + (0x00000037 << 1));   //主机给定的U相PWM占空比
volatile Uint16 *HostDutyV        = (Uint16 *)(FPGA_BASE + (0x00000038 << 1));   //主机给定的V相PWM占空比
volatile Uint16 *HostDutyW        = (Uint16 *)(FPGA_BASE + (0x00000039 << 1));   //主机给定的W相PWM占空比
volatile Uint16 *HostAng          = (Uint16 *)(FPGA_BASE + (0x0000003A << 1));   //转子初始电角度
volatile Uint16 *MotRomCtrl       = (Uint16 *)(FPGA_BASE + (0x0000003B << 1));   //绝对式编码器EEPROM访问控制
volatile Uint16 *MotRomCtrl2      = (Uint16 *)(FPGA_BASE + (0x0000003E << 1));   //绝对式编码器EEPROM访问控制2
volatile Uint16 *HostSysCtrl      = (Uint16 *)(FPGA_BASE + (0x0000003C << 1));   //系统控制


//状态参数表，即DSP从FPGA中读取的参数，读
volatile Uint16 *SysState         = (Uint16 *)(FPGA_BASE + (0x00000000 << 1));   //系统状态
volatile Uint16 *PosRefLow        = (Uint16 *)(FPGA_BASE + (0x00000001 << 1));   //位置命令低16位
volatile int16  *PosRefHigh       = (int16  *)(FPGA_BASE + (0x00000002 << 1));   //位置命令高16位
volatile Uint16 *PosRefLow2       = (Uint16 *)(FPGA_BASE + (0x00000017 << 1));   //位置命令低16位
volatile int16  *PosRefHigh2      = (int16  *)(FPGA_BASE + (0x00000018 << 1));   //位置命令高16位
volatile Uint16 *PosFbkLow        = (Uint16 *)(FPGA_BASE + (0x00000003 << 1));   //位置响应低16位
volatile int16  *PosFbkHigh       = (int16  *)(FPGA_BASE + (0x00000004 << 1));   //位置响应高16位
volatile int16  *VcmdRaw          = (int16  *)(FPGA_BASE + (0x00000005 << 1));   //模拟电压命令检测值
volatile int16  *MotorSpeed       = (int16  *)(FPGA_BASE + (0x00000006 << 1));   //速度反馈定标值，实际转速=(Nmax×MotorSpeed)>>15
volatile Uint16 *MechAng          = (Uint16 *)(FPGA_BASE + (0x00000007 << 1));   //机械角度
volatile int16  *Iu               = (int16  *)(FPGA_BASE + (0x00000008 << 1));   //U相电流检测值
volatile int16  *Iv               = (int16  *)(FPGA_BASE + (0x00000009 << 1));   //V相电流检测值
volatile int16  *Id               = (int16  *)(FPGA_BASE + (0x0000000A << 1));   //D轴电流响应
volatile int16  *Iq               = (int16  *)(FPGA_BASE + (0x0000000B << 1));   //Q轴电流响应
volatile int16  *Vd               = (int16  *)(FPGA_BASE + (0x0000000C << 1));   //D轴电压调制度
volatile int16  *Vq               = (int16  *)(FPGA_BASE + (0x0000000D << 1));   //Q轴电压调制度
volatile int16  *Va               = (int16  *)(FPGA_BASE + (0x0000000E << 1));   //Alafa轴电压调制度
volatile int16  *Vb               = (int16  *)(FPGA_BASE + (0x0000000F << 1));   //Beta轴电压调制度
volatile Uint16 *SysErr           = (Uint16 *)(FPGA_BASE + (0x00000010 << 1));   //系统故障状态
volatile Uint16 *TmFlt            = (Uint16 *)(FPGA_BASE + (0x00000012 << 1));   //超时故障状态
//volatile Uint16 *PhslackState     = (Uint16 *)(FPGA_BASE + (0x000000011 << 1));  //缺相检测状态
volatile Uint16 *PosLnrLow        = (Uint16 *)(FPGA_BASE + (0x00000013 << 1));   //全闭环编码器位置反馈低16位
volatile int16  *PosLnrHigh       = (int16 *)(FPGA_BASE + (0x00000014 << 1));   //全闭环编码器位置反馈高16位
//volatile Uint16 *PosBufferLow  = (Uint16 *)(FPGA_BASE + (0x00000015 << 1));  //中断定长反馈低16位
//volatile int16  *PosBufferHigh = (int16  *)(FPGA_BASE + (0x00000016 << 1));  //中断定长反馈高16位
volatile Uint16 *EcatSyncState    = (Uint16 *)(FPGA_BASE + (0x0000001F << 1));	 //Ecat同步状态
volatile Uint16 *AbsEncErr        = (Uint16 *)(FPGA_BASE + (0x00000020 << 1));   //绝对式编码器故障状态
volatile Uint16 *AbsAngLow        = (Uint16 *)(FPGA_BASE + (0x00000021 << 1));   //编码器绝对位置低16位
volatile Uint16 *AbsEncState      = (Uint16 *)(FPGA_BASE + (0x00000022 << 1));   //绝对式编码器运行状态
volatile Uint16 *AbsExtData       = (Uint16 *)(FPGA_BASE + (0x00000025 << 1));   //串行编码器扩展数据
volatile Uint16 *TAMAGAbsTurn     = (Uint16 *)(FPGA_BASE + (0x00000023 << 1));   //绝对式编码器多圈数据
volatile Uint16 *HDHAbsTurn       = (Uint16 *)(FPGA_BASE + (0x00000023 << 1));   //绝对式编码器多圈数据
volatile Uint16 *NKAbsTurn        = (Uint16 *)(FPGA_BASE + (0x00000023 << 1));   //NIKON编码器多圈数据
volatile Uint16 *MotRomState      = (Uint16 *)(FPGA_BASE + (0x0000003B << 1));   //绝对式编码器EEPROM访问状态
volatile Uint16 *MotRomState2     = (Uint16 *)(FPGA_BASE + (0x0000003C << 1));   //绝对式编码器EEPROM访问状态2
volatile Uint16 *FPGA_RevCode     = (Uint16 *)(FPGA_BASE + (0x00000030 << 1));   //FPGA版本号
volatile Uint16 *FPGA_NSRevCode   = (Uint16 *)(FPGA_BASE + (0x00000031 << 1));    //FPGA非标版本号

//测试用，读写
volatile Uint16 *TestData         = (Uint16 *)(FPGA_BASE + (0x0000003F << 1));

/**************************************************************************
                 InitFPGA(void)   上电时初始化与FPGA相关配置
**************************************************************************/

/*******************************************************************************
  函数名:  InitFPGA(void)     上电时初始化与FPGA相关配置    ^_^
  输入:    H00电机功能码 H01驱动器参数功能码 H0A组保护参数功能码
  输出:    一系列报错功能码
  子函数:  FPGA_VerMatch()上电查看FPGA版本和ST版本之间是否匹配
      FPGA_InitPosCmd()上电初始化位置脉冲指令控制寄存器  FPGA_InitPWM_AD_SYS_Config()上电初始化PWM发波电流采样系统相关配置
      FPGA_InitEncCtrl_CalcVar()上电初始化编码器分频输出 FPGA_InitEnc_CtrlVar()上电初始化编码器控制/结构体STR_FPGA_CalcVar变量
      FPGA_InitHostAng()得到上电初始电角度               FPGA_InitCurSpdCtrlData()上电初始化电流、速度环相关控制参数
    
  描述:  上电时初始化与FPGA相关配置包括：建立通讯连接，平台版本验证 更新控制参数 PWM发波电流采样系统相关配置
         位置脉冲指令控制寄存器初始化  编码器与分频输出初始化 电流速度闭环相关参数初始化 转子位置初始化
         调用该函数的地方：XXXXXXXXXXX
********************************************************************************/ 
void InitFPGA(void)
{
    FPGA_VerMatch();              //上电查看FPGA版本和ST版本之间是否匹配

    FPGA_ClcAbsEncTime();           //计算串行编码器总通讯时间

    FPGA_InitPWM_AD_SYS_Config(); //上电初始化PWM发波、电流采样、系统相关配置

    FPGA_InitPosCmd();            //上电初始化位置脉冲指令控制寄存器

    FPGA_InitEncCtrl_CalcVar();   //上电初始化编码器控制、结构体STR_FPGA_CalcVar变量如编码器线数、电压等级和系数等

    MainLoopStopUpdateFPGA();         //更新母线电压相关参数

    MainLoopFPGAUpdate();             //主循环运行更新参数如PI电流环参数

    FPGA_InitEncDivCtrl();        //上电初始化编码器分频输出控制

    FPGA_InitCurSpdCtrlData();    //上电初始化电流、速度环相关控制参数

    FPGA_InitHostAng();           //得到上电初始电角度

   //速度反馈低通滤波器初始化
    NewSpdFdbLowpassFilter.Fs = STR_MTR_Gvar.System.SpdFreq;
    NewSpdFdbLowpassFilter.Fc = FunCodeUnion.code.GN_SpdLpFiltFc;
    MTR_InitNewLowPassFilt(&NewSpdFdbLowpassFilter);

    NewSpdFdbLowpassFilter1.Fs = STR_MTR_Gvar.System.SpdFreq;
    NewSpdFdbLowpassFilter1.Fc = FunCodeUnion.code.GN_SpdLpFiltFc;
    MTR_InitNewLowPassFilt(&NewSpdFdbLowpassFilter1);

    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)   //转化成0.0001mm/s
    {
        STR_FPGA_CalcVar.M_SpdCoff = (((int64)STR_MTR_Gvar.System.SpdFreq * 10000L) << 10)
                                                / STR_MTR_Gvar.FPGA.EncRev;
    }
    else
    {
        //速度调节器积分部分所用所需速度反馈（M法测速）计算系数 单位为0.0001r/min
        STR_FPGA_CalcVar.M_SpdCoff = (((int64)STR_MTR_Gvar.System.SpdFreq * 10000L * 60L) << 10)
                                                / STR_MTR_Gvar.FPGA.EncRev;
    }

    STR_MTR_Gvar.FPGA.PosFdbAbsVal_ZInt = 0;

    STR_MTR_Gvar.GlobalFlag.bit.RevlDir = FunCodeUnion.code.BP_RevlDir; 

    STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn = FunCodeUnion.code.OEM_HighPrecisionAIEn; 
}

/*******************************************************************************
  函数名:  MainLoopStopUpdateFPGA(void)      主循环停机更新FPGA相关控制参数如母线电压增益调节系数   ^_^
  输入:  
  输出:    
  子函数:
    
  描述:  主循环更新FPGA相关控制参数如母线电压增益调节系数
        H0130母线电压增益系数应该是停机设定，立即生效，
        主要是为了配合生产工装测试泄放功能，不是停机设定，重新上电有效
********************************************************************************/ 
void MainLoopStopUpdateFPGA(void)
{
    //母线电压采样系数220V
    if(FunCodeUnion.code.OEM_VoltClass == 220)      //H01_04  OEM驱动器电压级
    {   
        STR_FPGA_CalcVar.DcBusVolt  =  DCVOLT220;
    }
    else if(FunCodeUnion.code.OEM_VoltClass == 380)
    {   
        STR_FPGA_CalcVar.DcBusVolt  =  DCVOLT380;
    }

    /*if(FunCodeUnion.code.PL_DivSourceSel == 1)  //1～脉冲同步输出
    {
        //高低速脉冲位置指令选择 0--低速，1--高速  
        UNI_FPGA_EncDivCtrl.bit.PosCmdMux = FunCodeUnion.code.PL_PulsePosCmdSel;   //脉冲指令源选择
        *EncDivCtrl = UNI_FPGA_EncDivCtrl.all;          //编码器分频输出控制    
    }
    
    //IS620N，后续非标修改
    if(15 > FunCodeUnion.code.OEM_FpgaVersion)   //旧版本
    {
        //高低速脉冲位置指令选择 0--低速，1--高速  与212不同处
        UNI_FPGA_PosCmd.bit.PosCmdMux = FunCodeUnion.code.PL_PulsePosCmdSel;
        *PosCmd = UNI_FPGA_PosCmd.all;           //得到最终的位置脉冲指令控制器配置
    }*/
}


/*******************************************************************************
  函数名:  MainLoopFPGAUpdate();          主循环实时更新FPGA相关控制参数   ^_^
  输入:  
  输出:    
  子函数:
    
  描述:  将实时性要求不高的FPGA读写操作移至主循环更新如D,Q轴反电动势系数、电流环PI参数
********************************************************************************/ 
void MainLoopFPGAUpdate(void)
{ 
    int32  Temp32_1 = 0;
    int32  Temp16 = 0;
    int32  Temp16_1 = 0; 

    //D,Q轴反电动势系数的写入
    *Ked = (Uint16)(((Uint64)STR_FPGA_CalcVar.Ked * FunCodeUnion.code.OEM_KedGain * PERTHOU_Q20) >> 20);  //D轴反电势补偿系数
    *Keq = (Uint16)(((Uint64)STR_FPGA_CalcVar.Keq * FunCodeUnion.code.OEM_KeqGain * PERTHOU_Q20) >> 20);  //Q轴反电势补偿系数

    //D轴电流环增益
    Temp16 = (STR_FPGA_CalcVar.IdKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CurIdKpSec) >> 16;      //H01_24 D轴电流环比例增益
    *KpIdSec = Temp16;

    Temp16_1 = (STR_FPGA_CalcVar.IdKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIdKp) >> 16;       //H01_52 性能优先模式D轴比例增益
    if(FunCodeUnion.code.GN_ServoCtrlMode < 2) *KpId = Temp16_1;                                   
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 2) *KpId = (Temp16 + Temp16_1) >> 1;
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 3) *KpId = Temp16 + ((Temp16_1 - Temp16) >> 2);

    //D轴电流环积分补偿因子
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CurIdKpSec 
           * (Uint64)FunCodeUnion.code.OEM_CurIdKiSec) / 100;                                       //H01_25 D轴电流环积分补偿因子
    *KiIdSec = Temp32_1 >> 16; 
    
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CapIdKp         //H01_53 性能优先模式D轴积分补偿因子
               * (Uint64)FunCodeUnion.code.OEM_CapIdKi) / 100;
    *KiId = Temp32_1 >> 16; 
    
        
    //Q轴电流环增益
    Temp16 = (STR_FPGA_CalcVar.IqKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CurIqKpSec) >> 16;;     //H01_27 Q轴电流环比例增益
    *KpIqSec = Temp16;

    Temp16_1 = (STR_FPGA_CalcVar.IqKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 16;       //H01_54 性能优先模式Q轴比例增益 
    if(FunCodeUnion.code.GN_ServoCtrlMode < 2) *KpIq = Temp16_1; 
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 2) *KpIq = (Temp16 + Temp16_1) >> 1;
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 3) *KpIq = Temp16 + ((Temp16_1 - Temp16) >> 2);

    //Q轴电流环积分补偿因子
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CurIqKpSec 
           * (Uint64)FunCodeUnion.code.OEM_CurIqKiSec) / 100;                                       //H01_28 Q轴电流环积分补偿系数  
    *KiIqSec = Temp32_1 >> 16;                      
    
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CapIqKp        //H01_55 Q轴电流环积分补偿系数 
           * (Uint64)FunCodeUnion.code.OEM_CapIqKi) / 100;    
    *KiIq = Temp32_1 >> 16; 

    //速度反馈低通滤波器初始化
    NewSpdFdbLowpassFilter.Fs = STR_MTR_Gvar.System.SpdFreq;
    NewSpdFdbLowpassFilter.Fc = FunCodeUnion.code.GN_SpdLpFiltFc;
    MTR_InitNewLowPassFilt(&NewSpdFdbLowpassFilter);

    NewSpdFdbLowpassFilter1.Fs = STR_MTR_Gvar.System.SpdFreq;
    NewSpdFdbLowpassFilter1.Fc = FunCodeUnion.code.GN_SpdLpFiltFc;
    MTR_InitNewLowPassFilt(&NewSpdFdbLowpassFilter1);

    if(UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)  //为光电式编码器
    {        
        //两个机械周期对应的脉冲数
        Temp32_1 = STR_FPGA_CalcVar.AngToCnt * (Uint32)FunCodeUnion.code.MT_PolePair * 2; 

        if(ABS(STR_MTR_Gvar.FPGA.PosFdbAbsVal_ZInt - STR_MTR_Gvar.FPGA.PosFdbAbsValue) >= Temp32_1)
        {
            STR_MTR_Gvar.FPGA.PosFdbAbsVal_ZInt = STR_MTR_Gvar.FPGA.PosFdbAbsValue;
            PostErrMsg(MULTIRERR);
        }
    }
	//过速报错警告速度值赋值,以减少中断时的计算量
    if(FunCodeUnion.code.ER_OvrSpdErr == 0)
    {
        //STR_FPGA_CalcVar.MaxSpdPos_OverWarn = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速警告速度值 
        //STR_FPGA_CalcVar.MaxSpdNeg_OverWarn = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverWarn;                    //反转过速警告速度值
        STR_FPGA_CalcVar.MaxSpdPos_OverErr  = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速报错速度值
        STR_FPGA_CalcVar.MaxSpdNeg_OverErr  = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverErr;                     //反转过速报错速度值 
        
    }
    else
    {
        //STR_FPGA_CalcVar.MaxSpdPos_OverWarn = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速警告速度值 
        //STR_FPGA_CalcVar.MaxSpdNeg_OverWarn = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverWarn;                    //反转过速警告速度值
        STR_FPGA_CalcVar.MaxSpdPos_OverErr  = 10000L * (int16)FunCodeUnion.code.ER_OvrSpdErr; //正转过速报错速度值
        
        Temp16 = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd;
		if(STR_FPGA_CalcVar.MaxSpdPos_OverErr>Temp16)//设置速度超过最大速度1.2倍，使用最高速度的1.2倍做为阈值
		{
		    STR_FPGA_CalcVar.MaxSpdPos_OverErr = Temp16 ;
		}
		
        STR_FPGA_CalcVar.MaxSpdNeg_OverErr  = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverErr;                     //反转过速报错速度值 
    
    }

    UpdateH0B_FPGA_State();
    
    //全闭环掉电检测使能
    if((FunCodeUnion.code.FC_FeedbackMode != 0) && (LnrEnCtrl2_Reg.bit.LnrFltEnbl != 1))
    {
        LnrEnCtrl2_Reg.bit.LnrFltEnbl = 1;
        *LnrEncCtrl2 = LnrEnCtrl2_Reg.all;     
    }

    if((FunCodeUnion.code.FC_FeedbackMode == 0) && (LnrEnCtrl2_Reg.bit.LnrFltEnbl != 0))
    {
        LnrEnCtrl2_Reg.bit.LnrFltEnbl = 0;
        *LnrEncCtrl2 = LnrEnCtrl2_Reg.all;     
    }
}

/**************************************************************************
              GetFPGAParam(void)     读取FPGA中的参数值至ST中
**************************************************************************/

/*******************************************************************************
  函数名:  GetFPGAParam(void)     读取FPGA中的参数值至ST中    ^_^
  输入:    STR_MTR_Gvar.FunCode.SpdFdbFilt_On  H08_22;速度滤波
  输出:    STR_MTR_Gvar.FPGA.SpdFdb电机转速              STR_MTR_Gvar.FPGA.MechaAngle机械角度（从原点开始的脉冲数）
STR_MTR_Gvar.FPGA.Etheta 输出 当前电机电角度值(0-32768)  DQ轴和三相电流  STR_MTR_Gvar.FPGA.IqFdb  IdFdb IuFdb IvFdb IwFdb;
gstr_Gvar.SpdDisp显示速度暂无滤波后续滤移至FUNC     STR_MTR_Gvar.FPGA.Ai3FPGAOut //AI3高精度采样
以下变量暂时没有在别的地方引用：alfa、beta、DQ轴电压给定 UalfaRef  UbetaRef UdRef UqRef 
STR_MTR_Gvar.FPGA.SysStatus  STR_MTR_Gvar.FPGA.SysErr FPGA运行状态和报错状态
  
  子函数:  FPGA_PostErr()       FPGA给出的报警信号处理
           FPGA_SpdFbCal()      速度反馈值
           FPGA_MechToElecAng() 机械角度到电角度的转换
           FPGA_CtrlParaUpdate()更新FPGA控制传送至ST的参数如电流反馈、电压给定、控制状态、绝对位置指令和反馈值、中断定长锁存脉冲值
  描述:  每次进FPGA中断执行，读取FPGA中的参数值至ST中
         更新FPGA给出的报警信号；更新计算速度反馈值；更新转换机械角度到电角度；
         更新从FPGA传送至ST的控制参数如电流反馈、电压给定、控制状态、绝对位置指令和反馈值、中断定长锁存脉冲值
         调用该函数的地方：XXXXXXXXXXX
********************************************************************************/ 
void GetFPGAParam(void)
{
    static int32 AiBuf[6] = {0};        //储存的AI3采样值缓冲区
    static int8  AiBufIndex = 6;        //AI3采样值的索引值
    int32  AiMax = 0;
    int32  AiMin = 0;
    int32  AiSum = 0;
     
    FPGA_CtrlParaUpdate();  //更新从FPGA传送至ST的控制参数如电流反馈、电压给定、控制状态、绝对位置指令和反馈值、中断定长锁存脉冲值

    FPGA_SpdFbCal();        //计算速度反馈值


    if(STR_MTR_Gvar.ScheldularFlag.bit.SpdFlg == 1 ) 
    {
        M_SpdFdbCal();   //用于速度调节器积分部分的速度反馈运算
    }

    //高精度AI
    if(0 != STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn)
    {
        if(AiBufIndex == 6)
        {
            if(1 == STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn) AiBuf[0] = 0 - (int32)*VcmdRaw;
            else if(2 == STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn) AiBuf[0] = (int32)((Uint16)32768 - (Uint16)*VcmdRaw);

            AiBuf[1] = AiBuf[0];
            AiBuf[2] = AiBuf[0];
            AiBuf[3] = AiBuf[0];
            AiBuf[4] = AiBuf[0];
            AiBuf[5] = AiBuf[0];
            AiBufIndex = 1;

            STR_MTR_Gvar.FPGA.HighPrecisionAI = AiBuf[0];
        }
        else
        {
            if(1 == STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn) AiBuf[AiBufIndex] = 0 - (int32)*VcmdRaw;
            else if(2 == STR_MTR_Gvar.GlobalFlag.bit.HighPrecisionAIEn)  AiBuf[AiBufIndex] = (int32)((Uint16)32768 - (Uint16)*VcmdRaw);

            //中值滤波没有使能时
            if(0 == FunCodeUnion.code.AI2_MidFltrEn)
            {
                STR_MTR_Gvar.FPGA.HighPrecisionAI = AiBuf[AiBufIndex];
            }

            AiBufIndex ++;
            if(AiBufIndex > 5) AiBufIndex = 0;

            if(AiBuf[0] > AiBuf[1])
            {
                AiMax = AiBuf[0];
                AiMin = AiBuf[1];
            }
            else
            {
                AiMax = AiBuf[1];
                AiMin = AiBuf[0];
            }

            if(AiMax < AiBuf[2]) AiMax = AiBuf[2];
            if(AiMax < AiBuf[3]) AiMax = AiBuf[3];
            if(AiMax < AiBuf[4]) AiMax = AiBuf[4];
            if(AiMax < AiBuf[5]) AiMax = AiBuf[5];

            if(AiMin > AiBuf[2]) AiMin = AiBuf[2];
            if(AiMin > AiBuf[3]) AiMin = AiBuf[3];
            if(AiMin > AiBuf[4]) AiMin = AiBuf[4];
            if(AiMin > AiBuf[5]) AiMin = AiBuf[5];

            AiSum = AiBuf[0] + AiBuf[1] + AiBuf[2] + AiBuf[3] + AiBuf[4] + AiBuf[5] - AiMax - AiMin;
            
            //中值滤波使能时
            if(1 == FunCodeUnion.code.AI2_MidFltrEn)
            {
                STR_MTR_Gvar.FPGA.HighPrecisionAI = (AiSum >> 2);
            }
        }
    }
}

/**************************************************************************
             SetFPGAParam(void)      发送DSP相关数据至FPGA
**************************************************************************/
/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  UV相电流的校正,使相电流采样值在通同一直流时能保持平衡
********************************************************************************/ 
void FPGA_UToVCoff(void)
{
    static Uint16 CompStep = 0;     //校正步骤
    static Uint16 TimeCnt = 0;      //延时计数器
    static int64  TotalIu = 0;      //Iu采样值的和
    static int64  TotalIv = 0;      //Iv采样值的和
    static Uint16 KedGain;          //保存的D轴补偿量
    static Uint16 KpIq;             //保存的Q轴Kp值
    static Uint16 KiIq;             //保存的Q轴Ki值


    STR_MTR_Gvar.GetIqRef.IdRef = STR_MTR_Gvar.GetIqRef.IqRate_MT;   //D轴给定额定电流

    if(CompStep == 0)       //保存需要改变的量
    {
        KedGain   = FunCodeUnion.code.OEM_KedGain;
        KpIq      = FunCodeUnion.code.OEM_CurIqKpSec;
        KiIq      = FunCodeUnion.code.OEM_CurIqKiSec;

        FunCodeUnion.code.OEM_KedGain         = 0;          //将D轴补偿，Q轴PI参数置0;
        FunCodeUnion.code.OEM_CurIqKpSec       = 0;
        FunCodeUnion.code.OEM_CurIqKiSec       = 0;  
         
        if((UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL) 
           || (UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL))  //增量式编码器不工作
        {
            UNI_FPGA_IncEncCtrl.bit.EncEnbl = 0;
            *INCEncCtrl  = UNI_FPGA_IncEncCtrl.all;     //旋转编码器控制
            *HostAng = 0;
        }

        CompStep = 1;
    }
    else if(CompStep == 1)
    {
        *HostSysCtrl = (0x01 | STR_MTR_Gvar.FPGA.SysCtrl);     //禁止W相PWM输出

        STR_MTR_Gvar.GetIqRef.IqRef = 0;

        TotalIu  = 0;
        TotalIv  = 0;
        CompStep = 2;

       UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
    }
    else if(CompStep == 2)       //延时2ms
    {
        TimeCnt ++;
        if(TimeCnt > 32)
        {
            TimeCnt  = 0;
            CompStep = 3;
        }
       UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
    }   
    else if(CompStep == 3)    // 得到采样值并进行计算
    {
        TotalIu += STR_MTR_Gvar.FPGA.IuFdb;
        TotalIv -= STR_MTR_Gvar.FPGA.IvFdb;
        TimeCnt ++;
        if(TimeCnt >= 4096)//连续采集4096次计算平均值
        {
            TimeCnt  = 0;
            CompStep = 4;
            TotalIu = TotalIu << 15;
            STR_MTR_Gvar.FPGA.UVAdjustResult = ABS((int64)TotalIu / TotalIv);
        }

        UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
    }
    else if(CompStep == 4)    //恢复参数
    {
        FunCodeUnion.code.OEM_KedGain   = KedGain;
        FunCodeUnion.code.OEM_CurIqKpSec = KpIq;
        FunCodeUnion.code.OEM_CurIqKiSec = KiIq;

        CompStep = 0;

        *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;
        UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
    }
}


/*******************************************************************************
  函数名:  SetFPGAParam(void)      发送DSP相关数据至FPGA  ^_^
  输入: 
  输出: 
  子函数: FPGA_InitIfbOffset(void)    电流偏置较正初始化
  描述:发送DSP相关数据至FPGA包括电流环初始化完成标志位；DI和H00d01警报清除；更新电压补偿系数、电流环DQ轴反电势补偿系数PI增益；
       母线电压采样结果进行运算Udc_Live为的实时母线电压值，单位对应0.1v
       IS500 220V时3V对应母线电压的最高电压500v，也对应ADC数字量4096  
       IS500_380V时3V对应母线电压的最高电压1000v，也对应ADC数字量4096
       IS550_380V借用变频器结构3.3V对应母线电压的最高电压1000v，AD采样数字量4096对应低压3V即母线电压909V 
       在XXXXXXXXX中调用该函数
********************************************************************************/ 
void SetFPGAParam(void)
{
    int32 IqRefTemp = 0;

    //从此处运行到*HostSon赋值,共需要214/120us 
    RunCaseShortGnd();

    //母线电压补偿
    *HostBusGain = (Uint16)((((Uint32)STR_FPGA_CalcVar.DcBusVolt << 14) * 10) / UNI_MTR_FUNCToMTR_List_16kHz.List.Udc_Live); 

    //H08_06 性能模式配置
    if(1 == FunCodeUnion.code.GN_ServoCtrlMode)
    {
        UNI_FPGA_ZsvMinCtrl.bit.ZsvMinEn = 1;
    }
    else
    {
        UNI_FPGA_ZsvMinCtrl.bit.ZsvMinEn = 0;
    }
    *ZsvMinCtrl     =  UNI_FPGA_ZsvMinCtrl.all;

    //性能模式下电流环PI参数的写入
    if(FunCodeUnion.code.GN_ServoCtrlMode == 1)
    {
        FPGA_CurGainSwitch();
    }

    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对转矩指令取反
    {
        IqRefTemp = - STR_MTR_Gvar.GetIqRef.IqRef;
    }
    else
    {
        IqRefTemp = STR_MTR_Gvar.GetIqRef.IqRef;
    }


    *HostToqRef = (int16)IqRefTemp;   //得到转矩指令；
    *HostIdRef  = (int16)STR_MTR_Gvar.GetIqRef.IdRef;

    *HostSon = UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus;

    //测试从中断开始到此处转矩指令赋值用的时间
    STR_PUB_Gvar.McuIqCalTime = GetSysTime_1MHzClk() - STR_PUB_Gvar.ToqIntStartTime;
    STR_PUB_Gvar.McuIqCalTime = STR_PUB_Gvar.McuIqCalTime & 0xFFFF;

    // 没报故障的情况, 到OvSpdFdbErrMonitor()结束完下运行时间 170/120 us
    FPGA_PostErr();         //FPGA给出的报警信号处理

    //进行故障复位
    UNI_FPGA_AlarmClr.bit.HostBrake = UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.BrkOut;   //泄放输出信号

    if(STR_PUB_Gvar.MTRAlmRst == 1)
    {
        UNI_FPGA_AlarmClr.bit.HostFltClr = 1;
        *HostAlarmClr = UNI_FPGA_AlarmClr.all;
        STR_PUB_Gvar.MTRAlmRst = 2;
        UNI_FPGA_AlarmClr.bit.HostFltClr = 1;
        *HostAlarmClr = UNI_FPGA_AlarmClr.all;
        UNI_FPGA_AlarmClr.bit.HostFltClr = 0;
        *HostAlarmClr = UNI_FPGA_AlarmClr.all;
    }
    else
    {
        UNI_FPGA_AlarmClr.bit.HostFltClr = 0;
        *HostAlarmClr = UNI_FPGA_AlarmClr.all;
    }

    FPGA_MechToElecAng();   //机械角度到电角度的转换

    OvSpdFdbErrMonitor();   //速度反馈超速报错监控

    //Soff时运行时间88/120us Son时运行时间26/120us
    if(STR_MTR_Gvar.GlobalFlag.bit.CurSampleRdy == 0)      //只有在初始化完成后才是电机电流检测正常！  
    {
        if(0 == FPGA_InitIfbOffset())                  //函数返回值(0~还需继续累加 1~累加完成)判断是否完成电流偏置较正初始化
        {
            STR_MTR_Gvar.GlobalFlag.bit.CurSampleRdy = 0; //电机电流检测准备好标志位
            return;                                   //没有进行U,V相电流校正时不能进入下面的设置
        } 
        else
        {
            STR_MTR_Gvar.GlobalFlag.bit.CurSampleRdy = 1; //电机电流检测准备好标志位
        }
    }
    else
    {
        FPGA_InitIfbOffset();            //在未使能状态下进行U,V相零偏校正   20120607wzg1470    
    }
}

/**************************************************************************
             begin of InitFPGA()文件内函数 共八个函数
**************************************************************************/

/*******************************************************************************
  ① 函数名:  FPGA_IsFPGARdy(void)        ^_^
  输入:  无
  输出:  返回值：FPGA是否准备好读写数据。
  子函数:无
  描述:  ①查询测试寄存器0x3F的内容，若在1000ms时间内每次读取的数据均不是0x0000，则意味着MCU与iASIC连接失败，
         系统存在硬件故障；但在此期间只要检测到该数据为0x0000，则执行下一步。
         ②向测试寄存器0x3F中写入某个特定数据，如0xAAAA，然后读取其值，若读取到的值与写入的完全一致，
         则表示MCU与iASIC之间的通讯连接已完全正常，可以执行后续操作；否则说明系统存在硬件故障。
         最后函数返回值为0 没准备好 为1准备好
         调用该函数的地方：InitFPGA()
********************************************************************************/ 
Uint16 FPGA_IsFPGARdy(void)
{
     Uint16 WriteData = 0x6666;  //写入的数据  
     Uint16 ReadData;            //读出的数值
     Uint16 RightCnt = 0;        //读写不匹配的报错数值
     int32  cnt = 0;
     int32  cnt1 = 0;

    ReadData = *TestData;

    //判断是否是软件复位
    if(ReadData == 0xA5A5)
    {
        //连续读50次都等于0xA5A5就认为是软件复位
        while(cnt <= 50L)
        {
            ReadData = *TestData;
            if(ReadData == 0xA5A5) cnt1++;
            if(cnt1 > 100) break;
            DELAY_US(1);
            cnt++;
        }

        if(cnt1 == cnt)
        {
            return 1;
        }
    }

     //读取的值不为0x0000则连续等待100000L次，
     //若100000L次后仍不正确则返回值0，并报错er102逻辑配置故障
    while(cnt <= 100000L)
    {
        ReadData  = *TestData;
        if(ReadData == 0x0000)
             break;
        DELAY_US(1);
        cnt++;                 //若返回值不为0则循环等待100000L次
    }
     //若等待100000L次未读到值则返回值0，并报错er102逻辑配置故障
    if(cnt >= 100000L)    
    {
        return 0;
    }

    //为确保无误，再连续读写50次成功才认为与FPGA通信无误。
    //tbg:允许一定时间的等待再判断是否是FPGA故障 tongwenzou @20110402
    cnt = 0;  
    while(cnt <= 100000L)//发生一次错误就再次读写50次
    {
        *TestData = WriteData;  //写入0x6666或其取反值
        DELAY_US(1);
        ReadData  = *TestData;  //读出之前写入值
        DELAY_US(1);

        if(ReadData != WriteData)  //判断是否发生错误并计算对或错的次数
        {
            RightCnt = 0;
        }
        else
        {
            RightCnt ++;
            WriteData = ~WriteData;
        }

        if(RightCnt > 50)          //连续对50次就退出
        {
            *TestData = 0;  //wzg

            ReadData = *SysState;
            if(0x0001 == (ReadData & 0x0001))
            {
                PostErrMsg(CTRLPOWERUDVDC1);
            }
            return 1;
        }
        DELAY_US(1);
        cnt++;
    }
    //100000L次内都没有一次连续50次读写成功则报错。
    return 0;
}

/*******************************************************************************
  ② 函数名:  FPGA_VerMatch(void)        ^_^
  输入: OEM_FpgaVersion H01_01 FPGA软件版本号
  输出: 报错er103
  子函数:无
  描述:  查看FPGA版本和ST版本之间是否匹配
         调用该函数的地方：InitFPGA()
********************************************************************************/ 
Static_Inline void FPGA_VerMatch(void)
{
    Uint32 Temp = 0;

    FunCodeUnion.code.OEM_FpgaVersion = *FPGA_RevCode;  //读取FPGA版本号H0101,并修改了MCU软件大版本号确定为06
    Temp = *FPGA_NSRevCode;
    Temp = Temp << 4;
    FunCodeUnion.code.MT_FpgaNonStandardVerL = (Uint16)Temp;
    FunCodeUnion.code.MT_FpgaNonStandardVerH = (Uint16)(Temp >> 16);

    //若ST与FPGA版本不匹配报Er102 
    if((FunCodeUnion.code.OEM_FpgaVersion / 1000) != 
       (FunCodeUnion.code.OEM_SoftVersion / 1000)) 
    {
        PostErrMsg(FPGAINITERR);            //FPGA和MCU产品型号不匹配 系统参数异常
        FunCodeUnion.code.ModbusRcvDeal = 0;
    }

    if(FunCodeUnion.code.OEM_FpgaVersion < 1120 ) PostErrMsg(VERSIONUNMATCH); 
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
Static_Inline void FPGA_ClcAbsEncTime(void)
{
    switch(FunCodeUnion.code.MT_EncoderSel)
    {
    #if HC_ENC_SW
        case 0x13:
            CalcHCEncTransTime();
        break;
    #endif

    #if NOKIN_ENC_SW
        case 0x12:
            CalcNikonEncTransTime();
        break;
    #endif

    #if TAMAGAWA_ENC_SW
        case 0x10:
            CalcTAMAGEncTransTime();
        break;
    #endif

    #if HDH_ENCODER_SW
        case 0x11:
            CalcHDHEncTransTime();
        break;
    #endif

        case 0x20:
            CalcRDCTransTime();
        break;
        default:
        break;
    }
}


/*******************************************************************************
  ③函数名:  FPGA_InitPWM_AD_SYS_Config(void)        ^_^
  输入: STR_MTR_Gvar.System.CarFreq     STR_MTR_Gvar.System.ToqFreq 
        STR_MTR_Gvar.FunCode.OEM_ADSamDly  AD采样延时 STR_MTR_Gvar.FunCode.OEM_FOCStartDly FOC启动延时H01_26
        STR_MTR_Gvar.FunCode.OEM_PwmDeadT死区时间H01_14
        STR_MTR_Gvar.FunCode.OEM_DeadComp 死区补偿时间H01-20  STR_MTR_Gvar.FunCode.ER_SdmFiltTm  Sigma_Delta滤波时间H0A22
        STR_MTR_Gvar.FunCode.ER_GkFiltTm  TZ信号滤波时间H0A23 占5bit
  输出: 
  子函数:无
  描述:  上电初始化PWM发波、电流采样、系统相关配置
          调用该函数的地方：InitFPGA()
********************************************************************************/ 
Static_Inline void FPGA_InitPWM_AD_SYS_Config(void)
{
    UNI_FPGA_COMPANG_REG     UNI_FPGA_CompAng    = {0};     //相角补偿因子设置寄存器
    UNI_FPGA_CURSAMPCTRL_REG UNI_FPGA_CurSampCtrl = {0};    //电流采样启动延时寄存器
//    UNI_FPGA_ZSVMINCTRL_REG  UNI_FPGA_ZsvMinCtrl = {0};     //最小零矢量设置寄存器 
    UNI_FPGA_SYSCONFIG_REG   UNI_FPGA_SysConFig  = {0};     //FPGA硬件系统设置寄存器

    Uint16 Temp16 = 0;
    int32  Ts = 0;        //电流环控制周期转换成的FPGA周期
    int32  Tdead = 0;     //死区时间转换成的FPGA周期
    int32  Tfoc = 0;      //FOC计算时间转换成的FPGA周期
    int32  Tmcu = 0;      //MCU转矩指令处理时间转换成的FPGA周期
//    int32  Tdly = 0;      //电流检测回路延时转换成的FPGA周期
    int32  Tabsenc = 0;   //串行编码器总通讯时间转换成的FPGA周期
    int32  Tsck = 0;      //Σ-Δ调制器时钟周期转换成的FPGA周期
    int32  Tspd = 0;      //速度计算时间转换成的FPGA周期
    int32  Tbaud = 0;     //串行编码器通信波特率时间转换成的FPGA周期

    //电流环控制周期
    Ts = (int32)((STR_MTR_Gvar.System.ToqPrd_Q10 * 1000) >> 10) / FPGA_PRD;
 
    //死区时间，H0114
    Tdead = (FunCodeUnion.code.OEM_PwmDeadT * 10) / FPGA_PRD;

    //FOC计算时间, H0131
    Tfoc = ((int32)(int16)FunCodeUnion.code.OEM_FocCalTime * 10) / FPGA_PRD;    

    //MCU计算的转矩指令处理时间，H0147
    Tmcu = ((int32)(int16)FunCodeUnion.code.OEM_IqCalTm * 10) / FPGA_PRD;

    //电流检测回路延时，H0148
//    Tdly = ((int32)(int16)FunCodeUnion.code.OEM_ADSamDly * 10) / FPGA_PRD;

    //串行编码器总通讯时间
    Tabsenc = STR_MTR_Gvar.AbsRom.AbsTransferTime;

    //Σ-Δ调制器时钟周期,100ns,对应AMC1203或HCPL-7860的时钟频率为10MHz
    Tsck = 100 / FPGA_PRD;

    //串行编码器通信波特率时间
    Tbaud = STR_MTR_Gvar.AbsRom.AbsCommBaud;

    //获取编码器型号
    UNI_FPGA_EncType.all    = FunCodeUnion.code.MT_EncoderSel & 0x0fff;


    if((UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL) 
       || (UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL))  //光电式编码器,直线光栅尺
    {
        Tspd = 2500 / FPGA_PRD;    //速度计算时间2.5us

        UNI_FPGA_CompAng.bit.CompAngScl_I = 0;      //电流相角超前补偿
    }
    else if((UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL) | (UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL))     //绝对式编码器
    {
        Tspd = 100 / FPGA_PRD;     //速度计算时间0.1us

        //电流相角超前补偿
        UNI_FPGA_CompAng.bit.CompAngScl_I = ((Tabsenc + Tspd + Tmcu - (10 * Tbaud)) << 6) / Ts;

    }

    Temp16  = (FPGA_CLK / STR_MTR_Gvar.System.CarFreq) - 1;
    STR_MTR_Gvar.FPGA.PWMPrdMode.bit.PwmPeriod = Temp16; 

    //PWM模式设置和相角补偿设置
    if(STR_MTR_Gvar.System.ToqFreq == STR_MTR_Gvar.System.CarFreq)     //电流环采样频率=IGBT开关频率
    {
        STR_MTR_Gvar.FPGA.PWMPrdMode.bit.PwmMode = 1;

        UNI_FPGA_CompAng.bit.CompAngScl_U = UNI_FPGA_CompAng.bit.CompAngScl_I + 16;    //电压相角超前补偿
    }
    else                                        //电流环采样频率=IGBT开关频率*2     双调制
    {
        STR_MTR_Gvar.FPGA.PWMPrdMode.bit.PwmMode = 0;

        UNI_FPGA_CompAng.bit.CompAngScl_U = UNI_FPGA_CompAng.bit.CompAngScl_I + 32; //电压相角超前补偿
    }

    *CompAngScl   = UNI_FPGA_CompAng.all;  

    //第一组电流采样启动延时，默认32抽取率
    Temp16                                    = 1 << (5 + FunCodeUnion.code.OEM_Decimation);
    UNI_FPGA_CurSampCtrl.bit.Decimation       = FunCodeUnion.code.OEM_Decimation;    
    UNI_FPGA_CurSampCtrl.bit.SampleStartDelay = Ts - (Tfoc >> 1) - ((3 * Temp16 * Tsck) >> 1);
    *CurSampCtrl = UNI_FPGA_CurSampCtrl.all;
    STR_MTR_Gvar.FPGA.CurSampCtrl             = UNI_FPGA_CurSampCtrl.all;     //赋值给全局变量


    //第二组电流采样启动延时，默认128抽取率
//    Temp16                                    = 1 << (5 + FunCodeUnion.code.OEM_SecDecimation);
//    UNI_FPGA_CurSampCtrl.bit.Decimation       = FunCodeUnion.code.OEM_SecDecimation;
//    UNI_FPGA_CurSampCtrl.bit.SampleStartDelay = Ts + Tdly - ((3 * Temp16 * Tsck) >> 1);
    *CurSampCtrlSec = UNI_FPGA_CurSampCtrl.all;

    //速度计算启动延时
    Temp16           = Ts - Tspd - Tmcu;
	UNI_FPGA_PeriodCtrl.bit.SpeedStartDelay = Temp16;
    InitPeriodCtrlReg();
    *SpeedStartDelay = UNI_FPGA_PeriodCtrl.all;

    //H0129最小零矢量时间，用以保证相电流在零矢量上采样，伺服采用性能优先模式时电机转速可能无法升到最高！
    UNI_FPGA_ZsvMinCtrl.bit.ZsvMinTm = (3 * (1<<(5+FunCodeUnion.code.OEM_Decimation)) * Tsck) + Tfoc + (2 * Tdead);
    if(1 == FunCodeUnion.code.GN_ServoCtrlMode)
    {
        UNI_FPGA_ZsvMinCtrl.bit.ZsvMinEn = 1;
    }
    else
    {
        UNI_FPGA_ZsvMinCtrl.bit.ZsvMinEn = 0;
    }
    *ZsvMinCtrl     =  UNI_FPGA_ZsvMinCtrl.all;

    //相电流平衡因子
    *IfbCompGain = FunCodeUnion.code.OEM_V2UCalCoff;

    //U V相过流保护门限
#if POWERDRIVER_TYPE==POWDRV_IS650
    Temp16 = ((Uint32)FunCodeUnion.code.OEM_OvrCurUV << 7) / 1000;
#else
    Temp16 = ((Uint32)FunCodeUnion.code.OEM_OvrCurUV << 7) / 320;
#endif

    //低12位：IGBT死区时间；  高4位：U V相过流保护门限低4位 
    STR_MTR_Gvar.FPGA.DeadTmCtrl = (Tdead  & 0x0FFF) | (Temp16 << 12) ;
    *DeadTmCtrl                  = STR_MTR_Gvar.FPGA.DeadTmCtrl;

    //低12位：死区补偿时间=周期*DeadTmComp,带小数点；  高4位：U V相过流保护门限高4位    
    STR_MTR_Gvar.FPGA.DeadTmComp = (((FunCodeUnion.code.OEM_DeadComp * 10) / FPGA_PRD) & 0x0FFF) | ((Temp16 & 0xf0) << 8);
    *DeadTmComp                  = STR_MTR_Gvar.FPGA.DeadTmComp;


    //系统配置初始化
    //注意：当620P控制板接500A的驱动板时，以下三个寄存器应该设置为零！！！
    //原因是620P驱动板相比500A驱动板在UV相电流采样及Σ-Δ调制器时间上
    //进行了硬件取反，因此程序也进行了取反设置。
    UNI_FPGA_SysConFig.bit.GateKillSns = 0; //bit2 IGBT过流保护有效电平 1～取反

    UNI_FPGA_SysConFig.bit.IfbUInv = 1;    //bit4 U相电流极性取反1～取反
    UNI_FPGA_SysConFig.bit.IfbVInv = 1;    //bit5 V相电流极性取反1～取反

    UNI_FPGA_SysConFig.bit.ValidEdge = 1;  //bit6 Σ-Δ调制器有效时钟边沿1～上升沿；
    UNI_FPGA_SysConFig.bit.BrakeSns = 0;   //bit7, 母线电压泄放驱动低电平有效  

#if POWERDRIVER_TYPE == POWDRV_IS650
    UNI_FPGA_SysConFig.bit.SdmClkEnable = FunCodeUnion.code.OEM_SdmClkEnbl;
#else
    UNI_FPGA_SysConFig.bit.StoSns   = 1;   //bit8, STO信号高电平有效
#endif

    UNI_FPGA_SysConFig.bit.SdmFiltTm = FunCodeUnion.code.ER_SdmFiltTm ;  //sigma——delta滤波时间，25ns*SdmFiltTm/3 
    UNI_FPGA_SysConFig.bit.GkFiltTm  = FunCodeUnion.code.ER_GkFiltTm;    //TZ滤波，单位25ns*GkFiltTm

    *SysConfig    = UNI_FPGA_SysConFig.all;  //系统配置
}

/*******************************************************************************
  ④ 函数名:  FPGA_InitPosCmd(void)        ^_^
  输入: STR_MTR_Gvar.FunCode.ER_PosRotFiltTm   //H0A24，输入脉冲和编码器滤波时间
        STR_MTR_Gvar.FunCode.PL_PosPulsInMode  //H0515 指令脉冲形态
  输出: *PosCmd
  子函数:无
  描述:  上电初始化位置脉冲指令控制寄存器配置
         调用该函数的地方：InitFPGA()
********************************************************************************/ 
Static_Inline void FPGA_InitPosCmd(void)
{
    UNI_FPGA_PosCmd.bit.PosCmdEnbl = 1;              //常使能      

    UNI_FPGA_PosCmd.bit.PosCmdEdge = 0;       //位置脉冲边沿选择，默认下降沿有效 

    if(FunCodeUnion.code.PL_PosPulsInMode == 2)      //脉冲模式为A+B相正交脉冲
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 1;          //4倍频
    }
    else if(FunCodeUnion.code.PL_PosPulsInMode == 3) //FPGA中都是Puls是脉冲且Sign为负时正向计数
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 2;          //CW+CCW,经测试与默认逻辑是反的    
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 1;
    }
    else if(FunCodeUnion.code.PL_PosPulsInMode == 0) //方向加脉冲  正逻辑 FPGA中SIGN负逻辑时Puls正向计数
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 0;
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 1;
    } 
    else   //  方向加脉冲  +  负逻辑
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 0;
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 0;
    }        
    
    UNI_FPGA_PosCmd.bit.PosCmdFiltTm = FunCodeUnion.code.ER_PosCmdFiltTmLow;  //脉冲滤波时间，单位25ns 
    *PosCmd = UNI_FPGA_PosCmd.all;     //低速脉冲命令
    
    if(FunCodeUnion.code.PL_PosPulsInMode == 2)      //脉冲模式为A+B相正交脉冲
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 1;          //4倍频
    }
    else if(FunCodeUnion.code.PL_PosPulsInMode == 3) //FPGA中都是Puls是脉冲且Sign为负时正向计数
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 2;          //CW+CCW,经测试与默认逻辑是反的    
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 1;
    }
    else if(FunCodeUnion.code.PL_PosPulsInMode == 0) //方向加脉冲  正逻辑 FPGA中SIGN负逻辑时Puls正向计数
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 0;
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 1;
    } 
    else   //  方向加脉冲  +  负逻辑
    {
        UNI_FPGA_PosCmd.bit.PosCmdMode = 0;
        UNI_FPGA_PosCmd.bit.PosCmdInv  = 0;
    }
            
    UNI_FPGA_PosCmd.bit.PosCmdFiltTm = FunCodeUnion.code.ER_PosCmdFiltTmHigh;  //脉冲滤波时间，单位25ns 
    *PosCmd2 = UNI_FPGA_PosCmd.all;     //高速脉冲命令         
}

/*******************************************************************************
  ⑤ 函数名:  FPGA_InitEncCtrl_CalcVar(void)        ^_^
  输入: STR_MTR_Gvar.FunCode.MT_EncoderSel编码器选择H00_30   STR_MTR_Gvar.FunCode.ER_PosRotFiltTm输入脉冲和编码器滤波时间H0A24
        STR_MTR_Gvar.FunCode.MT_EncoderPensH编码器线数高位H00_32       MT_EncoderPensL 编码器线数低位H00_31
        STR_MTR_Gvar.FunCode.PL_PosCodeDiv_H编码器分频脉冲数高位H0518  PL_PosCodeDiv_L 编码器分频脉冲数低位H0517
        STR_MTR_Gvar.FunCode.MT_PolePair 永磁同步电机极对数H00_17      MT_RateVolt 额定电压H00_09
        STR_MTR_Gvar.System.ToqFreq   转矩调度频率                     STR_MTR_Gvar.System.SpdFreq速度调度频率
  输出: 报错er103
  子函数:无
  描述:上电初始化编码器控制寄存器、结构体STR_FPGA_CalcVar变量如编码器线数，分频输出，电压等级和电压采样系数，
       Is500 三相220V时 外部采样3V对应母线电压的最高电压500v,单位0.1v   即ADC数字量4096对应实际母线电压5000（0.1V）
       IS500 三相380V时 3V对应1000V即ADC数字量4096对应实际母线电压10000（0.1V） #define DC380MAX_IS500 10000L      
       IS550借用变频器驱动 3.3V对应1000V  AD采样时数字量4096对应低压3V即母线电压909V 
       调用该函数的地方：InitFPGA()
********************************************************************************/
Static_Inline void FPGA_InitEncCtrl_CalcVar(void)
{
    int32 Temp16;

    if(UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)
    {
        //初始设置bit0 编码器使能 bit1 Z脉宽大于A（B）相周期1/3 bit2测速脉冲选择A相 bit3~4=00下降沿有效
        //bit5=0 A超前B bit6=1正极性 bit7=1省线式 bit8~15=00001000  2^8*50ns
        UNI_FPGA_IncEncCtrl.all = 0x10C3;  
        UNI_FPGA_IncEncCtrl.bit.RotEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm; //编码器滤波时间，单位25ns

        if(UNI_FPGA_EncType.bit.SMALLTYPE == COM_INC_ENCODER_SEL)       //非省线式
        {
            //初始设置bit0 编码器使能 bit1 Z脉宽大于A（B）相周期1/3 bit2测速脉冲选择A相 bit3~4=00下降沿有效
            //bit5=0 A超前B bit6=1正极性 bit7=1省线式 bit8~15=00001000  2^8*50ns
            UNI_FPGA_IncEncCtrl.all = 0x10C3;  
            UNI_FPGA_IncEncCtrl.bit.RotEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm; //编码器滤波时间，单位25ns

            UNI_FPGA_IncEncCtrl.bit.WireSaving = 0;
            STR_MTR_Gvar.FPGA.SysCtrl          = 0;     //不再利用Z相脉冲校准电角度

            if(UNI_FPGA_EncType.bit.SPECIAL == DNH)         //丹纳赫编码器
            {
                UNI_FPGA_IncEncCtrl.bit.RotIncZLen = 0;
            }
            else if(UNI_FPGA_EncType.bit.SPECIAL == DMC)   //多摩川编码器
            {
                UNI_FPGA_IncEncCtrl.bit.RotIncZLen = 1;
            }
    
            *INCEncCtrl    = UNI_FPGA_IncEncCtrl.all;    //旋转编码器控制
        }
        else if(UNI_FPGA_EncType.bit.SMALLTYPE == SAV_INC_ENCODER_SEL)  //省线式
        {
            //初始设置bit0 编码器使能 bit1 Z脉宽大于A（B）相周期1/3 bit2测速脉冲选择A相 bit3~4=00下降沿有效
            //bit5=0 A超前B bit6=1正极性 bit7=1省线式 bit8~15=00001000  2^8*50ns
            UNI_FPGA_IncEncCtrl.all = 0x10C3;  
            UNI_FPGA_IncEncCtrl.bit.RotEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm; //编码器滤波时间，单位            
            
            UNI_FPGA_IncEncCtrl.bit.WireSaving = 1;
            STR_MTR_Gvar.FPGA.SysCtrl          = 0;       //系统控制设置

            if(UNI_FPGA_EncType.bit.SPECIAL == DNH)         //丹纳赫编码器
            {
                UNI_FPGA_IncEncCtrl.bit.RotIncZLen = 0;
            }
            else if(UNI_FPGA_EncType.bit.SPECIAL == DMC)   //多摩川编码器
            {
                UNI_FPGA_IncEncCtrl.bit.RotIncZLen = 1;
            }
    
            *INCEncCtrl    = UNI_FPGA_IncEncCtrl.all;    //旋转编码器
        }
        else if(UNI_FPGA_EncType.bit.SMALLTYPE == ABZ_INC_ENCODER_SEL)  //only abz
        {
            UNI_FPGA_IncEncCtrl.all = 0x1003;

            UNI_FPGA_IncEncCtrl.bit.RotEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm;   //编码器滤波
            *INCEncCtrl    = UNI_FPGA_IncEncCtrl.all;    //旋转编码器
            STR_MTR_Gvar.FPGA.SysCtrl = 0x001C;      //系统控制屏蔽掉与Z信号相关的监控
        }
        else      //暂不支持其他类型编码器
        {
            PostErrMsg(ENCMATCHERR); 
        }  

        //编码器线数
        STR_MTR_Gvar.FPGA.EncRev = ((Uint32)FunCodeUnion.code.MT_EncoderPensH << 16) 
                              + FunCodeUnion.code.MT_EncoderPensL;
        //编码器分辨率为编码器线数的4倍
        STR_MTR_Gvar.FPGA.EncRev = STR_MTR_Gvar.FPGA.EncRev << 2;

        //若为光电式编码器则4倍频 若为绝对式则不需要4倍频 
        *RotEnc_MaxCnt = (Uint16)(STR_MTR_Gvar.FPGA.EncRev - 1);   //增量式编码器线数

        //除数为零处理
        if(FunCodeUnion.code.MT_PolePair == 0) FunCodeUnion.code.MT_PolePair = 1;

        //每个电周期对应的脉冲数
        STR_FPGA_CalcVar.AngToCnt  = (STR_MTR_Gvar.FPGA.EncRev / FunCodeUnion.code.MT_PolePair);

        //将以一个电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式 32768_Q16=(1<<31)
        STR_FPGA_CalcVar.CntToAng_Q16 = ((Uint32)1 << 31) / STR_FPGA_CalcVar.AngToCnt;

        //将初始实际电角度数换算成以4*PRR为基值的
        *RotEnc_ZCnt   = (Uint16)((FunCodeUnion.code.MT_InitTheta * STR_FPGA_CalcVar.AngToCnt) / 3600); //增量式编码器Z脉冲电角度
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)
    {
        UNI_FPGA_IncEncCtrl.all = 0x1043;

        UNI_FPGA_IncEncCtrl.bit.RotEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm;   //编码器滤波

        UNI_FPGA_IncEncCtrl.bit.RotEncZPol = 1;     //Z信号正极性
        UNI_FPGA_IncEncCtrl.bit.WireSaving = 0;     //无论是否接入UVW信号，ABZ中都不隐含UVW信号

        *INCEncCtrl = UNI_FPGA_IncEncCtrl.all;

        STR_MTR_Gvar.FPGA.SysCtrl = 0x0018;      //系统控制屏蔽掉与Z信号相关的监控


        //每个电周期对应的脉冲数
        STR_FPGA_CalcVar.AngToCnt = ((Uint32)FunCodeUnion.code.MT_EncoderPensL * 2 * 100000)
                                    / FunCodeUnion.code.MT_EncoderPensH;

        *RotEnc_MaxCnt = (Uint16)((STR_FPGA_CalcVar.AngToCnt * FunCodeUnion.code.MT_PolePair) - 1);

        *RotEnc_ZCnt   = (Uint16)((FunCodeUnion.code.MT_InitTheta * STR_FPGA_CalcVar.AngToCnt) / 3600); //增量式编码器Z脉冲电角度

        //每个脉冲对应的电角度值
        STR_FPGA_CalcVar.CntToAng_Q16 = ((Uint32)1L << 31) / STR_FPGA_CalcVar.AngToCnt;

        STR_MTR_Gvar.FPGA.EncRev = 100000 / FunCodeUnion.code.MT_EncoderPensH;     //用以表示1mm对应多少脉冲
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL)    //绝对式编码器
    {  
        STR_MTR_Gvar.FPGA.SysCtrl = 0x001C;      //系统控制屏蔽掉与Z信号相关的监控
        
        //编码器分辨率
        STR_MTR_Gvar.FPGA.EncRev = ((Uint32)FunCodeUnion.code.MT_EncoderPensH << 16) 
                              + FunCodeUnion.code.MT_EncoderPensL;

        //除数为零处理
        if(FunCodeUnion.code.MT_PolePair == 0) FunCodeUnion.code.MT_PolePair = 1;

        //每个电周期对应的脉冲数
        STR_FPGA_CalcVar.AngToCnt  = (65536L / FunCodeUnion.code.MT_PolePair); 
    
        //将以一个电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  
        //32768对应1个电周期的脉冲数 Q16格式 32768_Q16=(1<<31)
        STR_FPGA_CalcVar.CntToAng_Q16 = ((Uint32)1L << 31) / STR_FPGA_CalcVar.AngToCnt;
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL)    //旋变
    {
        STR_MTR_Gvar.FPGA.SysCtrl = 0x001C;      //系统控制屏蔽掉与Z信号相关的监控
        
        //编码器分辨率
        STR_MTR_Gvar.FPGA.EncRev = ((Uint32)FunCodeUnion.code.MT_EncoderPensH << 16) 
                              + FunCodeUnion.code.MT_EncoderPensL;

        //除数为零处理
        if(FunCodeUnion.code.MT_PolePair == 0) FunCodeUnion.code.MT_PolePair = 1;

        //每个电周期对应的脉冲数
        STR_FPGA_CalcVar.AngToCnt  = (65536L / FunCodeUnion.code.MT_PolePair); 
    
        //将以一个电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  
        //32768对应1个电周期的脉冲数 Q16格式 32768_Q16=(1<<31)
        STR_FPGA_CalcVar.CntToAng_Q16 = ((Uint32)1L << 31) / STR_FPGA_CalcVar.AngToCnt;
    }
    else  //暂不支持其它类型编码器
    {
        PostErrMsg(ENCMATCHERR);
    }
    *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;

    STR_MTR_Gvar.FPGA.AngToCnt    = STR_FPGA_CalcVar.AngToCnt;

    //过速报错警告速度值改为只在初始化赋值,以减少中断时的计算量

    if(FunCodeUnion.code.ER_OvrSpdErr == 0)
    {
        //STR_FPGA_CalcVar.MaxSpdPos_OverWarn = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速警告速度值 
        //STR_FPGA_CalcVar.MaxSpdNeg_OverWarn = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverWarn;                    //反转过速警告速度值
        STR_FPGA_CalcVar.MaxSpdPos_OverErr  = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速报错速度值
        STR_FPGA_CalcVar.MaxSpdNeg_OverErr  = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverErr;                     //反转过速报错速度值 
        
    }
    else
    {
        //STR_FPGA_CalcVar.MaxSpdPos_OverWarn = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd; //正转过速警告速度值 
        //STR_FPGA_CalcVar.MaxSpdNeg_OverWarn = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverWarn;                    //反转过速警告速度值
        STR_FPGA_CalcVar.MaxSpdPos_OverErr  = 10000L * (int16)FunCodeUnion.code.ER_OvrSpdErr; //正转过速报错速度值

        Temp16 = 12000L * (int16)FunCodeUnion.code.MT_MaxSpd;
		if(STR_FPGA_CalcVar.MaxSpdPos_OverErr>Temp16)//设置速度超过最大速度1.2倍，使用最高速度的1.2倍做为阈值
		{
		    STR_FPGA_CalcVar.MaxSpdPos_OverErr = Temp16 ;
		}
        STR_FPGA_CalcVar.MaxSpdNeg_OverErr  = (-1L)* STR_FPGA_CalcVar.MaxSpdPos_OverErr;                     //反转过速报错速度值 
    
    }
}

/*******************************************************************************
  ⑥ 函数名:  FPGA_InitRotEncCtrl(void)        ^_^
  输入: STR_MTR_Gvar.FunCode.MT_EncoderSel    //H00_30; 编码器选择
        STR_MTR_Gvar.FunCode.ER_PosRotFiltTm  //H0A24，输入脉冲和编码器滤波时间
  输出:  er110 分频脉冲输出设定故障
  子函数:无
  描述:  上电初始化编码器分频输出控制  
          调用该函数的地方：InitFPGA() 
********************************************************************************/ 
Static_Inline void FPGA_InitEncDivCtrl(void)
{
    Uint32 DivEncCnt;            //分频输出脉冲数

    UNI_FPGA_EncDivCtrl.all = 0;                   //分频输出寄存器初始化

    if(FunCodeUnion.code.PL_DivSourceSel == 0)  //0～编码器分频输出
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivMux = 0;    //编码器分频输出
        UNI_FPGA_EncDivCtrl.bit.EncDivEnbl = 1;   //分频输出使能
    }
    else if(FunCodeUnion.code.PL_DivSourceSel == 1)  //1～脉冲同步输出
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivMux = 1;   //脉冲同步输出
        UNI_FPGA_EncDivCtrl.bit.EncDivEnbl = 1;   //分频输出使能
    }
    else if(FunCodeUnion.code.PL_DivSourceSel == 2)  //2～分频输出禁止
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivMux = 0;   ////脉冲同步输出禁止
        UNI_FPGA_EncDivCtrl.bit.EncDivEnbl = 1;   //分频输出禁止
		FPGA_InitLnrEncCtrl();
    }

    if(FunCodeUnion.code.BP_PulsFdbDir == 1)   //根据H0203来确定分频输出方向
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivInv = 1;
    }
    else
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivInv = 0;
    }

    if(FunCodeUnion.code.PL_EncDivZPolSet == 0)
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivPol = 1;          //Z脉冲始终使用正极性,Z脉冲为高电平
    }
    else
    {
        UNI_FPGA_EncDivCtrl.bit.EncDivPol = 0;          //Z脉冲始终使用负极性,Z脉冲为低电平
    }

    UNI_FPGA_EncDivCtrl.bit.EncDivOCZInv = 0;       //输出OCZ脉冲的极性设置为1

    //高低速脉冲位置指令选择 0--低速，1--高速  
    UNI_FPGA_EncDivCtrl.bit.PosCmdMux = FunCodeUnion.code.PL_PulsePosCmdSel;   //脉冲指令源选择

    //STO信号滤波时间设置
    UNI_FPGA_EncDivCtrl.bit.StoFiltTm = (FunCodeUnion.code.ER_StoFiltTm * 10) / FPGA_PRD;

    *EncDivCtrl = UNI_FPGA_EncDivCtrl.all;          //编码器分频输出控制


    DivEncCnt = FunCodeUnion.code.PL_PosCodeDiv_L;                //分频输出脉冲数
    DivEncCnt = DivEncCnt << 2;

    if(UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)  //为光电式编码器
    {
        if(DivEncCnt > STR_MTR_Gvar.FPGA.EncRev)       //确保分子/分母 > 1,且设定值不超过范围
        {
            PostErrMsg(PULSOUTSETWARN);          //分频脉冲输出设定故障

           *EncDiv_Num = 1;    //编码器输出脉冲分频比之分子
           *EncDiv_Den = 1;    //编码器输出脉冲分频比之分母
 
        }
        else
        {
            *EncDiv_Num = STR_MTR_Gvar.FPGA.EncRev;          //编码器输出脉冲分频比之分子
            *EncDiv_Den = DivEncCnt; //编码器输出脉冲分频比之分母 
        }
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)     //直线光栅尺
    {
        if(DivEncCnt > STR_MTR_Gvar.FPGA.AngToCnt)  
                  //确保分子/分母 > 1,且设定值不超过范围
        {
            PostErrMsg(PULSOUTSETWARN);          //分频脉冲输出设定故障

           *EncDiv_Num = 1;    //编码器输出脉冲分频比之分子
           *EncDiv_Den = 1;    //编码器输出脉冲分频比之分母
 
        }
        else
        {
            *EncDiv_Num = STR_MTR_Gvar.FPGA.AngToCnt;          //编码器输出脉冲分频比之分子
            *EncDiv_Den = DivEncCnt; //编码器输出脉冲分频比之分母 
        }        
    }
    else if((UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL) | (UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL))     //绝对式编码器
    {
        //设定值不能超出范围
        if((DivEncCnt > STR_MTR_Gvar.FPGA.EncRev) || (DivEncCnt > 0x1FFFF))  
        {
            PostErrMsg(PULSOUTSETWARN);          //分频脉冲输出设定故障

            *DivCntMaxLow  = 0xFFFF;     //绝对编码器分频输出脉冲数
            *DivCntMaxHigh = 1;        //取分频输出的最高位
        }
        else
        {
            *DivCntMaxLow  = DivEncCnt & 0xFFFF;             //绝对编码器分频输出脉冲数
            *DivCntMaxHigh = (DivEncCnt >> 16) & 0x0001;     //取分频输出的最高高位
        }
    }
}

/*******************************************************************************
  ⑦ 函数名:  FPGA_InitRotEncCtrl(void)        ^_^
  输入: 
  输出: 
  子函数:无
  描述:  上电初始化电流、速度环相关控制参数    电角度、速度、电流定标
          调用该函数的地方：InitFPGA()
********************************************************************************/ 
Static_Inline void FPGA_InitCurSpdCtrlData(void)
{
    Uint16 Temp16   = 0;
    Uint16 Temp16_1 = 0;
    Uint32 Temp32_1 = 0;
    Uint32 Temp32_2 = 0;
    Uint64 Temp64_1 = 0;
    Uint64 Temp64_2 = 0;   //用于计算的中间变量

    if(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)
    {
        STR_FPGA_CalcVar.FPGA_MaxSpd = BLMTMAXSPD;
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)
    {
        STR_FPGA_CalcVar.FPGA_MaxSpd = 8192;       //避免配置及速度计算时产生截尾
    }
    else if((UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL) | (UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL))     //绝对式编码器
    {
        STR_FPGA_CalcVar.FPGA_MaxSpd = 8000;
    }

    if(UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)      //增量式编码器
    {
        Temp32_1      = ((Uint32)FunCodeUnion.code.MT_PolePair) << 25;  
        Temp32_2      = STR_MTR_Gvar.FPGA.EncRev;
        *ElecAngAcl   = (Uint16)(Temp32_1 / Temp32_2);   //电角度定标因子= (P*32768<<10)/(4*PPR)

        Temp64_1      = ((Uint64)FPGA_CLK * 60L) << 7;
        Temp64_2      = (Uint64)STR_MTR_Gvar.FPGA.EncRev * (Uint64)STR_FPGA_CalcVar.FPGA_MaxSpd;
        *SpdScl       = (Uint16)(Temp64_1 / Temp64_2);

        //MinSpd =(nmin<<15)/nmax，其中nmin为最低需求转速(r/min)，nmax为最高转速(r/min)
        Temp32_1      = (Uint32)FunCodeUnion.code.BP_MinMeasureSpd << 15; 
        Temp32_2      = (Uint32)STR_FPGA_CalcVar.FPGA_MaxSpd * 10;         
        *MinSpd       = (Uint16)(Temp32_1 / Temp32_2);    //最低可测量转速,带一个小数位

        //NilMtrSpdDly=60×速度采样频率/(4×PPR×nmin)，其中PPR为编码器线数，nmin为最低需求转速
        Temp32_1      = (Uint32)STR_MTR_Gvar.System.SpdFreq * 60 * 10;
        Temp32_2      = (Uint32)STR_MTR_Gvar.FPGA.EncRev * FunCodeUnion.code.BP_MinMeasureSpd;
        *NilMtrSpdDly = (Uint16)(Temp32_1 / Temp32_2) - 1;//电机零速检测延时 wzg1470

        Temp32_1          = ((Uint32)FunCodeUnion.code.BP_SpdCmp) << 15; 
        Temp32_2          = (Uint32)STR_FPGA_CalcVar.FPGA_MaxSpd;
        *SpdCmp           = (Uint16)(Temp32_1 / Temp32_2);//内部转速测量比较及切换门限
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)   //光栅编码器
    {
        Temp32_1      = ((Uint32)FunCodeUnion.code.MT_PolePair ) << 25;  
        Temp32_2      = ((Uint32)FunCodeUnion.code.MT_EncoderPensL * 1000 * 100 * 2) 
                          / FunCodeUnion.code.MT_EncoderPensH;
        *ElecAngAcl   = (Uint16)(Temp32_1 / Temp32_2);          //电角度定标因子= (P*32768<<10)/(rpr)

        Temp64_1      = ((Uint64)FunCodeUnion.code.MT_EncoderPensH * FPGA_CLK) << 7;  //跟直线电机光栅分辨率有关
        Temp64_2      = (Uint64)10000 * STR_FPGA_CalcVar.FPGA_MaxSpd;
        *SpdScl       = (Uint16)(Temp64_1 / Temp64_2);


        Temp32_1      = (Uint32)STR_MTR_Gvar.System.SpdFreq * FunCodeUnion.code.MT_EncoderPensH;
        Temp32_2      = (Uint32)10000 * FunCodeUnion.code.BP_MinMeasureSpd;
        *NilMtrSpdDly = (Uint16)(Temp32_1 / Temp32_2) - 1;//电机零速检测延时

        Temp64_1          = ((Uint64)FunCodeUnion.code.MT_EncoderPensH * 4 * STR_MTR_Gvar.System.SpdFreq) << 15; 
        Temp32_2          = (Uint32)10000 * STR_FPGA_CalcVar.FPGA_MaxSpd;
        *SpdCmp           = (Uint16)(Temp64_1 / Temp32_2);//内部转速测量比较及切换门限

    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL)    //绝对式编码器
    {
        Temp32_1      = ((Uint32)FunCodeUnion.code.MT_PolePair) << 25;  
        Temp32_2      = 1L << 16;
        *ElecAngAcl   = (Uint16)(Temp32_1 / Temp32_2);

        Temp64_1      = ((Uint64)STR_MTR_Gvar.System.ToqFreq * 60L) << 21;      //绝对编码器测速周期和电流环相关
        if(FunCodeUnion.code.MT_EncoderSel == 0x13)
        {
            Temp64_2      = (Uint64)1048576L * STR_FPGA_CalcVar.FPGA_MaxSpd; 
        }
        else
        {
            Temp64_2      = (Uint64)STR_MTR_Gvar.FPGA.EncRev * STR_FPGA_CalcVar.FPGA_MaxSpd;        
        }        
        *SpdScl       = (Uint16)(Temp64_1 / Temp64_2);
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL)    //旋变
    {
        Temp32_1      = ((Uint32)FunCodeUnion.code.MT_PolePair) << 25;  
        Temp32_2      = 1L << 16;
        *ElecAngAcl   = (Uint16)(Temp32_1 / Temp32_2);

        Temp64_1      = ((Uint64)STR_MTR_Gvar.System.ToqFreq * 60L) << 21;      //绝对编码器测速周期和电流环相关
        Temp64_2      = (Uint64)STR_MTR_Gvar.FPGA.EncRev * STR_FPGA_CalcVar.FPGA_MaxSpd;
        *SpdScl       = (Uint16)(Temp64_1 / Temp64_2);
    }

    //IfbScl=(IS_7860Gain×4096<<12)/(16384×SQRT(2)×IN×1.64676)，其中IS_7860Gain为电流传感器的最大理想量程，
    //IN为电机额定电流有效值。该因子可将额定电流映射到数值±4096，即当绕组电流峰值为SQRT(2)*IN时IQ值为4096
#if POWERDRIVER_TYPE==POWDRV_IS650
    Temp64_1      = ((Uint64)FunCodeUnion.code.IS_7860Gain * 1000000LL) << 20;  //利用电流采样最大量程(单位IS620:0.01A; IS650:0.1A)计算   
    Temp64_2      = (Uint64)FunCodeUnion.code.MT_RateCurrent * SQRT2_Q10 * 164676LL;
#else
    Temp64_1      = ((Uint64)FunCodeUnion.code.IS_7860Gain * 100000LL) << 20;  //利用电流采样最大量程(单位IS620:0.01A; IS650:0.1A)计算   
    Temp64_2      = (Uint64)FunCodeUnion.code.MT_RateCurrent * SQRT2_Q10 * 164676LL;
#endif       
    *IfbScl       = (Uint16)(Temp64_1 / Temp64_2); //电流定标因子 

    //老60Z电机需要特殊处理为3.6倍最大电流
    if( (14 == (FunCodeUnion.code.MT_MotorModel / 1000)) 
     && (FunCodeUnion.code.OEM_ServoSeri != 60003)
     && ( (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 703)
       || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 704)
       || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 804) ) )      
    {    	
    	*ToqLimP      = 14745;      //正向转矩限幅3.6
    	*ToqLimN      = -14745;     //反向转矩限幅
    }
    else if(( (FunCodeUnion.code.MT_MotorModel == 703)
          || (FunCodeUnion.code.MT_MotorModel == 704)
          || (FunCodeUnion.code.MT_MotorModel == 804) ) && (FunCodeUnion.code.OEM_ServoSeri != 60003))     
    {    	
    	*ToqLimP      = 14745;      //正向转矩限幅3.6
    	*ToqLimN      = -14745;     //反向转矩限幅
    }
	else
	{	
   	   *ToqLimP      = 12288;      //正向转矩限幅3
   	   *ToqLimN      = -12288;     //反向转矩限幅
	}

    *IErrLim      = 4096;       //第一组电流环控制器积分误差限幅值,FPGA内部给定值
    *IErrLimSec   = 2048;       //第二组电流环控制器积分误差限幅值,FPGA内部给定值
    *VdLim        = 6632;       //D轴电压调制度限幅值,以SVPWM设计时电压矢量最大幅值2/3Udc确定！
    *VqLim        = 6632;       //Q轴电压调制度限幅值 VdLim = (Q轴最大电压调制度<<14)/1.64676

    //Ked=8192×PI×P×nmax×Ls×SQRT(2)×IN/(15×1.64676×UN)，其中P为极对数，Ls为电机绕组电感，
    //IN为电机额定电流有效值，UN为额定直流母线电压
    Temp64_1      = (Uint64)FunCodeUnion.code.MT_PolePair * STR_FPGA_CalcVar.FPGA_MaxSpd; 
    Temp64_1      = Temp64_1 * PI_Q10 * SQRT2_Q10 * FunCodeUnion.code.MT_StatInductQ;
    Temp64_1      = (Temp64_1 * FunCodeUnion.code.MT_RateCurrent) >> 7;
    Temp64_2      = (Uint64)STR_FPGA_CalcVar.DcBusVolt * 15 * 164676 * 100;  
    Temp16        = (Uint16)(Temp64_1 / Temp64_2);                //D轴反电动势常数
    *Ked          = Temp16;
    STR_FPGA_CalcVar.Ked = Temp16;    //得到初始值

    //Keq=8192×PI×P×nmax×ψF/(15×1.64676×UN)，其中P为极对数，ψF为电机永磁转子磁链，UN为额定直流母线电压
    //其中ψF=SQRT(2)* Ve*60/(SQRT(3)*2PI*P)   Ve为线反电动势
    Temp64_1      = (Uint64)FunCodeUnion.code.MT_RevEleCoe;
    Temp64_1      = (Temp64_1 * 60 * STR_FPGA_CalcVar.FPGA_MaxSpd * SQRT6_Q10) << 2;
    Temp32_1      = (Uint32)STR_FPGA_CalcVar.DcBusVolt * 15 * 164676 * 3; 
    Temp16        = (Uint16)(Temp64_1 / Temp32_1);                //Q轴反电动势常数
    *Keq          = Temp16;     //得到初始值
    STR_FPGA_CalcVar.Keq = Temp16;

    //理论上，Kp=2^11×4×SQRT(2)×In×Ls×2π×电流环带宽/(1.64676×Un)，其中电流环带宽为设计指标；
    //但实际上该参数需通过实测，以达到最佳性能为准
    //2^11×4×SQRT(2)×In×Ls×2π/(1.64676×Un)
    Temp64_1      = (Uint64)FunCodeUnion.code.MT_RateCurrent * 4 * SQRT2_Q10 * 2* PI_Q10;
    Temp64_1      = (Temp64_1 * FunCodeUnion.code.MT_StatInductD) << 7;
    Temp64_2      = (Uint64)STR_FPGA_CalcVar.DcBusVolt * 164676L * 100L;  
    STR_FPGA_CalcVar.IdKpCoef_Q16 = Temp64_1 / Temp64_2;

    Temp64_1      = (Uint64)FunCodeUnion.code.MT_RateCurrent * 4 * SQRT2_Q10 * 2* PI_Q10;
    Temp64_1      = (Temp64_1 * FunCodeUnion.code.MT_StatInductQ) << 7;
    Temp64_2      = (Uint64)STR_FPGA_CalcVar.DcBusVolt * 164676L * 100L;  
    STR_FPGA_CalcVar.IqKpCoef_Q16 = Temp64_1 / Temp64_2;

    //说明：理论上，Ki=2^16×2×SQRT(2)×In×Rs×电流环采样时间×2π×电流环带宽/(1.64676×UN) 
    //其中电流环带宽为设计指标；但实际上该参数需通过实测，以达到最佳性能为准
    //2^16×2×SQRT(2)×In×Rs×电流环采样时间×2π/(1.64676×Un) 
    Temp64_1      = (Uint64)FunCodeUnion.code.MT_RateCurrent * 2 * SQRT2_Q10 * 2* PI_Q10;
    Temp64_1      = (Temp64_1 * FunCodeUnion.code.MT_StatResist * STR_MTR_Gvar.System.ToqPrd_Q10) << 2;
    Temp64_2      = (Uint64)STR_FPGA_CalcVar.DcBusVolt * 1646760L * 1000L * 100L;  
    STR_FPGA_CalcVar.KiCoef_Q16 = Temp64_1 / Temp64_2; 

    //D,Q轴反电动势系数的写入
    *Ked = (Uint16)(((Uint64)STR_FPGA_CalcVar.Ked * FunCodeUnion.code.OEM_KedGain * PERTHOU_Q20) >> 20);  //D轴反电势补偿系数
    *Keq = (Uint16)(((Uint64)STR_FPGA_CalcVar.Keq * FunCodeUnion.code.OEM_KeqGain * PERTHOU_Q20) >> 20);  //Q轴反电势补偿系数

    //D轴电流环增益
    Temp16 = (STR_FPGA_CalcVar.IdKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CurIdKpSec) >> 16;      //H01_24 D轴电流环比例增益
    *KpIdSec = Temp16;

    Temp16_1 = (STR_FPGA_CalcVar.IdKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIdKp) >> 16;       //H01_52 性能优先模式D轴比例增益
    if(FunCodeUnion.code.GN_ServoCtrlMode < 2) *KpId = Temp16_1;                                   
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 2) *KpId = (Temp16 + Temp16_1) >> 1;
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 3) *KpId = Temp16 + ((Temp16_1 - Temp16) >> 2);

    //D轴电流环积分补偿因子
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CurIdKpSec 
           * (Uint64)FunCodeUnion.code.OEM_CurIdKiSec) / 100;                                       //H01_25 D轴电流环积分补偿因子
    *KiIdSec = Temp32_1 >> 16; 
    
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CapIdKp     
           * (Uint64)FunCodeUnion.code.OEM_CapIdKi) / 100;                                          //H01_53 性能优先模式D轴积分补偿因子
    *KiId = Temp32_1 >> 16;
        
    //Q轴电流环增益
    Temp16 = (STR_FPGA_CalcVar.IqKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CurIqKpSec) >> 16;;     //H01_27 Q轴电流环比例增益
    *KpIqSec = Temp16;

    Temp16_1 = (STR_FPGA_CalcVar.IqKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 16;       //H01_54 性能优先模式Q轴比例增益 
    if(FunCodeUnion.code.GN_ServoCtrlMode < 2) *KpIq = Temp16_1; 
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 2) *KpIq = (Temp16 + Temp16_1) >> 1;
    else if(FunCodeUnion.code.GN_ServoCtrlMode == 3) *KpIq = Temp16 + ((Temp16_1 - Temp16) >> 2);

    //Q轴电流环积分补偿因子
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CurIqKpSec 
           * (Uint64)FunCodeUnion.code.OEM_CurIqKiSec) / 100;                                       //H01_28 Q轴电流环积分补偿因子  
    *KiIqSec = Temp32_1 >> 16; 
   
    Temp32_1 = ((Uint64)STR_FPGA_CalcVar.KiCoef_Q16 * (Uint64)FunCodeUnion.code.OEM_CapIqKp 
           * (Uint64)FunCodeUnion.code.OEM_CapIqKi) / 100;                                          //H01_55 性能优先模式Q轴积分补偿因子
    *KiIq = Temp32_1 >> 16;

    //将所有参数的配置数据均写入相应的寄存器地址中后，注意将U、V两相的电流偏移量（地址0x20和0x21）此时应设为0。
    //注：完成该项初始化操作后，iASIC进入激活状态，中断信号开始起作用。
    *IfbU_Offset  = 0;   //U相电流偏移量,上电给零，FPGA进入激活状态，可以发中断，然后读采样值计算再写
    *IfbV_Offset  = 0;   //V相电流偏移量，上电给零，FPGA进入激活状态，可以发中断，然后读采样值计算再写
}

/*******************************************************************************
  ⑧ 函数名:  FPGA_InitRotEncCtrl(void)        ^_^
  输入: FPGA系统状态寄存器
  输出: 非省线是FPGA初始角 、erA34编码器回送校验异常
  子函数:无
  描述:  得到上电初始电角度
         调用该函数的地方：InitFPGA()
********************************************************************************/ 
Static_Inline void FPGA_InitHostAng(void)
{
    UNI_FPGA_SYSSTATE_REG   UNI_FPGA_SysState   = {0};     //⑥系统状态寄存器
    Uint32 FPGA_UVW = 0;
    Uint32 cnt = 0;
    Uint16 Temp = 0;
    Uint32 UTheta = 0;         //U相上升沿对应的电角度
    Uint32 Temp32 = 0;

    UNI_FPGA_SysState.all = *SysState;
    

    while(UNI_FPGA_SysState.bit.PwrOnInitDone != 0x01)  //等待直到上电初始化进程结束
    {
        DELAY_US(1);
        UNI_FPGA_SysState.all = *SysState;     //读取*SysState直到FPGA准备完成状态位为1

        if(cnt++ > 1000000UL)    //1000000次后可退出,后面可以报警
        {
            break; 
        }
    }

    if(UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL)     //旋变
    {
        Temp32 = ((Uint32)FunCodeUnion.code.MT_InitTheta * 65536L) / FunCodeUnion.code.MT_PolePair; 
        *HostAng = (Uint16)((Uint32)Temp32 / 3600L);   

        STR_MTR_Gvar.FPGA.MechaAngle = *MechAng;             //机械角度0~65535
        Temp = STR_MTR_Gvar.FPGA.MechaAngle;

        //只要Temp大于AngToCnt则减AngToCnt(一个电周期对应脉冲数)
        while(Temp > STR_FPGA_CalcVar.AngToCnt)
        {
            Temp = Temp % STR_FPGA_CalcVar.AngToCnt;
        }

        //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
        STR_MTR_Gvar.FPGA.Etheta  = (Uint16)(((Uint64)Temp * STR_FPGA_CalcVar.CntToAng_Q16) >> 16);  //得到当前电角度值 
    }
    else if(UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL)     //绝对式编码器
    {
        *HostAng = FunCodeUnion.code.MT_ThetaOffsetL;

        STR_MTR_Gvar.FPGA.MechaAngle = *MechAng;             //机械角度0~65535
        Temp = STR_MTR_Gvar.FPGA.MechaAngle;

        //只要Temp大于AngToCnt则减AngToCnt(一个电周期对应脉冲数)
        while(Temp > STR_FPGA_CalcVar.AngToCnt)
        {
            Temp = Temp % STR_FPGA_CalcVar.AngToCnt;
        }

        //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
        STR_MTR_Gvar.FPGA.Etheta  = (Uint16)(((Uint64)Temp * STR_FPGA_CalcVar.CntToAng_Q16) >> 16);  //得到当前电角度值


    }
    else if((UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)
            || (UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL))  //为光电式编码器
    {
        UTheta = FunCodeUnion.code.MT_UposedgeTheta / 10;
    
        //初始角写入状态为1,表明写过初始角,用于软件复位
        if(UNI_FPGA_SysState.bit.AngInitDone == 0x01) 
        {
            STR_MTR_Gvar.FPGA.MechaAngle = *MechAng;             //机械角度0~4*PPR
            Temp = STR_MTR_Gvar.FPGA.MechaAngle;

            //只要Temp大于AngToCnt则减AngToCnt(一个电周期对应脉冲数)
            while(Temp > STR_FPGA_CalcVar.AngToCnt)
            {
                Temp = Temp % STR_FPGA_CalcVar.AngToCnt;
            }
        
            //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
            STR_MTR_Gvar.FPGA.Etheta  = (Uint16)(((Uint64)Temp * STR_FPGA_CalcVar.CntToAng_Q16) >> 16);  //得到当前电角度值
    
            STR_FPGA_CalcVar.OldElecAng = STR_MTR_Gvar.FPGA.Etheta;
        }
        else
        {
            STR_MTR_Gvar.FPGA.MechaAngle = 0; //机械角度
            
            switch(UNI_FPGA_SysState.bit.EncUvw)  //得到UVW的初始状态进而确认初始角，U上升沿->180度
            {
                case 2:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_30 + UTheta + 180) % 360)) / 360;
                    break;
                case 3:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_90 + UTheta + 180) % 360)) / 360;
                    break;
                case 1:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_150 + UTheta + 180) % 360)) / 360;
                    break;
                case 5:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_210 + UTheta + 180) % 360)) / 360;
                    break;
                case 4:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_270 + UTheta + 180) % 360)) / 360;
                    break;
                case 6:
                    FPGA_UVW = (STR_FPGA_CalcVar.AngToCnt * ((THETA_330 + UTheta + 180) % 360)) / 360;
                    break;
                default:
                    STR_FPGA_CalcVar.EnFbChkErr = 1;       //编码器回送校验异常
                    if(((UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)&&
                         (UNI_FPGA_EncType.bit.SMALLTYPE != ABZ_INC_ENCODER_SEL)) 
                       ||((UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)&&
                         (UNI_FPGA_EncType.bit.SMALLTYPE == SAV_INC_ENCODER_SEL)))
                    {
                        PostErrMsg(ENFBCHKERR);         //相位读取错误；
                    }
                    break;           
            }

            //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
            //得到初始电角度，用以判定是否UVW不正常
            STR_MTR_Gvar.FPGA.Etheta = (Uint16)((STR_FPGA_CalcVar.CntToAng_Q16 * FPGA_UVW) >> 16);
            STR_FPGA_CalcVar.OldElecAng = STR_MTR_Gvar.FPGA.Etheta; 
            *HostAng             = (Uint16)FPGA_UVW; 

        }
    }
}
/**************************************************************************
             end of InitFPGA()文件内函数 共八个函数
**************************************************************************/


/*******************************************************************************
  函数名:  FPGA_InitIfbOffset(void)    电流偏置较正初始化    ^_^
  输入: FPGA系统状态寄存器
  输出: ①UV相偏置：*IfbU_Offset *IfbV_Offset
        ②函数返回值（0或1）判断是否完成电流偏置较正初始化
  子函数:无
  描述:  上电计算得到U，V相电流的偏移量,在累加U,V相电流采样值时，必须是每个中断采集一次; 
         前10次电流环中断进入电流采样作废，然后累加初始电流1024次取平均值作为偏移量。
         函数返回值（ 0～还需继续累加  1～累加完成）判断是否完成电流偏置较正初始化
         在SetFPGAParam(void)（发送DPS数据至FPGA）中调用该函数
********************************************************************************/ 
Static_Inline Uint16 FPGA_InitIfbOffset(void)
{
    static Uint16 IfbCnt  = 0;     //用于计算累加次数
    static int32  Temp32_1 = 0;
    static int32  Temp32_2 = 0;    //用于计算的中间变量,累加采到的电流值
    static int16  OffsetU = 0;
    static int16  OffsetV = 0;

    if((AuxFunCodeUnion.code.OEM_ResetABSTheta == 1)
       && ((FunCodeUnion.code.ER_AngIntSel == 2)||(FunCodeUnion.code.ER_AngIntSel == 3)))    //电压注入时不再纠偏
    {
        return 0;
    }

    if((UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == 0)||
      (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == 1) )    //伺服使能时不进行零偏校正
    {
        if(IfbCnt < 30)                //前2个数据FPGA也不确定Iu，Iv会发出什么值，把前30个丢掉
        {
            IfbCnt++;
            Temp32_1 = 0;
            Temp32_2 = 0;      
        }
        else if(IfbCnt < 4126)        //连续采集4096个Iu,Iv的数值进行计算,注意*Iu,*Iv都是较偏后的采样值
        {
            Temp32_1 += (*Iu) + OffsetU;
            Temp32_2 += (*Iv) + OffsetV;
            IfbCnt++;
        }
        else if(IfbCnt == 4126)
        {
            OffsetU      = (int16)(Temp32_1 >> 12);
            OffsetV      = (int16)(Temp32_2 >> 12);
            *IfbU_Offset = OffsetU;
            *IfbV_Offset = OffsetV;
            IfbCnt = 0;

            return 1;
        }
        
    }
    else
    {
        IfbCnt   = 0;
        Temp32_1 = 0;
        Temp32_2 = 0;
    }

    return 0;
}




/**************************************************************************
             begin of GetFPGAParam()文件内函数  共四个函数
**************************************************************************/


/*******************************************************************************
  ② 函数名:  FPGA_SpdFbCal(void)     速度反馈计算   ^_^
  输入:    FPGA反馈速度值
           平均滤波功能码STR_MTR_Gvar.FunCode.SpdFdbFilt_On 
  输出:   STR_MTR_Gvar.FPGA.SpdFdb
  子函数:无
  描述:  得到速度反馈值，并根据功能码判断是否使用均值滤波 
         实际速度（r/min） =nmax * (*MotorSpeed) / 2^15      设定nmax = 6553rpm对应FPGA内部2^15
         在GetFPGAParam(void)（从FPGA中获取数据）中调用该函数
********************************************************************************/
Static_Inline void FPGA_SpdFbCal()
{
    static int32 SpdSum = 0;               //累加的速度和
    static int32 FiltSpd[16] = {0};         //滑动平均的速度中间值
    static int16 SpdBufIndex = 0;          //数组的索引值
    int32  SpdTemp = 0;

    //读取速度值
    if(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL)
    {
        SpdTemp = (int32)((1000L * (int16)(*MotorSpeed) * (int64)STR_FPGA_CalcVar.FPGA_MaxSpd) >> 15);
    }
    else
    {
        SpdTemp = (int32)((10000L * (int16)(*MotorSpeed) * (int64)STR_FPGA_CalcVar.FPGA_MaxSpd) >> 15);
    }

    if(FunCodeUnion.code.GN_SpdFbSel == 0)    
    {
        if(FunCodeUnion.code.GN_SpdFdbFilt_On != 0)     //平均滤波处理,用功能码来设置
        {
            SpdSum -= FiltSpd[SpdBufIndex];
            FiltSpd[SpdBufIndex] = SpdTemp;
            SpdSum += FiltSpd[SpdBufIndex];
            SpdBufIndex ++;
    
            if(SpdBufIndex >= (1 << FunCodeUnion.code.GN_SpdFdbFilt_On))
            {
                SpdBufIndex = 0;
            }
            
            SpdTemp = SpdSum >> FunCodeUnion.code.GN_SpdFdbFilt_On;
        }
        else if(FunCodeUnion.code.GN_SpdLpFiltFc < 4000)    //截止频率小于4000时才进行低通滤波
        {
            NewSpdFdbLowpassFilter1.Input = SpdTemp;
            MTR_NewLowPassFiltCalc(&NewSpdFdbLowpassFilter1);
            SpdTemp = NewSpdFdbLowpassFilter1.Output;
        }
    }
    
    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对反馈取反
    {
        STR_MTR_Gvar.FPGA.SpdFdb = 0 - SpdTemp;              //得到滤波后的速度 
    }
    else
    {    
        STR_MTR_Gvar.FPGA.SpdFdb = SpdTemp;              //得到滤波后的速度 
    }   
     
}

/*******************************************************************************
  函数名:  M_SpdFdbCal()      ^_^
  输入:   1.
          2.
  输出:   1.速度前馈输出STR_SpdRegu.ToqFdFwd_Out
  子函数:无         
  描述:  得到位置反馈，用于速度调节器积分部分的速度反馈运算
********************************************************************************/ 
Static_Inline void M_SpdFdbCal(void)
{
    int32 NewPos;
    int32 SpdTemp;
    static int32 SpdDelPos;
    static int32 OldPos = 0;
    static int16 FirstFlag = 0;
    static int32 SpdSum = 0;               //累加的速度和
    static int32 FiltSpd[16] = {0};         //滑动平均的速度中间值
    static int16 SpdBufIndex = 0;          //数组的索引值

    NewPos               = STR_MTR_Gvar.FPGA.PosFdbAbsValue;      //反馈的绝对位置；
    SpdDelPos = NewPos - OldPos;
    OldPos               = NewPos;

    if(FirstFlag == 0)                  //第一次进入时需要清零
    {
        FirstFlag = 1;
        SpdDelPos = 0;
    }

    SpdTemp = (int32)((STR_FPGA_CalcVar.M_SpdCoff * SpdDelPos) >> 10);

    STR_MTR_Gvar.FPGA.M_SpdFdb = SpdTemp;              //得到M法测速值
    if(FunCodeUnion.code.GN_SpdFdbFilt_On != 0)     //平均滤波处理,用功能码来设置
    {
        SpdSum -= FiltSpd[SpdBufIndex];
        FiltSpd[SpdBufIndex] = SpdTemp;
        SpdSum += FiltSpd[SpdBufIndex];
        SpdBufIndex ++;

        if(SpdBufIndex >= (1 << FunCodeUnion.code.GN_SpdFdbFilt_On))
        {
            SpdBufIndex = 0;
        }
        
        SpdTemp = SpdSum >> FunCodeUnion.code.GN_SpdFdbFilt_On;
    }
    else if(FunCodeUnion.code.GN_SpdLpFiltFc < 4000)    //截止频率小于4000时才进行低通滤波
    {
        NewSpdFdbLowpassFilter.Input = SpdTemp;
        MTR_NewLowPassFiltCalc(&NewSpdFdbLowpassFilter);
        SpdTemp = NewSpdFdbLowpassFilter.Output;
    }

    STR_MTR_Gvar.FPGA.M_SpdFdbFlt = SpdTemp;		 //得到滤波后的速度
}


/*******************************************************************************
  函数名:  OvSpdFdbErrMonitor()       ^_^
  输入:   1.速度反馈STR_MTR_Gvar.FPGA.SpdFdb
          2.电机最大运行速度STR_MTR_Gvar.FunCode.MT_MaxSpd
  输出:   1.速度超速报错标志位
  子函数:无         
  描述:  速度反馈的过速报错监控，当反馈速度连续500次大于最高转速的120%或者超过最高转速150%时，
         系统发出反馈超速报错标志，电机即进入不可控状态，
         因112#（FPGA）平台没有速度反馈处理文件故将此函数放至速度调节器内。  
         该函数在时基中断程序（速度位置环调度）G_MTR_Task_TBINT(void)中调度执行     
********************************************************************************/ 
Static_Inline void OvSpdFdbErrMonitor(void)
{   
    static Uint16 OvSpdCnt = 0; 
  
    if((STR_MTR_Gvar.FPGA.SpdFdb > STR_FPGA_CalcVar.MaxSpdPos_OverErr)        //正转过速警告速度值=最高转速的120% 
     ||(STR_MTR_Gvar.FPGA.SpdFdb < STR_FPGA_CalcVar.MaxSpdNeg_OverErr))        //反转过速警告速度值=最高转速的120%
    {
        if(OvSpdCnt++ > 5)                             //计数5次连续超速报过速  
        {
            PostErrMsg(OVERSPD);                          //发出报错超速标志
        }

    }
    else
    {                                                    //非5次连续超速则计数清零
       OvSpdCnt = 0;
    } 
}
/*******************************************************************************
  ③ 函数名:  FPGA_MechToElecAng(void)     角度转换   ^_^
  输入:    *MechAng 对应编码器线数的机械角度值（为脉冲值）
  输出:  STR_MTR_Gvar.FPGA.MechaAngle机械角度（从原点开始的脉冲数）   STR_MTR_Gvar.FPGA.Etheta输出 当前电机电角度值(0-32768)
  子函数:无
  描述:①将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
      此处机械角电机转子旋转一圈，范围为0~4*PPR->0度~360度（机械角）  其中PPR为编码器每相（A相）每转发出的脉冲数
      ②检验第一次Z脉冲来时电角度是否偏差超过30度，从而判定编码器是否有问题，小于330度是为了
      避免360度到0度切换时误报警 for inc_enc
      16K电流环ELECANG_30相对于10000线光电编码器转速为79968rpm，相对于17位编码器转速为6101，因此只适用于光电编码器
      在GetFPGAParam(void)（从FPGA中获取数据）中调用该函数
********************************************************************************/ 
Static_Inline void FPGA_MechToElecAng(void)   
{
    Uint32  Temp = 0;

    if((UNI_FPGA_EncType.bit.BIGTYPE == ABS_ENCODER_SEL) | (UNI_FPGA_EncType.bit.BIGTYPE == ROTATING_TRANSFORMER_SEL))     //绝对式编码器
    {
        //对于串行编码器，则MechAng=AbsAng[19:4]-HostAng，当电机转子旋转一圈时，
        //MechAng= 0~216'0°~360°，其中AbsAng为编码器的返回位置
        STR_MTR_Gvar.FPGA.MechaAngle = *MechAng;             //机械角度0~65535
        Temp = STR_MTR_Gvar.FPGA.MechaAngle;

        //只要Temp大于AngToCnt则减AngToCnt(一个电周期对应脉冲数)
        //必须用大于等于，不能大于
        while(Temp >= STR_FPGA_CalcVar.AngToCnt)
        {
            Temp = Temp % STR_FPGA_CalcVar.AngToCnt;
        }

        //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
        STR_MTR_Gvar.FPGA.Etheta  = (Uint16)(((Uint64)Temp * STR_FPGA_CalcVar.CntToAng_Q16) >> 16);  //得到当前电角度值
    }
    else if((UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL)
            ||(UNI_FPGA_EncType.bit.BIGTYPE == LIN_ENCODER_SEL))  //为光电式编码器或直线光栅
    {
        STR_MTR_Gvar.FPGA.MechaAngle = *MechAng;             //机械角度0~4*PPR
        Temp = STR_MTR_Gvar.FPGA.MechaAngle;

        //只要Temp大于AngToCnt则减AngToCnt(一个电周期对应脉冲数)
        //必须用大于等于，不能大于
        if(STR_FPGA_CalcVar.AngToCnt == 0) STR_FPGA_CalcVar.AngToCnt = STR_MTR_Gvar.FPGA.MechaAngle;

        while(Temp >= STR_FPGA_CalcVar.AngToCnt)
        {
            Temp = Temp % STR_FPGA_CalcVar.AngToCnt;
        }

        //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  32768对应1个电周期的脉冲数 Q16格式
        STR_MTR_Gvar.FPGA.Etheta  = (Uint16)(((Uint64)Temp * STR_FPGA_CalcVar.CntToAng_Q16) >> 16);  //得到当前电角度值

       //用以检验第一次Z脉冲来时电角度是否偏差超过30度，从而判定编码器是否有问题，小于330度是为了避免360度到0度切换时误报警 for inc_enc
       //16K电流环ELECANG_30相对于10000线光电编码器转速为79968rpm，相对于17位编码器转速为6101，因此只适用于光电编码器
        if((UNI_FPGA_IncEncCtrl.bit.EncEnbl == 1) &&
           (UNI_FPGA_EncType.bit.BIGTYPE == INC_ENCODER_SEL) &&    //WZG 增量码盘 非UV相电流平衡校正
           (AuxFunCodeUnion.code.OEM_ResetABSTheta == 0) &&   //未执行初始角辨识时才判断
           (ABS(STR_FPGA_CalcVar.OldElecAng - STR_MTR_Gvar.FPGA.Etheta) > ELECANG_45) &&       //wzg
           (ABS(STR_FPGA_CalcVar.OldElecAng - STR_MTR_Gvar.FPGA.Etheta) < ELECANG_315))       //wzg
        {
            *HostSon = DISPWM;
            PostErrMsg(ENCDERR_Z);
        }

        STR_FPGA_CalcVar.OldElecAng = STR_MTR_Gvar.FPGA.Etheta;
    }

}


/*******************************************************************************
  ④ 函数名:  FPGA_CtrlParaUpdate(void)     更新FPGA控制参数传送至ST   ^_^
  输入: 
  输出: 电流反馈、电压给定、控制状态、绝对位置指令和反馈值、中断定长锁存脉冲值 
  子函数:无
  描述: 更新FPGA控制传送至ST的参数  
        如电流反馈、电压给定、控制状态、绝对位置指令和反馈值、中断定长锁存脉冲值
********************************************************************************/ 
Static_Inline void FPGA_CtrlParaUpdate(void)   
{
    int32 Temp = 0;
    UNI_FPGA_SYSSTATE_REG   UNI_FPGA_SysState   = {0};     //⑥系统状态寄存器

    STR_MTR_Gvar.FPGA.IdFdb      = *Id;
      
    UNI_FPGA_SysState.all = *SysState;
    STR_MTR_Gvar.MTRtoFUNCFlag.bit.CurSmpMode = UNI_FPGA_SysState.bit.SampleMode; 

    Temp = *Vq;
    STR_MTR_Gvar.FPGA.Vd = *Vd;
    STR_MTR_Gvar.FPGA.Vq = Temp;
    if ((Temp > 5500) || (Temp < -5500))    STR_MTR_Gvar.MTRtoFUNCFlag.bit.VoltVectorLmt = 1;
    else    STR_MTR_Gvar.MTRtoFUNCFlag.bit.VoltVectorLmt = 0;
    Temp = *Iq;
    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对反馈取反
    {
        STR_MTR_Gvar.FPGA.IqFdb = 0 - Temp;
    }
    else
    {
        STR_MTR_Gvar.FPGA.IqFdb = Temp;
    }

#if POWERDRIVER_TYPE==POWDRV_IS650
    //IS_7860Gain 电流采样最大量程(单位IS620:0.01A; IS650:0.1A)计算 
    STR_MTR_Gvar.FPGA.IuFdb      = (((int32)FunCodeUnion.code.IS_7860Gain * (int32)(*Iu))*10)>>14;
    STR_MTR_Gvar.FPGA.IvFdb      = (((int32)FunCodeUnion.code.IS_7860Gain * (int32)(*Iv))*10)>>14;
    STR_MTR_Gvar.FPGA.IwFdb      = 0 - STR_MTR_Gvar.FPGA.IuFdb - STR_MTR_Gvar.FPGA.IvFdb;
#else
    //IS_7860Gain 电流采样最大量程(单位IS620:0.01A; IS650:0.1A)计算 
    STR_MTR_Gvar.FPGA.IuFdb      = ((int32)FunCodeUnion.code.IS_7860Gain * (int32)(*Iu))>>14;
    STR_MTR_Gvar.FPGA.IvFdb      = ((int32)FunCodeUnion.code.IS_7860Gain * (int32)(*Iv))>>14;
    STR_MTR_Gvar.FPGA.IwFdb      = 0 - STR_MTR_Gvar.FPGA.IuFdb - STR_MTR_Gvar.FPGA.IvFdb;
#endif  

    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对反馈取反
    {
        if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.UVAdjustRatioEn == 0)
        {
            Temp = STR_MTR_Gvar.FPGA.IvFdb;
            STR_MTR_Gvar.FPGA.IvFdb          = STR_MTR_Gvar.FPGA.IwFdb;
            STR_MTR_Gvar.FPGA.IwFdb          = Temp;
        }
    }

    STR_MTR_Gvar.FPGA.SysStatus  = *SysState;

    //计算由FPGA输出的绝对位置反馈值，原112在Servo_EncSample.c中计算
    Temp =  (int32)A_SHIFT16_PLUS_B((*PosFbkHigh),(*PosFbkLow));
    
    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对反馈取反
    {
        STR_MTR_Gvar.FPGA.PosFdbAbsValue = 0 - Temp;
    }
    else
    {
        STR_MTR_Gvar.FPGA.PosFdbAbsValue = Temp; 
    }

    //计算由FPGA输出的位置脉冲指令，原112在Servo_Pulse.c中计算
    STR_MTR_Gvar.FPGA.PulsePosRef = (int32)A_SHIFT16_PLUS_B((*PosRefHigh),(*PosRefLow));

    //计算由FPGA输出的位置脉冲指令2高速
    STR_MTR_Gvar.FPGA.PulsePosRef2 = (int32)A_SHIFT16_PLUS_B((*PosRefHigh2),(*PosRefLow2));

    //全闭环位置反馈 
    Temp = A_SHIFT16_PLUS_B((*PosLnrHigh),(*PosLnrLow));
    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)     //旋转方向变化时对反馈取反
    {
        STR_MTR_Gvar.FPGA.LnrPosFdb = 0 - Temp;
    }
    else
    {
        STR_MTR_Gvar.FPGA.LnrPosFdb = Temp; 
    }
}
/**************************************************************************
             end of GetFPGAParam()文件内函数  共四个函数
**************************************************************************/


/*******************************************************************************
  函数名:  FPGA_CurGainSwitch(void)
  输入: 电流给定
  输出: 无
  子函数:无
  描述: 根据电流偏差进行增益切换
********************************************************************************/ 
Static_Inline void FPGA_CurGainSwitch(void)
{
    int32 AbsIqErr;
    Uint32 KpTemp;

    AbsIqErr = ABS(STR_MTR_Gvar.GetIqRef.IqRef - STR_MTR_Gvar.FPGA.IqFdb);
    
    if(AbsIqErr < STR_MTR_Gvar.GetIqRef.IqPoint1)
    {
        *KpId = (STR_FPGA_CalcVar.IdKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIdKp) >> 16; //H01_52 性能优先模式D轴比例增益 
        *KpIq = (STR_FPGA_CalcVar.IqKpCoef_Q16 * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 16; //H01_54 性能优先模式Q轴比例增益 
    }
    else if(AbsIqErr < STR_MTR_Gvar.GetIqRef.IqPoint2)
    {
        KpTemp = 1024 + ((STR_MTR_Gvar.GetIqRef.FirSlope_Q10 * (AbsIqErr - STR_MTR_Gvar.GetIqRef.IqPoint1)) >> 10);
        KpTemp = (KpTemp * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 10;         //根据百分比算出增益
        KpTemp = (KpTemp * STR_FPGA_CalcVar.IqKpCoef_Q16) >> 16;
        *KpId = KpTemp;
        *KpIq = KpTemp;
    }
    else if(AbsIqErr < STR_MTR_Gvar.GetIqRef.IqPoint3)    //系数准确除1000，近似处理除1024  
    {
        KpTemp = ((Uint32)FunCodeUnion.code.OEM_CapSecKpCoff * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 10;
        KpTemp = (KpTemp * STR_FPGA_CalcVar.IqKpCoef_Q16) >> 16;
        *KpId =  KpTemp;
        *KpIq =  KpTemp;
    }
    else if(AbsIqErr < STR_MTR_Gvar.GetIqRef.IqPoint4)     
    {
        KpTemp = FunCodeUnion.code.OEM_CapSecKpCoff         
                 + ((STR_MTR_Gvar.GetIqRef.SecSlope_Q10 * (AbsIqErr - STR_MTR_Gvar.GetIqRef.IqPoint3)) >> 10);
        KpTemp = (KpTemp * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 10;     //根据百分比算出增益
        KpTemp = (KpTemp * STR_FPGA_CalcVar.IqKpCoef_Q16) >> 16;
        *KpId = KpTemp;
        *KpIq = KpTemp; 
    }
    else  //系数准确百分比形式除1000，近似处理除1024   
    {
        KpTemp = ((Uint32)FunCodeUnion.code.OEM_CapThirdKpCoff * (Uint32)FunCodeUnion.code.OEM_CapIqKp) >> 10;
        KpTemp = (KpTemp * STR_FPGA_CalcVar.IqKpCoef_Q16) >> 16;
        *KpId = KpTemp;
        *KpIq = KpTemp;
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/
void RunCaseShortGnd(void)
{
    static int8   ShortGndStep = 0;
    static int8   Cnt = 0;
    static Uint16 PwmPeriod = 0;
    static Uint16 PwmStep = 0;  //1%
    static Uint16 PwmDuty = 0;  //范围1%——95%
    static int32  OvCur = 0;
    static Uint16 Delay20msCnt = 0;
    static Uint32 UdcTemp = 0;

    //对地短路检测完退出
    if(STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndDone == MTR_Valid) 
    {
        return;
    } 

    switch(ShortGndStep)
    {
        case 0:     //配置

            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndRunFlag = 0;

            //电机电流检测准备好标志位无效时退出
            //母线电压未准备好时退出
            if( (STR_MTR_Gvar.GlobalFlag.bit.CurSampleRdy == MTR_Invalid)                         
             || (UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.UdcOk == MTR_Invalid) ) 
            {
                Delay20msCnt = 0;
                return;
            }

            //速度反馈大于10rpm退出
            if(ABS(STR_MTR_Gvar.FPGA.M_SpdFdbFlt) >= 100000) 
            {
                Delay20msCnt = 0;
                return;
            } 
            
            if(Delay20msCnt < (STR_MTR_Gvar.System.ToqFreq / 50))
            {
                Delay20msCnt ++;
                return;
            } 

            PwmPeriod  = (FPGA_CLK / STR_MTR_Gvar.System.CarFreq) - 1;
            PwmStep = PwmPeriod / 200;
            PwmDuty = 0;

            *HostSysCtrl = (0xF700 | STR_MTR_Gvar.FPGA.SysCtrl);
            *HostDutyU = PwmDuty;
            UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
            ShortGndStep = 1;             
            OvCur = FunCodeUnion.code.OEM_RateCurrent >> 2;
            UdcTemp =  UNI_MTR_FUNCToMTR_List_16kHz.List.Udc_Live;

            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndRunFlag = 1;
            break;

        case 1:     //检测中
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndRunFlag = 1;

            if(ABS(STR_MTR_Gvar.FPGA.IuFdb) > OvCur)
            {
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
                PostErrMsg(OUTSHORT2GND0);
                *HostDutyU = 0;
                ShortGndStep = 2;
                break;
            }
            else if((UNI_MTR_FUNCToMTR_List_16kHz.List.Udc_Live > (UdcTemp + 650))
					&&(FunCodeUnion.code.OEM_ServoSeri<20014)) 
            {
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;
                PostErrMsg(OUTSHORT2GND1);
                *HostDutyU = 0;
                ShortGndStep = 2;
                break;
            }

            Cnt ++;
            if(0 == (Cnt & 1))
            {
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
                break;
            }

            if(PwmDuty < PwmPeriod)
            {
                PwmDuty += PwmStep;
                *HostDutyU = PwmDuty;
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = ENPWM;
            }
            else
            {
                UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.PwmStatus = DISPWM;                                
                *HostDutyU = 0;
                ShortGndStep = 2;
            }
            break;
        case 2:     //退出
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndDone = MTR_Valid;
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndRunFlag = 0; 
            *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;
            ShortGndStep = 3;
            break;
        default:
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndDone = MTR_Valid;
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.ShortGndRunFlag = 0;
            break;
    }
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/
void UpdateH0B_FPGA_State(void)
{
    AuxFunCodeUnion.code.DP_FpgaSysErr = *SysErr;        //获取FPGA给出的报警信息
    AuxFunCodeUnion.code.DP_FpgaTmFlt = *TmFlt;        //获取FPGA超时故障标准位
    AuxFunCodeUnion.code.DP_FpgaSysState = *SysState;    //用于显示系统状态
    AuxFunCodeUnion.code.DP_FpgaAbsRomErr = *AbsEncErr;     //H0b28，绝对编码器故障信息显示
}

/*******************************************************************************
  函数名: void FPGA_InitLnrEncCtrl(void)

  输  入:  外部位置反馈     
  输  出:   NULL
  子函数:                                       
  描  述:  全闭环编码器控制初始化，暂时未添加
********************************************************************************/
Static_Inline void FPGA_InitLnrEncCtrl(void)
{
    union_LNRENCTRL_REG LnrEnCtrl_Reg = {0};

	LnrEnCtrl_Reg.all = 0;	          //寄存器初始化

	LnrEnCtrl_Reg.bit.LnrEncEnbl = 1;

	LnrEnCtrl_Reg.bit.LnrEncFiltTm = FunCodeUnion.code.ER_RotEncFiltTm; //编码器滤波时间，单位25ns

	*LnrEncCtrl = LnrEnCtrl_Reg.all; 
    
    //直线编码器断线检测滤波时间 设为16
    LnrEnCtrl2_Reg.bit.LnrEncFiltTm2 = 16;
    LnrEnCtrl2_Reg.bit.LnrFltEnbl = 0;
    *LnrEncCtrl2 = LnrEnCtrl2_Reg.all;          
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void AbsMod1_MultiTurnOffset(void)
{
#if HC_ENC_SW
    if((FunCodeUnion.code.MT_EncoderSel == 0x13) && (23 == (FunCodeUnion.code.MT_ABSEncVer / 1000)))
    {
        FunCodeUnion.code.PL_EncMultiTurnOffset = *AbsExtData;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));
    }
#endif
#if NOKIN_ENC_SW
    else if(FunCodeUnion.code.MT_EncoderSel == 0x12)
    {
        FunCodeUnion.code.PL_EncMultiTurnOffset = *NKAbsTurn;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));
    }
#endif
#if TAMAGAWA_ENC_SW
    if(FunCodeUnion.code.MT_EncoderSel == 0x10)
    {
        FunCodeUnion.code.PL_EncMultiTurnOffset = *TAMAGAbsTurn;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_EncMultiTurnOffset));  
    }
#endif    

#if HDH_ENCODER_SW
    if(FunCodeUnion.code.MT_EncoderSel == 0x11)
    {
        FunCodeUnion.code.PL_HDHEncMultiTurnOffset = 4095 - (*HDHAbsTurn);
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_HDHEncMultiTurnOffset));
    }
#endif


}

/********************************* END OF FILE *********************************/




