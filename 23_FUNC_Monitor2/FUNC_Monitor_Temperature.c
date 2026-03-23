/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名: FUNC_Monitor_Temperature.c       模块温度监控处理程序                                                 
 创建人：陈培正、王军干   创建日期：2008.11.10
 修改人：朱祥华                 修改日期：2011.11.21 
 描述： 模块温度监控处理程序
     1.
 修改记录：  
    1. 朱祥华       2011.11.21
       变更内容：温度变化缓慢其H0b组查询采样更新变为4HZ
                 
    2. xx.xx.xx      XX
       变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_Monitor_Temperature.h"
#include "FUNC_Monitor_TemperaTable.h"
#include "FUNC_AdcDriver_ST.h"
#include "FUNC_Filter.h"
#include "FUNC_GPIODriver.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define Tempera44Degree    8    //表格的每八位代表44度
#define Tempera48Degree    9    //表格的每八位代表48度
#define IPMCurseIndexNum   22   //IPM温度曲线检测点0~22
#define IPMTemperaMIN      12   //监测最小温度
#define IPMTemperaMAX      100  //监测最大温度

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义*/  
STR_MONITOR_IPMTEMPERATURE   STR_Monit_IPMTempera; //定义IPM温度监控变量

//模块温度采样数字滤波器
STR_BILINEAR_LOWPASS_FILTER  IPMTemperaFilter={0};
/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */ 
Uint16 *TemperaTableSel = (Uint16 *)TemperaTable_ExternalModule;        //指向温度曲线，初始化为0系列400W~1500w，防止非法引用

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitTemperatureProcess(void);     //上电初始化IPM温度测量参数配置
void Tempera_1k_Monitor(void);         //母线电压监控相关报错
void GetIPM_4Hz_Temperature(void);    //获取IPM温度交由H0b组显示



/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//暂无


/*******************************************************************************
  函数名:  InitTemperatureProcess(void)     上电初始化IPM温度测量参数配置
          （务必置于功能码、伺服系统控制参数初始化之后）    ^_^
  输入:    H0104电压等级 H0130母线电压增益调整 H0115至H0117母线电压过压保护、泄放点、欠压点；H0102驱动器型号
  输出:    
  子函数:  无
  描述:目前只有IPM温度监控，后续其它如电机温度监控可加入。
       参照IS500驱动器与电机参数表单(2.80)温度曲线  IPM过热温度点外置为85度，内置为90度
       0  -0.4kw     SizeA         铝基板曲线                  IPM过热报错点90度
       0.5kw-1.5kw   SizeB、SizeC  外置模块曲线                IPM过热报错点85度          
       2.0kw-7.5kw   SizeD、SizeE  BSM,Infinon两种为同样的曲线 IPM过热报错点90度
       其他          BSM曲线                                   IPM过热报错点90度

620P驱动器
     0 - 0.75Kw SIZE A  铝基板曲线                  IPM过热报错点90度
     其他          BSM曲线                                   IPM过热报错点90度

********************************************************************************/ 
void InitTemperatureProcess()
{
    Uint16 i = 0;
    Uint16 j = 0;        
    Uint16 temp0 = 0;

    //根据不同的驱动器型号在初始化时选择不同的温度曲线表格和不同的IPM过热报错点
    //并这一过热报错温度值转换成ADC采样值，曲线时间插补计算公式Yi=Y1-dY=Y1-((Y1-Y2)*(Xi-X1)/(X2-X1))
    //其中Y为ADC值 X代表温度值 ,X表格查询最小单位为每份4度，
#if POWERDRIVER_TYPE==POWDRV_IS650
   if((FunCodeUnion.code.OEM_ServoSeri == 20005)
	        ||(FunCodeUnion.code.OEM_ServoSeri == 20004)) //650
    {
        TemperaTableSel = (Uint16 *)TemperaTable_BSMModule ;        //IPM过热报错点95度
        //92度时代表ADC值为453,96度时代表ADC值为414  经插值算法计算后105度代表ADC值为424
        //因为Y1=453,Y2=414,Xi=95,X2=96,X1=92，所以Yi=453-29=424
        STR_Monit_IPMTempera.ErrValue = 340;
    }
	else if((FunCodeUnion.code.OEM_ServoSeri == 20006)
	      ||(FunCodeUnion.code.OEM_ServoSeri == 20000)
		  ||(FunCodeUnion.code.OEM_ServoSeri == 20001)
	      ||(FunCodeUnion.code.OEM_ServoSeri == 20002)
		  ||(FunCodeUnion.code.OEM_ServoSeri == 20003))
	{
        TemperaTableSel = (Uint16 *)TemperaTable_BSMModule ;        //IPM过热报错点100度
        STR_Monit_IPMTempera.ErrValue = 378;
	}
	else if((FunCodeUnion.code.OEM_ServoSeri == 20007) //650
		      ||(FunCodeUnion.code.OEM_ServoSeri == 20010)) //650

    {
        TemperaTableSel = (Uint16 *)TemperaTable_ExternalModule ;        //IPM过热报错点100度
        STR_Monit_IPMTempera.ErrValue = 273;
    }
	else if(FunCodeUnion.code.OEM_ServoSeri == 20008)
	{
        TemperaTableSel = (Uint16 *)TemperaTable_ExternalModule ;        //IPM过热报错点93度
        STR_Monit_IPMTempera.ErrValue = 334;
	}
    else if((FunCodeUnion.code.OEM_ServoSeri == 20009)
    ||(FunCodeUnion.code.OEM_ServoSeri == 20011)
    ||(FunCodeUnion.code.OEM_ServoSeri == 20015))
    {
        TemperaTableSel = (Uint16 *)TemperaTable_ExternalModule ;        //IPM过热报错点95度
        STR_Monit_IPMTempera.ErrValue = 325;
	}
    else if(FunCodeUnion.code.OEM_ServoSeri == 20014)
     {
        TemperaTableSel = (Uint16 *)TemperaTable_ExternalModule ;        //IPM过热报错点95度
        STR_Monit_IPMTempera.ErrValue = 365;
     }

#else
    if(FunCodeUnion.code.OEM_ServoSeri == 3)   // SizeA   400W 铝基板 一个结构
    {
        TemperaTableSel = (Uint16 *)TemperaTable_AluBaseBoard400w ;      //IPM过热报错点90度
        //88度时代表ADC值为522,92度时代表ADC值为475  经插值算法计算后90度代表ADC值为498
        //因为Y1=522,Y2=475,Xi=90,X2=92,X1=88，所以Yi=522-23.5=498.5
        STR_Monit_IPMTempera.ErrValue = 1270;

    }
    else if((FunCodeUnion.code.OEM_ServoSeri == 2) || (FunCodeUnion.code.OEM_ServoSeri == 5))  // SizeA   200W  750W都是铝基板 一个结构
    {
        TemperaTableSel = (Uint16 *)TemperaTable_AluBaseBoard ;      //IPM过热报错点90度
        //88度时代表ADC值为522,92度时代表ADC值为475  经插值算法计算后90度代表ADC值为498
        //因为Y1=522,Y2=475,Xi=90,X2=92,X1=88，所以Yi=522-23.5=498.5
        STR_Monit_IPMTempera.ErrValue = 498;

    }
    else if( (FunCodeUnion.code.OEM_ServoSeri == 6)
     || (FunCodeUnion.code.OEM_ServoSeri == 7)
     || (FunCodeUnion.code.OEM_ServoSeri == 10001)
     || (FunCodeUnion.code.OEM_ServoSeri == 10002)               
     || (FunCodeUnion.code.OEM_ServoSeri == 10003)
     || (FunCodeUnion.code.OEM_ServoSeri == 10004) ) //SizeC
    {
        TemperaTableSel = (Uint16 *)TemperaTable_BSMModule ;        //IPM过热报错点95度
        //92度时代表ADC值为453,96度时代表ADC值为414  经插值算法计算后95度代表ADC值为424
        //因为Y1=453,Y2=414,Xi=95,X2=96,X1=92，所以Yi=453-29=424
        STR_Monit_IPMTempera.ErrValue = 424;
    }
#endif
    else   //其余功率暂定为BSM温度传感器
    {
        TemperaTableSel = (Uint16 *)TemperaTable_BSMModule ;        //IPM过热报错点90度
        //88度时代表ADC值为497,92度时代表ADC值为453  经插值算法计算后90度代表ADC值为475
        //因为Y1=497,Y2=453,Xi=90,X2=92,X1=88，所以Yi=497-22=475
        STR_Monit_IPMTempera.ErrValue = 475;
    }

#if POWERDRIVER_TYPE==POWDRV_IS650
    if(0 < FunCodeUnion.code.ER_DriverTempErr)
    {
        //进行插补运算求过温点
        if(FunCodeUnion.code.ER_DriverTempErr < 20) FunCodeUnion.code.ER_DriverTempErr = 20;
        if(FunCodeUnion.code.ER_DriverTempErr > 105) FunCodeUnion.code.ER_DriverTempErr = 105;
        i = (FunCodeUnion.code.ER_DriverTempErr - 12) / 4;
        j = (FunCodeUnion.code.ER_DriverTempErr - 12) % 4; 
        temp0 = TemperaTableSel[i] - TemperaTableSel[i+1];
        STR_Monit_IPMTempera.ErrValue = TemperaTableSel[i] - (temp0 * j) / 4;
    }	
#else
    if(0 < FunCodeUnion.code.ER_DriverTempErr)
    {
        //进行插补运算求过温点
        if(FunCodeUnion.code.ER_DriverTempErr < 20) FunCodeUnion.code.ER_DriverTempErr = 20;
        if(FunCodeUnion.code.ER_DriverTempErr > 100) FunCodeUnion.code.ER_DriverTempErr = 100;
        i = (FunCodeUnion.code.ER_DriverTempErr - 12) / 4;
        j = (FunCodeUnion.code.ER_DriverTempErr - 12) % 4; 
        temp0 = TemperaTableSel[i] - TemperaTableSel[i+1];
        STR_Monit_IPMTempera.ErrValue = TemperaTableSel[i] - (temp0 * j) / 4;
    }
#endif

    AuxFunCodeUnion.code.DP_DriverTemp = IPMTemperaMIN;         //IPM模块初始值为12度最小值
    //AD采样滤波器初始化
    IPMTemperaFilter.Ts = 1000;               //模块温度采样周期1ms
    IPMTemperaFilter.Tc = 30000;  //滤波时间系数30ms
    InitLowPassFilter(&IPMTemperaFilter);
}




/*******************************************************************************
  函数名:Tempera_1k_Monitor(void)     
         目前只有IPM温度监控和风扇控制（1ms主循环更新），后续其它如电机温度监控可加入。
  输入:  温度AD采样值   IPM过温报错点 IPM温度曲线表单入口
  输出:  IPM过温报错
  子函数:  暂无
  描述:   Son状态打开风扇，其余状态当模块温度大于48度打开风扇，小于44度关闭风扇。
          对采样的模块温度进行监控，若过温则报出错误。
          初始化程序中已经将过热点温度转换成ADC数字值，在监控中直接将滤波后的实时
          模块温度值与此值比较即可。另外温度曲线为反时限曲线即温度越高，电阻越小，AD采样值越小。
********************************************************************************/ 
void Tempera_1k_Monitor() 
{
    static Uint16 Tempera_MonitorCNT = 0;

#if DRIVER_TYPE == SERVO_650N
    if(FunCodeUnion.code.ER_UseInProduce==1)return; //????oíà??ˉòa?ó2?±¨Er?￡660	
    //电机PTC检测
	if(GPIO_ReadMOTORPTCDSP())
	{
	    PostErrMsg(MOTORTEMPOT);
	}
#endif
    
    Get_ADC_ChannelValueIPM_1k();  //获取IPM采样值

    //模块温度采样值计算(加滤波)，STR_Monit_IPMTempera.ADCValue为滤波后的采样值
    IPMTemperaFilter.Input = STR_FUNC_Gvar.ADC_Samp.IPMT;
    LowPassFilter(&IPMTemperaFilter);
    STR_Monit_IPMTempera.ADCValue = IPMTemperaFilter.Output;

    if(Tempera_MonitorCNT <= 1000)
    { //上电等待1S再进行监控，即等待滤波输出值稳定，防止由于滤波输出值太小而温度监控误操作
        Tempera_MonitorCNT ++;  //1000次滤波之前，暂且直接取ADC采样值，只用于温度H0B组显示，不进行监控处理。
        STR_Monit_IPMTempera.ADCValue = STR_FUNC_Gvar.ADC_Samp.IPMT;
        return;
    }

    //在监控中，直接根据采样值和设定值比较
    if((2 != FunCodeUnion.code.ER_PowerOffSaveToEeprom) && (1 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave))
    {
        GPIO_Disable_FAN();  // 掉电时关闭风扇
    }
    else if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
      || (STR_Monit_IPMTempera.ADCValue <  TemperaTableSel[Tempera48Degree]) //温度越高，电阻越小，AD采样值越小
      || (STR_FUNC_Gvar.Monitor2Flag.bit.RbOverLoadWarn == 1) )
    { //Son状态或温度超过48度或制动过载电阻报警状态则输出低电平打开风扇
         GPIO_Enable_FAN();   
    }
    else if(STR_Monit_IPMTempera.ADCValue > TemperaTableSel[Tempera44Degree])
    {  //若温度小于44度则输出高电平关闭风扇，提高风扇寿命
         GPIO_Disable_FAN(); 
    }

    //温度大于故障温度时报错（温度越高，电阻越小，AD采样值越小）
    if(STR_Monit_IPMTempera.ADCValue < STR_Monit_IPMTempera.ErrValue)
    {
        PostErrMsg(RADIATOROT);    //散热片温度过高
    }
}



/*******************************************************************************
  函数名:Get_IPMTemperature(void)     
         获取模块温度用于H0B组数码管显示（20Hz主循环调度） 滤波时间30ms
  输入:  温度AD采样值   IPM温度曲线表单入口
  输出:  IPM实测温度
  子函数:  暂无
  描述:   原模块表单入口为温度值，输出为模块AD采样值，
          此程序内根据AD采样值反推出入口温度值，并进行四舍五入、插补运算
          计算的结果交由功能模块H0b组显示，
          温度为大惯性变量，且面板显示更新频率也为4Hz,因此其调度频率改为4Hz
********************************************************************************/ 
void GetIPM_4Hz_Temperature(void) 
{
    Uint16 i;
    Uint16 temp0;
    Uint16 temp1;

    //温度曲线为反时限曲线即温度越高，电阻越小，AD采样值越小。
    if (STR_Monit_IPMTempera.ADCValue >= TemperaTableSel[0])
    {//下限幅点，则置12度
        AuxFunCodeUnion.code.DP_DriverTemp = IPMTemperaMIN;
    }
    else if (STR_Monit_IPMTempera.ADCValue <= TemperaTableSel[IPMCurseIndexNum])
    {//上限幅点，则置100度
        AuxFunCodeUnion.code.DP_DriverTemp = IPMTemperaMAX;
    }
	else
	{
        for (i = 0; i < IPMCurseIndexNum; i++)
        {//查询表单，反推温度值
            if ((STR_Monit_IPMTempera.ADCValue >  TemperaTableSel[i+1]) 
               && (STR_Monit_IPMTempera.ADCValue <= TemperaTableSel[i]))
            {
                break;
            }
        }
        //进行插补运算，期间进行进行四舍五入运算
        temp0 = (TemperaTableSel[i] - STR_Monit_IPMTempera.ADCValue) << 2;
        temp1 = TemperaTableSel[i] - TemperaTableSel[i+1];
        AuxFunCodeUnion.code.DP_DriverTemp  = IPMTemperaMIN + (i << 2) + ((temp0 + (temp1 >> 1)) / temp1);
    }
}

/********************************* END OF FILE *********************************/
