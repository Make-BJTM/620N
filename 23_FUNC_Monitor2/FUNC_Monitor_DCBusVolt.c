/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_DCBusVolt.c       直流母线电压监控相关处理程序                                                 
 创建人：陈培正、王军干、童文邹 创建日期：2008.11.10
 修改人：朱祥华                 修改日期：2011.11.21 
 描述： 母线电压监控相关处理程序
     1.
 修改记录：  
    1. 朱祥华       2011.11.21
       变更内容：母线电压相关的监控控制内容：如软启动、欠压过压监控处理、母线电压泄放处理
                 
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "PUB_Library_Function.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_Filter.h"
#include "FUNC_Monitor_DCBusVolt.h"
#include "FUNC_GPIODriver.h"
#include "FUNC_AdcDriver_ST.h"
#include "FUNC_Monitor_PhaseLack.h"
#include "FUNC_InterfaceProcess.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//暂无

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义*/  
STR_DCBUSVOLTAGEPROCESS STR_DCBusVolt;

STR_BILINEAR_LOWPASS_FILTER  ADCPwrFilter={0};  //AD采样值滤波器

#if POWERDRIVER_TYPE==POWDRV_IS650
STR_BILINEAR_LOWPASS_FILTER  BrkCurrentFilter = BILINEAR_LOWPASS_FILTER_Defaults ;
#endif
/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
#if POWERDRIVER_TYPE==POWDRV_IS650
Uint16  BrkIBMax[9]={107,162,218,280,350,437,538,538,875}; 
#endif

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitDCBusVoltProcess(void);          //母线电压相关初始化程序
void DCBusVoltErr_Monitor(void);          //母线电压监控相关报错
void DcBusVolt_SoftStart(void);           //软启动监控相关处理


//以下三个为母线电压泄放相关函数
void StopUpdateDCBusBrake(void);          //停机更新母线泄放程序，更新功能码之后执行
void INT_16K_DCBusBrkProcess(void);      //中断电流环执行母线电压泄放开通关断程序
void DCBusVoltBrake_Monitor(void);        //1ms监控母线电压泄放,比如是否过载

void CtrlDCBusVoltBrake_Monitor(void);    //控制电源母线电源监控

#if POWERDRIVER_TYPE==POWDRV_IS650
void InitBrkCurrentCheck(void);
void INT_16KBrkCurrentCheck(void);
#endif
void LineVolt_Monitor(void);              //驱动器输出线电压有效值监控
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//以下函数在DCBusVoltErr_Monitor()中调用
//Static_Inline void LowDCBusVolt_Warn_ToqLmt(void);  //欠压警告时报警处理以及转矩限制


/*******************************************************************************
  函数名:  InitDCBusVoltProcess(void)     上电初始化母线电压相关配置
          （务必置于功能码、伺服系统控制参数初始化之后）    ^_^
  输入:    H0104电压等级 H0130母线电压增益调整 H0115至H0117母线电压过压保护、泄放点、欠压点；H0102驱动器型号
  输出:    母线电压控制相关变量包括电压保护、和电压泄放等
  子函数:  无
  描述:    初始化时计算母线电压欠压、缺相、过压、制动电阻泄放相关变量初始化
********************************************************************************/ 
void InitDCBusVoltProcess(void)
{
     //母线电压采样系数220V
    if(FunCodeUnion.code.OEM_VoltClass == 220)      //H01_04  OEM驱动器电压级
    {
        STR_DCBusVolt.UdcOver = FunCodeUnion.code.OEM_UdcOver * 10;// 计算过压点420V
        STR_DCBusVolt.UdcLeak = FunCodeUnion.code.OEM_UdcLeak * 10;// 计算泻放点380V
        STR_DCBusVolt.UdcLow = FunCodeUnion.code.OEM_UdcLow * 10;  // 计算欠压点200V
        
//        STR_DCBusVolt.UdcRate     = 3100;       // 赋母线电压额定值310V
        //根据规格书定义母线电压工作范围,在此范围内可建立母线电压 （三相AC200~240V，+10~-15%）
        STR_DCBusVolt.UdcMin      = 2400-50;   //200V*（1-15%）* sqrt(2)=240V
        STR_DCBusVolt.UdcMax      = 3730+50;   //240V*（1+10%）* sqrt(2)=373V
    }
    else if(FunCodeUnion.code.OEM_VoltClass == 380)
    {   
        STR_DCBusVolt.UdcOver = FunCodeUnion.code.OEM_UdcOver * 10;// 380v过压点760V
        STR_DCBusVolt.UdcLeak = FunCodeUnion.code.OEM_UdcLeak * 10;  // 母线泻放电压为700v
        STR_DCBusVolt.UdcLow = FunCodeUnion.code.OEM_UdcLow * 10;    // 欠压值为 380v
//        STR_DCBusVolt.UdcRate     = 5400;    // 赋母线电压额定值
        //根据规格书定义母线电压工作范围,在此范围内可建立母线电压 （三相AC380~440V，+10~-15%）
        STR_DCBusVolt.UdcMin      = 4560-50;   //380V*（1-15%）* sqrt(2) = 456V
        STR_DCBusVolt.UdcMax      = 6840+50;   //440V*（1+10%）* sqrt(2) = 684V
    } //可继续添加其他电压等级


   //根据驱动器型号选择制动电阻的峰值能量
   switch(FunCodeUnion.code.OEM_ServoSeri) 
   {  //220V驱动器1.5kw及以下均用40W50欧姆的金色电阻，超过3S容易烧坏。
       case 0:         case 1:         case 2:         case 3:
       case 4:         case 5:         case 6:         case 7:
       STR_DCBusVolt.MaxBrkT_RatePower = 3000;//3s
        break;

       //220V驱动器2kw、3kw、5kw均用100W的船形电阻，超过2S容易烧坏。
       case 8:         case 9:        case 10:
       STR_DCBusVolt.MaxBrkT_RatePower = 20000;//20s
      break;

      //380V驱动器1.5kw及以下均用40W金色电阻，超过3S容易烧坏。
       case 10000:     case 10001:     case 10002:
       STR_DCBusVolt.MaxBrkT_RatePower = 3000;//3s
      break;

      //380V驱动器2kw及3kw均用100W船型电阻，超过25S容易烧坏。100w船型
       case 10003:     case 10004:
       STR_DCBusVolt.MaxBrkT_RatePower = 25000;//25s
      break;

      //380V驱动器5kw、6kw及7.5kw均用100W船型电阻，超过20S容易烧坏
       case 10005:     case 10006:     case 10007:
       STR_DCBusVolt.MaxBrkT_RatePower = 20000;//20s
      break;

      default:
      STR_DCBusVolt.MaxBrkT_RatePower = 20000;//20s
      break;
    }

    //电压检测计数清零
    STR_DCBusVolt.SSMonitCnt = 0;
    
    //欠压转矩限制标志位初始化清零
    //STR_MTR_Gvar.GlobalFlag.bit.UdcLow_ToqLmtFlg = 0;
    //初始化时将欠压标志位清零
    STR_DCBusVolt.UnderUdcErrFlg = 0;

    //上电初始化时先使能软启动，并置软启动状态为软启动使能状态
    GPIO_Enable_SoftStart();   //置1  Bit Set   继电器打开接入限流电阻，软启动使能

    STR_DCBusVolt.SoftStartFlag = SS_Enable;  //软启动状态

    STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg = STR_DCBusVolt.SoftStartFlag;

    STR_DCBusVolt.Brk_Status = 0;            //初始化时不进行制动监控

    //上电初始化时电压建立标志位为0
    STR_DCBusVolt.UdcOkFlag = INVALID;   
    //监控周期内累积的制动电阻热量，初始热容量假设为0
    STR_DCBusVolt.Brk_HeatTotal_Q8 = 0;
    //伺服准备好清0
    STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy = INVALID;

    ADCPwrFilter.Ts = 1000;
    ADCPwrFilter.Tc = 10000;   // tc = 10ms
    InitLowPassFilter(&ADCPwrFilter);


    if(FunCodeUnion.code.OEM_VoltClass == 220)      //H01_04  OEM驱动器电压级
    {   
        STR_DCBusVolt.CutoffLvl = 1900;
        STR_DCBusVolt.CtrlUdcCoeff_Q12 = 3750;
    }
    else if(FunCodeUnion.code.OEM_VoltClass == 380)
    {
        STR_DCBusVolt.CutoffLvl = 3500;
        STR_DCBusVolt.CtrlUdcCoeff_Q12 = 7000;    
    }

    STR_DCBusVolt.BrkPrd_Q8  = ((Uint32)1000 << 8) / STR_FUNC_Gvar.System.ToqFreq;         //周期单位为ms Q8 ,所以为1000
}


/*******************************************************************************
  函数名:  StopUpdateDCBusBrake(void)    实时性要求极低，可以主循环中小于1ms更新
         停机更新与母线电压保护相关配置（初始化也更新，务必置于电流环相关和功能码初始化之后）  ^_^
  输入:  H0222至H0227内置或外置制动电阻相关参数
  输出:  电阻大致热容量、电阻每个开通泄放周期产生热量和判断周期内的散发热量计算， 

  子函数:无
  描述:  根据H0225能耗电阻类型设置，计算电阻大致热容量、电阻每个开通泄放周期（8K）产生热量和散发热量计算，
         并判断外置电阻是否小于允许最小电阻值。每个开通周期产生热量为(Udc^2/R)*制动开关周期，每个周期
         散发热量则根据H0224散热系数和（电阻标称功率*制动开关周期）定。开关周期内产生或散发的热量均为Q8格式。
********************************************************************************/ 
void StopUpdateDCBusBrake(void)
{
    Uint32 BrkTemp = 0;

    //BP_RBChoice H0225能耗电阻设置，0为内置，1为外置自然冷，2为外置风冷，3则单靠电容吸收
    switch (FunCodeUnion.code.BP_RBChoice)
    {       //MaxBrkT_RatePower为制动电阻热容量计算常数  目前设定为允许额定功率下运行5000ms
        case 0: //内置电阻热容量等于MaxBrkT_RatePower *电阻标称功率 代表最后单位为W.ms
            STR_DCBusVolt.Brk_HeatCapacity_Q8 = (int64)STR_DCBusVolt.MaxBrkT_RatePower * FunCodeUnion.code.BP_RBPSizeI << 8 ;
            //内置单个制动管开关周期内贡献热量((Udc^2/R_Out)*1000)*BrkPrd_Q8    BrkFreq为泄放调度周期 最后单位为 W.ms
            BrkTemp = (Uint32)FunCodeUnion.code.OEM_UdcLeak * FunCodeUnion.code.OEM_UdcLeak 
                      / FunCodeUnion.code.BP_RBOhmI;
            STR_DCBusVolt.Brk_HeatAddDetal_Q8 = BrkTemp * STR_DCBusVolt.BrkPrd_Q8 ;
            //内置单个制动管开关周期内散热量，默认为30% 最后单位为 W.ms
            STR_DCBusVolt.Brk_HeatSubDetal_Q8 = (Uint32)FunCodeUnion.code.BP_RBPSizeI * FunCodeUnion.code.BP_RBEfficiency
                                       * STR_DCBusVolt.BrkPrd_Q8/100;
        break;
        
        case 1: //外置自然冷电阻MaxBrkT_RatePower为制动电阻热容量计算常数  目前设定为允许额定功率下运行5000ms
            //外置电阻热容量等于MaxBrkT_RatePower *电阻标称功率 代表最后单位为W.ms
            STR_DCBusVolt.Brk_HeatCapacity_Q8 = (int64)STR_DCBusVolt.MaxBrkT_RatePower * FunCodeUnion.code.BP_RBPSizeO << 8 ;
            //外置单个制动管开关周期内贡献热量((Udc^2/R_Out)*1000)*BrkPrd_Q8    BrkFreq为泄放调度周期 最后单位为 W.ms
            BrkTemp = (Uint32)FunCodeUnion.code.OEM_UdcLeak * FunCodeUnion.code.OEM_UdcLeak 
                       / FunCodeUnion.code.BP_RBOhmO;
            STR_DCBusVolt.Brk_HeatAddDetal_Q8 = BrkTemp * STR_DCBusVolt.BrkPrd_Q8 ;
            //外置单个制动管开关周期内散热量，风冷则实际散热系数为功能码散热系数H0224的两倍 最后单位为 W.ms
            STR_DCBusVolt.Brk_HeatSubDetal_Q8 = (Uint32)FunCodeUnion.code.BP_RBPSizeO * FunCodeUnion.code.BP_RBEfficiency
                                       * STR_DCBusVolt.BrkPrd_Q8/100;
            if (FunCodeUnion.code.BP_RBOhmO < FunCodeUnion.code.BP_RBMinOhm)
            { //外置电阻过小则警告
                PostErrMsg(RBTOOSMALLWARN);  //制动电阻阻值过小报警
            } //外置电阻符合要求
            else  
            {
                WarnAutoClr(RBTOOSMALLWARN);  //清除电阻阻值过小报警
            }

        case 2: //外置风冷电阻，MaxBrkT_RatePower为制动电阻热容量计算常数  目前设定为允许额定功率下运行5000ms
            //外置电阻热容量等于MaxBrkT_RatePower *电阻标称功率 代表最后单位为W.ms
            STR_DCBusVolt.Brk_HeatCapacity_Q8 = (int64)STR_DCBusVolt.MaxBrkT_RatePower * FunCodeUnion.code.BP_RBPSizeO << 8 ;
            //外置单个制动管开关周期内贡献热量((Udc^2/R_Out)*1000)*BrkPrd_Q8    BrkFreq为泄放调度周期 最后单位为 W.ms
            BrkTemp = (Uint32)FunCodeUnion.code.OEM_UdcLeak * FunCodeUnion.code.OEM_UdcLeak 
                       / FunCodeUnion.code.BP_RBOhmO;
           STR_DCBusVolt.Brk_HeatAddDetal_Q8 = BrkTemp * STR_DCBusVolt.BrkPrd_Q8 ;
            //外置单个制动管开关周期内散热量，风冷则实际散热系数为功能码散热系数H0224的两倍 最后单位为 W.ms
           STR_DCBusVolt.Brk_HeatSubDetal_Q8 = (Uint32)FunCodeUnion.code.BP_RBPSizeO * FunCodeUnion.code.BP_RBEfficiency * 2
                                      * STR_DCBusVolt.BrkPrd_Q8 /100;
            if (FunCodeUnion.code.BP_RBOhmO < FunCodeUnion.code.BP_RBMinOhm)
            {   //外置电阻过小则警告
                PostErrMsg(RBTOOSMALLWARN);  //制动电阻阻值过小报警
            }   //外置电阻符合要求
            else  
            {
                WarnAutoClr(RBTOOSMALLWARN);  //清除电阻阻值过小报警
            }
        break;

        default: 
        break;
    }
}

/*******************************************************************************
  函数名:  DCBusVoltBrake_Monitor(void)     
           母线电压制动泄放处理程序（1ms主循环更新）  ^_^
  输入:    制动电阻类型选择BP_RBChoice  
           输入状态标志InputRdyFlg
           电压是否建立完毕标志位UdcOkFlag 
           实时母线电压Udc_Live
           H0A00 是否使能缺相报错ER_InPLProtectSel  
           驱动器型号H0102 OEM_ServoSeri
  输出:    制动电阻开通关断状态位STR_DCBusVolt.Brk_Status   制动电阻过载报警位
  子函数:  暂无
  描述:   大致计算母线电压泄放时制动电阻的热累积量是否过载，此处热量计算母线电压值取母线电压泄放点值，
          计算1ms内累积的热量是否超过电阻的热容量值。电阻总热容量值由则认为电阻可在额定功率下运行MaxBrkT_RatePower
          程序中所有热量的单位均为w.ms。。
关于断电处理总结：
①正常接线且H0A00不置2：可进行断电泄放，但不一定够时间断电保存，
②共母线接线： H0A00不置2时上电不能进入伺服RDY，共母线且H0A00置2时可以进入RDY，但只能到欠压点断电保存，不能断电泄放。
③L1C L2C控制电引自母线驱动电接交流且H0A00不置2：驱动电一断即可选择进行断电保存，然后进行断电泄放
④当H0A00置2的情况：任何接线方式均不能断电泄放。
注意：
①要确保掉电记忆有效请将控制电引自母线，驱动电接交流接线，H0A00不置2；
②共母线时请将H0A00置2否则不能进入RDY状态；
********************************************************************************/ 
void DCBusVoltBrake_Monitor(void)
{
    int32 Brk_MonitHeatAdd_Q8 = 0;
    int32 Brk_MonitHeatSub_Q8 = 0;
    static Uint16 BrkCountNum = 0;
    Uint8   BrkStatusTemp = 0;

    static Uint8 RelayDelayCnt = 255;  
    static Uint8 RelayStateLast = 1;

    BrkStatusTemp = STR_DCBusVolt.Brk_Status;

    if ((FunCodeUnion.code.BP_RBChoice == 3) || 
        (STR_DCBusVolt.RbTooSmallWarn == 1) ||
        ((FunCodeUnion.code.BP_RBChoice == 0)&&(FunCodeUnion.code.OEM_ServoSeri<=3)))
    {  //不用制动电阻或者制动电阻选择故障或者伺服未准备好，另400W以下机型无内置泄放电阻，则不允许泄放
        STR_DCBusVolt.Brk_Status = 0;   //制动电阻非正常态，关闭制动管泄放
        return;  //返回不再处理泄放和掉电保存程序
    }    //其余情况 则先置正常泄放监控态
    else 
    {
        BrkStatusTemp = 1;
    }

    //等待继电器断开20ms后，再进行掉电泄放
    if(255 == RelayDelayCnt)
    {
        RelayDelayCnt = 0;
        RelayStateLast = STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg;
    }
     
    if( (0 == RelayStateLast) && (1 == STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg) )
    {
        RelayDelayCnt = 1;
    }
    else if( (0 < RelayDelayCnt) && (1 == RelayStateLast) && (1 == STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg) )
    {
        if(20 > RelayDelayCnt)  RelayDelayCnt ++;
    }
    else
    {
        RelayDelayCnt = 0;
    } 

    RelayStateLast = STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg;

    //servo 断电处理，若输入缺相且母线准备好标志仍为1且PowerOffSave=2掉电保存停止，则进行断电放电处理
    //正常连接且H0A00!=2时，断L1 L2 L3后30ms即检测到InputRdyFlg=INPUT_NRD即开始断电泄放，
    //共母线连接或者H0A00=2时InputRdyFlg一直为INPUT_NRD或者软启动未完成(PowerOffBleedFlg=0)则不会进行掉电泄放处理
    if( (STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_NRD)
        && (STR_DCBusVolt.PowerOffBleedFlg == 1) 
		&&(STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn == 0) && (20 == RelayDelayCnt)
        &&(0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
    {//50V(人体安全电压为直流50V)以上则连续泄放1s,
     //若有一次检测到非三相全缺态则停止泄放,防止未泄放完成立即上电，从InputRdyFlg=0到置1检测需要30ms,检测期间会一直误泄放。
        if(   (STR_FUNC_Gvar.ADC_Samp.Udc_Live >= 500)&&(BrkCountNum < 1800)
            && (STR_Monit_PhaseLack.PL_PinStatus == 1))
        { //软件启动完成可泄放，泄放过程中若检测有一次非三相动力全缺即停止泄放
            BrkCountNum++;
            BrkStatusTemp = 3; //掉电泄放状态
            STR_DCBusVolt.PowerOffBleedFlg = 1;
        }
        else  //放电之后，将母线电压OK标志UDCOKFLAG置无效,
        {
            BrkStatusTemp = 2; //掉电停止泄放状态
            STR_DCBusVolt.PowerOffBleedFlg = 0; //不允许断电泄放
            BrkCountNum = 0;
        }
    }    //正常泄放状态监控，与断电泄放分开    伺服未准备好不进行泄放监控
    else //if (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != NRD)
    {
        BrkCountNum = 0;

        //进行累积热容量计算并进行相关的判断处理
        Brk_MonitHeatAdd_Q8 = (int32)STR_DCBusVolt.Brk_HeatAddDetal_Q8 * STR_DCBusVolt.EnableBrkCnt;    //计算监控1ms内电阻吸收热量
        Brk_MonitHeatSub_Q8 = (int32)STR_DCBusVolt.Brk_HeatSubDetal_Q8 * STR_DCBusVolt.DisableBrkCnt;   //计算监控1ms内电阻散热量
        STR_DCBusVolt.Brk_HeatTotal_Q8 += (Brk_MonitHeatAdd_Q8 - Brk_MonitHeatSub_Q8);

        STR_DCBusVolt.EnableBrkCnt = 0;     //电阻累积热量计数清零，以便下一个1ms监控周期重新计数
        STR_DCBusVolt.DisableBrkCnt = 0;   

        if(STR_DCBusVolt.Brk_HeatTotal_Q8 < 0)
        {//即假设常温下的热量为0
            STR_DCBusVolt.Brk_HeatTotal_Q8 =0;
        }

#if POWERDRIVER_TYPE==POWDRV_IS650
		if(STR_DCBusVolt.BrkOC==1)
		{
		     //刹车电流检测 过流 报Er.921
             BrkStatusTemp = 0;
			 STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn = 1;
		}
        else if(STR_DCBusVolt.Brk_HeatTotal_Q8 > STR_DCBusVolt.Brk_HeatCapacity_Q8)
    #else
        if(STR_DCBusVolt.Brk_HeatTotal_Q8 > STR_DCBusVolt.Brk_HeatCapacity_Q8)
    #endif
        {   //大于最大热容量则报电阻过载警告
            STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn = 1;
            PostErrMsg(RBOVERLOADWARN);       //输出欠压警告
            BrkStatusTemp = 0;
        }    //小于75%则清除警告
        else if ((STR_DCBusVolt.Brk_HeatTotal_Q8 < ((STR_DCBusVolt.Brk_HeatCapacity_Q8 * 3)>>2))
        &&(STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn == 1))
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn = 0;
            WarnAutoClr(RBOVERLOADWARN);  //清除制动电阻过载报警
            BrkStatusTemp = 1 ;
        }
    }

    STR_DCBusVolt.Brk_Status = BrkStatusTemp;
}

/*******************************************************************************
  函数名:  INT_16K_DCBusBrkProcess(void)     
           在电流环中断8K中进行母线电压制动泄放处理
           而母线电压的实时计算112#平台移入FPGA单元文件中更新，无FPGA平台则移入SVPWM中更新    ^_^
  输入:    母线电压计算系数 等
  输出:    母线电压计算，泄放开通、判断次数计数
  子函数:  暂无
  描述:  此处母线电压的采样计算频率与电流环同步，泄放程序判断为8K
         母线电压泄放开关管大于泄放点即打开，小于泄放点即关闭，如在关闭泄放点处加入滞回
         会导致母线电压切换不平滑，因此为保证母线电压泄放快速响应和切换时母线电压变换平滑
         并综合考虑制动管的散热，将泄放检测频率定为8K，且不加入滞回判断（陈老大确认制动管开关频率上限为20K）
********************************************************************************/ 
void INT_16K_DCBusBrkProcess(void)
{
    static Uint8 BleedCnt = 0;

    STR_DCBusVolt.DisableBrkCnt++; //不管制动管开关处于何种状态，均进行散热计数

    switch (STR_DCBusVolt.Brk_Status)  //泄放状态
    {
        case 0: //无制动电阻或运行过程中过载、为故障态时
        case 2: //断电泄放完毕
            //GPIO_Disable_Brake();  //置1  结束泄放
            STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 0;   //结束泄放
        break;

        case 3: //断电泄放开始
            BleedCnt++; 

            if(0x01 == (BleedCnt & 0x01))
            { //以电流环的50%占空比泄放
                //GPIO_Enable_Brake();   //清零 开始泄放
                STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 1;   //开始泄放
            }
            else 
            {
                //GPIO_Disable_Brake();
                STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 0;   //结束泄放
            }
            //STR_DCBusVolt.EnableBrkCnt++;   //泄放次数计数
        break;

        case 1: //正常运行泄放监控状态 
            if((STR_FUNC_Gvar.ADC_Samp.Udc_Live  > STR_DCBusVolt.UdcLeak)
               &&(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != NRD))
            { //母线电压大于泄放点且非nrd状态即开启泄放
                //GPIO_Enable_Brake();   //清零 开始泄放 
                STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 1;   //开始泄放
                STR_DCBusVolt.EnableBrkCnt++;   //计泄放次数计数
            }//无需滞回判断
            else //if(STR_FUNC_Gvar.ADC_Samp.Udc_Live < STR_DCBusVolt.UdcLeak) //- 200))
            {  //母线电压小于泄放点即关闭，实际如果加入滞回后泄放切换时母线电压切换会不平滑，所以无滞回
                //GPIO_Disable_Brake();  //置1  结束泄放
                STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 0;   //结束泄放
            }
        break;

        default:
            //GPIO_Disable_Brake();  //置1  结束泄放
            STR_FUNC_Gvar.Monitor2Flag.bit.BrkOut = 0;   //结束泄放
        break;
     }
}

/*******************************************************************************
  函数名:  DCBusVoltErr_Monitor(void)     过压欠压报错处理程序（主循环1ms调度运算）    ^_^
  输入:    实时母线电压 欠压报错点 过压报错点 
  输出:    欠压、过压报警标志位
  子函数:   LowDCBusVolt_Warn_ToqLmt(); //母线电压小于（欠压点+55V）时 欠压报警或转矩限制功能选择
  描述:  母线电压监控函数，当母线电压过高或低进行相应的处理
         调用该函数的地方：主循环1ms调度
********************************************************************************/ 
void DCBusVoltErr_Monitor(void)
{
    static Uint16 UdcRecoverCnt = 0;
    static int16  UdcLowErr_Cnt = 0;     //欠压报错计数滞回
    static int16  UdcOver_Cnt = 0;     //欠压报错计数滞回
    static Uint16 PowerOnCnt = 0;
   

    //实时母线电压连续大于过压点2次即报过压故障并返回
    if(STR_FUNC_Gvar.ADC_Samp.Udc_Live > STR_DCBusVolt.UdcOver)
    {
        UdcOver_Cnt++;
        if(UdcOver_Cnt >= 2)
        {
            PostErrMsg(OVERVOLTAGE);   //过电压报错
            return;
        }
    }
    else UdcOver_Cnt = 0;

    if(PowerOnCnt++ <= PowerOnWaitTime) //启机之后等待10ms，防止启机时电压还未建立，而报欠压
    {
        return;
    }
    //软启动不生效时检测欠压故障  连续6次检测电压低于UdcLow则报错并返回
    //且非缺三相的情况下，母线电压欠压情况下报欠压错误。
    if((STR_DCBusVolt.SoftStartFlag == SS_Disable)&&(STR_FUNC_Gvar.ADC_Samp.Udc_Live < STR_DCBusVolt.UdcLow))
    {
        if(UdcLowErr_Cnt++ >= 6)
        {
            UdcLowErr_Cnt = 0;

            if(STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_RDY) 
            {
                //三相驱动电均断线时不允许报出欠压，否则正常接线时断电时会报出欠压错误，E2ROM就会进行保存错误码操作
                if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN) PostErrMsg(UDVDC);         //输出欠压故障
            }

            STR_DCBusVolt.UnderUdcErrFlg = 1; 
            return;
        } 
    }
    else
    {
        UdcLowErr_Cnt = 0;
    }

    if(STR_DCBusVolt.UnderUdcErrFlg == 1)
    {
       //只要欠压即将UdcOkFlag置非,断电泄放不再与UdcOkFlag标志位关联
        STR_DCBusVolt.UdcOkFlag = INVALID;
    }

    //需要再判断,判是否仍处于欠压状态，UnderUdcErrFlg断电再次软启动时需要用到
    //母线电压连续8次大于(欠压点+30V)且欠压标志位已经置1，则将欠压标志位清0，
    if((STR_FUNC_Gvar.ADC_Samp.Udc_Live > (STR_DCBusVolt.UdcLow + 300)) && (STR_DCBusVolt.UnderUdcErrFlg ==1))
    {
        if(UdcRecoverCnt++ >= 8)
        {
            STR_DCBusVolt.UnderUdcErrFlg = 0;
        }
    }
    else
    {
        UdcRecoverCnt = 0;
    }

    //若欠压处理功能启动（默认不启用）且伺服准备好，则根据母线电压进行转矩限制处理
 //   if((FunCodeUnion.code.ER_UdcLowFncSel != 0)
 //       &&(STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy == VALID))
 //   { 
 //       LowDCBusVolt_Warn_ToqLmt();      //母线电压小于（欠压点+55V）时 欠压报警或转矩限制功能选择
 //   }
}


/*******************************************************************************
  函数名:  DcBusVolt_SoftStart(void)     软启动程序（主循环1ms调度运算）    ^_^
  输入:    实时母线电压和母线电压相关标志位
  输出:    驱动器、电机过载警告报警标志位
  子函数:  
  描述:   软启动保护程序共两种情况：一种是上电软启动，一种输入掉电后，
          电压跌落到一定程度需要打开软启动的继电器，防止再上电的话整流桥电流冲击太大。
          软启后，当母线电压在STR_DCBusVolt.UdcMin和STR_DCBusVolt.UdcMax范围内且2ms内
          电压变化小于15V时才结束软启动
********************************************************************************/ 
void DcBusVolt_SoftStart(void)
{
    STR_DCBusVolt.SSMonitCnt++;

    //两个时刻可以打开继电器使能软启动：
    //①上电初始化时将继电器打开，接入限流电阻，软启动使能
    //   STR_DCBusVolt.SoftStartFlag在程序初始化时即置为SS_Enable   继电器打开软启动开启状态
    //②运行期间导致母线电压低于欠压点即掉驱动电（此时肯定报欠电压）再软件或者DI故障复位时
    //此处代码是为情况②处理的，共母线的时候如果0A00不置2时（三相驱动电均断线）InputRdyFlg会一直置INPUT_NRD，由此UdcOK一直都不会
    //有效，如果不加入欠压这个条件此时RELAY信号就有可能会不停开通关断。
    if(  (STR_DCBusVolt.UdcOkFlag == INVALID)
      && (STR_DCBusVolt.SoftStartFlag == SS_Disable)
      && (STR_DCBusVolt.UnderUdcErrFlg == 1) )
    {  //欠压后UDCOKFLAG置非法，且软启动关闭
          //GPIO_WriteRLYDSP(SS_Enable);              //将继电器打开，接入限流电阻，软启动使能
          GPIO_Enable_SoftStart();   //置1  Bit Set   继电器打开接入限流电阻，软启动使能
          STR_DCBusVolt.SoftStartFlag = SS_Enable;  //驱动器进入软件启动状态
    }
    else if(  (STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_NRD)
           && (0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
      //无电源输入时继电器打开
    {
        GPIO_Enable_SoftStart();   //置1  Bit Set   继电器打开接入限流电阻，软启动使能
        STR_DCBusVolt.SoftStartFlag = SS_Enable;  //驱动器进入软件启动状态
    }
    else if( (STR_DCBusVolt.Brk_Status == 3)
          && (0 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))  //掉电泄放时继电器打开
    {
        GPIO_Enable_SoftStart();   //置1  Bit Set   继电器打开接入限流电阻，软启动使能
        STR_DCBusVolt.SoftStartFlag = SS_Enable;  //驱动器进入软件启动状态
    }

    //计算两次监控之间的电压变化率，并保存旧值
    STR_DCBusVolt.DeltaUdc = STR_FUNC_Gvar.ADC_Samp.Udc_Live - STR_DCBusVolt.UdcOld;
    STR_DCBusVolt.DeltaUdc = ABS(STR_DCBusVolt.DeltaUdc);
    STR_DCBusVolt.UdcOld = STR_FUNC_Gvar.ADC_Samp.Udc_Live;

    //2ms处理一次，处于软启动状态，且未欠压则判断何时关闭软启动
    if(((STR_DCBusVolt.SSMonitCnt & DC_MONIT_FREQ) == DC_MONIT_FREQ) &&
       (STR_DCBusVolt.SoftStartFlag == SS_Enable) && (STR_DCBusVolt.UnderUdcErrFlg == 0) &&
       (STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_RDY))
    {       
        //电压在UdcMin、UdcMax内,且2ms内母线电压变化率小于15V则结束软启动，闭合继电器
        if( (STR_DCBusVolt.DeltaUdc < 150)
         && (STR_FUNC_Gvar.ADC_Samp.Udc_Live < STR_DCBusVolt.UdcMax)
         && (STR_FUNC_Gvar.ADC_Samp.Udc_Live > STR_DCBusVolt.UdcMin) )
        {
            STR_DCBusVolt.SoftStartFlag = SS_Disable;  //置软启动禁止标志
        }
    }

    //置软启动禁止标志为1时,延时1ms处理
    if((STR_DCBusVolt.SoftStartFlag == SS_Disable))
    {
        //GPIO_WriteRLYDSP(SS_Disable);     //软启动禁止
        GPIO_Disable_SoftStart();  //清零 Bit Reset 继电器关闭短路限流电阻，软启动结束

        //置母线电压建立OK
        STR_DCBusVolt.UdcOkFlag = VALID;

        STR_DCBusVolt.PowerOffBleedFlg = 1;   //在软启动完成之后，若断电则允许泄放
    }

    //初始化后台循环，如果电压建立并且电流采样初始化完成，则进入伺服准备好状态
    if((STR_DCBusVolt.UdcOkFlag == VALID)  
       && (STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_RDY)  
       && (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.ShortGndDone == VALID)  //对地短路检测完
       && ((2 == FunCodeUnion.code.ER_PowerOffSaveToEeprom) || (1 == STR_DCBusVolt.CtrlUdcOk)))  //控制电准备好，H0A03=2时    
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy = VALID;   //母线电压正常时输出的rdy逻故障时就进入到故障态与此信号无关。
    }
    else 
    {
        if((VALID == STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy)
        && (INPUT_RDY != STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg) 
        && (1 == STR_FUNC_Gvar.MonitorFlag.bit.BrkDealOver))
        {
            //主回路掉电时需要等待抱闸处理结束（抱闸锁死）
            STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy = VALID;
        }
        else
        {
            STR_FUNC_Gvar.Monitor2Flag.bit.ServoRdy = INVALID;        
        }
    }

    STR_FUNC_Gvar.Monitor2Flag.bit.RelayFlg = STR_DCBusVolt.SoftStartFlag;

    //传递给FUNC模块的母线电压标准位
    if( (STR_DCBusVolt.UdcOkFlag == VALID)  
     && (STR_FUNC_Gvar.Monitor2Flag.bit.InputRdyFlg == INPUT_RDY) )
    {
        
        STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk = 1;
    }
    else
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.UdcOk = 0;
    }
}

/*******************************************************************************
  函数名:  LowDCBusVolt_Warn_ToqLmt(void)     欠压时转矩限制处理程序    ^_^
  输入:    STR_FUNC_Gvar.ADC_Samp.Udc_Live实时母线电压
  输出:     转矩限制电机模块全局标志位UDCLow_ToqLmtFlg 和欠压报警
  子函数:  无
  描述:    欠压报警和欠压转矩限制处理 H0A08 ER_UdcLowFncSel 为0则不进行欠压报警处理，
           为1则只报警不限转矩，为2则报警且限转矩，默认为0
           欠压报警后的转矩限制程序保留，但实际由于H0A08功能码范围为0~1
           0-不检出主回路欠电压警告；1-检出主回路欠电压警告  因此功能暂时处于屏蔽状态 
********************************************************************************/ 
/*Static_Inline void LowDCBusVolt_Warn_ToqLmt(void)
{
    static int16 UdcTorqLmtCnt = 0;//用于转矩恢复时间计算
    static Uint16 UdcLow_CntTemp = 0;//用于转矩恢复时间计算

    //若母线电压连续3次大于欠压点，且小于（欠压点+50V）时则进行
    //欠压报警或者欠压转矩限制处理
    if((STR_FUNC_Gvar.ADC_Samp.Udc_Live >= STR_DCBusVolt.UdcLow)
       && (STR_FUNC_Gvar.ADC_Samp.Udc_Live < STR_DCBusVolt.UdcLow + 500))
    {
        if(UdcLow_CntTemp++ >= 3)
        {
            UdcLow_CntTemp = 0;
            if(FunCodeUnion.code.ER_UdcLowFncSel == 1) //为0(默认)时不报错不限制，为1报警不限制转矩，为2报警且内部限制
            {  //母线欠压警告，发送错误信息
                STR_DCBusVolt.UdcLowWarn = 1;//上位机可根据警报进行转矩限制
                //PostErrMsg(UDVDCWARN);       //输出欠压警告
                STR_MTR_Gvar.GlobalFlag.bit.UdcLow_ToqLmtFlg = 0;  //传递转矩限制标志位
            }
            else if(FunCodeUnion.code.ER_UdcLowFncSel == 2)  //为2报警且内部限制
            {
                STR_DCBusVolt.UdcLowWarn = 1;
                //PostErrMsg(UDVDCWARN);       //输出欠压警告
                STR_MTR_Gvar.GlobalFlag.bit.UdcLow_ToqLmtFlg = 1; //传递转矩限制标志位
                //计算主回路电压下降时转矩限制解除时间 H0218单位为ms 监控周期也为1ms
                UdcTorqLmtCnt = FunCodeUnion.code.BP_FreeTorqLmtTime;
            }
        }
    }//若ER_UdcLowFncSel == 2且电压大于（欠压点+55V）且处报警置位状态则等待UdcTorqLmtCnt次后解除警报和转矩限制
    else if((STR_FUNC_Gvar.ADC_Samp.Udc_Live > (STR_DCBusVolt.UdcLow + 550)) && 
            (STR_DCBusVolt.UdcLowWarn == 1))
    {   
        if(FunCodeUnion.code.ER_UdcLowFncSel == 2)
        {
            if(UdcTorqLmtCnt >= 0)   //UdcTorqLimtCnt次则清零
            {                                                 
                UdcTorqLmtCnt--;
            }
            else    //清转矩限制标志位
            {
                 STR_MTR_Gvar.GlobalFlag.bit.UdcLow_ToqLmtFlg = 0;
                 STR_DCBusVolt.UdcLowWarn = 0;
                 //WarnAutoClr(UDVDCWARN);      //清除输出欠压警告
            }
        }
        else  
        {
            STR_DCBusVolt.UdcLowWarn = 0;
            //WarnAutoClr(UDVDCWARN);      //清除输出欠压警告
        }
    }//end if 解除报警报和转矩限制
    else   UdcLow_CntTemp = 0;    //欠压报警计数清零
}*/

/*******************************************************************************
函数名:
输入  : 无
输出  : 无
描述  : 
********************************************************************************/ 
void CtrlDCBusVoltBrake_Monitor(void)
{
    static Uint16 PowerOnCnt = 0;
    static Uint16 CtrlUdcOld = 0;
    static Uint16 Cnt100ms = 0;
	static Uint16 PowerBadFlag = 0;

    Uint16 DeltaCtrlUdc = 0;
    Uint32 Temp = 0;

#if POWERDRIVER_TYPE==POWDRV_IS650
    ADCPwrFilter.Input = STR_FUNC_Gvar.ADC_Samp.Udc_Live;
    LowPassFilter(&ADCPwrFilter); 
    STR_DCBusVolt.CtrlUdc = ADCPwrFilter.Output; 
    AuxFunCodeUnion.code.DP_CtrlUdc = ADCPwrFilter.Output;
#else 
    Get_ADC_ChannelValuePwr_1k();  
    ADCPwrFilter.Input = STR_FUNC_Gvar.ADC_Samp.Pwr;
    LowPassFilter(&ADCPwrFilter); 
    Temp = (ADCPwrFilter.Output * STR_DCBusVolt.CtrlUdcCoeff_Q12) >> 12;  //单位0.1V 
    STR_DCBusVolt.CtrlUdc = Temp * (int32)FunCodeUnion.code.OEM_UdcGain / 1000L; 
    AuxFunCodeUnion.code.DP_CtrlUdc = STR_DCBusVolt.CtrlUdc;
#endif 

    //CtrlUdcOk处理加入一个滞环STR_DCBusVolt.UdcMin和STR_DCBusVolt.CutoffLvl
    if(STR_DCBusVolt.CtrlUdc > STR_DCBusVolt.UdcMin) 
    {  
        STR_DCBusVolt.CtrlUdcOk = 1; 
    }
    else if(STR_DCBusVolt.CtrlUdc < STR_DCBusVolt.CutoffLvl)
    {
        STR_DCBusVolt.CtrlUdcOk = 0;
    }

    //上电延时100ms
    if((1 == STR_DCBusVolt.CtrlUdcOk) &&(PowerOnCnt < 100))
    {
        PowerOnCnt++;
        STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave = 0;
    }

    if(100 != PowerOnCnt) return;

    //CutoffPowerSave处理 控制电小于掉电阀值 CutoffPowerSave置1
    if(STR_DCBusVolt.CtrlUdc < STR_DCBusVolt.CutoffLvl)
    {
        STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave = 1;
        Cnt100ms = 0;
        CtrlUdcOld = STR_DCBusVolt.CtrlUdc;
		PowerBadFlag = 1; 

        
    }
	if((STR_DCBusVolt.CtrlUdc > STR_DCBusVolt.UdcMin)&&(1 ==PowerBadFlag ))
	{
 		if(2 != FunCodeUnion.code.ER_PowerOffSaveToEeprom) 
		{
			PostErrMsg(CTRLPOWERUDVDC2);
			PowerBadFlag = 0;
		}		
	}
    //控制电大于母线最小值且100ms变化率小于50V  CutoffPowerSave置0
    if(1 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave)
    {
        if(STR_DCBusVolt.CtrlUdc > STR_DCBusVolt.UdcMin) 
        {
            Cnt100ms ++;
            if(Cnt100ms > 100)
            {
                DeltaCtrlUdc = ABS(STR_DCBusVolt.CtrlUdc - CtrlUdcOld);
    
                //100ms内母线电压变化值小于50V,CutoffPowerSave置0
                if(DeltaCtrlUdc < 500) STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave = 0;
    
                Cnt100ms = 0;
                CtrlUdcOld = STR_DCBusVolt.CtrlUdc;
            }
        }
        else
        {
            Cnt100ms = 0;
            CtrlUdcOld = STR_DCBusVolt.CtrlUdc;         
        }                 
    }
    else
    {
        Cnt100ms = 0;
        CtrlUdcOld = STR_DCBusVolt.CtrlUdc;      
    }
}

/*******************************************************************************
  函数名: 
  输入  : 无  
  输出  : 无
  子函数:
  描述  : INT_16KBrkCurrentCheck刹车电流检测及保护
********************************************************************************/
#if POWERDRIVER_TYPE==POWDRV_IS650
void InitBrkCurrentCheck(void)
{
	BrkCurrentFilter.Ts = (Uint16)(STR_FUNC_Gvar.System.ToqPrd_Q10 >> 10);
    BrkCurrentFilter.Tc = (int32)10*10L;  //默认0.1ms滤波                                      
	BrkCurrentFilter.InitLowPassFilter(&BrkCurrentFilter); 

    STR_DCBusVolt.BrkOC = 0;
}
#endif

/*******************************************************************************
  函数名: 
  输入  : 无  
  输出  : 无
  子函数:
  描述  : INT_16KBrkCurrentCheck刹车电流检测及保护
********************************************************************************/
#if POWERDRIVER_TYPE==POWDRV_IS650
void INT_16KBrkCurrentCheck(void)
{
    int32   TempIb = 0;
	int32   Ibmax;
	Uint8   Temp_Index;
	Uint8   BrkStatusTemp = 0;

    if(FunCodeUnion.code.OEM_ServoSeri >= 20008) return;
	
    BrkStatusTemp = STR_DCBusVolt.Brk_Status;  

    BrkCurrentFilter.Input = STR_FUNC_Gvar.ADC_Samp.Ib;    
    BrkCurrentFilter.LowPassFilter(&BrkCurrentFilter);
    TempIb = BrkCurrentFilter.Output;

	Temp_Index = FunCodeUnion.code.OEM_ServoSeri%100 ; 
    if(Temp_Index > 8) Temp_Index = 8; 

	Ibmax = BrkIBMax[Temp_Index]*FunCodeUnion.code.ER_BlkOverCurrent/100; 	
	
	if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != NRD)
	{
		if(TempIb>Ibmax)
		{
			STR_DCBusVolt.BrkOC=1;
            BrkStatusTemp = 0;
			PostErrMsg(BLKOVERCURRENT);
		}
//		else
//		{
//			STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn = 0;
//            BrkStatusTemp = 1;
//		    WarnAutoClr(BLKOVERCURRENT);  //清除制动电阻过载报警
//		}
	}

	STR_DCBusVolt.Brk_Status = BrkStatusTemp;
}
#endif

void LineVolt_Monitor(void)
{
    Uint64 Temp = 0;
    Uint16 LineVoltPU = 0;
    Uint32 Temp1 = 0;
    Uint32 Temp2 = 0;
    

    Temp = ((Uint64)UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.Vd * UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.Vd + 
            (Uint64)UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.Vq * UNI_FUNC_MTRToFUNC_SlowList_1kHz.List.Vq);
    if(Temp > 32987568L)       
    {
        Temp = 32987568L;
    }
    LineVoltPU = qsqrt((Uint32)Temp); 

    Temp1 = (Uint32)(STR_FUNC_Gvar.ADC_Samp.Udc_Live * LineVoltPU / 8123);
    Temp2 = (Uint32)(STR_FUNC_Gvar.ADC_Samp.Udc_Live * 4300 / FunCodeUnion.code.OEM_CarrWaveFreq);

    if(1 == FunCodeUnion.code.GN_ServoCtrlMode)
    {
        if(Temp1 > Temp2)
        {
            AuxFunCodeUnion.code.DP_LineVolt = (Uint16)Temp2;
        }
        else
        {
            AuxFunCodeUnion.code.DP_LineVolt = (Uint16)Temp1;
        }
    }
    else
    {
        AuxFunCodeUnion.code.DP_LineVolt = (Uint16)Temp1;
    }

}
/********************************* END OF FILE *********************************/
