/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:     FUNC_PanelDisplay.c
 创建人:    王治国                创建日期：2011.11.07      
 描述:
    1. 
    2. 
缩写表：
    1. Abs   ->   absolute
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCodeDefault.h"
#include "FUNC_PanelKey.h" 
//#include "FUNC_Panel.h"
#include "FUNC_InterfaceProcess.h"
#include "ECT_InterFace.h"
#include "ECT_ESMDisplay.h"
#include "CANopen_OD.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
Uint8   ESMState  = 0;


//按键电路板宏定义
#define  S5AS5R5ID1_VERB   1

// 8段式数码管的编码
// 显示字符段码在数组中对应的下标
#define DISPCODE_0       0   // 0
#define DISPCODE_1       1   // 1
#define DISPCODE_2       2   // 2
#define DISPCODE_3       3   // 3
#define DISPCODE_4       4   // 4
#define DISPCODE_5       5   // 5
#define DISPCODE_S       5   // 5
#define DISPCODE_6       6   // 6
#define DISPCODE_7       7   // 7
#define DISPCODE_8       8   // 8
#define DISPCODE_9       9   // 9
#define DISPCODE_A       10  // A
#define DISPCODE_b       11  // b
#define DISPCODE_C       12  // C
#define DISPCODE_d       13  // d
#define DISPCODE_E       14  // E
#define DISPCODE_F       15  // F
#define DISPCODE_P       16  // P
#define DISPCODE_NULL    17  // 全灭
#define DISPCODE_T       18  // T
#define DISPCODE_Y       19  // Y
#define DISPCODE_n       20  // n
#define DISPCODE_H       21  // H
#define DISPCODE_h       22  // h
#define DISPCODE_L       23  // L
#define DISPCODE_o       24  // o

#define DISPCODE_LINE_LO 25  // 下-
#define DISPCODE_LINE    26  // 中-
#define DISPCODE_LINE_UP 27  // 上-

#define DISPCODE_u       28  // u
#define DISPCODE_r       29  // r
#define DISPCODE_DOT     30  // 显示小数点，其他编码&即可
#define DISPCODE_ALL     31  // 全亮
#define DISPCODE_J       32  // J
#define DISPCODE_K       33  // K
#define DISPCODE_U       34  // U
#define DISPCODE_LINE_LO_UP       35  // 下-和上-

#if POWERDRIVER_TYPE==POWDRV_IS650
    #define SELECTDIOD      0xFE      
    #define DISPRUN         0xDF	  
    #define DISPERR         0xEF 
#endif
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//LED数码管选择使能
//按键电路有更改,请定义相关宏并在下面添加行对应设定值
#if  S5AS5R5ID1_VERB
//*************************************
//   __     __     __     __     __
//  |__|   |__|   |__|   |__|   |__|
//  |__|   |__|   |__|   |__|   |__|
//
//   0      1      2      3      4
//  0xDF   0xEF   0xF7   0xFB   0xFD
//*************************************
const  Uint8   TubeSelect[5] ={0xDF,0xEF,0xF7,0xFB,0xFD};
#endif


//数码管显示字符对应编码
//****************************************************************
//   __      __      __      __      __      __      __      __   
//  |__|    |__|    |__|    |__|    |__|    |__|    |__|    |__|  
//  |__|.   |__|.   |__|.   |__|.   |__|.   |__|.   |__|.   |__|. 

//   __                                                           
//             |                            |        __           
//                     |     __     |                           . 

//  0xFE    0xFD    0xFB    0xF7    0xEF    0xDF    0xBF    0x7F  
//****************************************************************
const Uint8 DispCodeTable[36]=
{0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,
//  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   b,   C,   d,   E,
 0x8e,0x8c,0xff,0x87,0x91,0xab,0x89,0x8b,0xc7,0xa3, 0xf7, 0xbf, 0xfe, 0xe3,
//  F,   P,全灭,   T,   Y,   n,   H,   h,   L,   o,  下-,  中-,  上-,    u,
 0xaf, 0x7f, 0x00, 0xe0, 0x0D ,0xc1,0xf6};
//  r, 小数点,全亮,   J,  K,    U

//二进制显示字符对应编码
const Uint8 DispBinaryTable[4]={
  0xEB, 0xED, 0xDB, 0xDD};
//低低, 低高, 高低, 高高
const Uint8 DispBinaryTable_High[2]={
  0xFB, 0xFD};
//低, 高


//n个LED数码管显示数据的最大值
const Uint32 DecDispMax[9] = {
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000};

const Uint32 HexDispMax[7] = {
    0x10,        
    0x100,       
    0x1000,      
    0x10000,     
    0x100000,    
    0x1000000,   
    0x10000000 };

const Uint32 BinDispMax[8] = {
    0x4,        
    0x10,       
    0x40,      
    0x100,     
    0x400,    
    0x1000,   
    0x4000,
    0x10000 };

//面板显示更新计数器
static Uint16 PanelDisCnt =250;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void PanelDisplay(void);  

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void PanelDisplayUpdate(STR_PANELOPERRATION *p);
Static_Inline void LEDDisplayOperation(STR_PANELOPERRATION *p);

static void PanelDispFunZero(STR_PANELOPERRATION *p);
static void PanelDispFun1st(STR_PANELOPERRATION *p);
static void PanelDispFun2nd(STR_PANELOPERRATION *p);
static void PanelDispFun3rd(STR_PANELOPERRATION *p);
static void PanelDispFun4th(STR_PANELOPERRATION *p);
static void PanelDispFun5th(STR_PANELOPERRATION *p);
static void PanelDispFun6th(STR_PANELOPERRATION *p);
static void PanelDispFun7th(STR_PANELOPERRATION *p);
static void PanelDispFun8th(STR_PANELOPERRATION *p);

void (*PanelDispFun[9])(STR_PANELOPERRATION *p) = {
    PanelDispFunZero,
    PanelDispFun1st,
    PanelDispFun2nd,
    PanelDispFun3rd,
    PanelDispFun4th,
    PanelDispFun5th,
    PanelDispFun6th,
    PanelDispFun7th,
    PanelDispFun8th,
};
//默认显示H0B组参数
Static_Inline Uint8 DefaultDispH0B(STR_PANELOPERRATION *p);

//设定显示数据
Static_Inline void SetDispData(STR_PANELOPERRATION *p, Uint8 TubeData0, Uint8 TubeData1st, 
                          Uint8 TubeData2nd, Uint8 TubeData3rd, Uint8 TubeData4th);

//十进制格式显示数据
Static_Inline void DecDisplay(STR_PANELOPERRATION *p);
//十进制格式显示数据译码
Static_Inline void DispDecCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData);
//十六进制格式显示数据
Static_Inline void HexDisplay(STR_PANELOPERRATION *p);
//十六进制格式显示数据译码
Static_Inline void DispHexCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData);
//二进制格式显示数据
Static_Inline void BinDisplay(STR_PANELOPERRATION *p);
//二进制格式显示数据译码
Static_Inline void DispBinCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData);
/*******************************************************************************
  函数名:    void PanelDisplay(void)
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 本函数程序调度周期改为250Hz
    2. 闪烁频率为2Hz
********************************************************************************/
void PanelDisplay(void)
{
    if(FunCodeUnion.code.BP_PanelDisFreq == 0)
    {
        PanelDisCnt = 250;
    }
    else
    {
        PanelDisCnt = 1 + (250 / FunCodeUnion.code.BP_PanelDisFreq);
    }

    PanelDisplayUpdate(&ServoPanel);
    
    LEDDisplayOperation(&ServoPanel);
}

/*******************************************************************************
  函数名:    Static_Inline void LEDDisplayOperation(STR_PANELOPERRATION *p)
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 本函数程序调度周期改为250Hz
    2. 闪烁频率为2Hz
********************************************************************************/
Static_Inline void LEDDisplayOperation(STR_PANELOPERRATION *p)
{
    Uint16 LEDLightFreq = 0;
    Uint16 LEDDisplayFreq = 0;
    
    //获取当前数码管的使能及显示数据
    p->Display.TxTubeSel = TubeSelect[p->Display.TubeIndex];
    p->Display.TxDispData = p->Display.DispData[p->Display.TubeIndex];


    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus==3)
    {
        LEDLightFreq   = PanelDisCnt>>2;//
        LEDDisplayFreq = PanelDisCnt>>1;//
    }
    else if(ESMState ==ESM_SAFEOPERATION)
    {
        LEDLightFreq   = 50;//200ms
        LEDDisplayFreq = 300;//1200ms
    }
    else if(ESMState ==ESM_PREOPERATION)
    {
        LEDLightFreq   = 50;//200ms
        LEDDisplayFreq = 100;//400ms
    }
    else
    {
        LEDLightFreq   = PanelDisCnt>>2;//
        LEDDisplayFreq = PanelDisCnt>>1;//
    }
    
    //数码管闪烁处理
    if(p->Display.CtrlReg.all & 0x1F)   //有闪烁
    {
        p->Display.Timer ++;

        //如果定时器大于125且当前数码管闪控制烁位为1,那么当前数码管灭
        if((p->Display.Timer > LEDLightFreq) && ((p->Display.CtrlReg.all >> p->Display.TubeIndex)& 0x01))
        { 
            if(((p->Display.TxDispData & 0x80) != 0x80) && p->Display.CtrlReg.bit.DotFlicker)  //如果有小数点,显示小数点
            {
                p->Display.TxDispData = 0x7F;
            }
            else                                //当前数码管灭
            {
                p->Display.TxDispData = 0xFF;
            } 
        }
        
        if(p->Display.Timer > LEDDisplayFreq)
        {
            p->Display.Timer = 0;
        } 
    }
    else  //无闪烁
    {
        p->Display.Timer = 0;
    }
    
    //更新显示数码管序号
    if(p->Display.TubeIndex >= 4) p->Display.TubeIndex = 0;
    else p->Display.TubeIndex ++;

    STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelTubeSel = p->Display.TxTubeSel;
    STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelDispData = p->Display.TxDispData;
}


/*******************************************************************************
  函数名:    Static_Inline void LEDDisplayOperation(STR_PANELOPERRATION *p)
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 本函数程序调度周期改为250Hz
    2. 闪烁频率为2Hz
********************************************************************************/
/*Static_Inline void LEDDisplayOperation(STR_PANELOPERRATION *p)
{

    //获取当前数码管的使能及显示数据
    p->Display.TxTubeSel = TubeSelect[p->Display.TubeIndex];
    p->Display.TxDispData = p->Display.DispData[p->Display.TubeIndex];


    //数码管闪烁处理
    if(p->Display.CtrlReg.all & 0x1F)   //有闪烁
    {
        p->Display.Timer ++;

        //如果定时器大于125且当前数码管闪控制烁位为1,那么当前数码管灭
        if((p->Display.Timer > 61) && ((p->Display.CtrlReg.all >> p->Display.TubeIndex)& 0x01))
        { 
            if(((p->Display.TxDispData & 0x80) != 0x80) && p->Display.CtrlReg.bit.DotFlicker)  //如果有小数点,显示小数点
            {
                p->Display.TxDispData = 0x7F;
            }
            else                                //当前数码管灭
            {
                p->Display.TxDispData = 0xFF;
            } 
        }
        
        if(p->Display.Timer > 125)
        {
            p->Display.Timer = 0;
        } 
    }
    else  //无闪烁
    {
        p->Display.Timer = 0;
    }

    //更新显示数码管序号
    if(p->Display.TubeIndex >= 4) p->Display.TubeIndex = 0;
    else p->Display.TubeIndex ++;

    STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelTubeSel = p->Display.TxTubeSel;
    STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelDispData = p->Display.TxDispData;
}*/
/*******************************************************************************
  函数名: void DiodDisp(void)
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 本函数程序调度周期改为250Hz
    2. 闪烁频率为2Hz
********************************************************************************/
#if POWERDRIVER_TYPE==POWDRV_IS650
void DiodDisp(Uint8* pTubeSel,Uint8* pDispData) 
{
	static Uint8 TimeCnt=0;
	static Uint8 FlashTime=0;
	if(STR_PUB_Gvar.AllInitDone==1)
	{
	    TimeCnt++;
	}
	else
	{
	    TimeCnt=0;
		FlashTime=0;
	}

	if((TimeCnt & 0x01)==1)
	{ 		

		if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus==2)
		{
		    *pTubeSel  = 0xFF -SELECTDIOD;
		    *pDispData = 0xFF -DISPRUN;
			FlashTime=0;
		}
		else if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus==3)
		{
		    FlashTime++;
		   
		    *pTubeSel  = 0xFF -SELECTDIOD;
		    *pDispData = 0xFF -DISPERR;
			
			if(FlashTime>25)
			{
	            *pDispData = 0xFF -0xFF;
				
				if(FlashTime>=125)
				{
				    FlashTime=0;
				}
			}
		}
	}
	else
	{
		*pTubeSel  = 0xFF - STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelTubeSel;		
		*pDispData = 0xFF -STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelDispData;

	}
}
#endif

/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
Static_Inline void PanelDisplayUpdate(STR_PANELOPERRATION *p)
{
    static Uint16 Cnt = 0;
    static Uint8 Index = 0;
    static Uint32 DisTestCnt = 0;

    //复位显示缓冲区数据
    p->Display.DispData[0] = DispCodeTable[DISPCODE_NULL];
    p->Display.DispData[1] = DispCodeTable[DISPCODE_NULL];
    p->Display.DispData[2] = DispCodeTable[DISPCODE_NULL];
    p->Display.DispData[3] = DispCodeTable[DISPCODE_NULL];
    p->Display.DispData[4] = DispCodeTable[DISPCODE_NULL];

    //初始化未完成,只显示RESET,返回
    if(STR_PUB_Gvar.AllInitDone == 0)
    {
        SetDispData(p,DispCodeTable[DISPCODE_r],DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_S],
                      DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_T]);
        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }

    //hardfault中断,返回
    if(STR_PUB_Gvar.AllInitDone == 2)
    {
        //更新显示值             
        if(Index < 16) 
        {
            p->Display.DispData[0] = DispCodeTable[Index & 0x0F];
        }
        else
        {
            p->Display.DispData[0] = DispCodeTable[Index & 0x0F] & DispCodeTable[DISPCODE_DOT];            
        }

        p->Display.DispData[1] = DispCodeTable[(HFInfor.Dis_16Bits[Index] >> 12) & 0x0F];
        p->Display.DispData[2] = DispCodeTable[(HFInfor.Dis_16Bits[Index] >> 8) & 0x0F];
        p->Display.DispData[3] = DispCodeTable[(HFInfor.Dis_16Bits[Index] >> 4) & 0x0F];
        p->Display.DispData[4] = DispCodeTable[HFInfor.Dis_16Bits[Index] & 0x0F];
                                            

        Cnt++;
        if(Cnt > 500)
        {
            Cnt = 0;
            Index ++;
            if(Index > 27) Index = 0;
        }

        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }


    //系统复位操作时显示F.1n1t
    if(STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 1)
    {
        //显示F.init
        SetDispData(p,DispCodeTable[DISPCODE_F]& DispCodeTable[DISPCODE_DOT],
                      DispCodeTable[DISPCODE_1],DispCodeTable[DISPCODE_n],
                      DispCodeTable[DISPCODE_1],DispCodeTable[DISPCODE_T]);
        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }
    
    #if CAN_ENABLE_SWITCH
    if(STR_FUNC_Gvar.ManageFunCodeOutput.CANReadComOD == 1)
    {        
        SetDispData(p,DispCodeTable[DISPCODE_r],DispCodeTable[DISPCODE_E],
                      DispCodeTable[DISPCODE_A],DispCodeTable[DISPCODE_d],
                      DispCodeTable[DISPCODE_NULL]);
        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }
    #endif
    
    //报错显示处理
    if(STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow == 1)
    {
        static  Uint16 ErrLatch = 0;

        //故障复位成功后将ErrLatch清0
        if(1 == STR_FUNC_Gvar.MonitorFlag.bit.ErrResetExecut)
        {
            STR_FUNC_Gvar.MonitorFlag.bit.ErrResetExecut = 0;
            ErrLatch = 0;
        }

        if( (STR_FUNC_Gvar.Monitor.HighLevelErrCode != 0) &&
            ( (p->Menu.CtrlReg.bit.ErrShowSure == 0) || 
              (p->Menu.CtrlReg.bit.ErrShowSure && (ErrLatch != STR_FUNC_Gvar.Monitor.HighLevelErrCode)) ) )
        {
            p->Menu.CtrlReg.bit.ErrShowSure = 0;
            ErrLatch = STR_FUNC_Gvar.Monitor.HighLevelErrCode;
            SetDispData(p,DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_r] & DispCodeTable[DISPCODE_DOT],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 8) & 0x000F],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 4) & 0x000F],
                      DispCodeTable[STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0x000F]);

            //需要全部闪烁
            p->Display.CtrlReg.all = 0x1F;
            return;
        }         
    }

    //电流环PI参数自调谐时
    if((STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1) && 
       (p->Menu.CtrlReg.bit.Class != CLASS_7TH)) 
    {
        //显示PI
        SetDispData(p,DispCodeTable[DISPCODE_P],DispCodeTable[DISPCODE_1],DispCodeTable[DISPCODE_NULL],
                      DispCodeTable[DISPCODE_NULL],DispCodeTable[STR_FUNC_Gvar.ToqPiTune.Step]);
        return;
    }

    //面板显示测试
    if(1 == AuxFunCodeUnion.code.FA_DisTest)
    {
        DisTestCnt ++;
        //延时1min
        if(DisTestCnt > 15000) AuxFunCodeUnion.code.FA_DisTest = 0;

        //显示8.8.8.8.8.
        SetDispData(p,DispCodeTable[DISPCODE_ALL],
                      DispCodeTable[DISPCODE_ALL],
                      DispCodeTable[DISPCODE_ALL],
                      DispCodeTable[DISPCODE_ALL],
                      DispCodeTable[DISPCODE_ALL]);
        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;    
    }
    else
    {
        DisTestCnt = 0;
    }

    //菜单显示更新
    PanelDispFun[p->Menu.CtrlReg.bit.Class](p);
}


/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 显示伺服状态机错误编码
    2.
********************************************************************************/
/*static void PanelDispFunZero(STR_PANELOPERRATION *p)
{

	Uint16  ESM = 0;
    Uint8   CommState = 0;
    //Uint8   ESMState  = 0;
	Uint8   ESMStateDisplay[5];				// 状态机  		1 ：初始化   init       面板显示:Int
                                //              2 ：预运行   pro_int    面板显示:Pnt
                                //				4 ：安全运行 safe_op    面板显示:Sop
                                //				8 ：运行     op         面板显示:Ope
//	Uint32  ErrCode;			// 错误状态		0 ：无错误   >0:状态码
	Uint16  PortStatus;		//0x00--端口未连接  0x01:端口连接    0x02:端口有数据通过

    
    CommState = C_CommState(&ESM);

    //1、显示端口连接状态，有数据通过时，需要闪烁,其他不闪烁  闪烁频率与IS620P暂时保持一致，如
    PortStatus = ESM&0x0003;

    //2、找到在字符显示表中的位置，不闪烁
    ESMState  = (Uint8)((ESM>>8)&0x3F);
        
    //无故障
    if(CommState == 0)//ESM状态显示+通讯连接指示
    {
        switch(PortStatus)
        {
            case 0x0000://Port0Status=0;Port1Status =0;
                ESMStateDisplay[0] = DISPCODE_NULL;
                ESMStateDisplay[1] = DISPCODE_NULL;
                //无闪烁
                p->Display.CtrlReg.all = 0;
                break;
                
            case 0x0001://Port0Status=0;Port1Status =1;
                ESMStateDisplay[0] = DISPCODE_NULL;
                ESMStateDisplay[1] = DISPCODE_LINE;
                //无闪烁
                p->Display.CtrlReg.all = 2;
                break;
            
            case 0x0002://Port0Status=1;Port1Status =0;
                ESMStateDisplay[0] = DISPCODE_LINE;
                ESMStateDisplay[1] = DISPCODE_NULL;
                //无闪烁
                p->Display.CtrlReg.all = 1;
                break;

            case 0x0003://Port0Status=1;Port1Status =1;
                ESMStateDisplay[0] = DISPCODE_LINE;
                ESMStateDisplay[1] = DISPCODE_LINE;
                //无闪烁
                p->Display.CtrlReg.all = 3;
                break;
                
            default:
                break;
        }

        switch(ESMState)
        {
            case ESM_INITIAl://INIT状态，持续1800ms左右
                ESMStateDisplay[2] = DISPCODE_1;
                break;

            case ESM_PREOPERATION://PREOP状态，持续330ms
                ESMStateDisplay[2] = DISPCODE_P;
                break;

            case ESM_SAFEOPERATION://SAFEOP状态，持续360ms左右
                ESMStateDisplay[2] = DISPCODE_S;
                break;

            case ESM_OPERATION:
                ESMStateDisplay[2] = DISPCODE_o;
                break;
                
            case ESM_FAULT:
                ESMStateDisplay[2] = DISPCODE_F;
                break;

            case ESM_STOP:
                ESMStateDisplay[2] = DISPCODE_T;
                break;
                
            default:
                ESMStateDisplay[2] = DISPCODE_NULL;
                break;
        }
    }
    
        
    switch(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
    {
        case 0:
            //伺服未准备好
            ESMStateDisplay[3] = DISPCODE_n;
            ESMStateDisplay[4] = DISPCODE_r;
            SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                          DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);
            break;

        case 1:         //伺服准备好
            if(DefaultDispH0B(p) == 1)
            {
                //伺服未准备好
                ESMStateDisplay[3] = DISPCODE_r;
                ESMStateDisplay[4] = DISPCODE_Y;
                SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                              DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);
            }
            break;

        case 2:         //伺服运行   这里需要添加根据H0232设定显示H0B组的功能
            if(DefaultDispH0B(p) == 1)
            {
                //伺服未准备好
                ESMStateDisplay[3] = DISPCODE_r;
                ESMStateDisplay[4] = DISPCODE_n;
                SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                              DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);
            }
            break;
        case 3:         //伺服故障
            //伺服未准备好
            ESMStateDisplay[3] = DISPCODE_F;
            ESMStateDisplay[4] = DISPCODE_T;
            SetDispData(p,DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_r] & DispCodeTable[DISPCODE_DOT],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 8) & 0x000F],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 4) & 0x000F],
                      DispCodeTable[STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0x000F]);
            //无闪烁
            p->Display.CtrlReg.all = 0;
            break;
    }
}*/
static void PanelDispFunZero(STR_PANELOPERRATION *p)
{

	Uint16  ESM = 0;
    Uint8   CommState = 0;
    //Uint8   ESMState  = 0;
	Uint8   ESMStateDisplay[5];				// 状态机  		1 ：初始化   init       面板显示:Int
                                //              2 ：预运行   pro_int    面板显示:Pnt
                                //				4 ：安全运行 safe_op    面板显示:Sop
                                //				8 ：运行     op         面板显示:Ope
//	Uint32  ErrCode;			// 错误状态		0 ：无错误   >0:状态码
	Uint16  PortStatus;		//0x00--端口未连接  0x01:端口连接    0x02:端口有数据通过

    
    CommState = C_CommState(&ESM);

    //1、显示端口连接状态，有数据通过时，需要闪烁,其他不闪烁  闪烁频率与IS620P暂时保持一致，如
    PortStatus = ESM&0x0003;

    //2、找到在字符显示表中的位置，不闪烁
    ESMState  = (Uint8)((ESM>>8)&0x3F);
        
    //无故障
    if(CommState == 0)//ESM状态显示+通讯连接指示
    {
        switch(PortStatus)
        {
            case 0x0000://Port0Status=0;Port1Status =0;
                ESMStateDisplay[0] = DISPCODE_NULL;
                //无闪烁
                p->Display.CtrlReg.bit.ZeroFlicker = 0;
                break;
                
            case 0x0001://Port0Status=1;Port1Status =0;
            #if DRIVER_TYPE == SERVO_650N
                ESMStateDisplay[0] = DISPCODE_LINE_UP;
            #else
                ESMStateDisplay[0] = DISPCODE_LINE_LO;
            #endif
                //无闪烁
                p->Display.CtrlReg.bit.ZeroFlicker = 0;
                break;
            
            case 0x0002://Port0Status=0;Port1Status =1;
            #if DRIVER_TYPE == SERVO_650N
                ESMStateDisplay[0] = DISPCODE_LINE_LO;
            #else
                ESMStateDisplay[0] = DISPCODE_LINE_UP;
            #endif
                //无闪烁
                p->Display.CtrlReg.bit.ZeroFlicker = 0;
                break;

            case 0x0003://Port0Status=1;Port1Status =1;
                ESMStateDisplay[0] = DISPCODE_LINE_LO_UP;
                //无闪烁
                p->Display.CtrlReg.bit.ZeroFlicker = 0;
                break;
                
            default:
                break;
        }

        switch(ESMState)
        {
            case ESM_INITIAl://INIT状态，持续1800ms左右
                ESMStateDisplay[1] = DISPCODE_1;
                //无闪烁
                p->Display.CtrlReg.bit.FirstFlicker = 0;
                break;

            case ESM_PREOPERATION://PREOP状态，持续330ms
                ESMStateDisplay[1] = DISPCODE_2;
                p->Display.CtrlReg.bit.FirstFlicker = 1;
                break;

            case ESM_SAFEOPERATION://SAFEOP状态，持续360ms左右
                ESMStateDisplay[1] = DISPCODE_4;
                p->Display.CtrlReg.bit.FirstFlicker = 1;
                break;

            case ESM_OPERATION:
                ESMStateDisplay[1] = DISPCODE_8;
                p->Display.CtrlReg.bit.FirstFlicker = 0;
                break;
                
            case ESM_FAULT:
                ESMStateDisplay[1] = DISPCODE_F;
                p->Display.CtrlReg.bit.FirstFlicker = 0;
                break;

            case ESM_STOP:
                ESMStateDisplay[1] = DISPCODE_T;
                p->Display.CtrlReg.bit.FirstFlicker = 0;
                break;
                
            default:
                ESMStateDisplay[1] = DISPCODE_NULL;
                p->Display.CtrlReg.bit.FirstFlicker = 0;
                break;
        }
    }
    
    ESMStateDisplay[2] = ObjectDictionaryStandard.DeviceControl.ModesOfOperationDisplay;
    
    switch(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
    {
        case 0:
            //伺服未准备好
            ESMStateDisplay[3] = DISPCODE_n;
            ESMStateDisplay[4] = DISPCODE_r;
            SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                          DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);

            //无闪烁
            p->Display.CtrlReg.bit.SecondFlicker = 0;
            p->Display.CtrlReg.bit.ThirdFlicker = 0;
            p->Display.CtrlReg.bit.ForthFlicker = 0;

            break;

        case 1:         //伺服准备好
            if(DefaultDispH0B(p) == 1)
            {
                //ESMStateDisplay[2] = DISPCODE_r;
                ESMStateDisplay[3] = DISPCODE_r;
                ESMStateDisplay[4] = DISPCODE_Y;
                SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                              DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);

                //无闪烁
                p->Display.CtrlReg.bit.SecondFlicker = 0;
                p->Display.CtrlReg.bit.ThirdFlicker = 0;
                
                if(AuxFunCodeUnion.code.DP_MotorSpd == 0)
                {
                    p->Display.CtrlReg.bit.ForthFlicker = 0;
                }
                else
                {
                    p->Display.CtrlReg.bit.ForthFlicker = 1;

                }
            }
            break;

        case 2:         //伺服运行   这里需要添加根据H0232设定显示H0B组的功能
            if(DefaultDispH0B(p) == 1)
            {
                //ESMStateDisplay[2] = DISPCODE_r;
                ESMStateDisplay[3] = DISPCODE_r;
                ESMStateDisplay[4] = DISPCODE_n;
                
                SetDispData(p,DispCodeTable[ESMStateDisplay[0]],DispCodeTable[ESMStateDisplay[1]],DispCodeTable[ESMStateDisplay[2]],
                          DispCodeTable[ESMStateDisplay[3]],DispCodeTable[ESMStateDisplay[4]]);

                //无闪烁
                p->Display.CtrlReg.bit.SecondFlicker = 0;
                p->Display.CtrlReg.bit.ThirdFlicker = 0;
                
                if(AuxFunCodeUnion.code.DP_MotorSpd == 0)
                {
                    p->Display.CtrlReg.bit.ForthFlicker = 0;
                }
                else
                {
                    p->Display.CtrlReg.bit.ForthFlicker = 1;
                }
            }
            break;
        case 3:         //伺服故障
            SetDispData(p,DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_r] & DispCodeTable[DISPCODE_DOT],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 8) & 0x000F],
                      DispCodeTable[(STR_FUNC_Gvar.Monitor.HighLevelErrCode >> 4) & 0x000F],
                      DispCodeTable[STR_FUNC_Gvar.Monitor.HighLevelErrCode & 0x000F]);
            //无闪烁
            p->Display.CtrlReg.all = 0;
            break;
    }
}

/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. H0232显示H0B组参数
    2.
********************************************************************************/
Static_Inline Uint8 DefaultDispH0B(STR_PANELOPERRATION *p)
{
    int8    DispGroupLatch;              //显示功能码组锁存
    int8    DispOffsetLatch;             //显示功能码组内偏移锁存
    static Uint16 Cnt = 0;               //更新显示值计数器
    
    //在转速为零或HOB组保留参数时，返回1
    if( (AuxFunCodeUnion.code.DP_MotorSpd == 0) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode >= H0B_PANELDISPLEN) || 
        (FunCodeUnion.code.BP_DefaultDisplayCode == 4) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 6) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 8) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 14) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 16) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 18) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 20) ||
        (  (FunCodeUnion.code.BP_DefaultDisplayCode > 30)  
         &&(FunCodeUnion.code.BP_DefaultDisplayCode < 53) ) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 54) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 56)   )
    {
        return 1;
    }
    else
    {
        DispGroupLatch = p->Menu.DispGroup;
        DispOffsetLatch = p->Menu.DispOffset;

        p->Menu.DispGroup = 0x0B;
        p->Menu.DispOffset = FunCodeUnion.code.BP_DefaultDisplayCode;
        
        //每隔1s更新显示值
        Cnt ++;
        if(Cnt >= PanelDisCnt)
        {
            Cnt = 0;
            if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD) 
            {   //32位数据
                p->Menu.DispData = A_SHIFT16_PLUS_B(GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset + 1),
                                                    GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset));
            }
            else
            {   //16位数据
                p->Menu.DispData = (Uint32)GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset);
                p->Menu.DispData &= 0xFFFF;
                //如果是负数,高十六位置1
                if((GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN) &&
                  (p->Menu.DispData > 32767))
                {
                    p->Menu.DispData |= 0xFFFF0000;
                }
            }
        }
        else if( ((p->Menu.DispGroup == 0x0B) && (p->Menu.DispOffset == 0x03)) ||
                 ((p->Menu.DispGroup == 0x0B) && (p->Menu.DispOffset == 0x05)) )
        {
            p->Menu.DispData = (Uint32)GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset);
            p->Menu.DispData &= 0xFFFF;
            //如果是负数,高十六位置1
           if((GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN) &&
              (p->Menu.DispData > 32767))
            {
                p->Menu.DispData |= 0xFFFF0000;
            }
        }

        switch( GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) )
        {
            case ATTRIB_YNUM_TYP://十进制显示
                DecDisplay(p);
                //如果翻页,第0位闪烁显示页符号
                if(GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset) > 5)
                {
                    p->Display.CtrlReg.bit.ZeroFlicker = 1;
                }
                break;

            case ATTRIB_HNUM_TYP: //十六进制显示 
                HexDisplay(p);
                //如果翻页,第0位闪烁显示页符号
                if(GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset) > 5)
                {
                    p->Display.CtrlReg.bit.ZeroFlicker = 1;
                }
                break;

            case ATTRIB_NNUM_TYP: //二进制显示,DI1-10位,DO-8位
                BinDisplay(p);
                break;
        }

        //无闪烁
        p->Display.CtrlReg.all = 0;

        p->Menu.DispGroup = DispGroupLatch;
        p->Menu.DispOffset = DispOffsetLatch;
        return 0;
    }
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
static void PanelDispFun1st(STR_PANELOPERRATION *p)
{
    SetDispData(p,DispCodeTable[DISPCODE_H],
                  DispCodeTable[(p->Menu.DispGroup >> 4) & 0x0F],
                  DispCodeTable[p->Menu.DispGroup & 0x0F]& DispCodeTable[DISPCODE_DOT],
                  DispCodeTable[DISPCODE_NULL],DispCodeTable[DISPCODE_NULL]);

    //当前处理位闪烁
    p->Display.CtrlReg.all = 1 << p->Menu.CtrlReg.bit.DealBit;
    p->Display.CtrlReg.bit.DotFlicker = 1;
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
static void PanelDispFun2nd(STR_PANELOPERRATION *p)
{
    static Uint16 Delay1S_Cnt = 0;

    //第二级菜单显示SAVE特殊处理,只显示2秒
    if (p->Menu.CtrlReg.bit.Class2ndDisp_SAVE == 1)
    {
        Delay1S_Cnt ++;
        if(Delay1S_Cnt >= 500)
        {
            Delay1S_Cnt = 0;
            p->Menu.CtrlReg.bit.Class2ndDisp_SAVE = 0;
        }

        SetDispData(p,DispCodeTable[DISPCODE_5],
                      DispCodeTable[DISPCODE_A],
                      DispCodeTable[DISPCODE_U],
                      DispCodeTable[DISPCODE_E],
                      DispCodeTable[DISPCODE_NULL]);
    
        //当前无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }

    SetDispData(p,DispCodeTable[DISPCODE_H],
                  DispCodeTable[(p->Menu.DispGroup >> 4) & 0x0F],
                  DispCodeTable[p->Menu.DispGroup & 0x0F]& DispCodeTable[DISPCODE_DOT],
                  DispCodeTable[p->Menu.DispOffset / 10],
                  DispCodeTable[p->Menu.DispOffset % 10]);

    //当前处理位闪烁
    p->Display.CtrlReg.all = 1 << p->Menu.CtrlReg.bit.DealBit;
    p->Display.CtrlReg.bit.DotFlicker = 1;
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
static void PanelDispFun3rd(STR_PANELOPERRATION *p)
{
    static Uint16 Cnt = 0; 

    //如果是用户密码解密切换状态,显示-----
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT_SW)
    {
        SetDispData(p,DispCodeTable[DISPCODE_LINE],
                      DispCodeTable[DISPCODE_LINE],
                      DispCodeTable[DISPCODE_LINE],
                      DispCodeTable[DISPCODE_LINE],
                      DispCodeTable[DISPCODE_LINE]);
        //无闪烁
        p->Display.CtrlReg.all = 0;
        return;
    }

    //如果是显示属性,那么每隔1s更新显示值
    if(GetAttrib_Writable(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_DISP_WRT) 
    {
        Cnt ++;
        if(Cnt >= PanelDisCnt)
        {
            Cnt = 0;
            if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD) 
            {   //32位数据
                p->Menu.DispData = A_SHIFT16_PLUS_B(GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset + 1),
                                                    GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset));
            }
            else
            {   //16位数据
                p->Menu.DispData = (Uint32)GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset);
                p->Menu.DispData &= 0xFFFF;
                //如果是负数,高十六位置1
                if((GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN) &&
                  (p->Menu.DispData > 32767))
                {
                    p->Menu.DispData |= 0xFFFF0000;
                }
            }
        }
        else if( ((p->Menu.DispGroup == 0x0B) && (p->Menu.DispOffset == 0x03)) ||
                 ((p->Menu.DispGroup == 0x0B) && (p->Menu.DispOffset == 0x05)) )
        {
            p->Menu.DispData = (Uint32)GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset);
            p->Menu.DispData &= 0xFFFF;
            //如果是负数,高十六位置1
           if((GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN) &&
              (p->Menu.DispData > 32767))
            {
                p->Menu.DispData |= 0xFFFF0000;
            }
        }
    }

    //当第三级菜单的模式为RUNREAD或NORMALMODE时,如果当前操作的功能码是停机设定属性,需要实时更新第三级菜单的模式
    if((p->Menu.CtrlReg.bit.Class3rdMode == RUNREAD) || (p->Menu.CtrlReg.bit.Class3rdMode == NORMALMODE))
    {
       if(GetAttrib_Writable(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_POSD_WRT)
       {
            if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
            {
                p->Menu.CtrlReg.bit.Class3rdMode = RUNREAD;     //伺服运行状态只读,不可以更改
            }
            else
            {
                p->Menu.CtrlReg.bit.Class3rdMode = NORMALMODE;  
            }
        }

        //低频辨识过程中不允许修改
        if((p->Menu.DispGroup == 0x09) && ((p->Menu.DispOffset == 38) || (p->Menu.DispOffset == 39)))
        {
            if(FunCodeUnion.code.AT_LowOscMod == 1)
            {
                p->Menu.CtrlReg.bit.Class3rdMode = RUNREAD;     //伺服运行状态只读,不可以更改
            }
            else
            {
                p->Menu.CtrlReg.bit.Class3rdMode = NORMALMODE;
            }
        }
    }

    switch( GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) )
    {
        case ATTRIB_YNUM_TYP://十进制显示
            DecDisplay(p);
            break;

        case ATTRIB_HNUM_TYP: //十六进制显示 
            HexDisplay(p);
            if((p->Menu.DispGroup == 0x0B) && (p->Menu.DispOffset == 34))
            {
                //前两个管显示Er。
                p->Display.DispData[0] = DispCodeTable[DISPCODE_E];
                p->Display.DispData[1] = DispCodeTable[DISPCODE_r] & DispCodeTable[DISPCODE_DOT];
            }
            break;

        case ATTRIB_NNUM_TYP: //二进制显示,DI1-10位,DO-8位
            BinDisplay(p);
            break;
    }

    p->Display.CtrlReg.all = 0x00;

    //如果是正常模式,当前处理位闪烁
    if((p->Menu.CtrlReg.bit.Class3rdMode == NORMALMODE) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT))
    {
        p->Display.CtrlReg.all = 1 << p->Menu.CtrlReg.bit.DealBit;
    }

    //如果翻页,第0位闪烁显示页符号
    if((GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset) > 5) && 
       (GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) != ATTRIB_NNUM_TYP))
    {
        p->Display.CtrlReg.bit.ZeroFlicker = 1;
    }

    //小数点永远都不闪烁
    p->Display.CtrlReg.bit.DotFlicker = 1;
}
/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 分成有无符号,有无小数点
    2. 2^32 = 42 949 67296
    3. 显示位数包括符号位
********************************************************************************/
Static_Inline void DecDisplay(STR_PANELOPERRATION *p)
{
    Uint8   DispBits_DotPos = 0;
    Uint8   DispDataSign = 0;
    Uint8   DispPages = 0;
    Uint32  AbsDispData = 0;

    DispBits_DotPos = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);
    if(DispBits_DotPos == 0) DispBits_DotPos = 1;

    //显示页数判断
    if(DispBits_DotPos < 6)
    {
        DispPages = 1;
    }
    else if(DispBits_DotPos < 9)
    {
        DispPages = 2;
    }
    else
    {
        DispPages = 3;
    }

    //取显示数据绝对值
    AbsDispData = p->Menu.DispData;

    if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
    {
        //符号判断
        if((int32)AbsDispData < 0)
        {
            DispDataSign = 1;
            AbsDispData = 0 - AbsDispData;
        }

        //去掉符号位
        DispBits_DotPos = DispBits_DotPos - 1;
    }

    switch(DispPages)
    {
        case 1: //++++++++++++++++++++++++++++++++需要1页显示++++++++++++++++++++++++++++++++
            if(DispDataSign)
            { 
                p->Display.DispData[4 - DispBits_DotPos] = DispCodeTable[DISPCODE_LINE];     //如果是负数 显示负号
            }

            AbsDispData = AbsDispData % DecDispMax[DispBits_DotPos -1];
            DispDecCoder(p, DispBits_DotPos, AbsDispData);                              //显示字符
            break;

        case 2: //++++++++++++++++++++++++++++++++需要2页显示++++++++++++++++++++++++++++++++
            if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)
            {
                p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE_LO];               //第0页,第0个数码管闪烁显示低-

                if(DispDataSign) 
                {
                    p->Display.DispData[0] &= 0x7F;                                     //如果是负数 第0位显示小数点 
                }

                AbsDispData = AbsDispData % 10000;
                DispDecCoder(p, 4, AbsDispData);                                //显示字符
            }
            else
            {
                p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE];                  //第1页,第0个数码管闪烁显示中-

                if(DispDataSign) 
                {
                    p->Display.DispData[8 - DispBits_DotPos] = DispCodeTable[DISPCODE_LINE];   //如果是负数且第9-DispBits位显示负号
                }

                AbsDispData = AbsDispData % DecDispMax[DispBits_DotPos -1];
                DispDecCoder(p, DispBits_DotPos - 4, AbsDispData / 10000);                    //显示字符
            }
            break;

        case 3: //++++++++++++++++++++++++++++++++需要3页显示++++++++++++++++++++++++++++++++
            if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)
            {
                p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE_LO];               //第0页,第0个数码管闪烁显示低-
                if(DispDataSign) 
                {
                    p->Display.DispData[0] &= 0x7F;                                     //如果是负数 第0位显示小数点
                }
                DispDecCoder(p, 4, AbsDispData % 10000);                                //显示字符
            }
            else if(p->Menu.CtrlReg.bit.Page == PAGE_1ST)
            {
                p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE];                  //第1页,第0个数码管闪烁显示中-
                if(DispDataSign) 
                {
                    p->Display.DispData[0] &= 0x7F;                                     //如果是负数 第0位显示小数点
                }
                AbsDispData = AbsDispData %100000000;
                DispDecCoder(p, 4, AbsDispData / 10000);                                  //显示字符
            }
            else if(p->Menu.CtrlReg.bit.Page == PAGE_2ND)
            {
                p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE_UP];               //第2页,第0个数码管闪烁显示上-
                if(DispDataSign) 
                {
                    p->Display.DispData[12 - DispBits_DotPos] = DispCodeTable[DISPCODE_LINE];     //如果是负数 第12 - DispBits位显示负号
                }
                AbsDispData = AbsDispData / 100000000;
                DispDecCoder(p, DispBits_DotPos - 8, AbsDispData);  //显示字符
            }
            break;
    }

    //++++++++++++++++++++++++++++++++小数点显示++++++++++++++++++++++++++++++++
    //小数点只能支持4位,在第0页显示
    DispBits_DotPos = GetAttrib_DotBit(p->Menu.DispGroup , p->Menu.DispOffset);
    if((p->Menu.CtrlReg.bit.Page == PAGE_ZERO) && (DispBits_DotPos > 0))
    {
        p->Display.DispData[4 - DispBits_DotPos] &= 0x7F;
    }
}
/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 无符号数
    2.
********************************************************************************/
Static_Inline void DispDecCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData)
{
    Uint32 Quotient = 0;

    switch( DispBits )
    {
        //显示字符
        case 5:
            Quotient = DispData / 10000;
            DispData = DispData - Quotient * 10000;
            p->Display.DispData[0] = DispCodeTable[Quotient];

        case 4:
            Quotient = DispData / 1000;
            DispData = DispData - Quotient * 1000;
            p->Display.DispData[1] = DispCodeTable[Quotient];
        case 3:
            Quotient = DispData / 100;
            DispData = DispData - Quotient * 100;
            p->Display.DispData[2] = DispCodeTable[Quotient];

        case 2:
            Quotient = DispData / 10;
            DispData = DispData - Quotient * 10;
            p->Display.DispData[3] = DispCodeTable[Quotient];

        case 1:
            p->Display.DispData[4] = DispCodeTable[DispData];
    }
}

/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 无符号数
    2.
********************************************************************************/
Static_Inline void HexDisplay(STR_PANELOPERRATION *p)
{
    Uint8   DispBits = 0; 
    Uint8   DispBits_DotPos = 0;

    DispBits = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);
    if(DispBits == 0) DispBits = 1;

    if(DispBits > 5) 
    {
        //需要翻页显示
        if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)
        {
            //第0页,第0个数码管闪烁显示低-
            p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE_LO];

            //显示字符
            DispHexCoder(p, 4, p->Menu.DispData & 0xFFFF);
        }
        else if(p->Menu.CtrlReg.bit.Page == PAGE_1ST)
        {
            //第0个数码管闪烁显示中-
            p->Display.DispData[0] = DispCodeTable[DISPCODE_LINE];

            //显示字符
            if(DispBits == 8)
            {
                DispHexCoder(p, DispBits - 4, (p->Menu.DispData >> 16));
            }
            else
            {
                DispHexCoder(p, DispBits - 4, (p->Menu.DispData % HexDispMax[DispBits - 1]) >> 16);
            }
        }
    }
    else
    {
        //显示字符
        DispHexCoder(p, DispBits, p->Menu.DispData % HexDispMax[DispBits - 1] );
    }

    //++++++++++++++++++++++++++++++++小数点显示++++++++++++++++++++++++++++++++
    //小数点只能支持4位,在第0页显示
    DispBits_DotPos = GetAttrib_DotBit(p->Menu.DispGroup , p->Menu.DispOffset);
    if((p->Menu.CtrlReg.bit.Page == PAGE_ZERO) && (DispBits_DotPos > 0))
    {
        p->Display.DispData[4 - DispBits_DotPos] &= 0x7F;
    }
}

/*******************************************************************************
  函数名:    
  输入:    
  输出:   无 
  子函数: 无
  描述：
    1. 无符号数
    2.
********************************************************************************/
Static_Inline void DispHexCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData)
{
    switch( DispBits )
    {
        //显示字符
        case 5:
            p->Display.DispData[0] = DispCodeTable[(DispData >> 16) & 0x0F];
    
        case 4:
            p->Display.DispData[1] = DispCodeTable[(DispData >> 12) & 0x0F];
    
        case 3:
            p->Display.DispData[2] = DispCodeTable[(DispData >> 8) & 0x0F];
    
        case 2:
            p->Display.DispData[3] = DispCodeTable[(DispData >> 4) & 0x0F];
    
        case 1:
            p->Display.DispData[4] = DispCodeTable[DispData & 0x0F];
    }
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
Static_Inline void BinDisplay(STR_PANELOPERRATION *p)
{
    Uint8   DispBits = 0; 

    DispBits = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);

    if(DispBits == 0) DispBits = 1;
    if(DispBits > 10) DispBits = 10;

    //显示字符
    DispBinCoder(p, DispBits,p->Menu.DispData % BinDispMax[DispBits - 1]);
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
Static_Inline void DispBinCoder(STR_PANELOPERRATION *p, Uint8 DispBits, Uint32 DispData)
{
    //如果显示位数为奇数,先处理最高位
    switch( DispBits )
    {
        //显示字符
        case 9:
            p->Display.DispData[0] = DispBinaryTable_High[(DispData >> 8) & 0x01];
            break;

        case 7:
            p->Display.DispData[1] = DispBinaryTable_High[(DispData >> 6) & 0x01];
            break;

        case 5:
            p->Display.DispData[2] = DispBinaryTable_High[(DispData >> 4) & 0x01];
            break;

        case 3:
            p->Display.DispData[3] = DispBinaryTable_High[(DispData >> 2) & 0x01];
            break;

        case 1:
            p->Display.DispData[4] = DispBinaryTable_High[DispData & 0x01];
            break;
    }

    DispBits = DispBits >> 1;

    switch( DispBits )
    {
        //显示字符
        case 5:
            p->Display.DispData[0] = DispBinaryTable[(DispData >> 8) & 0x03];

        case 4:
            p->Display.DispData[1] = DispBinaryTable[(DispData >> 6) & 0x03];

        case 3:
            p->Display.DispData[2] = DispBinaryTable[(DispData >> 4) & 0x03];

        case 2:
            p->Display.DispData[3] = DispBinaryTable[(DispData >> 2) & 0x03];

        case 1:
            p->Display.DispData[4] = DispBinaryTable[DispData & 0x03];
    }
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
static void PanelDispFun4th(STR_PANELOPERRATION *p)
{
    //无闪烁
    p->Display.CtrlReg.all = 0;

    switch(p->Menu.CtrlReg.bit.Class3rdMode)
    {
        case NORMALMODE:    //显示DONE

			if(AuxFunCodeUnion.code.FA_AutoTune == 1)    //正在调整时需要闪烁
			{
                SetDispData(p,DispCodeTable[DISPCODE_T],DispCodeTable[DISPCODE_u],DispCodeTable[DISPCODE_n],
                             DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_NULL]);
			    
				p->Display.CtrlReg.all = 0x0F;	   //全部闪烁		    			    
			}
			else		   //辨识结束或未发生辨识时
			{
			    if((AuxFunCodeUnion.code.H2F_AutoTuneStats & 0x80)!=0)	 //辨识失败
				{
                    SetDispData(p,DispCodeTable[DISPCODE_F],DispCodeTable[DISPCODE_A],DispCodeTable[DISPCODE_1],
                                 DispCodeTable[DISPCODE_L],DispCodeTable[DISPCODE_NULL]);				    
				}
				else   //辨识成功或未辨识
				{
                    SetDispData(p,DispCodeTable[DISPCODE_d],DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_n],
                                 DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_NULL]);				    
				}
			}
            break;
        case READONLY:      //显示RONLY
            SetDispData(p,DispCodeTable[DISPCODE_r],DispCodeTable[DISPCODE_o],
                          DispCodeTable[DISPCODE_n],DispCodeTable[DISPCODE_L],DispCodeTable[DISPCODE_Y]);
            break;
        case RUNREAD :      //显示S_on
            SetDispData(p,DispCodeTable[DISPCODE_5],DispCodeTable[DISPCODE_LINE_LO],
                          DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_n],DispCodeTable[DISPCODE_NULL]);
            break;
//        case USERLOCK:       //显示LOCKD
//            SetDispData(p,DispCodeTable[DISPCODE_L],DispCodeTable[DISPCODE_o],
//                          DispCodeTable[DISPCODE_C],DispCodeTable[DISPCODE_K],DispCodeTable[DISPCODE_d]);
//            break;
        case USERLOCK:       //显示-----
            SetDispData(p,DispCodeTable[DISPCODE_LINE],DispCodeTable[DISPCODE_LINE],
                          DispCodeTable[DISPCODE_LINE],DispCodeTable[DISPCODE_LINE],DispCodeTable[DISPCODE_LINE]);
            break;
        case USERDECRYPT:
            if(p->Menu.UserPass == FunCodeUnion.code.BP_UserPass_Rsvd)
            {
                SetDispData(p,DispCodeTable[DISPCODE_d],DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_n],
                             DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_NULL]);
            }
            else
            {
                //显示Error
                SetDispData(p,DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_r],
                              DispCodeTable[DISPCODE_r],DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_r]);
            }
            break;

		default:
            break;
    }
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
static void PanelDispFun5th(STR_PANELOPERRATION *p)
{
    //JOG显示
    SetDispData(p,DispCodeTable[DISPCODE_J],DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_9],
                  DispCodeTable[DISPCODE_NULL],DispCodeTable[DISPCODE_NULL]);

    //不需要全部闪烁
    p->Display.CtrlReg.all = 0x00;
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
static void PanelDispFun6th(STR_PANELOPERRATION *p)
{
    //显示辨识结果
    p->Menu.DispGroup = 0x08;
    p->Menu.DispOffset =15;
    p->Menu.DispData = AuxFunCodeUnion.code.OnLineInertiaRatio;
    //十进制显示
    DecDisplay(p);
    //不需要全部闪烁
    p->Display.CtrlReg.all = 0x00;
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
static void PanelDispFun7th(STR_PANELOPERRATION *p)
{
    
    if(STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1)
    {
        //显示PI
        SetDispData(p,DispCodeTable[DISPCODE_P],DispCodeTable[DISPCODE_1],DispCodeTable[DISPCODE_NULL],
                      DispCodeTable[DISPCODE_NULL],DispCodeTable[STR_FUNC_Gvar.ToqPiTune.Step]);
    }
    else
    {
        //显示DonE
        SetDispData(p,DispCodeTable[DISPCODE_d],DispCodeTable[DISPCODE_o],DispCodeTable[DISPCODE_n],
                      DispCodeTable[DISPCODE_E],DispCodeTable[DISPCODE_NULL]);
    }


    //不需要全部闪烁
    p->Display.CtrlReg.all = 0x00;
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
static void PanelDispFun8th(STR_PANELOPERRATION *p)
{
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
Static_Inline void SetDispData(STR_PANELOPERRATION *p, Uint8 TubeData0, Uint8 TubeData1st, Uint8 TubeData2nd, Uint8 TubeData3rd, Uint8 TubeData4th)
{
    p->Display.DispData[0] &= (Uint8)TubeData0;
    p->Display.DispData[1] &= (Uint8)TubeData1st;
    p->Display.DispData[2] &= (Uint8)TubeData2nd;
    p->Display.DispData[3] &= (Uint8)TubeData3rd;
    p->Display.DispData[4] &= (Uint8)TubeData4th; 
}


/********************************* END OF FILE *********************************/
