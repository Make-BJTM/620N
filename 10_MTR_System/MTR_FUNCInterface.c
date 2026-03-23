/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    MTR_FUNCInterface.c
 创建人:    李浩                    创建日期：2012.03.26
 描述:  MTR 发送给 FUNC模块的变量接口
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "MTR_FUNCInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义*/



/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

//STR_MTRTOFUNC_FLAGBUFF_16kHz        STR_MTRToFUNC_FlagBuff_16kHz;


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

/* 初始化中MTR传送给FUNC的变量表-初始化变量表 */
Uint32 * const pMTRToFUNC_InitListAddr[MTRTOFUNC_INITLIST_NUM] =
{
    (Uint32 *) &STR_MTR_Gvar.GetIqRef.IqRate_MT,        //当电机相有效值电流为电机额定电流时对应IqRef的（即IqRate_MT）输入值

    (Uint32 *) &STR_MTR_Gvar.FPGA.EncRev,               //电机分辨率数,FUNC_PosCtrl计算位置增量反馈时使用

    (Uint32 *) &STR_MTR_Gvar.GetIqRef.ToqRefConst_Q12,  //输出转矩指令转换系数: 百分数（0.1%）->内部转矩(Iq)数字指令值转换  
    
    (Uint32 *) &STR_MTR_Gvar.AbsRom.AbsPosDetection,    //绝对位置检测系统模式 
};


//---------------------------------------------------------------------------------------//

/* 主循环_1K调度的MTR传送给FUNC的变量表——慢表 */

Uint32 * const pMTRToFUNC_List_1kHz_Addr[MTRTOFUNC_LIST_1kHz_NUM] =
{
    (Uint32 *) &STR_MTR_Gvar.FPGA.UVAdjustResult,       //UV相电流平衡校正结果
    (Uint32 *) &STR_MTR_Gvar.GetIqRef.IqPosLmt_F,         //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的正向IqRef最后限制值
    (Uint32 *) &STR_MTR_Gvar.GetIqRef.IqNegLmt_F,         //经驱动器、电机最大Iq值和外部转矩限制值综合计算后的反向IqRef最后限制值
    (Uint32 *) &STR_MTR_Gvar.FPGA.Vd,         //d轴电压调制度
    (Uint32 *) &STR_MTR_Gvar.FPGA.Vq,         //q轴电压调制度
};

//---------------------------------------------------------------------------------------------//

/* 16K调度的MTR传送给FUNC的变量表——快表*/
Uint32 * const pMTRToFUNC_List_16kHz_Addr[MTRTOFUNC_LIST_16kHz_NUM] =
{
    (Uint32 *) &STR_MTR_Gvar.FPGA.MechaAngle,

    (Uint32 *) &STR_MTR_Gvar.GetIqRef.IqRef,

    (Uint32 *) &STR_MTR_Gvar.ToFUNCSpdFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.IqFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.IdFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.IuFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.IvFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.IwFdb,

    (Uint32 *) &STR_MTR_Gvar.FPGA.PosFdbAbsValue,

    (Uint32 *) &STR_MTR_Gvar.FPGA.PulsePosRef,

    (Uint32 *) &STR_MTR_Gvar.FPGA.PulsePosRef2,

    (Uint32 *) &STR_MTR_Gvar.FPGA.Etheta,

    (Uint32 *) &STR_MTR_Gvar.OscTarget.CH1_Test,

    (Uint32 *) &STR_MTR_Gvar.OscTarget.CH2_Test,

    (Uint32 *) &STR_MTR_Gvar.OscTarget.CH3_Test,

    (Uint32 *) &STR_MTR_Gvar.OscTarget.CH4_Test,

    (Uint32 *) &STR_MTR_Gvar.MTRtoFUNCFlag.all,           //MTR传递给FUNC的标志位

    (Uint32 *) &STR_MTR_Gvar.FPGA.LnrPosFdb,        //FPGA 采集的全闭环绝对位置 

    (Uint32 *) &STR_MTR_Gvar.AbsRom.MultiAbsPosFdbL,  
          
    (Uint32 *) &STR_MTR_Gvar.AbsRom.MultiAbsPosFdbH,

    (Uint32 *) &STR_MTR_Gvar.AbsRom.SingleAbsPosFdb,
     
    (Uint32 *) &STR_MTR_Gvar.AbsRom.FpgaPosFdbInit,    

    (Uint32 *) &STR_MTR_Gvar.FPGA.HighPrecisionAI,            
};

//------------------------------------------------------------------------------------------------//
/* 16K调度的MTR传送给FUNC的变量表——快表 中断后端调用*/
Uint32 * const pMTRToFUNC_FSAList_16kHz_Addr[2] =
{
    (Uint32 *) &STR_MTR_Gvar.SpeedRegulator.FSA_SpdReguOutPlusChirp,

    (Uint32 *) &STR_MTR_Gvar.SpeedRegulator.SpdReguOut,          
};

//------------------------------------------------------------------------------------------------//
/* Exported_Functions ---------------------------------------------------------*/
/* 供外部使用的函数声明 */
void G_MTR_FUNCGetInitList(Uint32 * pFUNC_InitListHeadAddr);

void G_MTR_FUNCGetList_1kHz(Uint32 * pFUNC_List_1kHz_HeadAddr);

void G_MTR_FUNCGetList_16kHz(Uint32 * pFUNC_List_16kHz_HeadAddr);

void G_MTR_FUNCGetFSAList_16kHz(Uint32 * pFUNC_FSAList_16kHz_HeadAddr);

/* Private_Functions ---------------------------------------------------------*/
/* 内部使用的函数声明 */




/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  描述: 提供给功能模块调用，获取MTR传递过来的接口变量函数，该函数在上电初始化中调用    
********************************************************************************/
void G_MTR_FUNCGetInitList(Uint32 * pFUNC_InitListHeadAddr)
{
    Uint16 i = 0;

    for(i=0; i<MTRTOFUNC_INITLIST_NUM; i++)
    {
        *(pFUNC_InitListHeadAddr + i) = *pMTRToFUNC_InitListAddr[i];
    }

}


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  描述: 提供给功能模块调用，获取MTR传递过来的接口变量函数，该函数调度1K
    
********************************************************************************/
void G_MTR_FUNCGetList_1kHz(Uint32 * pFUNC_List_1kHz_HeadAddr)
{
    Uint16 i=0;

    for(i=0; i<MTRTOFUNC_LIST_1kHz_NUM; i++)
    {
        *(pFUNC_List_1kHz_HeadAddr + i) = *pMTRToFUNC_List_1kHz_Addr[i];
    }
}


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  描述: 提供给功能模块调用，获取MTR传递过来的接口变量函数，该函数调度16K
********************************************************************************/
void G_MTR_FUNCGetList_16kHz(Uint32 * pFUNC_List_16kHz_HeadAddr)
{
    //选为M/T法测速
    if(FunCodeUnion.code.GN_SpdFbSel == 0)
    {
        STR_MTR_Gvar.ToFUNCSpdFdb = STR_MTR_Gvar.FPGA.SpdFdb;
    }
    else  //总线式编码器，直线光栅尺且选定了H0825时，使用M法测速
    {
        STR_MTR_Gvar.ToFUNCSpdFdb = STR_MTR_Gvar.FPGA.M_SpdFdbFlt;
    }

    //21个变量，如果用FOR循环赋值，赋值代码执行时间362/72  us
    //直接赋值，赋值代码执行时间80/72  us
    *(pFUNC_List_16kHz_HeadAddr + 0) = *pMTRToFUNC_List_16kHz_Addr[0];
    *(pFUNC_List_16kHz_HeadAddr + 1) = *pMTRToFUNC_List_16kHz_Addr[1];
    *(pFUNC_List_16kHz_HeadAddr + 2) = *pMTRToFUNC_List_16kHz_Addr[2];
    *(pFUNC_List_16kHz_HeadAddr + 3) = *pMTRToFUNC_List_16kHz_Addr[3];
    *(pFUNC_List_16kHz_HeadAddr + 4) = *pMTRToFUNC_List_16kHz_Addr[4];
    *(pFUNC_List_16kHz_HeadAddr + 5) = *pMTRToFUNC_List_16kHz_Addr[5];
    *(pFUNC_List_16kHz_HeadAddr + 6) = *pMTRToFUNC_List_16kHz_Addr[6];
    *(pFUNC_List_16kHz_HeadAddr + 7) = *pMTRToFUNC_List_16kHz_Addr[7];
    *(pFUNC_List_16kHz_HeadAddr + 8) = *pMTRToFUNC_List_16kHz_Addr[8];
    *(pFUNC_List_16kHz_HeadAddr + 9) = *pMTRToFUNC_List_16kHz_Addr[9];
    *(pFUNC_List_16kHz_HeadAddr + 10) = *pMTRToFUNC_List_16kHz_Addr[10];
    *(pFUNC_List_16kHz_HeadAddr + 11) = *pMTRToFUNC_List_16kHz_Addr[11];
    *(pFUNC_List_16kHz_HeadAddr + 12) = *pMTRToFUNC_List_16kHz_Addr[12];
    *(pFUNC_List_16kHz_HeadAddr + 13) = *pMTRToFUNC_List_16kHz_Addr[13];
    *(pFUNC_List_16kHz_HeadAddr + 14) = *pMTRToFUNC_List_16kHz_Addr[14];
    *(pFUNC_List_16kHz_HeadAddr + 15) = *pMTRToFUNC_List_16kHz_Addr[15];
    *(pFUNC_List_16kHz_HeadAddr + 16) = *pMTRToFUNC_List_16kHz_Addr[16];
    *(pFUNC_List_16kHz_HeadAddr + 17) = *pMTRToFUNC_List_16kHz_Addr[17];
    *(pFUNC_List_16kHz_HeadAddr + 18) = *pMTRToFUNC_List_16kHz_Addr[18];
    *(pFUNC_List_16kHz_HeadAddr + 19) = *pMTRToFUNC_List_16kHz_Addr[19];
    *(pFUNC_List_16kHz_HeadAddr + 20) = *pMTRToFUNC_List_16kHz_Addr[20];
    *(pFUNC_List_16kHz_HeadAddr + 21) = *pMTRToFUNC_List_16kHz_Addr[21];
    *(pFUNC_List_16kHz_HeadAddr + 22) = *pMTRToFUNC_List_16kHz_Addr[22];

}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  描述: 提供给扫频功能模块调用，获取MTR传递过来的扫频状态变量函数，该函数调度16K，在中断尾部调用
********************************************************************************/
void G_MTR_FUNCGetFSAList_16kHz(Uint32 * pFUNC_FSAList_16kHz_HeadAddr)
{
    *(pFUNC_FSAList_16kHz_HeadAddr + 0) = *pMTRToFUNC_FSAList_16kHz_Addr[0];
    *(pFUNC_FSAList_16kHz_HeadAddr + 1) = *pMTRToFUNC_FSAList_16kHz_Addr[1];
}

/********************************* END OF FILE *********************************/
