/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_XintPosition.c
 创建人：
 修订人：何云壮                 修订日期：2012.02.09 
 描述： 
     1.中断定长功能的实现

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_InterfaceProcess.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_PosCtrl.h" 
#include "FUNC_LineInterplt.h"
#include "FUNC_XIntPosition.h"
#include "FUNC_GPIODriver.h"     // ST平台补丁函数需要用到GPIO_ReadDI9DSP()

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
static STR_XINTPOS_ATTRIB  STR_XintPos = XintPosAttribDflts;
static STR_LINE_INTERPLT_ATTRIB STR_XiLnIntplt = LineInterpltAttribDflts;


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */

void XintPosDrvConfig(void);
void XintPosEnJudgment(int32 DeltaPosFdb);

int32 XintPosCmd(void);  // 实现中断定长, 给出插补指令 
void XintPosReset(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */

/*******************************************************************************
  函数名: void XintPosDrvConfig() 
  输  入:           
  输  出:   
  子函数: PostErrMsg()                                      
  描  述: 根据FunCodeUnion.code.PL_XintPosSel 配置中断, 具体根据硬件平台是用
          ST ARM处理器 还是 TI DSP处理器而有所不同. 上电初始化一次
********************************************************************************/
void XintPosDrvConfig(void)
{
    //中断定长触发配置 提供给FPGA
    volatile Uint16 *XintTrigConfig   = (Uint16 *)(FPGA_BASE + (0x0000000B << 1)); 

    //620P 中断定长边沿触发选择，FPGA接口
    if ( STR_FUNC_Gvar.DiDoOutput.XintPosEn == 1 )
    {
		if(FunCodeUnion.code.FC_FeedbackMode != 0)
		{
			PostErrMsg(MULTPOSCLASHFULCLOP); //参数设置错误
		}

        switch (FunCodeUnion.code.DILogicSel9)
        {
            case 0:
                 //中断定长使能，Di9触发输入，外部信号下降沿触发,
                 //但由于Di9硬件取反的原因，此处给FPGA配置为上升沿触发
                 *XintTrigConfig = 0x0900; //与620P反相
                 break;

            case 1: 
                 //中断定长使能，Di9触发输入，外部信号上升沿触发,
                 //但由于Di9硬件取反的原因，此处给FPGA配置为下降沿触发
                 *XintTrigConfig = 0x0500; //与620P反相
                 break;

            case 2:
                 //中断定长使能，Di9触发输入，外部信号上升沿触发,
                 //但由于Di9硬件取反的原因，此处给FPGA配置为下降沿触发
                 *XintTrigConfig = 0x0500; //与620P反相 
                 break;

            case 3:
                 //中断定长使能，Di9触发输入，外部信号下降沿触发,
                 //但由于Di9硬件取反的原因，此处给FPGA配置为上升沿触发
                 *XintTrigConfig = 0x0900; //与620P反相
                 break;

            case 4:
                 //中断定长使能，Di9触发输入，外部信号边沿触发,
                 //虽然Di9硬件取反，但仍给FPGA配置为边沿触发
                 *XintTrigConfig = 0x0D00; //与620P反相
                 break;
            default :
                 *XintTrigConfig = 0;     //禁止中断定长功能的中断触发
                 PostErrMsg(DEFUALTERR);
                 return;  //注意此处RETURN了
        }
    }
    else    //手轮使能及倍率保留，但Di8和Di9端口不再作为手轮的AB正交脉冲输入，因此此处不用考虑手轮
    {
        *XintTrigConfig = 0;    //禁止中断定长功能的中断触发
        return;  //注意此处RETURN了
    }      

}


/*******************************************************************************
  函数名: 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: 1.只要使能了中断定长功能，就检测中断信号是否产生
          2.但中断信号的产生，不一定就让中断定长工作，中断定长工作还有其它限制条件
          3.只有在中断定长工作时，才读取FPGA在中断定长中断产生时的绝对位置锁存值。
********************************************************************************/
void XintPosEnJudgment(int32 DeltaPosFdb)
{
    volatile Uint16 *PosBufferLow  = (Uint16 *)(FPGA_BASE + (0x00000019 << 1));  //中断定长反馈低16位
    volatile int16  *PosBufferHigh = (int16  *)(FPGA_BASE + (0x0000001A << 1));  //中断定长反馈高16位
	int64 moveDist = 0;
	int32 moveTime = 0;
    int32 Temp = 0;

    if (STR_FUNC_Gvar.DiDoOutput.XintPosEn == 0 ) return;

    if (0 == ((*PosBufferHigh) & 0xC000))   return;//FPGA位置锁存未执行

    //当中断定长中断产生时，且H0526=0；且当前滤波后的速度反馈绝对值小于10rpm，则不响应中断定长
	if ( (FunCodeUnion.code.PL_XintMoveSpd == 0) 
      && (ABS(STR_FUNC_Gvar.SpdCtrl.SpdAfterDoFlt) < 10) )       
    {
        return;
    }

    if ((2 == STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)    // 要求伺服正在运行
       && (POSMOD == STR_FUNC_Gvar.MonitorFlag.bit.RunMod)     // 当前处于位置模式
       && (0 == STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork)   // 此刻没有运行中断定长
       && (0 == STR_FUNC_Gvar.DivarRegHi.bit.XintInhibit)        // 没有屏蔽中断定长触发
       && (0 == STR_InnerGvarPosCtrl.MutexBit.bit.HomeWork))   // 当前没有运行回原点
    {           
        //-----重点内容-----
        Temp = (*PosBufferLow) & 0x7FFF;   //单位25ns
        //符号位处理
        if(0x4000 == (Temp & 0x4000))       
        {            
            Temp = Temp | 0xFFFF8000;  //负数
        }
        moveTime = (int32)Temp * 25L;     //单位ns

        moveDist = ((int64)DeltaPosFdb * (int64)((int32)moveTime) * (int64)STR_FUNC_Gvar.System.ToqFreq) /1000000000LL;
        //-----重点内容-----
                        
        STR_XintPos.PosXintLatch = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt + moveDist;        

        STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork = 1;    // 置中断定长启用标志
    }
}

/*******************************************************************************
  函数名: void XintPosCmd() 
  输  入:           
  输  出:   
  子函数: LineIntpltInit(), LineIntplt()                                      
  描  述: 计算中断定位距离, 设置运行速度并启动插补过程
********************************************************************************/
int32 XintPosCmd()
{
    static Uint16 isFirst = 1; // 
    int64 moveDist = 0;        // 定长距离
    int64 tempDist = 0;
    int32 zdstartspd = 0;      // 插补起步速度
    Uint32 zdacctime = 0;       // 插补加减速时间
    int32 zdspd = 0;           // 插补最大速度
    int32 deltPos = 0;         // 定长距离补偿值

    if(isFirst) // 限制下面这段代码在启动中断定长的过程中只执行一次
    {
        //清除中断定长完成信号 1：允许下一次中断 
        STR_InnerGvarPosCtrl.MutexBit.bit.Xintcoin = 0;          // 中断定长完成信号复位
        STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin = 0;    //中断定长完成信号取消
        //指定脉冲总数再乘以此时的齿轮比, 左移LINEAMPBIT位参见插补初始化函数说明
        moveDist = A_SHIFT16_PLUS_B((FunCodeUnion.code.PL_XintPosRef_Hi),
                                    (FunCodeUnion.code.PL_XintPosRef_Lo));

        // 根据前一刻的运动方向确定定长距离补偿值 
        //620P所做修改
        if(STR_InnerGvarPosCtrl.MutexBit.bit.PosRefDir == 0)
        {
            //求当前相对于中断时的位置量(绝对值)
            deltPos = STR_InnerGvarPosCtrl.PosFdbAbsValueLast - STR_XintPos.PosXintLatch;

            //定长距离 = 指定长度 - 补偿值(绝对值)
            tempDist = 0-deltPos;
        }
        else
        {   
            //求当前相对于中断时的位置量(绝对值)
            deltPos = - STR_InnerGvarPosCtrl.PosFdbAbsValueLast + STR_XintPos.PosXintLatch; 

            //定长距离 = -指定长度 + 补偿值(绝对值)
            moveDist = 0-moveDist;
            tempDist = deltPos;
        }

        //补偿位置低通滤波器余数
        //存在风险：中断定长运行完后，低通滤波器余数可能不为0，这样就没有走完中断位移长度
        //WZG1470,重要
        tempDist -= - STR_InnerGvarPosCtrl.PosLPFilterRemainder;

        //补偿误差计算时用到的增量位置反馈及位置偏差
        tempDist += STR_FUNC_Gvar.PosCtrl.PosFdb - STR_FUNC_Gvar.PosCtrl.PosAmplifErr;

        // 以上计算出的 tempDist 是乘齿轮比之后的值, 而最终的计算结果是要还未乘齿轮比的值, 
        // 因此先将 tempDist 的值乘齿轮比的倒数
        tempDist = (tempDist * (int64)STR_InnerGvarPosCtrl.Denominator
                                        + ((int64)STR_InnerGvarPosCtrl.Numerator>>1))
                    / (int64)STR_InnerGvarPosCtrl.Numerator;

        // 合并各种补偿值之后的距离
        moveDist += tempDist;

        // 起步速度等于前一时刻的运行速度
        //620P所做修改
        zdstartspd = ABS(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SpdFdb) / 10000L;

        if(zdstartspd < 1L)   zdstartspd = 1L; 
   
        // 设置插补加减速时间, 换算成以插补周期为单位
        zdacctime = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * FunCodeUnion.code.PL_XintAccTime) >> 13; 

        // 设置插补所能达到的最大速度, 不小于起步速度
        if(FunCodeUnion.code.PL_XintMoveSpd == 0)
        {
            zdspd = zdstartspd;
        }
        else
        {
            zdspd = FunCodeUnion.code.PL_XintMoveSpd;
        }

        // 启动中断定长插补过程
        LineIntpltInit(zdstartspd, zdspd, 12, zdacctime, zdacctime, moveDist, &STR_XiLnIntplt);
        isFirst = 0;
    }
    else if ((STR_XiLnIntplt.IntpltStatus == 0) &&
             (ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < STR_InnerGvarPosCtrl.Parrive))
    {
        STR_InnerGvarPosCtrl.MutexBit.bit.Xintcoin = 1;         //中断定长完成标志置1	
        STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin = 1;    //中断定长完成信号Do输出

        if(0 == FunCodeUnion.code.PL_XintFreeEn)
        {   // 该功能码等于0表示不能通过DI解除中断定长锁定状态(复位相关标志位)
            // 因此中断定长完成之后, 这里自动复位相关标志位, 以备下一次使用
            STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork = 0;  // 告知系统中断定长过程结束
            //STR_InnerGvarPosCtrl.MutexBit.bit.Xintcoin = 0;          // 中断定长完成信号复位

            //STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin = 0;    //中断定长完成信号取消
            isFirst = 1;
        }
    }

    // 功能码PL_XintFreeEn == 1 表示只能通过DI输入解除中断定长锁定状态(复位相关标志位)
    // 如果中断定长的锁定状态没有被解除, 
/*
    上海现场试机，要求在中断定长运行中，DI中断定长状态解除信号生效时，
    停止中断定长运行，正常接收位置指令。
    内部讨论，标准版本暂不这么修改，如果上海试机成功，批量购买时，做一个非标提供给客户
    if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.XintFree) && (1 == FunCodeUnion.code.PL_XintFreeEn))
*/
    if ((VALID == STR_FUNC_Gvar.DivarRegLw.bit.XintFree) && (1 == FunCodeUnion.code.PL_XintFreeEn)
        && (1 == STR_InnerGvarPosCtrl.MutexBit.bit.Xintcoin)&&(STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin == 1))
    {
        STR_InnerGvarPosCtrl.MutexBit.bit.XiPosWork = 0; // 告知系统中断定长过程结束
        //STR_InnerGvarPosCtrl.MutexBit.bit.Xintcoin = 0;         // 中断定长完成信号复位

        //STR_FUNC_Gvar.PosCtrl.DovarReg_Xintcoin = 0;    //中断定长完成信号取消
        isFirst = 1;
    }

    if (RUN != STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus)
    {   // 停机及异常时复位插补器
        STR_XiLnIntplt.IntpltStatus = 0;
    }
    
    // 给出每个插补周期的位置指令值
    return LineIntplt(&STR_XiLnIntplt);
}


/*******************************************************************************
  函数名: void XintPosReset() 
  输  入: STR_XiLnIntplt          
  输  出:   
  子函数: LineIntpltReset                                      
  描  述: 复位中断定长的插补结构
********************************************************************************/
void XintPosReset(void)
{
    // 直线插补器参数复位	
    if (1 == STR_XiLnIntplt.IntpltStatus)
	{
        LineIntpltReset(&STR_XiLnIntplt);
	}
}


/********************************* END OF FILE *********************************/
