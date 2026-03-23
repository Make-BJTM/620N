/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    XX.c                                                           
 创建人：XX                创建日期：XX.XX.XX 
 修改人：XX                修改日期：XX.XX.XX 
 修改人：XX                修改日期：XX.XX.XX 
 描述： 
    1.
    2.
/-------------------------------------------------------------/
代码守则(2014年12月)
一 . 嵌套不能超过4层
二． 函数不能超过150行
三． 指针使用前必须有明确值
四． 变量名无二义、见名知义、不能与全局变量同名
五． 子函数名必须体现函数功能
六． 条件判断式和计算表达式中类型不一致量必须强制类型转换
七． 禁止用魔幻数字
八． 全局变量的增减必须评审
九． 条件判断中禁止用非’!’
十． 编译后不能有警告


前提条件 网络进入OP模式，再加上Son且CSP模式。
不满足前提条件  buff深度为0
满足前提条件，开始buff深度为0，等待深度为2，深度变化趋势为 0 1 2 2 2….

 异常： 0 1 3…     0 3….   其它
 只要开始后 深度大于2  就开始使用规则处理

规则：
1. 连续两次深度大于2，剔除掉一个或多个，取buff中保证深度2的单元
2. 连续两次深度等于1，补偿1次，如果非CSP补偿单元与深度2单元一致，如果是CSP，
   根据上次绝对位置指令和深度2单元绝对位置计算插补绝对位置。
3. 连续两次深度为1、 0，补偿2次，如果非CSP补偿单元与深度2单元一致，如果是CSP，
   根据上次绝对位置指令和深度2单元绝对位置计算插补绝对位置。
********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_FunCode.h"
#include "FUNC_GlobalVariable.h"
#include "CANopen_OD.h"
#include "CANopen_Pub.h"
#include "FUNC_CSPCmdBuffer.h"



/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义*/
#define START0          0
#define START1          1
#define BUFFER          2
#define ESTIMATE        3
#define NOBUUFFER       4


/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */
//暂无

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
STR_CSPCMDBUFFERVAR   STR_BufferVar;
STR_CMDBUFFERVARIABLE STR_CmdBufferVar;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */		 
//暂无

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void CSPBufferVarUpdate(void);
void WrCSPBufferIrq(void);
void RdCSPBufferSync(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void WrBufferIrq_Inner(void);
Static_Inline void RdBufferSync_Inner(void);

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
void CSPBufferVarUpdate(void)
{
    Uint8 IsRunSt = 0;
    
    if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)  IsRunSt = 1;
    
    if(0 == IsRunSt)
    {
		STR_BufferVar.BuffMod = FunCodeUnion.code.CM_PosBuffEnale;
    } 
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
void RdCSPBufferSync(void)
{
	RdBufferSync_Inner();     
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
void WrCSPBufferIrq(void)
{
	WrBufferIrq_Inner(); 
}


/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
Static_Inline void WrBufferIrq_Inner(void)
{
    int32 Temp = 0;
    
    STR_BufferVar.WrAddr ++;
    STR_BufferVar.WrAddr = STR_BufferVar.WrAddr & 0x07;
    
    //目标位置
    Temp = (int32)ObjectDictionaryStandard.ProPosMode.TargetPosition 
				+ (int32)ObjectDictionaryStandard.CstOffset.PosOff;

    //607E bit7=1位置指令反向
    if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x80)== 0x80) Temp = 0 - Temp;
    STR_BufferVar.Cmd[STR_BufferVar.WrAddr].TargetPos = Temp;    
    
    //速度前馈
    Temp = (int32)ObjectDictionaryStandard.CstOffset.VelOff;     //60B1h
    //607E bit6=1速度指令反向	
    if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x40)== 0x40)Temp = 0 - Temp;    
    STR_BufferVar.Cmd[STR_BufferVar.WrAddr].SpdRef = Temp;      
    
    //转矩前馈 
    Temp = (int16)ObjectDictionaryStandard.CstOffset.ToqOff;    //60B2h  
    //607E bit5=1转矩指令反向
    if ((((Uint8)ObjectDictionaryStandard.ProPosMode.Polarity) & 0x20)== 0x20)Temp = 0 - Temp;
    STR_BufferVar.Cmd[STR_BufferVar.WrAddr].ToqRef = Temp;     
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
Static_Inline void SetToCpu0Ram(int8 Addr)
{
    STR_BufferVar.EstimatedDeltaPos = 
        STR_BufferVar.Cmd[Addr].TargetPos - STR_CmdBufferVar.CSP_TargetPos;
    
    STR_CmdBufferVar.CSP_TargetPos = STR_BufferVar.Cmd[Addr].TargetPos;    //目标位置 
    STR_CmdBufferVar.CSP_SpdRef    = STR_BufferVar.Cmd[Addr].SpdRef;       //速度前馈
    STR_CmdBufferVar.CSP_ToqRef    = STR_BufferVar.Cmd[Addr].ToqRef;       //转矩前馈 
}

/*******************************************************************************
  函数名:  
  输入:    
  输出:    
  描述:  
********************************************************************************/ 
Static_Inline void RdBufferSync_Inner(void)
{
    Uint8 RdAddrTemp = 0;
    Uint8 Deep[2];
    
    if( (STR_BufferVar.BuffMod != 1) 
     || (STR_FUNC_Gvar.MonitorFlag.bit.ESMState==0)
     || (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN)
     || (STR_CanSyscontrol.Mode != ECTCSPMOD) )
    {
        STR_BufferVar.WrAddrLatch[2] = STR_BufferVar.WrAddr;
        STR_BufferVar.WrAddrLatch[1] = STR_BufferVar.WrAddr;
        STR_BufferVar.WrAddrLatch[0] = STR_BufferVar.WrAddr;
        
        STR_BufferVar.RdAddrLatch[2] = STR_BufferVar.WrAddr;
        STR_BufferVar.RdAddrLatch[1] = STR_BufferVar.WrAddr;
        STR_BufferVar.RdAddrLatch[0] = STR_BufferVar.WrAddr;        
        
        SetToCpu0Ram(STR_BufferVar.WrAddr);
       
        STR_BufferVar.BuffState = START0;          
        return;
    }

    Deep[1] = STR_BufferVar.WrAddrLatch[2] - STR_BufferVar.RdAddrLatch[2];
    Deep[0] = STR_BufferVar.WrAddrLatch[1] - STR_BufferVar.RdAddrLatch[1];    

    Deep[1] = Deep[1] & 0x07; 
    Deep[0] = Deep[0] & 0x07;        
    
    switch(STR_BufferVar.BuffState)
    {
        case START0:   //启动状态
        case START1:  
            STR_BufferVar.BuffState ++;
            RdAddrTemp = STR_BufferVar.RdAddrLatch[0]; 
            SetToCpu0Ram(RdAddrTemp);
        break;
    
        case BUFFER:  //正常状态  下个状态为正常状态或预估状态
            if((Deep[1] > 3) && (Deep[0] > 3))
            {
                RdAddrTemp = STR_BufferVar.WrAddr - 2;
                RdAddrTemp = RdAddrTemp & 0x07;
                SetToCpu0Ram(RdAddrTemp);
            }
            else if((Deep[1] == 1) && (Deep[0] == 1))
            {
                STR_CmdBufferVar.CSP_TargetPos += STR_BufferVar.EstimatedDeltaPos;
                RdAddrTemp = STR_BufferVar.RdAddrLatch[0]; 
            }
            else if((Deep[1] == 1) && (Deep[0] == 0))
            {
                STR_CmdBufferVar.CSP_TargetPos += STR_BufferVar.EstimatedDeltaPos;
                RdAddrTemp = STR_BufferVar.RdAddrLatch[0];                 
                STR_BufferVar.BuffState = ESTIMATE;
            }
            else
            {            
                RdAddrTemp = STR_BufferVar.RdAddrLatch[0] + 1;
                RdAddrTemp = RdAddrTemp & 0x07;
                SetToCpu0Ram(RdAddrTemp);
            }
            
        break;

        case ESTIMATE:  //预估状态 保持1次 下个状态为故障状态或正常状态            
            if(STR_BufferVar.RdAddrLatch[0] == STR_BufferVar.WrAddr)
            {
                RdAddrTemp = STR_BufferVar.WrAddr;
                SetToCpu0Ram(RdAddrTemp);
                STR_BufferVar.BuffState = NOBUUFFER;
            }
            else
            {
                STR_CmdBufferVar.CSP_TargetPos += STR_BufferVar.EstimatedDeltaPos;
                RdAddrTemp = STR_BufferVar.RdAddrLatch[0] + 1;
                RdAddrTemp = RdAddrTemp & 0x07;
                STR_BufferVar.BuffState = BUFFER;
            }
        break;

        
        case NOBUUFFER:   //故障状态 不再切回正常状态
            RdAddrTemp = STR_BufferVar.WrAddr;
            SetToCpu0Ram(RdAddrTemp);
        break;
        
        default:
            STR_BufferVar.BuffState = NOBUUFFER;
            RdAddrTemp = STR_BufferVar.WrAddr;
            SetToCpu0Ram(RdAddrTemp);            
        break;
    }   

    STR_BufferVar.WrAddrLatch[2] = STR_BufferVar.WrAddrLatch[1];
    STR_BufferVar.WrAddrLatch[1] = STR_BufferVar.WrAddrLatch[0];
    STR_BufferVar.WrAddrLatch[0] = STR_BufferVar.WrAddr;

    STR_BufferVar.RdAddrLatch[2] = STR_BufferVar.RdAddrLatch[1];
    STR_BufferVar.RdAddrLatch[1] = STR_BufferVar.RdAddrLatch[0];
    STR_BufferVar.RdAddrLatch[0] = RdAddrTemp;     
}

/********************************* END OF FILE *********************************/

