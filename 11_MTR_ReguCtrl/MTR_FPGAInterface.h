/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:  MTR_FPGAInterface.h                                                          
 创建人：姚虹                创建日期：2010.09.25
 修改人：朱祥华              修改日期：2011.11.13 
 描述： 
    1.FPGA相关的变量，地址等.
 修改记录：  
    1. xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
#ifndef MTR_FPGAINTERFACE_H
#define MTR_FPGAINTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif	


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 宏定义 常数类*/
#define FPGA_CLK   40000000     //FPGA时钟频率（Hz）
#define FPGA_PRD   25           //FPGA时钟周期（ns）
#define ADSAMP_CLK 10000000     //AMC1203或HCPL-7860的时钟频率为10MHz
#define ADSAMP_PRD 100          //AMC1203或HCPL-7860的时钟周期100ns
#define SQRT6_Q10  2508 
#define PERTHOU_Q20 1049        //千分之一的Q20值
#define DCVOLT220  310          //三相220V给定时的直流母线电压
#define DCVOLT380  540          //三相380V给定时的直流母线电压
#define DCVOLTMAX  830          //采样电阻最大采样值对应的母线电压
#define PI_Q10     3217
#define THETA_30   30           //角度值
#define THETA_90   90
#define THETA_150  150
#define THETA_210  210
#define THETA_270  270
#define THETA_330  330
#define ELECANG_30 2730          //30度电角度对应的计数值 ,0~360 对应 0～32768
#define ELECANG_330 30037
#define ELECANG_45      4096     //45度电角度对应的计数值 ,0~360 对应 0～32768
#define ELECANG_315     28672

#define BLMTMAXSPD    32768     //单位0.1mm/s
#define BLMTMINSPD    10        //单位0.1mm/s
//编码器大类
#define INC_ENCODER_SEL       0    //增量
#define ABS_ENCODER_SEL       1    //绝对
#define ROTATING_TRANSFORMER_SEL       2    //旋变
#define LIN_ENCODER_SEL       3    //光栅
//增量编码器中的小类
#define COM_INC_ENCODER_SEL   0    //普通增量
#define SAV_INC_ENCODER_SEL   1    //省线增量
#define ABZ_INC_ENCODER_SEL   2    //only abz
//绝对编码器中的小类
#define TMG_ABS_ENCODER_SEL   0    //多摩川17位绝对编码器
#define HDH_ABS_ENCODER_SEL	  1	   //海德汉19位绝对式编码器  
#define NOKIN_ABS_ENCODER_SEL 2    //尼康20位绝对编码器       
#define INO_ABS_ENCODER_SEL	  3	   //汇川20位编码器         

//#define DNH_ABS_ENCODER_SEL   1    //丹纳赫BISS协议绝对编码器  暂时没有使用

//编码器厂家设置
#define DNH                   1   //丹纳赫编码器
#define DMC                   0   //多摩川编码器
//母线电压的范围：对应不同的驱动器电压等级
//#define DC220MAX 4640L  //0.1V  MD300S2.2  500V,
#define DC220MAX           5000L
//IS500  3.3V对应1000V  AD采样时数字量4096对应低压3V即母线电压1000V 
#define DC380MAX_IS500     10000L  
//IS550借用变频器驱动 3.3V对应1000V  AD采样时数字量4096对应低压3V即母线电压909V 
#define DC380MAX_IS550     9091L   


/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */
#define FPGA_WRITEPWMBLK(A) *HostSon = (!A)



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
//FPGA系统配置寄存器寄存器新平台用到的数据结构定义

//电流采样延时控制
typedef struct{
    Uint16 SampleStartDelay:14;
    Uint16 Decimation:2;
}STR_FPGA_CURSAMPCTRL_BIT;

typedef union{
    volatile Uint16                    all;
    volatile STR_FPGA_CURSAMPCTRL_BIT  bit;
}UNI_FPGA_CURSAMPCTRL_REG;

//最小零矢量时间控制
typedef struct{
    Uint16 ZsvMinTm:15;
    Uint16 ZsvMinEn:1;
}STR_FPGA_ZSVMINCTRL_BIT;
typedef union{
    volatile Uint16                    all;
    volatile STR_FPGA_ZSVMINCTRL_BIT   bit;
}UNI_FPGA_ZSVMINCTRL_REG;

typedef struct{
    Uint16 SpeedStartDelay:14;   //速度计算启动延时
	Uint16 SyncEdge:1;			//0~总线同步信号的下降沿有效；1~上升沿有效。
	Uint16 SyncMode:1;          //0~自动跟踪并同步于EtherCAT总线；1~FPGA内部自行完成同步控制
}STR_FPGA_PERIODCTRL_BIT;
typedef union{
    volatile Uint16                    all;
    volatile STR_FPGA_PERIODCTRL_BIT   bit;    
}UNI_FPGA_PERIODCTRL_REG;

/*1169(周兆勇) 2015-06-29 16:26:35
除了“SdmClkEnbl”调整到0x08的[8](原620P的StoSns位)，其他都与620N一样*/

//①FPGA硬件系统设置寄存器
 
typedef struct{
    Uint16 GateSnsL:1;   //bit0 下桥臂IGBT门极驱动有效电平，0～低电平，1～高电平 
    Uint16 GateSnsH:1;   //bit1 上桥臂IGBT门极驱动有效电平，0～低电平，1～高电平
    Uint16 GateKillSns:1;//bit2 IGBT过流保护有效电平
    Uint16 RelaySns:1;   //bit3 继电器动作有效电平，0～低电平，1～高电平
    Uint16 IfbUInv:1;    //bit4 U相电流极性取反 0～不取反，1～取反
    Uint16 IfbVInv:1;    //bit5 V相电流极性取反 0～不取反，1～取反
    Uint16 ValidEdge:1;  //bit6 Σ-Δ调制器有效时钟边沿，0～下降沿，1～上升沿；
    Uint16 BrakeSns:1;   //bit7 母线电压泄放驱动有效电平  0～低电平有效，1～高电平有     
#if POWERDRIVER_TYPE == POWDRV_IS650      
    Uint16 SdmClkEnable:1;  //bit8  Σ-Δ调制器外部时钟使能
#else
    Uint16 StoSns:1;     //bit8 STO信号有效电平     0～低电平有效，1～高电平有效 
#endif 
    Uint16 SdmFiltTm:2;  //bit9~10 sigma_Delta调制器信号滤波时间，Tm(ns) = 系统周期(ns)*SdmFilt/3;
    Uint16 GkFiltTm:5;   //bit11~15 IGBT硬件过流信号滤波时间,    Tm(ns) = 系统周期(ns) * GkFiltTm;
}STR_FPGA_SYS_CONFIG_BIT; 

typedef union{
    volatile Uint16                   all;
    volatile STR_FPGA_SYS_CONFIG_BIT  bit;
}UNI_FPGA_SYSCONFIG_REG;


//②位置脉冲指令寄存器控制寄存器
typedef struct{
    Uint16 PosCmdEnbl:1;    //bit0 位置命令使能
    Uint16 PosCmdMode:2;    //bit1~bit2 命令模式：方向+脉冲，或AB正交，或CW/CCW；
    Uint16 PosCmdEdge:1;    //bit3 命令边沿选择：1～上升沿有效，0～下降沿有效
    Uint16 PosCmdInv:1;     //bit4 0～位置脉冲不取反， 1～位置脉冲取反
    Uint16 PosCmdMux:1;     //bit5 0～低速脉冲命令， 1～高速脉冲命令  (v1.5以上版本改为移到0x15 *EncDivCtrl寄存器中)
    Uint16 rsvd:2;          //bit6~bit7
    Uint16 PosCmdFiltTm:8;  //bit8~bit15  命令脉冲信号滤波时间	  Tm(ns) = 系统周期(ns) * PosCmdFiltTm;
}STR_FPGA_POSCMD_BIT;
typedef union{
    volatile Uint16                all;
    volatile STR_FPGA_POSCMD_BIT   bit;
}UNI_FPGA_POSCMD_REG;



//③旋转编码器控制寄存器
typedef struct{
    Uint16 EncEnbl:1;      //bit0 编码器使能。 1～使能，0～不使能
    Uint16 RotIncZLen:1;   //bit1 编码器Z脉宽选择。  1～Z脉宽大于A(B)相周期的1/3，0～Z脉宽小于A(B)相周期的1/3；
    Uint16 EncPhsMux:1;    //bit2 测速脉冲选择， 0～A相，1~B相
    Uint16 EncEdgeMux:2;   //bit3~bit4 测速脉冲边沿选择，0～下降沿有效，1～正转上升沿反转下降沿，2～正转下降沿，反转上升沿，3～上升沿
    Uint16 RotEncInv:1;    //bit5 旋变反向，0～A超前于B,1～B超前于A；
    Uint16 RotEncZPol:1;   //bit6 旋变Z脉冲极性；0～负极性，1～正极性
    Uint16 WireSaving:1;   //bit7 省线式增量型编码器，0～非省线式，1～省线式
    Uint16 RotEncFiltTm:8; //bit8~bit15 编码器信号滤波时间   Tm(ns) = 系统周期(ns) * RotEncFiltTm;
}STR_FPGA_INCENC_CTRL_BIT;
typedef union{
    volatile Uint16                  all;
    volatile STR_FPGA_INCENC_CTRL_BIT   bit;
}UNI_FPGA_INCENC_CTRL_REG;

/*
说明：AbsType：0-EQI1331，1-EQI1325；
通讯频率=40MHz/2/(BaudSelect+1)；默认值为9（即2MHz），建议通过功能码设置。
*/
typedef struct{
    Uint16 Rsvd:7;
    Uint16 HDHAbsType:1;      
    Uint16 BaudSelect:8;   
}STR_FPGA_HDHABSENC_SEL_BIT;
typedef union{
    volatile Uint16                  all;
    volatile STR_FPGA_HDHABSENC_SEL_BIT   bit;
}UNI_FPGA_HDHABSENC_SEL_REG;

typedef struct{
    Uint16 Rsvd0:2;
    Uint16 NikType:1;      //0-20bit，1-17bit（当BaudSelect=2时，可选择4M波特率，其它值默认2.5M）
    Uint16 AbsType:2;      //0-编码器接口复位，1-汇川编码器(20/23bit)，2-尼康编码器，3-多摩川编码器(17bit)；
    Uint16 Rsvd1:9;
    Uint16 BaudSelect:2;   //0-2MHz（默认兼容第一代编码器），1-2.5MHz，2-4MHZ，3-未定义，该参数仅适用于汇川第二代编码器。
}STR_FPGA_ABSENC_SEL_BIT;
typedef union{
    volatile Uint16                  all;
    volatile STR_FPGA_ABSENC_SEL_BIT   bit;
}UNI_FPGA_ABSENC_SEL_REG;

//④编码器类型设置寄存器
typedef struct{
    Uint16 SMALLTYPE:4;    //bit0~3,小类,用以区分
    Uint16 BIGTYPE:4;      //bit4~7,大类 绝对 增量 旋变
    Uint16 SPECIAL:4;      //bit8~11 编码器厂家设置,决定Z脉宽
    Uint16 ResetOn:1;      //bit12 重新复位绝对值码盘计数
    Uint16 rsvd:3;         //bit13~15  reserved
}STR_FPGA_ENC_TYPE_BIT;
typedef union{
    volatile Uint16                  all;
    volatile STR_FPGA_ENC_TYPE_BIT   bit;
}UNI_FPGA_ENC_TYPE_REG;

//⑤分频输出控制寄存器
typedef struct{
    Uint16 EncDivEnbl:1;  //编码器分频输出使能，0～禁止，1～使能
    Uint16 EncDivInv:1;   //输出脉冲反相   0～A超前于B，1～B超前于A
    Uint16 EncDivPol:1;   //输出Z脉冲的极性，0～负极性，1～正极性
    Uint16 EncDivMux:1;   //信号源选择，0～编码器分频输出，1～脉冲同步输出
    Uint16 EncDivOCZInv:1; //输出OCZ脉冲的极性
    Uint16 PosCmdMux:1;   //脉冲指令源选择
//    Uint16 rsvd:1;
    Uint16 StoFiltTm:10;  //STO信号滤波时间   实际的STO信号滤波时间(ns)=系统主时钟周期(ns)×StoFiltTm
}STR_FPGA_ENC_DIVCTRL_BIT; 
typedef union{
    volatile Uint16                    all;
    volatile STR_FPGA_ENC_DIVCTRL_BIT  bit;
}UNI_FPGA_ENC_DIV_REG;

//直线/全闭环外部编码器设置
struct LNRENCTRL_Bit
{
    Uint16 LnrEncEnbl:1;   //直线编码器使能控制。  0～禁止， 1～使能
	Uint16 LnrEncInv:1;	   //编码器AB脉冲反相。    0～A相超前于B相，1～A相滞后于B相；
	Uint16 rsvd:6;        
	Uint16 LnrEncFiltTm:8;  //编码器信号滤波时间   Tm(ns) = 系统周期(ns) * LnrEncFiltTm;
};

typedef union
{
    Uint16 all;
	struct LNRENCTRL_Bit bit;
}union_LNRENCTRL_REG;

//直线/全闭环外部编码器设置2
struct LNRENCTRL2_Bit
{
    Uint16 LnrEncFiltTm2:8;   //直线编码器断线检测滤波时间
	Uint16 LnrFltEnbl:1;	  //断线检测使能     
	Uint16 HDHAbsTstTime:7;    //海德汉编码器恢复时间
};

typedef union
{
    Uint16 all;
	struct LNRENCTRL2_Bit bit;
}union_LNRENCTRL2_REG;


//相角补偿因子寄存器
typedef struct{
    Uint16 CompAngScl_I:8;     //电流相角超前补偿因子
    Uint16 CompAngScl_U:8;     //电压相角超前补偿因子
}STR_FPGA_COMPANG_BIT;

typedef union{
    volatile Uint16                all;
    volatile STR_FPGA_COMPANG_BIT  bit; 
}UNI_FPGA_COMPANG_REG;

//报警命令寄存器
typedef struct{
    Uint16 HostFltClr:1;     //0～正常，1～清除报警信号（电平敏感，非边沿敏感）；
    Uint16 HostBrake:1;      //0～正常，1～启动BRAKE（电平敏感，非边沿敏感）
    Uint16 RSVD:14;          //
}STR_FPGA_ALARMCLR_BIT;

typedef union{
    volatile Uint16                  all;
    volatile STR_FPGA_ALARMCLR_BIT   bit;
}UNI_FPGA_ALARMCLR_REG;


//系统控制寄存器
typedef struct{
    Uint16 IfbCalibEnbl:1; //相电流平衡校正使能  0～正常，1～禁止W相PWM输出（仅用于相电流平衡校正）；
    Uint16 AngInitEnbl :1; //初始角自动辨识使能  0～正常，1～机械角度计数值总是等于HostAng（仅用于正交编码器的初始角辨识）；
    Uint16 EncZDsbl:1;     //编码器Z相脉冲屏蔽  0～正常，1～电机编码器的Z相脉冲被屏蔽（此时与Z脉冲相关的功能均失效）；
    Uint16 ZPlsMonDsbl:1; //禁止Z相脉冲缺失监控 0～正常，1～不监控电机编码器的Z相脉冲缺失故障（即RotEncZFlt报警被禁止）；
    Uint16 ZCntMonDsbl:1; //禁止ABZ相计数监控  0～正常，1～不监控电机编码器的ABZ相计数错误（即RotZCntFlt报警被禁止）；
    Uint16 Rsvd:4;        //保留
    Uint16 PwmDutySrc:1;  //PWM占空比选择   0～矢量控制，1～主机给定
    Uint16 PwmDisUH:1;    //U相上桥臂IGBT门极驱动禁止    0～门极正常驱动，1～门极驱动关断
    Uint16 PwmDisUL:1;    //U相下桥臂IGBT门极驱动禁止
    Uint16 PwmDisVH:1;    //V相上桥臂IGBT门极驱动禁止
    Uint16 PwmDisVL:1;    //V相下桥臂IGBT门极驱动禁止
    Uint16 PwmDisWH:1;    //W相上桥臂IGBT门极驱动禁止
    Uint16 PwmDisWL:1;    //W相下桥臂IGBT门极驱动禁止
}STR_FPGA_SYSCTRL_BIT;

typedef union{
    volatile Uint16                all;
    volatile STR_FPGA_SYSCTRL_BIT  bit;
}UNI_FPGA_SYSCTRL_REG;


//⑥系统状态寄存器
typedef struct{
    Uint16 PwrOnInitDone:1;  //bit0 上电初始化进程
    Uint16 nExtRst:1;        //bit1 复位引脚状态
    Uint16 RlyActive:1;      //bit2 直流母线旁路继电器状态
    Uint16 Fault:1;          //bit3 系统总故障状态

    Uint16 SrvRdy:1;         //bit4 系统Ready状态
    Uint16 PwmEnbl:1;        //bit5 PWM使能状态
    Uint16 TrqOvrlimP:1;     //bit6 正向转矩限制中
    Uint16 TrqOvrlimN:1;     //bit7 反向转矩限制中

    Uint16 SampleMode:1;     //bit8 当前采样模式  0～Sinc3抽取率为第一组设置值，1～Sinc3抽取率为第二组设置值
    Uint16 Rsvd:1;		     //bit9 保留位
    Uint16 MtrEncType:2;     //bit10-11  电机编码器类型  0～无效类型,1～省线式（隐含UVW相）,2～仅含ABZ相,3～串行编码器；
    Uint16 AngInitDone:1;    //bit12 初始角写入状态
    Uint16 EncUvw:3;         //bit13~15 编码器UVW状态
}STR_FPGA_SYSSTATE_BIT;
typedef union{
    volatile Uint16                   all;
    volatile STR_FPGA_SYSSTATE_BIT   bit;
}UNI_FPGA_SYSSTATE_REG;

//⑦系统故障状态寄存器
typedef struct{
    Uint16  ExecTmFlt:1;       //bit 0 系统采样或运算超时    故障码  EXECTMFLT            0x2208     //FPGA系统采样运算超时
    Uint16  SdmFltU:1;         //bit 1 U相电流Sigma——Delta调制器故障  故障码  SDMFLTU     0x2205     //U相Sigma-Delta调制器故障,只报警   姚虹V1.30版本将该故障注释掉
    Uint16  SdmFltV:1;         //bit 2 V相电流Sigma——Delta调制器故障  故障码  SDMFLTV     0x2206     //V相Sigma-Delta调制器故障          姚虹V1.30版本将该故障注释掉
    Uint16  AdcFlt:1;          //bit 3 16位A/D转换器故障
    Uint16  GateKillFlt:1;     //bit 4 IGBT硬件过流          故障码  HWOVERCURRENT        0x0201     //硬件过流故障
    Uint16  OvrCurFlt:1;       //bit 5 D/Q轴电流溢出（定标出现问题）  故障码  DQOVRCUR    0x2207     //D/Q电流溢出；
    Uint16  OvrSpdFlt:1;       //bit 6 转速溢出              故障码  OVERSPD              0x2500     //过速
    Uint16  RotEncUvwFlt:1;    //bit 7 编码器UVW相逻辑错误   故障码  ENFBCHKERR           0x0A34     //编码器回送校验异常
    Uint16  RotEncAbFlt:1;     //bit 8 编码器AB相同时翻转错误 故障码  MULTIRERR           0x0A35     //Z断线
    Uint16  RotEncZFlt:1;      //bit 9 编码器Z相缺失故障     故障码  MULTIRERR            0x0A35     //Z断线
    Uint16  RotZCntFlt:1;      //bit 10 编码器ABZ相计数错误   故障码  ENCDERR_AB          0x0741     //编码器AB干扰故障
    Uint16  OvrCurU:1;         //bit11 驱动器U相过流
    Uint16  OvrCurV:1;         //bit12 驱动器V相过流
    Uint16  Sto1:1;           //bit13 STO1状态
    Uint16  Sto2:1;           //bit14 STO2状态
    Uint16  McuLost:1;         //bit 15 MCU访问间隔超时
}UNI_FPGA_SYSERR_BIT;
typedef union{
    volatile Uint16                all;
    volatile UNI_FPGA_SYSERR_BIT   bit;
}UNI_FPGA_SYSERR_REG;

//超时故障状态寄存器
typedef struct{
    Uint16  McuTmFlt:1;     //bit 0 MCU未及时更新转矩指令引起的超时错误
    Uint16  AbsTmFlt:1;     //bit 1 编码器通讯超时引起的错误
    Uint16  FocTmFlt:1;     //bit 2 电流采样（7860）超时引起的错误
    Uint16  AdcTmFlt:1;     //bit 3 A/D转换过长引起的错误
    Uint16  LnrEncFlt:1;    //bit 4 直线编码器断线
    Uint16  SyncLost:1;     //bit 5 EtherCAT总线同步信号丢失
    Uint16  Rsvd:10;        //bit 6-15 
}UNI_FPGA_TMFLT_BIT;
typedef union{
    volatile Uint16                all;
    volatile UNI_FPGA_TMFLT_BIT    bit;
}UNI_FPGA_TMFLT_REG; 

 
//用于计算FPGA系数的数据结构体
typedef struct{
    Uint16 EnFbChkErr;            //编码器UVW相逻辑错误
    Uint16 Ked;                 //D轴反电势常数，用于D轴反电动势补偿
    Uint16 Keq;                  //Q轴反电势常数，用于Q轴反电动势补偿
    Uint16 DcBusVolt;           //直流母线电压值
    Uint16 SpdToqRate;           //速度环和电流环采样频率比
    int32  OldElecAng;           //上周期电角度值
    Uint32 AngToCnt;             //当前电角度对应的计数值  每个电周期对应脉冲数
    Uint32 CntToAng_Q16;         //将以电周期脉冲总数为基值的脉冲数转换成以32768为基值的电角度值  
                                 //32768对应1个电周期的脉冲数 Q16格式
    Uint32 FPGA_MaxSpd;        //FPGA中定义的用于计算的最高转速 
//    int32 MaxSpdPos_OverWarn;      //正转过速警告速度值 
//    int32 MaxSpdNeg_OverWarn;      //反转过速警告速度值
    int32 MaxSpdPos_OverErr;         //正转过速报错速度值
    int32 MaxSpdNeg_OverErr;         //反转过速报错速度值
    int32 IqKpCoef_Q16;              //Q轴比例增益系数
    int32 IdKpCoef_Q16;              //D轴比例增益系数
    int32 KiCoef_Q16;                //积分增益系数

    int64   M_SpdCoff;                //M法测速系数值
}STR_FPGA_CALCULATE_VAR;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//运行命令，即运行中DSP提供给FPGA的参数
extern volatile Uint16 *HostSon;           //伺服ON/OFF 目前提供给SVPWM文件做PWM关断暂用
extern volatile Uint16 *HostAng;   //转子初始电角度
extern volatile Uint16 *HostPosRst;   //位置偏差计数器清零

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
extern Uint16 FPGA_IsFPGARdy(void);             //上电初始化判断与FPGA读写是否正常
extern void InitFPGA(void);                     //FPGA上电配置初始化
extern void FPGA_UToVCoff(void);                //UV相电流的校正,使相电流采样值在通同一直流时能保持平衡
extern void SetFPGAParam(void);                 //发送DSP相关数据至FPGA 
extern void GetFPGAParam(void);                 //得到FPGA中的状态参数   
extern void MainLoopStopUpdateFPGA(void);       //主循环更新FPGA相关控制参数
extern void MainLoopFPGAUpdate(void);           //主循环运行更新参数如PI电流环参数
extern void FPGA_PostErr(void);                 //得到FPGA的报警信号及处理
extern void Init_FPGAInterrupt(void);               //初始化FPGA 寄存器PWMPrd_Mode
extern void InitPeriodCtrlReg(void);

#ifdef __cplusplus
}
#endif

#endif /* MTR_FPGAINTERFACE_H */	

/********************************* END OF FILE *********************************/
