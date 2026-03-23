/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_DiDo.c
 创建人：童文邹                
 修改人：李浩               创建日期：11.11.18 
 描述： 1. 
        2.
		3.
 修改记录：  
     1）xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2) xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"

#include "FUNC_DiDo.h"
#include "FUNC_SpiDriver_ST.h"
//#ifdef  VIRTUAL_DI_ENABLE
#include "FUNC_VirtualDiDo.h"
//#endif
//#ifdef  VIRTUAL_DO_ENABLE
#include "FUNC_VirtualDiDo.h"
//#endif

#include "FUNC_ErrorCode.h" 

#include "FUNC_GPIODriver.h"

#include "CANopen_DeviceCtrl.h"
//#include "FUNC_XIntPosition.h"
#include "CANopen_PP.h"
#include "ECT_CSP.h"
#include "ECT_Probe.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/




/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

STR_DI_STATE   STR_DiState;

STR_DO_STATE   STR_DoState;

UNI_DI_PORT    UNI_InitSonDiPort;
UNI_DI_PORT    UNI_StopSonDiPort;

//#ifdef  VIRTUAL_DI_ENABLE
STR_VIR_DI_STATE     STR_VirDiState;

UNI_VIR_DI_PORT      UNI_InitSonVirDiPort;
UNI_VIR_DI_PORT      UNI_StopSonVirDiPort;
//#endif
//#ifdef  VIRTUAL_DO_ENABLE
STR_VIR_DO_STATE   STR_VirDoState;
//#endif


UNI_DOVARREG        UNI_DovarReg;

volatile Uint64     DivarReg = 0;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

//普通10个Di端口功能选择地址表
Uint16 * const DiPortFunSelAddList[9] =
{
    //功能码地址表
    (Uint16 *) &FunCodeUnion.code.DIFuncSel1,              //H0302 Di1 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel2,              //H0304 Di2 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel3,              //H0306 Di3 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel4,              //H0308 Di4 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel5,              //H03010 Di5 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel6,              //H03012 Di6 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel7,              //H03014 Di7 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel8,              //H03016 Di8 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DIFuncSel9,              //H03018 Di9 端子功能选择
};


//普通8个Do端口功能选择地址表
Uint16 * const DoPortFunSelAddList[5] =
{
    //功能码地址表
    (Uint16 *) &FunCodeUnion.code.DOFuncSel1,              //H0400 Do1 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DOFuncSel2,              //H0402 Do2 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DOFuncSel3,              //H0404 Do3 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DOFuncSel4,              //H0406 Do4 端子功能选择
    (Uint16 *) &FunCodeUnion.code.DOFuncSel5,              //H0408 Do5 端子功能选择
};


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void InitDiDoState(void);
void DiProcess(void);
void DoProcess(void);
void DoAllocateError(void);
void DiAllocateError(void);
void DiDoStateStopUpdata(void);
void DealDiFromGpio(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void InitDiFuncAndLogic(void);
void InitDoFuncAndLogic(void);

Static_Inline void DiFuncSetErr(STR_DI_STATE *p);
Static_Inline void DoFuncSetErr(STR_DO_STATE *p);

Static_Inline void DealDiFromSpi(STR_DI_STATE *p);

Static_Inline void DiPortChangeCheck(void);

Static_Inline void DovarRegEnableGather(void);

/*******************************************************************************
  函数名: void InitDiDoState ()
  输入:                 
  输出:     
  子函数: 
  描述: DiDo初始化       
  返回值:  
********************************************************************************/ 
void InitDiDoState()
{
    Uint16 Temp = 0;
    volatile Uint16 *ExtDiFiltTm = (Uint16 *)(FPGA_BASE + (0x0000000C << 1));   //外部Di滤波配置

    Uint8 i = 0;
    /*注意下面两个函数调用先后时序不能颠倒*/
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        STR_DiState.XIntPosEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9; //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
    	STR_FUNC_Gvar.DiDoOutput.XintPosEn     = FunCodeUnion.code.PL_XintPosSel ;
    }
    else
    {
        STR_DiState.TouchProbeEnDi8LogicLatch = FunCodeUnion.code.DILogicSel8;
        STR_DiState.TouchProbeEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9;
        
        STR_FUNC_Gvar.DiDoOutput.TouchProbe1En = (FunCodeUnion.code.DIFuncSel8 == 38)?1:0;
        STR_FUNC_Gvar.DiDoOutput.TouchProbe2En = (FunCodeUnion.code.DIFuncSel9 == 39)?1:0;
    }
#endif


    DiAllocateError();         //Di功能分配故障检测
    DoAllocateError();         //Do功能分配故障检测

    InitDiFuncAndLogic();       //9个Di端口的功能分配，以及Di有效方式(电平或边沿)选择
    
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        STR_DiState.XIntPosEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9; //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
    }
    else
    {
        STR_DiState.TouchProbeEnDi8LogicLatch = FunCodeUnion.code.DILogicSel8;
        STR_DiState.TouchProbeEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9;
    }
#endif

    for(i=0;i<9;i++)  //锁存上电初始化后Son的Di端口分配号，用于检查rdy之后Son的Di端口分配是否变化时的检查用
    {
        if( STR_DiState.DiFuncSel[i] == 1 ) 
        {
            UNI_InitSonDiPort.all |= ( 1 << (i+1) );
        }
    }

    STR_DiState.SpiDiValue = UNI_SpiDiReg.all;    //Di1 ~ Di7 为由Spi输入，因此在初始化中应存储由Spi输入的Di1~Di8的状态
    STR_DiState.SpiDiValueLast = STR_DiState.SpiDiValue; 

    //与620P反相
    STR_DiState.GpioDi8Value = (Uint8)GPIO_ReadDI8DSP();   //Di8 为由GPIO输入，因此在初始化中应存储由GPIO输入的Di8状态
    STR_DiState.GpioDi9Value = (Uint8)GPIO_ReadDI9DSP();  //Di9 为由GPIO输入，因此在初始化中应存储由GPIO输入的Di9状态


    /*Di功能使能标志清零，Di未作用*/
    DivarReg = 0;
    DivarReg |= ( (Uint64)FunCodeUnion.code.DIDeftL + ((Uint64)FunCodeUnion.code.DIDeftH<<16) + 
                                   ((Uint64)FunCodeUnion.code.DIDeftHL<<32) + ((Uint64)FunCodeUnion.code.DIDeftHH<<48));


    //37个DI功能: 1 1010 1100 0111 0000 0111 1111 1000 0000 0110
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))

    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        DivarReg &= 0xFFFFFFFFFFFFFFFD;   //此条语句多余  
    }
    else
    {
        DivarReg &= 0x0000001AC707F806;//注意此条语句在620N中不多余
    }
#endif
    STR_FUNC_Gvar.DivarRegLw.all = (Uint32)DivarReg;

    STR_FUNC_Gvar.DivarRegHi.all = (Uint32)(DivarReg>>32);
    STR_FUNC_Gvar.SoftIntDivarRegLw.all = (Uint32)DivarReg;

    STR_FUNC_Gvar.SoftIntDivarRegHi.all = (Uint32)(DivarReg >> 32);

    /*5个Do端口的Do功能和逻辑电平选择，以及Do功能使能标志清零，Do未作用*/
    InitDoFuncAndLogic();


//#ifdef  VIRTUAL_DI_ENABLE
    InitVirDiFuncAndLogic(&STR_VirDiState);  //16个虚拟Di端口的功能分配，以及虚拟Do的有效方式(高低电平)选择

    AuxFunCodeUnion.code.CC_VDILevel = FunCodeUnion.code.CM_VDIDefaultValue; //由于H3100功能码不存在Eeprom，所以先通过H0C10功能码对H3100功能码进行初始化，防止上电开始时的H3100电平错误
    STR_VirDiState.VirDiStateNew.all = AuxFunCodeUnion.code.CC_VDILevel;  //对虚拟Di端口的逻辑电平进行初始化
    STR_VirDiState.VirDiStateOld.all = STR_VirDiState.VirDiStateNew.all;

    for(i=0;i<16;i++)
    {
        if( STR_VirDiState.VirDiFuncSel[i] == 1 )
        {
            UNI_InitSonVirDiPort.all |= ( 1 << i );
        }
    }
//#endif

//#ifdef  VIRTUAL_DO_ENABLE
    InitVirDoFuncAndLogic(&STR_VirDoState);  //初始化虚拟的16个Do端口的Do功能和逻辑电平选择
//#endif

    //620P  高速Di8和Di9滤波配置
     Temp = (FunCodeUnion.code.ER_Di9FiltTm<<8);
     Temp = (Temp | FunCodeUnion.code.ER_Di8FiltTm);
    *ExtDiFiltTm = Temp;
    
#if ECT_ENABLE_SWITCH
    ObjectDictionaryStandard.ComEntryDIDO.DigitalInputs.all = 0;
    union_OmronInside.all = 0;
#endif
}


/*******************************************************************************
  函数名: DiDoStateStopUpdata()
  输入:               
  输出:   
  子函数: 
  描述:  DiDo分配停机更新函数   
  返回值:  
********************************************************************************/ 
void DiDoStateStopUpdata()
{   
    DiAllocateError();

#if ECT_ENABLE_SWITCH
    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        DiPortChangeCheck();
    }
    else
    {
        STR_DiState.DiPortChangeEn       = 0;
        STR_VirDiState.VirDiPortChangeEn = 0;
    }
#else
    DiPortChangeCheck();
#endif

    if ( STR_DiState.DiPortChangeEn == 0 )
    {              
        InitDiFuncAndLogic();       //9个Di端口的功能分配，以及Di有效方式(电平或边沿)选择        
    }

    //#ifdef  VIRTUAL_DI_ENABLE
    if ( STR_VirDiState.VirDiPortChangeEn == 0 )
    {
        InitVirDiFuncAndLogic(&STR_VirDiState);  //16个虚拟Di端口的功能分配，以及虚拟Do的有效方式(高低电平)选择
    } 
    //#endif 
    
    //DoAllocateError(); 

    /*5个Do端口的Do功能和逻辑电平选择，以及Do功能使能标志清零，Do未作用*/
    InitDoFuncAndLogic();
    //#ifdef  VIRTUAL_DO_ENABLE
    InitVirDoFuncAndLogic(&STR_VirDoState);  //初始化虚拟的16个Do端口的Do功能和逻辑电平选择
    //#endif  
}




/*******************************************************************************
  函数名: void InitDiFuncAndLogic() 
  输入:   功能码              
  输出:   9个Di端口的功能选择，以及有效的逻辑电平选择  
  子函数: 无
  描述:  根据功能码的值对10个Di端口的Di功能进行选择，以及根据功能码对Di信号有效的逻辑方式
        (即电平，边沿有效选择)进行选择，同时，若/ALM_RST被选择，则选择其边沿有效的方式
  返回值:  
********************************************************************************/ 
void InitDiFuncAndLogic()
{
    Uint8 i = 0; 
    /*根据功能码对9个Di端子进行34个Di功能中的一个进行选择*/
    STR_DiState.DiFuncSel[0] = FunCodeUnion.code.DIFuncSel1;
    STR_DiState.DiFuncSel[1] = FunCodeUnion.code.DIFuncSel2;
    STR_DiState.DiFuncSel[2] = FunCodeUnion.code.DIFuncSel3;
    STR_DiState.DiFuncSel[3] = FunCodeUnion.code.DIFuncSel4;
    STR_DiState.DiFuncSel[4] = FunCodeUnion.code.DIFuncSel5;
    STR_DiState.DiFuncSel[5] = FunCodeUnion.code.DIFuncSel6;
    STR_DiState.DiFuncSel[6] = FunCodeUnion.code.DIFuncSel7;
    STR_DiState.DiFuncSel[7] = FunCodeUnion.code.DIFuncSel8;
    STR_DiState.DiFuncSel[8] = FunCodeUnion.code.DIFuncSel9;
    /*根据功能码对9个Di端子的逻辑有效的电平或边缘进行选择*/
    STR_DiState.DiLogicSel[0] = FunCodeUnion.code.DILogicSel1;
    STR_DiState.DiLogicSel[1] = FunCodeUnion.code.DILogicSel2;
    STR_DiState.DiLogicSel[2] = FunCodeUnion.code.DILogicSel3;
    STR_DiState.DiLogicSel[3] = FunCodeUnion.code.DILogicSel4;
    STR_DiState.DiLogicSel[4] = FunCodeUnion.code.DILogicSel5;
    STR_DiState.DiLogicSel[5] = FunCodeUnion.code.DILogicSel6;
    STR_DiState.DiLogicSel[6] = FunCodeUnion.code.DILogicSel7;
    STR_DiState.DiLogicSel[7] = FunCodeUnion.code.DILogicSel8;
    STR_DiState.DiLogicSel[8] = FunCodeUnion.code.DILogicSel9;

    for(i=0;i<9;i++)
    {
        if(STR_DiState.DiFuncSel[i] == 2)   //如果警报复位(/ALM_RST)的Di功能在10个Di端口中被选择，则其应选为边沿有效
        {
            STR_DiState.DiLogicSel[i] = (STR_DiState.DiLogicSel[i] < 2)? 4 : STR_DiState.DiLogicSel[i];//0和1时，强制为上升，下降沿有效
        }

        if((STR_DiState.DiFuncSel[i] == 31) || (STR_DiState.DiFuncSel[i] == 33))  //如果原点开关(OrgNear)的Di功能在10个Di端口中被选择，则其应选为电平有效
        {                                                                         //如果中断定长禁止(XintInhibit)的Di功能在10个Di端口中被选择，则其应选为电平有效
            if(STR_DiState.DiLogicSel[i] == 2)  //上升沿有效时，改为高电平有效
            {
                STR_DiState.DiLogicSel[i] = 1;
            }
            else if (STR_DiState.DiLogicSel[i] == 3) //下降沿有效时，改为低电平有效
            {   
                STR_DiState.DiLogicSel[i] = 0;
            }
            else if (STR_DiState.DiLogicSel[i] == 4) //上升沿/下降沿都有效时，改为低电平有效
            {   
                STR_DiState.DiLogicSel[i] = 0;
            }
        }
    }
}



/*******************************************************************************
  函数名: void InitDoFuncAndLogic()
  输入:   功能码             
  输出:   8个Do端口的功能选择，以及有效的逻辑电平选择  
  子函数: 无
  描述:   根据功能码的值对8个Do端口的Do功能以及根据功能码对Do逻辑电平进行初始化     
  返回值:  
********************************************************************************/ 
void InitDoFuncAndLogic()
{
    /*根据功能码对5个Do端子进行18个Do功能中的一个进行选择*/ 
    STR_DoState.DoFuncSel[0] = FunCodeUnion.code.DOFuncSel1;
    STR_DoState.DoFuncSel[1] = FunCodeUnion.code.DOFuncSel2;
    STR_DoState.DoFuncSel[2] = FunCodeUnion.code.DOFuncSel3;
    STR_DoState.DoFuncSel[3] = FunCodeUnion.code.DOFuncSel4;
    STR_DoState.DoFuncSel[4] = FunCodeUnion.code.DOFuncSel5;

    /*根据功能码对5个Do端子的逻辑有效的电平或边缘进行选择*/
    STR_DoState.DoLogicSel[0] = FunCodeUnion.code.DOLogicSel1;
    STR_DoState.DoLogicSel[1] = FunCodeUnion.code.DOLogicSel2;
    STR_DoState.DoLogicSel[2] = FunCodeUnion.code.DOLogicSel3;
    STR_DoState.DoLogicSel[3] = FunCodeUnion.code.DOLogicSel4;
    STR_DoState.DoLogicSel[4] = FunCodeUnion.code.DOLogicSel5;
}


/*******************************************************************************
  函数名: void DiAllocateError()
  输入:                 
  输出:     
  子函数: DiFuncSetErr(&STR_DiState)；VirDiFuncSetErr(&STR_VirDiState)
  描述:   该函数主要是判断普通Di和虚拟Di的端口分配相同的Di功能时，进行故障报警     
  返回值:  
********************************************************************************/ 
void DiAllocateError()
{   
    Uint8 i = 0;
    Uint8 j = 0;

    DiFuncSetErr(&STR_DiState); //9个Di端口功能分配错误，进行故障处理

//#ifdef  VIRTUAL_DI_ENABLE
    
    VirDiFuncSetErr(&STR_VirDiState); //16个Di端口功能分配错误，进行故障处理

    //if (FunCodeUnion.code.CM_UseVDI == 1)  //只有虚拟Di使能的情况才进行普通Di和虚拟Di的功能是否重复，故障报警
    //{
        for (i=0;i<16;i++)  //虚拟Di和普通Di的端口分配的Di的功能相同时，进行故障处理
        {
            for(j=0;j<9;j++)
            {
                if( ((*DiPortFunSelAddList[j]) != 0)&&
                    ((*VirtualDiPortFunSelAddList[i]) != 0)&&
                    ((*VirtualDiPortFunSelAddList[i]) == (*DiPortFunSelAddList[j]))
                  )
                {
                    if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
                    {
                        PostErrMsg(DILOADERR);
                    }
                    return;
                }
            }
        }
    //}
//#endif
}


/*******************************************************************************
  函数名: void DiFuncSetErr(STR_DI_STATE *p)
  输入:   STR_DiState.DiFuncSel[i]              
  输出:   DILOADERR，故障码，报错处理  
  子函数: PostErrMsg(DILOADERR)
  描述:   1.Di功能选择超出可选范围时，进行报错处理
          2.在32个Di功能中，若其中一个Di功能被多次分配到不同的Di端口上，则进行报错处理     
  返回值:  
********************************************************************************/ 
Static_Inline void DiFuncSetErr(STR_DI_STATE *p)
{
    Uint8 i = 0;
    Uint8 j = 0;

    
    for(i=0;i<9;i++)
    {   /* Di功能分配超限报错 */
        if((*DiPortFunSelAddList[i]) > DI_FUNC_SEL_MAX)    //Di功能选择超出可选范围报错处理，防溢出检测 
        {                                           
            if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
            {
                PostErrMsg(DILOADERR);
            }
            return;
        }
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
        if(FunCodeUnion.code.OEM_LocalModeEn==1)
        {
        }
        else
        {
            if( ((*DiPortFunSelAddList[i]) == 1)
                ||((*DiPortFunSelAddList[i]) == 4)
                ||((*DiPortFunSelAddList[i]) == 5)
                ||((*DiPortFunSelAddList[i]) == 6)
                ||((*DiPortFunSelAddList[i]) == 7)
                ||((*DiPortFunSelAddList[i]) == 8)
                ||((*DiPortFunSelAddList[i]) == 9)
                ||((*DiPortFunSelAddList[i]) == 10)
                ||((*DiPortFunSelAddList[i]) == 11)
                ||((*DiPortFunSelAddList[i]) == 13)
                ||((*DiPortFunSelAddList[i]) == 20)
                ||((*DiPortFunSelAddList[i]) == 21)
                ||((*DiPortFunSelAddList[i]) == 22)
                ||((*DiPortFunSelAddList[i]) == 23)
                ||((*DiPortFunSelAddList[i]) == 24)
                ||((*DiPortFunSelAddList[i]) == 28)
                ||((*DiPortFunSelAddList[i]) == 29)
                ||((*DiPortFunSelAddList[i]) == 30)
                ||((*DiPortFunSelAddList[i]) == 33)
                )    //Di功能选择超出可选范围报错处理，防溢出检测 
            {                                           
                if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
                {
                    PostErrMsg(DILOADERR);
                }
                return;
            }
        }
#endif
        /* Di功能分配重复报错 */
        if( ( (*DiPortFunSelAddList[i]) > 0) && ( (*DiPortFunSelAddList[i]) < (DI_FUNC_SEL_MAX + 1) ) )  //在34个Di功能中，若其中一个Di功能被多次分配到不同的Di端口上，则进行报错处理
        {                                           
            for(j=0;j<i;j++)                        
            {
                if((*DiPortFunSelAddList[i]) == (*DiPortFunSelAddList[j]))            
                {                                       
                    if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
                    {
                        PostErrMsg(DILOADERR);
                    }
                    return;
                }
            } 
        }
    }

    
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        /* 当中断定长使能(Di9硬件端口作为中断定长的中断信号输入口时)，Di9 又被选择其他Di功能时，应进行Di功能分配故障报错*/
        if( (STR_FUNC_Gvar.DiDoOutput.XintPosEn == 1) && ( (*DiPortFunSelAddList[8]) != 0) )   
        {
            if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
            {
                PostErrMsg(DILOADERR); 
            }
            return;
        }
        //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
        if( STR_FUNC_Gvar.DiDoOutput.XintPosEn == 1 )
        {
            if (STR_DiState.XIntPosEnDi9LogicLatch != FunCodeUnion.code.DILogicSel9) 
            {
                PostErrMsg(PCHGDWARN);
                STR_DiState.XIntPosEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9;    
            }
            return; 
        }
    }
    else
    {
        /* 当中断定长使能(Di9硬件端口作为中断定长的中断信号输入口时)，Di9 又被选择其他Di功能时，应进行Di功能分配故障报错*/
        if( (STR_FUNC_Gvar.DiDoOutput.TouchProbe1En == 0) && ( (*DiPortFunSelAddList[7]) == 38) )   
        {
            PostErrMsg(PCHGDWARN);
            return;
        }

        if( (STR_FUNC_Gvar.DiDoOutput.TouchProbe1En == 1) && ( (*DiPortFunSelAddList[7]) != 38) )   
        {
            if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
            {
                PostErrMsg(DILOADERR); 
            }
            return;
        }

        //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
        if( STR_FUNC_Gvar.DiDoOutput.TouchProbe1En == 1 )
        {
            if (STR_DiState.TouchProbeEnDi8LogicLatch != FunCodeUnion.code.DILogicSel8) 
            {
                PostErrMsg(PCHGDWARN);
                STR_DiState.TouchProbeEnDi8LogicLatch = FunCodeUnion.code.DILogicSel8;  
                return; 
            }
        }
        

        if( (STR_FUNC_Gvar.DiDoOutput.TouchProbe2En == 0) && ( (*DiPortFunSelAddList[8]) == 39) )   
        {
            PostErrMsg(PCHGDWARN);
            return;
        }

        /* 当中断定长使能(Di9硬件端口作为中断定长的中断信号输入口时)，Di9 又被选择其他Di功能时，应进行Di功能分配故障报错*/
        if( (STR_FUNC_Gvar.DiDoOutput.TouchProbe2En == 1) && ( (*DiPortFunSelAddList[8]) != 39) )   
        {
            if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
            {
                PostErrMsg(DILOADERR); 
            }
            return;
        }
        
        //中断定长使能时，Di9Logic电平选择锁存，此时发生变化时应报Er.941
        if( STR_FUNC_Gvar.DiDoOutput.TouchProbe2En == 1 )
        {
            if (STR_DiState.TouchProbeEnDi9LogicLatch != FunCodeUnion.code.DILogicSel9) 
            {
                PostErrMsg(PCHGDWARN);
                STR_DiState.TouchProbeEnDi9LogicLatch = FunCodeUnion.code.DILogicSel9;    
            }
            return; 
        }
    }
#endif
}

/*******************************************************************************
  函数名: void TouchProbe1andDIConflictError()
  输入:                 
  输出:     
  子函数: DoFuncSetErr(&STR_DiState)；VirDoFuncSetErr(&STR_VirDiState)
  描述:   该函数主要是判断普通Di和虚拟Di的端口分配相同的Di功能时，进行故障报警     
  返回值:  
********************************************************************************/ 
void TouchProbe1andDIConflictError()
{
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    //当探针功能使能，而DI8或者Di9被分配为其他DI功能时，应进行DI功能分配故障检测
    if((STR_FUNC_Gvar.DiDoOutput.TouchProbe1En == 1) && ((*DiPortFunSelAddList[7]) == 0))   
    {
        if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
        {
            PostErrMsg(DILOADERR); 
        }
    }
#endif

}

/*******************************************************************************
  函数名: void TouchProbe2andDIConflictError()
  输入:                 
  输出:     
  子函数: DoFuncSetErr(&STR_DiState)；VirDoFuncSetErr(&STR_VirDiState)
  描述:   该函数主要是判断普通Di和虚拟Di的端口分配相同的Di功能时，进行故障报警     
  返回值:  
********************************************************************************/ 
void TouchProbe2andDIConflictError()
{
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    //当探针功能使能，而DI8或者Di9被分配为其他DI功能时，应进行DI功能分配故障检测
    if((STR_FUNC_Gvar.DiDoOutput.TouchProbe2En == 1) && ( (*DiPortFunSelAddList[8]) == 0))   
    {
        if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DILOADERR)
        {
            PostErrMsg(DILOADERR); 
        }
    }
#endif

}

/*******************************************************************************
  函数名: void DoAllocateError()
  输入:                 
  输出:     
  子函数: DoFuncSetErr(&STR_DiState)；VirDoFuncSetErr(&STR_VirDiState)
  描述:   该函数主要是判断普通Di和虚拟Di的端口分配相同的Di功能时，进行故障报警     
  返回值:  
********************************************************************************/ 
void DoAllocateError()
{
    //Uint8 i = 0;
    //Uint8 j = 0;

    DoFuncSetErr(&STR_DoState);    //5个Do端口功能分配错误，进行故障处理

//#ifdef  VIRTUAL_DO_ENABLE
    VirDoFuncSetErr(&STR_VirDoState); //16个虚拟Do端口功能分配错误，进行故障处理

    //if(FunCodeUnion.code.CM_UseVDO == 1) //只有虚拟Do使能的情况才进行普通Do和虚拟Do的功能是否重复，故障报警
    //{
   /*     for (i=0;i<16;i++)    //虚拟Do和普通Do的端口分配的Do的功能相同时，进行故障处理
        {
            for (j=0;j<5;j++)
            {
                if( ( (*DoPortFunSelAddList[j]) != 0)&&
                    ( (*VirtualDoPortFunSelAddList[i]) != 0)&&
                    ( (*VirtualDoPortFunSelAddList[i]) == (*DoPortFunSelAddList[j]))
                  )            
                {                                       
                    if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                    {
                        PostErrMsg(DOLOADERR);
                    }
                    return;
                }
            }
        }*/
    //}

    if ( (STR_DoState.BrakeDoEn == 1) || (STR_VirDoState.BrakeVirDoEn == 1) )
    {
        STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn = 1;
    }
    else
    {
        STR_FUNC_Gvar.MonitorFlag.bit.BrakeEn = 0;
    }

//#endif 
}




/*******************************************************************************
  函数名: void DoFuncSetErr(STR_DO_STATE *p)
  输入:   STR_DoState.DoFuncSel[i]             
  输出:   DILOADERR，故障码，报错处理   
  子函数: PostErrMsg(DILOADERR)
  描述:   1.Do功能选择超出可选范围时，进行报错处理
          2.在8个Do功能中，若其中一个Do功能被多次分配到不同的Do端口上，则进行报错处理     
  返回值:  
********************************************************************************/ 
Static_Inline void DoFuncSetErr(STR_DO_STATE *p)
{
    Uint8 Temp = 0;
    Uint16 i = 0;
    //Uint16 j = 0;    

    /* Do超限以及Do功能重复分配报错*/
    for(i=0;i<5;i++)                                  
    {                                             
        if( (*DoPortFunSelAddList[i]) > DO_FUNC_SEL_MAX)    //防溢出检测
        {                                           
            if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
            {
                PostErrMsg(DOLOADERR);
            }              
            return;                                   
        } 
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
        if(FunCodeUnion.code.OEM_LocalModeEn==1)
        {
        }
        else
        {
            if(((*DoPortFunSelAddList[i]) ==6)
                ||((*DoPortFunSelAddList[i]) ==15)
                ||((*DoPortFunSelAddList[i]) ==16)
                ||((*DoPortFunSelAddList[i]) ==17))    //防溢出检测
            {                                           
                if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                {
                    PostErrMsg(DOLOADERR);
                }              
                return;                                   
            } 
        }
#endif
        
		//DO重分配故障取消2013.04.09KLC
 /*       if(( (*DoPortFunSelAddList[i]) > 0) && ( (*DoPortFunSelAddList[i]) < (DO_FUNC_SEL_MAX + 1)))
        {                                           
            for(j=0;j<i;j++)                        
            {                                         
                if( (*DoPortFunSelAddList[j]) == (*DoPortFunSelAddList[i]))            
                {                                       
                    if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                    {
                        PostErrMsg(DOLOADERR); 
                    }     
                    return;                               
                }                                       
            }                                         
        }
        */
        if((*DoPortFunSelAddList[i]) == 9)  //判断是否分配了抱闸信号
        {
            Temp = 1;
        }
    }
    if(Temp == 1)  //判断是否分配了抱闸信号
    {
        p->BrakeDoEn = 1;
    }
    else
    {
        p->BrakeDoEn = 0;
    }
}




/*******************************************************************************
  函数名:
  输入:               
  输出:   
  子函数: 
  描述:     
  返回值:  
********************************************************************************/ 
Static_Inline void DiPortChangeCheck()
{
    static Uint8  SoffChangeWarnFlag = 0;  
    static Uint8  VirSoffChangeWarnFlag = 0;  

    /* 普通10个Di端口的Di功能中Son的Di端口变更记录 */     
    if ( FunCodeUnion.code.DIFuncSel1 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort1 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort1 = 0; }

    if ( FunCodeUnion.code.DIFuncSel2 == 1 ) 
    {   UNI_StopSonDiPort.bit.DiPort2 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort2 = 0; }

    if ( FunCodeUnion.code.DIFuncSel3 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort3 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort3 = 0; }
                              
    if ( FunCodeUnion.code.DIFuncSel4 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort4 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort4 = 0; }     
    
    if ( FunCodeUnion.code.DIFuncSel5 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort5 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort5 = 0;}

    if ( FunCodeUnion.code.DIFuncSel6 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort6 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort6 = 0; }

    if ( FunCodeUnion.code.DIFuncSel7 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort7 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort7 = 0; }

    if ( FunCodeUnion.code.DIFuncSel8 == 1 )
    {   UNI_StopSonDiPort.bit.DiPort8 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort8 = 0; }

    if ( FunCodeUnion.code.DIFuncSel9 == 1 ) 
    {   UNI_StopSonDiPort.bit.DiPort9 = 1; }
    else
    {   UNI_StopSonDiPort.bit.DiPort9 = 0; }


    if( UNI_InitSonDiPort.all != UNI_StopSonDiPort.all)
    {
        STR_DiState.DiPortChangeEn = 1;
        if (SoffChangeWarnFlag == 0)
        {
            PostErrMsg(PCHGDWARN);
            SoffChangeWarnFlag = 1; 
        }  
    }
    else
    {
        STR_DiState.DiPortChangeEn = 0;
    }


//#ifdef  VIRTUAL_DI_ENABLE
    /* 16个虚拟Di端口的Di功能中Son的Di端口变更记录 */
    if ( FunCodeUnion.code.VI_VDIFuncSel1 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort1 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort1 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel2 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort2 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort2 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel3 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort3 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort3 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel4 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort4 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort4 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel5 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort5 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort5 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel6 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort6 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort6 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel7 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort7 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort7 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel8 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort8 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort8 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel9 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort9 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort9 = 0;}  
    
    if ( FunCodeUnion.code.VI_VDIFuncSel10 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort10 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort10 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel11 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort11 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort11 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel12 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort12 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort12 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel13 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort13 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort13 = 0;}  
    
    if ( FunCodeUnion.code.VI_VDIFuncSel14 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort14 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort14 = 0;}
    
    if ( FunCodeUnion.code.VI_VDIFuncSel15 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort15 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort15 = 0;} 
    
    if ( FunCodeUnion.code.VI_VDIFuncSel16 == 1 )
    {   UNI_StopSonVirDiPort.bit.VirDiPort16 = 1; }
    else
    {   UNI_StopSonVirDiPort.bit.VirDiPort16 = 0;} 
    
       
    if( UNI_InitSonVirDiPort.all != UNI_StopSonVirDiPort.all)
    {
        STR_VirDiState.VirDiPortChangeEn = 1;
        if (VirSoffChangeWarnFlag == 0)
        {
            PostErrMsg(PCHGDWARN);
            VirSoffChangeWarnFlag = 1; 
        }    
    }
    else
    {
        STR_VirDiState.VirDiPortChangeEn = 0;
    }

//#endif
}



/*******************************************************************************
  函数名: void DiProcess()
  输入:                 
  输出:     
  子函数: DealDiFromSpi(&STR_DiState);DealVirDi(&STR_VirDiState); 
  描述:  主要完成对来源于SPI和GPIO的普通Di进行处理，以及对虚拟Di进行处理    
  返回值:  
********************************************************************************/ 
void DiProcess()
{
    static Uint8  SimulateModeVirDiFirExc = 0;
    static Uint16  SimulateModeVirDiTemp = 0;
    Uint16 DILogical = 0;
    Uint16 DINum = 0;
	Uint32 utemp;

    static Uint32  Buffer0 = 0;
    static Uint32  Buffer1 = 0;
    Uint32 Temp = 0;
	
	#if ECT_ENABLE_SWITCH
    DILogical = 0x01BF;
    DINum = 0x003F;
    #else
    DILogical = 0x01FF;
    DINum = 0x007F;
    #endif
    //DI处理
    DealDiFromSpi(&STR_DiState);  //读取来自SPI的DI值，并给DI变量赋值

//    DealDiFromGpio(&STR_DiState);    //移到软中断里面

//#ifdef  VIRTUAL_DI_ENABLE
    //虚拟DI处理
    if(FunCodeUnion.code.CM_UseVDI==1)  //只有虚拟Di使能的情况下，才进行虚拟Di处理
    {
        //H0D17=0 或者 H0D17=2  非DI仿真模式下
        if((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 0) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 2)
            ||(AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 4))
        {
            STR_VirDiState.VirDiStateNew.all = AuxFunCodeUnion.code.CC_VDILevel;
        }
        else   //H0D17=1 或者 H0D17=3    Di仿真模式下电平强行输出
        {
            if (SimulateModeVirDiFirExc == 0)
            {
                SimulateModeVirDiTemp =  (AuxFunCodeUnion.code.FA_DiSimulateSet & DILogical);
                STR_VirDiState.VirDiStateNew.all = SimulateModeVirDiTemp;
                STR_VirDiState.VirDiStateOld.all = STR_VirDiState.VirDiStateNew.all;
                SimulateModeVirDiFirExc = 1;
            }
            else
            {
                SimulateModeVirDiTemp =  (AuxFunCodeUnion.code.FA_DiSimulateSet & DILogical);
                STR_VirDiState.VirDiStateNew.all = SimulateModeVirDiTemp;
            }
        }

        DealVirDi(&STR_VirDiState);
    }
    else 
    {
        STR_FUNC_Gvar.DiDoOutput.VirtualDiPortState = 0;    
    }
//#endif

    /*用于通过LED实时察看Di输入状态*/
    //H0D17=0 或者 H0D17=2 非DI仿真模式下
    if((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 0) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 2)
        ||(AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 4))
    {
        AuxFunCodeUnion.code.DP_DIState = (((Uint16)STR_DiState.SpiDiValue)&DINum) + (((Uint16)STR_DiState.GpioDi8Value & 0x0001) << 7) + (((Uint16)STR_DiState.GpioDi9Value & 0x0001 )<< 8);
    }
    else  //H0D17=1 或者 H0D17=3 Di仿真模式
    {
        AuxFunCodeUnion.code.DP_DIState = (AuxFunCodeUnion.code.FA_DiSimulateSet & DILogical);

    }

    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
        //Son信号滤波
        if(0 == FunCodeUnion.code.BP_SonFltTime)
        {
            STR_FUNC_Gvar.DiDoOutput.Son_Flt = STR_FUNC_Gvar.DivarRegLw.bit.Son;
        }
        else if(32 >= FunCodeUnion.code.BP_SonFltTime)
        {
            Temp = 0xFFFFFFFF >> (32 - FunCodeUnion.code.BP_SonFltTime);

            if(Temp == (Buffer0 & Temp))             //全是1
            {
                STR_FUNC_Gvar.DiDoOutput.Son_Flt = 1;
            }
            else if(0 == (Buffer0 & Temp))             //全是0
            {
                STR_FUNC_Gvar.DiDoOutput.Son_Flt = 0;
            } 
        }
        else if(64 >= FunCodeUnion.code.BP_SonFltTime)
        {
            Temp = 0xFFFFFFFF >> (64 - FunCodeUnion.code.BP_SonFltTime);

            if((0xFFFFFFFF == Buffer0) && (Temp == (Buffer1 & Temp)))             //全是1
            {
                STR_FUNC_Gvar.DiDoOutput.Son_Flt = 1;
            }
            
            if((0 == Buffer0) && (0 == (Buffer1 & Temp)))             //全是0
            {
                STR_FUNC_Gvar.DiDoOutput.Son_Flt = 0;
            }
        }
        else
        {
             STR_FUNC_Gvar.DiDoOutput.Son_Flt = STR_FUNC_Gvar.DivarRegLw.bit.Son;
        }

        Buffer1 = (Buffer1 << 1) & 0xFFFFFFFE;
        Buffer1 += Buffer0 >> 31;

        Buffer0 = (Buffer0 << 1) & 0xFFFFFFFE;
        Buffer0 += STR_FUNC_Gvar.DivarRegLw.bit.Son;          
    }
    else
    {
    }
#if ECT_ENABLE_SWITCH//对60FD的高16位赋值
    if(FunCodeUnion.code.CM_ECATHost != 2)
    {
    	utemp = ObjectDictionaryStandard.ComEntryDIDO.DigitalInputs.all & 0xFFFF;
    	utemp = utemp| (((0x1FF-AuxFunCodeUnion.code.DP_DIState) & 0x1BF)<<16);         //去掉DI7信号
    	ObjectDictionaryStandard.ComEntryDIDO.DigitalInputs.all = utemp;
    }
    else
    {

    }
#endif
}

/*******************************************************************************
  函数名: void DoProcess()
  输入:                
  输出:     
  子函数: VirDoProcess(&STR_VirDoState);
  描述:  完成对普通DO进行处理(将DO输出至SPI的高7位,直接赋值给具体SPI输出位)
         以及完成对虚拟Do进行处理      
  返回值:  
********************************************************************************/ 
void DoProcess()
{
    Uint8 i = 0;

    static Uint16  SimulateModeDoTemp = 0;
    Uint16 DONum = 0;
	
    
    #if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    DONum = 0x0007;
    #elif DRIVER_TYPE == SERVO_620P_407VG
    DONum = 0x001F;
    #endif
//H0D17=0 或者 H0D17=1 非Do仿真模式
    if ((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 0) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 1))
    {
        DovarRegEnableGather();    //收集各模块输出的Do有效标志位
    
        //Do处理
        STR_DoState.DoPortEnState = STR_FUNC_Gvar.DiDoOutput.DoPortState;
        
        for (i=0; i<5; i++)
        {
            if((!( (FunCodeUnion.code.DO_SourceSel >> i ) & 0x0001) )  //驱动器给定
				||(STR_DoState.DoFuncSel[i] == 9))						//抱闸信号必须不能用通信控制
            {
                if((STR_DoState.DoFuncSel[i] > 0)&&(STR_DoState.DoFuncSel[i] <= DO_FUNC_SEL_MAX))
                {
                    if(STR_DoState.DoLogicSel[i] == 0)
                    {
                        if(  ((UNI_DovarReg.all >> (STR_DoState.DoFuncSel[i]-1))&0x0001) == 1 )
                        {
                            UNI_SpiDoReg.all |= ( ((UNI_DovarReg.all >> (STR_DoState.DoFuncSel[i]-1))&0x0001)<<(8+i) );
                            STR_DoState.DoPortEnState |= (0x0001<<i);  //使能5个Do端口中相应的Do端口，1使能 
                        }
                        else
                        {
                            UNI_SpiDoReg.all &= ~( (((~UNI_DovarReg.all) >> (STR_DoState.DoFuncSel[i]-1))&0x0001)<<(8+i) );
                            STR_DoState.DoPortEnState &= ~(0x0001<<i);  //使能5个Do端口中相应的Do端口,0不使能  
                        }
                    }
                    else
                    {
                        if(  ((UNI_DovarReg.all>>(STR_DoState.DoFuncSel[i]-1))&0x0001) == 1  )
                        {
                            UNI_SpiDoReg.all &= ~( ((UNI_DovarReg.all>>(STR_DoState.DoFuncSel[i]-1))&0x0001)<<(8+i) );
                            STR_DoState.DoPortEnState |= (0x0001<<i);  //使能5个Do端口中相应的Do端口,0使能 
                        }
                        else
                        {
                            UNI_SpiDoReg.all |= ( (((~UNI_DovarReg.all)>>(STR_DoState.DoFuncSel[i]-1))&0x0001)<<(8+i) );
                            STR_DoState.DoPortEnState &= ~(0x0001<<i);  //使能5个Do端口中相应的Do端口,1不使能
                        }
                    }
                }
                else
                {
                    UNI_SpiDoReg.all &= ~(0x0001<<(8+i));
                    STR_DoState.DoPortEnState &= ~(0x0001<<i); //使能5个Do端口中相应的Do端口，0为不使能    
                }
            }
            else     //通讯给定
            {
                if ( ((AuxFunCodeUnion.code.CC_CommCtrlDO>>i)&0x0001) == 1 )
                {
                    UNI_SpiDoReg.all |= ((AuxFunCodeUnion.code.CC_CommCtrlDO>>i)&0x0001)<<(8+i);
                    STR_DoState.DoPortEnState |= (0x0001<<i);  //通讯的方式使能5个Do中相应的Do端口 ,1使能 
                }
                else
                {
                    UNI_SpiDoReg.all &= ~((((~AuxFunCodeUnion.code.CC_CommCtrlDO)>>i)&0x0001)<<(8+i)); 
                    STR_DoState.DoPortEnState &= ~(0x0001<<i); //使能5个Do端口中相应的Do端口，0不使能 
                }
            }   
        }    
        STR_FUNC_Gvar.DiDoOutput.DoPortState = STR_DoState.DoPortEnState; 
            
        AuxFunCodeUnion.code.DP_DOState = ~((UNI_SpiDoReg.bit.SpiDo2 << 1) + (UNI_SpiDoReg.bit.SpiDo3 << 2) + 
                                            (UNI_SpiDoReg.bit.SpiDo4 << 3) + (UNI_SpiDoReg.bit.SpiDo5 << 4) +
                                             UNI_SpiDoReg.bit.SpiDo1);

        AuxFunCodeUnion.code.DP_DOState &= DONum;

    //#ifdef  VIRTUAL_DO_ENABLE
        //虚拟Do处理
        if(FunCodeUnion.code.CM_UseVDO == 1)  //只有虚拟Do使能的情况下，才进行虚拟Do处理
        {
            VirDoProcess(&STR_VirDoState);
        }
        else
        {
            FunCodeUnion.code.VI_AllVDOLevel = 0;   
            STR_FUNC_Gvar.DiDoOutput.VirtualDoPortState = 0; 
        }
    }
    else if((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 2) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 3))  //H0D17=2 或者 H0D17=3 Do仿真模式
    {
        for(i=0; i<5; i++)
        {
            if(STR_DoState.DoLogicSel[i] == 0)      //有5个Do端口，每个Do端口都进行相应的电平逻辑选择处理，所以要分别进行处理
            {
                if( (((AuxFunCodeUnion.code.FA_DoSimulateSet & DONum)>>i)&0x0001) == 1 )
                {
                    SimulateModeDoTemp |= ((((AuxFunCodeUnion.code.FA_DoSimulateSet & DONum)>>i)&0x0001)<<(8+i));
                }
                else
                {
                    SimulateModeDoTemp &= ~(((((~AuxFunCodeUnion.code.FA_DoSimulateSet)&DONum)>>i)&0x0001)<<(8+i));
                }
            }
            else
            {
                if( (((AuxFunCodeUnion.code.FA_DoSimulateSet & DONum)>>i)&0x0001) == 1 )
                {
                    SimulateModeDoTemp &= ~((((AuxFunCodeUnion.code.FA_DoSimulateSet & DONum)>>i)&0x0001)<<(8+i));
                }
                else
                {
                    SimulateModeDoTemp |= (((((~AuxFunCodeUnion.code.FA_DoSimulateSet)&DONum)>>i)&0x0001)<<(8+i));
                }
            }
        }
        UNI_SpiDoReg.all = (SimulateModeDoTemp);   //Do__SPI输出

        //面板H0B05显示的电平状态
        AuxFunCodeUnion.code.DP_DOState = ~((UNI_SpiDoReg.bit.SpiDo2 << 1) + (UNI_SpiDoReg.bit.SpiDo3 << 2) + 
                                           (UNI_SpiDoReg.bit.SpiDo4 << 3) + (UNI_SpiDoReg.bit.SpiDo5 << 4) +
                                            UNI_SpiDoReg.bit.SpiDo1);

        AuxFunCodeUnion.code.DP_DOState &= DONum;

    }
	else
	{
	    SimulateModeDoTemp = (Uint16)((ObjectDictionaryStandard.ComEntryDIDO.DigitalOutputs.DOFunc.PhysicalOutput.all 
                                    & ObjectDictionaryStandard.ComEntryDIDO.DigitalOutputs.DOEnable.BitMask.all)>>16);  
		
        SimulateModeDoTemp = SimulateModeDoTemp & 0x07;
        
        SimulateModeDoTemp = SimulateModeDoTemp<<8;
		
		UNI_SpiDoReg.all = SimulateModeDoTemp;   //Do__SPI输出

        //面板H0B05显示的电平状态
        AuxFunCodeUnion.code.DP_DOState = ~((UNI_SpiDoReg.bit.SpiDo2 << 1) + (UNI_SpiDoReg.bit.SpiDo3 << 2) + 
                                           (UNI_SpiDoReg.bit.SpiDo4 << 3) + (UNI_SpiDoReg.bit.SpiDo5 << 4) +
                                            UNI_SpiDoReg.bit.SpiDo1);

        AuxFunCodeUnion.code.DP_DOState &= DONum;
    }


//#endif
}


/*******************************************************************************
  函数名: void DealDiFromSpi(STR_DI_STATE *p)
  输入:   STR_DiState              
  输出:   DivarReg  
  子函数: 
  描述:  处理来自于Spi的Di信号，首先对来自于Spi的Di电平状态进行滤波，之后从Di1到Di8
         依次判断Di由Spi输入的有效逻辑电平状态和由功能码设定的Di有效逻辑电平状态是否
         相等，若相等则把 DivarReg中相应的Di功能置1，表明该Di功能使能
        
  返回值:  
********************************************************************************/ 
Static_Inline void DealDiFromSpi(STR_DI_STATE *p)
{
    static Uint8 Cnt = 0;
    static Uint8 DiLogicFromSpi= 0;

    static Uint8  SimulateModeFirExc = 0;
    static Uint16  SimulateModeDiTemp = 0;

    Uint8 i = 0;

    Uint16 DINum = 0;
    
    #if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    DINum = 0x003F;
    #elif DRIVER_TYPE == SERVO_620P_407VG
    DINum = 0x007F;
    #endif

    STR_DiState.DiPortEnState = STR_FUNC_Gvar.DiDoOutput.DiPortState;
    
    //H0D17=0 或者 H0D17=2 非DI仿真模式下
    if((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 0) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 2)
        ||(AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 4))
    {
        /*读取Spi电平状态,含有滤波处理*/
        if(UNI_SpiDiReg.all != p->SpiDiValueLast)   //当Spi输入的Di值发生变化时，且变化后的值保持三个调度周期后，才更新SPI输入的Di状态
        {
            Cnt++ ;
        }
        else
        {
            Cnt = 0;
        }
        if (Cnt > 2)
        {
            Cnt = 0;
            p->SpiDiValue = (Uint8)(UNI_SpiDiReg.all&DINum);
        }
    }
    else   //H0D17=1 或者 H0D17=3 DI仿真模式下 Di仿真模式下电平强行输出
    {
        if (SimulateModeFirExc == 0)
        {
            SimulateModeDiTemp =  (AuxFunCodeUnion.code.FA_DiSimulateSet & DINum);
            p->SpiDiValue = (Uint8)SimulateModeDiTemp;
            p->SpiDiValueLast = p->SpiDiValue;
            SimulateModeFirExc = 1;
        }
        else
        {
            SimulateModeDiTemp =  (AuxFunCodeUnion.code.FA_DiSimulateSet & DINum);
            p->SpiDiValue = (Uint8)SimulateModeDiTemp;
        }
    }


    for(i=0;i<7;i++)
    {
        DiLogicFromSpi = ((p->SpiDiValue>>i)&0x01) + (((p->SpiDiValueLast>>i)&0x01)<<1); 
        if( (p->DiFuncSel[i] > 0)&&(p->DiFuncSel[i] < (DI_FUNC_SEL_MAX + 1)) )
        {
            if( ((p->DiLogicSel[i] == 0)&&(!((p->SpiDiValue>>i)&0x01)))||        //低电平有效，读入为0，默认,只看当前的高低状态
                ((p->DiLogicSel[i] == 1)&&(((p->SpiDiValue>>i)&0x01))) ||        //高电平有效，读入为1，只看当前的高低状态
                ((p->DiLogicSel[i] == 2)&&(DiLogicFromSpi == 1))||                  //上升沿有效，状态为上升沿
                ((p->DiLogicSel[i] == 3)&&(DiLogicFromSpi == 2))||                  //下降沿有效，状态为下降沿
                ((p->DiLogicSel[i] == 4)&&((DiLogicFromSpi == 1)||(DiLogicFromSpi == 2))) //上升下降沿均有效
              )
             {
                  DivarReg |= ((Uint64)0x01L<<(Uint32)(p->DiFuncSel[i]-1));  //把STR_FUNC_Gvar.DivarReg.all中对应的Di功能置1，即相应Di设置条件达到，使能Di
                  STR_DiState.DiPortEnState |= (0x01<<i);    //更新相应的9个Di端口的哪个被使能了，1为使能，0为未使能
             }
             else
             {
                  DivarReg &= ~((Uint64)0x01L<<(Uint32)(p->DiFuncSel[i]-1));
                  STR_DiState.DiPortEnState &= ~(0x01<<i);   //更新相应的9个Di端口的那个被使能了，1为使能，0为未使能
             }
        }
    }
    p->SpiDiValueLast = p->SpiDiValue;

    STR_FUNC_Gvar.DivarRegLw.all = (Uint32)DivarReg;

    STR_FUNC_Gvar.DivarRegHi.all = (Uint32)(DivarReg>>32);

    STR_FUNC_Gvar.DiDoOutput.DiPortState = STR_DiState.DiPortEnState;

#if ECT_ENABLE_SWITCH

    //硬件限位和软件限位可同时存在。
	if((STR_FUNC_Gvar.DivarRegLw.bit.Pot == 0)&&(STR_FUNC_Gvar.DivarRegLw.bit.Not == 0))    
	{
        if(STR_PosCtrlVar.PosFedReachPosLimit==1)
        {
            STR_FUNC_Gvar.DivarRegLw.bit.Pot = 1;
        }
	    else if(STR_PosCtrlVar.PosFedReachNegLimit==1)
		{
		    STR_FUNC_Gvar.DivarRegLw.bit.Not = 1;
		}
	}

#endif
    
}

/*******************************************************************************
  函数名: void DealDiFromGpio(STR_DI_STATE *p)
  输入:   STR_DiState             
  输出:   DivarReg  
  子函数: 
  描述:  处理来自于GPIO的Di9和Di10信号，首先对来自于Spi的Di电平状态进行滤波，之后从Di9到Di10
         依次判断Di由GPIO输入的有效逻辑电平状态和由功能码设定的Di有效逻辑电平状态是否
         相等，若相等则把 DivarReg中相应的Di功能置1，表明该Di功能使能    
  返回值:  
********************************************************************************/ 
void DealDiFromGpio(void)
{ 
    static Uint8  DiLogicFromGpio = 0;
    static Uint8  DiValueTemp = 0;
    static Uint8  DiValueTempLast = 0;

    static Uint8  SimulateModeDiGpioFirExc = 0;
    static Uint16  SimulateModeDiGpioTemp = 0;
    Uint64 DivarRegTemp = 0;

    STR_DiState.DiPortEnState = STR_FUNC_Gvar.DiDoOutput.DiPortState;

    DivarRegTemp = (Uint64)DivarReg;

    //H0D17=0 或者 H0D17=2 非DI仿真模式下
    if((AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 0) || (AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 2)
        ||(AuxFunCodeUnion.code.FA_DiDoSimulateEnable == 4))
    {
        STR_DiState.GpioDi8Value = (Uint8)GPIO_ReadDI8DSP();   //读取当前的Di8_Gpio电平状态
        STR_DiState.GpioDi9Value = (Uint8)GPIO_ReadDI9DSP();   //读取当前的Di9_Gpio电平状态
        DiValueTemp = (STR_DiState.GpioDi9Value<<1)+ STR_DiState.GpioDi8Value;
    }
    else   //H0D17=1 或者 H0D17=3 DI仿真模式下   Di仿真模式下电平强行输出
    {
        if (SimulateModeDiGpioFirExc == 0)
        {
            SimulateModeDiGpioTemp = (AuxFunCodeUnion.code.FA_DiSimulateSet & 0x0180);
            DiValueTemp = (Uint8)(SimulateModeDiGpioTemp>>7);
            DiValueTempLast = DiValueTemp;
            SimulateModeDiGpioFirExc = 1;
        }
        else
        {
            SimulateModeDiGpioTemp = (AuxFunCodeUnion.code.FA_DiSimulateSet & 0x0180);
            DiValueTemp = (Uint8)(SimulateModeDiGpioTemp>>7);
        }
    }

    DiLogicFromGpio = ((DiValueTempLast & 0x01) << 1)  +  ((DiValueTemp & 0x01));
    
    if( (STR_DiState.DiFuncSel[7] > 0) && ( STR_DiState.DiFuncSel[7] < (DI_FUNC_SEL_MAX + 1) ) )
    {
        if( ((STR_DiState.DiLogicSel[7] == 0)&&(!(DiValueTemp & 0x01)))||
            ((STR_DiState.DiLogicSel[7] == 1)&&((DiValueTemp & 0x01))) ||
            ((STR_DiState.DiLogicSel[7] == 2)&&(DiLogicFromGpio == 1))||                 
            ((STR_DiState.DiLogicSel[7] == 3)&&(DiLogicFromGpio == 2))||                 
            ((STR_DiState.DiLogicSel[7] == 4)&&((DiLogicFromGpio == 1)||(DiLogicFromGpio == 2)))
          )
        {
            DivarRegTemp |= ((Uint64)0x01L<<(Uint32)(STR_DiState.DiFuncSel[7]-1));
            STR_DiState.DiPortEnState |= (0x01<<7);   //更新相应的9个Di端口的那个被使能了，1为使能，0为未使能
        }
        else
        {
            DivarRegTemp &= ~((Uint64)0x01L<<(Uint32)(STR_DiState.DiFuncSel[7]-1));
            STR_DiState.DiPortEnState &= ~(0x01<<7);  //更新相应的9个Di端口的那个被使能了，1为使能，0为未使能
        }
    }
    
    DiLogicFromGpio = (((DiValueTempLast>>1) & 0x01) << 1)  +  (((DiValueTemp>>1) & 0x01)); 
    if( (STR_DiState.DiFuncSel[8] > 0) && ( STR_DiState.DiFuncSel[8] < (DI_FUNC_SEL_MAX + 1) ) )
    {
        if( ((STR_DiState.DiLogicSel[8] == 0)&&(!((DiValueTemp>>1)& 0x01)))||
            ((STR_DiState.DiLogicSel[8] == 1)&&(((DiValueTemp>>1)& 0x01))) ||
            ((STR_DiState.DiLogicSel[8] == 2)&&(DiLogicFromGpio == 1))||                 
            ((STR_DiState.DiLogicSel[8] == 3)&&(DiLogicFromGpio == 2))||                 
            ((STR_DiState.DiLogicSel[8] == 4)&&((DiLogicFromGpio == 1)||(DiLogicFromGpio == 2)))
          )
        {
            DivarRegTemp |= ((Uint64)0x01L<<(Uint32)(STR_DiState.DiFuncSel[8]-1));
            STR_DiState.DiPortEnState |= (0x01<<8);   //更新相应的9个Di端口的那个被使能了，1为使能，0为未使能
        }
        else
        {
            DivarRegTemp &= ~((Uint64)0x01L<<(Uint32)(STR_DiState.DiFuncSel[8]-1));
            STR_DiState.DiPortEnState &= ~(0x01<<8);  //更新相应的9个Di端口的那个被使能了，1为使能，0为未使能
        }
    }
       
    DiValueTempLast = DiValueTemp; 
 
    DivarReg = (Uint64)DivarRegTemp;
        
    STR_FUNC_Gvar.SoftIntDivarRegLw.all = (Uint32)DivarReg;

    STR_FUNC_Gvar.SoftIntDivarRegHi.all = (Uint32)(DivarReg >> 32);

    STR_FUNC_Gvar.DiDoOutput.DiPortState = STR_DiState.DiPortEnState;

#if ECT_ENABLE_SWITCH

    //硬件限位和软件限位可同时存在。
	if((STR_FUNC_Gvar.DivarRegLw.bit.Pot == 0)&&(STR_FUNC_Gvar.DivarRegLw.bit.Not == 0))    
	{
        if(STR_PosCtrlVar.PosFedReachPosLimit==1)
        {
            STR_FUNC_Gvar.DivarRegLw.bit.Pot = 1;
        }
	    else if(STR_PosCtrlVar.PosFedReachNegLimit==1)
		{
		    STR_FUNC_Gvar.DivarRegLw.bit.Not = 1;
		}
	}

#endif



#if ECT_ENABLE_SWITCH
    CanopenDIRefresh1k();
    CanopenDIRefresh4k();
#endif
    STR_FUNC_Gvar.OscTarget.Osc_DIFunc0 = (Uint16)DivarReg;
    STR_FUNC_Gvar.OscTarget.Osc_DIFunc1 = (Uint16)((Uint64)DivarReg >> 16);
    STR_FUNC_Gvar.OscTarget.Osc_DIFunc2 = (Uint16)((Uint64)DivarReg >> 32);
    STR_FUNC_Gvar.OscTarget.Osc_DIFunc3 = (Uint16)((Uint64)DivarReg >> 48);
}



/*******************************************************************************
  函数名: 
  输入:   
  输出:   
  子函数: 
  描述:  
  返回值:  
********************************************************************************/ 
Static_Inline void DovarRegEnableGather(void)
{

    UNI_DovarReg.bit.AngIntRdy  = STR_FUNC_Gvar.Monitor.DovarReg_AngIntRdy;
    
    UNI_DovarReg.bit.VArr = STR_FUNC_Gvar.SpdCtrl.DovarReg_VArr;
    
    UNI_DovarReg.bit.ToqReach = STR_FUNC_Gvar.ToqCtrl.DovarReg_ToqReach;

    UNI_DovarReg.bit.OrgOkElectric = STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOkElectric;

    UNI_DovarReg.bit.OrgOk = STR_FUNC_Gvar.PosCtrl.DovarReg_OrgOk;

    UNI_DovarReg.bit.Xintcoin = STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin;

    UNI_DovarReg.bit.AlmCode = STR_FUNC_Gvar.Monitor.DovarReg_AlmCode;

    UNI_DovarReg.bit.Alm = STR_FUNC_Gvar.Monitor.DovarReg_Alm;

    UNI_DovarReg.bit.Warn = STR_FUNC_Gvar.Monitor.DovarReg_Warn;

    UNI_DovarReg.bit.Blk = STR_FUNC_Gvar.Monitor.DovarReg_Blk;

    UNI_DovarReg.bit.Vlt = STR_FUNC_Gvar.ToqCtrl.DovarReg_Vlt;

    UNI_DovarReg.bit.Clt = STR_FUNC_Gvar.ToqCtrl.DovarReg_Clt;

    UNI_DovarReg.bit.Near = STR_FUNC_Gvar.PosCtrl.DovarReg_Near;

    UNI_DovarReg.bit.Coin = STR_FUNC_Gvar.PosCtrl.DovarReg_Coin;

    UNI_DovarReg.bit.VCmp = STR_FUNC_Gvar.SpdCtrl.DovarReg_VCmp;

    UNI_DovarReg.bit.Zero = STR_FUNC_Gvar.SpdCtrl.DovarReg_Zero; 

    UNI_DovarReg.bit.TGon = STR_FUNC_Gvar.SpdCtrl.DovarReg_TGon;

    UNI_DovarReg.bit.SRdy = STR_FUNC_Gvar.Monitor.DovarReg_SRdy;

    AuxFunCodeUnion.code.CR_FunOut = UNI_DovarReg.all;
    AuxFunCodeUnion.code.CR_FunOutH = UNI_DovarReg.all >> 16; 

    STR_FUNC_Gvar.OscTarget.Osc_DOFunc0 = (Uint16)UNI_DovarReg.all;
    STR_FUNC_Gvar.OscTarget.Osc_DOFunc1 = (Uint16)(UNI_DovarReg.all >> 16);
}




/********************************* END OF FILE *********************************/


