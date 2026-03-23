/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_MultiBlockPos.c
 创建人：匡两传                 创建日期：2009.12.16
 修订人：何云壮                 修订日期：2012.02.03 
 描述： 
     1.多段位置功能的实现
	 2.

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
#include "FUNC_MultiBlockPos.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
// 每段起步速度, 统一单位为rpm/min
#define MP_STARTSPD  (1)
#define MP_STOPSPD   (1)

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
extern STR_LOCAL_POSCONTROL STR_PosCtrl;

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
static STR_MULTI_BLOCK_POS STR_MultiPCMD = MultiPCMD_Defaults;
static STR_LINE_INTERPLT_ATTRIB STR_MbcLnIntplt	= LineInterpltAttribDflts;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */   
int32 MultiPosHandle(void);     // 实现多段位置功能, 伺服运行时调用
void MultiPosInit(void);       // 多段位置参数初始化, 伺服OFF时重新初始化
void MultiPosReset(void);      // 参数复位
void MultiPosRunUpdate(void);  // 实时更新多段位置参数

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline Uint16 GetNextBlockNum(void); //DI切换时获取段号


/*******************************************************************************
  函数名: void MultiPosHandle(void) 
  输  入: STR_MultiPCMD-多段位置结构变量        
  输  出:    
  子函数: LineIntpltInit() 插补初始化
          GetNextBlockNum() 通过DI输入切换段号时, 获取下一段的段号
		  LineIntplt() 给出插补位置指令
  描  述: 针对多段位置模式分别处理, 检查当前插补情况, 启动下一段插补  
********************************************************************************/ 
int32 MultiPosHandle(void)
{
	static Uint32 s_counter=0;     // 用于计算等待时间
    static Uint16 s_OldDIValue = INVALID; // 记录上次DI的值  	
	static Uint16 s_DISenCnt = 0;  // DI使能次数  
	static int8 firstflag = 1;
	  
    Uint32 downTime=0;   // 从指定速率减速到停止速率0的减速时间
    Uint32 riseTime=0;   // 从0起始速度加速到指定速率的加速时间
    int64  moveDist=0;   // 当前段位置长度(指令脉冲)
    int32 runSpeed=0;    // 匀速时的速度
    int32 startSpeed;    // 起始速度
	Uint16 isRealActive; // 判断是否真的触发信号
	int8 distsPrdctSign = 0; // 相邻两段位置指令长度之积的符号, 判断前后两段指令的方向是否相反时用到
	int32 diffPulse = 0;     // 前后两段指令之差
	Uint16 nextNum = 0;  // 连续模式循环运行时下一段的序号
    static int32  PreAbsPosRef = 0;
    static int32  PreNumerator = 0;              
    static int32  PreDenominator = 0;             

	STR_MULTI_BLOCK_POS *pMbc = &STR_MultiPCMD;
	STR_LINE_INTERPLT_ATTRIB *pMIntp = &STR_MbcLnIntplt;

    //只有在位置模式和位置指令源为多段位置且伺服ON时才有效
	if ((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
		&& (STR_FUNC_Gvar.MonitorFlag.bit.RunMod == POSMOD))
	{   // 判断多段使能信号
		if ((s_OldDIValue != STR_FUNC_Gvar.DivarRegLw.bit.PosInSen) && (INVALID == s_OldDIValue))
	    {   // 使能信号从无效到有效状态
	      	isRealActive = 1;  //多段使能启动
	      	pMbc->RunFlag = 1; // 多段位置运行标志置1
	      	
			// 断使能时正在插补的话可能留有余量, 设置余量处理方式
	      	if (IGNORE_REST == FunCodeUnion.code.MP_RemdPosDealFlg)	
			{
				pMbc->RefleshNum = 0;
				pMIntp->RealIntpltVal = 0; // 连续运行方式计算起步速度时用到
			}

			// 已执行的段数不小于设置的总执行段数时将已执行的段数置0
			if(pMbc->RefleshNum >= pMbc->ExeBlockNum)
            {
                pMbc->RefleshNum = 0;
			}

			// 下面的处理可以做到每次开使能时立即运行，没必要等待上段的时间
			pMbc->WaitFlag = 1;

            //20150119 市场反馈 等待时间设置为上限时 触发后无法立即运行
            //初始值按16K调度计算
			s_counter = 0xffff0000;//10000001L * 16L; //避免10000 S的对比失效
	    }
        else if ((s_OldDIValue != STR_FUNC_Gvar.DivarRegLw.bit.PosInSen)
                 && (VALID == s_OldDIValue) && (pMbc->RunMode !=2))
        {   // 使能信号从有效到无效
	      	isRealActive = 2; //断多段使能，开始降速
            PreAbsPosRef = 0x80000000;
            PreNumerator = 0;
            PreDenominator = 0;
	    }
		else
		{
			isRealActive = 0;
		}

        if(STR_PosCtrl.IsMultiPosCycleType == 0)
        {
            PreAbsPosRef = 0x80000000;
            PreNumerator = 0;
            PreDenominator = 0; 
        }


		// 保存多段位置使能信号DI端口的状态
	    s_OldDIValue = STR_FUNC_Gvar.DivarRegLw.bit.PosInSen;

		startSpeed = MP_STARTSPD;  // 获取当前段起步速度       

	    switch (pMbc->RunMode)//根据运行模式处理切换
	    {
	        case ONCE_TYPE: //单次运行
	        {
	          	// 断使能处理
				if ((isRealActive == 2 ) && (pMIntp->IntpltStatus == 1))
				{   //断使能时正在插补
				    ResetIntpltDownLength(pMIntp);
					pMIntp->LineRemainLengthQ16 = pMIntp->DownLengthQ16;
					break;
				}
	          			
	            if ((0 == pMIntp->IntpltStatus) && (1 == pMbc->RunFlag)
                    && (VALID == STR_FUNC_Gvar.DivarRegLw.bit.PosInSen)
					&& (ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < STR_InnerGvarPosCtrl.Parrive))//(ABS(STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr) < STR_InnerGvarPosCtrl.Parrive))
	            {   // 前段运行结束须满足如下四个条件:
				    // 插补状态, 多段位置运行标志为TRUE,
					// 多段位置使能信号为有效, 当前位置随动误差小于定位误差
	                if (!pMbc->WaitFlag) //等待处理
	                {
	                    pMbc->WaitFlag = 1;  // 多段位置等待标志置1
						s_counter = 0;       //开始计算等待时间
	                }
	                
					if (++s_counter < pMbc->WaitTimePerBlock[pMbc->RefleshNum])
	                {   //等待时间没到
	                    break;
	                }

	                //结束等待处理
	                pMbc->WaitFlag = 0;
	                // s_counter = 0;
	            						
	                if (pMbc->RefleshNum >= pMbc->ExeBlockNum)
	                {   // 已执行的段数 不小于设置的总执行段数
	                    pMbc->RefleshNum = 0;       // 已执行的段数置0
	                    pMIntp->IntpltStatus = 0; // 插补状态置0
	                    pMbc->RunFlag = 0;          // 单循环，运行一遍就停止
	                    break;
	                }
	                else
	                {
					    riseTime = pMbc->RiseorDownTime[pMbc->RefleshNum]; // 获取当前段加速时间
						downTime = riseTime;                               // 获取当前段减速时间
						runSpeed = pMbc->RunSpeed[pMbc->RefleshNum];   // 获取当前段匀速运行速度

						// 获取当前段位置指令长度(脉冲)
						if (INCREAMENTAL_TYPE == pMbc->RefType) // 相对位置指令
						{
							moveDist = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
                   
							if (VALID == STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel)
							{
							    moveDist = 0 - moveDist;
							}
						}
						else if (ABSOLUTE_TYPE == pMbc->RefType)// 绝对位置指令
						{
							//=========full cloop ,内外切换模式不能用
							if(FunCodeUnion.code.FC_FeedbackMode == 0)//非全闭环
							{
								
								moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
											+ STR_InnerGvarPosCtrl.CurrentAbsPos 
											- STR_FUNC_Gvar.PosCtrl.PosFdb)
										 * (int64)STR_InnerGvarPosCtrl.Denominator
										 / (int64)STR_InnerGvarPosCtrl.Numerator;
							}
							else if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
							{
								moveDist = (STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
											+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
											- STR_FUNC_Gvar.PosCtrl.ExPosFdb)
										 * (int64)STR_InnerGvarPosCtrl.Denominator
										 / (int64)STR_InnerGvarPosCtrl.Numerator;
									
							}
							else if((FunCodeUnion.code.FC_ExInErrFilterTime == 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
							{
								
								moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
											+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
											- STR_FUNC_Gvar.PosCtrl.PosFdb)
										 * (int64)STR_InnerGvarPosCtrl.Denominator
										 / (int64)STR_InnerGvarPosCtrl.Numerator;
							}
							else
							{
								PostErrMsg(MULTPOSCLASHFULCLOP); //参数设置错误
							}
							//===============
							moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum] - moveDist;   
						}
							  
					    // 启动下一段插补
	                    LineIntpltInit(startSpeed, runSpeed, MP_STOPSPD, riseTime, downTime,
						               moveDist, pMIntp);
			            pMbc->CurrentBlock =  pMbc->RefleshNum;   
	                }
	                
	                pMbc->RefleshNum++; // 更新段标号
	            }  // end if () 前段运行结束判断
	        } // end case ONCE_TYPE: //单次运行
	        break;
	        
	        case CYCLE_TYPE: // 循环运行
	        {
			 	// 断使能处理
				if ((isRealActive == 2 ) && (pMIntp->IntpltStatus == 1))
				{   //断使能时正在插补
				    ResetIntpltDownLength(pMIntp);
					pMIntp->LineRemainLengthQ16 = pMIntp->DownLengthQ16;
					break;
				}

	            if ((0 == pMIntp->IntpltStatus)
				    && (VALID == STR_FUNC_Gvar.DivarRegLw.bit.PosInSen)
				    && (ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < STR_InnerGvarPosCtrl.Parrive))//(ABS(STR_FUNC_Gvar.PosCtrl.PosAmplifErr) < STR_InnerGvarPosCtrl.Parrive))
	            {	//前段结束了
	                //等待处理
	                if (!pMbc->WaitFlag)
	                {
	                    pMbc->WaitFlag = 1;
						s_counter = 0; //等待计时开始
	                }

					if (++s_counter < pMbc->WaitTimePerBlock[pMbc->RefleshNum])
	                {   //等待时间没到
	                    break;
	                }
					
	                //结束等待处理
	                pMbc->WaitFlag = 0;
	                // s_counter = 0;
	             
	                if(pMbc->RefleshNum >= pMbc->ExeBlockNum)
	                {
	                    pMbc->RefleshNum = 0;	 //从头开始                  
	                }

				    riseTime = pMbc->RiseorDownTime[pMbc->RefleshNum]; // 获取当前段加速时间
					downTime = riseTime;                               // 获取当前段减速时间
					runSpeed = pMbc->RunSpeed[pMbc->RefleshNum];   // 获取当前段匀速运行速度
					
					// 获取当前段位置指令长度(脉冲)
					if (INCREAMENTAL_TYPE == pMbc->RefType) // 相对位置指令
					{
						moveDist = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
						if (VALID == STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel)
						{
						    moveDist = 0 - moveDist;
						}
					}
					else if (ABSOLUTE_TYPE == pMbc->RefType)// 绝对位置指令
					{
						// CurrentAbsPos是当前值, 所示需要减去最近一次的反馈
						//=========full cloop ,内外切换模式不能用
                        if( (PreAbsPosRef == pMbc->PulsNumPerBlock[pMbc->RefleshNum])
                         && (PreNumerator == STR_InnerGvarPosCtrl.Numerator)
                         && (PreDenominator == STR_InnerGvarPosCtrl.Denominator) )
                        {      
                            moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum];                       
                        }
						else if(FunCodeUnion.code.FC_FeedbackMode == 0)//非全闭环
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_InnerGvarPosCtrl.CurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							moveDist = (STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.ExPosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
								
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime == 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else
						{
							PostErrMsg(MULTPOSCLASHFULCLOP); //参数设置错误
						}

                        PreAbsPosRef = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
                        PreNumerator = STR_InnerGvarPosCtrl.Numerator;
                        PreDenominator = STR_InnerGvarPosCtrl.Denominator;

						//===============
						moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum] - moveDist;  
					}

	                //启动下一段
	                LineIntpltInit(startSpeed, runSpeed, MP_STOPSPD, riseTime, downTime,
					               moveDist, pMIntp);    
			        pMbc->CurrentBlock =  pMbc->RefleshNum;  //记录当前段             
	                pMbc->RefleshNum++;  // 更新段标号
	            }
	        }
	        break;
	        
	        case DI_SWITCH_TYPE://DI切换模式
	        {
	            // 判断是否真的切换，避免一次切换时切换信号太长造成误操作
	            if (isRealActive == 1)
	            {	            	
	                pMbc->RefleshNum = GetNextBlockNum(); //记录更新时的段号, 在段切换时有用
				    riseTime = pMbc->RiseorDownTime[pMbc->RefleshNum]; // 获取当前段加速时间
					downTime = riseTime;                               // 获取当前段减速时间
					runSpeed = pMbc->RunSpeed[pMbc->RefleshNum];   // 获取当前段匀速运行速度
					
					// 获取当前段位置指令长度(脉冲)
					if (INCREAMENTAL_TYPE == pMbc->RefType) // 相对位置指令
					{
						moveDist = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
						if (VALID == STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel)
						{
						    moveDist = 0 - moveDist;
						}
					}
					else if (ABSOLUTE_TYPE == pMbc->RefType)// 绝对位置指令
					{
						//=========full cloop ,内外切换模式不能用
						if(FunCodeUnion.code.FC_FeedbackMode == 0)//非全闭环
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_InnerGvarPosCtrl.CurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							moveDist = (STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.ExPosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
								
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime == 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else
						{
							PostErrMsg(MULTPOSCLASHFULCLOP); //参数设置错误
						}
						//===============
						moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum] - moveDist;  
					}
					
	                s_DISenCnt++;  // DI使能次数                  	
	            }
	            
	            if (s_DISenCnt != 0)
	            {
					s_DISenCnt = 0;

					if (pMIntp->IntpltStatus == 0)
	                {								 
					   // 启动插补
	                    LineIntpltInit(startSpeed, runSpeed, MP_STOPSPD, riseTime, downTime,
						               moveDist, pMIntp);
			            pMbc->CurrentBlock =  pMbc->RefleshNum;  //记录当前段
	                }
	            }
	        }
	        break;
	          
	        case CONTINUOUS_TYPE: //连续模式，没有等待时间
	        {
				// 断使能处理
				if((isRealActive == 2 ) && (pMIntp->IntpltStatus == 1))
				{   //断使能时正在插补
					if(pMIntp->PlanIntpltValQ16>30) //防止报超速
					{
						pMIntp->LineRemainLengthQ16 = pMIntp->PlanIntpltValQ16<<2;
					}
					else
					{
				    	pMIntp->LineRemainLengthQ16 = pMIntp->PlanIntpltValQ16;
					}
					break;
				}

	            if ((pMIntp->IntpltStatus == 0) && (pMbc->RunFlag)
                    && (VALID == STR_FUNC_Gvar.DivarRegLw.bit.PosInSen))
	            {
	                if (pMbc->RefleshNum >= pMbc->ExeBlockNum)
	                {
	                    if (FunCodeUnion.code.MP_Mod4RecyStartSect == 0) //顺序模式起始段
					    {
						    pMbc->RunFlag    = 0;  //单次，运行一遍就停止
						    pMbc->RefleshNum = 0;
							break;
					    }
					    else
					    {						    	                    					    	                    
		                    pMbc->RunFlag    = 1;
							// 全部多段位置都已执行一遍, 循环运行情况下设置下一循环的起始段
							pMbc->RefleshNum = FunCodeUnion.code.MP_Mod4RecyStartSect - 1;
					    }
	                }
					
				    // 顺序运行方式下, 起步速度与另外三种方式不同
					// 注意: 此时的起步速度设置不适当的话会导致段与段之间或者本轮循环的最后
					// 一段与下一轮循环的第一段之间过渡时产生毛刺
                    if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0x30)      //直线电机的处理
                    {
                        if(firstflag == 1)
                        {
                            startSpeed = MP_STARTSPD ;  //统一单位为rpm/min
                            firstflag = 0;
                        }
                        else
                        {
                             startSpeed = pMIntp->RealIntpltVal * STR_FUNC_Gvar.System.PosFreq
          			    	         / UNI_FUNC_MTRToFUNC_InitList.List.EncRev;  //统一单位为mm/s                        }
						}
                    }
                    else
                    {
                        if(firstflag == 1)
                        {
                            startSpeed = MP_STARTSPD ;  //统一单位为rpm/min
                            firstflag = 0;
                        }
                        else
                        {
                            startSpeed = (int64)pMIntp->RealIntpltVal * STR_FUNC_Gvar.System.PosFreq * 60L
                            / UNI_FUNC_MTRToFUNC_InitList.List.EncRev 
                            *(int64)STR_InnerGvarPosCtrl.Numerator
							/(int64)STR_InnerGvarPosCtrl.Denominator;
                        }
                    }
					riseTime = pMbc->RiseorDownTime[pMbc->RefleshNum]; // 获取当前段加速时间
					downTime = riseTime;                               // 获取当前段减速时间
					runSpeed = pMbc->RunSpeed[pMbc->RefleshNum];   // 获取当前段匀速运行速度

					// 顺序运行方式下, 根据位置指令类型不同, 需要作不同处理
                    if (INCREAMENTAL_TYPE == pMbc->RefType) //相对型位置指令
                    {
					    moveDist = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
								 
						// 补偿上一段多或者少插补的脉冲数 pMIntp->MbpExtraPlsQ16
	                    if (moveDist > 0)
						{   // 
					        moveDist = moveDist - pMIntp->MbpExtraPlsQ16;
						}
						else
						{
					   	    moveDist = moveDist + pMIntp->MbpExtraPlsQ16;
						}
                    
                        pMIntp->MbpExtraPlsQ16 = 0;
						
						// 当前段和下一段位置指令长度之积的符号, 用于判断前后两段方向是否相反
						if (pMbc->RefleshNum < pMbc->ExeBlockNum - 1)
						{
						    nextNum = pMbc->RefleshNum+1;
						}
						else
						{
						    if (FunCodeUnion.code.MP_Mod4RecyStartSect == 0)
							{
						        nextNum = 0;
							}
							else
							{
							    nextNum = FunCodeUnion.code.MP_Mod4RecyStartSect - 1;
							}
						}
						distsPrdctSign = ((moveDist>>31) & 0x1)
						               + ((pMbc->PulsNumPerBlock[nextNum]>>31) & 0x1);
						
						if (VALID == STR_FUNC_Gvar.DivarRegLw.bit.POSDirSel)
						{
						    moveDist = 0 - moveDist;
						}
                    }
                    else if (ABSOLUTE_TYPE == pMbc->RefType)//绝对型位置指令
                    {
						// CurrentAbsPos是当前值, 所示需要减去最近一次的反馈
						//=========full cloop ,内外切换模式不能用
                        if( (PreAbsPosRef == pMbc->PulsNumPerBlock[pMbc->RefleshNum])
                         && (PreNumerator == STR_InnerGvarPosCtrl.Numerator)
                         && (PreDenominator == STR_InnerGvarPosCtrl.Denominator) )
                        {      
                            moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum];                         
                        }
						else if(FunCodeUnion.code.FC_FeedbackMode == 0)//非全闭环
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_InnerGvarPosCtrl.CurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime != 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							moveDist = (STR_FUNC_Gvar.PosCtrl.ExPosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.ExPosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
								
						}
						else if((FunCodeUnion.code.FC_ExInErrFilterTime == 0) &&  (FunCodeUnion.code.FC_FeedbackMode == 1))
						{
							
							moveDist = (STR_FUNC_Gvar.PosCtrl.PosAmplifErr
										+ STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos 
										- STR_FUNC_Gvar.PosCtrl.PosFdb)
									 * (int64)STR_InnerGvarPosCtrl.Denominator
									 / (int64)STR_InnerGvarPosCtrl.Numerator;
						}
						else
						{
							PostErrMsg(MULTPOSCLASHFULCLOP); //参数设置错误
						}

                        PreAbsPosRef = pMbc->PulsNumPerBlock[pMbc->RefleshNum];
                        PreNumerator = STR_InnerGvarPosCtrl.Numerator;
                        PreDenominator = STR_InnerGvarPosCtrl.Denominator;

						//===============  
						moveDist = (int32)pMbc->PulsNumPerBlock[pMbc->RefleshNum] - moveDist;   
					
					    // 当前段和下一段绝对位置指令长度之积的符号, 用于判断前后两段方向是否相反
						if (pMbc->RefleshNum < pMbc->ExeBlockNum - 1)
						{
						    nextNum = pMbc->RefleshNum+1;
						}
						else
						{
						    if (FunCodeUnion.code.MP_Mod4RecyStartSect == 0)
							{
						        nextNum = 0;
							}
							else
							{
							    nextNum = FunCodeUnion.code.MP_Mod4RecyStartSect-1;
							}
						}
						diffPulse = pMbc->PulsNumPerBlock[nextNum]
						           - pMbc->PulsNumPerBlock[pMbc->RefleshNum];
						distsPrdctSign = ((moveDist>>31) & 0x1)
						               + ((diffPulse>>31) & 0x1);
                    }
					
					if ( ((pMbc->RefleshNum == (pMbc->ExeBlockNum -1))
					          && (FunCodeUnion.code.MP_Mod4RecyStartSect == 0))
				        || (distsPrdctSign == 1) )	                                       
					{   // 这里判断顺序运行方式下当前段是否要加入减速时间
						// 条件1: 当前段是最后一段 并且 不循环运行
						// 条件2: 当前段和下一段位置指令方向相反
						// 满足条件1 或者 条件2 就需要加入减速时间
						downTime = riseTime;
					}
					else
					{
						downTime = 0; // 取消该段减速时间, 以实现段与段连续切换
						pMIntp->MbpContinue = 1; // 设置需要特殊处理标志
					}

                    //启动下一段
                    LineIntpltInit(startSpeed, runSpeed, MP_STOPSPD, riseTime, downTime,
					               moveDist, pMIntp);       
		            pMbc->CurrentBlock = pMbc->RefleshNum;  //记录当前段
		            pMbc->RefleshNum++;  // 更新段标号						
	            }  // end if ((pMIntp->IntpltStatus == 0) && (pMbc->RunFlag)
	        } // end case
	        break;
	    } // end switch  
	}
    else
    {		
        PreAbsPosRef = 0x80000000;
        PreNumerator = 0;
        PreDenominator = 0;
    }
	
 	// 给出使用多段位置时每个插补周期的指令
	return LineIntplt(pMIntp);    
}


/*******************************************************************************
  函数名: void MultiPosInit(void) 
  输  入:        
  输  出:   
  子函数:                                       
  描  述: 多段位置的初始化函数  
********************************************************************************/
void MultiPosInit(void)
{
	// 记录多段位置插补结构变量 STR_MbcLnIntplt 的地址
	RecordIntpltStruct(&STR_MbcLnIntplt, 0);
	
	STR_MultiPCMD.CurrentBlock   = 0;   //当前段号所存值, 供显示用
	STR_MultiPCMD.WaitFlag       = 0;   //等待标志
	STR_MultiPCMD.RunFlag        = 1;   //运行标志
	STR_MultiPCMD.RefleshNum     = 0;	//段索引
	//0：单次运行，1：循环运行，2：DI切换模式 3：顺序模式
	STR_MultiPCMD.RunMode  =  FunCodeUnion.code.MP_PosRunMode;
}


/*******************************************************************************
  函数名: void MultiPosReset(void) 
  输  入:        
  输  出:   
  子函数:                                       
  描  述: 多段位置参数复位  
********************************************************************************/
void MultiPosReset(void)
{
    STR_MultiPCMD.RunFlag  = 0;

	// 直线插补器参数复位
	if (1 == STR_MbcLnIntplt.IntpltStatus)
	{
	    LineIntpltReset(&STR_MbcLnIntplt);
	}
}


/*******************************************************************************
  函数名: void MultiPosRunUpdate(void) 
  输  入:        
  输  出:   
  子函数:                                       
  描  述: 参数实时更新 
********************************************************************************/
void MultiPosRunUpdate(void)
{
    int16 i = 0;
	Uint16 *p;
    Uint32 temp1 = 0, temp2 = 0;
	
	STR_MultiPCMD.ExeBlockNum =  FunCodeUnion.code.MP_PosExeSects;  //多段总段数
	STR_MultiPCMD.TimeUnit    =  FunCodeUnion.code.MP_PosTimeUnit;  //时间单位
	STR_MultiPCMD.RefType = FunCodeUnion.code.MP_PosRefType;   //位置指令类型    
	STR_MultiPCMD.RemainderHndFlg = FunCodeUnion.code.MP_RemdPosDealFlg; //剩余位移处理方式
	STR_MultiPCMD.WaitTimePerBlock[0] = 0;//起始段前应该是0等待的
	
	// 取第一段第一个功能码的地址
	p = &(FunCodeUnion.code.MP_SEC1_PosRef_Lo);
	for (i = 0; i < MAX_BLOCKS; i++)
	{
		// 获取第i段位置指令脉冲数
		STR_MultiPCMD.PulsNumPerBlock[i] = (int32)(*p) + ((int32)(*(p+1)) << 16);

		// 获取第i段运行速度
		STR_MultiPCMD.RunSpeed[i] = *(p+2);
		
		// 获取第i段加减速时间(换算到以插补周期为单位)
		STR_MultiPCMD.RiseorDownTime[i] = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * (*(p+3))) >> 13;
		
	    // 获取第i段等待时间, 这里根据用户选择的时间单位不同而乘不同系数
		// 以将等待时间换算为以插补周期为单位
		if (1 == STR_MultiPCMD.TimeUnit)
		{   // 等待时间以 秒 为单位, 换算到以插补周期为单位
            temp1 = STR_FUNC_Gvar.System.PosFreq;
            temp2 = *(p+4);
		    STR_MultiPCMD.WaitTimePerBlock[i+1] = temp1 * temp2;
		}
		else
		{   // 等待时间以 毫秒 为单位, 换算到以插补周期为单位
            temp1 = *(p+4);
		    STR_MultiPCMD.WaitTimePerBlock[i+1] = (STR_InnerGvarPosCtrl.Ms2PtCoefQ13 * temp1) >> 13;
		}
		
	    // 获取多段位置第(i+1)段的第一个功能码的地址
	    p += 5;
	}
	p = 0;

	// 循环连续运行时自第二轮开始的起始段, 判断它是否大于总的执行段数
	if(FunCodeUnion.code.MP_Mod4RecyStartSect > FunCodeUnion.code.MP_PosExeSects )
	{
		FunCodeUnion.code.MP_Mod4RecyStartSect = 0;
	}
}


/*******************************************************************************
  函数名: void GetNextBlockNum(void) 
  输  入:        
  输  出:   
  子函数:                                       
  描  述: DI切换模式时获取段号  
********************************************************************************/
Static_Inline Uint16 GetNextBlockNum(void)
{
	Uint16 BlockNum = 0;
	
	// DI端口 Cmd1, Cmd2, Cmd3, Cmd4 分别表示四位二进制的第 0, 1, 2, 3位
	if(VALID == STR_FUNC_Gvar.DivarRegLw.bit.Cmd1)
	   BlockNum += 1;
	   
	if(VALID == STR_FUNC_Gvar.DivarRegLw.bit.Cmd2)
	   BlockNum += 2;
	   
	if(VALID == STR_FUNC_Gvar.DivarRegLw.bit.Cmd3)
	   BlockNum += 4;
	   
	if(VALID == STR_FUNC_Gvar.DivarRegLw.bit.Cmd4)
	   BlockNum += 8;
	
	return BlockNum;
}

/********************************* END OF FILE *********************************/
