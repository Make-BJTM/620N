/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_GlobalVariable.h
 创建人：李浩                创建日期：11.09.26 
 描述： 
     1.FUNC功能模块内的全局变量
     2.
 修改记录：  
     1.xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
*****************************************************************************/

#ifndef FUNC_GLOBALVARIABLE_H
#define FUNC_GLOBALVARIABLE_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif  


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "PUB_GlobalPrototypes.h" 


/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
//加减速时最大参考速度定义, 速度指令加减速、位置插补相关功能使用
#define SPDUPDOWM_MAXREF    1000


/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */


typedef struct{
    Uint32  Son:1;     // bit 0  伺服使能
    Uint32  AlmRst:1;  // bit 1  故障复位
    Uint32  GainSel:1; // bit 2  增益切换选择
    Uint32  CmdSel:1;  // bit 3  运行指令切换 
    Uint32  DirSel:1;  // bit 4  内部指令方向选择
    Uint32  Cmd1:1;    // bit 5~8  CMD1~4 内部指令切换
    Uint32  Cmd2:1;
    Uint32  Cmd3:1;
    Uint32  Cmd4:1;
    Uint32  M1Sel:1;   // bit 9~10 模式切换1，2
    Uint32  M2Sel:1;   
    Uint32  ZClamp:1;  // bit 11  零位固定
    Uint32  Inhibit:1; // bit 12  脉冲禁止
    Uint32  Pot:1;     // bit 13  禁止正向驱动    
    Uint32  Not:1;     // bit 14  禁止反向驱动
    Uint32  Pcl:1;     // bit 15  正转外部转矩限制
    Uint32  Ncl:1;     // bit 16  反转外部转矩限制
    Uint32  JogCmdP:1; // bit 17  正向点动         
    Uint32  JogCmdN:1; // bit 18  反向点动
    Uint32  PosStep:1; // bit 19  位置步进量 
    Uint32  Hx1:1;     // bit20  手轮倍率选择端子1 对应功能21
    Uint32  Hx2:1;     // bit21  手轮倍率选择端子2 
    Uint32  HxEn:1;    // bit22  手轮使能 
    Uint32  GearSel:1; // bit23  电子齿轮切换开关  对应功能24
    Uint32  TOQDirSel:1;  // bit24  转矩指令方向切换
    Uint32  SPDDirSel:1;  // bit25  速度指令方向切换
    Uint32  POSDirSel:1;  // bit26  位置指令方向切换
    Uint32  PosInSen:1;   // bit 27  多段位置使能信号 //zyj
    Uint32  XintFree:1;   // bit 28  中断定长状态解除信号 //zyj
    Uint32  Rsvd29:1;     // bit 29  保留 //LH
    Uint32  OrgNear:1;    // bit 30  原点挡块信号
    Uint32  OrgChuFa:1;   // bit 31  原点挡板使能信号
}STR_DIVARREG_Lw;

/*DI的32位共用体定义*/
typedef union{
    volatile Uint32                all;
    volatile STR_DIVARREG_Lw       bit;
}UNI_DIVARREG_LW;


typedef struct{
    Uint32  XintInhibit:1;  // bit32  中断定长禁止 
    Uint32  EmergencyStop:1;// bit33  刹车 34
    Uint32  ClrPosErr:1;    // bit34  清除位置偏差对应功能码输入为 35
    Uint32  V_SEL:1;        // bit35  速度限制选择 36
    Uint32  PulseInhibit:1; // bit36  脉冲指令禁止 37
    Uint32  Touch1:1;       // bit37  探针1 38
    Uint32  Touch2:1;       // bit38  探针2 39
    Uint32  Rsvd39:1;       // bit39  对应功能码输入为 40
    Uint32  Rsvd40:1;       // bit40  对应功能码输入为 41
    Uint32  Rsvd41:1;       // bit41  对应功能码输入为 42
    Uint32  Rsvd42:1;       // bit42  对应功能码输入为 43
    Uint32  Rsvd43:1;       // bit43  对应功能码输入为 44
    Uint32  Rsvd44:1;       // bit44  对应功能码输入为 45
    Uint32  Rsvd45:1; 	    // bit45  对应功能码输入为 46
    Uint32  Rsvd46:1; 	    // bit46  对应功能码输入为 47
    Uint32  Rsvd47:1; 	    // bit47  对应功能码输入为 48
    Uint32  Rsvd48:1; 	    // bit48  对应功能码输入为 49
    Uint32  Rsvd49:1; 	    // bit49  对应功能码输入为 50
    Uint32  Rsvd50:1; 	    // bit50  对应功能码输入为 51
    Uint32  Rsvd51:1; 	    // bit51  对应功能码输入为 52
    Uint32  Rsvd52:1; 	    // bit52  对应功能码输入为 53
    Uint32  Rsvd53:1; 	    // bit53  对应功能码输入为 54
    Uint32  Rsvd54:1; 	    // bit54  对应功能码输入为 55
    Uint32  Rsvd55:1; 	    // bit55  对应功能码输入为 56
    Uint32  Rsvd56:1;       // bit56  对应功能码输入为 57
    Uint32  Rsvd57:1; 	    // bit57  对应功能码输入为 58
    Uint32  Rsvd58:1; 	    // bit58  对应功能码输入为 59
    Uint32  Rsvd59:1; 	    // bit59  对应功能码输入为 60
    Uint32  Rsvd60:1; 	    // bit60  对应功能码输入为 61
    Uint32  Rsvd61:1; 	    // bit61  对应功能码输入为 62
    Uint32  Rsvd62:1;       // bit62  手轮正交脉冲输入 63
    Uint32  Rsvd63:1;       // bit63  手轮正交脉冲输入 64
}STR_DIVARREG_Hi;

/*DI的32位共用体定义*/
typedef union{
    volatile Uint32                all;
    volatile STR_DIVARREG_Hi       bit;
}UNI_DIVARREG_HI;



typedef struct{
    //Uint16  ToqFlag:1;     //转矩环调度标志位
    Uint16  SpdFlag:1;     //速度环调度标志位
    Uint16  PosFlag:1;     //位置环调度标志位
    Uint16  ZClampInnerPosFlag:1;     //wzg20111018 零速钳位(固定)函数内建位置环调度标志位
    Uint16  FPGAIntErr:1;     //FPGA中断发送故障
    Uint16  rsvd:12;
}STR_FUNC_SCHEDULAR_BIT;

typedef union{
    volatile Uint16               all;
    volatile STR_FUNC_SCHEDULAR_BIT     bit;
}UNI_FUNC_SCHEDULAR_FLAG_REG;

typedef struct{
    Uint16  CarFreq;            //输出 载波频率,Hz
    Uint16  ToqFreq;            //输出 转矩环的调节频率,单位Hz  FUN->MTR
    Uint16  SpdFreq;            //输出 速度环的调节频率,单位Hz  FUN->MTR
    Uint16  PosFreq;            //输出 位置环的调节频率,单位Hz  等价于 STR_FUNC_Gvar.PosLoopFreq

    Uint32  CarPrd;             //输出 载波周期的时间,单位ns
    Uint32  ToqPrd_Q10;         //输出 转矩环周期时间,单位us Q10格式  
    Uint32  SpdPrd_Q10;         //输出 速度环周期时间,单位us Q10格式  
    Uint32  PosPrd_Q10;         //输出 位置环周期时间,单位us Q10格式    等值于STR_FUNC_Gvar.PosLoopPrd_Q10 
    Uint32  HalfPosPrd;         //输出 半个位置环周期时间,单位us Q10格式    等值于STR_FUNC_Gvar.PosLoopPrd_Q10>>10>>1 

    
    Uint16  SpdMsk;
    Uint16  PosMsk;
    Uint16  SpdCnt;
    Uint16  PosCnt;

    Uint16  FpgaSyncModeSt;

	Uint16  Const4KMsk;
    Uint16  AdaptiveFilterMode;
}STR_FUNC_SYSTEM;


/*FUNC_GainSwitching输出变量结构体类型定义*/
typedef struct{
    int32 SpdKf_Q12;       //速度环调节器PDFF前馈系数0-4096  wzg20120329  	 Disable时为4096
	int32 SpdDampingKf_Q12;//速度环调节器Damping系数0-4096    Disable时为0，Max为4096
    int32 ToqRefFilterTc;  //速度环调节器转矩前馈滤波时间单位us (0-655350)

    int32 PosKp;           //位置环调节器比例系数 wzg20111020
    int32 SpdKp;           //速度环调节器比例系数 wzg20111020
    int32 SpdKi_Q10;       //速度环调节器积分系数 wzg20111020
}STR_GAINSWITCHING;

/*FUNC_PosCtrl输出变量结构体类型定义*/
typedef struct{     
    int32   PosRef;                     //wzg20111029 位置调节器输入，给增益切换选择使用 
    int32   PosRefLatch;                //LH20120115  位置调节器前的位置指令锁存信息，供超程监控时的超程方向判断使用  
    int32   PosAmplifErr;               //wzg20111029 经过电子齿轮后的位置偏差，给增益切换选择使用 
    //int32   PosAmplifErrCalibration;               //wzg20111029 经过电子齿轮后的位置偏差，给增益切换选择使用 
    int32   ExPosAmplifErr;             //            外部位置随动偏差
    int32   PosReguOut;                 //wzg20111029 位置调节器输出，给速度控制指令选择使用
    int32   PosFdb;                     //wzg20111029 增量位置反馈值，给速度控制零速钳位(零位固定)内建位置环使用
    int32   KpCoef_Q15;                 //wzg20111029 位置比例控制器的单位转换系数，给速度控制零速钳位(零位固定)内建位置环使用
    int32   FdFwdOut;                   //wzg20120320位置环速度前馈
    int32   Numerator;                //位置环使用的电子齿轮分子 (最大值为500)  
    int32   Denominator;              //位置环使用的电子齿轮分母
//    int32   PosRmned;                 //位置指令按16K计算时的余量
//    int32   PosRef_16K;               //16K位置调节时的位置指令
//
//    Uint32  PosDivSpd_Q10;            //位置环频率和速度环频率的比值
//    Uint32  PosRefNewFlag;             //位置指令更新标志位
    int32   ExInPosErrSum;             //外部偏差值反馈示波器用
    int32   ExCurrentAbsPos;           //外部当前绝对位置，全闭环时用
    int32   ExPosFdb;                  //外部位置反馈
//
    Uint32  ExPosCoefQ7;                  //外部闭环增益系数
    Uint8   HomStats;                   //原点回零状态，传递原点为限位开关时的状态1:正向进行中，2:反向进行中
    Uint8   DovarReg_Coin;              //位置到达Do输出
    Uint8   DovarReg_Near;              //位置接近Do输出
    Uint8   DovarReg_Xintcoin;          //中断定长完成Do输出
    Uint8   DovarReg_OrgOk;             //原点回零OK Do输出
    Uint8   DovarReg_OrgOkElectric;     //电气回零OK DO输出    
	Uint8   ExPosFeedbackFlag;          //外部闭环位置反馈起用标志 
	Uint8   DovarReg_Cmdok;//指令发送完成
}STR_POSCONTROL;

/*FUNC_SpdCtrl输出变量结构体类型定义*/
typedef struct{
    int32   SpdRef;                       //非转矩模式下速度调节器输入，给MTR模块使用
    int32   SpdRefLatch;                  //速度调节器前的速度指令锁存信息，供超程监控时的超程方向判断使用
    int32   SpdAfterDoFlt;                //用做当H0526=0且速度反馈小于10转时，不响应中断定长

    Uint8   DovarReg_TGon;                //电机旋转Do输出
    Uint8   DovarReg_Zero;                //电机零速Do输出
    Uint8   DovarReg_VCmp;                //速度一致Do输出
    Uint8   DovarReg_VArr;                //速度到达Do输出
}STR_SPDCONTROL;

/*FUNC_ToqCtrl输出变量结构体类型定义*/
typedef struct{
    int32   ToqCmd;                   //转矩模式下的转矩指令，给MTR模块使用
    int32   ToqCmdLatch;              //转矩指令锁存信息，供超程监控时的超程方向判断使用
    int32   SpdLmt;                   //转矩模式下的速度限制值,给MTR模块使用(目前版本转矩模式下速度限幅时通过构建伪速度调节器实现)
    Uint8   TorqPiTuneSon;            //电流环PI参数自调谐使能 0:调谐时SOFF 1:调谐时SON

    Uint8   DovarReg_Clt;             //转矩模式下扭矩限制Do输出
    Uint8   DovarReg_Vlt;             //转矩模式下速度限制Do输出
    Uint8   DovarReg_ToqReach;        //转矩模式下转矩到达Do输出
	
    int32   SpdLmtNeg;
    int32   SpdLmtPos;	
}STR_TOQCONTROL;


/*FUNC_DiDo输出变量结构体类型定义*/
typedef struct{
    Uint16  DiPortState;              //DI端口使能状态
    Uint16  DoPortState;              //DO端口使能状态
    Uint16  VirtualDiPortState;       //虚拟DI端口使能状态
    Uint16  VirtualDoPortState;       //虚拟DO端口使能状态
    Uint16  XintPosEn;                // 中断定长功能使能
    Uint16  TouchProbe1En;            //探针1使用外部邋DI为触发信号使能
    Uint16  TouchProbe2En;            //探针2使用外部邋DI为触发信号使能
    Uint16  Son_Flt;                  //DI Son滤波后
}STR_DIDO_OUTPUT;

/*FUNC_SpiDrive输出变量结构体类型定义*/
typedef struct{
    Uint8   InnerKey;
    Uint8   OuterKey;
}STR_SPI_DRIVE_OUTPUT;

/*FUNC_PANEL输出变量结构体类型定义*/
typedef struct{
    Uint8   TxPanelTubeSel;             //发送当前显示的数码管使能
    Uint8   TxPanelDispData;            //发送当前显示的数码管数据
    Uint8   AiAoDiDoUpdate;             //AiAoDiDo功能码更新
    Uint8   ResetFunCode;               //AiAoDiDo功能码更新
    Uint8   ResetOD;                    //402区OD恢复默认值
    Uint8   SaveFunCode;
    Uint8   SaveOD;
    Uint8   CANResetFunCode;
    Uint8   CANReadComOD;//1000系列读取EEPROM数据    
}STR_MANAGE_FUNCCODE_OUTPUT;

/*伺服监控状态标志位的位结构体类型定义*/
typedef struct{   
    /* 伺服运行状态
       0 伺服未准备好状态,此时SVRDY无效
       1 准备好状态  SERVO_OFF
          此时伺服上电或复位时自检正常，等待伺服使能IO信号；
       2 伺服正常运行态  SERVO_ON
          伺服上电或复位自检正常，收到伺服使能IO/Commu信号，已进入正常运行阶段；
       3 伺服故障态   
       伺服运行过程中出现故障，伺服停止运行；
    */
    Uint32  ServoRunStatus:2;                //LH 等价于原来的 SVSTFlag.bit.SERVORUNSTATUS:2;
    Uint32  RunMod:3;                        // /* 1 转矩模式;2 速度模式;4 位置模式  */

    Uint32  BrakeEn:1;                       //当Do配置抱闸功能，从而抱闸使能 DiDoConfigFlag.bit.BlkEnFlg -》gstr_Monitor.BlkFlag.bit.BlkEnFlg 
    Uint32  BrakeUnLock:1;                   //抱闸打开释放，不接收位置/速度/转矩指令标志位    
    Uint32  OTAckForHome:1;                  //供原点回归用的超程响应标志位
    Uint32  OTAckForLockPos:1;                   //供超程处理模式1时位置指令清零用 
    Uint32  OTClamp:1;                       //发生超程时，进入位置模式且位置指令为零，即为超程时零位置停机固定标志位        

    Uint32  ZeroSpdStop:1;                   //LH 等价于原来的 SVSTFlag.bit.ZEROSPDSTOPFLG //零速停机使能标志        
    Uint32  ToqStop:1;                       //LH 等价于原来的 SVSTFlag.bit.TOQSTOPFLG; //急停减速使能标志
    Uint32  PwmStatus:1;                     //LH 等价于原来的 SVSTFlag.bit.PWMSTATUSFLG  //PWM状态标志位
    Uint32  ErrorShow:1;                     //LH 等价于原来的 SVSTFlag.bit.ERROSHOWFLG; //是否显示故障
    Uint32  SpdReguDatClr:1;                 //速度环偏差等参数清零标志位

    Uint32  PosReguDatClr:1;                 //位置环相关参数清零标志位
    Uint32  UVAdjustRatioEn:1;               //UV相电流平衡校正使能
    Uint32  OffLnInertiaModeEn:1;             //惯量辨识使能
    Uint32  TorqPiTuneEn:1;                  //自调谐使能
    Uint32  SpdToqLoopNact:1;                //速度转矩环程序屏蔽标志位

    Uint32  SpdFSAEn:1;                      //速度频谱分析使能
    Uint32  PosJogWork:1;                    //定位试运标志位
    Uint32  SpdSwitchPosOffset:1;            //速度运行模式切换到位置运行模式时的补偿
    Uint32  ToqSwitchPosOffset:1;            //转矩运行模式切换到位置运行模式时的补偿
    Uint32  InnerServoSon:1;                 //内部使能标志

    Uint32  ModSwitchPeriod:1;               //运行模式切换的中间过度阶段标志位
    Uint32  ErrResetExecut:1;                //故障复位已执行标志位
    Uint32  DIJOGEn:2;                       //DI JOG使能 1 正转 2反转
    Uint32  SlopeStop:1;                      //斜坡停机

    Uint32  BrkDealOver:1;                   //抱闸输出处理完成信号(传递给MTR)
    Uint32  ESMState:1;                      //网络状态是否处于同步
}STR_MONITOR_FLAG;

/*伺服监控状态标志位的共用体类型定义*/
typedef union{
    volatile Uint32                 all;
    volatile STR_MONITOR_FLAG       bit;
}UNI_MONITOR_FLAG;

/*伺服监控状态标志位的位结构体类型定义*/
typedef struct{   
    Uint32  CutoffPowerSave:1;          //掉电保存
    Uint32  RbOverLoadWarn:1;           //电阻过载警告
    Uint32  InputRdyFlg:2;              //是否缺相  0刚上电输入未准备好  1 输入准备好  2 输入未准备好
    Uint32  PowOffZeroSpdStopFlag:1;    //掉电零速停机标志位
    Uint32  RelayFlg:1;                 //继电器标志位,1-继电器打开；0-继电器吸合     继电器打开时需要关PWM
    Uint32  BrkOut:1;                   //泄放输出信号
    Uint32  UdcOk:1;                    //母线电压Ok
    Uint32  ServoRdy:1;                 //伺服准备好 
	Uint32  OpenFSAEn:1;                          //开环扫频标志
    Uint32	LagFilterClr:1;				//时滞滤波器
    Uint32  Rsvd:21;                    // 
}STR_MONITOR2_FLAG;

/*伺服监控2状态标志位的共用体类型定义*/
typedef union{
    volatile Uint32                 all;
    volatile STR_MONITOR2_FLAG      bit;
}UNI_MONITOR2_FLAG;

/*伺服监控模块输出的全局变量结构体类型定义*/
typedef struct{
    Uint32  OffLnInrtCmd;        //惯量辨识    
    Uint16  ErrCode;             //当前的故障码
    Uint16  HighLevelErrCode;    //最高级别的故障码
    Uint16  SpdJOGCmd;           //速度JOG命令
    Uint16  ResZIndexcmd;

    Uint8   DovarReg_SRdy;       //伺服Rdy Do输出
    Uint8   DovarReg_Blk;        //抱闸Do输出
    Uint8   DovarReg_Warn;       //伺服警告Do输出
    Uint8   DovarReg_Alm;        //伺服故障Do输出
    Uint8   DovarReg_AlmCode;    //伺服故障编码Do输出
    Uint8   DovarReg_AngIntRdy;  //初始角度辨识完成DO信号

}STR_MONITOR; 

/*FUNC_AI输出变量结构体类型定义*/
typedef struct{
    int32   AI1VoltOut;             //电压线性变换及纠偏后的结果
    int32   AI2VoltOut;             //电压线性变换及纠偏后的结果
//    int32   AI3VoltOut;           //电压线性变换及纠偏后的结果
    Uint8   HighPrecisionAIEn;      //高精度AI使能
}STR_AIOUTPUT;

/*示波器采样对象结构体类型定义*/
typedef struct{
    int32  CH1_Test;                //功能模块示波器测试变量1
    int32  CH2_Test;                //功能模块示波器测试变量2
    int32  CH3_Test;                //功能模块示波器测试变量3
    int32  CH4_Test;                //功能模块示波器测试变量4
    int32  SpdRefOld;               //上次FPGA中断的速度指令
    int32  SpdFdbFlt;               //滤波后的速度反馈
    int32  CurrentAbsPos;           //当前绝对位置
    int32  InputPulseCnt;           //输入指令脉冲计数器
    int32  PosAmpErr;               //位置随动偏差
    int32  InputPulsCntRT;          //实时输入指令脉冲计数器
    int32  Osc_EncMultTurnData;
    int32  Osc_AvrLoad;
    int32  InputTarPosRT;           //实时输入目标位置显示
    int32  InputTarSpd;             //实时输入目标速度显示
    int32  InputTarToq;             //实时输入目标转速显示
    int32  SyncLength;              //SYNC0信号周期 单位12.5ns
    int32  CtrlWord;//控制字
    int32  StatusWord;//状态字
    int32  Osc_DIFunc0;
    int32  Osc_DIFunc1;
    int32  Osc_DIFunc2;
    int32  Osc_DIFunc3;
    int32  Osc_DOFunc0;
    int32  Osc_DOFunc1;
}STR_OSCILLOSCOPE_TARGET;


/*FUNC_RigidityLevelTable输出变量结构体类型定义*/
typedef struct{
    int32 ToqRefFltTc1st;     //第1组转矩指令滤波器
    int32 PosKp1st;           //第1组位置比例增益
    int32 SpdKp1st;           //第1组速度比例增益
    int32 SpdKi1st_Q10;       //第1组速度积分时间常数
    int32 ToqRefFltTc2nd;     //第2组转矩指令滤波器
    int32 PosKp2nd;           //第2组位置比例增益
    int32 SpdKp2nd;           //第2组速度比例增益
    int32 SpdKi2nd_Q10;       //第2组速度积分时间常数
    int32 AutoTuneFlag;       //手动自调整标志位，0~无调整，1~手动自调整
}STR_RIGIDITYLVLTAB;


/*FUNC_TorqPiTune输出变量结构体类型定义*/
typedef struct{
    int8  Step;         //调试步骤
    int32 ToqRef;       //转矩指令
}STR_TORQPITUNE;

/*FUNC_Fft.c输出变量结构体类型定义*/
typedef struct{
   Uint16     OscillateValue;   //获取振动幅
   Uint16     FftCalCnt;		//FFT运算成功次数
   Uint16     OscillateLevel;   //震动判定幅值
   Uint16     CalAfterNotchFlag;  //设置好陷波器后再次进行了一次FFT运算标志
}STR_FFT;

/*FUNC_FricIdentity.c输出变量结构体类型定义*/
typedef struct{
    Uint16     MultiBlockNum;   //获取振动幅
    Uint16     EndIdenFlag;     //摩擦辨识结束标志位
    Uint16     FricIdenCmd;         //摩擦辨识内部使能命令
}STR_FRIC_IDENTIFY;


//⑧ADC采样文件内（电机模块内全局变量）使用变量
//片内AD采样数据结构体,此结构体内的变量直接可以被算法调用
typedef struct{
//    int32  AI1;       //AI1 模拟量给定1 通道7
//    int32  AI2;       //AI2 模拟量给定2 通道8
    int32  Udc_Live;   //DC   母线电压 通道10
    Uint16 IPMT;      //Tempera 温度 通道11
    Uint16 Pwr;       //PWR    通道 12
	Uint16 Ib ;       //刹车电阻电流
}STR_DRIVE_ADC;
/*FUNC_FSASpeedCmd.c输出变量结构体类型定义*/
typedef struct{
   int32     FSAIqCmd;   //获取电流激励指令
   int32     FSASpdCmd;  //获取速度激励指令
}STR_FSACmd;

/*  FUNC模块变量结构体类型定义
 * 1. 运动控制使用一个结构体
 * 2. MTR模块传给FUNC模块变量使用一个结构体
 * 3. 功能较为独立的文件使用一个结构体，如转矩控制和增益切换文件
*/
typedef struct{
    STR_FUNC_SYSTEM         System;                 //FUNC_System 中调度频率和周期变量结构体

    STR_POSCONTROL          PosCtrl;                //FUNC_PosCtrl输出变量结构体

    STR_SPDCONTROL          SpdCtrl;                //FUNC_SpdCtrl输出变量结构体

    STR_GAINSWITCHING       GainSW;                 //FUNC_GainSwitching输出变量结构体

    STR_TOQCONTROL          ToqCtrl;                //FUNC_ToqCtrl输出变量结构体

    STR_DIDO_OUTPUT         DiDoOutput;             //FUNC_DiDo的输出变量结构体 

    STR_SPI_DRIVE_OUTPUT    SpiDrvOutput;           //FUNC_SpiDrive输出变量结构体

    STR_MANAGE_FUNCCODE_OUTPUT     ManageFunCodeOutput;      //FUNC_ManageFunCode输出变量结构体类型定义

    STR_MONITOR             Monitor;                 //FUNC_ServoMonitor输出的需全局功能模块使用的变量

    STR_AIOUTPUT            AI;                      //FUNC_AI输出变量结构体类型定义

    STR_OSCILLOSCOPE_TARGET        OscTarget;        //示波器采样对象结构体变量定义

    //以下下两个变量在主循环里面更新，更新速度慢可在函数任何部分读调用
    UNI_DIVARREG_LW                DivarRegLw;         //DI的共用体变量  //输出 关于DI功能的定义和操作

    UNI_DIVARREG_HI                DivarRegHi;         //DI的共用体变量  //输出 关于DI功能的定义和操作

    //以下下两个变量在软中断里面更新，更新速度快，在主循环读会因中断抢占出现逻辑错误。
    //针对在软中断或FPGA中处理，不会在主循环调度的DI功能可以使用以下变量。
    //在主循环中调度或主循环、中断都调度的DI功能，不要使用以下变量。
    UNI_DIVARREG_LW                SoftIntDivarRegLw;  //DI的共用体变量  //输出 关于DI功能的定义和操作

    UNI_DIVARREG_HI                SoftIntDivarRegHi;  //DI的共用体变量  //输出 关于DI功能的定义和操作

    UNI_MONITOR_FLAG               MonitorFlag;      //FUNC_ServoMointor 监控模块的全局标志位

    UNI_MONITOR2_FLAG              Monitor2Flag;      //FUNC_ServoMointor2 监控模块的全局标志位

    STR_RIGIDITYLVLTAB             RigidityLvlTab;   //刚性表输出

    UNI_FUNC_SCHEDULAR_FLAG_REG    ScheldularFlag;   //调度标志位

    STR_TORQPITUNE                 ToqPiTune;        //电流环参数自调谐

    STR_FFT                        Fft;              // Fft文件输出变量

    STR_DRIVE_ADC                  ADC_Samp;       //AD通道采样

    STR_FRIC_IDENTIFY              FricIdentify;   //摩擦辨识

    Uint16 CanNodeRunStat;              //临时放在这里CAN节点运行状态

    STR_FSACmd                        FsaCmd;         //开环频率特性分析

}STR_FUNC_GLOBALVARIABLE;


//硬件复位时记录寄存器
typedef union{
    Uint16  Dis_16Bits[28];
    Uint32  Reg_32Bits[14];
}UNI_HardFaultInformation;  

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_FUNC_GLOBALVARIABLE    STR_FUNC_Gvar;
extern UNI_HardFaultInformation   HFInfor;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

#ifdef __cplusplus
}
#endif /* extern "C"*/ 

#endif /*end of FUNC_GlobalVariable.h*/

/********************************* END OF FILE *********************************/






