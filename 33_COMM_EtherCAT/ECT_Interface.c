/*************** (C) COPYRIGHT 2013  Inovance Technology Co., Ltd****************
* File Name		: ECT_InterFace.c
* Author		: 	
* Version		:
* Date			: 
* Description	: 接口文件
* Modify		:
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "ECT_InterFace.h"
#include "CANopen_OD.h"
#include "FUNC_FunCode.h"
#include "FUNC_OperEeprom.h" 

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */




/*******************************************************************************
  函数名:  int32 InterpltPosRefCal(void)
  输入:    
  参数：      
  输出:          
  描述:    完成插补计算，输出位置指令，调度应该在位置环        
********************************************************************************/ 
void ECTTest(void)
{
    
}

/*******************************************************************************
  函数名:  void C_ECT_DrvStatus (Uint16 SyncErrLimit)
  输入:    
  参数：      
  输出:          
  描述:    ER08 报错阀值设置      
********************************************************************************/ 
//void C_ECT_DrvStatus (Uint16 SyncErrLimit)
//{
//    SyncErrLimit = FunCodeUnion.code.CM_SyncErrCnt;//H0C-35
//}


/*******************************************************************************
  函数名:  ECT_DrvStatus (Uint16 Port0CRC,Uint16 Port1CRC,Uint16 ForwardErr,
                            Uint16 HandleErr,Uint16 LinkLost,Uint8 SaveFlag)
  输入:    
        参数1：  Port0CRC		端口0  无效帧
        参数2：  Port1CRC		端口1  无效帧
        参数3：  ForwardErr		端口0、1 转发错误
        参数4：  HandleErr		处理单元和PDI 错误
        参数5：  LinkLost		端口0、1 端口丢失
        参数6：  SaveFlag		前面参数保存标志，0，不保存,1 掉电保存
  参数：      
  输出:   无       
  描述: 
********************************************************************************/ 
void C_ECT_DrvStatus (Uint16 Port0CRC,Uint16 Port1CRC,Uint16 ForwardErr,
Uint16 HandleErr,Uint16 LinkLost,Uint16 SyncLostCnt,Uint8 SaveFlag)
{

    FunCodeUnion.code.CM_Port0CRC = Port0CRC    ; //H0C-36
    FunCodeUnion.code.CM_Port1CRC = Port1CRC    ; //H0C-37
    FunCodeUnion.code.CM_ForwardErr = ForwardErr;//H0C-38
    FunCodeUnion.code.CM_HandleErr = HandleErr  ;//H0C-39
    FunCodeUnion.code.CM_LinkLost = LinkLost    ;//H0C-40
	FunCodeUnion.code.CM_SyncErrCnt0 = SyncLostCnt;//H0C-34

    if(SaveFlag==1)
    {
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_Port0CRC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_Port1CRC));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_ForwardErr));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_HandleErr));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_LinkLost));
		SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_SyncErrCnt0));
    }
    else
    {}
}

void C_EcatSetXmlVersion(Uint16 Ver)
{
	FunCodeUnion.code.CM_XmlVersion = Ver; 					//H0C-32
    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.CM_XmlVersion));
}

void C_EcatSetAlStatusCode(Uint16 alStatusCode)
{
	// 不需要保存EEPROM
	FunCodeUnion.code.CM_AlStatusCode = alStatusCode; 					//H0C-33
}
