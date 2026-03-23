/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    MTR_PDFFCtrl.c  
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.      
    2. 	   
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "MTR_PDFFCtrl.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"   

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
int32 PDFFCtrl(STR_PDFFCONTROLLER *p, int32 Ref, int32 Fdb_P, int32 Fdb_I);    //PDFF控制器

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */



/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.  
    2.
********************************************************************************/
int32 PDFFCtrl(STR_PDFFCONTROLLER *p, int32 Ref, int32 Fdb_P, int32 Fdb_I)
{
    int32  RefTemp = 0;
    int32  FdbTemp = 0;
    int32  ErrKf = 0;
    int64  OutputTemp = 0;
    int64  Err = 0;              //误差

    //前馈与比例部分的计算
    RefTemp = (int32)(((int64)Ref * (int64)p->Kf_Q12) >> 12);
	FdbTemp = (int32)(((int64)Fdb_P * (int64)p->DampingKfPlus1_Q12)>>12);//FdbTemp = Fdb(1+DampingKf)
    ErrKf = RefTemp - FdbTemp;
    OutputTemp = (int64)p->Kp * ErrKf;

    //积分部分的计算
    Err = Ref - Fdb_I;            //误差计算

    //积分运算
    if(p->Ki_Q10 == 0)
    {
        p->KiSum = 0;     //PI切换到P时,之前累加和KiSum清零
    }
    else if(p->SaturaFlag == 0)
    {
        p->KiSum += ((int64)p->Ki_Q10 * (int64)Err);
    }
    else if((p->SaturaFlag == 1) && (Err > 0))   //负向遇限处理
    {
        p->KiSum += ((int64)p->Ki_Q10 * (int64)Err); 
    }
    else if((p->SaturaFlag == 2) && (Err < 0))   //正向遇限处理
    {
        p->KiSum += ((int64)p->Ki_Q10 * (int64)Err);
    } 

    OutputTemp += (p->KiSum >> 10);

    //输出定标计算
    OutputTemp = OutputTemp * (int64)p->Kb_Scal_Q38;
    OutputTemp = OutputTemp >> 38;




    //输出限幅处理
    if((int32)OutputTemp > 32767) 
    {
        OutputTemp = 32767;
    }
    else if ((int32)OutputTemp < -32767) 
    {
        OutputTemp = -32767;
    }


    return((int32)OutputTemp);
}

/********************************* END OF FILE *********************************/
