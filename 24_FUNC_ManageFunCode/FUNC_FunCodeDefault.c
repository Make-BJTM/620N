/*******************************************************************************
 深圳市汇川技术有限公司 版权所有(C)All rights reserved.            
 文件名:    FUNC_FunCodeDefault.c
 创建人：童文邹                创建日期：2008.10.09 
 修改人：王治国                创建日期：2011.11.15 
 描述:
    1. 
    2. 
缩写说明：
    1. attrib   ->  attribute
    2. deft     ->  default

 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_FunCodeDefault.h" 

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//属性相关宏定义
//bit.Writable   BIT:0-1 
#define     ANY_WRT     0                   //随时设定          ----默认
#define     POSD_WRT    1                   //停机时设定
#define     DISP_WRT    2                   //仅显示 只读
#define     RSVD_WRT    3                   //保留参数 只读
//bit.UpperLmt  BIT:3 
#define     DRCT_LMTH   0                   //上限 直接限制     ----默认
#define     RLAT_LMTH   ((Uint32)1 << 3)    //上限 关联限制
//bit.LowerLmt  BIT:4  
#define     DRCT_LMTL   0                   //下限 直接限制     ----默认
#define     RLAT_LMTL   ((Uint32)1 << 4)    //下限 关联限制
//bit.DataBits   BIT:5 
#define     ONE_WORD    0                   //16位数据          ----默认
#define     TWO_WORD    ((Uint32)1 << 5)    //32位数据
//bit.DispBits  BIT:6-9 
#define     DISP_1    ((Uint32)1 << 6)      //需要1段数码管显示 
#define     DISP_2    ((Uint32)2 << 6)      //需要2段数码管显示 
#define     DISP_3    ((Uint32)3 << 6)      //需要3段数码管显示 
#define     DISP_4    ((Uint32)4 << 6)      //需要4段数码管显示 
#define     DISP_5    ((Uint32)5 << 6)      //需要5段数码管显示 
#define     DISP_6    ((Uint32)6 << 6)      //需要6段数码管显示 
#define     DISP_7    ((Uint32)7 << 6)      //需要7段数码管显示 
#define     DISP_8    ((Uint32)8 << 6)      //需要8段数码管显示 
#define     DISP_9    ((Uint32)9 << 6)      //需要9段数码管显示 
#define     DISP_10   ((Uint32)10 << 6)     //需要10段数码管显示
#define     DISP_11   ((Uint32)11 << 6)     //需要11段数码管显示
#define     DISP_12   ((Uint32)12 << 6)     //需要12段数码管显示

//bit.DotBit  BIT:10-12 
#define     ZERO_DOT    0                      //无小数位       ----默认
#define     ONE_DOT     ((Uint32)1 << 10)      //1位小数位
#define     TWO_DOT     ((Uint32)2 << 10)      //2位小数位
#define     THREE_DOT   ((Uint32)3 << 10)      //3位小数位
#define     FOUR_DOT    ((Uint32)4 << 10)      //4位小数位

//bit.Sign  BIT:13    
#define     UN_SIGN     0                       //无符号        ----默认
#define     INT_SIGN    ((Uint32)1 << 13)       //有符号
//bit.Active  BIT:14 
#define     INST_ACT    0                       //立即生效      ----默认
#define     NEXT_ACT    ((Uint32)1 << 14)       //下次上电生效
//bit.DataType   BIT:15-16   
#define     YNUM_TYP    0                       //0:十进制数据  ----默认
#define     HNUM_TYP    ((Uint32)1 << 15)       //1:十六进制数据
#define     NNUM_TYP    ((Uint32)2 << 15)       //2:非数字数据(Di Do显示)

//bit.DataIndex   BIT:17
#define     LOW_WORD    0                       //0:32位数据低十六位或16位数据      ----默认
#define     HIGH_WORD   ((Uint32)1 << 17)       //1:32位数据高十六位
//bit.OEMProtect   BIT:18
#define     UN_OEMPROTECT   0                   //0:不用保护        ----默认
#define     OEMPROTECT      ((Uint32)1 << 18)   //1:厂家保护
//bit.PanelAttrib  BIT:19-20 
#define     PANEL_UNCHANGE   ((Uint32)1 << 19)  //面板不可更改
#define     PANEL_UNSAVE     ((Uint32)2 << 19)  //面板可更改功能码但是不存Eeprom
#define     PANEL_UNREAD     ((Uint32)3 << 19)  //密码类参数,不可以读取,操作后不存Eeprom 
//bit.CommSaveEn  BIT:21
#define     COMM_SAVE_EEPROM     0                       //0: COMM更改功能码后可以存储到EEPROM      ----默认
#define     COMM_UNSAVE_EEPROM   ((Uint32)1 << 21)       //1: COMM更改功能码后不存储到EEPROM
//bit.CommReadEn BIT:22
#define     COMM_READ_EN     0          //读时返回真实值
#define     COMM_READ_DISABVLE   ((Uint32)1 << 22)      //读时返回0

// DB制动 相关的功能码的范围

#define NO1_STOP_MODE_MAX  0
#define NO2_STOP_MODE_MAX  1


//JOG 最大转速设定为电机最大转速
#define JOG_SPD_MAX         GetCodeIndex(FunCodeUnion.code.MT_RateSpd)

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//功能码属性表(包含辅助功能码)
const STR_FUNCODE_DEFAULT   FunCodeDeft[FUNCODEDFTLEN] = {
//----------------------------伺服电机参数  H00LEN = 50 -------------------------------------------------------
/*  Uint16 MT_MotorModel;           //H00_00 电机型号 用户用来选择的当前电机参数组
    Uint16 MT_RsdMotorModel;        //H00_01 内部电机参数数组的唯一标识
    Uint16 MT_NonStandardVerL;      //H00_02 非标号L
    Uint16 MT_NonStandardVerH;      //H00_03 非标号H
    Uint16 MT_ABSEncVer;            //H00_04 编码器版本号
    Uint16 MT_RsdAbsRomMotorModel;  //H00_05 内部总线电机型号
    Uint16 MT_FpgaNonStandardVerL;  //H00_06 FPGA非标号L
    Uint16 MT_FpgaNonStandardVerH;  //H00_07 FPGA非标号H
    Uint16 MT_AbsEncType;           //H00_08 绝对编码器类型  14100-多圈    其它-单圈
    Uint16 MT_RateVolt;             //H00_09 额定电压                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if NONSTANDARD_PROJECT == IS600P
/*H00_00*/  6,              0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT,
#else
/*H00_00*/  14000,          0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT,
#endif
/*H00_01*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_02*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_5| TWO_WORD| TWO_DOT,
/*H00_03*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_5| TWO_WORD| TWO_DOT| HIGH_WORD, 
/*H00_04*/  0,              0,                  65535,              DISP_WRT|    DISP_5|   ONE_DOT,
/*H00_05*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H00_06*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_5| TWO_WORD| TWO_DOT,
/*H00_07*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_5| TWO_WORD| TWO_DOT| HIGH_WORD, 
/*H00_08*/  0,              0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT,
/*H00_09*/  0,              0,                  65535,              POSD_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,

/*  Uint16 MT_RatePower;        //H00_10 额定功率
    Uint16 MT_RateCurrent;      //H00_11 额定电流 / 连续电流
    Uint16 MT_RateToq;          //H00_12 额定转矩 / 连续推力
    Uint16 MT_MaxToqOrCur;      //H00_13 最大转矩 / 最大电流
    Uint16 MT_RateSpd;          //H00_14 额定转速
    Uint16 MT_MaxSpd;           //H00_15 最大转速
    Uint16 MT_Inertia;          //H00_16 转动惯量 / 动子质量
    Uint16 MT_PolePair;         //H00_17 永磁同步电机极对数
    Uint16 MT_StatResist;       //H00_18 定子电阻
    Uint16 MT_StatInductQ;      //H00_19 定子电感Lq                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if NONSTANDARD_PROJECT == LINEARMOT       //功率单位为W
/*H00_10*/  39,             0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT|OEMPROTECT,
/*H00_11*/  230,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_12*/  375,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_13*/  690,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_14*/  1900,           0,                  65535,              POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H00_15*/  1900,           0,                  65535,              POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H00_16*/  290,            0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT| OEMPROTECT,
/*H00_17*/  1,              0,                  65535,              POSD_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT,
/*H00_18*/ 4000,            0,                  65535,              POSD_WRT|    DISP_5| THREE_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_19*/  100,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
#else
/*H00_10*/  75,             0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_11*/  470,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_12*/  239,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_13*/  716,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_14*/  3000,           0,                  65535,              POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H00_15*/  6000,           0,                  65535,              POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H00_16*/  130,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_17*/  4,              0,                  65535,              POSD_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT,
/*H00_18*/  500,            0,                  65535,              POSD_WRT|    DISP_5| THREE_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_19*/  327,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
#endif
/*  Uint16 MT_StatInductD;      //H00_20 定子电感Ld
    Uint16 MT_RevEleCoe;        //H00_21 反电势系数0.01mv/rpm  /  V/m/s
    Uint16 MT_ToqCoe;           //H00_22 转矩系数Kt  / 推力常数  0.01N/A
    Uint16 MT_EleConst;         //H00_23 电气常数Te
    Uint16 MT_MachConst;        //H00_24 机械常数Tm  / 电机常数   0.01N/W-2
    Uint16 MT_Rsvd0025;         //H00_25 
    Uint16 MT_Rsvd0026;         //H00_26 
    Uint16 MT_Rsvd0027;         //H00_27
    Uint16 MT_ThetaOffsetL;     //H00_28 绝对式码盘位置偏置L 
    Uint16 MT_ThetaOffsetH;     //H00_29 绝对式码盘位置偏置H       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if NONSTANDARD_PROJECT == LINEARMOT
/*H00_20*/  100,            0,                  65535,              POSD_WRT| DISP_5| TWO_DOT| NEXT_ACT|OEMPROTECT,
/*H00_21*/  6700,           0,                  65535,              POSD_WRT| DISP_5| THREE_DOT| NEXT_ACT|OEMPROTECT,
/*H00_22*/  1650,           0,                  65535,              POSD_WRT| DISP_5| TWO_DOT| NEXT_ACT|OEMPROTECT,
/*H00_23*/  25,             0,                  65535,              POSD_WRT| DISP_5| TWO_DOT|NEXT_ACT|OEMPROTECT,
/*H00_24*/  600,            0,                  65535,              POSD_WRT| DISP_5| TWO_DOT|NEXT_ACT|OEMPROTECT,
#else
/*H00_20*/  387,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_21*/  3330,           0,                  65535,              POSD_WRT| DISP_5| TWO_DOT| NEXT_ACT|OEMPROTECT,
/*H00_22*/  51,             0,                  65535,              POSD_WRT| DISP_5| TWO_DOT| NEXT_ACT|OEMPROTECT,     
/*H00_23*/  654,            0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H00_24*/  24,             0,                  65535,              POSD_WRT|    DISP_5|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
#endif
/*H00_25*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_26*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_27*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_28*/ 8192,            0,                  65535,              POSD_WRT| DISP_11 | TWO_WORD |  NEXT_ACT| OEMPROTECT,
/*H00_29*/  0,              0,                  65535,              POSD_WRT| DISP_11 | TWO_WORD |  NEXT_ACT| OEMPROTECT | HIGH_WORD,

/*  Uint16 MT_EncoderSel;       //H00_30 编码器选择
    Uint16 MT_EncoderPensL;     //H00_31 编码器线数L  / 直线电机极距  mm
    Uint16 MT_EncoderPensH;     //H00_32 编码器线数H  / 光栅尺分辨率  0.01um
    Uint16 MT_InitTheta;        //H00_33 Z信号对应电角度
    Uint16 MT_UposedgeTheta;    //H00_34 U相上升沿对应电角度
    Uint16 MT_AbsRomMotorModel;     //H00_35 总线电机型号
    Uint16 MT_Rsvd36;           //H00_36
    Uint16 MT_AbsEncFunBit;         //H00_37 绝对编码器功能设置位
    Uint16 MT_Rsvd38;           //H00_38
    Uint16 MT_Rsvd39;           //H00_39                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if NONSTANDARD_PROJECT == IS600P
/*H00_30*/  0x01,           0,                  65535,             POSD_WRT|  HNUM_TYP|    DISP_3|  NEXT_ACT|OEMPROTECT,
#elif NONSTANDARD_PROJECT == LINEARMOT
/*H00_30*/  0x30,           0,                  65535,             POSD_WRT|  HNUM_TYP|    DISP_3|  NEXT_ACT|OEMPROTECT,
#else
/*H00_30*/  0x13,           0,                  65535,             POSD_WRT|  HNUM_TYP|    DISP_3|  NEXT_ACT|OEMPROTECT,
#endif

#if NONSTANDARD_PROJECT == IS600P
/*H00_31*/  2500,           0,                  65535,              POSD_WRT| TWO_WORD| DISP_10| NEXT_ACT|OEMPROTECT,
/*H00_32*/  0,              0,                  65535,              POSD_WRT| TWO_WORD| DISP_10| NEXT_ACT| HIGH_WORD|OEMPROTECT,
#elif NONSTANDARD_PROJECT == LINEARMOT
/*H00_31*/  16,             0,                  65535,              POSD_WRT| DISP_4| NEXT_ACT| OEMPROTECT,
/*H00_32*/  100,            0,                  65535,              POSD_WRT| DISP_4| NEXT_ACT| TWO_DOT | OEMPROTECT,
#else
/*H00_31*/  0,              0,                  65535,              POSD_WRT| TWO_WORD| DISP_10| NEXT_ACT|OEMPROTECT,
/*H00_32*/  0x10,           0,                  65535,              POSD_WRT| TWO_WORD| DISP_10| NEXT_ACT| HIGH_WORD|OEMPROTECT,
#endif

/*H00_33*/  1800,           0,                  3600,               POSD_WRT|    DISP_4|  ONE_DOT |  NEXT_ACT|OEMPROTECT,
/*H00_34*/  1800,           0,                  3600,               POSD_WRT|    DISP_4|  ONE_DOT |  NEXT_ACT|OEMPROTECT,
/*H00_35*/  0,              0,                  65535,              POSD_WRT|    DISP_5, 
/*H00_36*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_37*/  0,              0,                  65535,              POSD_WRT|  HNUM_TYP|    DISP_2, 
/*H00_38*/  0,              0,                  65535,              RSVD_WRT, 
/*H00_39*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 MT_Rsvd40;               //H00_40
    Uint16 MT_Rsvd41;               //H00_41
    Uint16 MT_Rsvd43;               //H00_43
    Uint16 MT_Rsvd44;               //H00_44
    Uint16 MT_Rsvd45;               //H00_45
    Uint16 MT_Rsvd46;               //H00_46
    Uint16 MT_Rsvd47;               //H00_47
    Uint16 MT_Rsvd48;               //H00_48
    Uint16 MT_Rsvd49;               //H00_49              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H00_40*/  0,              0,                  65535,              RSVD_WRT,
/*H00_41*/  0,              0,                  65535,              RSVD_WRT,
/*H00_42*/  0,              0,                  65535,              RSVD_WRT,
/*H00_43*/  0,              0,                  65535,              RSVD_WRT,
/*H00_44*/  0,              0,                  65535,              RSVD_WRT,
/*H00_45*/  0,              0,                  65535,              RSVD_WRT,
/*H00_46*/  1,              0,                  65535,              RSVD_WRT,
/*H00_47*/  0,              0,                  65535,              RSVD_WRT,
/*H00_48*/  0,              0,                  65535,              RSVD_WRT,
/*H00_49*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 MT_Rsvd50;               //H00_50
    Uint16 MT_Rsvd51;               //H00_51
    Uint16 MT_Rsvd52;               //H00_52
    Uint16 MT_Rsvd53;               //H00_53
    Uint16 MT_Rsvd54;               //H00_54
    Uint16 MT_Rsvd55;               //H00_55
    Uint16 MT_Rsvd56;               //H00_56
    Uint16 MT_Rsvd57;               //H00_57
    Uint16 MT_Rsvd58;               //H00_58
    Uint16 MemCheck00;              //H00_59 组校验字          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H00_50*/  0,              0,                  65535,              RSVD_WRT,
/*H00_51*/  0,              0,                  65535,              RSVD_WRT,
/*H00_52*/  0,              0,                  65535,              RSVD_WRT,
/*H00_53*/  0,              0,                  65535,              RSVD_WRT,
/*H00_54*/  0,              0,                  65535,              RSVD_WRT,
/*H00_55*/  0,              0,                  65535,              RSVD_WRT,
/*H00_56*/  0,              0,                  65535,              RSVD_WRT,
/*H00_57*/  0,              0,                  65535,              RSVD_WRT,
/*H00_58*/  0,              0,                  65535,              RSVD_WRT,
/*H00_59*/  0x0A5C,         0,                  65535,              RSVD_WRT,

//----------------------------驱动器参数  H01LEN = 70 -------------------------------------------------------
/*  Uint16 OEM_SoftVersion;         //H01_00 软件版本号  不跟驱动器关联   不存储在Eeprom中
    Uint16 OEM_FpgaVersion;         //H01_01 FPGA软件版本号  不跟驱动器关联
    Uint16 OEM_ServoSeri;           //H01_02 伺服驱动系列号
    Uint16 OEM_RsdServoSeri;        //H01_03 保留的用于比较的驱动器型号
    Uint16 OEM_VoltClass;           //H01_04 电压级
    Uint16 OEM_RatePower;           //H01_05 额定功率
    Uint16 OEM_MaxPowerOut;         //H01_06 最大输出功率
    Uint16 OEM_RateCurrent;         //H01_07 驱动器额定输出电流
    Uint16 OEM_MaxCurrentOut;       //H01_08 驱动器最大输出电流
    Uint16 OEM_SdmClkEnbl;          //H01_09 Σ-Δ调制器外部时钟使能      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                    属性*/
/*H01_00*/  0,              0,                  65535,              DISP_WRT|    DISP_5|   ONE_DOT,
/*H01_01*/  0,              0,                  65535,              DISP_WRT|    DISP_5|   ONE_DOT,
#if POWERDRIVER_TYPE==POWDRV_IS650
/*H01_02*/  20005,          0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT,
#else
/*H01_02*/  5,              0,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT,
#endif
/*H01_03*/  5,              0,                  65535,              RSVD_WRT, 
/*H01_04*/  220,            0,                  65535,              DISP_WRT|    DISP_5|OEMPROTECT,
/*H01_05*/  75,             1,                  65535,              DISP_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_06*/  75,             1,                  65535,              DISP_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_07*/  550,            1,                  65535,              DISP_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_08*/  1690,           1,                  65535,              DISP_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_09*/  0,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,

/*  Uint16 OEM_CarrWaveFreq;        //H01_10 载波频率
    Uint16 OEM_ToqLoopFreqSel;      //H01_11 电流环调制频率选择
    Uint16 OEM_SpdLoopFreqScal;     //H01_12 速度环调度分频系数
    Uint16 OEM_PosLoopFreqScal;     //H01_13 位置环调度分频系数
    Uint16 OEM_PwmDeadT;            //H01_14 死区时间
    Uint16 OEM_UdcOver;             //H01_15 直流母线过压保护点
    Uint16 OEM_UdcLeak;             //H01_16 直流母线电压泄放点
    Uint16 OEM_UdcLow;              //H01_17 直流母线电压欠压点
    Uint16 OEM_OCProtectPoint;      //H01_18 驱动器过流保护点
    Uint16 IS_7860Gain;             //H01_19 7860采样系数                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                    属性*/
/*H01_10*/  8000,           4000,               20000,              POSD_WRT|    DISP_5|  NEXT_ACT|OEMPROTECT,
/*H01_11*/  1,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,
/*H01_12*/  1,              1,                  32,                 POSD_WRT|    DISP_2|  NEXT_ACT|OEMPROTECT,
/*H01_13*/  4,              2,                  128,                POSD_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT,
/*H01_14*/  200,            1,                  2000,               POSD_WRT|    DISP_4|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H01_15*/  420,            0,                  900,                POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H01_16*/  380,            0,                  900,                POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H01_17*/  200,            0,                  900,                POSD_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H01_18*/  100,            10,                 100,                POSD_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT,
/*H01_19*/  3200,           1,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT|OEMPROTECT,

/*  Uint16 OEM_DeadComp;            //H01_20 死区补偿量 
    Uint16 OEM_Rsvd21;              //H01_21
    Uint16 OEM_KedGain;             //H01_22 D轴反电动势补偿系数
    Uint16 OEM_KeqGain;             //H01_23 Q轴反电动势补偿系数
    Uint16 OEM_CurIdKpSec;          //H01_24 D轴电流环比例增益
    Uint16 OEM_CurIdKiSec;          //H01_25 D轴电流环积分补偿因子
    Uint16 OEM_Decimation;          //H01_26 电流采样Sinc3滤波器数据抽取率
    Uint16 OEM_CurIqKpSec;          //H01_27 Q轴电流环比例增益
    Uint16 OEM_CurIqKiSec;          //H01_28 Q轴电流环积分补偿因子
    Uint16 OEM_Rsvd29;              //H01_29                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                    属性*/
/*H01_20*/  200,            0,                  2000,                ANY_WRT|    DISP_4|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H01_21*/  0,              0,                  65535,              RSVD_WRT,
/*H01_22*/  600,            0,                  65535,               ANY_WRT|    DISP_5|   ONE_DOT|OEMPROTECT,
/*H01_23*/  1000,           0,                  65535,               ANY_WRT|    DISP_5|   ONE_DOT|OEMPROTECT,
/*H01_24*/  1000,           0,                  65535,               ANY_WRT|    DISP_5|OEMPROTECT,
/*H01_25*/  200,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H01_26*/  2,              0,                  3,                   ANY_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,
#else
/*H01_26*/  0,              0,                  3,                   ANY_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,
#endif
/*H01_27*/  1000,           0,                  65535,               ANY_WRT|    DISP_5|OEMPROTECT,
/*H01_28*/  100,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 OEM_UdcGain;             //H01_30 母线电压增益调整
    Uint16 OEM_FocCalTime;          //H01_31 FOC计算时间
    Uint16 OEM_V2UCalCoff;          //H01_32 UV采样相对增益
    Uint16 OEM_HighPrecisionAIEn;   //H01_33 高精度AI使能     手册和后台不公开
    Uint16 ErrMsgSaveDisable;       //H01_34 故障记录存储开关 0 存储 1 不存储    手册和后台不公开
    Uint16 OEM_LocalModeEn;         //H01_35 
    Uint16 OEM_Rsvd36;              //H01_36 
    Uint16 OEM_Rsvd37;              //H01_37 
    Uint16 OEM_Rsvd38;              //H01_38 
    Uint16 OEM_Rsvd39;              //H01_39            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H01_30*/  1000,           500,                1500,               POSD_WRT|    DISP_4|   ONE_DOT|OEMPROTECT,
/*H01_31*/  260,            100,                10000,               ANY_WRT|    DISP_4|  NEXT_ACT|OEMPROTECT,
/*H01_32*/  32768,          1,                  65535,              POSD_WRT|    DISP_5|  NEXT_ACT|OEMPROTECT,
/*H01_33*/  0,              0,                  2,                   ANY_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT,
/*H01_34*/  0,              0,                  1,                  DISP_WRT|    DISP_1|OEMPROTECT,
/*H01_35*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H01_36*/  0,              0,                  65535,              RSVD_WRT,
/*H01_37*/  0,              0,                  65535,              RSVD_WRT,
/*H01_38*/  0,              0,                  65535,              RSVD_WRT,
/*H01_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 OEM_Rsvd40;              //H01_40 
    Uint16 OEM_Rsvd41;              //H01_41 
    Uint16 OEM_Rsvd42;              //H01_42 
    Uint16 OEM_Rsvd43;              //H01_43 
    Uint16 OEM_SecDecimation;       //H01_44 第二组电流采样Sinc3滤波器数据抽取率
    Uint16 OEM_DutyU;               //H01_45 电压注入时得到的U相占空比
    Uint16 OEM_SrchCurFreq;         //H01_46 电压注入辨识电角度时得到的载频设置，不开放
    Uint16 OEM_IqCalTm;             //H01_47 MCU电流指令处理时间
    Uint16 OEM_ADSamDly;            //H01_48 电流检测回路延时
    Uint16 OEM_AbsTransDelay;       //H01_49 总线编码器数据传播延时   0.01us             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H01_40*/  0,              0,                  65535,              RSVD_WRT,
/*H01_41*/  0,              0,                  65535,              RSVD_WRT,
/*H01_42*/  0,              0,                  65535,              RSVD_WRT,
/*H01_43*/  0,              0,                  65535,              RSVD_WRT,
/*H01_44*/  2,              0,                  3,                   ANY_WRT|    DISP_1|  NEXT_ACT|OEMPROTECT, 
/*H01_45*/  0,              0,                  65535,              POSD_WRT|    DISP_5| OEMPROTECT ,
/*H01_46*/  8000,           2000,               20000,              POSD_WRT|    DISP_5| OEMPROTECT ,
/*H01_47*/  5500,           0,                  6000,               POSD_WRT|    DISP_4| TWO_DOT| NEXT_ACT| OEMPROTECT,
/*H01_48*/  100,      (Uint16)-4000,             4000,              POSD_WRT|    DISP_4| TWO_DOT| INT_SIGN|NEXT_ACT| OEMPROTECT,
/*H01_49*/  6100,           0,                  50000,              POSD_WRT|    DISP_5| TWO_DOT| NEXT_ACT| OEMPROTECT, 

/*  Uint16 OEM_DSPVerBD;             //H01_50 DSP软件内部版本号
    Uint16 OEM_FPGAVerBD;            //H01_51 FPGA软件内部版本号
    Uint16 OEM_CapIdKp;              //H01_52 性能优先模式D轴比例增益
    Uint16 OEM_CapIdKi;              //H01_53 性能优先模式D轴积分补偿因子
    Uint16 OEM_CapIqKp;              //H01_54 性能优先模式Q轴比例增益
    Uint16 OEM_CapIqKi;              //H01_55 性能优先模式Q轴积分补偿因子
    Uint16 OEM_CapSecKpCoff;         //H01_56 性能优先模式第二组比例增益系数
    Uint16 OEM_CapThirdKpCoff;       //H01_57 性能优先模式第三组比例增益系数
    Uint16 OEM_FirSwtchIq;           //H01_58 性能优先模式第一增益切换点
    Uint16 OEM_SecSwtchIq;           //H01_59 性能优先模式第二增益切换点*/
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H01_50*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   TWO_DOT|OEMPROTECT,
/*H01_51*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   TWO_DOT|OEMPROTECT,
/*H01_52*/  2000,           0,                  65535,               ANY_WRT|    DISP_5|OEMPROTECT,  
/*H01_53*/  200,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT|OEMPROTECT, 
/*H01_54*/  2000,           0,                  65535,               ANY_WRT|    DISP_5|OEMPROTECT, 
/*H01_55*/  100,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT|OEMPROTECT,
/*H01_56*/  1024,           0,                  10000,               ANY_WRT|    DISP_5|   ONE_DOT|OEMPROTECT, 
/*H01_57*/  1024,           0,                  10000,               ANY_WRT|    DISP_4|   ONE_DOT|OEMPROTECT,
/*H01_58*/  10,             0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT|OEMPROTECT,
/*H01_59*/  20,             0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT|OEMPROTECT,

/*  Uint16 OEM_ThirdSwtchIq;        //H01_60 性能优先模式第三增益切换点
    Uint16 OEM_FourthSwtchIq;       //H01_61 性能优先模式第四增益切换点
    Uint16 OEM_OvrCurUV;            //H01_62 U V相7860检测保护点
    Uint16 OEM_AbsTransCompTime;    //H01_63 总线编码器数据传输补偿时间 
    Uint16 OEM_HDHAbsTstTime;       //H01_64 海德汉编码器恢复时间 0.001us
    Uint16 OEM_HDHEncBaud;          //H01_65 海德汉编码器通讯频率 kHz
    Uint16 OEM_EncCmdTransLineDelay;//H01_66 总线编码器命令字传输线延时 0.01us
    Uint16 OEM_Rsvd67;              //H01_67 
    Uint16 OEM_Rsvd68;              //H01_68
    Uint16 MemCheck01;              //H01_69 组校验字         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H01_60*/  1000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT|OEMPROTECT, 
/*H01_61*/  2000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT|OEMPROTECT, 
#if POWERDRIVER_TYPE==POWDRV_IS650
/*H01_62*/  424,            0,                  1000,                ANY_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT,
#else 
/*H01_62*/  280,            0,                  320,                 ANY_WRT|    DISP_3|  NEXT_ACT|OEMPROTECT, 
#endif                                                 
/*H01_63*/  0,              0,                  1000,               POSD_WRT|    DISP_4|   TWO_DOT|  NEXT_ACT|OEMPROTECT,
/*H01_64*/  1000,           0,                  40000,               ANY_WRT|    DISP_5| THREE_DOT|  NEXT_ACT|OEMPROTECT,
/*H01_65*/  2000,           0,                  10000,               ANY_WRT|    DISP_5|  NEXT_ACT|OEMPROTECT, 
/*H01_66*/  0,              0,                  200,                POSD_WRT|    DISP_3|   TWO_DOT|  NEXT_ACT|OEMPROTECT, 
/*H01_67*/  0,              0,                  65535,              RSVD_WRT,
/*H01_68*/  0,              0,                  65535,              RSVD_WRT, 
/*H01_69*/  0x1A5C,         0,                  65535,              RSVD_WRT, 


//----------------------------基本控制参数  H02LEN = 50 -------------------------------------------------------
/*  Uint16 BP_ModeSelet;            //H02_00 模式选择
    Uint16 BP_AbsPosDetectionSel;   //H02_01 绝对位置检测系统选择
    Uint16 BP_RevlDir;              //H02_02 指令方向选择-1:控制位置指令 速度指令 转矩指令乘以负一
    Uint16 BP_PulsFdbDir;           //H02_03 速度反馈方向选择-1:控制速度反馈乘以负一
    Uint16 BP_MinMeasureSpd;        //H02_04 最小速度设置
    Uint16 BP_StopSoff;             //H02_05 伺服OFF停止方式选择
    Uint16 BP_StopNo2;              //H02_06 故障停止方式No.2选择
    Uint16 BP_OPStop;               //H02_07 超程停止方式
    Uint16 BP_StopNo1;              //H02_08 故障停机方式No.1选择 
    Uint16 BP_SonBrkDelay;          //H02_09 上电伺服On时抱闸打开释放延时             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if ECT_ENABLE_SWITCH
/*H02_00*/  9,              0,                  9,                  POSD_WRT|    DISP_1,
#else
/*H02_00*/  1,              0,                  8,                  POSD_WRT|    DISP_1,
#endif
/*H02_01*/  0,              0,                  3,                  POSD_WRT|    DISP_1|  NEXT_ACT, 
/*H02_02*/  0,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT,
/*H02_03*/  0,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H02_04*/  10,             1,                  100,                POSD_WRT|    DISP_3|   ONE_DOT|  NEXT_ACT,
#else
/*H02_04*/  40,             2,                  140,                POSD_WRT|    DISP_3|   ONE_DOT|  NEXT_ACT,
#endif
/*H02_05*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H02_06*/  0,              0,                  NO2_STOP_MODE_MAX,  POSD_WRT|    DISP_1,
/*H02_07*/  1,              0,                  2,                  POSD_WRT|    DISP_1,
/*H02_08*/  0,              0,                  NO1_STOP_MODE_MAX,  POSD_WRT|    DISP_1,
/*H02_09*/  250,            0,                  500,                 ANY_WRT|    DISP_3,

/*  Uint16 BP_Clasp2OffTime;        //H02_10 抱闸指令--伺服OFF延迟时间
    Uint16 BP_ClaspSpdLmt;          //H02_11 抱闸指令输出速度限制值
    Uint16 BP_Off2ClaspTime;        //H02_12 伺服OFF-抱闸指令等待时间
    Uint16 BP_SpdCmp;               //H02_13 转速测量及比较门限，2倍频或4倍频测速
    Uint16 BP_StopModStateCutSpd;   //H02_14 停机方式和停机状态切换速度条件值 (内部功能码，不对外公开)
    Uint16 BP_AlmDispSel;           //H02_15 LED警告显示选择
    Uint16 BP_OTStopAuxFun;         //H02_16 超程停机时锁存辅助处理  (内部功能码，不对外公开)
    Uint16 BP_PowOffZeroSpdStopEn;  //H02_17 掉电零速停机使能
    Uint16 BP_SonFltTime;           //H02_18 伺服on信号滤波时间 
    Uint16 BP_SonBrkDelayTime;      //H02_19 伺服On上电抱闸延时打开时间*/
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H02_10*/  150,            1,                  1000,                ANY_WRT|    DISP_4,
/*H02_11*/   30,            0,                  3000,                ANY_WRT|    DISP_4,
/*H02_12*/  500,            1,                  1000,                ANY_WRT|    DISP_4,
/*H02_13*/  100,            0,                  3000,               POSD_WRT|    DISP_4|  NEXT_ACT,
/*H02_14*/  100,            10,                 6000,                POSD_WRT|    DISP_4,
/*H02_15*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H02_16*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H02_17*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H02_18*/  0,              0,                  64,                 POSD_WRT|    DISP_2,
/*H02_19*/  0,              0,                  1000,               POSD_WRT|    DISP_4,

/*  Uint16 BP_Rsvd20;               //H02_20
    Uint16 BP_RBMinOhm;             //H02_21 驱动器允许的能耗电阻最小值
    Uint16 BP_RBPSizeI;             //H02_22 内置能耗电阻功率容量
    Uint16 BP_RBOhmI;               //H02_23 内置能耗电阻阻值
    Uint16 BP_RBEfficiency;         //H02_24 电阻散热系数
    Uint16 BP_RBChoice;             //H02_25 能耗电阻设置
    Uint16 BP_RBPSizeO;             //H02_26 外置能耗电阻功率容量
    Uint16 BP_RBOhmO;               //H02_27 外置能耗电阻阻值
    Uint16 BP_Rsvd28;               //H02_28 
    Uint16 BP_UserPass_Rsvd;        //H02_29 用户密码锁存             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H02_20*/  0,              0,                  65535,              RSVD_WRT, 
/*H02_21*/  40,             1,                  1000,               DISP_WRT|    DISP_4,
/*H02_22*/  40,             1,                  65535,              DISP_WRT|    DISP_5,
/*H02_23*/  50,             1,                  1000,               DISP_WRT|    DISP_4,
/*H02_24*/  30,             10,                 100,                POSD_WRT|    DISP_3, 
/*H02_25*/  0,              0,                  3,                  POSD_WRT|    DISP_1,
/*H02_26*/  40,             1,                  65535,              POSD_WRT|    DISP_5,
/*H02_27*/  50,             1,                  1000,               POSD_WRT|    DISP_4,
/*H02_28*/  0,              0,                  65535,              RSVD_WRT,
/*H02_29*/  0,              0,                  65535,              RSVD_WRT,/*用户密码锁存*/

/*  Uint16 BP_UserPass;             //H02_30 用户密码
    Uint16 BP_InitServo;            //H02_31 系统参数初始化
    Uint16 BP_DefaultDisplayCode;   //H02_32 H0B功能码选择 (用于面板监控模式下显示)
    Uint16 BP_EtherCatVer;          //H02_33 EtherCat软件版本号
    Uint16 BP_CanVer;               //H02_34 CAN软件版本号
    Uint16 BP_PanelDisFreq;         //H02_35 面板数据刷新频率
    Uint16 BP_ESIVER;               //H02_36
    Uint16 BP_StopModStateCutSpd2;  //H02_37 停机方式和停机状态切换速度条件值 (内部功能码，不对外公开)
    Uint16 BP_Rsvd38;               //H02_38
    Uint16 BP_Rsvd39;               //H02_39               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H02_30*/  0,              0,                  65535,              RSVD_WRT|    DISP_5,
/*H02_31*/  0,              0,                  2,                  POSD_WRT|    DISP_1|PANEL_UNSAVE|COMM_UNSAVE_EEPROM,
/*H02_32*/  50,             0,                  99,                  ANY_WRT|    DISP_2,
#if ECT_ENABLE_SWITCH
/*H02_33*/  0,              0,                  65535,              DISP_WRT|    DISP_5|    FOUR_DOT, 
/*H02_34*/  0,              0,                  65535,              RSVD_WRT, 
#else
/*H02_33*/  0,              0,                  65535,              RSVD_WRT, 
/*H02_34*/  0,              0,                  65535,              DISP_WRT|    DISP_5|    TWO_DOT, 
#endif
/*H02_35*/  0,              0,                  20,                 ANY_WRT|   DISP_2,
/*H02_36*/  23,              0,                  65535,             DISP_WRT|  DISP_5| ONE_DOT, 
/*H02_37*/  6000,            10,                 6000,              POSD_WRT|  DISP_4,
/*H02_38*/  0,              0,                  65535,              RSVD_WRT, 
/*H02_39*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 MT_EnVisable;            //H02_40 电机组参数可见使能
    Uint16 OEM_OEMPass;             //H02_41 厂家密码
    Uint16 BP_Rsvd42;               //H02_42
    Uint16 BP_Rsvd43;               //H02_43
    Uint16 BP_Rsvd44;               //H02_44
    Uint16 BP_Rsvd45;               //H02_45
    Uint16 BP_Rsvd46;               //H02_46
    Uint16 BP_Rsvd47;               //H02_47
    Uint16 BP_Rsvd48;               //H02_48
    Uint16 MemCheck02;              //H02_49 组校验字                        */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H02_40*/  0,              0,                  65535,               ANY_WRT|    DISP_5|PANEL_UNREAD|COMM_UNSAVE_EEPROM,
/*H02_41*/  0,              0,                  65535,               ANY_WRT|    DISP_5|PANEL_UNREAD|COMM_UNSAVE_EEPROM|COMM_READ_DISABVLE,
/*H02_42*/  0,              0,                  65535,              RSVD_WRT,
/*H02_43*/  0,              0,                  65535,              RSVD_WRT,
/*H02_44*/  0,              0,                  65535,              RSVD_WRT,
/*H02_45*/  0,              0,                  65535,              RSVD_WRT,
/*H02_46*/  0,              0,                  65535,              RSVD_WRT,
/*H02_47*/  0,              0,                  65535,              RSVD_WRT,
/*H02_48*/  0,              0,                  65535,              RSVD_WRT,
/*H02_49*/  0x2A5C,         0,                  65535,              RSVD_WRT,


//----------------------------端子输入参数  H03LEN = 100 -------------------------------------------------------
/*  Uint16 DIDeftL;                 //H03_00 DI信号未分配默认状态 低16位低字节
    Uint16 DIDeftH;                 //H03_01 DI信号未分配默认状态 低16位高字节
    Uint16 DIFuncSel1;              //H03_02 DI1端子功能选择
    Uint16 DILogicSel1;             //H03_03 DI1端子逻辑选择
    Uint16 DIFuncSel2;              //H03_04 DI2端子功能选择
    Uint16 DILogicSel2;             //H03_05 DI2端子逻辑选择
    Uint16 DIFuncSel3;              //H03_06 DI3端子功能选择
    Uint16 DILogicSel3;             //H03_07 DI3端子逻辑选择
    Uint16 DIFuncSel4;              //H03_08 DI4端子功能选择
    Uint16 DILogicSel4;             //H03_09 DI4端子逻辑选择            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_00*/  0,              0,                  65535,                ANY_WRT|  HNUM_TYP|    DISP_4|  NEXT_ACT,
/*H03_01*/  0,              0,                  65535,                ANY_WRT|  HNUM_TYP|    DISP_4|  NEXT_ACT,
/*H03_02*/  14,             0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_03*/  0,              0,                  4,                    ANY_WRT|    DISP_1,
/*H03_04*/  15,             0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_05*/  0,              0,                  4,                    ANY_WRT|    DISP_1,
/*H03_06*/  0,             0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_07*/  0,              0,                  4,                    ANY_WRT|    DISP_1,
/*H03_08*/  0,              0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_09*/  0,              0,                  4,                    ANY_WRT|    DISP_1,

/*  Uint16 DIFuncSel5;              //H03_10 DI5端子功能选择
    Uint16 DILogicSel5;             //H03_11 DI5端子逻辑选择
    Uint16 DIFuncSel6;              //H03_12 DI6端子功能选择
    Uint16 DILogicSel6;             //H03_13 DI6端子逻辑选择
    Uint16 DIFuncSel7;              //H03_14 DI7端子功能选择
    Uint16 DILogicSel7;             //H03_15 DI7端子逻辑选择
    Uint16 DIFuncSel8;              //H03_16 DI8端子功能选择
    Uint16 DILogicSel8;             //H03_17 DI8端子逻辑选择
    Uint16 DIFuncSel9;              //H03_18 DI9端子功能选择
    Uint16 DILogicSel9;             //H03_19 DI9端子逻辑选择                */ 
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_10*/  0,              0,                   DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_11*/  0,               0,                   4,                    ANY_WRT|    DISP_1,
/*H03_12*/  0,               0,                   DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_13*/  0,               0,                   4,                    ANY_WRT|    DISP_1,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H03_14*/  0,              0,                  65535,                RSVD_WRT,
/*H03_15*/  0,              0,                  65535,                RSVD_WRT,
#else
/*H03_14*/  3,              0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_15*/  0,              0,                  4,                    ANY_WRT|    DISP_1,
#endif
/*H03_16*/  38,             0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_17*/  1,              0,                  4,                    ANY_WRT|    DISP_1,
/*H03_18*/  31,              0,                  DI_FUNC_SEL_MAX,      ANY_WRT|    DISP_2,
/*H03_19*/  0,              0,                  4,                    ANY_WRT|    DISP_1,

/*  Uint16 DIFuncSel10;             //H03_20 DI10端子功能选择
    Uint16 DILogicSel10;            //H03_21 DI10端子逻辑选择
    Uint16 DI_Rsvd22;               //H03_22 DI11端子功能选择
    Uint16 DI_Rsvd23;               //H03_23 DI11端子逻辑选择
    Uint16 DI_Rsvd24;               //H03_24 DI12端子功能选择
    Uint16 DI_Rsvd25;               //H03_25 DI12端子逻辑选择
    Uint16 DI_Rsvd26;               //H03_26 DI13端子功能选择
    Uint16 DI_Rsvd27;               //H03_27 DI13端子逻辑选择
    Uint16 DI_Rsvd28;               //H03_28 DI14端子功能选择
    Uint16 DI_Rsvd29;               //H03_29 DI14端子逻辑选择               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_20*/  0,              0,                  65535,               ANY_WRT|    DISP_1,
/*H03_21*/  0,              0,                  65535,               ANY_WRT|    DISP_1,
/*H03_22*/  0,              0,                  65535,              RSVD_WRT,
/*H03_23*/  0,              0,                  65535,              RSVD_WRT,
/*H03_24*/  0,              0,                  65535,              RSVD_WRT,
/*H03_25*/  0,              0,                  65535,              RSVD_WRT,
/*H03_26*/  0,              0,                  65535,              RSVD_WRT,
/*H03_27*/  0,              0,                  65535,              RSVD_WRT,
/*H03_28*/  0,              0,                  65535,              RSVD_WRT,
/*H03_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 DI_Rsvd30;               //H03_30 DI15端子功能选择
    Uint16 DI_Rsvd31;               //H03_31 DI15端子逻辑选择
    Uint16 DI_Rsvd32;               //H03_32 DI16端子功能选择
    Uint16 DI_Rsvd33;               //H03_33 DI16端子逻辑选择
    Uint16 DIDeftHL;                //H03_34 DI信号未分配默认状态 高16位低字节
    Uint16 DIDeftHH;                //H03_35 DI信号未分配默认状态 高16位高字节
    Uint16 DI_Rsvd36;               //H03_36
    Uint16 DI_Rsvd37;               //H03_37
    Uint16 DI_Rsvd38;               //H03_38
    Uint16 DI_Rsvd39;               //H03_39                            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_30*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_31*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_32*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_33*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_34*/  0,              0,                  65535,              ANY_WRT|  HNUM_TYP|    DISP_4|  NEXT_ACT,
/*H03_35*/  0,              0,                  65535,              ANY_WRT|  HNUM_TYP|    DISP_4|  NEXT_ACT,
/*H03_36*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_37*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_38*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_39*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 DI_Rsvd41;               //H03_41
    Uint16 DI_Rsvd42;               //H03_42
    Uint16 DI_Rsvd43;               //H03_43
    Uint16 DI_Rsvd44;               //H03_44
    Uint16 DI_Rsvd45;               //H03_45
    Uint16 DI_Rsvd46;               //H03_46
    Uint16 DI_Rsvd47;               //H03_47
    Uint16 DI_Rsvd48;               //H03_48
    Uint16 DI_Rsvd49;               //H03_49                            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_40*/  0,              0,                  65535,              RSVD_WRT,
/*H03_41*/  0,              0,                  65535,              RSVD_WRT,
/*H03_42*/  0,              0,                  65535,              RSVD_WRT,
/*H03_43*/  0,              0,                  65535,              RSVD_WRT,
/*H03_44*/  0,              0,                  65535,              RSVD_WRT,
/*H03_45*/  0,              0,                  65535,              RSVD_WRT,
/*H03_46*/  0,              0,                  65535,              RSVD_WRT,
/*H03_47*/  0,              0,                  65535,              RSVD_WRT,
/*H03_48*/  0,              0,                  65535,              RSVD_WRT,
/*H03_49*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 AI1_Offset;              //H03_50 AI1偏置                1mV
    Uint16 AI1_InFltTime;           //H03_51 AI1输入滤波时间        0.01ms
    Uint16 AI1_MidFltrEn;           //H03_52 AI1输入中值滤波使能
    Uint16 AI1_DeadT;               //H03_53 AI1死区                0.1mV
    Uint16 Ai1_ZeroDrift;           //H03_54 AI1零漂                0.1mV
    Uint16 AI2_Offset;              //H03_55 AI2偏置                1mV
    Uint16 AI2_InFltTime;           //H03_56 AI2输入滤波时间        0.01ms
    Uint16 AI2_MidFltrEn;           //H03_57 AI2输入中值滤波使能
    Uint16 AI2_DeadT;               //H03_58 AI2死区                0.1mV
    Uint16 Ai2_ZeroDrift;           //H03_59 AI2零漂                0.1mV                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H03_50*/  0,              0,                  65535,              RSVD_WRT,
/*H03_51*/  0,              0,                  65535,              RSVD_WRT,
/*H03_52*/  0,              0,                  65535,              RSVD_WRT,
/*H03_53*/  0,              0,                  65535,              RSVD_WRT,
/*H03_54*/  0,              0,                  65535,              RSVD_WRT,
/*H03_55*/  0,              0,                  65535,              RSVD_WRT,
/*H03_56*/  0,              0,                  65535,              RSVD_WRT,
/*H03_57*/  0,              0,                  65535,              RSVD_WRT,
/*H03_58*/  0,              0,                  65535,              RSVD_WRT,
/*H03_59*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H03_50*/  0,              (Uint16)-5000,      5000,                ANY_WRT|    DISP_5|  INT_SIGN,
/*H03_51*/  200,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT,
/*H03_52*/  1,              0,                  1,                   ANY_WRT|    DISP_1,
/*H03_53*/  100,            0,                  10000,               ANY_WRT|    DISP_5|   ONE_DOT,
/*H03_54*/  0,              (Uint16)-5000,      5000,                ANY_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
/*H03_55*/  0,              (Uint16)-5000,      5000,                ANY_WRT|    DISP_5|  INT_SIGN,
/*H03_56*/  200,            0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT,
/*H03_57*/  1,              0,                  1,                   ANY_WRT|    DISP_1,
/*H03_58*/  100,            0,                  10000,               ANY_WRT|    DISP_5|   ONE_DOT,
/*H03_59*/  0,              (Uint16)-5000,      5000,                ANY_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
#endif
/*  Uint16 AI3_Offset;              //H03_60 AI3偏置                1mV     (保留)
    Uint16 AI3_InFltTime;           //H03_61 AI3输入滤波时间        0.01ms  (保留)
    Uint16 AI3_MidFltrEn;           //H03_62 AI3输入中值滤波使能            (保留)
    Uint16 AI3_DeadT;               //H03_63 AI3死区                0.1mV   (保留)
    Uint16 Ai3_ZeroDrift;           //H03_64 AI3零漂                0.1mV   (保留)
    Uint16 AI_Rsvd65;               //H03_65
    Uint16 AI_Rsvd66;               //H03_66
    Uint16 AI_Rsvd67;               //H03_67
    Uint16 AI_Rsvd68;               //H03_68 
    Uint16 AI_Rsvd69;               //H03_69                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H03_60*/  0,              0,                  65535,              RSVD_WRT,
/*H03_61*/  0,              0,                  65535,              RSVD_WRT,
/*H03_62*/  0,              0,                  65535,              RSVD_WRT,
/*H03_63*/  0,              0,                  65535,              RSVD_WRT,
/*H03_64*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H03_60*/  0,              (Uint16)-5000,      5000,               RSVD_WRT|    DISP_5|  INT_SIGN,
/*H03_61*/  200,            0,                  65535,              RSVD_WRT|    DISP_5|   TWO_DOT,
/*H03_62*/  1,              0,                  1,                  RSVD_WRT|    DISP_1,
/*H03_63*/  100,            0,                  10000,               ANY_WRT|    DISP_5,
/*H03_64*/  0,              (Uint16)-5000,      5000,               RSVD_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
#endif
/*H03_65*/  0,              0,                  65535,              RSVD_WRT,
/*H03_66*/  0,              0,                  65535,              RSVD_WRT,
/*H03_67*/  0,              0,                  65535,              RSVD_WRT,
/*H03_68*/  0,              0,                  65535,              RSVD_WRT,
/*H03_69*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 AI_Rsvd70;               //H03_70 
    Uint16 AI_Rsvd71;               //H03_71
    Uint16 AI_Rsvd72;               //H03_72 
    Uint16 AI_Rsvd73;               //H03_73 
    Uint16 AI_Rsvd74;               //H03_74 
    Uint16 AI_Rsvd75;               //H03_75 
    Uint16 AI_Rsvd76;               //H03_76 
    Uint16 AI_Rsvd77;               //H03_77
    Uint16 AI_Rsvd78;               //H03_78
    Uint16 AI_Rsvd79;               //H03_79                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_70*/  0,              0,                  65535,              RSVD_WRT,
/*H03_71*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_71*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_73*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_74*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_75*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_76*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_77*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_78*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_79*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 AI_SpdGain;              //H03_80 模拟量10V对应速度1RPM
    Uint16 AI_ToqGain;              //H03_81 模拟量10V对应转矩
    Uint16 AI_Rsvd82;               //H03_82
    Uint16 AI_Rsvd83;               //H03_83
    Uint16 AI_Rsvd84;               //H03_84
    Uint16 AI_Rsvd85;               //H03_85
    Uint16 AI_Rsvd86;               //H03_86
    Uint16 AI_Rsvd87;               //H03_87
    Uint16 AI_Rsvd88;               //H03_88
    Uint16 AI_Rsvd89;               //H03_89              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H03_80*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_81*/  0,              0,                  65535,              RSVD_WRT, 
#else
/*H03_80*/  3000,           0,                  6000,               POSD_WRT|    DISP_4,
/*H03_81*/  100,            0,                  800,                POSD_WRT|    DISP_3|  TWO_DOT,
#endif
/*H03_82*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_83*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_84*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_85*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_86*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_87*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_88*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_89*/  0,              0,                  65535,              RSVD_WRT, 


/*  Uint16 AI_Rsvd90;               //H03_90
    Uint16 AI_Rsvd91;               //H03_91
    Uint16 AI_Rsvd92;               //H03_92
    Uint16 AI_Rsvd93;               //H03_93
    Uint16 AI_Rsvd94;               //H03_94
    Uint16 AI_Rsvd95;               //H03_95
    Uint16 AI_Rsvd96;               //H03_96
    Uint16 AI_Rsvd97;               //H03_97
    Uint16 AI_Rsvd98;               //H03_98
    Uint16 MemCheck03;              //H03_99 组校验字                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H03_90*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_91*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_92*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_93*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_94*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_95*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_96*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_97*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_98*/  0,              0,                  65535,              RSVD_WRT, 
/*H03_99*/  0x3A5C,         0,                  65535,              RSVD_WRT,


//----------------------------端子输出参数  H04LEN = 70 -------------------------------------------------------
/*  Uint16 DOFuncSel1;                  //H04_00 DO01端子功能选择 
    Uint16 DOLogicSel1;                 //H04_01 DO01端子逻辑选择 
    Uint16 DOFuncSel2;                  //H04_02 DO02端子功能选择 
    Uint16 DOLogicSel2;                 //H04_03 DO02端子逻辑选择 
    Uint16 DOFuncSel3;                  //H04_04 DO03端子功能选择 
    Uint16 DOLogicSel3;                 //H04_05 DO03端子逻辑选择 
    Uint16 DOFuncSel4;                  //H04_06 DO04端子功能选择 
    Uint16 DOLogicSel4;                 //H04_07 DO04端子逻辑选择 
    Uint16 DOFuncSel5;                  //H04_08 DO05端子功能选择 
    Uint16 DOLogicSel5;                 //H04_09 DO05端子逻辑选择                */ 
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_00*/  1,              0,                  DO_FUNC_SEL_MAX,    ANY_WRT|    DISP_2,
/*H04_01*/  0,              0,                  1,                  ANY_WRT|    DISP_1,
/*H04_02*/  5,              0,                  DO_FUNC_SEL_MAX,    ANY_WRT|    DISP_2,
/*H04_03*/  0,              0,                  1,                  ANY_WRT|    DISP_1,
/*H04_04*/  3,              0,                  DO_FUNC_SEL_MAX,    ANY_WRT|    DISP_2,
/*H04_05*/  0,              0,                  1,                  ANY_WRT|    DISP_1,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H04_06*/  0,              0,                  65535,              RSVD_WRT,
/*H04_07*/  0,              0,                  65535,              RSVD_WRT,
/*H04_08*/  0,              0,                  65535,              RSVD_WRT,
/*H04_09*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H04_06*/  11,             0,                  DO_FUNC_SEL_MAX,    ANY_WRT|    DISP_2,
/*H04_07*/  0,              0,                  1,                  ANY_WRT|    DISP_1,
/*H04_08*/  16,             0,                  DO_FUNC_SEL_MAX,    ANY_WRT|    DISP_2,
/*H04_09*/  0,              0,                  1,                  ANY_WRT|    DISP_1,
#endif
/*  Uint16 DOFuncSel6;                  //H04_10 DO06端子功能选择 
    Uint16 DOLogicSel6;                 //H04_11 DO06端子逻辑选择 
    Uint16 DOFuncSel7;                  //H04_12 DO07端子功能选择 
    Uint16 DOLogicSel7;                 //H04_13 DO07端子逻辑选择 
    Uint16 DOFuncSel8;                  //H04_14 DO08端子功能选择 
    Uint16 DOLogicSel8;                 //H04_15 DO08端子逻辑选择 
    Uint16 DO_Rsvd16;                   //H04_16 DO09端子功能选择 
    Uint16 DO_Rsvd17;                   //H04_17 DO09端子逻辑选择 
    Uint16 DO_Rsvd18;                   //H04_18 DO10端子功能选择 
    Uint16 DO_Rsvd19;                   //H04_19 DO10端子逻辑选择                             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_10*/  0,              0,                  65535,              RSVD_WRT,
/*H04_11*/  0,              0,                  65535,              RSVD_WRT,
/*H04_12*/  0,              0,                  65535,              RSVD_WRT,
/*H04_13*/  0,              0,                  65535,              RSVD_WRT,
/*H04_14*/  0,              0,                  65535,              RSVD_WRT,
/*H04_15*/  0,              0,                  65535,              RSVD_WRT,
/*H04_16*/  0,              0,                  65535,              RSVD_WRT,
/*H04_17*/  0,              0,                  65535,              RSVD_WRT,
/*H04_18*/  0,              0,                  65535,              RSVD_WRT,
/*H04_19*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 DO_Rsvd20;                   //H04_20 DO11端子功能选择 
    Uint16 DO_Rsvd21;                   //H04_21 DO11端子逻辑选择 
    Uint16 DO_SourceSel;                //H04_22 DO来源选择
    Uint16 DO_Rsvd23;                   //H04_23
    Uint16 DO_Rsvd24;                   //H04_24
    Uint16 DO_Rsvd25;                   //H04_25
    Uint16 DO_Rsvd26;                   //H04_26
    Uint16 DO_Rsvd27;                   //H04_27
    Uint16 DO_Rsvd28;                   //H04_28
    Uint16 DO_Rsvd29;                   //H04_29                                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_20*/  0,              0,                  65535,              RSVD_WRT,
/*H04_21*/  0,              0,                  65535,              RSVD_WRT,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H04_22*/  0,              0,                  7,                  POSD_WRT|    DISP_3,
#else
/*H04_22*/  0,              0,                  31,                 POSD_WRT|    DISP_5,
#endif
/*H04_23*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_24*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_25*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_26*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_27*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_28*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_29*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 DO_Rsvd30;                   //H04_30
    Uint16 DO_Rsvd31;                   //H04_31
    Uint16 DO_Rsvd32;                   //H04_32
    Uint16 DO_Rsvd33;                   //H04_33
    Uint16 DO_Rsvd34;                   //H04_34
    Uint16 DO_Rsvd35;                   //H04_35
    Uint16 DO_Rsvd36;                   //H04_36
    Uint16 DO_Rsvd37;                   //H04_37
    Uint16 DO_Rsvd38;                   //H04_38
    Uint16 DO_Rsvd39;                   //H04_39                                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_30*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_31*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_32*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_33*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_34*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_35*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_36*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_37*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_38*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_39*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 DO_Rsvd40;                   //H04_40
    Uint16 DO_Rsvd41;                   //H04_41
    Uint16 DO_Rsvd42;                   //H04_42
    Uint16 DO_Rsvd43;                   //H04_43
    Uint16 DO_Rsvd44;                   //H04_44
    Uint16 DO_Rsvd45;                   //H04_45
    Uint16 DO_Rsvd46;                   //H04_46
    Uint16 DO_Rsvd47;                   //H04_47
    Uint16 DO_Rsvd48;                   //H04_48
    Uint16 DO_Rsvd49;                   //H04_49                               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_40*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_41*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_42*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_43*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_44*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_45*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_46*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_47*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_48*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_49*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 AO1SignalSel;                //H04_50 A01信号选择
    Uint16 AO1Offset;                   //H04_51 AO1偏移量电压 mV
    Uint16 AO1Gain;                     //H04_52 AO1倍率   0.01倍
    Uint16 AO2SignalSel;                //H04_53 AO2信号选择
    Uint16 AO2Offset;                   //H04_54 AO2偏移量电压 mV
    Uint16 AO2Gain;                     //H04_55 AO2倍率   0.01倍
    Uint16 AO_Rsvd56;                   //H04_56
    Uint16 AO_Rsvd57;                   //H04_57
    Uint16 AO_Rsvd58;                   //H04_58
    Uint16 AO_Rsvd59;                   //H04_59                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H04_50*/  0,              0,                  7,                   ANY_WRT|    DISP_1,
#else
/*H04_50*/  0,              0,                  9,                   ANY_WRT|    DISP_1,
#endif
/*H04_51*/  5000,           (Uint16)-10000,     10000,               ANY_WRT|    DISP_6|  INT_SIGN,
/*H04_52*/  100,            (Uint16)-9999,      9999,                ANY_WRT|    DISP_5|   TWO_DOT|  INT_SIGN,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H04_53*/  0,              0,                  7,                   ANY_WRT|    DISP_1,
#else
/*H04_53*/  0,              0,                  9,                   ANY_WRT|    DISP_1,
#endif
/*H04_54*/  5000,           (Uint16)-10000,     10000,               ANY_WRT|    DISP_6|  INT_SIGN,
/*H04_55*/  100,            (Uint16)-9999,      9999,                ANY_WRT|    DISP_5|   TWO_DOT|  INT_SIGN,
/*H04_56*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_57*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_58*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_59*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 AO_Rsvd60;                   //H04_60
    Uint16 AO_Rsvd61;                   //H04_61
    Uint16 AO_Rsvd62;                   //H04_62
    Uint16 AO_Rsvd63;                   //H04_63
    Uint16 AO_Rsvd64;                   //H04_64
    Uint16 AO_Rsvd65;                   //H04_65
    Uint16 AO_Rsvd66;                   //H04_66
    Uint16 AO_Rsvd67;                   //H04_67
    Uint16 AO_Rsvd68;                   //H04_68
    Uint16 MemCheck04;                  //H04_69 组校验字                       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H04_60*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_61*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_62*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_63*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_64*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_65*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_66*/  0,              0,                  65535,              RSVD_WRT,
/*H04_67*/  0,              0,                  65535,              RSVD_WRT, 
/*H04_68*/  0,              0,                  65535,              RSVD_WRT,
/*H04_69*/  0x4A5C,         0,                  65535,              RSVD_WRT, 


//----------------------------位置控制参数  H05LEN = 90 -------------------------------------------------------
/*  Uint16 PL_PosSurcASel;              //H05_00 位置指令A来源
    Uint16 PL_PulsePosCmdSel;           //H05_01 高低速脉冲位置指令选择 0--低速，1--高速
    Uint16 PL_Rsvd02;                   //H05_02 这个32bit功能码留给20bit编码器和2500线编码器的换算系数使用
    Uint16 PL_Rsvd03;                   //H05_03 
    Uint16 PL_PulseInFilterTime;        //H05_04 一阶低通滤波时间
    Uint16 PL_PosStep;                  //H05_05 步进位置指令总量
    Uint16 PL_PosAverge_tme;            //H05_06 位置指令移动平均时间
    Uint16 PL_PosFirCmxLow;             //H05_07 电子齿数比1 分子 L
    Uint16 PL_PosFirCmxHigh;            //H05_08 电子齿数比1 分子 H
    Uint16 PL_PosFirCdvLow;             //H05_09 电子齿数比1 分母 L                                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_00*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H05_01*/  1,              1,                  1,                  POSD_WRT|    DISP_1,
#else
/*H05_01*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
#endif
/*H05_02*/  0,              0,                  0,                  POSD_WRT|  TWO_WORD|   DISP_10| NEXT_ACT,
/*H05_03*/  0x0,            0,                  0x10,               POSD_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_04*/  0,              0,                  65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H05_05*/  50,             (Uint16)-9999,      9999,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H05_06*/  0,              0,                  1280,               POSD_WRT|    DISP_4|   ONE_DOT,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H05_07*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_08*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_09*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
#else
/*H05_07*/  4,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_08*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_09*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
#endif
/*  Uint16 PL_PosFirCdvHigh;            //H05_10 电子齿数比1 分母 H
    Uint16 PL_PosSecCmxLow;             //H05_11 电子齿数比2 分子 L
    Uint16 PL_PosSecCmxHigh;            //H05_12 电子齿数比2 分子 H
    Uint16 PL_PosSecCdvLow;             //H05_13 电子齿数比2 分母 L
    Uint16 PL_PosSecCdvHigh;            //H05_14 电子齿数比2 分母 H
    Uint16 PL_PosPulsInMode;            //H05_15 指令脉冲形态
    Uint16 PL_PerrClrMode;              //H05_16 清除动作选择
    Uint16 PL_PosCodeDiv_L;             //H05_17 编码器分频脉冲数 L
    Uint16 PL_PosCodeDiv_H;             //H05_18 编码器分频脉冲数 H
    Uint16 PL_PosForeBaceSel;           //H05_19 位置前馈控制选择                               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if NONSTANDARD_PROJECT == LINEARMOT
/*H05_10*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_11*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_12*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_13*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_14*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
#else
/*H05_10*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_11*/  4,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_12*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_13*/  1,              1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_14*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
#endif

#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H05_15*/  2,              0,                  3,                  POSD_WRT|    DISP_1|  NEXT_ACT,
#else
/*H05_15*/  0,              0,                  3,                  POSD_WRT|    DISP_1|  NEXT_ACT,
#endif

/*H05_16*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H05_17*/  2500,          35,                 32767,               POSD_WRT |DISP_5| NEXT_ACT,
/*H05_18*/  0,              0,                 65535,               RSVD_WRT,
#if ECT_ENABLE_SWITCH
/*H05_19*/  1,              0,                  2,                  POSD_WRT|    DISP_1,
#else
/*H05_19*/  1,              0,                  3,                  POSD_WRT|    DISP_1,
#endif

/*  Uint16 PL_ReachConditio;            //H05_20 定位完成信号COIN输出条件
    Uint16 PL_PosReachValue;            //H05_21 定位完成幅度
    Uint16 PL_PosNearValue;             //H05_22 定位完成接近信号NEAR幅度
    Uint16 PL_XintPosSel;               //H05_23 中断位移使能
    Uint16 PL_XintPosRef_Lo;            //H05_24 中断位移长度 L
    Uint16 PL_XintPosRef_Hi;            //H05_25 中断位移长度 H
    Uint16 PL_XintMoveSpd;              //H05_26 移动速度
    Uint16 PL_XintAccTime;              //H05_27 加减速时间
    Uint16 PL_XintSmSpd;                //H05_28 预留 中断定长是否以中断时速度运行
    Uint16 PL_XintFreeEn;               //H05_29 中断锁定信号使能               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_20*/  0,              0,                  2,                   ANY_WRT|    DISP_1,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H05_21*/  20,             1,                  65535,               ANY_WRT|    DISP_5,
#else
/*H05_21*/  7,              1,                  65535,               ANY_WRT|    DISP_5,
#endif
/*H05_22*/  65535,          1,                  65535,               ANY_WRT|    DISP_5,
/*H05_23*/  0,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT,
/*H05_24*/  10000,          0,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H05_25*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H05_26*/  200,            0,                  6000,                ANY_WRT|    DISP_4,
/*H05_27*/  10,             0,                  1000,               ANY_WRT|    DISP_4,
/*H05_28*/  0,              0,                  65535,              RSVD_WRT, 
/*H05_29*/  1,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 PL_OriginResetEn;            //H05_30 原点复位使能
    Uint16 PL_OriginResetMode;          //H05_31 原点复位模式
    Uint16 PL_OriginHighSpeedSearch;    //H05_32 原点高速搜索速度
    Uint16 PL_OriginLowSpeedSearch;     //H05_33 原点低速搜索速度
    Uint16 PL_OriginRiseDownTime;       //H05_34 原点加减速时间
    Uint16 PL_OriginSearchTime;         //H05_35 原点查找时间
    Uint16 PL_OriginOffsetLow;          //H05_36 原点机械偏移量 L
    Uint16 PL_OriginOffsetHigh;         //H05_37 原点机械偏移量 H
    Uint16 PL_DivSourceSel;             //H05_38 伺服脉冲输出来源选择
    Uint16 PL_PosGearChangeEn;          //H05_39 电子齿轮切换使能                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_30*/  0,              0,                  7,                   ANY_WRT|    DISP_1,
/*H05_31*/  0,              0,                  9,                  POSD_WRT|    DISP_1,
/*H05_32*/  100,            0,                  3000,               ANY_WRT|    DISP_4,
/*H05_33*/  10,             0,                  1000,               ANY_WRT|    DISP_4,
/*H05_34*/  1000,           0,                  1000,               ANY_WRT|    DISP_4,

#if ECT_ENABLE_SWITCH
/*H05_35*/  50000,          0,                  65535,              ANY_WRT|    DISP_5,
#else
/*H05_35*/  10000,          0,                  65535,              ANY_WRT|    DISP_5,
#endif

/*H05_36*/  0,              0,                  0,                  ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H05_37*/  0,              0xC000,             0x4000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H05_38*/  0,              0,                  2,                  POSD_WRT|    DISP_1|  NEXT_ACT,
/*H05_39*/  0,              0,                  1,                  POSD_WRT|    DISP_1,

/*  Uint16 PL_OriginOriginSel;          //H05_40 bit0-原点是否偏移，bit1-遇限位是否反向找零 
    Uint16 PL_EncDivZPolSet;            //H05_41 Z脉冲输出极性设置，0-正极性设置(Z脉冲为高电平) 1-负极性设置(Z脉冲为低电平)
    Uint16 PL_PosPulsInMode2;           //H05_42 高速脉冲指令脉冲形态 保留给非标使用
    Uint16 PL_PosCmdEdge;               //H05_43 位置脉冲边沿选择：1～上升沿有效，0～下降沿有效 
    Uint16 PL_EncMultiTurnOffset;       //H05_44 编码器多圈数据偏置
    Uint16 PL_HDHEncMultiTurnOffset;    //H05_45 海德汉编码器多圈数据偏置
    Uint16 PL_MultiAbsPosOffsetL;       //H05_46 多圈绝对位置偏置L
    Uint16 PL_MultiAbsPosOffsetH;       //H05_47 多圈绝对位置偏置H
    Uint16 PL_MultiAbsPosOffsetH32L;    //H05_48 多圈绝对位置偏置H32L
    Uint16 PL_MultiAbsPosOffsetH32H;    //H05_49 多圈绝对位置偏置H32H             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_40*/  0,              0,                  3,                  POSD_WRT|    DISP_1|  ZERO_DOT|   UN_SIGN,  
/*H05_41*/  1,              0,                  1,                  POSD_WRT|    DISP_1|  ZERO_DOT| NEXT_ACT| UN_SIGN,
/*H05_42*/  0,              0,                  3,                  RSVD_WRT|    DISP_1|  NEXT_ACT, 
/*H05_43*/  0,              0,                  1,                   ANY_WRT|    DISP_1|  NEXT_ACT,  
/*H05_44*/  0,              0,                  65535,              POSD_WRT|   DISP_5, 
/*H05_45*/  0,              0,                  4095,               POSD_WRT|   DISP_4,  
/*H05_46*/  0,              0x0000,             0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H05_47*/  0,              0x8000,             0x7FFF,             POSD_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H05_48*/  0,              0x0000,             0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H05_49*/  0,              0x8000,             0x7FFF,             POSD_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,

/*  Uint16 PL_AbsMode2MechGearNum;      //H05_50 多圈绝对位置模式2 机械齿轮比分子
    Uint16 PL_AbsMode2MechGearDen;      //H05_51 多圈绝对位置模式2 机械齿轮比分母
    Uint16 PL_MechAbsPosLimUpL;         //H05_52 多圈绝对位置模式2 机械绝对位置上限值 低32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH;         //H05_53 多圈绝对位置模式2 机械绝对位置上限值 低32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH32L;      //H05_54 多圈绝对位置模式2 机械绝对位置上限值 高32位（编码器单位）
    Uint16 PL_MechAbsPosLimUpH32H;      //H05_55 多圈绝对位置模式2 机械绝对位置上限值 高32位（编码器单位）
    Uint16 PL_Rsvd56;                   //H05_56 
    Uint16 PL_Rsvd57;                   //H05_57 
    Uint16 PL_Rsvd58;                   //H05_58
    Uint16 PL_Rsvd59;                   //H05_59           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_50*/  1,              1,                  65535,              POSD_WRT|    DISP_5,
/*H05_51*/  1,              1,                  65535,              POSD_WRT|    DISP_5,
/*H05_52*/  0,              0,                  0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11,
/*H05_53*/  0,              0,                  0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11| HIGH_WORD,
/*H05_54*/  0,              0,                  0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11,
/*H05_55*/  0,              0,                  0xFFFF,             POSD_WRT|  TWO_WORD|   DISP_11| HIGH_WORD,
/*H05_56*/  0,              0,                  65535,              RSVD_WRT,
/*H05_57*/  0,              0,                  65535,              RSVD_WRT,
/*H05_58*/  0,              0,                  65535,              RSVD_WRT,
/*H05_59*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 PL_CoinHoldTime;             //H05_60 定位完成保持时间
	Uint16 PL_PosWinUnitSet;            //H05_61 位置到达窗口单位设置 0-编码器单位 1-指令单位
    Uint16 PL_Rsvd62;                   //H05_62 
    Uint16 PL_Rsvd63;                   //H05_63 
    Uint16 PL_Rsvd64;                   //H05_64 
    Uint16 PL_Rsvd65;                   //H05_65 
    Uint16 PL_Rsvd66;                   //H05_66 
    Uint16 PL_Rsvd67;                   //H05_67 
    Uint16 PL_Rsvd68;                   //H05_68 
    Uint16 PL_Rsvd69;                   //H05_69           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_60*/  0,              0,                  30000,              ANY_WRT |DISP_5,
/*H05_61*/  1,              0,                  1,                 	POSD_WRT|DISP_1,
/*H05_62*/  0,              0,                  65535,              RSVD_WRT,
/*H05_63*/  0,              0,                  65535,              RSVD_WRT,
/*H05_64*/  0,              0,                  65535,              RSVD_WRT,
/*H05_65*/  0,              0,                  65535,              RSVD_WRT,
/*H05_66*/  0,              0,                  65535,              RSVD_WRT,
/*H05_67*/  0,              0,                  65535,              RSVD_WRT,
/*H05_68*/  0,              0,                  65535,              RSVD_WRT,
/*H05_69*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 PL_Rsvd70;                   //H05_70
    Uint16 PL_Rsvd71;                   //H05_71 
    Uint16 PL_Rsvd72;                   //H05_72 
    Uint16 PL_Rsvd73;                   //H05_73 
    Uint16 PL_Rsvd74;                   //H05_74 
    Uint16 PL_Rsvd75;                   //H05_75 
    Uint16 PL_Rsvd76;                   //H05_76 
    Uint16 PL_Rsvd77;                   //H05_77 
    Uint16 PL_Rsvd78;                   //H05_78 
    Uint16 PL_Rsvd79;                   //H05_79           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_70*/  0,              0,                  65535,              RSVD_WRT,
/*H05_71*/  0,              0,                  65535,              RSVD_WRT,
/*H05_72*/  0,              0,                  65535,              RSVD_WRT,
/*H05_73*/  0,              0,                  65535,              RSVD_WRT,
/*H05_74*/  0,              0,                  65535,              RSVD_WRT,
/*H05_75*/  0,              0,                  65535,              RSVD_WRT,
/*H05_76*/  0,              0,                  65535,              RSVD_WRT,
/*H05_77*/  0,              0,                  65535,              RSVD_WRT,
/*H05_78*/  0,              0,                  65535,              RSVD_WRT,
/*H05_79*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 PL_Rsvd80;                   //H05_80
    Uint16 PL_Rsvd81;                   //H05_81 
    Uint16 PL_Rsvd82;                   //H05_82 
    Uint16 PL_Rsvd83;                   //H05_83 
    Uint16 PL_Rsvd84;                   //H05_84 
    Uint16 PL_Rsvd85;                   //H05_85 
    Uint16 PL_Rsvd86;                   //H05_86 
    Uint16 PL_Rsvd87;                   //H05_87 
    Uint16 PL_Rsvd88;                   //H05_88 
    Uint16 MemCheck05;                  //H05_89 组校验字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H05_80*/  0,              0,                  65535,              RSVD_WRT,
/*H05_81*/  0,              0,                  65535,              RSVD_WRT,
/*H05_82*/  0,              0,                  65535,              RSVD_WRT,
/*H05_83*/  0,              0,                  65535,              RSVD_WRT,
/*H05_84*/  0,              0,                  65535,              RSVD_WRT,
/*H05_85*/  0,              0,                  65535,              RSVD_WRT,
/*H05_86*/  0,              0,                  65535,              RSVD_WRT,
/*H05_87*/  0,              0,                  65535,              RSVD_WRT,
/*H05_88*/  0,              0,                  65535,              RSVD_WRT,
/*H05_89*/  0x5A5C,         0,                  65535,              RSVD_WRT,


//----------------------------速度控制参数  H06LEN = 50 -------------------------------------------------------
/*  Uint16 SL_SpdACommandFrom;          //H06_00 速度指令A来源
    Uint16 SL_SpdBCommandFrom;          //H06_01 速度指令B来源
    Uint16 SL_SpdABSwitchFlag;          //H06_02 速度指令选择
    Uint16 SL_SpdCMDFromKeyBoard;       //H06_03 速度指令键盘设定值     1rpm
    Uint16 SL_JOGSpdCommand;            //H06_04 点动速度设定值     1rpm
    Uint16 SL_SpdCMDRiseTime;           //H06_05 速度指令加速斜坡时间  1ms
    Uint16 SL_SpdCMDDownTime;           //H06_06 速度指令减速斜坡时间  1ms
    Uint16 SL_SpdMaxLimit;              //H06_07 最大转速限制值        1rpm
    Uint16 SL_SpdPositiveLimit;         //H06_08 速度正向限制          1rpm
    Uint16 SL_SpdNegativeLimit;         //H06_09 速度反向限制          1rpm                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H06_00*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H06_01*/  0,              0,                  5,                  POSD_WRT|    DISP_1,
#else
/*H06_01*/  1,              0,                  5,                  POSD_WRT|    DISP_1,
#endif
/*H06_02*/  0,              0,                  4,                  POSD_WRT|    DISP_1,
/*H06_03*/  200,            (Uint16)-6000,      6000,                ANY_WRT|    DISP_5|  INT_SIGN,
/*H06_04*/  100,            0,                  6000,                ANY_WRT|    DISP_4,
/*H06_05*/  0,              0,                  65535,               ANY_WRT|    DISP_5,
/*H06_06*/  0,              0,                  65535,               ANY_WRT|    DISP_5,
/*H06_07*/  6000,           0,                  6000,                ANY_WRT|    DISP_4,
/*H06_08*/  6000,           0,                  6000,                ANY_WRT|    DISP_4,
/*H06_09*/  6000,           0,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 SL_Rsvd10;                   //H06_10 速度反馈滤波选择 (删掉)
    Uint16 SL_SpdForwardChooseFlag;     //H06_11 转矩前馈选择
    Uint16 SL_Rsvd12;                   //H06_12 
    Uint16 SL_Rsvd13;                   //H06_13 
    Uint16 SL_Rsvd14;                   //H06_14
    Uint16 SL_SpdZeroClampTolerance;    //H06_15 零速钳位/零位固定速度指令阈值    1rpm
    Uint16 SL_SpdZeroTolerance;         //H06_16 电机旋转状态阈值          1rpm
    Uint16 SL_SpdReachTolerance;        //H06_17 速度一致信号宽度    1rpm
    Uint16 SL_SpdArriveTolerance;       //H06_18 速度到达信号阈值    1rpm
    Uint16 SL_ZeroSpdTolerance;         //H06_19 零速输出信号阈值    1rpm                                      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H06_10*/  0,              0,                  65535,              RSVD_WRT,
#if ECT_ENABLE_SWITCH
/*H06_11*/  1,              0,                  2,                   ANY_WRT|    DISP_1,
#else
/*H06_11*/  1,              0,                  1,                   ANY_WRT|    DISP_1,
#endif

/*H06_12*/  10,             0,                  65535,              RSVD_WRT,
/*H06_13*/  0,              0,                  65535,               RSVD_WRT, 
/*H06_14*/  0,              0,                  65535,              RSVD_WRT, 
/*H06_15*/  10,             0,                  6000,                ANY_WRT|    DISP_4,
/*H06_16*/  20,             0,                  1000,                ANY_WRT|    DISP_4,
/*H06_17*/  10,             0,                  100,                 ANY_WRT|    DISP_3,
/*H06_18*/  1000,           10,                 6000,                ANY_WRT|    DISP_4,
/*H06_19*/  10,             1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 SL_Rsvd20;                   //H06_20
    Uint16 SL_Rsvd21;                   //H06_21
    Uint16 SL_Rsvd22;                   //H06_22
    Uint16 SL_Rsvd23;                   //H06_23
    Uint16 SL_Rsvd24;                   //H06_24
    Uint16 SL_Rsvd25;                   //H06_25
    Uint16 SL_Rsvd26;                   //H06_26
    Uint16 SL_Rsvd27;                   //H06_27
    Uint16 SL_Rsvd28;                   //H06_28
    Uint16 SL_Rsvd29;                   //H06_29            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H06_20*/  0,              0,                  65535,              RSVD_WRT,
/*H06_21*/  0,              0,                  65535,              RSVD_WRT,
/*H06_22*/  0,              0,                  65535,              RSVD_WRT,
/*H06_23*/  0,              0,                  65535,              RSVD_WRT,
/*H06_24*/  0,              0,                  65535,              RSVD_WRT,
/*H06_25*/  0,              0,                  65535,              RSVD_WRT,
/*H06_26*/  0,              0,                  65535,              RSVD_WRT,
/*H06_27*/  0,              0,                  65535,              RSVD_WRT,
/*H06_28*/  0,              0,                  65535,              RSVD_WRT,
/*H06_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SL_Rsvd30;                   //H06_30
    Uint16 SL_Rsvd31;                   //H06_31
    Uint16 SL_Rsvd32;                   //H06_32
    Uint16 SL_Rsvd33;                   //H06_33
    Uint16 SL_Rsvd34;                   //H06_34
    Uint16 SL_Rsvd35;                   //H06_35
    Uint16 SL_Rsvd36;                   //H06_36
    Uint16 SL_Rsvd37;                   //H06_37
    Uint16 SL_Rsvd38;                   //H06_38
    Uint16 SL_Rsvd39;                   //H06_39            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H06_30*/  0,              0,                  65535,              RSVD_WRT,
/*H06_31*/  0,              0,                  65535,              RSVD_WRT,
/*H06_32*/  0,              0,                  65535,              RSVD_WRT,
/*H06_33*/  0,              0,                  65535,              RSVD_WRT,
/*H06_34*/  0,              0,                  65535,              RSVD_WRT,
/*H06_35*/  0,              0,                  65535,              RSVD_WRT,
/*H06_36*/  0,              0,                  65535,              RSVD_WRT,
/*H06_37*/  0,              0,                  65535,              RSVD_WRT,
/*H06_38*/  0,              0,                  65535,              RSVD_WRT,
/*H06_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SL_Rsvd40;                   //H06_40
    Uint16 SL_Rsvd41;                   //H06_41
    Uint16 SL_Rsvd42;                   //H06_42
    Uint16 SL_Rsvd43;                   //H06_43
    Uint16 SL_Rsvd44;                   //H06_44
    Uint16 SL_Rsvd45;                   //H06_45
    Uint16 SL_Rsvd46;                   //H06_46
    Uint16 SL_Rsvd47;                   //H06_47
    Uint16 SL_Rsvd48;                   //H06_48
    Uint16 MemCheck06;                  //H06_49 组校验字            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H06_40*/  0,              0,                  65535,              RSVD_WRT,
/*H06_41*/  0,              0,                  65535,              RSVD_WRT,
/*H06_42*/  0,              0,                  65535,              RSVD_WRT,
/*H06_43*/  0,              0,                  65535,              RSVD_WRT,
/*H06_44*/  0,              0,                  65535,              RSVD_WRT,
/*H06_45*/  0,              0,                  65535,              RSVD_WRT,
/*H06_46*/  0,              0,                  65535,              RSVD_WRT,
/*H06_47*/  0,              0,                  65535,              RSVD_WRT,
/*H06_48*/  0,              0,                  65535,              RSVD_WRT,
/*H06_49*/  0x6A5C,         0,                  65535,              RSVD_WRT,


//----------------------------转矩控制参数  H07LEN = 70 -------------------------------------------------------
/*  Uint16 TL_ToqCmdSourceA;          //H07_00 转矩指令A来源
    Uint16 TL_ToqCmdSourceB;          //H07_01 转矩指令B来源
    Uint16 TL_ToqCmdSel;              //H07_02 转矩指令选择
    Uint16 TL_ToqCmdInner;            //H07_03 转矩指令键盘设定
    Uint16 TL_Rsvd04;                 //H07_04 
    Uint16 TL_ToqFiltTime;            //H07_05 转矩指令滤波时间1
    Uint16 TL_ToqFiltTime2;           //H07_06 转矩指令滤波时间2
    Uint16 TL_ToqLmtSource;           //H07_07 转矩限制来源
    Uint16 TL_ToqLmtAISel;            //H07_08 T-LMT选择
    Uint16 TL_ToqPlusLmtIn;           //H07_09 正转内部转矩限制值                       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H07_00*/  0,              0,                  0,                  POSD_WRT|    DISP_1,
/*H07_01*/  0,              0,                  0,                  POSD_WRT|    DISP_1,
#else
/*H07_00*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H07_01*/  1,              0,                  2,                  POSD_WRT|    DISP_1,
#endif
/*H07_02*/  0,              0,                  4,                  POSD_WRT|    DISP_1,
/*H07_03*/  0,              (Uint16)-3000,      3000,                ANY_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
/*H07_04*/  0,              0,                  1,                  RSVD_WRT,
/*H07_05*/  79,              0,                  3000,                ANY_WRT|    DISP_4|   TWO_DOT,
/*H07_06*/  79,              0,                  3000,               ANY_WRT|    DISP_4|   TWO_DOT,

#if ECT_ENABLE_SWITCH
/*H07_07*/  2,              0,                  4,                  ANY_WRT|    DISP_1,
#else
/*H07_07*/  0,              0,                  4,                  ANY_WRT|    DISP_1,
#endif

#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H07_08*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H07_08*/  2,              1,                  2,                  ANY_WRT|    DISP_1,
#endif

/*H07_09*/  3000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT,

/*  Uint16 TL_ToqMinusLmtIn;          //H07_10 反转内部转矩限制值
    Uint16 TL_ToqPlusLmtTrig;         //H07_11 正转侧外部转矩限制值
    Uint16 TL_ToqMinusLmtTrig;        //H07_12 反转侧外部转矩限制值
    Uint16 TL_ToqPlusComp;            //H07_13 正向转矩补偿量0.1%
    Uint16 TL_ToqMinusComp;           //H07_14 反向转矩补偿量0.1%
    Uint16 TL_EmergentToq;            //H07_15 急停转矩
    Uint16 TL_Rsvd16;                 //H07_16 
    Uint16 TL_SpdLmtSel;              //H07_17 速度限制来源选择
    Uint16 TL_SpdLmtAiSel;            //H07_18 V-LMT选择
    Uint16 TL_SpdLmtIn;               //H07_19 转矩控制时内部速度限制值                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/ 
/*注释      默认值          下限                上限                属性*/
/*H07_10*/  3000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H07_11*/  3000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H07_12*/  3000,           0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H07_13*/  0,              0,                  500,                RSVD_WRT,
/*H07_14*/  0,              0,                  500,                RSVD_WRT,
/*H07_15*/  1000,           0,                  3000,               POSD_WRT|    DISP_4|   ONE_DOT,
/*H07_16*/  0,              0,                  65535,              RSVD_WRT,
#if ECT_ENABLE_SWITCH
/*H07_17*/  1,              0,                  2,                   ANY_WRT|    DISP_1,
#else
/*H07_17*/  0,              0,                  2,                   ANY_WRT|    DISP_1,
#endif

#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H07_18*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H07_18*/  1,              1,                  2,                   ANY_WRT|    DISP_1,
#endif
/*H07_19*/  3000,           0,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 TL_SpdLmtNegIn;            //H07_20 转矩控制时内部速度负向限制值
    Uint16 TL_ToqRchStandard;         //H07_21 转矩到达基准值
    Uint16 TL_ToqRchOn;               //H07_22 转矩到达DO信号开启时输出转矩值
    Uint16 TL_ToqRchOff;              //H07_23 转矩到达DO信号关闭时输出转矩值
    Uint16 TL_Rsvd24;                 //H07_24 
    Uint16 TL_Rsvd25;                 //H07_25 
    Uint16 TL_Rsvd26;                 //H07_26 
    Uint16 TL_Rsvd27;                 //H07_27 
    Uint16 TL_Rsvd28;                 //H07_28 
    Uint16 TL_Rsvd29;                 //H07_29     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H07_20*/  3000,           0,                  6000,                ANY_WRT|    DISP_4,
/*H07_21*/  0,              0,                  3000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H07_22 */ 200,  GetCodeIndex(FunCodeUnion.code.TL_ToqRchOff),  3000,   ANY_WRT| RLAT_LMTL|    DISP_4| ONE_DOT,
/*H07_23*/  100,  0,      GetCodeIndex(FunCodeUnion.code.TL_ToqRchOn),   ANY_WRT| RLAT_LMTH|    DISP_4| ONE_DOT,
/*H07_24*/  0,              0,                  65535,              RSVD_WRT,
/*H07_25*/  0,              0,                  65535,              RSVD_WRT,
/*H07_26*/  0,              0,                  65535,              RSVD_WRT,
/*H07_27*/  0,              0,                  65535,              RSVD_WRT,
/*H07_28*/  0,              0,                  65535,              RSVD_WRT,
/*H07_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 TL_Rsvd30;                 //H07_30 
    Uint16 TL_Rsvd31;                 //H07_31 
    Uint16 TL_Rsvd32;                 //H07_32 
    Uint16 TL_Rsvd33;                 //H07_33 
    Uint16 TL_Rsvd34;                 //H07_34 
    Uint16 TL_Rsvd35;                 //H07_35 
    Uint16 TL_Rsvd36;                 //H07_36 
    Uint16 TL_Rsvd37;                 //H07_37 
    Uint16 TL_Rsvd38;                 //H07_38 
    Uint16 TL_Rsvd39;                 //H07_39             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H07_30*/  0,              0,                  65535,              RSVD_WRT,
/*H07_31*/  0,              0,                  65535,              RSVD_WRT,
/*H07_32*/  0,              0,                  65535,              RSVD_WRT,
/*H07_33*/  0,              0,                  65535,              RSVD_WRT,
/*H07_34*/  0,              0,                  65535,              RSVD_WRT,
/*H07_35*/  0,              0,                  65535,              RSVD_WRT,
/*H07_36*/  0,              0,                  65535,              RSVD_WRT,
/*H07_37*/  0,              0,                  65535,              RSVD_WRT,
/*H07_38*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H07_39*/  0,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 TL_OVSpdLmtWaitT           //H07_40 是否超过速度限制判断等待时间 0.1ms (zxh)
    Uint16 TL_Rsvd41;                 //H07_41
    Uint16 TL_Rsvd42;                 //H07_42
    Uint16 TL_Rsvd43;                 //H07_43
    Uint16 TL_Rsvd44;                 //H07_44
    Uint16 TL_Rsvd45;                 //H07_45
    Uint16 TL_Rsvd46;                 //H07_46
    Uint16 TL_Rsvd47;                 //H07_47
    Uint16 TL_Rsvd48;                 //H07_48
    Uint16 TL_Rsvd49;                 //H07_49             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/

/*注释      默认值          下限                上限                属性*/
/*H07_40*/  10,             5,                  300,                 ANY_WRT|    DISP_3|   ONE_DOT,
/*H07_41*/  0,              0,                  65535,              RSVD_WRT,
/*H07_42*/  0,              0,                  65535,              RSVD_WRT,
/*H07_43*/  0,              0,                  65535,              RSVD_WRT,
/*H07_44*/  0,              0,                  65535,              RSVD_WRT,
/*H07_45*/  0,              0,                  65535,              RSVD_WRT,
/*H07_46*/  0,              0,                  65535,              RSVD_WRT,
/*H07_47*/  0,              0,                  65535,              RSVD_WRT,
/*H07_48*/  0,              0,                  65535,              RSVD_WRT,
/*H07_49*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 TL_Rsvd50;                 //H07_50
    Uint16 TL_Rsvd51;                 //H07_51
    Uint16 TL_Rsvd52;                 //H07_52
    Uint16 TL_Rsvd53;                 //H07_53
    Uint16 TL_Rsvd54;                 //H07_54
    Uint16 TL_Rsvd55;                 //H07_55
    Uint16 TL_Rsvd56;                 //H07_56
    Uint16 TL_Rsvd57;                 //H07_57
    Uint16 TL_Rsvd58;                 //H07_58
    Uint16 TL_Rsvd59;                 //H07_59             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H07_50*/  0,              0,                  65535,              RSVD_WRT,
/*H07_51*/  0,              0,                  65535,              RSVD_WRT,
/*H07_52*/  0,              0,                  65535,              RSVD_WRT,
/*H07_53*/  0,              0,                  65535,              RSVD_WRT,
/*H07_54*/  0,              0,                  65535,              RSVD_WRT,
/*H07_55*/  0,              0,                  65535,              RSVD_WRT,
/*H07_56*/  0,              0,                  65535,              RSVD_WRT,
/*H07_57*/  0,              0,                  65535,              RSVD_WRT,
/*H07_58*/  0,              0,                  65535,              RSVD_WRT,
/*H07_59*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 TL_Rsvd60;                 //H07_60
    Uint16 TL_Rsvd61;                 //H07_61
    Uint16 TL_Rsvd62;                 //H07_62
    Uint16 TL_Rsvd63;                 //H07_63
    Uint16 TL_Rsvd64;                 //H07_64
    Uint16 TL_Rsvd65;                 //H07_65
    Uint16 TL_Rsvd66;                 //H07_66
    Uint16 TL_Rsvd67;                 //H07_67
    Uint16 TL_Rsvd68;                 //H07_68
    Uint16 MemCheck07;                //H07_69 组校验字                      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H07_60*/  0,              0,                  65535,              RSVD_WRT,
/*H07_61*/  0,              0,                  65535,              RSVD_WRT,
/*H07_62*/  0,              0,                  65535,              RSVD_WRT,
/*H07_63*/  0,              0,                  65535,              RSVD_WRT,
/*H07_64*/  0,              0,                  65535,              RSVD_WRT,
/*H07_65*/  0,              0,                  65535,              RSVD_WRT,
/*H07_66*/  0,              0,                  65535,              RSVD_WRT,
/*H07_67*/  0,              0,                  65535,              RSVD_WRT,
/*H07_68*/  0,              0,                  65535,              RSVD_WRT,
/*H07_69*/  0x7A5C,         0,                  65535,              RSVD_WRT,


//----------------------------增益类参数  H08LEN = 90 -------------------------------------------------------
/*  Uint16 GN_Spd_Kp;                 //H08_00 速度环增益  0.1Hz
    Uint16 GN_Spd_Ti;                 //H08_01 速度环积分时间常数  0.01ms
    Uint16 GN_Pos_Kp;                 //H08_02 位置环增益  0.1Hz
    Uint16 GN_Spd_Kp2;                //H08_03 第2速度环增益  0.1Hz
    Uint16 GN_Spd_Ti2;                //H08_04 第2速度环积分时间常数  0.01ms
    Uint16 GN_Pos_Kp2;                //H08_05 第2位置环增益  0.1Hz
    Uint16 GN_ServoCtrlMode;          //H08_06 性能模式：0～高速优先，1～性能优先
    Uint16 GN_Rsvd07;                 //H08_07
    Uint16 GN_Gn2ndSetup;             //H08_08 第二增益模式设置
    Uint16 GN_GnSwModeSel;            //H08_09 增益切换条件选择                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_00*/  250,            1,                 20000,                ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_01*/  3183,           15,                51200,                ANY_WRT|    DISP_5|   TWO_DOT,
/*H08_02*/  400,            0,                 20000,                ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_03*/  400,            1,                 20000,                ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_04*/  2000,           15,                51200,                ANY_WRT|    DISP_5|   TWO_DOT,
/*H08_05*/  640,            0,                 20000,                ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_06*/  2,              0,                 3,                   POSD_WRT|    DISP_1,
/*H08_07*/  0,              0,                 65535,               RSVD_WRT,
/*H08_08*/  1,              0,                 1,                    ANY_WRT|    DISP_1,
/*H08_09*/  0,              0,                 10,                   ANY_WRT|    DISP_2,

/*  Uint16 GN_GnSwDlyTm;              //H08_10 增益切换延时时间 0.1ms
    Uint16 GN_GnSwLvl;                //H08_11 增益切换等级
    Uint16 GN_GnSwHysteresis;         //H08_12 增益切换时滞
    Uint16 GN_PosGnSwTm;              //H08_13 位置增益切换时间 0.1ms
    Uint16 GN_Rsvd14;                 //H08_14 
    Uint16 GN_InertiaRatio;           //H08_15 负载转动惯量比=负载惯量与电机本体惯量比值
    Uint16 GN_Rsvd16;                 //H08_16 
    Uint16 GN_PosRegOutFltrT;         //H08_17 位置调节器输出滤波时间参数
    Uint16 GN_SpdFbFltrT;             //H08_18 速度前馈滤波时间参数
    Uint16 GN_SpdFb_Kp;               //H08_19 速度前馈增益                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_10*/  50,             0,                  10000,               ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_11*/  50,             0,                  20000,               ANY_WRT|    DISP_5,
/*H08_12*/  30,             0,                  20000,               ANY_WRT|    DISP_5,
/*H08_13*/  30,             0,                  10000,               ANY_WRT|    DISP_5|   ONE_DOT,
/*H08_14*/  0,              0,                  65535,              RSVD_WRT,
/*H08_15*/  100,            0,                  12000,               ANY_WRT|    DISP_5|   TWO_DOT,
/*H08_16*/  0,              0,                  65535,               ANY_WRT|    DISP_5|   TWO_DOT,
/*H08_17*/  0,              0,                  65535,              RSVD_WRT,
/*H08_18*/  50,             0,                  6400,                ANY_WRT|    DISP_4|   TWO_DOT,
/*H08_19*/  0,              0,                  1000,                ANY_WRT|    DISP_4|   ONE_DOT,

/*  Uint16 GN_ToqFbFltrT;             //H08_20 转矩前馈滤波时间参数
    Uint16 GN_ToqFb_Kp;               //H08_21 转矩前馈增益
    Uint16 GN_SpdFdbFilt_On;          //H08_22 FPGA速度平均值滤波使能  默认为0
    Uint16 GN_SpdLpFiltFc;            //H08_23 速度反馈低通滤波截止频率
    Uint16 GN_Pdff_Kf;                //H08_24 PDFF控制系数	   Disable时为100%
    Uint16 GN_SpdFbSel;               //H08_25 速度反馈选择
    Uint16 GN_Damping_Kf;             //H08_26 Damping_Kf控制系数   Disable时为0，Max为100%	理论匹配值为25%
    Uint16 GN_Rsvd27;                 //H08_27
    Uint16 GN_Rsvd28;                 //H08_28
    Uint16 GN_Rsvd29;                 //H08_29          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_20*/  50,             0,                  6400,                ANY_WRT|    DISP_4|   TWO_DOT,
/*H08_21*/  0,              0,                  2000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H08_22*/  0,              0,                  4,                  POSD_WRT|    DISP_1,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H08_23*/  1000,           100,                 4000,               ANY_WRT|    DISP_4,
#else
/*H08_23*/  4000,           100,                 4000,               ANY_WRT|    DISP_4,
#endif
/*H08_24*/  1000,           0,                  1000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H08_25*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H08_26*/  0,              0,                  1000,                ANY_WRT|    DISP_4|   ONE_DOT,
/*H08_27*/  0,              0,                  65535,              RSVD_WRT,
/*H08_28*/  0,              0,                  65535,              RSVD_WRT,
/*H08_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 GN_Rsvd30;                 //H08_30
    Uint16 GN_Rsvd31;                 //H08_31
    Uint16 GN_Rsvd32;                 //H08_32
    Uint16 GN_Rsvd33;                 //H08_33
    Uint16 GN_Rsvd34;                 //H08_34
    Uint16 GN_Rsvd35;                 //H08_35
    Uint16 GN_Rsvd36;                 //H08_36
    Uint16 GN_Rsvd37;                 //H08_37
    Uint16 GN_Rsvd38;                 //H08_38
    Uint16 GN_Rsvd39;                 //H08_39                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_30*/  0,              0,                  65535,              RSVD_WRT,
/*H08_31*/  0,              0,                  65535,              RSVD_WRT,
/*H08_32*/  0,              0,                  65535,              RSVD_WRT,
/*H08_33*/  0,              0,                  65535,              RSVD_WRT,
/*H08_34*/  0,              0,                  65535,              RSVD_WRT,
/*H08_35*/  0,              0,                  65535,              RSVD_WRT,
/*H08_36*/  0,              0,                  65535,              RSVD_WRT,
/*H08_37*/  0,              0,                  65535,              RSVD_WRT,
/*H08_38*/  0,              0,                  65535,              RSVD_WRT,
/*H08_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 GN_Rsvd40;                 //H08_40
    Uint16 GN_Rsvd41;                 //H08_41
    Uint16 GN_Rsvd42;                 //H08_42
    Uint16 GN_Rsvd43;                 //H08_43
    Uint16 GN_Rsvd44;                 //H08_44
    Uint16 GN_Rsvd45;                 //H08_45
    Uint16 GN_Rsvd46;                 //H08_46
    Uint16 GN_Rsvd47;                 //H08_47
    Uint16 GN_Rsvd48;                 //H08_48
    Uint16 GN_Rsvd49;                 //H08_49                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_40*/  0,              0,                  65535,              RSVD_WRT,
/*H08_41*/  0,              0,                  65535,              RSVD_WRT,
/*H08_42*/  0,              0,                  65535,              RSVD_WRT,
/*H08_43*/  0,              0,                  65535,              RSVD_WRT,
/*H08_44*/  0,              0,                  65535,              RSVD_WRT,
/*H08_45*/  0,              0,                  65535,              RSVD_WRT,
/*H08_46*/  0,              0,                  65535,              RSVD_WRT,
/*H08_47*/  0,              0,                  65535,              RSVD_WRT,
/*H08_48*/  0,              0,                  65535,              RSVD_WRT,
/*H08_49*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 GN_Rsvd50;                 //H08_50
    Uint16 GN_Rsvd51;                 //H08_51
    Uint16 GN_Rsvd52;                 //H08_52
    Uint16 GN_Rsvd53;                 //H08_53
    Uint16 GN_Rsvd54;                 //H08_54
    Uint16 GN_Rsvd55;                 //H08_55
    Uint16 GN_Rsvd56;                 //H08_56
    Uint16 GN_Rsvd57;                 //H08_57
    Uint16 GN_Rsvd58;                 //H08_58
    Uint16 GN_Rsvd59;                 //H08_59                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_50*/  0,              0,                  65535,              RSVD_WRT,
/*H08_51*/  0,              0,                  65535,              RSVD_WRT,
/*H08_52*/  0,              0,                  65535,              RSVD_WRT,
/*H08_53*/  0,              0,                  65535,              RSVD_WRT,
/*H08_54*/  0,              0,                  65535,              RSVD_WRT,
/*H08_55*/  0,              0,                  65535,              RSVD_WRT,
/*H08_56*/  0,              0,                  65535,              RSVD_WRT,
/*H08_57*/  0,              0,                  65535,              RSVD_WRT,
/*H08_58*/  0,              0,                  65535,              RSVD_WRT,
/*H08_59*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 GN_Rsvd60;                 //H08_60
    Uint16 GN_Rsvd61;                 //H08_61
    Uint16 GN_Rsvd62;                 //H08_62
    Uint16 GN_Rsvd63;                 //H08_63
    Uint16 GN_Rsvd64;                 //H08_64
    Uint16 GN_Rsvd65;                 //H08_65
    Uint16 GN_Rsvd66;                 //H08_66
    Uint16 GN_Rsvd67;                 //H08_67
    Uint16 GN_Rsvd68;                 //H08_68
    Uint16 GN_Rsvd69;                 //H08_69                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_60*/  0,              0,                  65535,              RSVD_WRT,
/*H08_61*/  0,              0,                  65535,              RSVD_WRT,
/*H08_62*/  0,              0,                  65535,              RSVD_WRT,
/*H08_63*/  0,              0,                  65535,              RSVD_WRT,
/*H08_64*/  0,              0,                  65535,              RSVD_WRT,
/*H08_65*/  0,              0,                  65535,              RSVD_WRT,
/*H08_66*/  0,              0,                  65535,              RSVD_WRT,
/*H08_67*/  0,              0,                  65535,              RSVD_WRT,
/*H08_68*/  0,              0,                  65535,              RSVD_WRT,
/*H08_69*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 GN_Rsvd70;                 //H08_70
    Uint16 GN_Rsvd71;                 //H08_71
    Uint16 GN_Rsvd72;                 //H08_72
    Uint16 GN_Rsvd73;                 //H08_73
    Uint16 GN_Rsvd74;                 //H08_74
    Uint16 GN_Rsvd75;                 //H08_75
    Uint16 GN_Rsvd76;                 //H08_76
    Uint16 GN_Rsvd77;                 //H08_77
    Uint16 GN_Rsvd78;                 //H08_78
    Uint16 GN_Rsvd79;                 //H08_79                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_70*/  0,              0,                  65535,              RSVD_WRT,
/*H08_71*/  0,              0,                  65535,              RSVD_WRT,
/*H08_72*/  0,              0,                  65535,              RSVD_WRT,
/*H08_73*/  0,              0,                  65535,              RSVD_WRT,
/*H08_74*/  0,              0,                  65535,              RSVD_WRT,
/*H08_75*/  0,              0,                  65535,              RSVD_WRT,
/*H08_76*/  0,              0,                  65535,              RSVD_WRT,
/*H08_77*/  0,              0,                  65535,              RSVD_WRT,
/*H08_78*/  0,              0,                  65535,              RSVD_WRT,
/*H08_79*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 GN_Rsvd80;                 //H08_80
    Uint16 GN_Rsvd81;                 //H08_81
    Uint16 GN_Rsvd82;                 //H08_82
    Uint16 GN_Rsvd83;                 //H08_83
    Uint16 GN_Rsvd84;                 //H08_84
    Uint16 GN_Rsvd85;                 //H08_85
    Uint16 GN_Rsvd86;                 //H08_86
    Uint16 GN_Rsvd87;                 //H08_87
    Uint16 GN_Rsvd88;                 //H08_88
    Uint16 MemCheck08;                //H08_89 组校验字                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H08_80*/  0,              0,                  65535,              RSVD_WRT,
/*H08_81*/  0,              0,                  65535,              RSVD_WRT,
/*H08_82*/  0,              0,                  65535,              RSVD_WRT,
/*H08_83*/  0,              0,                  65535,              RSVD_WRT,
/*H08_84*/  0,              0,                  65535,              RSVD_WRT,
/*H08_85*/  0,              0,                  65535,              RSVD_WRT,
/*H08_86*/  0,              0,                  65535,              RSVD_WRT,
/*H08_87*/  0,              0,                  65535,              RSVD_WRT,
/*H08_88*/  0,              0,                  65535,              RSVD_WRT,
/*H08_89*/  0x8A5C,         0,                  65535,              RSVD_WRT,


//----------------------------自整定参数  H09LEN = 40 -------------------------------------------------------
/*  Uint16 AutoTuningModeSel;           //H09_00 自调整模式选择
    Uint16 Rigidity_Level1st;           //H09_01 第1组刚性等级选择
    Uint16 TL_AdaptiveFilterMode;       //H09_02 自适应滤波器模式选择，模仿松下 
    Uint16 InertiaIdyCountModeSel;      //H09_03 在线惯量辨识模式
    Uint16 AT_LowOscMod;                //H09_04 低频振动模式选择
    Uint16 OffLnInertiaModeSel;         //H09_05 离线惯量辨识模式
    Uint16 InertiaSpdMax;               //H09_06 惯量辨识时到达的最大速度
    Uint16 InertiaAcceTime ;            //H09_07 惯量辨识时加速至最大速度时间
    Uint16 InertiaWaitTime;             //H09_08 每次惯量辨识之后的等待时间
    Uint16 InertiaIdy_Circle;           //H09_09 完整单次惯量辨识转动圈数    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_00*/  0,              0,                  4,                   ANY_WRT|  DISP_1,
/*H09_01*/  12,             0,                  31,                  ANY_WRT|  DISP_2,
/*H09_02*/  0,              0,                  4,                   ANY_WRT|  DISP_1,
/*H09_03*/  0,              0,                  3,                   ANY_WRT|  DISP_1,
/*H09_04*/  0,              0,                  1,                   ANY_WRT|  DISP_1,
/*H09_05*/  0,              0,                  1,                  POSD_WRT|  DISP_1,
/*H09_06*/  500,           100,                 1000,               POSD_WRT|  DISP_4,
/*H09_07*/  125,            20,                  800,               POSD_WRT|  DISP_3,
/*H09_08*/  800,           50,                  10000,              POSD_WRT|  DISP_5,
/*H09_09*/  0,              0,                  65535,              DISP_WRT|  DISP_5|  TWO_DOT,

/*  Uint16 AT_RespnLevel;                //H09_10 一键式调整响应设置
	Uint16 AT_VibThrshld;                //H09_11 一键式调整振动判定阈值
    Uint16 TL_NotchFiltFreqA;           //H09_12 第1组陷波器频率
    Uint16 TL_NotchFiltBandWidthA;      //H09_13 第1组陷波器宽度等级
    Uint16 TL_NotchFiltAttenuatLvlA;    //H09_14 第1组陷波器衰减等级
    Uint16 TL_NotchFiltFreqB;           //H09_15 第2组陷波器频率
    Uint16 TL_NotchFiltBandWidthB;      //H09_16 第2组陷波器宽度等级
    Uint16 TL_NotchFiltAttenuatLvlB;    //H09_17 第2组陷波器衰减等级
    Uint16 TL_NotchFiltFreqC;           //H09_18 第3组陷波器频率
    Uint16 TL_NotchFiltBandWidthC;      //H09_19 第3组陷波器宽度等级              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_10*/  1,              0,                  2,                   ANY_WRT|  DISP_2,
/*H09_11*/  20,             0,                  1000,              ANY_WRT|  ONE_DOT | DISP_4,
/*H09_12*/  4000,           50,                 4000,               ANY_WRT|    DISP_4,
/*H09_13*/  2,              0,                  20,                 ANY_WRT|    DISP_2,
/*H09_15*/  0,             0,                  99,                 ANY_WRT|    DISP_3,
/*H09_15*/  4000,           50,                 4000,               ANY_WRT|    DISP_4,
/*H09_16*/  2,              0,                  20,                 ANY_WRT|    DISP_2,
/*H09_17*/  0,             0,                  99,                 ANY_WRT|    DISP_3,
/*H09_18*/  4000,           50,                 4000,               ANY_WRT|    DISP_4,
/*H09_19*/  2,              0,                  20,                 ANY_WRT|    DISP_2,

/*  Uint16 TL_NotchFiltAttenuatLvlC;    //H09_20 第3组陷波器衰减等级
    Uint16 TL_NotchFiltFreqD;           //H09_21 第4组陷波器频率
    Uint16 TL_NotchFiltBandWidthD;      //H09_22 第4组陷波器宽度等级
    Uint16 TL_NotchFiltAttenuatLvlD;    //H09_23 第4组陷波器衰减等级
    Uint16 TL_NotchFiltFreqDisp;        //H09_24 共振频率辨识结果
    Uint16 AT_LoadMode;                   //H09_25 一键式调整对应的外部模式(轨迹模式还是定位模式) 
    Uint16 AT_Rsvd26;                   //H09_26 
    Uint16 AT_Rsvd27;                   //H09_27
    Uint16 AT_Rsvd28;                   //H09_28
    Uint16 AT_Rsvd29;                   //H09_29               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_20*/  0,             0,                  99,                  ANY_WRT|    DISP_3,
/*H09_21*/  4000,           50,                 4000,                ANY_WRT|    DISP_4,
/*H09_22*/  2,              0,                 20,                ANY_WRT|    DISP_2,
/*H09_23*/  0,             0,                  99,                  ANY_WRT|    DISP_3,
/*H09_24*/  0,              0,                  4000,               DISP_WRT|    DISP_4,
/*H09_25*/  1,              0,                  2,                  ANY_WRT|    DISP_1,
/*H09_26*/  0,              0,                  65535,              RSVD_WRT,
/*H09_27*/  0,              0,                  65535,              RSVD_WRT,
/*H09_28*/  0,              0,                  65535,              RSVD_WRT,
/*H09_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 AT_ToqDisKp;                 //H09_30 扰动转矩补偿增益
    Uint16 AT_DobFiltTime;              //H09_31 扰动观测器滤波时间
    Uint16 AT_ConstToqComp;             //H09_32 恒定转矩补偿值
    Uint16 AT_ToqPlusComp;              //H09_33 正向摩擦补偿
    Uint16 AT_ToqMinusComp;             //H09_34 反向摩擦补偿
    Uint16 AT_Rsvd35;                   //H09_35 
    Uint16 AT_Rsvd36;                   //H09_36 
    Uint16 AT_Rsvd37;                   //H09_37 
    Uint16 AT_LowOscFreqA;              //H09_38 第一个低频振动频率
    Uint16 AT_LowOscFiltA;              //H09_39 第一个低频共振频率滤波设定             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_30*/  0,              (Uint16)-1000,      1000,                ANY_WRT| DISP_5 | ONE_DOT | INT_SIGN,
/*H09_31*/  50,             0,                  2500,                ANY_WRT| DISP_5 | TWO_DOT,
/*H09_32*/  0,              (Uint16)-1000,      1000,                ANY_WRT| DISP_5 | ONE_DOT | INT_SIGN,
/*H09_33*/  0,              (Uint16)-1000,      1000,                ANY_WRT| DISP_5 | ONE_DOT | INT_SIGN,
/*H09_34*/  0,              (Uint16)-1000,      1000,                ANY_WRT| DISP_5 | ONE_DOT | INT_SIGN,
/*H09_35*/  0,              0,                  500,                 RSVD_WRT,
/*H09_36*/  0,              0,                  500,                 RSVD_WRT,
/*H09_37*/  0,              0,                  65535,              RSVD_WRT,
/*H09_38*/  1000,           10,                  1000,              POSD_WRT| DISP_4| ONE_DOT,
/*H09_39*/  2,             0,                   10,                 POSD_WRT| DISP_1,

/*  Uint16 AT_LowFreqRatio;             //H09_40  低频抖动分母频率和分子频率比
    Uint16 AT_Rsvd41;                   //H09_41
    Uint16 AT_Rsvd42;                   //H09_42  一键式调整时运行圈数低16位
    Uint16 AT_Rsvd43;                   //H09_43  一键式调整时运行圈数高16位
    Uint16 AT_Rsvd44;                   //H09_44  一键式调整时运行速度,rpm
    Uint16 AT_Rsvd45;                   //H09_45  一键式调整时运行加减速时间,ms
    Uint16 AT_Rsvd46;                   //H09_46  一键式调整时运行等待时间,ms
    Uint16 AT_Rsvd47;                   //H09_47
    Uint16 AT_Rsvd48;                   //H09_48
    Uint16 AT_Rsvd49;                   //H09_49                        */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_40*/  12,              0,                 30,                 POSD_WRT| DISP_2| ONE_DOT,
/*H09_41*/  0,              0,                  65535,              RSVD_WRT,
/*H09_42*/  40000,           0,                  0xffff,             ANY_WRT| TWO_WORD| DISP_11,
/*H09_43*/  0,              0,                  0x7fff,             ANY_WRT| TWO_WORD| DISP_11| HIGH_WORD,
/*H09_44*/  400,            0,                  3000,              ANY_WRT| DISP_4,
/*H09_45*/  100,            0,                  20000,              ANY_WRT| DISP_5,
/*H09_46*/  500,           0,                  20000,              ANY_WRT| DISP_5,
/*H09_47*/  0,              0,                  65535,              RSVD_WRT,
/*H09_48*/  0,              0,                  65535,              RSVD_WRT,
/*H09_49*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 AT_Rsvd50;                   //H09_50
    Uint16 AT_Rsvd51;                   //H09_51
    Uint16 AT_Rsvd52;                   //H09_52
    Uint16 AT_Rsvd53;                   //H09_53
    Uint16 AT_Rsvd54;                   //H09_54
    Uint16 AT_Rsvd55;                   //H09_55
    Uint16 AT_Rsvd56;                   //H09_56
    Uint16 AT_Rsvd57;                   //H09_57
    Uint16 AT_Rsvd58;                   //H09_58
    Uint16 MemCheck09;                  //H09_59        组校验字                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H09_50*/  0,              0,                  65535,              RSVD_WRT,
/*H09_51*/  0,              0,                  65535,              RSVD_WRT,
/*H09_52*/  0,              0,                  65535,              RSVD_WRT,
/*H09_53*/  0,              0,                  65535,              RSVD_WRT,
/*H09_54*/  0,              0,                  65535,              RSVD_WRT,
/*H09_55*/  0,              0,                  65535,              RSVD_WRT,
/*H09_56*/  0,              0,                  65535,              RSVD_WRT,
/*H09_57*/  0,              0,                  65535,              RSVD_WRT,
/*H09_58*/  0,              0,                  65535,              RSVD_WRT,
/*H09_59*/  0x9A5C,         0,                  65535,              RSVD_WRT,


//----------------------------故障与保护参数  H0ALEN = 32 -------------------------------------------------------
/*  Uint16 ER_InPLProtectSel;           //H0A00 电源输入缺相保护选择
    Uint16 AbsPosActSet;                //H0A01 
    Uint16 ER_Rsvd02;                   //H0A02 模块温度警告点    (删掉)
    Uint16 ER_PowerOffSaveToEeprom；    //H0A03 是否执行掉电保存EEPROM
    Uint16 ER_MotOLProtect_Kp;          //H0A04 电机过载保护增益
    Uint16 ER_AngCnt;                   //H0A05 角度辨识时电周期分成的份数——不开放
    Uint16 ER_MotorLoadRate;            //H0A06 电机过载等级
    Uint16 ER_UVWIdenEn;                //H0A07 UVW相序辨识使能                                      
    Uint16 ER_UdcLowFncSel;             //H0A08 过速判断阀值
    Uint16 ER_MaxPulsFreq;              //H0A09 位置最大脉冲输入频率                                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0A_00*/  0,              0,                  2,                   ANY_WRT|    DISP_1,
/*H0A_01*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H0A_02*/  0,              0,                  65535,              RSVD_WRT,
/*H0A_03*/  0,              0,                  2,                   ANY_WRT|    DISP_1,
/*H0A_04*/  100,            50,                 300,                POSD_WRT|    DISP_3,
/*H0A_05*/  8,              0,                  40,                 ANY_WRT|    DISP_2,
/*H0A_06*/  0,              0,                  400,              POSD_WRT|    DISP_3,
#if NONSTANDARD_PROJECT == LINEARMOT
/*H0A_07*/  0,              0,                  1,                  ANY_WRT| DISP_1,
#else
/*H0A_07*/  1,              0,                  1,                  ANY_WRT| DISP_1,
#endif
/*H0A_08*/  0,              0,                  10000,              ANY_WRT| DISP_5,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0A_09*/ 1000,            100,                1000,               POSD_WRT|    DISP_4,
#else
/*H0A_09*/ 4000,            100,                4000,               POSD_WRT|    DISP_4,
#endif

/*  Uint16 ER_PerrFaultVluLow;          //H0A10 位置偏差过大故障设定值低16位
    Uint16 ER_PerrFaultVluHigh;         //H0A11 位置偏差过大故障设定值高16位
    Uint16 ER_RunAwaySel;               //H0A12 飞车保护选择 1飞车报警  0 屏蔽飞车报警   默认1
    Uint16 ER_AngIntSel;                //H0A13 初始角度辨识方式选择
    Uint16 ER_DutySearchEn;             //H0A14 电压注入辨识时占空比搜索使能
    Uint16 ER_MotRunPuls;               //H0A15 电机转动判定阈值——用于不找Z的角度辨识
    Uint16 ER_LowOscPuls;               //H0A16 伺服低频振动位置偏差判断阈值
    Uint16 ER_BlkOverCurrent;           //H0A17 制动电流百分比
    Uint16 ER_DriverTempErr;            //H0A18 驱动器过温保护点
    Uint16 ER_Di8FiltTm;                //H0A19 DI8滤波时间                                                     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0A_10*/  32767,          1,                  0,                   ANY_WRT|  TWO_WORD|   DISP_10,
/*H0A_11*/  0,              0,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H0A_12*/  1,              0,                  1,                   ANY_WRT|  DISP_1,
/*H0A_13*/  0,              0,                  3,                  POSD_WRT|  DISP_1,
/*H0A_14*/  1,              0,                  1,                  POSD_WRT|  DISP_1,
/*H0A_15*/  5,              1,                  1000,                ANY_WRT|  DISP_4,
/*H0A_16*/  5,              0,                  1000,                ANY_WRT|  DISP_4,
/*H0A_17*/  120,            50,                 200,                 ANY_WRT|  DISP_3,
#if POWERDRIVER_TYPE==POWDRV_IS650
/*H0A_18*/  0,              0,                  105,                POSD_WRT|  DISP_3| NEXT_ACT,
#else
/*H0A_18*/  0,              0,                  100,                POSD_WRT|  DISP_3| NEXT_ACT,
#endif
/*H0A_19*/  80,             0,                  255,                POSD_WRT|  DISP_3| NEXT_ACT,

/*  Uint16 ER_Di9FiltTm;                //H0A20 DI9滤波时间
    Uint16 ER_StoFiltTm;                //H0A21 STO信号滤波时间
    Uint16 ER_SdmFiltTm;                //H0A22 Sigma_Delta滤波时间
    Uint16 ER_GkFiltTm;                 //H0A23 TZ信号滤波时间
    Uint16 ER_PosCmdFiltTm;             //H0A24 低速输入脉冲滤波时间
    Uint16 ER_SpdDispFilt;              //H0A25 速度显示滤波时间
    Uint16 ER_MTOLClose;                //H0A26 关闭过载警告
    Uint16 ER_SpdDoFilt;                //H0A27 速度Do滤波时间
    Uint16 ER_RotEncFiltTm;             //H0A28 正交编码器滤波时间
    Uint16 ER_LnrEncFiltTm;             //H0A29 直线编码器滤波时间           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0A_20*/  80,             0,                  255,                POSD_WRT|    DISP_3| NEXT_ACT,
/*H0A_21*/  0,              0,                  500,                POSD_WRT|   TWO_DOT| DISP_4| NEXT_ACT,
/*H0A_22*/  1,              0,                  3,                  POSD_WRT|    DISP_1|  NEXT_ACT,
/*H0A_23*/  15,             0,                  31,                 POSD_WRT|    DISP_2|  NEXT_ACT,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0A_24*/  0,             0,                  65535,                RSVD_WRT,
#else
/*H0A_24*/  30,             0,                  255,                POSD_WRT|    DISP_3|  NEXT_ACT,
#endif

/*H0A_25*/  50,             0,                  5000,               POSD_WRT|    DISP_4,
/*H0A_26*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H0A_27*/  10,             0,                  5000,               POSD_WRT|    DISP_4,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0A_28*/  5,              0,                  255,                POSD_WRT|    DISP_3|  NEXT_ACT,
#else
/*H0A_28*/  30,              0,                  255,                POSD_WRT|    DISP_3|  NEXT_ACT,
#endif
/*H0A_29*/  15,             0,                  255,                POSD_WRT|    DISP_3|  NEXT_ACT,

/*  Uint16 ER_PosCmdFiltTmHigh;         //H0A30 高速速输入脉冲滤波时间
    Uint16 NKAbsEncRstPwrOn;            //H0A31 尼康编码器上电复位
    Uint16 ER_LockedRotorTimer;         //H0A32 堵转过温保护时间窗口
    Uint16 ER_LockedRotorSel;           //H0A33 堵转过温保护使能
    Uint16 EncFrameMode;                //H0A34 2代编码器帧格式
    Uint16 DisRdEncEepromPwrOn;         //H0A35 上电禁止读取编码器EEPROM
    Uint16 EncMultOvDisable;            //H0A36 编码器多圈溢出故障禁止
    Uint16 ER_Rsvd37;                   //H0A37
    Uint16 ER_Rsvd38;                   //H0A38	
    Uint16 MemCheck0A;                  //H0A39 组校验字                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0A_30*/  3,              0,                  255,                POSD_WRT|    DISP_3|  NEXT_ACT,
/*H0A_31*/  0,              0,                  1,                   ANY_WRT|    DISP_1|  NEXT_ACT,
/*H0A_32*/  200,            10,                 65535,               ANY_WRT|    DISP_5,
/*H0A_33*/  1,              0,                  1,                   ANY_WRT|    DISP_1,
/*H0A_34*/  0,              0,                  2,                   ANY_WRT|    DISP_1|  NEXT_ACT,
/*H0A_35*/  0,              0,                  1,                   ANY_WRT|    DISP_1|  NEXT_ACT,
/*H0A_36*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
#if DRIVER_TYPE == SERVO_650N
/*H0A_37*/  0,              0,                  1,                   POSD_WRT|    DISP_1| NEXT_ACT,
#else
/*H0A_37*/  0,              0,                  65535,              RSVD_WRT,
#endif
/*H0A_38*/  0,              0,                  65535,              	RSVD_WRT,
/*H0A_39*/  0xAA5C,         0,                  65535,              RSVD_WRT,


//----------------------------通信参数  H0CLEN = 50 -------------------------------------------------------
/*  Uint16 CM_AxisAdress;               //H0C_00 轴/驱动器地址
    Uint16 CM_Rsvd01;                   //H0C_01
    Uint16 CM_BodeRate;                 //H0C_02 RS232波特率设置
    Uint16 CM_Parity;                   //H0C_03 奇偶校验设置
    Uint16 CM_StationAdd;               //H0C_04 EtherCAT从站站点正名
    Uint16 CM_StationAlias;             //H0C_05 EtherCAT从站站点别名
    Uint16 CM_Rsvd06;                   //H0C_06
    Uint16 CM_Rsvd07;                   //H0C_07
    Uint16 CM_CanSpdSel;                //H0C_08 CAN通信速率选择
    Uint16 CM_UseVDI;                   //H0C_09 是否使用VDI             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0C_00*/  1,              1,                  247,                 ANY_WRT|    DISP_3,
/*H0C_01*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_02*/  5,              0,                  6,                   ANY_WRT|    DISP_1,
/*H0C_03*/  0,              0,                  3,                   ANY_WRT|    DISP_1,
#if ECT_ENABLE_SWITCH
/*H0C_04*/  0,              0,                  65535,              DISP_WRT|DISP_5,
/*H0C_05*/  0,              0,                  65535,              POSD_WRT|DISP_5,
#else
/*H0C_04*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_05*/  0,              0,                  65535,              RSVD_WRT,
#endif
/*H0C_06*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_07*/  0,              0,                  65535,              RSVD_WRT,
#if ECT_ENABLE_SWITCH
/*H0C_08*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H0C_08*/  5,              0,                  7,                  ANY_WRT|DISP_1,
#endif
/*H0C_09*/  0,              0,                  1,                  POSD_WRT|DISP_1,

/*  Uint16 CM_VDIDefaultValue;          //H0C_10 上电后VDI默认值
    Uint16 CM_UseVDO;                   //H0C_11 是否使用VDO
    Uint16 CM_VDODefaultValue;          //H0C_12 VDO功能选择为0时默认值
    Uint16 CM_WriteEepromEnable;        //H0C_13 写Eeprom开关
    Uint16 CM_ErrorType;                //H0C_14 区分错误码类型bywshp1013
    Uint16 CM_Rsvd15;                   //H0C_15
    Uint16 CM_Rsvd16;                   //H0C_16
    Uint16 CM_Rsvd17;                   //H0C_17
    Uint16 CM_Rsvd18;                   //H0C_18
    Uint16 CM_Rsvd19;                   //H0C_19          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*HOC_10*/  0,              0,                  65535,              ANY_WRT|DISP_5|NEXT_ACT,
/*H0C_11*/  0,              0,                  1,                  POSD_WRT|DISP_1,
/*H0C_12*/  0,              0,                  65535,              POSD_WRT|HNUM_TYP|DISP_4,
#if ECT_ENABLE_SWITCH
/*H0C_13*/  3,              0,                  3,                  ANY_WRT|DISP_1,
#else
/*H0C_13*/  1,              0,                  1,                  ANY_WRT|DISP_1,
#endif
/*H0C_14*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|DISP_4|PANEL_UNCHANGE,
/*H0C_15*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_16*/  0,              0,                  65535,              RSVD_WRT,  
/*H0C_17*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_18*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_19*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 CM_Rsvd20;                   //H0C_20
    Uint16 CM_Rsvd21;                   //H0C_21
    Uint16 CM_Rsvd22;                   //H0C_22
    Uint16 CM_Rsvd23;                   //H0C_23
    Uint16 CM_Rsvd24;                   //H0C_24
    Uint16 CM_SendDelay;                //H0C_25 MODBUS应答发送延时
    Uint16 CM_Modbus32BitsSeq;          //H0C_26 32位高低位次序选择
    Uint16 CM_Rsvd27;                   //H0C_27
    Uint16 CM_Rsvd28;                   //H0C_28
    Uint16 CM_Rsvd29;                   //H0C_29              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/ 
/*H0C_20*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_21*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_22*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_23*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_24*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_25*/  1,              0,                  5000,                ANY_WRT|   DISP_4,
/*H0C_26*/  1,              0,                  1,                   ANY_WRT|   DISP_1,
/*H0C_27*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_28*/  0,              0,                  65535,              RSVD_WRT, 
/*H0C_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 CM_ModbusErrFrameType;       //H0C_30 错误帧类型  1标准  0兼容以前
    Uint16 ModbusRcvDeal;               //H0C_31 Modbus接收处理 0 使能接收中断 1电流环中断查询
    Uint16 CM_XmlVersion;               //H0C_32 XML版本
    Uint16 CM_AlStatusCode;             //H0C_33 AL状态码
    Uint16 CM_SyncErrCnt0;              //H0C_34 同步丢失次数
    Uint16 CM_SyncErrCnt;               //H0C_35  EtherCAT同步中断丢失允许次数
    Uint16 CM_Port0CRC;                 //H0C_36 端口0  无效帧
    Uint16 CM_Port1CRC;                 //H0C_37 端口1  无效帧
    Uint16 CM_ForwardErr;               //H0C_38 端口0、1 转发错误
    Uint16 CM_HandleErr;                //H0C_39 处理单元和PDI 错误    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0C_30*/  1,              0,                  1,                   ANY_WRT|    DISP_1,
/*H0C_31*/  0,              0,                  1,                   ANY_WRT|    DISP_1|  NEXT_ACT,
#if ECT_ENABLE_SWITCH
/*H0C_32*/  0,              0,                  65535,              DISP_WRT|TWO_DOT| DISP_5,
/*H0C_33*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
/*H0C_34*/  0,              0,                  65535,              DISP_WRT|DISP_5,
/*H0C_35*/  9,              0,                  65535,              ANY_WRT |DISP_5,
/*H0C_36*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
/*H0C_37*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
/*H0C_38*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
/*H0C_39*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
#else
/*H0C_32*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_33*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_34*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_35*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_36*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_37*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_38*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_39*/  0,              0,                  65535,              RSVD_WRT,
#endif
/*  Uint16 CM_LinkLost;                 //H0C_40 端口0、1 端口丢失
    Uint16 CM_ECATHost;                 //H0C_41 EtherCAT主站选择 非标功能，为匹配各家的MC模块
    Uint16 CM_SyncDetecMethod;          //H0C_42 同步检测机制
    Uint16 CM_FpgaSyncModeSel;          //H0C_43 FPGA同步模式选择  0-自同步 1-OP模式下 载波与SYNC同步  2-位置环同步
    Uint16 CM_SyncErrThreshold;         //H0C_44 FPGA同步检测偏差阈值
    Uint16 CM_PosBuffEnale;             //H0C_45 位置缓存选择 0--无缓存  1--1个缓存
    Uint16 CM_CSPCmdIncErrCnt;          //H0C_46 CSP位置指令增量过大阈值
    Uint16 CM_CSPCmdErrCnt;             //H0C_47 CSP位置指令增量过大次数
    Uint16 CM_ESM;                      //H0C_48 EtherCAT状态机
    Uint16 MemCheck0C;                  //H0C_49 组校验字                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if ECT_ENABLE_SWITCH
/*H0C_40*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|   DISP_4,
/*H0C_41*/  2,              0,                  3,                  POSD_WRT|   DISP_1|  NEXT_ACT,
/*H0C_42*/  0,              0,                  1,                  POSD_WRT|   DISP_1,
/*H0C_43*/  1,              0,                  2,                  POSD_WRT|   DISP_1,
/*H0C_44*/  3000,           0,                  4000,               POSD_WRT|   DISP_4,
/*H0C_45*/  0,              0,                  1,                  POSD_WRT|   DISP_1,
/*H0C_46*/  3,              1,                  7,                  ANY_WRT|    DISP_1,
/*H0C_47*/  0,              0,                  65535,              DISP_WRT|   DISP_5,
/*H0C_48*/  0,              0,                  65535,              DISP_WRT|HNUM_TYP|DISP_2,
#else
/*H0C_40*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_41*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_42*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_43*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_44*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_45*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_46*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_47*/  0,              0,                  65535,              RSVD_WRT,
/*H0C_48*/  0,              0,                  65535,              RSVD_WRT,
#endif
/*H0C_49*/  0xCA5C,         0,                  65535,              RSVD_WRT,


//----------------------------预留参数组  H0ELEN = 50 ------------------------------------------------------- 
/*  Uint16 H0E_Rsvd00;                  //H0E_00
    Uint16 H0E_Rsvd01;                  //H0E_01
    Uint16 H0E_Rsvd02;                  //H0E_02
    Uint16 H0E_Rsvd03;                  //H0E_03
    Uint16 H0E_Rsvd04;                  //H0E_04
    Uint16 H0E_Rsvd05;                  //H0E_05
    Uint16 H0E_Rsvd06;                  //H0E_06
    Uint16 H0E_Rsvd07;                  //H0E_07
    Uint16 H0E_Rsvd08;                  //H0E_08
    Uint16 H0E_Rsvd09;                  //H0E_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0E_00*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_01*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_02*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_03*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_04*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_05*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_06*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_07*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_08*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_09*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H0E_Rsvd10;                  //H0E_10
    Uint16 H0E_Rsvd11;                  //H0E_11
    Uint16 H0E_Rsvd12;                  //H0E_12
    Uint16 H0E_Rsvd13;                  //H0E_13
    Uint16 H0E_Rsvd14;                  //H0E_14
    Uint16 H0E_Rsvd15;                  //H0E_15
    Uint16 H0E_Rsvd16;                  //H0E_16
    Uint16 H0E_Rsvd17;                  //H0E_17
    Uint16 H0E_Rsvd18;                  //H0E_18
    Uint16 H0E_Rsvd19;                  //H0E_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0E_10*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_11*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_12*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_13*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_14*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_15*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_16*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_17*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_18*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_19*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H0E_Rsvd20;                  //H0E_20
    Uint16 H0E_Rsvd21;                  //H0E_21
    Uint16 H0E_Rsvd22;                  //H0E_22
    Uint16 H0E_Rsvd23;                  //H0E_23
    Uint16 H0E_Rsvd24;                  //H0E_24
    Uint16 H0E_Rsvd25;                  //H0E_25
    Uint16 H0E_Rsvd26;                  //H0E_26
    Uint16 H0E_Rsvd27;                  //H0E_27
    Uint16 H0E_Rsvd28;                  //H0E_28
    Uint16 H0E_Rsvd29;                  //H0E_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0E_20*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_21*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_22*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_23*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_24*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_25*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_26*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_27*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_28*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H0E_Rsvd30;                  //H0E_30
    Uint16 H0E_Rsvd31;                  //H0E_31
    Uint16 H0E_Rsvd32;                  //H0E_32
    Uint16 H0E_Rsvd33;                  //H0E_33
    Uint16 H0E_Rsvd34;                  //H0E_34
    Uint16 H0E_Rsvd35;                  //H0E_35
    Uint16 H0E_Rsvd36;                  //H0E_36
    Uint16 H0E_Rsvd37;                  //H0E_37
    Uint16 H0E_Rsvd38;                  //H0E_38
    Uint16 H0E_Rsvd39;                  //H0E_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0E_30*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_31*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_32*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_33*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_34*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_35*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_36*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_37*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_38*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_39*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 H0E_Rsvd40;                  //H0E_40
    Uint16 H0E_Rsvd41;                  //H0E_41
    Uint16 H0E_Rsvd42;                  //H0E_42
    Uint16 H0E_Rsvd43;                  //H0E_43
    Uint16 H0E_Rsvd44;                  //H0E_44
    Uint16 H0E_Rsvd45;                  //H0E_45
    Uint16 H0E_Rsvd46;                  //H0E_46
    Uint16 H0E_Rsvd47;                  //H0E_47
    Uint16 H0E_Rsvd48;                  //H0E_48
    Uint16 MemCheck0E;                  //H0E_49     组校验字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0E_40*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_41*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_42*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_43*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_44*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_45*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_46*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_47*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_48*/  0,              0,                  65535,              RSVD_WRT,
/*H0E_49*/  0xEA5C,         0,                  65535,              RSVD_WRT,


//----------------------------预留参数组  H0FLEN = 50 ------------------------------------------------------- 
/* 
   Uint16 FC_FeedbackMode;			  //H0F_00 编码器反馈模式
   Uint16 FC_ExCoderDir;			  //H0F_01 外部编码器使用方式
   Uint16 FC_ExCoderLine_L; 		  //H0F_02 外部编码器线数
   Uint16 FC_ExCoderLine_H; 		  //H0F_03外部编码器线数
   Uint16 FC_ExCoderPulse_L;		  //H0F_04 电机旋转一圈外部编码器脉冲数
   Uint16 FC_ExCoderPulse_H;		  //H0F_05 电机旋转一圈外部编码器脉冲数    Uint16 FC_Rsvd05;				  //H0F_05
   Uint16 FC_ExKpCoff;				 //H0F_06,外部增益系数
    Uint16 H0F_Rsvd07;                      //H0F_07
    Uint16 FC_MixCtrlMaxPulse_L;        //H0F_08 混合控制偏差最大值
    Uint16 FC_MixCtrlMaxPulse_H;        //H0F_09
    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if ECT_ENABLE_SWITCH
/*H0F_00*/  0,              0,                  1,                   POSD_WRT|    DISP_1,
#else
/*H0F_00*/  0,              0,                  3,                   POSD_WRT|    DISP_1,
#endif
/*H0F_01*/  0,              0,                  1,                   POSD_WRT|    DISP_1,
/*H0F_02*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_03*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_04*/  10000,          1,                  0,                   POSD_WRT|  TWO_WORD|   DISP_11,
/*H0F_05*/  0,              0,                  0x4000,              POSD_WRT|  TWO_WORD|   DISP_11| HIGH_WORD|NEXT_ACT,
/*H0F_06*/  100,            0,                  65535,               RSVD_WRT| DISP_5|TWO_DOT,
/*H0F_07*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_08*/  1000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11,
/*H0F_09*/  0,              0,                 0x4000,               ANY_WRT|  TWO_WORD|   DISP_11| HIGH_WORD,

/*  Uint16 FC_MixCtrlPulseClr;         //H0F_10 混合控制偏差清除设定
    Uint16 H0F_Rsvd11;                  	 //H0F_11
    Uint16 H0F_Rsvd12;                  	 //H0F_12
    Uint16 FC_ExInErrFilterTime;      	 //H0F_13 外内偏差一阶低通滤波时间
    Uint16 FC_Rsvd14;                    	 //H0F_14
    Uint16 FC_Rsvd15;                   	 //H0F_15
    Uint16 FC_ExInPosErrSum_Lo;    	 // H0F.16 全闭环外部位置误差低位(显示用)
    Uint16 FC_ExInPosErrSum_Hi;     	 // H0F.17 全闭环外部位置误差高位(显示用)
    Uint16 FC_InnerPulseFeedback_L;    //H0F_18  内部编码器反馈值
    Uint16 FC_InnerPulseFeedback_H;    //H0F_19
    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0F_10*/  1,              0,                  100,                 ANY_WRT|    DISP_3,
/*H0F_11*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_12*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_13*/  0,              0,                  65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H0F_14*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_15*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_16*/  0,              0,                  0,                  DISP_WRT|  TWO_WORD|  INT_SIGN| DISP_11,
/*H0F_17*/  0,              0xc000,             0x4000,             DISP_WRT|  TWO_WORD|  INT_SIGN| DISP_11| HIGH_WORD,
/*H0F_18*/  0,              0,                  0,                  DISP_WRT|  TWO_WORD|  INT_SIGN| DISP_11,
/*H0F_19*/  0,              0xc000,             0x4000,             DISP_WRT|  TWO_WORD|  INT_SIGN| DISP_11| HIGH_WORD,

/*
    Uint16 FC_ExPulseFeedback_L;       //H0F_20 外部编码器反馈值
    Uint16 FC_ExPulseFeedback_H;       //H0F_21
    Uint16 H0F_Rsvd22;                  //H0F_22
    Uint16 H0F_Rsvd23;                  //H0F_23
    Uint16 H0F_Rsvd24;                  //H0F_24
    Uint16 FC_TouchSet;                 //H0F_25 探针扩展设置
    Uint16 H0F_Rsvd26;                  //H0F_26
    Uint16 H0F_Rsvd27;                  //H0F_27
    Uint16 H0F_Rsvd28;                  //H0F_28
    Uint16 H0F_Rsvd29;                  //H0F_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0F_20*/  0,              0,                  0,                  DISP_WRT| TWO_WORD|INT_SIGN|DISP_11,
/*H0F_21*/  0,              0xc000,             0x4000,             DISP_WRT| TWO_WORD|INT_SIGN|DISP_11| HIGH_WORD,
/*H0F_22*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_23*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_24*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_25*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_26*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_27*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_28*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H0F_Rsvd30;                  //H0F_30
    Uint16 H0F_Rsvd31;                  //H0F_31
    Uint16 H0F_Rsvd32;                  //H0F_32
    Uint16 H0F_Rsvd33;                  //H0F_33
    Uint16 H0F_Rsvd34;                  //H0F_34
    Uint16 H0F_Rsvd35;                  //H0F_35
    Uint16 H0F_Rsvd36;                  //H0F_36
    Uint16 H0F_Rsvd37;                  //H0F_37
    Uint16 H0F_Rsvd38;                  //H0F_38
    Uint16 H0F_Rsvd39;                  //H0F_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0F_30*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_31*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_32*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_33*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_34*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_35*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0F_36*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_37*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_38*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H0F_Rsvd40;                  //H0F_40
    Uint16 H0F_Rsvd41;                  //H0F_41
    Uint16 H0F_Rsvd42;                  //H0F_42
    Uint16 H0F_Rsvd43;                  //H0F_43
    Uint16 H0F_Rsvd44;                  //H0F_44
    Uint16 H0F_Rsvd45;                  //H0F_45
    Uint16 H0F_Rsvd46;                  //H0F_46
    Uint16 H0F_Rsvd47;                  //H0F_47
    Uint16 H0F_Rsvd48;                  //H0F_48
    Uint16 MemCheck0F;                  //H0F_49     组校验字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0F_40*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_41*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_42*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_43*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_44*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_45*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_46*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_47*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_48*/  0,              0,                  65535,              RSVD_WRT,
/*H0F_49*/  0xFA5C,         0,                  65535,              RSVD_WRT,


//0828--------------预留参数组  H10LEN = 50 ------------------------------------------------------- 
/*  Uint16 SF_PID1RefSel;               //H10_00 PID1控制器指令源 index: 871
    Uint16 SF_PID1FdbSel;               //H10_01 PIDx反馈源
    Uint16 SF_PID1KeyRef;               //H10_02 PIDx键盘设定值
    Uint16 SF_PID1ActDir;               //H10_03 PIDx作用方向 0-正方向
    Uint16 SF_PID1FdbRang_Lo;           //H10_04 PIDx给定反馈量程 L
    Uint16 SF_PID1FdbRang_Hi;           //H10_05 PIDx给定反馈量程 H
    Uint16 SF_PID1EnSel_Rsvd;           //H10_06 PIDx使能开关  保留
    Uint16 SF_Rsvd07;                   //H10_07
    Uint16 SF_Rsvd08;                   //H10_08
    Uint16 SF_Rsvd09;                   //H10_09                           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H10_00*/  0,              0,                  3,                  RSVD_WRT|    DISP_1,
/*H10_01*/  3,              1,                  3,                  RSVD_WRT|    DISP_1,
/*H10_02*/  0,              (Uint16)-1000,      1000,               RSVD_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
/*H10_03*/  0,              0,                  1,                  RSVD_WRT|    DISP_1,
/*H10_04*/  1000,           0,                  0x0000,             RSVD_WRT|  TWO_WORD|   DISP_10,
/*H10_05*/  0,              0,                  0x4000,             RSVD_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H10_06*/  0,              0,                  1,                  RSVD_WRT|    DISP_1,
/*H10_07*/  0,              0,                  65535,              RSVD_WRT,
/*H10_08*/  0,              0,                  65535,              RSVD_WRT,
/*H10_09*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SF_PID1_Kp;                  //H10_10 PIDxProportional Gain 0.1HZ
    Uint16 SF_PID1_Ti;                  //H10_11 PIDx积分时间 ms
    Uint16 SF_PID1_Td;                  //H10_12 PIDx微分时间 ms
    Uint16 SF_PID1_MaxOut;              //H10_13
    Uint16 SF_Rsvd14;                   //H10_14
    Uint16 SF_PID1_InverFc;             //H10_15 PIDx反转截止频率
    Uint16 SF_PID1_ErrLmt;              //H10_16 PIDx偏差极限
    Uint16 SF_PID1_IntegLmt;            //H10_17 PIDx积分极限
    Uint16 SF_PID1_DiffLmt;             //H10_18 PIDx微分极限
    Uint16 SF_Rsvd19;                   //H10_19                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H10_10*/  100,            1,                  10000,              RSVD_WRT|    DISP_5|   TWO_DOT,
/*H10_11*/  2000,           1,                  10000,              RSVD_WRT|    DISP_5,
/*H10_12*/  0,              0,                  10000,              RSVD_WRT|    DISP_5,
/*H10_13*/  0,              0,                  65535,              RSVD_WRT,
/*H10_14*/  0,              0,                  65535,              RSVD_WRT,
/*H10_15*/  200,            0,                  65535,              RSVD_WRT|    DISP_5|   TWO_DOT,
/*H10_16*/  0,              0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_17*/  1000,           0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_18*/  50,             0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_19*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SF_PID2RefSel;               //H10_20 PID2控制器指令源 index: 871
    Uint16 SF_PID2FdbSel;               //H10_21 PIDx反馈源
    Uint16 SF_PID2KeyRef;               //H10_22 PIDx键盘设定值
    Uint16 SF_PID2ActDir;               //H10_23 PIDx作用方向 0-正方向
    Uint16 SF_PID2FdbRang_Lo;           //H10_24 PIDx给定反馈量程 L
    Uint16 SF_PID2FdbRang_Hi;           //H10_25 PIDx给定反馈量程 H
    Uint16 SF_PID2EnSel_Rsvd;           //H10_26 PIDx使能开关  保留
    Uint16 SF_Rsvd27;                   //H10_27
    Uint16 SF_Rsvd28;                   //H10_28
    Uint16 SF_Rsvd29;                   //H10_29                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H10_20*/  0,              0,                  3,                  RSVD_WRT|    DISP_1,
/*H10_21*/  2,              1,                  3,                  RSVD_WRT|    DISP_1,
/*H10_22*/  0,              (Uint16)-1000,      1000,               RSVD_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
/*H10_23*/  0,              0,                  1,                  RSVD_WRT|    DISP_1,
/*H10_24*/  1000,           0,                  0x0000,             RSVD_WRT|  TWO_WORD|   DISP_10,
/*H10_25*/  0,              0,                  0x4000,             RSVD_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H10_26*/  0,              0,                  1,                  RSVD_WRT|    DISP_1,
/*H10_27*/  0,              0,                  65535,              RSVD_WRT,
/*H10_28*/  0,              0,                  65535,              RSVD_WRT,
/*H10_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SF_PID2_Kp;                  //H10_30 PIDxProportional Gain
    Uint16 SF_PID2_Ti;                  //H10_31 PIDx积分时间
    Uint16 SF_PID2_Td;                  //H10_32 PIDx微分时间
    Uint16 SF_Rsvd33;                   //H10_33
    Uint16 SF_Rsvd34;                   //H10_34
    Uint16 SF_PID2_InverFc;             //H10_35 PIDx反转截止频率
    Uint16 SF_PID2_ErrLmt;              //H10_36 PIDx偏差极限
    Uint16 SF_PID2_IntegLmt;            //H10_37 PIDx积分极限
    Uint16 SF_PID2_DiffLmt;             //H10_38 PIDx微分极限
    Uint16 SF_Rsvd39;                   //H10_39                        */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H10_30*/  200,            1,                  10000,              RSVD_WRT|    DISP_5|   ONE_DOT,
/*H10_31*/  2000,           1,                  10000,              RSVD_WRT|    DISP_5,
/*H10_32*/  0,              0,                  10000,              RSVD_WRT|    DISP_5,
/*H10_33*/  0,              0,                  65535,              RSVD_WRT,
/*H10_34*/  0,              0,                  65535,              RSVD_WRT,
/*H10_35*/  200,            0,                  65535,              RSVD_WRT|    DISP_5|   TWO_DOT,
/*H10_36*/  0,              0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_37*/  1000,           0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_38*/  50,             0,                  1000,               RSVD_WRT|    DISP_4|   ONE_DOT,
/*H10_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 SF_Rsvd40;                   //H10_40
    Uint16 SF_Rsvd41;                   //H10_41
    Uint16 SF_Rsvd42;                   //H10_42
    Uint16 SF_Rsvd43;                   //H10_43
    Uint16 SF_Rsvd44;                   //H10_44
    Uint16 SF_Rsvd45;                   //H10_45
    Uint16 SF_Rsvd46;                   //H10_46
    Uint16 SF_Rsvd47;                   //H10_47
    Uint16 SF_Rsvd48;                   //H10_48
    Uint16 MemCheck10;                  //H10_49   组校验字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H10_40*/  0,              0,                  65535,              RSVD_WRT,
/*H10_41*/  0,              0,                  65535,              RSVD_WRT,
/*H10_42*/  0,              0,                  65535,              RSVD_WRT,
/*H10_43*/  0,              0,                  65535,              RSVD_WRT,
/*H10_44*/  0,              0,                  65535,              RSVD_WRT,
/*H10_45*/  0,              0,                  65535,              RSVD_WRT,
/*H10_46*/  0,              0,                  65535,              RSVD_WRT,
/*H10_47*/  0,              0,                  65535,              RSVD_WRT,
/*H10_48*/  0,              0,                  65535,              RSVD_WRT,
/*H10_49*/  0xC5A0,         0,                  65535,              RSVD_WRT,


//----------------------------多段位置参数  H11LEN = 100 ------------------------------------------------------- 
/*  Uint16 MP_PosRunMode;               //H11_00 多段位置运行方式
    Uint16 MP_PosExeSects;              //H11_01 位移执行段数选择 起始点为第一段
    Uint16 MP_RemdPosDealFlg;           //H11_02 余量处理方式
    Uint16 MP_PosTimeUnit;              //H11_03 等待时间单位
    Uint16 MP_PosRefType;               //H11_04 位移指令类型选择
    Uint16 MP_Mod4RecyStartSect;        //H11_05 循环模式起始段选择
    Uint16 MP_Rsvd06;                   //H11_06
    Uint16 MP_Rsvd07;                   //H11_07
    Uint16 MP_Rsvd08;                   //H11_08
    Uint16 MP_Rsvd09;                   //H11_09                        */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_00*/  1,              0,                  3,                  POSD_WRT|    DISP_1,
/*H11_01*/  1,              1,                  16,                 POSD_WRT|    DISP_2,
/*H11.02*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H11.03*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H11.04*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H11.05*/  0,              0,                  16,                 POSD_WRT|    DISP_2,
/*H11.06*/  0,              0,                  65535,              RSVD_WRT,
/*H11.07*/  0,              0,                  65535,              RSVD_WRT,
/*H11.08*/  0,              0,                  65535,              RSVD_WRT,
/*H11.09*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 MP_Rsvd10;                   //H11_10
    Uint16 MP_Rsvd11;                   //H11_11
    Uint16 MP_SEC1_PosRef_Lo;           //H11_12 第1段移动位移L
    Uint16 MP_SEC1_PosRef_Hi;           //H11_13 第1段移动位移H
    Uint16 MP_SEC1_MoveSpd;             //H11_14 第1段移动速度
    Uint16 MP_SEC1_AccTime;             //H11_15 第1段移动加减速时间
    Uint16 MP_SEC1_WaitTime;            //H11_16 第1段位移完成后等待时间
    Uint16 MP_SEC2_PosRef_Lo;           //H11_17 第2段移动位移L
    Uint16 MP_SEC2_PosRef_Hi;           //H11_18 第2段移动位移H
    Uint16 MP_SEC2_MoveSpd;             //H11_19 第2段移动速度                      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11.10*/  0,              0,                  65535,              RSVD_WRT,
/*H11.11*/  0,              0,                  65535,              RSVD_WRT,
/*H11_12*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_13*/  0,         0xC000,                  0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_14*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_15*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_16*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_17*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_18*/    0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_19*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC2_AccTime;             //H11_20 第2段移动加减速时间
    Uint16 MP_SEC2_WaitTime;            //H11_21 第2段位移完成后等待时间
    Uint16 MP_SEC3_PosRef_Lo;           //H11_22 第3段移动位移L
    Uint16 MP_SEC3_PosRef_Hi;           //H11_23 第3段移动位移H
    Uint16 MP_SEC3_MoveSpd;             //H11_24 第3段移动速度
    Uint16 MP_SEC3_AccTime;             //H11_25 第3段移动加减速时间
    Uint16 MP_SEC3_WaitTime;            //H11_26 第3段位移完成后等待时间
    Uint16 MP_SEC4_PosRef_Lo;           //H11_27 第4段移动位移L
    Uint16 MP_SEC4_PosRef_Hi;           //H11_28 第4段移动位移H
    Uint16 MP_SEC4_MoveSpd;             //H11_29 第4段移动速度                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_20*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_21*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_22*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_23*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_24*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_25*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_26*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_27*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_28*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_29*/  200,            1,                  6000,                ANY_WRT|    DISP_4,  

/*  Uint16 MP_SEC4_AccTime;             //H11_30 第4段移动加减速时间
    Uint16 MP_SEC4_WaitTime;            //H11_31 第4段位移完成后等待时间
    Uint16 MP_SEC5_PosRef_Lo;           //H11_32 第5段移动位移L
    Uint16 MP_SEC5_PosRef_Hi;           //H11_33 第5段移动位移H
    Uint16 MP_SEC5_MoveSpd;             //H11_34 第5段移动速度
    Uint16 MP_SEC5_AccTime;             //H11_35 第5段移动加减速时间
    Uint16 MP_SEC5_WaitTime;            //H11_36 第5段位移完成后等待时间
    Uint16 MP_SEC6_PosRef_Lo;           //H11_37 第6段移动位移L
    Uint16 MP_SEC6_PosRef_Hi;           //H11_38 第6段移动位移H
    Uint16 MP_SEC6_MoveSpd;             //H11_39 第6段移动速度                  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_30*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_31*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_32*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_33*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_34*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_35*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_36*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_37*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_38*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_39*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC6_AccTime;             //H11_40 第6段移动加减速时间
    Uint16 MP_SEC6_WaitTime;            //H11_41 第6段位移完成后等待时间
    Uint16 MP_SEC7_PosRef_Lo;           //H11_42 第7段移动位移L
    Uint16 MP_SEC7_PosRef_Hi;           //H11_43 第7段移动位移H
    Uint16 MP_SEC7_MoveSpd;             //H11_44 第7段移动速度
    Uint16 MP_SEC7_AccTime;             //H11_45 第7段移动加减速时间
    Uint16 MP_SEC7_WaitTime;            //H11_46 第7段位移完成后等待时间
    Uint16 MP_SEC8_PosRef_Lo;           //H11_47 第8段移动位移L
    Uint16 MP_SEC8_PosRef_Hi;           //H11_48 第8段移动位移H
    Uint16 MP_SEC8_MoveSpd;             //H11_49 第8段移动速度                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_40*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_41*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_42*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_43*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_44*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_45*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_46*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_47*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_48*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_49*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC8_AccTime;             //H11_50 第8段移动加减速时间
    Uint16 MP_SEC8_WaitTime;            //H11_51 第8段位移完成后等待时间
    Uint16 MP_SEC9_PosRef_Lo;           //H11_52 第9段移动位移L
    Uint16 MP_SEC9_PosRef_Hi;           //H11_53 第9段移动位移H
    Uint16 MP_SEC9_MoveSpd;             //H11_54 第9段移动速度
    Uint16 MP_SEC9_AccTime;             //H11_55 第9段移动加减速时间
    Uint16 MP_SEC9_WaitTime;            //H11_56 第9段位移完成后等待时间
    Uint16 MP_SEC10_PosRef_Lo;          //H11_57 第10段移动位移L
    Uint16 MP_SEC10_PosRef_Hi;          //H11_58 第10段移动位移H
    Uint16 MP_SEC10_MoveSpd;            //H11_59 第10段移动速度                     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_50*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_51*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_52*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_53*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_54*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_55*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_56*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_57*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_58*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_59*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC10_AccTime;            //H11_60 第10段移动加减速时间
    Uint16 MP_SEC10_WaitTime;           //H11_61 第10段位移完成后等待时间
    Uint16 MP_SEC11_PosRef_Lo;          //H11_62 第11段移动位移L
    Uint16 MP_SEC11_PosRef_Hi;          //H11_63 第11段移动位移H
    Uint16 MP_SEC11_MoveSpd;            //H11_64 第11段移动速度
    Uint16 MP_SEC11_AccTime;            //H11_65 第11段移动加减速时间
    Uint16 MP_SEC11_WaitTime;           //H11_66 第11段位移完成后等待时间
    Uint16 MP_SEC12_PosRef_Lo;          //H11_67 第12段移动位移L
    Uint16 MP_SEC12_PosRef_Hi;          //H11_68 第12段移动位移H
    Uint16 MP_SEC12_MoveSpd;            //H11_69 第12段移动速度                             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_60*/  10,             0,                  65535,               ANY_WRT|    DISP_6,
/*H11_61*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_62*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_63*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_64*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_65*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_66*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_67*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_68*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_69*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC12_AccTime;            //H11_70 第12段移动加减速时间
    Uint16 MP_SEC12_WaitTime;           //H11_71 第12段位移完成后等待时间
    Uint16 MP_SEC13_PosRef_Lo;          //H11_72 第13段移动位移L
    Uint16 MP_SEC13_PosRef_Hi;          //H11_73 第13段移动位移H
    Uint16 MP_SEC13_MoveSpd;            //H11_74 第13段移动速度
    Uint16 MP_SEC13_AccTime;            //H11_75 第13段移动加减速时间
    Uint16 MP_SEC13_WaitTime;           //H11_76 第13段位移完成后等待时间
    Uint16 MP_SEC14_PosRef_Lo;          //H11_77 第14段移动位移L
    Uint16 MP_SEC14_PosRef_Hi;          //H11_78 第14段移动位移H
    Uint16 MP_SEC14_MoveSpd;            //H11_79 第14段移动速度                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_70*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_71*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_72*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_73*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_74*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_75*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_76*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_77*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_78*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_79*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC14_AccTime;            //H11_80 第14段移动加减速时间
    Uint16 MP_SEC14_WaitTime;           //H11_81 第14段位移完成后等待时间
    Uint16 MP_SEC15_PosRef_Lo;          //H11_82 第15段移动位移L
    Uint16 MP_SEC15_PosRef_Hi;          //H11_83 第15段移动位移H
    Uint16 MP_SEC15_MoveSpd;            //H11_84 第15段移动速度
    Uint16 MP_SEC15_AccTime;            //H11_85 第15段移动加减速时间
    Uint16 MP_SEC15_WaitTime;           //H11_86 第15段位移完成后等待时间
    Uint16 MP_SEC16_PosRef_Lo;          //H11_87 第16段移动位移L
    Uint16 MP_SEC16_PosRef_Hi;          //H11_88 第16段移动位移H
    Uint16 MP_SEC16_MoveSpd;            //H11_89 第16段移动速度                 */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_80*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_81*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_82*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_83*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_84*/  200,            1,                  6000,                ANY_WRT|    DISP_4,
/*H11_85*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_86*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11_87*/ 10000,           0,                  0x0000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H11_88*/     0,           0xC000,             0x4000,              ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H11_89*/  200,            1,                  6000,                ANY_WRT|    DISP_4,

/*  Uint16 MP_SEC16_AccTime;            //H11_90 第16段移动加减速时间
    Uint16 MP_SEC16_WaitTime;           //H11_91 第16段位移完成后等待时间
    Uint16 MP_Rsvd92;                   //H11_92
    Uint16 MP_Rsvd93;                   //H11_93
    Uint16 MP_Rsvd94;                   //H11_94
    Uint16 MP_Rsvd95;                   //H11_95
    Uint16 MP_Rsvd96;                   //H11_96
    Uint16 MP_Rsvd97;                   //H11_97
    Uint16 MP_Rsvd98;                   //H11_98
    Uint16 MemCheck11;                  //H11_99 组校验字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H11_90*/  10,             0,                  65535,               ANY_WRT|    DISP_5,
/*H11_91*/  10,             0,                  10000,               ANY_WRT|    DISP_5,
/*H11.92*/  0,              0,                  65535,              RSVD_WRT,
/*H11.93*/  0,              0,                  65535,              RSVD_WRT,
/*H11.94*/  0,              0,                  65535,              RSVD_WRT,
/*H11.95*/  0,              0,                  65535,              RSVD_WRT,
/*H11.96*/  0,              0,                  65535,              RSVD_WRT,
/*H11.97*/  0,              0,                  65535,              RSVD_WRT,
/*H11.98*/  0,              0,                  65535,              RSVD_WRT,
/*H11.99*/  0xC5A1,         0,                  65535,              RSVD_WRT,   


//----------------------------多段指令(速度)参数  H12LEN = 68 ------------------------------------------------------- 
/*  Uint16 MTS_RefRunMode;              //H12_00 多段速度指令运行方式
    Uint16 MTS_ExeSects;                //H12_01 速度指令终点段数选择
    Uint16 MTS_RunTimeUnit;             //H12_02 运行时间单位选择
    Uint16 MTS_RiseTime1;               //H12_03 加速时间1
    Uint16 MTS_FallTime1;               //H12_04 减速时间1
    Uint16 MTS_RiseTime2;               //H12_05 加速时间2
    Uint16 MTS_FallTime2;               //H12_06 减速时间2
    Uint16 MTS_RiseTime3;               //H12_07 加速时间3
    Uint16 MTS_FallTime3;               //H12_08 减速时间3
    Uint16 MTS_RiseTime4;               //H12_09 加速时间4                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_00*/  1,              0,                  2,                  POSD_WRT|    DISP_1,
/*H12_01*/  16,             1,                  16,                 POSD_WRT|    DISP_2,
/*H12_02*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H12_03*/  10,             0,                  65535,              POSD_WRT|    DISP_5,
/*H12_04*/  10,             0,                  65535,              POSD_WRT|    DISP_5,
/*H12_05*/  50,             0,                  65535,              POSD_WRT|    DISP_5,
/*H12_06*/  50,             0,                  65535,              POSD_WRT|    DISP_5,
/*H12_07*/  100,            0,                  65535,              POSD_WRT|    DISP_5,
/*H12_08*/  100,            0,                  65535,              POSD_WRT|    DISP_5,
/*H12_09*/  150,            0,                  65535,              POSD_WRT|    DISP_5,

/*  Uint16 MTS_FallTime4;               //H12_10 减速时间4
    Uint16 MTS_Rsvd11;                  //H12_11
    Uint16 MTS_Rsvd12;                  //H12_12
    Uint16 MTS_Rsvd13;                  //H12_13
    Uint16 MTS_Rsvd14;                  //H12_14
    Uint16 MTS_Rsvd15;                  //H12_15
    Uint16 MTS_Rsvd16;                  //H12_16
    Uint16 MTS_Rsvd17;                  //H12_17
    Uint16 MTS_Rsvd18;                  //H12_18
    Uint16 MTS_Rsvd19;                  //H12_19                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_10*/  150,            0,                  65535,              POSD_WRT|    DISP_5,
/*H12_11*/  0,              0,                  65535,              RSVD_WRT,
/*H12_12*/  0,              0,                  65535,              RSVD_WRT,
/*H12_13*/  0,              0,                  5,                  RSVD_WRT,
/*H12_14*/  0,              0,                  65535,              RSVD_WRT,
/*H12_15*/  0,              0,                  65535,              RSVD_WRT,
/*H12_16*/  0,              0,                  65535,              RSVD_WRT,
/*H12_17*/  0,              0,                  65535,              RSVD_WRT,
/*H12_18*/  0,              0,                  65535,              RSVD_WRT,
/*H12_19*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 MTS_SEC1_Ref;                //H12_20 第1段指令
    Uint16 MTS_SEC1_RunTime;            //H12_21 第1段运行时间
    Uint16 MTS_SEC1_RiFaSel;            //H12_22 第1段升降速时间
    Uint16 MTS_SEC2_Ref;                //H12_23 第2段指令
    Uint16 MTS_SEC2_RunTime;            //H12_24 第2段运行时间
    Uint16 MTS_SEC2_RiFaSel;            //H12_25 第2段升降速时间
    Uint16 MTS_SEC3_Ref;                //H12_26 第3段指令
    Uint16 MTS_SEC3_RunTime;            //H12_27 第3段运行时间
    Uint16 MTS_SEC3_RiFaSel;            //H12_28 第3段升降速时间
    Uint16 MTS_SEC4_Ref;                //H12_29 第4段指令                              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_20*/  0,              (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_21*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_22*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_23*/  100,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_24*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_25*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_26*/  300,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_27*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_28*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_29*/  500,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,

/*  Uint16 MTS_SEC4_RunTime;            //H12_30 第4段运行时间
    Uint16 MTS_SEC4_RiFaSel;            //H12_31 第4段升降速时间
    Uint16 MTS_SEC5_Ref;                //H12_32 第5段指令
    Uint16 MTS_SEC5_RunTime;            //H12_33 第5段运行时间
    Uint16 MTS_SEC5_RiFaSel;            //H12_34 第5段升降速时间
    Uint16 MTS_SEC6_Ref;                //H12_35 第6段指令
    Uint16 MTS_SEC6_RunTime;            //H12_36 第6段运行时间
    Uint16 MTS_SEC6_RiFaSel;            //H12_37 第6段升降速时间
    Uint16 MTS_SEC7_Ref;                //H12_38 第7段指令
    Uint16 MTS_SEC7_RunTime;            //H12_39 第7段运行时间                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_30*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_31*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_32*/  700,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_33*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_34*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_35*/  900,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_36*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_37*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_38*/  600,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_39*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,

/*  Uint16 MTS_SEC7_RiFaSel;            //H12_40 第7段升降速时间
    Uint16 MTS_SEC8_Ref;                //H12_41 第8段指令
    Uint16 MTS_SEC8_RunTime;            //H12_42 第8段运行时间
    Uint16 MTS_SEC8_RiFaSel;            //H12_43 第8段升降速时间
    Uint16 MTS_SEC9_Ref;                //H12_44 第9段指令
    Uint16 MTS_SEC9_RunTime;            //H12_45 第9段运行时间
    Uint16 MTS_SEC9_RiFaSel;            //H12_46 第9段升降速时间
    Uint16 MTS_SEC10_Ref;               //H12_47 第10段指令
    Uint16 MTS_SEC10_RunTime;           //H12_48 第10段运行时间
    Uint16 MTS_SEC10_RiFaSel;           //H12_49 第10段升降速时间                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_40*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_41*/  300,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_42*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_43*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_44*/  100,            (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_45*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_46*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_47*/  (Uint16)-100,   (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_48*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_49*/  0,               0,                 4,                  POSD_WRT|    DISP_1,

/*  Uint16 MTS_SEC11_Ref;               //H12_50 第11段指令
    Uint16 MTS_SEC11_RunTime;           //H12_51 第11段运行时间
    Uint16 MTS_SEC11_RiFaSel;           //H12_52 第11段升降速时间
    Uint16 MTS_SEC12_Ref;               //H12_53 第12段指令
    Uint16 MTS_SEC12_RunTime;           //H12_54 第12段运行时间
    Uint16 MTS_SEC12_RiFaSel;           //H12_55 第12段升降速时间
    Uint16 MTS_SEC13_Ref;               //H12_56 第13段指令
    Uint16 MTS_SEC13_RunTime;           //H12_57 第13段运行时间
    Uint16 MTS_SEC13_RiFaSel;           //H12_58 第13段升降速时间
    Uint16 MTS_SEC14_Ref;               //H12_59 第14段指令                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_50*/  (Uint16)-300,   (Uint16)-6000,      6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_51*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_52*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_53*/  (Uint16)-500,    (Uint16)-6000,     6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_54*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_55*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_56*/  (Uint16)-700,    (Uint16)-6000,     6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_57*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_58*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_59*/  (Uint16)-900,    (Uint16)-6000,     6000,               POSD_WRT|    DISP_5|  INT_SIGN,

/*  Uint16 MTS_SEC14_RunTime;           //H12_60 第14段运行时间
    Uint16 MTS_SEC14_RiFaSel;           //H12_61 第14段升降速时间
    Uint16 MTS_SEC15_Ref;               //H12_62 第15段指令
    Uint16 MTS_SEC15_RunTime;           //H12_63 第15段运行时间
    Uint16 MTS_SEC15_RiFaSel;           //H12_64 第15段升降速时间
    Uint16 MTS_SEC16_Ref;               //H12_65 第16段指令
    Uint16 MTS_SEC16_RunTime;           //H12_66 第16段运行时间
    Uint16 MTS_SEC16_RiFaSel;           //H12_67 第16段升降速时间
    Uint16 MTS_Rsvd67;                  //H12_68
    Uint16 MemCheck12;                  //H12_69 组校验字                       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H12_60*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_61*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_62*/  (Uint16)-600,    (Uint16)-6000,      6000,              POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_63*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_64*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_65*/  (Uint16)-300,    (Uint16)-6000,     6000,               POSD_WRT|    DISP_5|  INT_SIGN,
/*H12_66*/  50,              0,                 65535,              POSD_WRT|    DISP_5|   ONE_DOT,
/*H12_67*/  0,               0,                 4,                  POSD_WRT|    DISP_1,
/*H12_68*/  0,               0,                 65535,              RSVD_WRT,
/*H12_69*/  0xC5A2,          0,                 65535,              RSVD_WRT,


//----------------------------(CANOPEN)预留参数组  H13LEN = 50 -------------------------------------------------------
/*  Uint16 H13_Rsvd00;                  //H13_00
    Uint16 TP_Tp1Type;                  //H13_01 Tpdo1类型
    Uint16 TP_Tp1MapObjCnt;             //H13_02 Tpdo1映射对象计数
    Uint16 TP_Tp1MapObj1Low16;          //H13_03 Tpdo1映射对象1低16位
    Uint16 TP_Tp1MapObj1Hi16;           //H13_04 Tpdo1映射对象1高16位
    Uint16 TP_Tp1MapObj2Low16;          //H13_05 Tpdo1映射对象2低16位
    Uint16 TP_Tp1MapObj2Hi16;           //H13_06 Tpdo1映射对象2低16位
    Uint16 TP_Tp1MapObj3Low16;          //H13_07 Tpdo1映射对象3低16位
    Uint16 TP_Tp1MapObj3Hi16;           //H13_08 Tpdo1映射对象3低16位
    Uint16 TP_Tp1MapObj4Low16;          //H13_09 Tpdo1映射对象4低16位          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H13_00*/ 0,               0,                 65535,              RSVD_WRT,
/*H13_01*/ 0,           0x0000,               0xFFFF,              RSVD_WRT,
/*H13_02*/ 0,           0x8000,               0x7FFF,              RSVD_WRT,
/*H13_03*/ 0,           0x0000,               0xFFFF,              RSVD_WRT,
/*H13_04*/ 0,           0x0000,               0xFFFF,              RSVD_WRT,
/*H13_05*/ 0,               0,                 65535,              RSVD_WRT,
/*H13_06*/ 0,               0,                 65535,              RSVD_WRT,
/*H13_07*/ 2,               0,                 65535,              RSVD_WRT,
/*H13_08*/ 0,               0,                 5000,               RSVD_WRT,
/*H13_09*/0,               0,                 65535,               RSVD_WRT,
/*  Uint16 TP_Tp1MapObj4Hi16;           //H13_10 Tpdo1映射对象4低16位
    Uint16 TP_Tp2Type;                  //H13_11 Tpdo2类型
    Uint16 TP_Tp2MapObjCnt;             //H13_12 Tpdo2映射对象计数
    Uint16 TP_Tp2MapObj1Low16;          //H13_13 Tpdo2映射对象1低16位
    Uint16 TP_Tp2MapObj1Hi16;           //H13_14 Tpdo2映射对象1高16位
    Uint16 TP_Tp2MapObj2Low16;          //H13_15 Tpdo2映射对象2低16位
    Uint16 TP_Tp2MapObj2Hi16;           //H13_16 Tpdo2映射对象2低16位
    Uint16 TP_Tp2MapObj3Low16;          //H13_17 Tpdo2映射对象3低16位
    Uint16 TP_Tp2MapObj3Hi16;           //H13_18 Tpdo2映射对象3低16位
    Uint16 TP_Tp2MapObj4Low16;          //H13_19 Tpdo2映射对象4低16位                 */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H13_10*/0,               0,                 65535,              RSVD_WRT,
/*H13_11*/0,               0,                 65535,              RSVD_WRT,
/*H13_12*/0,               0,                 65535,              RSVD_WRT,
/*H13_13*/0,               0,                 65535,              RSVD_WRT,
/*H13_14*/0,               0,                 65535,              RSVD_WRT,
/*H13_15*/0,               0,                 65535,              RSVD_WRT,
/*H13_16*/0,               0,                 65535,              RSVD_WRT,
/*H13_17*/0,               0,                 65535,              RSVD_WRT,
/*H13_18*/0,               0,                 65535,              RSVD_WRT,
/*H13_19*/0,               0,                 65535,              RSVD_WRT,
/*  Uint16 TP_Tp2MapObj4Hi16;           //H13_20 Tpdo2映射对象4低16位
    Uint16 TP_Tp3Type;                  //H13_21 Tpdo3类型
    Uint16 TP_Tp3MapObjCnt;             //H13_22 Tpdo3映射对象计数
    Uint16 TP_Tp3MapObj1Low16;          //H13_23 Tpdo3映射对象1低16位
    Uint16 TP_Tp3MapObj1Hi16;           //H13_24 Tpdo3映射对象1高16位
    Uint16 TP_Tp3MapObj2Low16;          //H13_25 Tpdo3映射对象2低16位
    Uint16 TP_Tp3MapObj2Hi16;           //H13_26 Tpdo3映射对象2低16位
    Uint16 TP_Tp3MapObj3Low16;          //H13_27 Tpdo3映射对象3低16位
    Uint16 TP_Tp3MapObj3Hi16;           //H13_28 Tpdo3映射对象3低16位
    Uint16 TP_Tp3MapObj4Low16;          //H13_29 Tpdo3映射对象4低16位               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H13_20*/0,               0,                 65535,              RSVD_WRT,
/*H13_21*/0,               0,                 65535,              RSVD_WRT,
/*H13_22*/0,               0,                 65535,              RSVD_WRT,
/*H13_23*/0,               0,                 65535,              RSVD_WRT,
/*H13_24*/0,               0,                 65535,              RSVD_WRT,
/*H13_25*/0,               0,                 65535,              RSVD_WRT,
/*H13_26*/0,               0,                 65535,              RSVD_WRT,
/*H13_27*/0,               0,                 65535,              RSVD_WRT,
/*H13_28*/0,               0,                 65535,              RSVD_WRT,
/*H13_29*/0,               0,                 65535,              RSVD_WRT,
/*  Uint16 TP_Tp3MapObj4Hi16;           //H13_30 Tpdo3映射对象4低16位
    Uint16 TP_Tp4Type;                  //H13_31 Tpdo4类型
    Uint16 TP_Tp4MapObjCnt;             //H13_32 Tpdo4映射对象计数
    Uint16 TP_Tp4MapObj1Low16;          //H13_33 Tpdo4映射对象1低16位
    Uint16 TP_Tp4MapObj1Hi16;           //H13_34 Tpdo4映射对象1高16位
    Uint16 TP_Tp4MapObj2Low16;          //H13_35 Tpdo4映射对象2低16位
    Uint16 TP_Tp4MapObj2Hi16;           //H13_36 Tpdo4映射对象2低16位
    Uint16 TP_Tp4MapObj3Low16;          //H13_37 Tpdo4映射对象3低16位
    Uint16 TP_Tp4MapObj3Hi16;           //H13_38 Tpdo4映射对象3低16位
    Uint16 TP_Tp4MapObj4Low16;          //H13_39 Tpdo4映射对象4低16位                  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H13_30*/0,               0,                 65535,              RSVD_WRT,
/*H13_31*/0,               0,                 65535,              RSVD_WRT,
/*H13_32*/0,               0,                 65535,              RSVD_WRT,
/*H13_33*/0,               0,                 65535,              RSVD_WRT,
/*H13_34*/0,               0,                 65535,              RSVD_WRT,
/*H13_35*/0,               0,                 65535,              RSVD_WRT,
/*H13_36*/0,               0,                 65535,              RSVD_WRT,
/*H13_37*/0,               0,                 65535,              RSVD_WRT,
/*H13_38*/0,               0,                 65535,              RSVD_WRT,
/*H13_39*/0,               0,                 65535,              RSVD_WRT,
/*  Uint16 TP_Tp4MapObj4Hi16;           //H13_40 Tpdo4映射对象4低16位
    Uint16 H13_Rsvd41;                  //H13_41
    Uint16 H13_Rsvd42;                  //H13_42
    Uint16 H13_Rsvd43;                  //H13_43
    Uint16 H13_Rsvd44;                  //H13_44
    Uint16 H13_Rsvd45;                  //H13_45
    Uint16 H13_Rsvd46;                  //H13_46
    Uint16 H13_Rsvd47;                  //H13_47
    Uint16 H13_Rsvd48;                  //H13_48
    Uint16 MemCheck13;                  //H13_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*0*/0,               0,                 65535,              RSVD_WRT,
/*H13_41*/  0,              0,                  65535,              RSVD_WRT,
/*H13_42*/  0,              0,                  65535,              RSVD_WRT,
/*H13_43*/  0,              0,                  65535,              RSVD_WRT,
/*H13_44*/  0,              0,                  65535,              RSVD_WRT,
/*H13_45*/  0,              0,                  65535,              RSVD_WRT,
/*H13_46*/  0,              0,                  65535,              RSVD_WRT,
/*H13_47*/  0,              0,                  65535,              RSVD_WRT,
/*H13_48*/  0,              0,                  65535,              RSVD_WRT,
/*H13_49*/  0xC5A3,         0,                  65535,              RSVD_WRT,


//----------------------------(CANOPEN)预留参数组  H14LEN = 50 -------------------------------------------------------
/*  Uint16 H14_Rsvd00;                  //H14_00
    Uint16 RP_Rp1Type;                  //H14_01 Rpdo1类型
    Uint16 RP_Rp1MapObjCnt;             //H14_02 Rpdo1映射对象计数
    Uint16 RP_Rp1MapObj1Low16;          //H14_03 Rpdo1映射对象1低16位
    Uint16 RP_Rp1MapObj1Hi16;           //H14_04 Rpdo1映射对象1高16位
    Uint16 RP_Rp1MapObj2Low16;          //H14_05 Rpdo1映射对象2低16位
    Uint16 RP_Rp1MapObj2Hi16;           //H14_06 Rpdo1映射对象2低16位
    Uint16 RP_Rp1MapObj3Low16;          //H14_07 Rpdo1映射对象3低16位
    Uint16 RP_Rp1MapObj3Hi16;           //H14_08 Rpdo1映射对象3低16位
    Uint16 RP_Rp1MapObj4Low16;          //H14_09 Rpdo1映射对象4低16位               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H14_00*/0,               0,                 65535,              RSVD_WRT,
/*H14_01*/0,               0,                 65535,              RSVD_WRT,
/*H14_02*/0,               0,                 65535,              RSVD_WRT,
/*H14_03*/0,               0,                 65535,              RSVD_WRT,
/*H14_04*/0,               0,                 65535,              RSVD_WRT,
/*H14_05*/0,               0,                 65535,              RSVD_WRT,
/*H14_06*/0,               0,                 65535,              RSVD_WRT,
/*H14_07*/0,               0,                 65535,              RSVD_WRT,
/*H14_08*/0,               0,                 65535,              RSVD_WRT,
/*H14_09*/0,               0,                 65535,              RSVD_WRT,

/*  Uint16 RP_Rp1MapObj4Hi16;           //H14_10 Rpdo1映射对象4低16位
    Uint16 RP_Rp2Type;                  //H14_11 Rpdo2类型
    Uint16 RP_Rp2MapObjCnt;             //H14_12 Rpdo2映射对象计数
    Uint16 RP_Rp2MapObj1Low16;          //H14_13 Rpdo2映射对象1低16位
    Uint16 RP_Rp2MapObj1Hi16;           //H14_14 Rpdo2映射对象1高16位
    Uint16 RP_Rp2MapObj2Low16;          //H14_15 Rpdo2映射对象2低16位
    Uint16 RP_Rp2MapObj2Hi16;           //H14_16 Rpdo2映射对象2低16位
    Uint16 RP_Rp2MapObj3Low16;          //H14_17 Rpdo2映射对象3低16位
    Uint16 RP_Rp2MapObj3Hi16;           //H14_18 Rpdo2映射对象3低16位
    Uint16 RP_Rp2MapObj4Low16;          //H14_19 Rpdo2映射对象4低16位                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H14_10*/0,               0,                 65535,              RSVD_WRT,
/*H14_11*/0,               0,                 65535,              RSVD_WRT,
/*H14_12*/0,               0,                 65535,              RSVD_WRT,
/*H14_13*/0,               0,                 65535,              RSVD_WRT,
/*H14_14*/0,               0,                 65535,              RSVD_WRT,
/*H14_15*/0,               0,                 65535,              RSVD_WRT,
/*H14_16*/0,               0,                 65535,              RSVD_WRT,
/*H14_17*/0,               0,                 65535,              RSVD_WRT,
/*H14_18*/0,               0,                 65535,              RSVD_WRT,
/*H14_19*/0,               0,                 65535,              RSVD_WRT,

/*  Uint16 RP_Rp2MapObj4Hi16;           //H14_20 Rpdo2映射对象4低16位
    Uint16 RP_Rp3Type;                  //H14_21 Rpdo3类型
    Uint16 RP_Rp3MapObjCnt;             //H14_22 Rpdo3映射对象计数
    Uint16 RP_Rp3MapObj1Low16;          //H14_23 Rpdo3映射对象1低16位
    Uint16 RP_Rp3MapObj1Hi16;           //H14_24 Rpdo3映射对象1高16位
    Uint16 RP_Rp3MapObj2Low16;          //H14_25 Rpdo3映射对象2低16位
    Uint16 RP_Rp3MapObj2Hi16;           //H14_26 Rpdo3映射对象2低16位
    Uint16 RP_Rp3MapObj3Low16;          //H14_27 Rpdo3映射对象3低16位
    Uint16 RP_Rp3MapObj3Hi16;           //H14_28 Rpdo3映射对象3低16位
    Uint16 RP_Rp3MapObj4Low16;          //H14_29 Rpdo3映射对象4低16位               */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H14_20*/0,               0,                 65535,              RSVD_WRT,
/*H14_21*/0,               0,                 65535,              RSVD_WRT,
/*H14_22*/0,               0,                 65535,              RSVD_WRT,
/*H14_23*/0,               0,                 65535,              RSVD_WRT,
/*H14_24*/0,               0,                 65535,              RSVD_WRT,
/*H14_25*/0,               0,                 65535,              RSVD_WRT,
/*H14_26*/0,               0,                 65535,              RSVD_WRT,
/*H14_27*/0,               0,                 65535,              RSVD_WRT,
/*H14_28*/0,               0,                 65535,              RSVD_WRT,
/*H14_29*/0,               0,                 65535,              RSVD_WRT,

/*  Uint16 RP_Rp3MapObj4Hi16;           //H14_30 Rpdo3映射对象4低16位
    Uint16 RP_Rp4Type;                  //H14_31 Rpdo4类型
    Uint16 RP_Rp4MapObjCnt;             //H14_32 Rpdo4映射对象计数
    Uint16 RP_Rp4MapObj1Low16;          //H14_33 Rpdo4映射对象1低16位
    Uint16 RP_Rp4MapObj1Hi16;           //H14_34 Rpdo4映射对象1高16位
    Uint16 RP_Rp4MapObj2Low16;          //H14_35 Rpdo4映射对象2低16位
    Uint16 RP_Rp4MapObj2Hi16;           //H14_36 Rpdo4映射对象2低16位
    Uint16 RP_Rp4MapObj3Low16;          //H14_37 Rpdo4映射对象3低16位
    Uint16 RP_Rp4MapObj3Hi16;           //H14_38 Rpdo4映射对象3低16位
    Uint16 RP_Rp4MapObj4Low16;          //H14_39 Rpdo4映射对象4低16位                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H14_30*/0,               0,                 65535,              RSVD_WRT,
/*H14_31*/0,               0,                 65535,              RSVD_WRT,
/*H14_32*/0,               0,                 65535,              RSVD_WRT,
/*H14_33*/0,               0,                 65535,              RSVD_WRT,
/*H14_34*/0,               0,                 65535,              RSVD_WRT,
/*H14_35*/0,               0,                 65535,              RSVD_WRT,
/*H14_36*/0,               0,                 65535,              RSVD_WRT,
/*H14_37*/0,               0,                 65535,              RSVD_WRT,
/*H14_38*/0,               0,                 65535,              RSVD_WRT,
/*H14_39*/0,               0,                 65535,              RSVD_WRT,

/*  Uint16 RP_Rp4MapObj4Hi16;           //H14_40 Rpdo4映射对象4低16位
    Uint16 H14_Rsvd41;                  //H14_41
    Uint16 H14_Rsvd42;                  //H14_42
    Uint16 H14_Rsvd43;                  //H14_43
    Uint16 H14_Rsvd44;                  //H14_44
    Uint16 H14_Rsvd45;                  //H14_45
    Uint16 H14_Rsvd46;                  //H14_46
    Uint16 H14_Rsvd47;                  //H14_47
    Uint16 H14_Rsvd48;                  //H14_48
    Uint16 MemCheck14;                  //H14_49           组校验字   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H14_40*/  0,               0,                 65535,              RSVD_WRT,
/*H14_41*/  0,              0,                  65535,              RSVD_WRT,
/*H14_42*/  0,              0,                  65535,              RSVD_WRT,
/*H14_43*/  0,              0,                  65535,              RSVD_WRT,
/*H14_44*/  0,              0,                  65535,              RSVD_WRT,
/*H14_45*/  0,              0,                  65535,              RSVD_WRT,
/*H14_46*/  0,              0,                  65535,              RSVD_WRT,
/*H14_47*/  0,              0,                  65535,              RSVD_WRT,
/*H14_48*/  0,              0,                  65535,              RSVD_WRT,
/*H14_49*/  0xCA54,         0,                  65535,              RSVD_WRT,


//----------------------------预留参数组  H15LEN = 50 -------------------------------------------------------
/*  Uint16 H15_Rsvd00;                  //H15_00
    Uint16 H15_Rsvd01;                  //H15_01
    Uint16 H15_Rsvd02;                  //H15_02
    Uint16 H15_Rsvd03;                  //H15_03
    Uint16 H15_Rsvd04;                  //H15_04
    Uint16 H15_Rsvd05;                  //H15_05
    Uint16 H15_Rsvd06;                  //H15_06
    Uint16 H15_Rsvd07;                  //H15_07
    Uint16 H15_Rsvd08;                  //H15_08
    Uint16 H15_Rsvd09;                  //H15_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H15_00*/  0,              0,                  65535,              RSVD_WRT,
/*H15_01*/  0,              0,                  65535,              RSVD_WRT,
/*H15_02*/  0,              0,                  65535,              RSVD_WRT,
/*H15_03*/  0,              0,                  65535,              RSVD_WRT,
/*H15_04*/  0,              0,                  65535,              RSVD_WRT,
/*H15_05*/  0,              0,                  65535,              RSVD_WRT,
/*H15_06*/  0,              0,                  65535,              RSVD_WRT,
/*H15_07*/  0,              0,                  65535,              RSVD_WRT,
/*H15_08*/  0,              0,                  65535,              RSVD_WRT,
/*H15_09*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H15_Rsvd10;                  //H15_10
    Uint16 H15_Rsvd11;                  //H15_11
    Uint16 H15_Rsvd12;                  //H15_12
    Uint16 H15_Rsvd13;                  //H15_13
    Uint16 H15_Rsvd14;                  //H15_14
    Uint16 H15_Rsvd15;                  //H15_15
    Uint16 H15_Rsvd16;                  //H15_16
    Uint16 H15_Rsvd17;                  //H15_17
    Uint16 H15_Rsvd18;                  //H15_18
    Uint16 H15_Rsvd19;                  //H15_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H15_10*/  0,              0,                  65535,              RSVD_WRT,
/*H15_11*/  0,              0,                  65535,              RSVD_WRT,
/*H15_12*/  0,              0,                  65535,              RSVD_WRT,
/*H15_13*/  0,              0,                  65535,              RSVD_WRT,
/*H15_14*/  0,              0,                  65535,              RSVD_WRT,
/*H15_15*/  0,              0,                  65535,              RSVD_WRT,
/*H15_16*/  0,              0,                  65535,              RSVD_WRT,
/*H15_17*/  0,              0,                  65535,              RSVD_WRT,
/*H15_18*/  0,              0,                  65535,              RSVD_WRT,
/*H15_19*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 H15_Rsvd20;                  //H15_20
    Uint16 H15_Rsvd21;                  //H15_21
    Uint16 H15_Rsvd22;                  //H15_22
    Uint16 H15_Rsvd23;                  //H15_23
    Uint16 H15_Rsvd24;                  //H15_24
    Uint16 H15_Rsvd25;                  //H15_25
    Uint16 H15_Rsvd26;                  //H15_26
    Uint16 H15_Rsvd27;                  //H15_27
    Uint16 H15_Rsvd28;                  //H15_28
    Uint16 H15_Rsvd29;                  //H15_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H15_20*/  0,              0,                  65535,              RSVD_WRT,
/*H15_21*/  0,              0,                  65535,              RSVD_WRT,
/*H15_22*/  0,              0,                  65535,              RSVD_WRT,
/*H15_23*/  0,              0,                  65535,              RSVD_WRT,
/*H15_24*/  0,              0,                  65535,              RSVD_WRT,
/*H15_25*/  0,              0,                  65535,              RSVD_WRT,
/*H15_26*/  0,              0,                  65535,              RSVD_WRT,
/*H15_27*/  0,              0,                  65535,              RSVD_WRT,
/*H15_28*/  0,              0,                  65535,              RSVD_WRT,
/*H15_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H15_Rsvd30;                  //H15_30
    Uint16 H15_Rsvd31;                  //H15_31
    Uint16 H15_Rsvd32;                  //H15_32
    Uint16 H15_Rsvd33;                  //H15_33
    Uint16 H15_Rsvd34;                  //H15_34
    Uint16 H15_Rsvd35;                  //H15_35
    Uint16 H15_Rsvd36;                  //H15_36
    Uint16 H15_Rsvd37;                  //H15_37
    Uint16 H15_Rsvd38;                  //H15_38
    Uint16 H15_Rsvd39;                  //H15_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H15_30*/  0,              0,                  65535,              RSVD_WRT,
/*H15_31*/  0,              0,                  65535,              RSVD_WRT,
/*H15_32*/  0,              0,                  65535,              RSVD_WRT,
/*H15_33*/  0,              0,                  65535,              RSVD_WRT,
/*H15_34*/  0,              0,                  65535,              RSVD_WRT,
/*H15_35*/  0,              0,                  65535,              RSVD_WRT,
/*H15_36*/  0,              0,                  65535,              RSVD_WRT,
/*H15_37*/  0,              0,                  65535,              RSVD_WRT,
/*H15_38*/  0,              0,                  65535,              RSVD_WRT,
/*H15_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H15_Rsvd40;                  //H15_40
    Uint16 H15_Rsvd41;                  //H15_41
    Uint16 H15_Rsvd42;                  //H15_42
    Uint16 H15_Rsvd43;                  //H15_43
    Uint16 H15_Rsvd44;                  //H15_44
    Uint16 H15_Rsvd45;                  //H15_45
    Uint16 H15_Rsvd46;                  //H15_46
    Uint16 H15_Rsvd47;                  //H15_47
    Uint16 H15_Rsvd48;                  //H15_48
    Uint16 MemCheck15;                  //H15_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H15_40*/  0,              0,                  65535,              RSVD_WRT,
/*H15_41*/  0,              0,                  65535,              RSVD_WRT,
/*H15_42*/  0,              0,                  65535,              RSVD_WRT,
/*H15_43*/  0,              0,                  65535,              RSVD_WRT,
/*H15_44*/  0,              0,                  65535,              RSVD_WRT,
/*H15_45*/  0,              0,                  65535,              RSVD_WRT,
/*H15_46*/  0,              0,                  65535,              RSVD_WRT,
/*H15_47*/  0,              0,                  65535,              RSVD_WRT,
/*H15_48*/  0,              0,                  65535,              RSVD_WRT,
/*H15_49*/  0xC5A5,         0,                  65535,              RSVD_WRT,


//----------------------------(CANLINK)预留参数组  H16LEN = 50 -------------------------------------------------------
/*  Uint16 H16_Rsvd00;                  //H16_00
    Uint16 CG_CfgInfo1_1;               //H16_01 配置信息1的第1个字
    Uint16 CG_CfgInfo1_2;               //H16_02 配置信息1的第2个字
    Uint16 CG_CfgInfo1_3;               //H16_03 配置信息1的第3个字
    Uint16 CG_CfgInfo1_4;               //H16_04 配置信息1的第4个字
    Uint16 CG_CfgInfo2_1;               //H16_05 配置信息2的第1个字
    Uint16 CG_CfgInfo2_2;               //H16_06 配置信息2的第2个字
    Uint16 CG_CfgInfo2_3;               //H16_07 配置信息2的第3个字
    Uint16 CG_CfgInfo2_4;               //H16_08 配置信息2的第4个字
    Uint16 CG_CfgInfo3_1;               //H16_09 配置信息3的第1个字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H16_00*/  0,              0,                  65535,              RSVD_WRT,
/*H16_01*/  0,              0,                  65535,              RSVD_WRT,
/*H16_02*/  0,              0,                  65535,              RSVD_WRT,
/*H16_03*/  0,              0,                  65535,              RSVD_WRT,
/*H16_04*/  0,              0,                  65535,              RSVD_WRT,
/*H16_05*/  0,              0,                  65535,              RSVD_WRT,
/*H16_06*/  0,              0,                  65535,              RSVD_WRT,
/*H16_07*/  0,              0,                  65535,              RSVD_WRT,
/*H16_08*/  0,              0,                  65535,              RSVD_WRT,
/*H16_09*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 CG_CfgInfo3_2;               //H16_10 配置信息3的第2个字
    Uint16 CG_CfgInfo3_3;               //H16_11 配置信息3的第3个字
    Uint16 CG_CfgInfo3_4;               //H16_12 配置信息3的第4个字
    Uint16 CG_CfgInfo4_1;               //H16_13 配置信息4的第1个字
    Uint16 CG_CfgInfo4_2;               //H16_14 配置信息4的第2个字
    Uint16 CG_CfgInfo4_3;               //H16_15 配置信息4的第3个字
    Uint16 CG_CfgInfo4_4;               //H16_16 配置信息4的第4个字
    Uint16 CG_CfgInfo5_1;               //H16_17 配置信息5的第1个字
    Uint16 CG_CfgInfo5_2;               //H16_18 配置信息5的第2个字
    Uint16 CG_CfgInfo5_3;               //H16_19 配置信息5的第3个字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H16_10*/  0,              0,                  65535,              RSVD_WRT,
/*H16_11*/  0,              0,                  65535,              RSVD_WRT,
/*H16_12*/  0,              0,                  65535,              RSVD_WRT,
/*H16_13*/  0,              0,                  65535,              RSVD_WRT,
/*H16_14*/  0,              0,                  65535,              RSVD_WRT,
/*H16_15*/  0,              0,                  65535,              RSVD_WRT,
/*H16_16*/  0,              0,                  65535,              RSVD_WRT,
/*H16_17*/  0,              0,                  65535,              RSVD_WRT,
/*H16_18*/  0,              0,                  65535,              RSVD_WRT,
/*H16_19*/  0,              0,                  65535,              RSVD_WRT, 

/*  Uint16 CG_CfgInfo5_4;               //H16_20 配置信息5的第4个字
    Uint16 CG_CfgInfo6_1;               //H16_21 配置信息6的第1个字
    Uint16 CG_CfgInfo6_2;               //H16_22 配置信息6的第2个字
    Uint16 CG_CfgInfo6_3;               //H16_23 配置信息6的第3个字
    Uint16 CG_CfgInfo6_4;               //H16_24 配置信息6的第4个字
    Uint16 CG_CfgInfo7_1;               //H16_25 配置信息7的第1个字
    Uint16 CG_CfgInfo7_2;               //H16_26 配置信息7的第2个字
    Uint16 CG_CfgInfo7_3;               //H16_27 配置信息7的第3个字
    Uint16 CG_CfgInfo7_4;               //H16_28 配置信息7的第4个字
    Uint16 CG_CfgInfo8_1;               //H16_29 配置信息8的第1个字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H16_20*/  0,              0,                  65535,              RSVD_WRT,
/*H16_21*/  0,              0,                  65535,              RSVD_WRT,
/*H16_22*/  0,              0,                  65535,              RSVD_WRT,
/*H16_23*/  0,              0,                  65535,              RSVD_WRT,
/*H16_24*/  0,              0,                  65535,              RSVD_WRT,
/*H16_25*/  0,              0,                  65535,              RSVD_WRT,
/*H16_26*/  0,              0,                  65535,              RSVD_WRT,
/*H16_27*/  0,              0,                  65535,              RSVD_WRT,
/*H16_28*/  0,              0,                  65535,              RSVD_WRT,
/*H16_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 CG_CfgInfo8_2;               //H16_30 配置信息8的第2个字
    Uint16 CG_CfgInfo8_3;               //H16_31 配置信息8的第3个字
    Uint16 CG_CfgInfo8_4;               //H16_32 配置信息8的第4个字
    Uint16 CG_CfgInfo9_1;               //H16_33 配置信息9的第1个字
    Uint16 CG_CfgInfo9_2;               //H16_34 配置信息9的第2个字
    Uint16 CG_CfgInfo9_3;               //H16_35 配置信息9的第3个字
    Uint16 CG_CfgInfo9_4;               //H16_36 配置信息9的第4个字
    Uint16 CG_CfgInfo10_1;              //H16_37 配置信息10的第1个字
    Uint16 CG_CfgInfo10_2;              //H16_38 配置信息10的第2个字
    Uint16 CG_CfgInfo10_3;              //H16_39 配置信息10的第3个字           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H16_30*/  0,              0,                  65535,              RSVD_WRT,
/*H16_31*/  0,              0,                  65535,              RSVD_WRT,
/*H16_32*/  0,              0,                  65535,              RSVD_WRT,
/*H16_33*/  0,              0,                  65535,              RSVD_WRT,
/*H16_34*/  0,              0,                  65535,              RSVD_WRT,
/*H16_35*/  0,              0,                  65535,              RSVD_WRT,
/*H16_36*/  0,              0,                  65535,              RSVD_WRT,
/*H16_37*/  0,              0,                  65535,              RSVD_WRT,
/*H16_38*/  0,              0,                  65535,              RSVD_WRT,
/*H16_39*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 CG_CfgInfo10_4;              //H16_40 配置信息10的第4个字
    Uint16 CG_DelCanCfgInfo;            //H16_41 删除CAN配置信息
    Uint16 CG_CanCfgNum;                //H16_42 CANLINK配置信息个数
    Uint16 H16_Rsvd43;                  //H16_43
    Uint16 H16_Rsvd44;                  //H16_44
    Uint16 H16_Rsvd45;                  //H16_45
    Uint16 H16_Rsvd46;                  //H16_46
    Uint16 H16_Rsvd47;                  //H16_47
    Uint16 H16_Rsvd48;                  //H16_48
    Uint16 MemCheck16;                  //H16_49               组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H16_40*/  0,              0,                  65535,              RSVD_WRT,
/*H16_41*/  0,              0,                  65535,              RSVD_WRT,
/*H16_42*/  0,              0,                  65535,              RSVD_WRT,
/*H16_43*/  0,              0,                  65535,              RSVD_WRT,
/*H16_44*/  0,              0,                  65535,              RSVD_WRT,
/*H16_45*/  0,              0,                  65535,              RSVD_WRT,
/*H16_46*/  0,              0,                  65535,              RSVD_WRT,
/*H16_47*/  0,              0,                  65535,              RSVD_WRT,
/*H16_48*/  0,              0,                  65535,              RSVD_WRT,
/*H16_49*/  0xC5A6,         0,                  65535,              RSVD_WRT,


//----------------------------VDI/VDO参数  H17LEN = 70 -------------------------------------------------------
 
/*  Uint16 VI_VDIFuncSel1;              //H17_00 VDI1端子功能选择
    Uint16 VI_VDILogicSel1;             //H17_01 VDI1端子逻辑选择
    Uint16 VI_VDIFuncSel2;              //H17_02 VDI2端子功能选择
    Uint16 VI_VDILogicSel2;             //H17_03 VDI2端子逻辑选择
    Uint16 VI_VDIFuncSel3;              //H17_04 VDI3端子功能选择
    Uint16 VI_VDILogicSel3;             //H17_05 VDI3端子逻辑选择
    Uint16 VI_VDIFuncSel4;              //H17_06 VDI4端子功能选择
    Uint16 VI_VDILogicSel4;             //H17_07 VDI4端子逻辑选择
    Uint16 VI_VDIFuncSel5;              //H17_08 VDI5端子功能选择
    Uint16 VI_VDILogicSel5;             //H17_09 VDI5端子逻辑选择             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_00*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_01*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_02*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_03*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_04*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_05*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_06*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_07*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_08*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_09*/  0,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 VI_VDIFuncSel6;              //H17_10 VDI6端子功能选择
    Uint16 VI_VDILogicSel6;             //H17_11 VDI6端子逻辑选择
    Uint16 VI_VDIFuncSel7;              //H17_12 VDI7端子功能选择
    Uint16 VI_VDILogicSel7;             //H17_13 VDI7端子逻辑选择
    Uint16 VI_VDIFuncSel8;              //H17_14 VDI8端子功能选择
    Uint16 VI_VDILogicSel8;             //H17_15 VDI8端子逻辑选择
    Uint16 VI_VDIFuncSel9;              //H17_16 VDI9端子功能选择
    Uint16 VI_VDILogicSel9;             //H17_17 VDI9端子逻辑选择
    Uint16 VI_VDIFuncSel10;             //H17_18 VDI10端子功能选择
    Uint16 VI_VDILogicSel10;            //H17_19 VDI10端子逻辑选择                              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_10*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_11*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_12*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_13*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_14*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_15*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_16*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_17*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_18*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_19*/  0,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 VI_VDIFuncSel11;             //H17_20 VDI11端子功能选择
    Uint16 VI_VDILogicSel11;            //H17_21 VDI11端子逻辑选择
    Uint16 VI_VDIFuncSel12;             //H17_22 VDI12端子功能选择
    Uint16 VI_VDILogicSel12;            //H17_23 VDI12端子逻辑选择
    Uint16 VI_VDIFuncSel13;             //H17_24 VDI13端子功能选择
    Uint16 VI_VDILogicSel13;            //H17_25 VDI13端子逻辑选择
    Uint16 VI_VDIFuncSel14;             //H17_26 VDI14端子功能选择
    Uint16 VI_VDILogicSel14;            //H17_27 VDI14端子逻辑选择
    Uint16 VI_VDIFuncSel15;             //H17_28 VDI15端子功能选择
    Uint16 VI_VDILogicSel15;            //H17_29 VDI15端子逻辑选择                  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_20*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_21*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_22*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_23*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_24*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_25*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_26*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_27*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_28*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_29*/  0,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 VI_VDIFuncSel16;             //H17_30 VDI16端子功能选择
    Uint16 VI_VDILogicSel16;            //H17_31 VDI16端子逻辑选择
    Uint16 VI_AllVDOLevel;              //H17_32 VDO虚拟电平
    Uint16 VI_VDOFuncSel1;              //H17_33 VDO1端子功能选择
    Uint16 VI_VDOLogicSel1;             //H17_34 VDO1端子逻辑选择
    Uint16 VI_VDOFuncSel2;              //H17_35 VDO2端子功能选择
    Uint16 VI_VDOLogicSel2;             //H17_36 VDO2端子逻辑选择
    Uint16 VI_VDOFuncSel3;              //H17_37 VDO3端子功能选择
    Uint16 VI_VDOLogicSel3;             //H17_38 VDO3端子逻辑选择
    Uint16 VI_VDOFuncSel4;              //H17_39 VDO4端子功能选择                       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_30*/  0,              0,                  DI_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_31*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_32*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_8,
/*H17_33*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_34*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_35*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_36*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_37*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_38*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_39*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,

/*  Uint16 VI_VDOLogicSel4;             //H17_40 VDO4端子逻辑选择
    Uint16 VI_VDOFuncSel5;              //H17_41 VDO5端子功能选择
    Uint16 VI_VDOLogicSel5;             //H17_42 VDO5端子逻辑选择
    Uint16 VI_VDOFuncSel6;              //H17_43 VDO6端子功能选择
    Uint16 VI_VDOLogicSel6;             //H17_44 VDO6端子逻辑选择
    Uint16 VI_VDOFuncSel7;              //H17_45 VDO7端子功能选择
    Uint16 VI_VDOLogicSel7;             //H17_46 VDO7端子逻辑选择
    Uint16 VI_VDOFuncSel8;              //H17_47 VDO8端子功能选择
    Uint16 VI_VDOLogicSel8;             //H17_48 VDO8端子逻辑选择
    Uint16 VI_VDOFuncSel9;              //H17_49 VDO9端子功能选择                       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_40*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_41*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_42*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_43*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_44*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_45*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_46*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_47*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_48*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_49*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,

/*  Uint16 VI_VDOLogicSel9;             //H17_50 VDO9端子逻辑选择
    Uint16 VI_VDOFuncSel10;             //H17_51 VDO10端子功能选择
    Uint16 VI_VDOLogicSel10;            //H17_52 VDO10端子逻辑选择
    Uint16 VI_VDOFuncSel11;             //H17_53 VDO11端子功能选择
    Uint16 VI_VDOLogicSel11;            //H17_54 VDO11端子逻辑选择
    Uint16 VI_VDOFuncSel12;             //H17_55 VDO12端子功能选择
    Uint16 VI_VDOLogicSel12;            //H17_56 VDO12端子逻辑选择
    Uint16 VI_VDOFuncSel13;             //H17_57 VDO13端子功能选择
    Uint16 VI_VDOLogicSel13;            //H17_58 VDO13端子逻辑选择
    Uint16 VI_VDOFuncSel14;             //H17_59 VDO14端子功能选择                      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_50*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_51*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_52*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_53*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_54*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_55*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_56*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_57*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_58*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_59*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,

/*  Uint16 VI_VDOLogicSel14;            //H17_60 VDO14端子逻辑选择
    Uint16 VI_VDOFuncSel15;             //H17_61 VDO15端子功能选择
    Uint16 VI_VDOLogicSel15;            //H17_62 VDO15端子逻辑选择
    Uint16 VI_VDOFuncSel16;             //H17_63 VDO16端子功能选择
    Uint16 VI_VDOLogicSel16;            //H17_64 VDO16端子逻辑选择
    Uint16 H17_Rsvd65;                  //H17_65
    Uint16 H17_Rsvd66;                  //H17_66
    Uint16 H17_Rsvd67;                  //H17_67
    Uint16 H17_Rsvd68;                  //H17_68
    Uint16 MemCheck17;                  //H17_69                    组校验字                    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H17_60*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_61*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_62*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_63*/  0,              0,                  DO_FUNC_SEL_MAX,     ANY_WRT|    DISP_2,
/*H17_64*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H17_65*/  0,              0,                  65535,              RSVD_WRT,
/*H17_66*/  0,              0,                  65535,              RSVD_WRT,
/*H17_67*/  0,              0,                  65535,              RSVD_WRT,
/*H17_68*/  0,              0,                  65535,              RSVD_WRT,
/*H17_69*/  0xC5A7,         0,                  65535,              RSVD_WRT,


//----------------------------预留参数组  H18LEN = 50 -------------------------------------------------------
/*  Uint16 H18_Rsvd00;                  //H18_00
    Uint16 H18_Rsvd01;                  //H18_01
    Uint16 H18_Rsvd02;                  //H18_02
    Uint16 H18_Rsvd03;                  //H18_03
    Uint16 H18_Rsvd04;                  //H18_04
    Uint16 H18_Rsvd05;                  //H18_05
    Uint16 H18_Rsvd06;                  //H18_06
    Uint16 H18_Rsvd07;                  //H18_07
    Uint16 H18_Rsvd08;                  //H18_08
    Uint16 H18_Rsvd09;                  //H18_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H18_00*/  0x0080,         0x0080,             0x0080,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_01*/  0x0000,         0x0000,             0x4000,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_02*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10,
/*H18_03*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD,
/*H18_04*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H18_05*/  0,              0,                  255,                POSD_WRT|DISP_3,
/*H18_06*/  0,              0,                  0x00FF,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_07*/  0,              0,                  0x8000,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_08*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_09*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H18_00*/0,               0,                 65535,              RSVD_WRT,
/*H18_01*/0,               0,                 65535,              RSVD_WRT,
/*H18_02*/0,               0,                 65535,              RSVD_WRT,
/*H18_03*/0,               0,                 65535,              RSVD_WRT,
/*H18_04*/0,               0,                 65535,              RSVD_WRT,
/*H18_05*/0,               0,                 65535,              RSVD_WRT,
/*H18_06*/0,               0,                 65535,              RSVD_WRT,
/*H18_07*/0,               0,                 65535,              RSVD_WRT,
/*H18_08*/0,               0,                 65535,              RSVD_WRT,
/*H18_09*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H18_Rsvd10;                  //H18_10
    Uint16 H18_Rsvd11;                  //H18_11
    Uint16 H18_Rsvd12;                  //H18_12
    Uint16 H18_Rsvd13;                  //H18_13
    Uint16 H18_Rsvd14;                  //H18_14
    Uint16 H18_Rsvd15;                  //H18_15
    Uint16 H18_Rsvd16;                  //H18_16
    Uint16 H18_Rsvd17;                  //H18_17
    Uint16 H18_Rsvd18;                  //H18_18
    Uint16 H18_Rsvd19;                  //H18_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H18_10*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_11*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_12*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_13*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_14*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_15*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_16*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_17*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_18*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H18_19*/  0,              0,                  255,                POSD_WRT|DISP_3, 
#else
/*H18_10*/0,               0,                 65535,              RSVD_WRT,
/*H18_11*/0,               0,                 65535,              RSVD_WRT,
/*H18_12*/0,               0,                 65535,              RSVD_WRT,
/*H18_13*/0,               0,                 65535,              RSVD_WRT,
/*H18_14*/0,               0,                 65535,              RSVD_WRT,
/*H18_15*/0,               0,                 65535,              RSVD_WRT,
/*H18_16*/0,               0,                 65535,              RSVD_WRT,
/*H18_17*/0,               0,                 65535,              RSVD_WRT,
/*H18_18*/0,               0,                 65535,              RSVD_WRT,
/*H18_19*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H18_Rsvd20;                  //H18_20
    Uint16 H18_Rsvd21;                  //H18_21
    Uint16 H18_Rsvd22;                  //H18_22
    Uint16 H18_Rsvd23;                  //H18_23
    Uint16 H18_Rsvd24;                  //H18_24
    Uint16 H18_Rsvd25;                  //H18_25
    Uint16 H18_Rsvd26;                  //H18_26
    Uint16 H18_Rsvd27;                  //H18_27
    Uint16 H18_Rsvd28;                  //H18_28
    Uint16 H18_Rsvd29;                  //H18_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H18_20*/  0x0200,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_21*/  0x0000,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_22*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H18_23*/  0x0300,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_24*/  0x8000,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_25*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H18_26*/  0x0400,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_27*/  0x8000,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_28*/  252,            0,                  255,                POSD_WRT|DISP_3,
/*H18_29*/  0x0500,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H18_20*/0,               0,                 65535,              RSVD_WRT,
/*H18_21*/0,               0,                 65535,              RSVD_WRT,
/*H18_22*/0,               0,                 65535,              RSVD_WRT,
/*H18_23*/0,               0,                 65535,              RSVD_WRT,
/*H18_24*/0,               0,                 65535,              RSVD_WRT,
/*H18_25*/0,               0,                 65535,              RSVD_WRT,
/*H18_26*/0,               0,                 65535,              RSVD_WRT,
/*H18_27*/0,               0,                 65535,              RSVD_WRT,
/*H18_28*/0,               0,                 65535,              RSVD_WRT,
/*H18_29*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H18_Rsvd30;                  //H18_30
    Uint16 H18_Rsvd31;                  //H18_31
    Uint16 H18_Rsvd32;                  //H18_32
    Uint16 H18_Rsvd33;                  //H18_33
    Uint16 H18_Rsvd34;                  //H18_34
    Uint16 H18_Rsvd35;                  //H18_35
    Uint16 H18_Rsvd36;                  //H18_36
    Uint16 H18_Rsvd37;                  //H18_37
    Uint16 H18_Rsvd38;                  //H18_38
    Uint16 H18_Rsvd39;                  //H18_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H18_30*/  0x8000,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_31*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H18_32*/  0,              0,                  65535,              RSVD_WRT,
/*H18_33*/  0,              0,                  65535,              RSVD_WRT,
/*H18_34*/  1,              0,                  8,                  POSD_WRT|DISP_1,
/*H18_35*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_36*/  0x6040,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_37*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_38*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_39*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H18_30*/0,               0,                 65535,              RSVD_WRT,
/*H18_31*/0,               0,                 65535,              RSVD_WRT,
/*H18_32*/0,               0,                 65535,              RSVD_WRT,
/*H18_33*/0,               0,                 65535,              RSVD_WRT,
/*H18_34*/0,               0,                 65535,              RSVD_WRT,
/*H18_35*/0,               0,                 65535,              RSVD_WRT,
/*H18_36*/0,               0,                 65535,              RSVD_WRT,
/*H18_37*/0,               0,                 65535,              RSVD_WRT,
/*H18_38*/0,               0,                 65535,              RSVD_WRT,
/*H18_39*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H18_Rsvd40;                  //H18_40
    Uint16 H18_Rsvd41;                  //H18_41
    Uint16 H18_Rsvd42;                  //H18_42
    Uint16 H18_Rsvd43;                  //H18_43
    Uint16 H18_Rsvd44;                  //H18_44
    Uint16 H18_Rsvd45;                  //H18_45
    Uint16 H18_Rsvd46;                  //H18_46
    Uint16 H18_Rsvd47;                  //H18_47
    Uint16 H18_Rsvd48;                  //H18_48
    Uint16 MemCheck18;                  //H18_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H18_40*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_41*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_42*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_43*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_44*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_45*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_46*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H18_47*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H18_48*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H18_40*/0,               0,                 65535,              RSVD_WRT,
/*H18_41*/0,               0,                 65535,              RSVD_WRT,
/*H18_42*/0,               0,                 65535,              RSVD_WRT,
/*H18_43*/0,               0,                 65535,              RSVD_WRT,
/*H18_44*/0,               0,                 65535,              RSVD_WRT,
/*H18_45*/0,               0,                 65535,              RSVD_WRT,
/*H18_46*/0,               0,                 65535,              RSVD_WRT,
/*H18_47*/0,               0,                 65535,              RSVD_WRT,
/*H18_48*/0,               0,                 65535,              RSVD_WRT,
#endif
/*H18_49*/  0xC5A8,         0,                  65535,              RSVD_WRT,
//----------------------------预留参数组  H19LEN = 50 -------------------------------------------------------
/*  Uint16 H19_Rsvd00;                  //H19_00
    Uint16 H19_Rsvd01;                  //H19_01
    Uint16 H19_Rsvd02;                  //H19_02
    Uint16 H19_Rsvd03;                  //H19_03
    Uint16 H19_Rsvd04;                  //H19_04
    Uint16 H19_Rsvd05;                  //H19_05
    Uint16 H19_Rsvd06;                  //H19_06
    Uint16 H19_Rsvd07;                  //H19_07
    Uint16 H19_Rsvd08;                  //H19_08
    Uint16 H19_Rsvd09;                  //H19_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H19_00*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_01*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_02*/  2,              0,                  8,                  POSD_WRT|DISP_1,
/*H19_03*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_04*/  0x6040,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_05*/  0x0008,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_06*/  0x6060,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_07*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_08*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_09*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H19_00*/0,               0,                 65535,              RSVD_WRT,
/*H19_01*/0,               0,                 65535,              RSVD_WRT,
/*H19_02*/0,               0,                 65535,              RSVD_WRT,
/*H19_03*/0,               0,                 65535,              RSVD_WRT,
/*H19_04*/0,               0,                 65535,              RSVD_WRT,
/*H19_05*/0,               0,                 65535,              RSVD_WRT,
/*H19_06*/0,               0,                 65535,              RSVD_WRT,
/*H19_07*/0,               0,                 65535,              RSVD_WRT,
/*H19_08*/0,               0,                 65535,              RSVD_WRT,
/*H19_09*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H19_Rsvd10;                  //H19_10
    Uint16 H19_Rsvd11;                  //H19_11
    Uint16 H19_Rsvd12;                  //H19_12
    Uint16 H19_Rsvd13;                  //H19_13
    Uint16 H19_Rsvd14;                  //H19_14
    Uint16 H19_Rsvd15;                  //H19_15
    Uint16 H19_Rsvd16;                  //H19_16
    Uint16 H19_Rsvd17;                  //H19_17
    Uint16 H19_Rsvd18;                  //H19_18
    Uint16 H19_Rsvd19;                  //H19_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H19_10*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_11*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_12*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_13*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_14*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_15*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_16*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_17*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_18*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_19*/  2,              0,                  8,                  POSD_WRT|DISP_1, 
#else
/*H19_10*/0,               0,                 65535,              RSVD_WRT,
/*H19_11*/0,               0,                 65535,              RSVD_WRT,
/*H19_12*/0,               0,                 65535,              RSVD_WRT,
/*H19_13*/0,               0,                 65535,              RSVD_WRT,
/*H19_14*/0,               0,                 65535,              RSVD_WRT,
/*H19_15*/0,               0,                 65535,              RSVD_WRT,
/*H19_16*/0,               0,                 65535,              RSVD_WRT,
/*H19_17*/0,               0,                 65535,              RSVD_WRT,
/*H19_18*/0,               0,                 65535,              RSVD_WRT,
/*H19_19*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H19_Rsvd20;                  //H19_20
    Uint16 H19_Rsvd21;                  //H19_21
    Uint16 H19_Rsvd22;                  //H19_22
    Uint16 H19_Rsvd23;                  //H19_23
    Uint16 H19_Rsvd24;                  //H19_24
    Uint16 H19_Rsvd25;                  //H19_25
    Uint16 H19_Rsvd26;                  //H19_26
    Uint16 H19_Rsvd27;                  //H19_27
    Uint16 H19_Rsvd28;                  //H19_28
    Uint16 H19_Rsvd29;                  //H19_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H19_20*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_21*/  0x6040,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_22*/  0x0020,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_23*/  0x607A,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_24*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_25*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_26*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_27*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_28*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_29*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H19_20*/0,               0,                 65535,              RSVD_WRT,
/*H19_21*/0,               0,                 65535,              RSVD_WRT,
/*H19_22*/0,               0,                 65535,              RSVD_WRT,
/*H19_23*/0,               0,                 65535,              RSVD_WRT,
/*H19_24*/0,               0,                 65535,              RSVD_WRT,
/*H19_25*/0,               0,                 65535,              RSVD_WRT,
/*H19_26*/0,               0,                 65535,              RSVD_WRT,
/*H19_27*/0,               0,                 65535,              RSVD_WRT,
/*H19_28*/0,               0,                 65535,              RSVD_WRT,
/*H19_29*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H19_Rsvd30;                  //H19_30
    Uint16 H19_Rsvd31;                  //H19_31
    Uint16 H19_Rsvd32;                  //H19_32
    Uint16 H19_Rsvd33;                  //H19_33
    Uint16 H19_Rsvd34;                  //H19_34
    Uint16 H19_Rsvd35;                  //H19_35
    Uint16 H19_Rsvd36;                  //H19_36
    Uint16 H19_Rsvd37;                  //H19_37
    Uint16 H19_Rsvd38;                  //H19_38
    Uint16 H19_Rsvd39;                  //H19_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H19_30*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_31*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_32*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_33*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_34*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_35*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_36*/  2,              0,                  8,                  POSD_WRT|DISP_1, 
/*H19_37*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_38*/  0x6040,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_39*/  0x0020,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H19_30*/0,               0,                 65535,              RSVD_WRT,
/*H19_31*/0,               0,                 65535,              RSVD_WRT,
/*H19_32*/0,               0,                 65535,              RSVD_WRT,
/*H19_33*/0,               0,                 65535,              RSVD_WRT,
/*H19_34*/0,               0,                 65535,              RSVD_WRT,
/*H19_35*/0,               0,                 65535,              RSVD_WRT,
/*H19_36*/0,               0,                 65535,              RSVD_WRT,
/*H19_37*/0,               0,                 65535,              RSVD_WRT,
/*H19_38*/0,               0,                 65535,              RSVD_WRT,
/*H19_39*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H19_Rsvd40;                  //H19_40
    Uint16 H19_Rsvd41;                  //H19_41
    Uint16 H19_Rsvd42;                  //H19_42
    Uint16 H19_Rsvd43;                  //H19_43
    Uint16 H19_Rsvd44;                  //H19_44
    Uint16 H19_Rsvd45;                  //H19_45
    Uint16 H19_Rsvd46;                  //H19_46
    Uint16 H19_Rsvd47;                  //H19_47
    Uint16 H19_Rsvd48;                  //H19_48
    Uint16 MemCheck19;                  //H19_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H19_40*/  0x60FF,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_41*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_42*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_43*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_44*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_45*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_46*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H19_47*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H19_48*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H19_40*/0,               0,                 65535,              RSVD_WRT,
/*H19_41*/0,               0,                 65535,              RSVD_WRT,
/*H19_42*/0,               0,                 65535,              RSVD_WRT,
/*H19_43*/0,               0,                 65535,              RSVD_WRT,
/*H19_44*/0,               0,                 65535,              RSVD_WRT,
/*H19_45*/0,               0,                 65535,              RSVD_WRT,
/*H19_46*/0,               0,                 65535,              RSVD_WRT,
/*H19_47*/0,               0,                 65535,              RSVD_WRT,
/*H19_48*/0,               0,                 65535,              RSVD_WRT,
#endif
/*H19_49*/  0xC5A9,         0,                  65535,              RSVD_WRT,
//----------------------------预留参数组  H1ALEN = 50 -------------------------------------------------------
/*  Uint16 H1A_Rsvd00;                  //H1A_00
    Uint16 H1A_Rsvd01;                  //H1A_01
    Uint16 H1A_Rsvd02;                  //H1A_02
    Uint16 H1A_Rsvd03;                  //H1A_03
    Uint16 H1A_Rsvd04;                  //H1A_04
    Uint16 H1A_Rsvd05;                  //H1A_05
    Uint16 H1A_Rsvd06;                  //H1A_06
    Uint16 H1A_Rsvd07;                  //H1A_07
    Uint16 H1A_Rsvd08;                  //H1A_08
    Uint16 H1A_Rsvd09;                  //H1A_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1A_00*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_01*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_02*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_03*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_04*/  0x0180,         0,                  65535,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_05*/  0x4000,         0,                  65535,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_06*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H1A_07*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_08*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_09*/  0x0280,              0,             65535,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H1A_00*/0,               0,                 65535,              RSVD_WRT,
/*H1A_01*/0,               0,                 65535,              RSVD_WRT,
/*H1A_02*/0,               0,                 65535,              RSVD_WRT,
/*H1A_03*/0,               0,                 65535,              RSVD_WRT,
/*H1A_04*/0,               0,                 65535,              RSVD_WRT,
/*H1A_05*/0,               0,                 65535,              RSVD_WRT,
/*H1A_06*/0,               0,                 65535,              RSVD_WRT,
/*H1A_07*/0,               0,                 65535,              RSVD_WRT,
/*H1A_08*/0,               0,                 65535,              RSVD_WRT,
/*H1A_09*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1A_Rsvd10;                  //H1A_10
    Uint16 H1A_Rsvd11;                  //H1A_11
    Uint16 H1A_Rsvd12;                  //H1A_12
    Uint16 H1A_Rsvd13;                  //H1A_13
    Uint16 H1A_Rsvd14;                  //H1A_14
    Uint16 H1A_Rsvd15;                  //H1A_15
    Uint16 H1A_Rsvd16;                  //H1A_16
    Uint16 H1A_Rsvd17;                  //H1A_17
    Uint16 H1A_Rsvd18;                  //H1A_18
    Uint16 H1A_Rsvd19;                  //H1A_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1A_10*/  0xC000,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_11*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H1A_12*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_13*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_14*/  0x0380,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_15*/  0xC000,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_16*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H1A_17*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_18*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_19*/  0x0480,         0,                  65535,             POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H1A_10*/0,               0,                 65535,              RSVD_WRT,
/*H1A_11*/0,               0,                 65535,              RSVD_WRT,
/*H1A_12*/0,               0,                 65535,              RSVD_WRT,
/*H1A_13*/0,               0,                 65535,              RSVD_WRT,
/*H1A_14*/0,               0,                 65535,              RSVD_WRT,
/*H1A_15*/0,               0,                 65535,              RSVD_WRT,
/*H1A_16*/0,               0,                 65535,              RSVD_WRT,
/*H1A_17*/0,               0,                 65535,              RSVD_WRT,
/*H1A_18*/0,               0,                 65535,              RSVD_WRT,
/*H1A_19*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1A_Rsvd20;                  //H1A_20
    Uint16 H1A_Rsvd21;                  //H1A_21
    Uint16 H1A_Rsvd22;                  //H1A_22
    Uint16 H1A_Rsvd23;                  //H1A_23
    Uint16 H1A_Rsvd24;                  //H1A_24
    Uint16 H1A_Rsvd25;                  //H1A_25
    Uint16 H1A_Rsvd26;                  //H1A_26
    Uint16 H1A_Rsvd27;                  //H1A_27
    Uint16 H1A_Rsvd28;                  //H1A_28
    Uint16 H1A_Rsvd29;                  //H1A_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1A_20*/  0xC000,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_21*/  255,            0,                  255,                POSD_WRT|DISP_3,
/*H1A_22*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_23*/  0,              0,                  65535,              POSD_WRT|DISP_5,
/*H1A_24*/  1,              0,                  8,                  POSD_WRT|DISP_1,
/*H1A_25*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_26*/  0x6041,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_27*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_28*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_29*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H1A_20*/0,               0,                 65535,              RSVD_WRT,
/*H1A_21*/0,               0,                 65535,              RSVD_WRT,
/*H1A_22*/0,               0,                 65535,              RSVD_WRT,
/*H1A_23*/0,               0,                 65535,              RSVD_WRT,
/*H1A_24*/0,               0,                 65535,              RSVD_WRT,
/*H1A_25*/0,               0,                 65535,              RSVD_WRT,
/*H1A_26*/0,               0,                 65535,              RSVD_WRT,
/*H1A_27*/0,               0,                 65535,              RSVD_WRT,
/*H1A_28*/0,               0,                 65535,              RSVD_WRT,
/*H1A_29*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1A_Rsvd30;                  //H1A_30
    Uint16 H1A_Rsvd31;                  //H1A_31
    Uint16 H1A_Rsvd32;                  //H1A_32
    Uint16 H1A_Rsvd33;                  //H1A_33
    Uint16 H1A_Rsvd34;                  //H1A_34
    Uint16 H1A_Rsvd35;                  //H1A_35
    Uint16 H1A_Rsvd36;                  //H1A_36
    Uint16 H1A_Rsvd37;                  //H1A_37
    Uint16 H1A_Rsvd38;                  //H1A_38
    Uint16 H1A_Rsvd39;                  //H1A_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1A_30*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_31*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_32*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_33*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_34*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_35*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_36*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_37*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_38*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_39*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
#else
/*H1A_30*/0,               0,                 65535,              RSVD_WRT,
/*H1A_31*/0,               0,                 65535,              RSVD_WRT,
/*H1A_32*/0,               0,                 65535,              RSVD_WRT,
/*H1A_33*/0,               0,                 65535,              RSVD_WRT,
/*H1A_34*/0,               0,                 65535,              RSVD_WRT,
/*H1A_35*/0,               0,                 65535,              RSVD_WRT,
/*H1A_36*/0,               0,                 65535,              RSVD_WRT,
/*H1A_37*/0,               0,                 65535,              RSVD_WRT,
/*H1A_38*/0,               0,                 65535,              RSVD_WRT,
/*H1A_39*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1A_Rsvd40;                  //H1A_40
    Uint16 H1A_Rsvd41;                  //H1A_41
    Uint16 H1A_Rsvd42;                  //H1A_42
    Uint16 H1A_Rsvd43;                  //H1A_43
    Uint16 H1A_Rsvd44;                  //H1A_44
    Uint16 H1A_Rsvd45;                  //H1A_45
    Uint16 H1A_Rsvd46;                  //H1A_46
    Uint16 H1A_Rsvd47;                  //H1A_47
    Uint16 H1A_Rsvd48;                  //H1A_48
    Uint16 MemCheck1A;                  //H1A_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1A_40*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_41*/  2,              0,                  8,                  POSD_WRT|DISP_1,
/*H1A_42*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_43*/  0x6041,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_44*/  0x0008,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_45*/  0x6061,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_46*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1A_47*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1A_48*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H1A_40*/0,               0,                 65535,              RSVD_WRT,
/*H1A_41*/0,               0,                 65535,              RSVD_WRT,
/*H1A_42*/0,               0,                 65535,              RSVD_WRT,
/*H1A_43*/0,               0,                 65535,              RSVD_WRT,
/*H1A_44*/0,               0,                 65535,              RSVD_WRT,
/*H1A_45*/0,               0,                 65535,              RSVD_WRT,
/*H1A_46*/0,               0,                 65535,              RSVD_WRT,
/*H1A_47*/0,               0,                 65535,              RSVD_WRT,
/*H1A_48*/0,               0,                 65535,              RSVD_WRT,
#endif
/*H1A_49*/  0xC5AA,         0,                  65535,              RSVD_WRT,
//----------------------------预留参数组  H1BLEN = 50 -------------------------------------------------------
/*  Uint16 H1B_Rsvd00;                  //H1B_00
    Uint16 H1B_Rsvd01;                  //H1B_01
    Uint16 H1B_Rsvd02;                  //H1B_02
    Uint16 H1B_Rsvd03;                  //H1B_03
    Uint16 H1B_Rsvd04;                  //H1B_04
    Uint16 H1B_Rsvd05;                  //H1B_05
    Uint16 H1B_Rsvd06;                  //H1B_06
    Uint16 H1B_Rsvd07;                  //H1B_07
    Uint16 H1B_Rsvd08;                  //H1B_08
    Uint16 H1B_Rsvd09;                  //H1B_09           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1B_00*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_01*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_02*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_03*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_04*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_05*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_06*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_07*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_08*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_09*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H1B_00*/0,               0,                 65535,              RSVD_WRT,
/*H1B_01*/0,               0,                 65535,              RSVD_WRT,
/*H1B_02*/0,               0,                 65535,              RSVD_WRT,
/*H1B_03*/0,               0,                 65535,              RSVD_WRT,
/*H1B_04*/0,               0,                 65535,              RSVD_WRT,
/*H1B_05*/0,               0,                 65535,              RSVD_WRT,
/*H1B_06*/0,               0,                 65535,              RSVD_WRT,
/*H1B_07*/0,               0,                 65535,              RSVD_WRT,
/*H1B_08*/0,               0,                 65535,              RSVD_WRT,
/*H1B_09*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1B_Rsvd10;                  //H1B_10
    Uint16 H1B_Rsvd11;                  //H1B_11
    Uint16 H1B_Rsvd12;                  //H1B_12
    Uint16 H1B_Rsvd13;                  //H1B_13
    Uint16 H1B_Rsvd14;                  //H1B_14
    Uint16 H1B_Rsvd15;                  //H1B_15
    Uint16 H1B_Rsvd16;                  //H1B_16
    Uint16 H1B_Rsvd17;                  //H1B_17
    Uint16 H1B_Rsvd18;                  //H1B_18
    Uint16 H1B_Rsvd19;                  //H1B_19           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1B_10*/  2,              0,                  8,                  POSD_WRT|DISP_1,
/*H1B_11*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_12*/  0x6041,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_13*/  0x0020,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_14*/  0x6064,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_15*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_16*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_17*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_18*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_19*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP, 
#else
/*H1B_10*/0,               0,                 65535,              RSVD_WRT,
/*H1B_11*/0,               0,                 65535,              RSVD_WRT,
/*H1B_12*/0,               0,                 65535,              RSVD_WRT,
/*H1B_13*/0,               0,                 65535,              RSVD_WRT,
/*H1B_14*/0,               0,                 65535,              RSVD_WRT,
/*H1B_15*/0,               0,                 65535,              RSVD_WRT,
/*H1B_16*/0,               0,                 65535,              RSVD_WRT,
/*H1B_17*/0,               0,                 65535,              RSVD_WRT,
/*H1B_18*/0,               0,                 65535,              RSVD_WRT,
/*H1B_19*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1B_Rsvd20;                  //H1B_20
    Uint16 H1B_Rsvd21;                  //H1B_21
    Uint16 H1B_Rsvd22;                  //H1B_22
    Uint16 H1B_Rsvd23;                  //H1B_23
    Uint16 H1B_Rsvd24;                  //H1B_24
    Uint16 H1B_Rsvd25;                  //H1B_25
    Uint16 H1B_Rsvd26;                  //H1B_26
    Uint16 H1B_Rsvd27;                  //H1B_27
    Uint16 H1B_Rsvd28;                  //H1B_28
    Uint16 H1B_Rsvd29;                  //H1B_29           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1B_20*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_21*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_22*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_23*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_24*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_25*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_26*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_27*/  2,              0,                  8,                  POSD_WRT|DISP_1,
/*H1B_28*/  0x0010,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_29*/  0x6041,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H1B_20*/0,               0,                 65535,              RSVD_WRT,
/*H1B_21*/0,               0,                 65535,              RSVD_WRT,
/*H1B_22*/0,               0,                 65535,              RSVD_WRT,
/*H1B_23*/0,               0,                 65535,              RSVD_WRT,
/*H1B_24*/0,               0,                 65535,              RSVD_WRT,
/*H1B_25*/0,               0,                 65535,              RSVD_WRT,
/*H1B_26*/0,               0,                 65535,              RSVD_WRT,
/*H1B_27*/0,               0,                 65535,              RSVD_WRT,
/*H1B_28*/0,               0,                 65535,              RSVD_WRT,
/*H1B_29*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1B_Rsvd30;                  //H1B_30
    Uint16 H1B_Rsvd31;                  //H1B_31
    Uint16 H1B_Rsvd32;                  //H1B_32
    Uint16 H1B_Rsvd33;                  //H1B_33
    Uint16 H1B_Rsvd34;                  //H1B_34
    Uint16 H1B_Rsvd35;                  //H1B_35
    Uint16 H1B_Rsvd36;                  //H1B_36
    Uint16 H1B_Rsvd37;                  //H1B_37
    Uint16 H1B_Rsvd38;                  //H1B_38
    Uint16 H1B_Rsvd39;                  //H1B_39           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1B_30*/  0x0020,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_31*/  0x606C,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_32*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_33*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_34*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP,
/*H1B_35*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_36*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP, 
/*H1B_37*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_38*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP, 
/*H1B_39*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
#else
/*H1B_30*/0,               0,                 65535,              RSVD_WRT,
/*H1B_31*/0,               0,                 65535,              RSVD_WRT,
/*H1B_32*/0,               0,                 65535,              RSVD_WRT,
/*H1B_33*/0,               0,                 65535,              RSVD_WRT,
/*H1B_34*/0,               0,                 65535,              RSVD_WRT,
/*H1B_35*/0,               0,                 65535,              RSVD_WRT,
/*H1B_36*/0,               0,                 65535,              RSVD_WRT,
/*H1B_37*/0,               0,                 65535,              RSVD_WRT,
/*H1B_38*/0,               0,                 65535,              RSVD_WRT,
/*H1B_39*/0,               0,                 65535,              RSVD_WRT,
#endif
/*  Uint16 H1B_Rsvd40;                  //H1B_40
    Uint16 H1B_Rsvd41;                  //H1B_41
    Uint16 H1B_Rsvd42;                  //H1B_42
    Uint16 H1B_Rsvd43;                  //H1B_43
    Uint16 H1B_Rsvd44;                  //H1B_44
    Uint16 H1B_Rsvd45;                  //H1B_45
    Uint16 H1B_Rsvd46;                  //H1B_46
    Uint16 H1B_Rsvd47;                  //H1B_47
    Uint16 H1B_Rsvd48;                  //H1B_48
    Uint16 MemCheck1B;                  //H1B_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if CAN_ENABLE_SWITCH
/*H1B_40*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP, 
/*H1B_41*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_42*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP, 
/*H1B_43*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_8|HNUM_TYP|HIGH_WORD,
/*H1B_44*/  0,              0,                  65535,              ANY_WRT|    DISP_4,
/*H1B_45*/  0,              0,                  65535,              RSVD_WRT,
/*H1B_46*/  0,              0,                  65535,              RSVD_WRT,
/*H1B_47*/  0,              0,                  65535,              RSVD_WRT,
/*H1B_48*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H1B_40*/0,               0,                 65535,              RSVD_WRT,
/*H1B_41*/0,               0,                 65535,              RSVD_WRT,
/*H1B_42*/0,               0,                 65535,              RSVD_WRT,
/*H1B_43*/0,               0,                 65535,              RSVD_WRT,
/*H1B_44*/0,               0,                 65535,              RSVD_WRT,
/*H1B_45*/0,               0,                 65535,              RSVD_WRT,
/*H1B_46*/0,               0,                 65535,              RSVD_WRT,
/*H1B_47*/0,               0,                 65535,              RSVD_WRT,
/*H1B_48*/0,               0,                 65535,              RSVD_WRT,
#endif
/*H1B_49*/  0xC5AB,         0,                  65535,              RSVD_WRT,
//----------------------------预留参数组  H1CLEN = 50 -------------------------------------------------------
/*  Uint16 H1C_Rsvd00;                  //H1C_00 605A
    Uint16 H1C_Rsvd01;                  //H1C_01 605D
    Uint16 H1C_Rsvd02;                  //H1C_02 6060
    Uint16 H1C_Rsvd03;                  //H1C_03 6065
    Uint16 H1C_Rsvd04;                  //H1C_04
    Uint16 H1C_Rsvd05;                  //H1C_05 6066
    Uint16 H1C_Rsvd06;                  //H1C_06 6067
    Uint16 H1C_Rsvd07;                  //H1C_07 
    Uint16 H1C_Rsvd08;                  //H1C_08 6068
    Uint16 H1C_Rsvd09;                  //H1C_09 606D           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1C_00*/  2,              0,                  7,                  POSD_WRT|DISP_1|NEXT_ACT,
/*H1C_01*/  1,              1,                  3,                  POSD_WRT|DISP_1|NEXT_ACT,
/*H1C_02*/  0,              0,                  10,                 POSD_WRT|DISP_2|NEXT_ACT,
/*H1C_03*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_04*/  0x0030,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_05*/  0,            	0,                  65535,              POSD_WRT|DISP_10|NEXT_ACT,
/*H1C_06*/  0,            	0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_07*/  734,            0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_08*/  0,              0,                  65535,              POSD_WRT|DISP_5|NEXT_ACT,
/*H1C_09*/  10,             0,                  65535,              POSD_WRT|DISP_5|NEXT_ACT,

/*  Uint16 H1C_Rsvd11;                  //H1C_10 606E
	Uint16 H1C_Rsvd11;                  //H1C_11 606F
    Uint16 H1C_Rsvd12;                  //H1C_12 6070
    Uint16 H1C_Rsvd13;                  //H1C_13 6071
    Uint16 H1C_Rsvd14;                  //H1C_14 6072
    Uint16 H1C_Rsvd15;                  //H1C_15 607A
    Uint16 H1C_Rsvd16;                  //H1C_16
    Uint16 H1C_Rsvd17;                  //H1C_17 607C
    Uint16 H1C_Rsvd18;                  //H1C_18
    Uint16 H1C_Rsvd19;                  //H1C_19 607D-01              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1C_10*/  0,              0,                  65535,              POSD_WRT|DISP_5|NEXT_ACT,
/*H1C_11*/  10,             0,                  65535,               RSVD_WRT|DISP_5|NEXT_ACT,
/*H1C_12*/  0,              0,                  65535,               RSVD_WRT|DISP_5|NEXT_ACT,
/*H1C_13*/  0,              (Uint16)-5000,      5000,                POSD_WRT|DISP_5|INT_SIGN|NEXT_ACT,
/*H1C_14*/  5000,           0,                  5000,                POSD_WRT|DISP_4|NEXT_ACT,
/*H1C_15*/  0,              0,                  0xFFFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|NEXT_ACT,
/*H1C_16*/  0,              0x8000,             0x7FFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD|NEXT_ACT,
/*H1C_17*/  0,              0x0000,             0xFFFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|NEXT_ACT,
/*H1C_18*/  0,              0x8000,             0x7FFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD|NEXT_ACT,
/*H1C_19*/  0,              0x0000,             0xFFFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|NEXT_ACT,

/*  
    Uint16 H1C_Rsvd20;                  //H1C_20
    Uint16 H1C_Rsvd21;                  //H1C_21 607D-02
    Uint16 H1C_Rsvd22;                  //H1C_22 
    Uint16 H1C_Rsvd23;                  //H1C_23 607E
    Uint16 H1C_Rsvd24;                  //H1C_24 607F
    Uint16 H1C_Rsvd25;                  //H1C_25
    Uint16 H1C_Rsvd26;                  //H1C_26 6081
    Uint16 H1C_Rsvd27;                  //H1C_27
    Uint16 H1C_Rsvd28;                  //H1C_28 6083
    Uint16 H1C_Rsvd29;                  //H1C_29		  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1C_20*/  0x8000,         0x8000,             0x7FFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD|NEXT_ACT,
/*H1C_21*/  0xFFFF,         0x0000,             0xFFFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|NEXT_ACT,
/*H1C_22*/  0x7FFF,         0x8000,             0x7FFF,              POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD|NEXT_ACT,
/*H1C_23*/  0,              0,                  255,                 POSD_WRT|DISP_3|NEXT_ACT,
/*H1C_24*/  0,              0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_25*/  0x0640,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_26*/  0xAAAB,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_27*/  0x001A,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_28*/  0xAAAB,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_29*/  0x682A,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,

/*  Uint16 H1C_Rsvd30;                  //H1C_30 6084
	Uint16 H1C_Rsvd31;                  //H1C_31 
    Uint16 H1C_Rsvd32;                  //H1C_32 6085
    Uint16 H1C_Rsvd33;                  //H1C_33
    Uint16 H1C_Rsvd34;                  //H1C_34 6086
    Uint16 H1C_Rsvd35;                  //H1C_35 6087
    Uint16 H1C_Rsvd36;                  //H1C_36
    Uint16 H1C_Rsvd37;                  //H1C_37 6091-01
    Uint16 H1C_Rsvd38;                  //H1C_38
    Uint16 H1C_Rsvd39;                  //H1C_39 6091-02  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1C_30*/  0xAAAB,         0,                  65535,               POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_31*/  0x682A,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_32*/  0xAAAB,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_33*/  0x682A,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_34*/  0,              0,                  1,                  POSD_WRT|DISP_1|NEXT_ACT,
/*H1C_35*/  65535,          0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_36*/  65535,          0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_37*/  1,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_38*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_39*/  1,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,

/*  Uint16 H1C_Rsvd40;                  //H1C_40
    Uint16 H1C_Rsvd41;                  //H1C_41 6093-01
    Uint16 H1C_Rsvd42;                  //H1C_42
    Uint16 H1C_Rsvd43;                  //H1C_43 6093-02
    Uint16 H1C_Rsvd44;                  //H1C_44 
    Uint16 H1C_Rsvd45;                  //H1C_45 6094-01
    Uint16 H1C_Rsvd46;                  //H1C_46 
    Uint16 H1C_Rsvd47;                  //H1C_47 6094-02
    Uint16 H1C_Rsvd48;                  //H1C_48 
    Uint16 H1C_Rsvd49;                  //H1C_49    组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1C_40*/  0,              0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_41*/  1,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_42*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_43*/  1,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_44*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_45*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_46*/  0x0010,         0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_47*/  60,             0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1C_48*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1C_49*/  0xC5AC,         0,                  65535,              RSVD_WRT,


//----------------------------预留参数组  H1DLEN = 50 -------------------------------------------------------
/*  Uint16 H1D_Rsvd00;                  //H1D_00 6095-01
    Uint16 H1D_Rsvd01;                  //H1D_01 
    Uint16 H1D_Rsvd02;                  //H1D_02 6095-02
    Uint16 H1D_Rsvd03;                  //H1D_03
    Uint16 H1D_Rsvd04;                  //H1D_04 6097-01
    Uint16 H1D_Rsvd05;                  //H1D_05
    Uint16 H1D_Rsvd06;                  //H1D_06 6097-02
    Uint16 H1D_Rsvd07;                  //H1D_07
    Uint16 H1D_Rsvd08;                  //H1D_08 6098
    Uint16 H1D_Rsvd09;                  //H1D_09 6099-01          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1D_00*/  1,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_01*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_02*/  1,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_03*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_04*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_05*/  0x0640,         0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_06*/  60,             0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_07*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_08*/  0,              0,                  35,                 POSD_WRT|DISP_2|NEXT_ACT,
/*H1D_09*/  0xAAAB,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*  Uint16 H1D_Rsvd10;                  //H1D_10
    Uint16 H1D_Rsvd11;                  //H1D_11 6099-02
    Uint16 H1D_Rsvd12;                  //H1D_12
    Uint16 H1D_Rsvd13;                  //H1D_13 609a
    Uint16 H1D_Rsvd14;                  //H1D_14
    Uint16 H1D_Rsvd15;                  //H1D_15 60c2-01
    Uint16 H1D_Rsvd16;                  //H1D_16 60c2-02
    Uint16 H1D_Rsvd17;                  //H1D_17 60c5
    Uint16 H1D_Rsvd18;                  //H1D_18 
    Uint16 H1D_Rsvd19;                  //H1D_19 60c6          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1D_10*/  0x001A,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_11*/  0xAAAB,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_12*/  0x0002,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_13*/  0xAAAB,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_14*/  0x682A,         0,                  65535,              POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_15*/  1,              0,                  20,                 RSVD_WRT|DISP_2|NEXT_ACT,
/*H1D_16*/  253,            253,                253,                RSVD_WRT|DISP_3|NEXT_ACT,
/*H1D_17*/  1000,           0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_18*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_19*/  1000,           0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|NEXT_ACT,

/*  Uint16 H1D_Rsvd20;                  //H1D_20
    Uint16 H1D_Rsvd21;                  //H1D_21 60e0
    Uint16 H1D_Rsvd22;                  //H1D_22 60e1
    Uint16 H1D_Rsvd23;                  //H1D_23 60e6
    Uint16 H1D_Rsvd24;                  //H1D_24 60fE-02
    Uint16 H1D_Rsvd25;                  //H1D_25 
    Uint16 H1D_Rsvd26;                  //H1D_26 60ff
    Uint16 H1D_Rsvd27;                  //H1D_27
    Uint16 H1D_Rsvd28;                  //H1D_28
    Uint16 H1D_Rsvd29;                  //H1D_29 */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1D_20*/  0,              0,                  65535,              RSVD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_21*/  5000,           0,                  5000,               POSD_WRT|DISP_4|NEXT_ACT,
/*H1D_22*/  5000,           0,                  5000,               POSD_WRT|DISP_4|NEXT_ACT,
/*H1D_23*/  0,              0,                  1,                  POSD_WRT|DISP_1|NEXT_ACT,
/*H1D_24*/  0,              0,                  0xFFFF,             POSD_WRT|TWO_WORD|DISP_10|NEXT_ACT,
/*H1D_25*/  0,              0,                  0xFFFF,             POSD_WRT|TWO_WORD|DISP_10|HIGH_WORD|NEXT_ACT,
/*H1D_26*/  0,              0x0000,             0xFFFF,             POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|NEXT_ACT,
/*H1D_27*/  0,              0x8000,             0x7FFF,             POSD_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD|NEXT_ACT,
/*H1D_28*/  0,              0,                  65535,              RSVD_WRT,
/*H1D_29*/  0,              0,                  65535,              RSVD_WRT,

/*  Uint16 H1D_Rsvd30;                  //H1D_30
    Uint16 H1D_Rsvd31;                  //H1D_31
    Uint16 H1D_Rsvd32;                  //H1D_32
    Uint16 ComMoniPara1IndexLow;        //H1D_33 通信监控参数1 索引-子索引 16进制显示
    Uint16 ComMoniPara1IndexHigh;       //H1D_34 
    Uint16 ComMoniPara2IndexLow;        //H1D_35 通信监控参数2 索引-子索引 16进制显示
    Uint16 ComMoniPara2IndexHigh;       //H1D_36
    Uint16 ComMoniPara3IndexLow;        //H1D_37 通信监控参数3 索引-子索引 16进制显示
    Uint16 ComMoniPara3IndexHigh;       //H1D_38 
    Uint16 ComMoniPara4IndexLow;        //H1D_39 通信监控参数4 索引-子索引 16进制显示  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1D_30*/  0,              0,                  65535,              RSVD_WRT,
/*H1D_31*/  0,              0,                  65535,              RSVD_WRT,
/*H1D_32*/  0,              0,                  65535,              RSVD_WRT,
/*H1D_33*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP,
/*H1D_34*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP|HIGH_WORD,
/*H1D_35*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP,
/*H1D_36*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP|HIGH_WORD,
/*H1D_37*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP,
/*H1D_38*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP|HIGH_WORD,
/*H1D_39*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP,

/*  
    Uint16 ComMoniPara4IndexHigh;       //H1D_40 
    Uint16 ComMoniPara1Low;             //H1D_41 通信监控参数1数据
    Uint16 ComMoniPara1High;            //H1D_42 
    Uint16 ComMoniPara2Low;             //H1D_43 通信监控参数2数据
    Uint16 ComMoniPara2High;            //H1D_44 
    Uint16 ComMoniPara3Low;             //H1D_45 通信监控参数3数据
    Uint16 ComMoniPara3High;            //H1D_46
    Uint16 ComMoniPara4Low;             //H1D_47 通信监控参数4数据
    Uint16 ComMoniPara4High;            //H1D_48
    Uint16 MemCheck1D;                  //H1D_49      组校验字     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H1D_40*/  0,              0,                  65535,              ANY_WRT|TWO_WORD|DISP_6|HNUM_TYP|HIGH_WORD,
/*H1D_41*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN,
/*H1D_42*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD,
/*H1D_43*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN,
/*H1D_44*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD,
/*H1D_45*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN,
/*H1D_46*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD,
/*H1D_47*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN,
/*H1D_48*/  0,              0,                  65535,              DISP_WRT|TWO_WORD|DISP_11|INT_SIGN|HIGH_WORD,
/*H1D_49*/  0xC5AD,         0,                  65535,              RSVD_WRT,

/*************************************************************************************************************
 *
 * 辅助功能码参数如下
 *
*************************************************************************************************************/
//----------------------------显示参数  H0BLEN = 43 -------------------------------------------------------
/*  Uint16 DP_MotorSpd;          //H0B_00  电机转速              
    Uint16 DP_SpdRef;            //H0B_01  速度指令
    Uint16 DP_ToqRef;            //H0B_02  内部转矩指令（相对于  转矩的值，归一化处理）
    Uint16 DP_DIState;           //H0B_03  输入信号监视DI
    Uint16 H0B_04_Rsvd;          //H0B_04  预留
    Uint16 DP_DOState;           //H0B_05  输出信号监视DO
    Uint16 H0B_06_Rsvd;          //H0B_06  预留
    Uint16 DP_AbsPosCnt_Lo;      //H0B_07  绝对位置计数器（32位十进制显示）低位
    Uint16 DP_AbsPosCnt_Hi;      //H0B_08  绝对位置计数器（32位十进制显示）高位
    Uint16 DP_MechaAngle;        //H0B_09  机械角度                                 */  
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_00*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  INT_SIGN,
/*H0B_01*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  INT_SIGN,
/*H0B_02*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|   ONE_DOT|  INT_SIGN,
/*H0B_03*/  0,              0,                  65535,              DISP_WRT|  NNUM_TYP|    DISP_9,
/*H0B_04*/  0,              0,                  65535,              RSVD_WRT,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0B_05*/  0,              0,                  65535,              DISP_WRT|  NNUM_TYP|    DISP_3,
#else
/*H0B_05*/  0,              0,                  65535,              DISP_WRT|  NNUM_TYP|    DISP_5,
#endif
/*H0B_06*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_07*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_08*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_09*/  0,              0,                  65535,              DISP_WRT|    DISP_5,

/*  Uint16 DP_EleTheta;          //H0B_10  电气角度
    Uint16 DP_SpdRef_Puse;       //H0B_11  输入指令脉冲速度（仅脉冲位置控制有效）
    Uint16 DP_AvrLoad;           //H0B_12  平均负载率%
    Uint16 DP_InPulseCnt_Lo;     //H0B_13  输入指令脉冲计数器（32位十进制显示）低位
    Uint16 DP_InPulseCnt_Hi;     //H0B_14  输入指令脉冲计数器（32位十进制显示）高位
    Uint16 DP_PosErrCnt_Lo;      //H0B_15  偏差计数器（位置偏差量）低位
    Uint16 DP_PosErrCnt_Hi;      //H0B_16  偏差计数器（位置偏差量）高位
    Uint16 DP_FbPuleCnt_Lo;      //H0B_17  反馈脉冲计数器（32位十进制显示）低位
    Uint16 DP_FbPuleCnt_Hi;      //H0B_18  反馈脉冲计数器（32位十进制显示）高位
    Uint16 DP_ServoTime_Lo;      //H0B_19  总运行时间（32位10进制显示） 低位                     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_10*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   ONE_DOT,
/*H0B_11*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  INT_SIGN,
/*H0B_12*/  0,              0,                  8000,               DISP_WRT|    DISP_4|   ONE_DOT,
/*H0B_13*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_14*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_15*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_16*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_17*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_18*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_19*/  0,              0,                  65535,              DISP_WRT|  TWO_WORD|   DISP_10|   ONE_DOT,

/*  Uint16 DP_ServoTime_Hi;      //H0B_20  总运行时间（32位10进制显示） 高位
    Uint16 DP_Ai1SampVolt;       //H0B_21  AI1采样电压 采样值(有效值)
    Uint16 DP_Ai2SampVolt;       //H0B_22  AI2采样电压 采样值(有效值)
    Uint16 DP_Ai3SampVolt;       //H0B_23  AI2采样电压 采样值(有效值)
    Uint16 DP_USampCur;          //H0B_24  U电流采样值(有效值)
    Uint16 DP_IdenTheta;         //H0B_25 电压注入辨识得到的角度
    Uint16 DP_Udc;               //H0B_26  母线电压
    Uint16 DP_DriverTemp;        //H0B_27  驱动器温度
    Uint16 DP_FpgaAbsRomErr;     //H0B_28  FPGA给出绝对编码器故障信息
    Uint16 DP_FpgaSysState;      //H0B_29  FPGA给出的系统状态信息  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_20*/  0,              0,                  65535,              DISP_WRT|  TWO_WORD|   DISP_10|   ONE_DOT| HIGH_WORD,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0B_21*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_22*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_23*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H0B_21*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  INT_SIGN|  TWO_DOT,
/*H0B_22*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  INT_SIGN|  TWO_DOT,
/*H0B_23*/  0,              (Uint16)-32767,     32767,              RSVD_WRT|    DISP_5|  INT_SIGN|  TWO_DOT,
#endif
/*H0B_24*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   TWO_DOT,
/*H0B_25*/  0,              0,                  3600,               DISP_WRT|    DISP_4|   ONE_DOT,
/*H0B_26*/  0,              0,                  65535,              DISP_WRT|    DISP_5|   ONE_DOT,
/*H0B_27*/  0,              0,                  65535,              DISP_WRT|    DISP_3,
/*H0B_28*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0B_29*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,


/*  Uint16 DP_FpgaSysErr;        //H0B_30  FPGA给出的系统故障信息
    Uint16 DP_EncStBit1;         //H0B_31  20位编码器状态位1
    Uint16 DP_EncStBit2;         //H0B_32  20位编码器状态位2
    Uint16 DP_DefotNo;           //H0B_33  故障记录的显示  0 --> 当前故障
    Uint16 DP_ErrCode;           //H0B_34  故障码
    Uint16 DP_DefotTime_Lo;      //H0B_35  所故障时间戳  低位
    Uint16 DP_DefotTime_Hi;      //H0B_36  所选故障时间戳  高位
    Uint16 DP_Spd_Defot;         //H0B_37  所选故障时转速
    Uint16 DP_CurtU_Defot;       //H0B_38  所选故障时U相电流
    Uint16 DP_CurtV_Defot;       //H0B_39  所选故障时V相电流                     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_30*/  0,              0,                  65535,              DISP_WRT|    HNUM_TYP |   DISP_4,
/*H0B_31*/  0,              0,                  65535,               ANY_WRT|    HNUM_TYP |   DISP_4,
/*H0B_32*/  0,              0,                  65535,               ANY_WRT|    HNUM_TYP |   DISP_4,
/*H0B_33*/  0,              0,                  9,                   ANY_WRT|    DISP_1,
/*H0B_34*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|   DISP_5,
/*H0B_35*/  0,              0,                  65535,              DISP_WRT|  TWO_WORD|  DISP_10|   ONE_DOT,
/*H0B_36*/  0,              0,                  65535,              DISP_WRT|  TWO_WORD|  DISP_10| HIGH_WORD,
/*H0B_37*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5| INT_SIGN,
/*H0B_38*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  TWO_DOT|  INT_SIGN,
/*H0B_39*/  0,              (Uint16)-32767,     32767,              DISP_WRT|    DISP_5|  TWO_DOT|  INT_SIGN,

/*  Uint16 DP_Udc_Defot;         //H0B_40  所选故障时母线电压
    Uint16 DP_DI_Defot;          //H0B_41  所选故障时输入端子状态
    Uint16 DP_DO_Defot;          //H0B_42  所选故障时输出端子状态
    Uint16 DP_ParaErrGroup;      //H0B_43  参数异常的功能码组号
    Uint16 DP_ParaErrOffset;     //H0B_44  参数异常的功能码组内偏置
    Uint16 DP_InnerErrCode;      //H0B_45  内部故障码
    Uint16 DP_FpgaAbsRomErr_Defot;  //H0B_46  所选故障时FPGA给出绝对编码器故障信息
    Uint16 DP_FpgaSysState_Defot;   //H0B_47  所选故障时FPGA给出的系统状态信息
    Uint16 DP_FpgaSysErr_Defot;  //H0B_48  所选故障时FPGA给出的系统故障信息
    Uint16 DP_EncStBit1_Defot;   //H0B_49  所选故障时20位编码器状态位 1                */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_40*/  0,              0,                  65535,              DISP_WRT|    DISP_5|   ONE_DOT,
/*H0B_41*/  0,              0,                  65535,              DISP_WRT|  NNUM_TYP|    DISP_9,
/*H0B_42*/  0,              0,                  65535,              DISP_WRT|  NNUM_TYP|    DISP_5,
/*H0B_43*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_2,
/*H0B_44*/  0,              0,                  65535,              DISP_WRT|    DISP_2,
/*H0B_45*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_46*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_47*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_48*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_49*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,

/*  Uint16 DP_EncRsvd_Defot;     //H0B_50  所选故障时编码器状态保留
    Uint16 DP_InnerErrCode_Defot;   //H0B_51  所选故障时内部故障码
    Uint16 DP_FpgaTmFlt_Defot;   //H0B_52  所选故障时故障时FPGA超时故障标准位 
    Uint16 DP_Rsvd53;            //H0B_53 随动偏差低位(指令单位)
    Uint16 DP_Rsvd54;            //H0B_54 随动偏差高位(指令单位)
    Uint16 DP_MotorSpdL;         //H0B_55  电机转速0.1rpm
    Uint16 DP_MotorSpdH;         //H0B_56  电机转速0.1rpm
    Uint16 DP_CtrlUdc;           //H0B_57  控制电母线电压
    Uint16 DP_MechanicalAbsPosL;    //H0B_58  机械绝对位置 低32位（编码器单位）
    Uint16 DP_MechanicalAbsPosH;    //H0B_59  机械绝对位置 低32位（编码器单位）  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_50*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_51*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_52*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_53*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_54*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_55*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|    DISP_6|   ONE_DOT|  INT_SIGN,
/*H0B_56*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|    DISP_6|   ONE_DOT|  INT_SIGN| HIGH_WORD,
/*H0B_57*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   ONE_DOT,
/*H0B_58*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_59*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,

/*  Uint16 DP_MechanicalAbsPosH32L; //H0B_60  机械绝对位置 高32位（编码器单位）
    Uint16 DP_MechanicalAbsPosH32H; //H0B_61  机械绝对位置 高32位（编码器单位）
    Uint16 DP_LineVolt;            //H0B_62   驱动器输出线电压有效值
    Uint16 DP_Rsvd63;            //H0B_63  
    Uint16 DP_InputPulseCntRTL;  //H0B_64  实时位置指令输入低16位
    Uint16 DP_InputPulseCntRTH;  //H0B_65  实时位置指令输入高16位
    Uint16 DP_Rsvd66;            //H0B_66
    Uint16 DP_Rsvd67;            //H0B_67
    Uint16 DP_FpgaTmFlt;         //H0B_68 FPGA超时故障标准位
    Uint16 DP_ABSEncErr;         //H0B_69 海德汉编码器故障      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_60*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_61*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_62*/  0,              0,                  65535,              DISP_WRT|    DISP_4|   ONE_DOT,
/*H0B_63*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_64*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_65*/  0,              0x8000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_66*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_67*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_68*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,
/*H0B_69*/  0,              0,                  65535,              DISP_WRT|  HNUM_TYP|    DISP_4,

/*  Uint16 DP_AbsExtData;        //H0B_70 串行编码器扩展数据/多圈数据
    Uint16 DP_ABSEncSFdbL;       //H0B_71 串行编码器反馈单圈位置
    Uint16 DP_ABSEncSFdbH;       //H0B_72 串行编码器反馈单圈位置
    Uint16 DP_Rsvd73;            //H0B_73
    Uint16 DP_Rsvd74;            //H0B_74
    Uint16 DP_JRatioOnline;      //H0B_75  在线惯量辨识时负载惯量比
    Uint16 DP_TLOnline;          //H0B_76  在线惯量辨识时外部负载 
    Uint16 DP_EncPosL;           //H0B_77 编码器位置L
    Uint16 DP_EncPosH;           //H0B_78 编码器位置H
    Uint16 DP_EncPosH32L;        //H0B_79 编码器位置高32位 L             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_70*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_71*/  0,              0x0000,             0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_11,
/*H0B_72*/  0,              0x0000,             0x7FFF,             DISP_WRT|  TWO_WORD|   DISP_11| HIGH_WORD,
/*H0B_73*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_74*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_75*/  0,              0,                  65535,              DISP_WRT| TWO_DOT| DISP_5 ,
/*H0B_76*/  0,           (Uint16)-1000,         1000,               DISP_WRT| ONE_DOT| DISP_5| INT_SIGN,
/*H0B_77*/	0,				0x0000, 			0xFFFF, 			DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/*H0B_78*/	0,				0x8000, 			0x7FFF, 			DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_79*/	0,				0x0000, 			0xFFFF, 			DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,


/*  Uint16 DP_EncPosH32H;        //H0B_80 编码器位置高32位 H
    Uint16 DP_MechSingleAbsPosL;    //H0B_81  旋转负载单圈位置 低32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH;    //H0B_82  旋转负载单圈位置 低32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH32L; //H0B_83  旋转负载单圈位置 高32位（编码器单位）
    Uint16 DP_MechSingleAbsPosH32H; //H0B_84  旋转负载单圈位置 高32位（编码器单位）
    Uint16 DP_MechSAbsPos_DemandUintL;    //H0B_85  旋转负载单圈位置（指令单位）
    Uint16 DP_MechSAbsPos_DemandUintH;    //H0B_86  旋转负载单圈位置（指令单位）
    Uint16 DP_Rsvd87;            //H0B_87
    Uint16 DP_Rsvd88;            //H0B_88
    Uint16 DP_Rsvd89;            //H0B_89     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_80*/	0,				0x8000, 			0x7FFF, 			DISP_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/*H0B_81*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10,
/*H0B_82*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H0B_83*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10,
/*H0B_84*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H0B_85*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10,
/*H0B_86*/  0,              0,                  0xFFFF,             DISP_WRT|  TWO_WORD|   DISP_10| HIGH_WORD,
/*H0B_87*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_88*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_89*/  0,              0,                  65535,              RSVD_WRT,


/*  Uint16 DP_Rsvd90;            //H0B_90
    Uint16 DP_Rsvd91;            //H0B_91
    Uint16 DP_Rsvd92;            //H0B_92
    Uint16 DP_Rsvd93;            //H0B_93
    Uint16 DP_MainLoop_PSTMax;   //H0B_94 4秒内主循环调度时间最大值        内部参数后期考虑删除
    Uint16 DP_MainLoop_PRTMax;   //H0B_95 4秒内主循环运行时间最大值        内部参数后期考虑删除
    Uint16 ToqInt_PRTime;        //H0B_96 4秒内电流环中断运行时间最大值    内部参数后期考虑删除
    Uint16 PosInt_PRTime;        //H0B_97 4秒内位置环中断运行时间最大值    内部参数后期考虑删除
    Uint16 DP_ZPosErrMax;        //H0B_98 增量编码器Z位置偏差最值          内部参数后期考虑删除
    Uint16 DP_ZPosErr;           //H0B_99 增量编码器Z位置偏差              内部参数后期考虑删除    */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0B_90*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_91*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_92*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_93*/  0,              0,                  65535,              RSVD_WRT,
/*H0B_94*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_95*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_96*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_97*/  0,              0,                  65535,              DISP_WRT|    DISP_5,
/*H0B_98*/  0,              0,                  65535,              DISP_WRT|    DISP_5|  INT_SIGN,
/*H0B_99*/  0,              0,                  65535,              DISP_WRT|    DISP_5|  INT_SIGN,

//----------------------------辅助功能  H0DLEN = 25 ------------------------------------------------------- 
/*  Uint16 FA_SoftRst;                  //H0D_00 软件复位操作
    Uint16 FA_DefaultRst;               //H0D_01 故障复位操作
    Uint16 FA_OffLnInrtMod;             //H0D_02 惯量辨识使能
    Uint16 OEM_ResetABSTheta;           //H0D_03 绝对编码器初始角辨识
    Uint16 MT_OperAbsROM;               //H0D_04 编码器ROM区读写
    Uint16 FA_EmergencyStop;            //H0D_05 紧急停车(仿紧急停止按钮功能)
    Uint16 FA_TorqPiTune;               //H0D_06 转矩PI自调谐
    Uint16 FA_FricIdentify;             //H0D_07 库仑摩擦力辨识
    Uint16 FA_Rsvd08;                   //H0D_08 
    Uint16 FA_AutoTune;                 //H0D_09 一键式调整使能                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0D_00*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H0D_01*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H0D_02*/  0,              0,                  65,                  ANY_WRT|    DISP_2,
/*H0D_03*/  0,              0,                  2,                  POSD_WRT|    DISP_1|OEMPROTECT,
/*H0D_04*/  0,              0,                  3,                  POSD_WRT|    DISP_1,
/*H0D_05*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H0D_06*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H0D_07*/  0,              0,                  1,                  RSVD_WRT,
/*H0D_08*/  0,              0,                  1,                  RSVD_WRT,
/*H0D_09*/  0,              0,                  1,                   ANY_WRT|    DISP_1,

/*  Uint16 FA_AIAdjustMode;             //H0D_10 AI自动零点偏移调整
    Uint16 FA_Jog;                      //H0D_11 JOG使能
    Uint16 FA_UVAdjustRatio;            //H0D_12 UV相电流平衡校正
    Uint16 FA_RiseTime1;                //H0D_13 转矩PI自调谐,阶跃响应上升时间   单位：1个电流环周期
    Uint16 FA_StableError;              //H0D_14 转矩PI自调谐,阶跃响应稳态误差  0.1%
    Uint16 FA_OverShot;                 //H0D_15 转矩PI自调谐,阶跃响应超调量  0.1%
    Uint16 FA_RiseTime2;                //H0D_16 转矩PI自调谐,阶跃响应上升时间   单位：1个电流环周期
    Uint16 FA_DiDoSimulateEnable;       //H0D_17 DIDO仿真模式下强行输出开关
    Uint16 FA_DiSimulateSet;            //H0D_18 DI仿真模式下强行输出给定
    Uint16 FA_DoSimulateSet;            //H0D_19 Do仿真模式下强行输出给定              */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0D_10*/  0,              0,                  65535,              RSVD_WRT,
#else
/*H0D_10*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
#endif
/*H0D_11*/  0,              0,                  JOG_SPD_MAX,         ANY_WRT| RLAT_LMTH|    DISP_4,
/*H0D_12*/  0,              0,                  1,                  POSD_WRT|    DISP_1|  NEXT_ACT,
/*H0D_13*/  1,              1,                  3,                  POSD_WRT|    DISP_2,
/*H0D_14*/  10,             10,                 50,                 POSD_WRT|    DISP_2|   ONE_DOT,
/*H0D_15*/  10,             0,                  50,                 POSD_WRT|    DISP_2|   ONE_DOT,
/*H0D_16*/  6,              5,                  10,                 POSD_WRT|    DISP_2,
/*H0D_17*/  0,              0,                  4,                   ANY_WRT|    DISP_1,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H0D_18*/  0x01BF,         0,                  0x01BF,              ANY_WRT|    HNUM_TYP|  DISP_3,
/*H0D_19*/  0,              0,                  0x0007,              ANY_WRT|    HNUM_TYP|  DISP_1,
#else
/*H0D_18*/  0x01FF,         0,                  0x01FF,              ANY_WRT|    HNUM_TYP|  DISP_3,
/*H0D_19*/  0,              0,                  0x001F,              ANY_WRT|    HNUM_TYP|  DISP_2,
#endif

/*  Uint16 FA_AbsEncRst;                //H0D_20 绝对编码器复位操作
    Uint16 FA_NKAbsEncRdErrStatus;      //H0D_21 读尼康编码器错误状态位
    Uint16 FA_DisTest;                  //H0D_22 显示测试
    Uint16 FA_Rsvd23;                   //H0D_23
    Uint16 FA_Rsvd24;                   //H0D_24       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H0D_20*/  0,              0,                  2,                  POSD_WRT|    DISP_1,
/*H0D_21*/  0,              0,                  1,                  POSD_WRT|    DISP_1,
/*H0D_22*/  0,              0,                  1,                   ANY_WRT|    DISP_1,
/*H0D_23*/  0,              0,                  65535,              RSVD_WRT,
/*H0D_24*/  0,              0,                  65535,              RSVD_WRT, 

//--------------易用性参数组   H2FLEN = 70 -------------------------------------------------------
/*  Uint16 GUIWorkMode;                 // H2F_00 后台工作模式  00:正常模式，51：后台工作模式
    Uint16 GUIModeSelet;                // H2F_01 后台模式选择
    Uint16 GUICmdSource;                // H2F_02 后台指令来源  (删掉) 
    Uint16 H2F_Rsvd03;		            // H2F_03 
    Uint16 H2F_Rsvd04;		            // H2F_04 保留
    Uint16 H2F_AutoTuneStats;		    // H2F_05 给上位机的状态信息
    Uint16 H2F_Rsvd06;		            // H2F_06 保留
    Uint16 H2F_Rsvd07;                  // H2F_07
    Uint16 CurveServoON;                // H2F_08 内部使能
    Uint16 JogSpeedRef;	                // H2F_09 JOG定位运行速度          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_00*/  0,              0,                     100,              ANY_WRT|  DISP_5,
/* H2F_01*/  0,              0,                       2,              ANY_WRT|  DISP_1,
/* H2F_02*/  0,              0,                   65535,              ANY_WRT|  DISP_5,
/* H2F_03*/  0,              1,                   65535,              RSVD_WRT,
/* H2F_04*/  0,              0,                   65535,              RSVD_WRT,
/* H2F_05*/  0,              0,                   65535,              ANY_WRT|  DISP_5,
/* H2F_06*/  0,              0,                   65535,              RSVD_WRT,
/* H2F_07*/  0,              0,                   65535,              ANY_WRT|  DISP_1,
/* H2F_08*/  0,              0,                       1,              ANY_WRT|  DISP_1,
/* H2F_09*/ 60,              0,                    3000,              ANY_WRT|  DISP_4,

/*  Uint16 JogRiseDownTime;             // H2F_10 JOG定位运行加减速
    Uint16 JogDir;                      // H2F_11 JOG定位运行方向
    Uint16 CurveSel;                    // H2F_12 定位试运行选择 0：JOG，1：定位试运行
    Uint16 CurvePNlimitSet;             // H2F_13 0:不设定，1：极限位置已设定
    Uint16 CurveCoordCCWLow;            // H2F_14 定位坐标1低位
    Uint16 CurveCoordCCWHigh;           // H2F_15 定位坐标2高位
    Uint16 CurveCoordCWLow;             // H2F_16 定位坐标1低位
    Uint16 CurveCoordCWHigh;            // H2F_17 定位坐标2高位
    Uint16 CurvePulseSendLow;           // H2F_18 已发送脉冲数低位
    Uint16 CurvePulseSendHigh;          // H2F_19 已发送脉冲数高位       */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_10*/ 50,              1,                  5000 ,              ANY_WRT|  DISP_4,
/* H2F_11*/  0,              0,                      2,              ANY_WRT|  DISP_1,
/* H2F_12*/  0,              0,                      1,              ANY_WRT|  DISP_1,
/* H2F_13*/  0,              0,                      2,              ANY_WRT|  DISP_1,
/* H2F_14*/  0,              0,                  0x0000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/* H2F_15*/  0,              0xC000,             0x4000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/* H2F_16*/  0,              0,                  0x0000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/* H2F_17*/  0,              0xC000,             0x4000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/* H2F_18*/  0,              0,                  0x0000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/* H2F_19*/  0,              0xC000,             0x4000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,

/*  Uint16 CurveMoveDisSetLow;          // H2F_20 运行距离设定低位
    Uint16 CurveMoveDisSetHigh;         // H2F_21 运行距离设定高位
    Uint16 CurveRunMode;                // H2F_22 运行模式 0:单次，1：连续
    Uint16 CurveRunDirSet;              // H2F_23 运行方向设定 0：不运行，1：正转运行，2：反转运行 3：暂停
    Uint16 WaitTime;                    // H2F_24 等待时间
    Uint16 CurvePertSpeedMax;           // H2F_25 允许运行最大速度
    Uint16 CurvePertLoadRatio;		    // H2F_26 允许负载率
    Uint16 CurvePertCycle;              // H2F_27 允许最大圈数
    Uint16 CurveWarne;                  // H2F_28 伺服运行状态及报警标志 
    Uint16 CurveClrPulseSend;           // H2F_29 清除发送脉冲           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_20*/  40000,          0,                  0x0000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN,
/* H2F_21*/  0,              0xC000,             0x4000,             ANY_WRT|  TWO_WORD|   DISP_11|  INT_SIGN| HIGH_WORD,
/* H2F_22*/  0,              0,                      1,              ANY_WRT|  DISP_1,
/* H2F_23*/  0,              0,                      2,              ANY_WRT|  DISP_1,
/* H2F_24*/  500,             0,                   5000,              ANY_WRT|  DISP_4,
/* H2F_25*/  1000,            0,                   6000,             ANY_WRT|  DISP_4,
/* H2F_26*/  1500,            0,                   4000,             ANY_WRT|  DISP_4| ONE_DOT,
/* H2F_27*/  100,             1,                   1050,            ANY_WRT|  DISP_4| ONE_DOT ,
/* H2F_28*/  0,              0,                  65535,              ANY_WRT|  DISP_5,
/* H2F_29*/  0,              0,                      1,              ANY_WRT|  DISP_1,

/*  Uint16 CurveMoveSpeedMax;           // H2F_30 定位运行速度
    Uint16 CurveMoveRiseDownTime;       // H2F_31 定位运行加减速
    Uint16 H2F_LctCondition;            // H2F_32 定位约束条件
    Uint16 H2F_Rsvd33;                  // H2F_33
    Uint16 GR_DataReady;                // H2F_34 数据已准备好次数
    Uint16 H2F_Rsvd35;                  // H2F_35 
    Uint16 GR_PosLagTime;               // H2F_36 定位时间
    Uint16 GR_oscillateLevel;           // H2F_37 振动等级
    Uint16 GR_LoadRatioAv;              // H2F_38 平均负载率
    Uint16 GR_OvershootPulse;           // H2F_39 过冲脉冲数           */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_30*/ 400,              0,                  3000,              ANY_WRT|  DISP_4,
/* H2F_31*/ 100,              0,                  5000 ,             ANY_WRT|  DISP_4,
/* H2F_32*/  0,              0,                  65535,              ANY_WRT|  DISP_1,
/* H2F_33*/  0,              0,                  65535,              ANY_WRT|  DISP_4,
/* H2F_34*/  0,              0,                  60000,              DISP_WRT|  DISP_5,
/* H2F_35*/  0,              0,                  60000,              RSVD_WRT,
/* H2F_36*/  0,              0,                  60000,              DISP_WRT|  DISP_4,
/* H2F_37*/  0,              0,                   1000,              DISP_WRT|  DISP_4  | ONE_DOT,
/* H2F_38*/  0,              0,                   3000,              DISP_WRT|  DISP_4  | ONE_DOT,
/* H2F_39*/  0,        (Uint16)-32768,            32767,             DISP_WRT|  DISP_5  | INT_SIGN,

/*  Uint16  H2F_Rsvd40;                 // H2F_40 重力负载
    Uint16  H2F_Rsvd41;                 // H2F_41 正向摩擦力
    Uint16  H2F_Rsvd42;                 // H2F_42 负向摩擦力
    Uint16  H2F_Rsvd43;                 // H2F_43 惯量辨识次数
    Uint16  H2F_Rsvd44;                 // H2F_44
    Uint16  H2F_Rsvd45;                 // H2F_45 
    Uint16  H2F_Rsvd46;                 // H2F_46
    Uint16  H2F_Rsvd47;                 // H2F_47
    Uint16  RatioNow;                   // H2F_48 在线惯量辨识结果
    Uint16  OnLineInertiaRatio;         // H2F_49 离线惯量辨识结果  */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_40*/  0,              0,                  3000,              DISP_WRT|  DISP_4  | ONE_DOT,
/* H2F_41*/  0,              0,                  3000,              DISP_WRT|  DISP_4  | ONE_DOT,
/* H2F_42*/  0,              0,                  3000,              DISP_WRT|  DISP_4  | ONE_DOT,
/* H2F_43*/  0,              0,                  65535,             DISP_WRT|  DISP_5,
/* H2F_44*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_45*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_46*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_47*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_48*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_49*/  100,            0,                  12000,              DISP_WRT|    DISP_5|   TWO_DOT,


/*  Uint16 ServoRunState;               // H2F_50 伺服运行状态 (删掉 )
    Uint16 FS_SpdLoopFreq;              // H2F_51 速度环调度频率, 将是频率特性分析速度反馈的采样频率
    Uint16 FS_SpdCmdAmpltd;             // H2F_52 速度指令振幅值
    Uint16 FS_SpdCmdOffset;             // H2F_53 速度指令偏置值
    Uint16 FS_AnalyzeStart;             // H2F_54 开始发送频率特性速度指令
    Uint16 FS_EnableSend;               // H2F_55 第3位为1表示速度反馈数据无效, 低两位为11表示允许发送速度反馈数据 	
    Uint16 FS_Mode;                    // H2F_56 扫频模式选择
    Uint16 FS_IqTurbAmpltd;            // H2F_57 开环扫频电流激励幅值
    Uint16 FS_SpdRefAmpltd;            // H2F_58 开环扫频速度指令幅值
    Uint16 FS_TorqSat;                 // H2F_59  报警标识     */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_50*/  0,              0,                  5,                  DISP_WRT| ZERO_DOT| UN_SIGN,
/* H2F_51*/  8000,           1,                  20000,              DISP_WRT| ZERO_DOT| UN_SIGN,
/* H2F_52*/  10,              0,                  1000,              ANY_WRT| ZERO_DOT| INT_SIGN,
/* H2F_53*/  200,            (Uint16)-1000,      1000,               ANY_WRT| ZERO_DOT| INT_SIGN,
/* H2F_54*/  0,              0,                  1,                  ANY_WRT| ZERO_DOT| UN_SIGN,
/* H2F_55*/  0,              0,                  7,                  ANY_WRT| ZERO_DOT| UN_SIGN,
/* H2F_56*/  1,              0,                  2,                 ANY_WRT| ZERO_DOT| UN_SIGN,
/* H2F_57*/  20,              0,                  150,              ANY_WRT| ZERO_DOT| UN_SIGN,
/* H2F_58*/  0,              0,                  65535,              DISP_WRT| ONE_DOT| UN_SIGN,
/* H2F_59*/  0,              0,                  65535,              ANY_WRT| ZERO_DOT| UN_SIGN,


/*  Uint16  H2F_FuncSwitch;             // H2F_60 新旧功能切换选择   
    Uint16  H2F_HighLvlErrCode;         // H2F_61 最高级故障
    Uint16  H2F_FSAState;                 // H2F_62
    Uint16  H2F_Rsvd63;                 // H2F_63
    Uint16  H2F_Rsvd64;                 // H2F_64
    Uint16  H2F_Rsvd65;                 // H2F_65
    Uint16  H2F_Rsvd66;                 // H2F_66
    Uint16  H2F_Rsvd67;                 // H2F_67
    Uint16  H2F_Rsvd68;                 // H2F_68
    Uint16  H2F_Rsvd69;                 // H2F_69                      */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/* H2F_60*/  7,              0,                  65535,              DISP_WRT| DISP_5,
/* H2F_61*/  0,              0,                  65535,              DISP_WRT| DISP_5,
/* H2F_62*/  0,              0,                  65535,              DISP_WRT| DISP_5,
/* H2F_63*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_64*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_65*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_66*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_67*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_68*/  0,              0,                  65535,              RSVD_WRT,
/* H2F_69*/  0,              0,                  65535,              RSVD_WRT,
    

//----------------------------通讯读取伺服状态参数  H30LEN = 10 -------------------------------------------------------
/*  Uint16 CR_SVST;              //H30_00 通信读取伺服状态
    Uint16 CR_FunOut;            //H30_01 通信读取FunOut Bit0-Bit15
    Uint16 CR_FunOutH;           //H30_02 通信读取FunOut Bit16-Bit31
    Uint16 CR_DeltaPulse;        //H30_03 通信读取输入脉冲指令采样值
    Uint16 H3004_Rsvd;           //H30_04 预留
    Uint16 H3005_Rsvd;           //H30_05 预留
    Uint16 H3006_Rsvd;           //H30_06 预留
    Uint16 H3007_Rsvd;           //H30_07 预留
    Uint16 H3008_Rsvd;           //H30_08 预留
    Uint16 H3009_Rsvd;           //H30_09 预留                   */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H30_00*/  0,              0,                  65535,              DISP_WRT,
/*H30_01*/  0,              0,                  65535,              DISP_WRT,
/*H30_02*/  0,              0,                  65535,              DISP_WRT,
/*H30_03*/  0,              0,                  65535,              DISP_WRT,
/*H30_04*/  0,              0,                  65535,              RSVD_WRT,
/*H30_05*/  0,              0,                  65535,              RSVD_WRT,
/*H30_06*/  0,              0,                  65535,              RSVD_WRT,
/*H30_07*/  0,              0,                  65535,              RSVD_WRT,
/*H30_08*/  0,              0,                  65535,              RSVD_WRT,
/*H30_09*/  0,              0,                  65535,              RSVD_WRT,

//----------------------------通信实现运动控制  H31LEN = 30 -------------------------------------------------------
/*   Uint16 CC_VDILevel;         //H31_00  VDI虚拟电平
     Uint16 H3101_Rsvd;          //H31_01  预留
     Uint16 H3102_Rsvd;          //H31_02  预留
     Uint16 H3103_Rsvd;          //H31_03  预留
     Uint16 CC_CommCtrlDO;       //H31_04  通信给定DO输出状态
     Uint16 H3105_Rsvd;          //H31_05  通信给定AO1输出电压
     Uint16 H3106_Rsvd;          //H31_06  通信给定AO2输出电压
     Uint16 CC_CommSendPosL;     //H31_07  通信给定位置增量L
     Uint16 CC_CommSendPosH;     //H31_08  通信给定位置增量H
     Uint16 CC_CommSendSpeedL;   //H31_09  通信给定速度L            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H31_00*/  0,              0,                  65535,               ANY_WRT,
/*H31_01*/  0,              0,                  65535,              RSVD_WRT,
/*H31_02*/  0,              0,                  65535,              RSVD_WRT,
/*H31_03*/  0,              0,                  65535,              RSVD_WRT,
#if  ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
/*H31_04*/  0,              0,                  7,                  ANY_WRT,
#else
/*H31_04*/  0,              0,                  31,                  ANY_WRT,
#endif
/*H31_05*/  0,              0,                  65535,              RSVD_WRT,
/*H31_06*/  0,              0,                  65535,              RSVD_WRT,
/*H31_07*/  0,              0,                  65535,              RSVD_WRT,
/*H31_08*/  0,              0,                  65535,              RSVD_WRT,
/*H31_09*/  0,              0xABC0,             0x5440,              ANY_WRT| TWO_WORD| INT_SIGN,

/*   Uint16 CC_CommSendSpeedH;   //H31_10  通信给定速度H
     Uint16 CC_CommSendTorqL;    //H31_11  转矩指令L
     Uint16 CC_CommSendTorqH;    //H31_12  转矩指令H
     Uint16 CC_MoniterRT;        //H31_13  监测CAN发送位置处理实时
     Uint16 CC_MoniterRT2;       //H31_14  监测CAN发送位置次数
     Uint16 CC_CommPosLmtSpd;    //H31_15  通信给定位置增量时，电机最大运行速度
     Uint16 H3116_Rsvd;          //H31_16  预留
     Uint16 H3117_Rsvd;          //H31_17  预留
     Uint16 H3118_Rsvd;          //H31_18  预留
     Uint16 H3119_Rsvd;          //H31_19  预留                          */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H31_10*/  0,              0xFF76,             0x0089,              ANY_WRT| TWO_WORD| INT_SIGN| HIGH_WORD,
/*H31_11*/  0,              0x7960,             0x86A0,              ANY_WRT| TWO_WORD| INT_SIGN,
/*H31_12*/  0,              0xFFFE,             0x0001,              ANY_WRT| TWO_WORD| INT_SIGN| HIGH_WORD,
/*H31_13*/  0,              0,                  65535,              RSVD_WRT,
/*H31_14*/  0,              0,                  65535,              RSVD_WRT,
/*H31_15*/  0,              0,                  65535,              RSVD_WRT,
/*H31_16*/  0,              0,                  65535,              RSVD_WRT,
/*H31_17*/  0,              0,                  65535,              RSVD_WRT,
/*H31_18*/  0,              0,                  65535,              RSVD_WRT,
/*H31_19*/  0,              0,                  65535,              RSVD_WRT,

/*   Uint16 H3120_Rsvd;          //H31_20  预留
     Uint16 H3121_Rsvd;          //H31_21  预留
     Uint16 H3122_Rsvd;          //H31_22  预留
     Uint16 H3123_Rsvd;          //H31_23  预留
     Uint16 H3124_Rsvd;          //H31_24  预留
     Uint16 H3125_Rsvd;          //H31_25  预留
     Uint16 H3126_Rsvd;          //H31_26  预留
     Uint16 H3127_Rsvd;          //H31_27  预留
     Uint16 H3128_Rsvd;          //H31_28  预留
     Uint16 H3129_Rsvd;          //H31_29  预留                         */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H31_20*/  0,              0,                  65535,              RSVD_WRT,
/*H31_21*/  0,              0,                  65535,              RSVD_WRT,
/*H31_22*/  0,              0,                  65535,              RSVD_WRT,
/*H31_23*/  0,              0,                  65535,              RSVD_WRT,
/*H31_24*/  0,              0,                  65535,              RSVD_WRT,
/*H31_25*/  0,              0,                  65535,              RSVD_WRT,
/*H31_26*/  0,              0,                  65535,              RSVD_WRT,
/*H31_27*/  0,              0,                  65535,              RSVD_WRT,
/*H31_28*/  0,              0,                  65535,              RSVD_WRT,
/*H31_29*/  0,              0,                  65535,              RSVD_WRT,


//----------------------------示波器  H32LEN = 23 -------------------------------------------------------
/*  Uint16 OS_UartStatus;          //H32_00  串口状态
    Uint16 OS_SeriesTxCtrl;        //H32_01  连续发送数据控制
    Uint16 OS_SectionCtrlA;        //H32_02  示波器缓冲区0-15扇区读写控制 
    Uint16 OS_SectionCtrlB;        //H32_03  示波器缓冲区16-31扇区读写控制
    Uint16 OS_SectionCtrlC;        //H32_04  示波器缓冲区32-47扇区读写控制
    Uint16 OS_SectionCtrlD;        //H32_05  示波器缓冲区48-63扇区读写控制
    Uint16 H3206_Rsvd;             //H32_06  预留
    Uint16 OS_Part;             //H32_07  多次采样当前段数
    Uint16 OS_SampleLen;             //H32_08  后台扫频采样数据长度
    Uint16 OS_SampleClk;           //H32_09  示波器采样时钟                             */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H32_00*/  0,              0,                  65535,               ANY_WRT,
/*H32_01*/  0,              0,                  65535,               ANY_WRT,
/*H32_02*/  0,              0,                  65535,               ANY_WRT,
/*H32_03*/  0,              0,                  65535,               ANY_WRT,
/*H32_04*/  0,              0,                  65535,               ANY_WRT,
/*H32_05*/  0,              0,                  65535,               ANY_WRT,
/*H32_06*/  0,              0,                  65535,              RSVD_WRT,
/*H32_07*/  0,              0,                  10,              ANY_WRT,
/*H32_08*/  1,              0,                  2,               ANY_WRT,
/*H32_09*/  0,              0,                  65535,               ANY_WRT,

/*  Uint16 OS_Mode;                //H32_10  示波器模式
    Uint16 OS_SampleClkPrescaler;  //H32_11  示波器采样时钟分频系数
    Uint16 OS_CHSel12;             //H32_12  示波器1/2通道变量选择
    Uint16 OS_CHSel34;             //H32_13  示波器3/4通道变量选择
    Uint16 OS_TrigEdge_A;          //H32_14  示波器触发沿设置A
    Uint16 OS_TrigLevel_A;         //H32_15  示波器触发水平A
    Uint16 OS_TrigEdge_B;          //H32_16  示波器触发沿设置B
    Uint16 OS_TrigLevel_B;         //H32_17  示波器触发水平B
    Uint16 OS_TrigSetup;           //H32_18  示波器触发设定
    Uint16 OS_TrigPosSetup;        //H32_19  示波器触发位置设定            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H32_10*/  0,              0,                  65535,               ANY_WRT,
/*H32_11*/  1,              1,                  65535,               ANY_WRT,
/*H32_12*/  0x8081,         0,                  65535,               ANY_WRT,
/*H32_13*/  0x8283,         0,                  65535,               ANY_WRT,
/*H32_14*/  0x100,          0,                  65535,               ANY_WRT,
/*H32_15*/  0x20A,          0,                  65535,               ANY_WRT,
/*H32_16*/  0,              0,                  65535,               ANY_WRT,
/*H32_17*/  0,              0,                  65535,               ANY_WRT,
/*H32_18*/  0,              0,                  65535,               ANY_WRT,
/*H32_19*/  10,             0,                  65535,               ANY_WRT,

/*  Uint16 OS_SampleEnable;        //H32_20  示波器采样使能
    Uint16 OS_SampleStatus;        //H32_21  示波器采样状态
    Uint16 OS_TrigPosOfBuff;       //H32_22  示波器缓冲区的触发位置
    Uint16 OS_CommDicnctStop;      //H32_23  Can通信中断，紧急停机 ，等效于H0D05            */
/*默认属性：ANY_WRT| YNUM_TYP| DRCT_LMTL| DRCT_LMTH| ONE_WORD| ZERO_DOT| UN_SIGN| INST_ACT| LOW_WORD|UN_OEMPROTECT*/
/*注释      默认值          下限                上限                属性*/
/*H32_20*/  0,              0,                  65535,               ANY_WRT,
/*H32_21*/  0,              0,                  65535,               ANY_WRT,
/*H32_22*/  0,              0,                  65535,               ANY_WRT,
/*H32_23*/  0,              0,                  65535,               ANY_WRT,
};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 



/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2. 
********************************************************************************/
Uint8 LimitCheck_0neWord(Uint16 FunCodeDeftIndex, Uint16 DataInput)
{
    Uint16 LmtTemp = 0;

    //----------取得下限值                    
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.LowerLmt == 0)  
    {
        //限值与功能码无关联, 属性表中存储的是限值                                                 //
        LmtTemp = FunCodeDeft[FunCodeDeftIndex].LowerLmt;                  
    }    
    else
    {
        //限值与功能码关联, 属性表中存储的是功能码序号
        LmtTemp = FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex].LowerLmt];
    }
    
    //----------根据符号判断是否超出下限
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.Sign == 0)    
    {
        //无符号
        if((Uint16)DataInput < (Uint16)LmtTemp) return 0x01;
    }
    else
    {
        //有符号
        if((int16)DataInput < (int16)LmtTemp) return 0x01;
    }
    
    //----------取得上限值                    
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.UpperLmt == 0)  
    {
        //限值与功能码无关联, 属性表中存储的是限值                                                 //
        LmtTemp = FunCodeDeft[FunCodeDeftIndex].UpperLmt;                  
    }    
    else
    {
        //限值与功能码关联, 属性表中存储的是功能码序号
        LmtTemp = FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex].UpperLmt];
    }
    
    //----------根据判断符号判断是否超出上限
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.Sign == 0)    
    {
        //无符号
        if((Uint16)DataInput > (Uint16)LmtTemp) return 0x02;
    }
    else
    {
        //有符号
        if((int16)DataInput > (int16)LmtTemp) return 0x02;
    }

    return 0;
}

/*******************************************************************************
  函数名: Uint8 LimitCheck_TwoWords(Uint16 FunCodeDeftIndex, Uint16 DataInput)
  输入:   无 
  输出:   无 
  子函数: 无
    描述：
    1.
    2.
********************************************************************************/
Uint8 LimitCheck_TwoWords(Uint16 FunCodeDeftIndex, Uint32 DataInput)
{
    Uint32 LmtTemp = 0;
    
    //----------取得下限值                    
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.LowerLmt == 0)  
    {
        //限值与功能码无关联, 属性表中存储的是限值                                                 //
        LmtTemp = A_SHIFT16_PLUS_B(FunCodeDeft[FunCodeDeftIndex+1].LowerLmt,
                                   FunCodeDeft[FunCodeDeftIndex].LowerLmt);                  
    }    
    else
    {
        //限值与功能码关联, 属性表中存储的是功能码序号
        LmtTemp = A_SHIFT16_PLUS_B(FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex+1].LowerLmt],
                                   FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex].LowerLmt]);
    }
    
    //----------根据判断符号判断是否超出下限                            
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.Sign == 0)    
    {
        //无符号
        if((Uint32)DataInput < (Uint32)LmtTemp) return 0x01;
    }
    else
    {
        //有符号
        if((int32)DataInput < (int32)LmtTemp) return 0x01;
    }
    
    //----------取得上限值                    
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.UpperLmt == 0)  
    {
        //限值与功能码无关联, 属性表中存储的是限值 
        LmtTemp = A_SHIFT16_PLUS_B(FunCodeDeft[FunCodeDeftIndex+1].UpperLmt,
                                   FunCodeDeft[FunCodeDeftIndex].UpperLmt);                  
    }    
    else
    {
        //限值与功能码关联, 属性表中存储的是功能码序号
        LmtTemp = A_SHIFT16_PLUS_B(FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex+1].UpperLmt],
                                   FunCodeUnion.all[FunCodeDeft[FunCodeDeftIndex].UpperLmt]);
    }
    
    //----------根据判断符号判断是否超出上限                  
    if(FunCodeDeft[FunCodeDeftIndex].Attrib.bit.Sign == 0)    
    {
        //无符号
        if((Uint32)DataInput > (Uint32)LmtTemp) return 0x02;
    }
    else
    {
        //有符号
        if((int32)DataInput > (int32)LmtTemp) return 0x02;
    } 

    return 0;
} 

/********************************* END OF FILE *********************************/
