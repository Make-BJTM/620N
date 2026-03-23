/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_VirtualDiDo.c
 创建人：童文邹
 修改人：李浩               创建日期：11.11.22
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
#include "FUNC_VirtualDiDo.h"
#include "FUNC_DiDo.h"
#include "FUNC_ErrorCode.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */


/* Private_TypesDefinitions --------------------------------------------------*/
/* 结构体变量定义 枚举变量定义 */
UNI_VIR_DO_REGISTER   UNI_VirDoReg;

extern UNI_DOVARREG        UNI_DovarReg;

extern Uint64        DivarReg;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

//虚拟Di16个端口功能选择地址表
Uint16 * const VirtualDiPortFunSelAddList[16] =
{
    //功能码
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel1,              //H1700 虚拟 Di1 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel2,              //H1702 虚拟 Di2 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel3,              //H1704 虚拟 Di3 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel4,              //H1706 虚拟 Di4 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel5,              //H1708 虚拟 Di5 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel6,              //H1710 虚拟 Di6 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel7,              //H1712 虚拟 Di7 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel8,              //H1714 虚拟 Di8 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel9,              //H1716 虚拟 Di9 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel10,             //H1718 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel11,             //H1720 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel12,             //H1722 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel13,             //H1724 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel14,             //H1726 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel15,             //H1728 虚拟 Di10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDIFuncSel16,             //H1730 虚拟 Di10 端子功能选择
};

//虚拟Do16个端口功能选择地址表
Uint16 * const VirtualDoPortFunSelAddList[16] =
{
    //功能码
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel1,              //H1733 虚拟 Do1 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel2,              //H1735 虚拟 Do2 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel3,              //H1737 虚拟 Do3 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel4,              //H1739 虚拟 Do4 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel5,              //H1741 虚拟 Do5 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel6,              //H1743 虚拟 Do6 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel7,              //H1745 虚拟 Do7 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel8,              //H1747 虚拟 Do8 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel9,              //H1749 虚拟 Do9 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel10,             //H1751 虚拟 Do10 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel11,             //H1753 虚拟 Do11 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel12,             //H1755 虚拟 Do12 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel13,             //H1757 虚拟 Do13 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel14,             //H1759 虚拟 Do14 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel15,             //H1761 虚拟 Do15 端子功能选择
    (Uint16 *) &FunCodeUnion.code.VI_VDOFuncSel16,             //H1763 虚拟 Do16 端子功能选择
};

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitVirDiFuncAndLogic(STR_VIR_DI_STATE *p);
void VirDiFuncSetErr(STR_VIR_DI_STATE *p);
void DealVirDi(STR_VIR_DI_STATE *p);

void InitVirDoFuncAndLogic(STR_VIR_DO_STATE *p);
void VirDoFuncSetErr(STR_VIR_DO_STATE *p);
void VirDoProcess(STR_VIR_DO_STATE *p);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */


/*******************************************************************************
  函数名: void InitVirDiFuncAndLogic(STR_VIR_DI_STATE *p)
  输入:   虚拟Di功能码
  输出:   p->VirDiFuncSel[i]; p->VirDiLogicSel[i]
  子函数:
  描述:   根据虚拟Di功能码，对其功能选择数组和逻辑电平数组进行初始化
  返回值:
********************************************************************************/
void InitVirDiFuncAndLogic(STR_VIR_DI_STATE *p)
{
    Uint16 i = 0;
    /*根据功能码对16个虚拟Di端子进行32个Di功能中的一个进行选择*/
    p->VirDiFuncSel[0] = FunCodeUnion.code.VI_VDIFuncSel1;
    p->VirDiFuncSel[1] = FunCodeUnion.code.VI_VDIFuncSel2;
    p->VirDiFuncSel[2] = FunCodeUnion.code.VI_VDIFuncSel3;
    p->VirDiFuncSel[3] = FunCodeUnion.code.VI_VDIFuncSel4;
    p->VirDiFuncSel[4] = FunCodeUnion.code.VI_VDIFuncSel5;
    p->VirDiFuncSel[5] = FunCodeUnion.code.VI_VDIFuncSel6;
    p->VirDiFuncSel[6] = FunCodeUnion.code.VI_VDIFuncSel7;
    p->VirDiFuncSel[7] = FunCodeUnion.code.VI_VDIFuncSel8;
    p->VirDiFuncSel[8] = FunCodeUnion.code.VI_VDIFuncSel9;
    p->VirDiFuncSel[9] = FunCodeUnion.code.VI_VDIFuncSel10;
    p->VirDiFuncSel[10] = FunCodeUnion.code.VI_VDIFuncSel11;
    p->VirDiFuncSel[11] = FunCodeUnion.code.VI_VDIFuncSel12;
    p->VirDiFuncSel[12] = FunCodeUnion.code.VI_VDIFuncSel13;
    p->VirDiFuncSel[13] = FunCodeUnion.code.VI_VDIFuncSel14;
    p->VirDiFuncSel[14] = FunCodeUnion.code.VI_VDIFuncSel15;
    p->VirDiFuncSel[15] = FunCodeUnion.code.VI_VDIFuncSel16;
    /*根据功能码对16个虚拟Di端子的逻辑有效的电平或边缘进行选择*/
    p->VirDiLogicSel[0] = FunCodeUnion.code.VI_VDILogicSel1;
    p->VirDiLogicSel[1] = FunCodeUnion.code.VI_VDILogicSel2;
    p->VirDiLogicSel[2] = FunCodeUnion.code.VI_VDILogicSel3;
    p->VirDiLogicSel[3] = FunCodeUnion.code.VI_VDILogicSel4;
    p->VirDiLogicSel[4] = FunCodeUnion.code.VI_VDILogicSel5;
    p->VirDiLogicSel[5] = FunCodeUnion.code.VI_VDILogicSel6;
    p->VirDiLogicSel[6] = FunCodeUnion.code.VI_VDILogicSel7;
    p->VirDiLogicSel[7] = FunCodeUnion.code.VI_VDILogicSel8;
    p->VirDiLogicSel[8] = FunCodeUnion.code.VI_VDILogicSel9;
    p->VirDiLogicSel[9] = FunCodeUnion.code.VI_VDILogicSel10;
    p->VirDiLogicSel[10] = FunCodeUnion.code.VI_VDILogicSel11;
    p->VirDiLogicSel[11] = FunCodeUnion.code.VI_VDILogicSel12;
    p->VirDiLogicSel[12] = FunCodeUnion.code.VI_VDILogicSel13;
    p->VirDiLogicSel[13] = FunCodeUnion.code.VI_VDILogicSel14;
    p->VirDiLogicSel[14] = FunCodeUnion.code.VI_VDILogicSel15;
    p->VirDiLogicSel[15] = FunCodeUnion.code.VI_VDILogicSel16;

    for(i=0;i<16;i++)
    {
        if(p->VirDiFuncSel[i] == 2) //当虚拟Di端口的选择的Di功能为/ALM_RST 时，逻辑选择为高
        {
            p->VirDiLogicSel[i] = 1;
        }
    }

}

/*******************************************************************************
  函数名:  void InitVirDoFuncAndLogic(STR_VIR_DO_STATE *p)
  输入:    虚拟Di功能码
  输出:    p->VirDoFuncSel[i]；p->VirDoLogicSel[i]；
  子函数:
  描述:   初始化虚拟Do的功能选择和逻辑有效电平的选择
  返回值:
********************************************************************************/
void InitVirDoFuncAndLogic(STR_VIR_DO_STATE *p)
{
    /*根据功能码对16个Do端子进行17个Do功能中的一个进行选择*/
    p->VirDoFuncSel[0] = FunCodeUnion.code.VI_VDOFuncSel1;
    p->VirDoFuncSel[1] = FunCodeUnion.code.VI_VDOFuncSel2;
    p->VirDoFuncSel[2] = FunCodeUnion.code.VI_VDOFuncSel3;
    p->VirDoFuncSel[3] = FunCodeUnion.code.VI_VDOFuncSel4;
    p->VirDoFuncSel[4] = FunCodeUnion.code.VI_VDOFuncSel5;
    p->VirDoFuncSel[5] = FunCodeUnion.code.VI_VDOFuncSel6;
    p->VirDoFuncSel[6] = FunCodeUnion.code.VI_VDOFuncSel7;
    p->VirDoFuncSel[7] = FunCodeUnion.code.VI_VDOFuncSel8;
    p->VirDoFuncSel[8] = FunCodeUnion.code.VI_VDOFuncSel9;
    p->VirDoFuncSel[9] = FunCodeUnion.code.VI_VDOFuncSel10;
    p->VirDoFuncSel[10] = FunCodeUnion.code.VI_VDOFuncSel11;
    p->VirDoFuncSel[11] = FunCodeUnion.code.VI_VDOFuncSel12;
    p->VirDoFuncSel[12] = FunCodeUnion.code.VI_VDOFuncSel13;
    p->VirDoFuncSel[13] = FunCodeUnion.code.VI_VDOFuncSel14;
    p->VirDoFuncSel[14] = FunCodeUnion.code.VI_VDOFuncSel15;
    p->VirDoFuncSel[15] = FunCodeUnion.code.VI_VDOFuncSel16;

    /*根据功能码对16个Do端子的逻辑有效的电平或边缘进行选择*/
    p->VirDoLogicSel[0] = FunCodeUnion.code.VI_VDOLogicSel1;
    p->VirDoLogicSel[1] = FunCodeUnion.code.VI_VDOLogicSel2;
    p->VirDoLogicSel[2] = FunCodeUnion.code.VI_VDOLogicSel3;
    p->VirDoLogicSel[3] = FunCodeUnion.code.VI_VDOLogicSel4;
    p->VirDoLogicSel[4] = FunCodeUnion.code.VI_VDOLogicSel5;
    p->VirDoLogicSel[5] = FunCodeUnion.code.VI_VDOLogicSel6;
    p->VirDoLogicSel[6] = FunCodeUnion.code.VI_VDOLogicSel7;
    p->VirDoLogicSel[7] = FunCodeUnion.code.VI_VDOLogicSel8;
    p->VirDoLogicSel[8] = FunCodeUnion.code.VI_VDOLogicSel9;
    p->VirDoLogicSel[9] = FunCodeUnion.code.VI_VDOLogicSel10;
    p->VirDoLogicSel[10] = FunCodeUnion.code.VI_VDOLogicSel11;
    p->VirDoLogicSel[11] = FunCodeUnion.code.VI_VDOLogicSel12;
    p->VirDoLogicSel[12] = FunCodeUnion.code.VI_VDOLogicSel13;
    p->VirDoLogicSel[13] = FunCodeUnion.code.VI_VDOLogicSel14;
    p->VirDoLogicSel[14] = FunCodeUnion.code.VI_VDOLogicSel15;
    p->VirDoLogicSel[15] = FunCodeUnion.code.VI_VDOLogicSel16;
}

/*******************************************************************************
  函数名: void VirDiFuncSetErr(STR_VIR_DI_STATE *p)
  输入:
  输出:   故障报警
  子函数: PostErrMsg(DILOADERR)
  描述:   当虚拟Di端口的对应Di功能选择重复时，或者Di功能选择超限时，进行故障报警
  返回值:
********************************************************************************/
void VirDiFuncSetErr(STR_VIR_DI_STATE *p) //16个虚拟Di端口功能分配错误，进行故障处理
{
    Uint16 i = 0;
    Uint16 j = 0;

    //if(FunCodeUnion.code.CM_UseVDI == 1)   // 如果虚拟Di使能时，则进行正常Di功能重复选择报错
    //{
        for(i=0;i<16;i++)
        {
            if( (*VirtualDiPortFunSelAddList[i]) > DI_FUNC_SEL_MAX)  //Di功能选择超出可选范围报错处理，防溢出检测
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
                if(  ((*VirtualDiPortFunSelAddList[i]) == 1)
                    ||((*VirtualDiPortFunSelAddList[i]) == 4)
                    ||((*VirtualDiPortFunSelAddList[i]) == 5)
                    ||((*VirtualDiPortFunSelAddList[i]) == 6)
                    ||((*VirtualDiPortFunSelAddList[i]) == 7)
                    ||((*VirtualDiPortFunSelAddList[i]) == 8)
                    ||((*VirtualDiPortFunSelAddList[i]) == 9)
                    ||((*VirtualDiPortFunSelAddList[i]) == 10)
                    ||((*VirtualDiPortFunSelAddList[i]) == 11)
                    ||((*VirtualDiPortFunSelAddList[i]) == 13)
                    ||((*VirtualDiPortFunSelAddList[i]) == 20)
                    ||((*VirtualDiPortFunSelAddList[i]) == 21)
                    ||((*VirtualDiPortFunSelAddList[i]) == 22)
                    ||((*VirtualDiPortFunSelAddList[i]) == 23)
                    ||((*VirtualDiPortFunSelAddList[i]) == 24)
                    ||((*VirtualDiPortFunSelAddList[i]) == 28)
                    ||((*VirtualDiPortFunSelAddList[i]) == 29)
                    ||((*VirtualDiPortFunSelAddList[i]) == 30)
                    ||((*VirtualDiPortFunSelAddList[i]) == 33)
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
            
            if(((*VirtualDiPortFunSelAddList[i]) > 0) && ((*VirtualDiPortFunSelAddList[i]) < (DI_FUNC_SEL_MAX + 1)))  //在32个Di功能中，若其中一个Di功能被多次分配到不同的虚拟Di端口上，则进行报错处理
            {
                for(j=0;j<i;j++)
                {
                    if((*VirtualDiPortFunSelAddList[i]) == (*VirtualDiPortFunSelAddList[j]))
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
    //}
}


/*******************************************************************************
  函数名: void VirDoFuncSetErr(STR_VIR_DO_STATE *p)
  输入:   p->VirDoFuncSel[i]
  输出:
  子函数: PostErrMsg(DOLOADERR);
  描述:   判断16个虚拟Do端口上的Do功能的分配是否超限和重复分配，若超限和重复分配
          则报故障错误
  返回值:
********************************************************************************/
void VirDoFuncSetErr(STR_VIR_DO_STATE *p)
{
    Uint8  Temp = 0;
    Uint16 i = 0;
    //Uint16 j = 0;

    //if(FunCodeUnion.code.CM_UseVDO == 1)
    //{
        for(i=0;i<16;i++)
        {
            if( (*VirtualDoPortFunSelAddList[i]) > DO_FUNC_SEL_MAX)    //防溢出检测
            {
                if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                {
                    PostErrMsg(DOLOADERR);
                }
                return;
            }

#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
            if(FunCodeUnion.code.OEM_LocalModeEn==1)
            {}
            else
            {
                if(((*VirtualDoPortFunSelAddList[i]) ==6)
                    ||((*VirtualDoPortFunSelAddList[i]) ==15)
                    ||((*VirtualDoPortFunSelAddList[i]) ==16)
                    ||((*VirtualDoPortFunSelAddList[i]) ==17))    //防溢出检测
                {                                           
                    if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                    {
                        PostErrMsg(DOLOADERR);
                    }              
                    return;                                   
                } 
            }
#endif






          /*  if(( (*VirtualDoPortFunSelAddList[i]) > 0) && ( (*VirtualDoPortFunSelAddList[i]) < (DO_FUNC_SEL_MAX + 1)))
            {
                for(j=0;j<i;j++)
                {
                    if((*VirtualDoPortFunSelAddList[j]) == (*VirtualDoPortFunSelAddList[i]))  //16个虚拟Di端口功能分配错误，进行故障处理
                    {
                        if (STR_FUNC_Gvar.Monitor.HighLevelErrCode != DOLOADERR)
                        {
                            PostErrMsg(DOLOADERR);
                        }
                        return;
                    }
                }
            }*/
            if( ((*VirtualDoPortFunSelAddList[i]) == 9) && (FunCodeUnion.code.CM_UseVDO == 1)  )  //判断是否分配了抱闸信号
            {
                Temp = 1;
            }
        }

        if( Temp == 1 )  //判断是否分配了抱闸信号
        {
            p->BrakeVirDoEn = 1;
        }
        else
        {
            p->BrakeVirDoEn = 0;
        }
}

/*******************************************************************************
  函数名: void DealVirDi(STR_VIR_DI_STATE *p)
  输入:   p->VirDiFuncSel[i]；p->VirDiLogicSel[i]；p->VirDiStateNew.all；p->VirDiStateOld.all
  输出:   UNI_DivarReg.all
  子函数:
  描述:   根据功能码设计的逻辑电平和由通讯给定的当前逻辑电平相匹配时，把UNI_DivarReg.all
          中相应的Di功能置位。
  返回值:
********************************************************************************/

void DealVirDi(STR_VIR_DI_STATE *p)
{
    Uint16 i = 0;
    static Uint8 VirDiLogicState = 0;

    p->VirDiPortEnState = STR_FUNC_Gvar.DiDoOutput.VirtualDiPortState;

    for(i=0;i<16;i++)
    {
        VirDiLogicState = ((p->VirDiStateNew.all>>i)&0x01)+(((p->VirDiStateOld.all>>i)&0x01)<<1);

        if((p->VirDiFuncSel[i] >0) && (p->VirDiFuncSel[i] < (DI_FUNC_SEL_MAX + 1)))  //若分配时为0，则无定义
        {

            if( ((p->VirDiLogicSel[i] == 0)&&(((p->VirDiStateNew.all>>i)&0x01)))||
                ((p->VirDiLogicSel[i] == 1)&&(VirDiLogicState == 1))
              ) //DI为高1或者DI逻辑由0变到1，使能相应Di功能
            {
                DivarReg |= ((Uint64)0x01L<<(Uint32)(p->VirDiFuncSel[i]-1));
                p->VirDiPortEnState |= (0x01<<i);
            }
            else //否则不使能相应Di功能
            {
                DivarReg &= ~((Uint64)0x01L<<(Uint32)(p->VirDiFuncSel[i]-1));
                p->VirDiPortEnState &= ~(0x01<<i);
            }
        }
    }
    p->VirDiStateOld.all = p->VirDiStateNew.all;

    STR_FUNC_Gvar.DiDoOutput.VirtualDiPortState = p->VirDiPortEnState;
}

/*******************************************************************************
  函数名: void VirDoProcess(STR_VIR_DO_STATE *p)
  输入:   p->VirDoFuncSel[i]；VirDoLogicSel[i]；STR_FUNC_Gvar.DovarReg.all
  输出:   虚拟Do功能码 FunCodeUnion.code.VI_AllVDOLevel
  子函数:
  描述:   根据当前虚拟Do端口的逻辑状态和STR_FUNC_Gvar.DovarReg.all，判断Do端口的
          相应的Do功能是否使能
  返回值:
********************************************************************************/
void VirDoProcess(STR_VIR_DO_STATE *p)
{
    Uint8 i = 0;

    p->VirDoPortEnState = STR_FUNC_Gvar.DiDoOutput.VirtualDoPortState;
    for (i=0; i<16; i++)   //对16个虚拟Do端口进行循环处理
    {
        if((p->VirDoFuncSel[i] > 0) && (p->VirDoFuncSel[i] <= DO_FUNC_SEL_MAX))   //当虚拟Do端口的功能没有超限时，则根据STR_FUNC_Gvar.DovarReg.all进行相应的Do功能输出
        {
            if(p->VirDoLogicSel[i] == 0)
            {
                if ( ((UNI_DovarReg.all>>(p->VirDoFuncSel[i]-1))&0x0001) == 1 )
                {
                    UNI_VirDoReg.all |= (((UNI_DovarReg.all>>(p->VirDoFuncSel[i]-1))&0x0001) << i);
                    p->VirDoPortEnState |= (0x0001<<i);     //相应Do端口使能
                }
                else
                {
                    UNI_VirDoReg.all &= ~((((~UNI_DovarReg.all)>>(p->VirDoFuncSel[i]-1))&0x0001) << i);
                    p->VirDoPortEnState &= ~(0x0001<<i);    //相应Do端口不使能
                }
            }
            else
            {
                if ( ((UNI_DovarReg.all>>(p->VirDoFuncSel[i]-1))&0x0001) == 1 )
                {
                    UNI_VirDoReg.all &= ~(((UNI_DovarReg.all>>(p->VirDoFuncSel[i]-1))&0x0001) << i);
                    p->VirDoPortEnState |= (0x0001<<i);    //相应Do端口使能
                }
                else
                {
                    UNI_VirDoReg.all |= ((((~UNI_DovarReg.all)>>(p->VirDoFuncSel[i]-1))&0x0001) << i);
                    p->VirDoPortEnState &= ~(0x0001<<i);    //相应Do端口不使能
                }
            }
        }
        else  //当虚拟Do端口的功能选择超限时，Do功能输出为功能码设定的默认值
        {
            if ( ((FunCodeUnion.code.CM_VDODefaultValue>>i) & 0x0001) == 1 )
            {
                UNI_VirDoReg.all |= (((FunCodeUnion.code.CM_VDODefaultValue>>i) & 0x0001)<<i);
                p->VirDoPortEnState |= (0x0001<<i);      //相应Do端口使能
            }
            else
            {
                UNI_VirDoReg.all &= ~((((~FunCodeUnion.code.CM_VDODefaultValue)>>i) & 0x0001)<<i);
                p->VirDoPortEnState &= ~(0x0001<<i);     //相应Do端口不使能
            } 
        }
    }
    FunCodeUnion.code.VI_AllVDOLevel = UNI_VirDoReg.all;  //把当前的虚拟Do功能使能状态存入功能码
    STR_FUNC_Gvar.DiDoOutput.VirtualDoPortState = p->VirDoPortEnState;
}



/********************************* END OF FILE *********************************/
