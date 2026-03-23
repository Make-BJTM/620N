/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_InterfaceProcess.c
 创建人:    李浩                  创建日期：2012.03.26
 描述:
    1.
    2.
 修改记录：
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_GlobalVariable.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义*/

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
UNI_FUNC_MTRTOFUNC_INITLIST      UNI_FUNC_MTRToFUNC_InitList;        //MTR 传递过来，供FUNC使用的接口变量表-上电初始化使用
UNI_FUNC_MTRTOFUNC_LIST_1kHz     UNI_FUNC_MTRToFUNC_SlowList_1kHz;   //MTR 传递过来，供FUNC使用的接口变量表-主循环调用-慢表
UNI_FUNC_MTRTOFUNC_LIST_16kHz    UNI_FUNC_MTRToFUNC_FastList_16kHz;  //MTR 传递过来，供FUNC使用的接口变量表-中断调用-快表
UNI_FUNC_MTRTOFUNC_FSALIST_16kHz    UNI_FUNC_MTRToFUNC_FSAFastList_16kHz;  //MTR 传递过来，供FUNC扫频使用的接口变量表-中断后端调用-快表

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void FUNC_Interrupt16kHz_InterfaceDeal(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


/*******************************************************************************
  函数名:  void FUNC_Interrupt16kHz_InterfaceDeal(void)
  输入:    
  输出:    
  描述: 功能模块内调用的，在主循环1K调度程序中进行的接口处理函数

********************************************************************************/
void FUNC_Interrupt16kHz_InterfaceDeal(void)
{
    STR_FUNC_Gvar.ToqCtrl.DovarReg_ToqReach = (Uint8)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.DOvarReg_ToqReach;
   
    STR_FUNC_Gvar.ToqCtrl.DovarReg_Vlt = (Uint8)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.DOvarReg_Vlt;

    STR_FUNC_Gvar.ToqCtrl.DovarReg_Clt = (Uint8)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.DOvarReg_Clt;  //扭矩限制DO信号输出
    
    
    //量纲由数字量转换成额定的0.1% 
    UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef = (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef * 1000 +
        Sign_NP(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqRef) * (UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT >> 1) ) / 
        UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT;  

    //量纲由数字量转换成额定的0.1% 
    UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb = (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb * 1000 + 
        Sign_NP(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IqFdb) * (UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT >> 1) ) /
        UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT;

    //量纲由数字量转换成额定的0.1%
    UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb = (UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb * 1000 +
        Sign_NP(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.IdFdb) * (UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT >> 1) ) /
        UNI_FUNC_MTRToFUNC_InitList.List.IqRate_MT;  //量纲由数字量转换成额定的0.1%

    //示波器采样上次FPGA中断的速度指令
    STR_FUNC_Gvar.OscTarget.SpdRefOld = STR_FUNC_Gvar.SpdCtrl.SpdRef;

}

/********************************* END OF FILE *********************************/
