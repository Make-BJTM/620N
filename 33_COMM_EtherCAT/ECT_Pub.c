
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "ECT_Pub.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_ServoError.h" //G2_LH_12.31 主要是提供PstErrMsg()函数
#include "FUNC_ErrorCode.h"
#include "FUNC_GlobalVariable.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define ElecRatioMin   1000000L    //(1/1000)的倒数*1000
#define ElecRatioMax   4000000L    //4000*1000

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */
//STR_ELECRATIO  STR_ElecRatio;




/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */




/*******************************************************************************
  函数名:  void InitECTElecRatio(STR_ELECRATIO *p)
  输入:    
           STR_ElecRatio.ElecGear6091_Numerator;
           STR_ElecRatio.ElecGear6091_Denominator;
  参数：       
  输出:           
  描述:    电子齿轮初始化     
********************************************************************************/ 
/*void InitECTElecRatio(STR_ELECRATIO *p)
{

    Uint32 NumTemp = 0;
    Uint32 DenTemp = 0;
    Uint32 NumDenTemp = 0;       //电子齿轮比暂存变量

    if(p->ElecRatio6091_Numerator == 0)p->ElecRatio6091_Numerator = UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
    
    NumTemp = p->ElecRatio6091_Numerator;
    DenTemp = p->ElecRatio6091_Denominator;

    if(NumTemp < DenTemp)//分子小于分母
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 0;

        NumDenTemp = DenTemp * 1000L/ NumTemp;
       // if(NumDenTemp > ElecRatioMin)PostErrMsg(GEALSETERR3);

    }
    else if(NumTemp > DenTemp)
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 0;
        
        NumDenTemp = NumTemp * 4000L/ DenTemp;
       // if(NumDenTemp > ElecRatioMax)PostErrMsg(GEALSETERR3);
    }
    else
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 1;

    }

    STR_ECTPPVar.ElecRatioNum = p->ElecRatio6091_Numerator;
    STR_ECTPPVar.ElecRatioDen = p->ElecRatio6091_Denominator;
        
        //电子齿轮的余数清零
    p->ElecRatioRemainderQ16 = 0;

}*/



/*******************************************************************************
  函数名:  void ECTElecRatioErrCheck(STR_ELECRATIO *p)
  输入:    
           STR_ElecRatio.ElecGear6091_Numerator;
           STR_ElecRatio.ElecGear6091_Denominator;
  参数：       
  输出:           
  描述:    电子齿轮运行更新   
********************************************************************************/ 
/*void ECTElecRatioUpdate(STR_ELECRATIO *p)
{

    Uint32 NumTemp = 0;
    Uint32 DenTemp = 0;
    Uint32 NumDenTemp = 0;       //电子齿轮比暂存变量

    STR_FUNC_Gvar.PosCtrl.ExPosFeedbackFlag = 0;//内外环切换标志始终为0

    if(p->ElecRatio6091_Numerator == 0)p->ElecRatio6091_Numerator = UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
    
    NumTemp = p->ElecRatio6091_Numerator;
    DenTemp = p->ElecRatio6091_Denominator;


    if(NumTemp < DenTemp)//分子小于分母
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 0;

        NumDenTemp = DenTemp * 1000L/ NumTemp;
        //if(NumDenTemp > ElecRatioMin)PostErrMsg(GEALSETERR3);

    }
    else if(NumTemp > DenTemp)
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 0;
        
        NumDenTemp = NumTemp * 4000L/ DenTemp;
        //if(NumDenTemp > ElecRatioMax)PostErrMsg(GEALSETERR3);
    }
    else
    {
        //齿轮比无效标志
        STR_ECTPPVar.ElecRatioUnEff = 1;

    }
    
    STR_ECTPPVar.ElecRatioNum = p->ElecRatio6091_Numerator;
    STR_ECTPPVar.ElecRatioDen = p->ElecRatio6091_Denominator;
} */






