/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "ECT_Probe.h"
#include "FUNC_PosCtrl.h"          
#include "CANopen_OD.h"
#include "FUNC_GlobalVariable.h"
#include "CANopen_Pub.h"
#include "FUNC_DiDo.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/



/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */



/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

STR_PROBEVARIBLES  STR_ProbeVar;


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */




/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void TouchProbeDrvConfig(void);
void TouchProbe1Func(void);
void TouchProbe2Func(void);
void TouchProbe1EnJudgment(int32 DeltaPosFdb);
void TouchProbe2EnJudgment(int32 DeltaPosFdb);
void TouchProbeStatus(void);
void TouchProbe1ZeroLatch(int64 LatchPos);
void TouchProbe2ZeroLatch(int64 LatchPos);

/*******************************************************************************
  函数名: void TouchProbeInit(void)

  输入:   无 
  输出:   无
  子函数:无
  描述:  探针初始化
********************************************************************************/ 
void TouchProbeInit(void)
{
    STR_ProbeVar.Probe1Ctrl.all = 0;
    STR_ProbeVar.Probe2Ctrl.all = 0;
    STR_ProbeVar.Probe1Status.all = 0;
    STR_ProbeVar.Probe2Status.all = 0;
    STR_ProbeVar.AbsPosLow = UNI_FUNC_MTRToFUNC_InitList.List.EncRev/10000L;
    STR_ProbeVar.AbsPosHig = UNI_FUNC_MTRToFUNC_InitList.List.EncRev - STR_ProbeVar.AbsPosLow;
    
    TouchProbeDrvConfig();

}

/*******************************************************************************
  函数名: void TouchProbeDrvConfig() 
  输  入:           
  输  出:   
  子函数: PostErrMsg()                                      
  描  述: 根据FunCodeUnion.code.PL_XintPosSel 配置中断, 具体根据硬件平台是用
          ST ARM处理器 还是 TI DSP处理器而有所不同. 上电初始化一次
********************************************************************************/
void TouchProbeDrvConfig(void)
{
    //中断定长触发配置 提供给FPGA
    volatile Uint16 *TouchProbeTrigConfig   = (Uint16 *)(FPGA_BASE + (0x0000000B << 1));

    Uint8 TouchProbe1Trig = 0;
    Uint8 TouchProbe2Trig = 0;
    
    //620N 探针1，FPGA接口
    if (STR_FUNC_Gvar.DiDoOutput.TouchProbe1En == 1)
    {
        switch (FunCodeUnion.code.DILogicSel8)
        {
            case 0:
                 TouchProbe1Trig = 0x05; //下降沿
                 break;

            case 1: 
			     TouchProbe1Trig = 0x09; //上升沿
                 break;

            case 2:
                TouchProbe1Trig = 0x09; //上升沿
                 break;

            case 3:
                 TouchProbe1Trig = 0x05; //下降沿
			break;

            case 4:
                 TouchProbe1Trig = 0x0D; //上升沿和下降沿
                 break;
            default :
                 TouchProbe1Trig = 0;  
                 PostErrMsg(DEFUALTERR);
                 return;  //注意此处RETURN了
        }
    }
    
    //620N 探针2，FPGA接口
    if (STR_FUNC_Gvar.DiDoOutput.TouchProbe2En == 1)
    {
        switch (FunCodeUnion.code.DILogicSel9)
        {
            case 0:
                 TouchProbe2Trig = 0x05; //下降沿
                 break;

            case 1: 
			     TouchProbe2Trig = 0x09; //上升沿
                 break;

            case 2:
                TouchProbe2Trig = 0x09; //上升沿
                 break;

            case 3:
                 TouchProbe2Trig = 0x05; //下降沿
			break;

            case 4:
                 TouchProbe2Trig = 0x0D; //上升沿和下降沿
                 break;
            default :
                 TouchProbe2Trig = 0;  
                 PostErrMsg(DEFUALTERR);
                 return;  //注意此处RETURN了

        }
    }
    if ((STR_FUNC_Gvar.DiDoOutput.TouchProbe1En==1)||(STR_FUNC_Gvar.DiDoOutput.TouchProbe2En==1))
    {
        *TouchProbeTrigConfig = ((Uint16)TouchProbe2Trig<<8) + (Uint16)TouchProbe1Trig;
    }
    else
    {
        *TouchProbeTrigConfig = 0;
        return;
    }

}
/*******************************************************************************
  函数名: void TouchProbeFunc(void)

  输入:   无 
  输出:   无
  子函数:无
  描述:   探针功能
********************************************************************************/ 
void TouchProbeFunc(int32 DeltaPosFdb)
{
    TouchProbe1Func();
    TouchProbe1EnJudgment(DeltaPosFdb);

    TouchProbe2Func();
    TouchProbe2EnJudgment(DeltaPosFdb);

    TouchProbeStatus();
}
/*******************************************************************************
  函数名: 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: ?
********************************************************************************/
void TouchProbe1EnJudgment(int32 DeltaPosFdb)
{
    volatile Uint16 *PosBuffer1Low  = (Uint16 *)(FPGA_BASE + (0x00000015 << 1));  //中断定长反馈低16位
    volatile int16  *PosBuffer1High = (int16  *)(FPGA_BASE + (0x00000016 << 1));  //中断定长反馈高16位

    int64 moveDist = 0;
	int32 moveTime = 0;
    int32 Temp = 0;
    
    Uint32 AbsPos = 0;
    static Uint32 AbsPosLatch = 0;

    Uint16 TouchProbe1Active =0;
	static Uint8 ContinousCount1 = 0;

	if(STR_ProbeVar.Probe1Ctrl.bit.Enable == 0)//探针未使能 
    {
        STR_ProbeVar.Probe1Status.all = 0;
		ContinousCount1 = 0;
        STR_ProbeVar.TP1PosCnt =0;
        STR_ProbeVar.TP1NegCnt =0;
        return;
    }
   
    STR_ProbeVar.Probe1Status.bit.Enable = 1; //探针1已使能

	STR_ProbeVar.Probe1Ctrl.bit.RiseEdgeEnable = (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x00000010)>>4);
	STR_ProbeVar.Probe1Ctrl.bit.DownEdgeEnable =  (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x00000020)>>5);

    if(STR_ProbeVar.Probe1Ctrl.bit.TrigSource == 1) //Z信号作为探针触发
    {

        if(FunCodeUnion.code.CM_ECATHost!=2)
        {
            STR_ProbeVar.Probe1Status.bit.TrigSource = 1;
            STR_ProbeVar.Probe1Status.bit.TrigStatus = 0;
        }
	
        if(FunCodeUnion.code.MT_MotorModel == 14000)return;//增量式电机，用Z中断

        AbsPos = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SingleAbsPosFdb;

        Temp = ABS(DeltaPosFdb);

        if(((AbsPos <= (STR_ProbeVar.AbsPosLow+Temp))&&(AbsPosLatch >= (STR_ProbeVar.AbsPosHig-Temp)))||
            ((AbsPosLatch <= (STR_ProbeVar.AbsPosLow+Temp))&&(AbsPos >= (STR_ProbeVar.AbsPosHig-Temp))))
        {
            if(DeltaPosFdb>0)//正转--AbsPos小，AbsPosLatch大
            {
                Temp = AbsPos;
            }
            else if(DeltaPosFdb<0)//反转--AbsPos大，AbsPosLatch小
            {
                Temp = UNI_FUNC_MTRToFUNC_InitList.List.EncRev - AbsPos;
                Temp = - Temp;
            }
            else
            {
                Temp = 0;
            }
            if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
		    {
				moveDist = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
			}
			else
			{
				moveDist = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt - Temp;;
			}

            TouchProbe1ZeroLatch(moveDist);
			
			
        }
        else
        {}
        AbsPosLatch = AbsPos;
        return;
    }
    
    if(STR_FUNC_Gvar.DiDoOutput.TouchProbe1En ==0)return;


    if(FunCodeUnion.code.CM_ECATHost!=2)
    {
        STR_ProbeVar.Probe1Status.bit.TrigSource = 0;
    	STR_ProbeVar.Probe1Status.bit.TrigStatus = 1-((AuxFunCodeUnion.code.DP_DIState&0x0080)>>7);
    }
	 	
	
    TouchProbe1Active = (*PosBuffer1High) & 0xC000;

    if((0 == TouchProbe1Active)||(0xC000 == TouchProbe1Active))return;//FPGA位置锁存未执行

    //-----重点内容-----
    Temp = (*PosBuffer1Low) & 0x7FFF;   //单位25ns
    //符号位处理
    if(0x4000 == (Temp & 0x4000))       
    {            
        Temp = Temp | 0xFFFF8000;  //负数
    }
    moveTime = (int32)Temp * 25L;     //单位ns

    moveDist = ((int64)DeltaPosFdb * (int64)((int32)moveTime) * (int64)STR_FUNC_Gvar.System.ToqFreq) /1000000000L;
    

    //负载机械位置
    if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
    {
        moveDist = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + moveDist;
        if((int64)moveDist < 0)
        {
	        if(((int64)moveDist + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 0)
	        {
	            moveDist = moveDist % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
	        }
			
	        if(((int64)moveDist + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 100)
	        {
	            moveDist = -(moveDist + STR_InnerGvarPosCtrl.AbsMod2PosUpLmt);
	        }
				
        }
        else
        {
		    if((int64)moveDist >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
		    {
		        moveDist = moveDist % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
		    
			}

	        if(((int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - (int64)moveDist) <= 100)
	        {
	            moveDist = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - moveDist;
	        }
        }
    }
    else
    {
        //-----重点内容-----
        moveDist = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt + moveDist;
    }

	if(STR_ProbeVar.Probe1Ctrl.bit.RiseEdgeEnable==1)//上升沿
    {
		if(0x8000 == TouchProbe1Active)
        {
            if(STR_ProbeVar.Probe1Ctrl.bit.TrigMode == 1)//连续存储模式
            {
                STR_ProbeVar.Pbobe1RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 1;
                STR_ProbeVar.TP1PosCnt++;
                
    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe1Status.bit.TrigSource = ContinousCount1&0x01;
    	            STR_ProbeVar.Probe1Status.bit.TrigStatus = (ContinousCount1&0x02)>>1;
    				ContinousCount1++;
    				if(ContinousCount1>=4)ContinousCount1 = 0;
    			}   
    		}
            else//单次存储模式
            {
                if(STR_ProbeVar.Probe1Status.bit.RiseEdgeStored ==0)
                {
                    STR_ProbeVar.Pbobe1RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                    STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 1;
                }
                
    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe1Status.bit.TrigSource = 0;
    	            STR_ProbeVar.Probe1Status.bit.TrigStatus = 0;
        			ContinousCount1 = 0;
    			}
                STR_ProbeVar.TP1PosCnt =0;
            }
        }
        else
        {}
    }
    else
    {
        STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 0;
    }

    if(STR_ProbeVar.Probe1Ctrl.bit.DownEdgeEnable==1)//下降沿
    {
	    if(0x4000 == TouchProbe1Active)
        {
            if(STR_ProbeVar.Probe1Ctrl.bit.TrigMode == 1)//连续存储模式
            {
                STR_ProbeVar.Pbobe1DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 1;
                STR_ProbeVar.TP1NegCnt++;


    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe1Status.bit.TrigSource = ContinousCount1&0x01;
    	            STR_ProbeVar.Probe1Status.bit.TrigStatus = (ContinousCount1&0x02)>>1;
    				ContinousCount1++;
    				if(ContinousCount1>=4)ContinousCount1 = 0;
    			}   
   
            }
            else//单次存储模式
            {

                if(STR_ProbeVar.Probe1Status.bit.DownEdgeStored ==0)
                {
                    STR_ProbeVar.Pbobe1DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                    STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 1;
                }
                
    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe1Status.bit.TrigSource = 0;
    	            STR_ProbeVar.Probe1Status.bit.TrigStatus = 0;
        			ContinousCount1 = 0;
    			}
                STR_ProbeVar.TP1NegCnt =0;
            }
        }
        else{}
    }
    else
    {
        STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 0;
    }

}

/*******************************************************************************
  函数名: 
  输  入:           
  输  出:   
  子函数:                                      
  描  述: ?
********************************************************************************/
void TouchProbe2EnJudgment(int32 DeltaPosFdb)
{
    volatile Uint16 *PosBuffer2Low  = (Uint16 *)(FPGA_BASE + (0x00000019 << 1));  //中断定长反馈低16位
    volatile int16  *PosBuffer2High = (int16  *)(FPGA_BASE + (0x0000001A << 1));  //中断定长反馈高16位
	int64 moveDist = 0;
	int32 moveTime = 0;
    int32 Temp = 0;

    Uint32 AbsPos = 0;
    static Uint32 AbsPosLatch = 0;

    Uint16 TouchProbe2Active =0;
	static Uint8 ContinousCount2 = 0;
    
    if(STR_ProbeVar.Probe2Ctrl.bit.Enable == 0)//探针未使能 
    {
        //60B9状态字清零
        STR_ProbeVar.Probe2Status.all = 0;
		ContinousCount2 = 0;
        STR_ProbeVar.TP2PosCnt =0;
        STR_ProbeVar.TP2NegCnt =0;
        return;
    }

    //60B9的bit0用于反映探针1是否使能
    STR_ProbeVar.Probe2Status.bit.Enable = 1;

	STR_ProbeVar.Probe2Ctrl.bit.RiseEdgeEnable = (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x00001000)>>12);
	STR_ProbeVar.Probe2Ctrl.bit.DownEdgeEnable =  (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x00002000)>>13);

    if(STR_ProbeVar.Probe2Ctrl.bit.TrigSource == 1) //探针选用的是触发信号是Z信号
    {

        if(FunCodeUnion.code.CM_ECATHost!=2)
        {
    		STR_ProbeVar.Probe2Status.bit.TrigSource = 1;
            STR_ProbeVar.Probe2Status.bit.TrigStatus = 0;
        }

        if(FunCodeUnion.code.MT_MotorModel == 14000)return;//增量式电机，用Z中断

        AbsPos = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.SingleAbsPosFdb;

        Temp = ABS(DeltaPosFdb);
        
        if(((AbsPos <= (STR_ProbeVar.AbsPosLow+Temp))&&(AbsPosLatch >= (STR_ProbeVar.AbsPosHig-Temp)))||
            ((AbsPosLatch <= (STR_ProbeVar.AbsPosLow+Temp))&&(AbsPos >= (STR_ProbeVar.AbsPosHig-Temp))))
        {
            if(DeltaPosFdb>0)//正转--AbsPos小，AbsPosLatch大
            {
                Temp = AbsPos;
            }
            else if(DeltaPosFdb<0)//反转--AbsPos大，AbsPosLatch小
            {
                Temp = UNI_FUNC_MTRToFUNC_InitList.List.EncRev - AbsPos;
                Temp = - Temp;
            }
            else
            {
                Temp = 0;
            }
            //moveDist = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt - Temp;
		    if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
		    {
				moveDist = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
				
			}
			else
			{
				moveDist = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt - Temp;
			}            
            TouchProbe2ZeroLatch(moveDist);

        }
        else
        {}
        AbsPosLatch = AbsPos;
        return;
    }
    
    if(STR_FUNC_Gvar.DiDoOutput.TouchProbe2En ==0)return;//DI功能未设置为39


	
    if(FunCodeUnion.code.CM_ECATHost!=2)
    {
        STR_ProbeVar.Probe2Status.bit.TrigSource = 0;
    	STR_ProbeVar.Probe2Status.bit.TrigStatus = 1-((AuxFunCodeUnion.code.DP_DIState&0x0100)>>8);
    }
	
	
    TouchProbe2Active = (*PosBuffer2High) & 0xC000;

    if((0 == TouchProbe2Active)||(0xC000 == TouchProbe2Active))return;//FPGA位置锁存未执行

    //-----重点内容-----
    Temp = (*PosBuffer2Low) & 0x7FFF;   //单位25ns
    //符号位处理
    if(0x4000 == (Temp & 0x4000))       
    {            
        Temp = Temp | 0xFFFF8000;  //负数
    }
    moveTime = (int32)Temp * 25L;     //单位ns

    moveDist = ((int64)DeltaPosFdb * (int64)((int32)moveTime) * (int64)STR_FUNC_Gvar.System.ToqFreq) /1000000000L;
    //-----重点内容-----
                    
    //负载机械位置
    if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
    {
        moveDist = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + moveDist;
        if((int64)moveDist < 0)
        {
	        if(((int64)moveDist + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 0)
	        {
	            moveDist = moveDist % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
	        }
			
	        if(((int64)moveDist + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 100)
	        {
	            moveDist = -(moveDist + STR_InnerGvarPosCtrl.AbsMod2PosUpLmt);
	        }
				
        }
        else
        {
		    if((int64)moveDist >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
		    {
		        moveDist = moveDist % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
		    
			}

	        if(((int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - (int64)moveDist) <= 100)
	        {
	            moveDist = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - moveDist;
	        }
        }
    }
    else
    {
        moveDist = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt + moveDist;
    }
	

	
    if(STR_ProbeVar.Probe2Ctrl.bit.RiseEdgeEnable==1)//上升沿
    {
	    if(0x8000 == TouchProbe2Active)
        {
            if(STR_ProbeVar.Probe2Ctrl.bit.TrigMode == 1)//连续存储模式
            {
                STR_ProbeVar.Pbobe2RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 1;
                STR_ProbeVar.TP2PosCnt++;
            	            
    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe2Status.bit.TrigSource = ContinousCount2&0x01;
    	            STR_ProbeVar.Probe2Status.bit.TrigStatus = (ContinousCount2&0x02)>>1;
    				ContinousCount2++;
    				if(ContinousCount2>=4)ContinousCount2 = 0;
    			}   
   
    		}
            else//单次存储模式
            {
    			if(STR_ProbeVar.Probe2Status.bit.RiseEdgeStored ==0)
                {
                    STR_ProbeVar.Pbobe2RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                    STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 1;
                }

    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe2Status.bit.TrigSource = 0;
    	            STR_ProbeVar.Probe2Status.bit.TrigStatus = 0;
        			ContinousCount2 = 0;
    			}
                STR_ProbeVar.TP2PosCnt=0;

            }
        }
        else
        {}
    }
    else
    {
        STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 0;
    }

    if(STR_ProbeVar.Probe2Ctrl.bit.DownEdgeEnable==1)//下降沿
    {
	    if(0x4000 == TouchProbe2Active)
        {
            if(STR_ProbeVar.Probe2Ctrl.bit.TrigMode == 1)//连续存储模式
            {
                STR_ProbeVar.Pbobe2DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 1;
                STR_ProbeVar.TP2NegCnt++;

    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe2Status.bit.TrigSource = ContinousCount2&0x01;
    	            STR_ProbeVar.Probe2Status.bit.TrigStatus = (ContinousCount2&0x02)>>1;
    				ContinousCount2++;
    				if(ContinousCount2>=4)ContinousCount2 = 0;
    			}   

            }
            else//单次存储模式
            {
                if(STR_ProbeVar.Probe2Status.bit.DownEdgeStored ==0)
                {
                    STR_ProbeVar.Pbobe2DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,moveDist);//指令单位
                    STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 1;
                }

    			if(FunCodeUnion.code.CM_ECATHost==2)
    			{
    	            STR_ProbeVar.Probe2Status.bit.TrigSource = 0;
    	            STR_ProbeVar.Probe2Status.bit.TrigStatus = 0;
        			ContinousCount2 = 0;
    			}
                STR_ProbeVar.TP2NegCnt=0;
            }
        }
        else{}
    }
    else
    {
        STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 0;
    }

    
}
/*******************************************************************************
  函数名: void TouchProbeStatus(void)

  输入:   无 
  输出:   无
  子函数:无
  描述:   探针功能状态
********************************************************************************/ 
void TouchProbeStatus(void)
{
    ObjectDictionaryStandard.TouchProbe.Pb1PosValatPE = (Uint32)STR_ProbeVar.Pbobe1RiseEdgeValue;
    ObjectDictionaryStandard.TouchProbe.Pb1PosValatNE = (Uint32)STR_ProbeVar.Pbobe1DownEdgeValue;
    ObjectDictionaryStandard.TouchProbe2.TP1PosEdgeCnt = STR_ProbeVar.TP1PosCnt;
    ObjectDictionaryStandard.TouchProbe2.TP1NegEdgeCnt = STR_ProbeVar.TP1NegCnt;


    ObjectDictionaryStandard.TouchProbe.Pb2PosValatPE = (Uint32)STR_ProbeVar.Pbobe2RiseEdgeValue;
    ObjectDictionaryStandard.TouchProbe.Pb2PosValatNE = (Uint32)STR_ProbeVar.Pbobe2DownEdgeValue;
    ObjectDictionaryStandard.TouchProbe2.TP2PosEdgeCnt = STR_ProbeVar.TP2PosCnt;
    ObjectDictionaryStandard.TouchProbe2.TP2NegEdgeCnt = STR_ProbeVar.TP2NegCnt;
    
    ObjectDictionaryStandard.TouchProbe.Status = ((Uint16)STR_ProbeVar.Probe2Status.all<<8) 
                                                 + (Uint16)STR_ProbeVar.Probe1Status.all;
}

/*******************************************************************************
  函数名: void TouchProbe1Func(void)

  输入:   无 
  输出:   无
  子函数:无
  描述:   探针1功能
********************************************************************************/ 
void TouchProbe1Func(void)
{
    Uint8  Probe1Enable = 0;
    static Uint8 Probe1EnableLatch = 0;

    Probe1Enable = (Uint8)(ObjectDictionaryStandard.TouchProbe.Func&0x00000001);
    
    //上升沿使能
    if((Probe1EnableLatch == 0)&&(Probe1Enable == 1))
    {
        //锁存控制字信息
        STR_ProbeVar.Probe1Ctrl.all = (Uint8)(ObjectDictionaryStandard.TouchProbe.Func&0x000000FF);

		//需要锁存的是触发源和触发模式，其他不需要
     }
    else if(Probe1Enable == 0)
    {
        STR_ProbeVar.Probe1Ctrl.bit.Enable = 0;
    }
    Probe1EnableLatch = Probe1Enable;
}


/*******************************************************************************
  函数名: void TouchProbe2Func(void)

  输入:   无 
  输出:   无
  子函数:无
  描述: 探针2功能
********************************************************************************/ 
void TouchProbe2Func(void)
{
    Uint8  Probe2Enable = 0;
    static Uint8 Probe2EnableLatch = 0;
    
    Probe2Enable = (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x00000100)>>8);
	
	//上升沿使能
    if((Probe2EnableLatch == 0)&&(Probe2Enable == 1))
    {
        
		//锁存控制字信息
        STR_ProbeVar.Probe2Ctrl.all = (Uint8)((ObjectDictionaryStandard.TouchProbe.Func&0x0000FF00)>>8);
    }
    else if(Probe2Enable == 0)
    {
        STR_ProbeVar.Probe2Ctrl.bit.Enable = 0;
    }
    Probe2EnableLatch = Probe2Enable;

}

void TouchProbe1ZeroLatch(int64 LatchPos)
{
    static Uint8 ContinousCount3 = 0;
    
    if(STR_ProbeVar.Probe1Ctrl.bit.TrigMode == 1)//连续存储模式
    {
		if(STR_ProbeVar.Probe1Ctrl.bit.RiseEdgeEnable==1)
		{
            STR_ProbeVar.Pbobe1RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
            STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 1;
            STR_ProbeVar.TP1PosCnt++;
            
			if(FunCodeUnion.code.CM_ECATHost==2)
			{
	            STR_ProbeVar.Probe1Status.bit.TrigSource = ContinousCount3&0x01;
	            STR_ProbeVar.Probe1Status.bit.TrigStatus = (ContinousCount3&0x02)>>1;
				ContinousCount3++;
				if(ContinousCount3>=4)ContinousCount3 = 0;
			}   
		}
		else
		{
		   STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 0;
		}

		if(STR_ProbeVar.Probe1Ctrl.bit.DownEdgeEnable==1)
		{
			STR_ProbeVar.Pbobe1DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
			STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 1;
            STR_ProbeVar.TP1NegCnt++;

            
			if(FunCodeUnion.code.CM_ECATHost==2)
			{
	            STR_ProbeVar.Probe1Status.bit.TrigSource = ContinousCount3&0x01;
	            STR_ProbeVar.Probe1Status.bit.TrigStatus = (ContinousCount3&0x02)>>1;
				ContinousCount3++;
				if(ContinousCount3>=4)ContinousCount3 = 0;
			}   
		}
		else
		{
		   STR_ProbeVar.Probe1Ctrl.bit.DownEdgeEnable = 0;
		}
		
	}
    else//单次存储模式
    {
		if(FunCodeUnion.code.CM_ECATHost==2)
		{
            STR_ProbeVar.Probe1Status.bit.TrigSource = 0;
            STR_ProbeVar.Probe1Status.bit.TrigStatus = 0;
			ContinousCount3 = 0;
		}   

		if(STR_ProbeVar.Probe1Ctrl.bit.RiseEdgeEnable==1)
		{
            if(STR_ProbeVar.Probe1Status.bit.RiseEdgeStored ==0)
            {
				STR_ProbeVar.Pbobe1RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
                STR_ProbeVar.Probe1Status.bit.RiseEdgeStored = 1;
            }

		}
		else
		{
			STR_ProbeVar.Probe1Status.bit.RiseEdgeStored  = 0;
		}

		if(STR_ProbeVar.Probe1Ctrl.bit.DownEdgeEnable==1)
		{
            if(STR_ProbeVar.Probe1Status.bit.DownEdgeStored ==0)
            {
                STR_ProbeVar.Pbobe1DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
                STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 1;
            }
		
		}
		else
		{
			STR_ProbeVar.Probe1Status.bit.DownEdgeStored = 0;
		}

        STR_ProbeVar.TP1PosCnt =0;
        STR_ProbeVar.TP1NegCnt =0;
		
		
    }
}

void TouchProbe2ZeroLatch(int64 LatchPos)
{
    static Uint8 ContinousCount4 = 0;

    
    if(STR_ProbeVar.Probe2Ctrl.bit.TrigMode == 1)//连续存储模式
    {
		if(STR_ProbeVar.Probe2Ctrl.bit.RiseEdgeEnable==1)
		{
            STR_ProbeVar.Pbobe2RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
            STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 1;
            STR_ProbeVar.TP2PosCnt++;
            
			if(FunCodeUnion.code.CM_ECATHost==2)
			{
	            STR_ProbeVar.Probe2Status.bit.TrigSource = ContinousCount4&0x01;
	            STR_ProbeVar.Probe2Status.bit.TrigStatus = (ContinousCount4&0x02)>>1;
				ContinousCount4++;
				if(ContinousCount4>=4)ContinousCount4 = 0;
			}   
		
		}
		else
		{
			STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 0;
		}

		if(STR_ProbeVar.Probe2Ctrl.bit.DownEdgeEnable==1)
		{
			
			STR_ProbeVar.Pbobe2DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
            STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 1;
            STR_ProbeVar.TP2NegCnt++;

			
			if(FunCodeUnion.code.CM_ECATHost==2)
			{
	            STR_ProbeVar.Probe2Status.bit.TrigSource = ContinousCount4&0x01;
	            STR_ProbeVar.Probe2Status.bit.TrigStatus = (ContinousCount4&0x02)>>1;
				ContinousCount4++;
				if(ContinousCount4>=4)ContinousCount4 = 0;
			}   
		}
		else
		{
			 STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 0;
		}													   
		
    }
    else//单次存储模式
    {
		if(FunCodeUnion.code.CM_ECATHost==2)
		{
            STR_ProbeVar.Probe2Status.bit.TrigSource = 0;
            STR_ProbeVar.Probe2Status.bit.TrigStatus = 0;
			ContinousCount4 = 0;
		}   

		if(STR_ProbeVar.Probe2Ctrl.bit.RiseEdgeEnable==1)
		{
            if(STR_ProbeVar.Probe2Status.bit.RiseEdgeStored ==0)
            {
				STR_ProbeVar.Pbobe2RiseEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
                STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 1;
            }
		}
		else
		{
			STR_ProbeVar.Probe2Status.bit.RiseEdgeStored = 0;
		}

		if(STR_ProbeVar.Probe2Ctrl.bit.DownEdgeEnable==1)
		{
            STR_ProbeVar.Probe2Status.bit.TrigStatus = 0;
			if(STR_ProbeVar.Probe2Status.bit.DownEdgeStored ==0)
            {
                STR_ProbeVar.Pbobe2DownEdgeValue = IncpUnit2UserPosUnit(&STR_Pos_Factor_Inverse,LatchPos);//指令单位
                STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 1;
            }		
		}
		else
		{
			STR_ProbeVar.Probe2Status.bit.DownEdgeStored = 0;
		}
        STR_ProbeVar.TP2PosCnt =0;
        STR_ProbeVar.TP2NegCnt =0;

    }
}


/*******************************************************************************
  函数名: void TouchProbeZeroIndexISR(void) 
  输  入:           
  输  出:   
  子函数:                                       
  描  述: z信号中断服务程序
********************************************************************************/
void TouchProbeZeroIndexISR(void)
{
	
    if(FunCodeUnion.code.MT_MotorModel != 14000)return;

    if((STR_ProbeVar.Probe1Ctrl.bit.Enable == 1)&&(STR_ProbeVar.Probe1Ctrl.bit.TrigSource == 1))
    {
        TouchProbe1ZeroLatch(STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt);
    }
    
    if((STR_ProbeVar.Probe2Ctrl.bit.Enable == 1)&&(STR_ProbeVar.Probe2Ctrl.bit.TrigSource == 1))
    {
        TouchProbe2ZeroLatch(STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt);
    }
    
}
/********************************* END OF FILE *********************************/
