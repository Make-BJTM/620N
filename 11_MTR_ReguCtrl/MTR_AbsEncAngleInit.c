/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    MTR_AbsEncAngleInit.c  
 创建人：   姚虹                   创建日期：2012.04.02
 修改人：   
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
#include "PUB_Main.h"
#include "MTR_AbsEncAngleInit.h"
#include "MTR_FPGAInterface.h"
#include "MTR_GlobalVariable.h"
#include "MTR_InterfaceProcess.h"
#include "MTR_Nikon_AbsRomOper.h"
#include "MTR_Tamagawa_AbsRomOper.h"
#include "MTR_AbsRomOper.h"
#include "MTR_HDH_RomOper.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */


/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  


/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */  
extern volatile Uint16 *HostSysCtrl;        //相电流平衡校正和绝对式初始复位操作 
extern volatile Uint16 *HostPosRst;         //位置偏差计数器清零
extern volatile Uint16 *HostAng;            //转子初始电角度


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void SerialCommEncAngleInit(void);         //H0D03绝对式编码器电角度初始化,在转矩中断函数调用



/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 



/*******************************************************************************
  函数名:  void SerialCommEncAngleInit(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
********************************************************************************/
void SerialCommEncAngleInit(void)
{
    static Uint16 ThetaInitStep = 0;    //绝对编码器复位操作步骤
    static Uint16 TimeDelay = 0;        //延时计数器
    static Uint32 WaitDelay = 0;        //等待计数器
    static int32 CntTheta_180 = 0;      //180度电角度对应的脉冲数，作为辨识时判定UVW相序的阈值
    static Uint16 CurAng = 0;
    static int32  UVWPos1 = 0;         //用于判别UVW是否接线错误的位置信息
    static int32  UVWPos2 = 0;
    static Uint16 UVWErrFlag = 0;      //UVW接线反标志位
    
    Uint32 Temp = 0;
    Uint32 Temp1 = 0;

    
    switch(ThetaInitStep)
    {
        case 0:  //置位置偏差计数器

			if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RDY)   return; //等待初始化配置完成
			 
		    *HostPosRst   = 1;  	 //位置偏差计数器清零

            //计算UVW相序接反判定阈值
            CntTheta_180 = STR_MTR_Gvar.FPGA.EncRev / (FunCodeUnion.code.MT_PolePair*2); 

		    ThetaInitStep = 2;	   //下一步
            break;

        case 2: //置控制寄存器初始值  
            *HostSysCtrl  = (0x02 | STR_MTR_Gvar.FPGA.SysCtrl);            //控制寄存器初始化
            STR_MTR_Gvar.GetIqRef.IdRef = 0;
            STR_MTR_Gvar.GetIqRef.IqRef = 0;
            ThetaInitStep = 3;	   //下一步 
            break;

        case 3:    //内部开启伺服使能
            STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncAngInitEn = 1;      //提示开始绝对式编码器角度辨识

            if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus == RUN)    //内部使能打开时进入下一步
            {
		        ThetaInitStep = 4;	   //下一步	    
            }

            break;
        //等待6S的时间确定已定位完成再进入参数恢复阶段,先给12度，
        //再回到0度，避免电角度180度时，电机不动
        case 4: 
             //逐步加大转矩，避免冲击     
            if(((WaitDelay&0xf) == 0) && (STR_MTR_Gvar.GetIqRef.IdRef < STR_MTR_Gvar.GetIqRef.IqRate_MT))
            {
                STR_MTR_Gvar.GetIqRef.IdRef += 2;
            }

            if(WaitDelay < (3L * (Uint32)STR_MTR_Gvar.System.ToqFreq ))  //定位到12度，角度给小避免电机往返动作太大
            {
                WaitDelay++;
                *HostAng = STR_MTR_Gvar.FPGA.AngToCnt / 6;
                CurAng   = STR_MTR_Gvar.FPGA.AngToCnt / 6;                                
            }
            else if(WaitDelay < (6L * (Uint32)STR_MTR_Gvar.System.ToqFreq ))  //定位到30度，角度给小避免电机往返动作太大
            {
                WaitDelay++;
//                *HostAng = STR_MTR_Gvar.FPGA.AngToCnt / 12;
//                CurAng   = STR_MTR_Gvar.FPGA.AngToCnt / 12;  
                if((CurAng > (STR_MTR_Gvar.FPGA.AngToCnt / 12)) && ((WaitDelay&0x3) == 0))
                {
                    CurAng--;
                }

                *HostAng = CurAng;
                             
            }
            else if(WaitDelay == (6L * (Uint32)STR_MTR_Gvar.System.ToqFreq ))
            {
                WaitDelay++;
                UVWPos1 = STR_MTR_Gvar.AbsRom.SingleAbsPosFdb;         //获取初次定位的位置
            }
            else if(WaitDelay < (9L * (Uint32)STR_MTR_Gvar.System.ToqFreq))  //重新定位到0度
            {
                WaitDelay++;

                if((CurAng > 0) && ((WaitDelay&0x3) == 0))
                {
                    CurAng--;
                }

                *HostAng = CurAng;
            }
            else if(WaitDelay == (9L * (Uint32)STR_MTR_Gvar.System.ToqFreq ))
            {
                WaitDelay++;
                UVWPos2 = STR_MTR_Gvar.AbsRom.SingleAbsPosFdb;        //获取第二个角度的位置
            }
            else
            {   //判断UVW错误时，需要避免位置值刚好处于2^20的切换点
                if(FunCodeUnion.code.ER_UVWIdenEn == 1)         
                {
                    if( (0 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)
                     && ((((UVWPos2 - UVWPos1) > 0) && ((UVWPos2 - UVWPos1) < CntTheta_180)) 
                      || ((UVWPos1 - UVWPos2) > CntTheta_180)) )    
                    {
                        UVWErrFlag = 1;    //置UVW接线反标志位，最后一步报警                   
                    }

                    if( (1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir)
                     && ((((UVWPos1 - UVWPos2) > 0) && ((UVWPos1 - UVWPos2) < CntTheta_180)) 
                      || ((UVWPos2 - UVWPos1) > CntTheta_180)))    
                    {
                        UVWErrFlag = 1;    //置UVW接线反标志位，最后一步报警                   
                    }                    
                }

                if(0 == UVWErrFlag)
                {
                    //得到要写入的绝对式编码器初始角偏移
                    Temp = 65536 / FunCodeUnion.code.MT_PolePair;
                    
                    if(0x800000 == STR_MTR_Gvar.FPGA.EncRev) Temp1 = (STR_MTR_Gvar.AbsRom.SingleAbsPosFdb >> 7) & 0xffff;
                    else Temp1 = (STR_MTR_Gvar.AbsRom.SingleAbsPosFdb >> 4) & 0xffff;

                    if(1 == STR_MTR_Gvar.GlobalFlag.bit.RevlDir) Temp1 = 65535 - Temp1;
                    Temp1 = Temp1 % Temp;

				    FunCodeUnion.code.MT_ThetaOffsetL = Temp1;
                    FunCodeUnion.code.MT_ThetaOffsetH = 0;
                                        
                    Temp1 = (Temp1 * 3600) + (Temp >> 1); 
                    FunCodeUnion.code.MT_InitTheta = (Uint16)((Uint32)Temp1 / Temp); 
                    FunCodeUnion.code.MT_UposedgeTheta = FunCodeUnion.code.MT_InitTheta;                     
                }
                
                ThetaInitStep = 5;	   //下一步
                WaitDelay = 0;
                CurAng    = 0;
                UVWPos2   = 0;
                UVWPos1   = 0;
            }
            break;

        case 5:             //须先停掉使能再进行读写操作，否则报错
            if(STR_MTR_Gvar.GetIqRef.IdRef > 0)
            {
                STR_MTR_Gvar.GetIqRef.IdRef -= 2;   //D轴电流还原
            }
            else
            {
                STR_MTR_Gvar.GetIqRef.IdRef = 0;   //D轴电流还原
            
                STR_MTR_Gvar.MTRtoFUNCFlag.bit.AbsEncAngInitEn = 0;   //提示绝对式编码器角度辨识结束
           
                if(UNI_MTR_FUNCToMTR_List_16kHz.List.StatusFlag.ServoRunStatus != RUN)	//延时确认使能停掉后进入下一步
                {                    
                    WaitDelay = 0;

                    if(UVWErrFlag == 0)
                    {
                        //保存辨识出的电角度
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_ThetaOffsetL)); 	
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_ThetaOffsetH));         
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_InitTheta)); 		    
        		        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_UposedgeTheta));  
    
                        //初始化初始角计数值，角度辨识后可以速度JOG运行
                        *HostAng = FunCodeUnion.code.MT_ThetaOffsetL;

                        ThetaInitStep = 6;
                    }
                    else
                    {
                        ThetaInitStep = 8;
                    }
                }
            }
            break;

        case 6:             
             
            if(FunCodeUnion.code.MT_EncoderSel == 0x13)    //汇川绝对式编码器
            {
            #if HC_ENC_SW
                Temp = HCAbsEnc_SaveThetaOffset(); 
            #else
                Temp = 1;
            #endif 
            } 
        #if NOKIN_ENC_SW
            else if(FunCodeUnion.code.MT_EncoderSel == 0x12)         //尼康绝对式编码器
            {
                if(0 == FunCodeUnion.code.DisRdEncEepromPwrOn) Temp = NKAbsEnc_SaveThetaOffset(); 
                else Temp = 1;
            }
        #endif 
        #if TAMAGAWA_ENC_SW
            else if(FunCodeUnion.code.MT_EncoderSel == 0x10)         //绝对式编码器
            {
                if(0 == FunCodeUnion.code.DisRdEncEepromPwrOn) Temp = TAMAGAbsEnc_SaveThetaOffset();
                else Temp = 1; 
            }
        #endif
        #if HDH_ENCODER_SW
            else if(FunCodeUnion.code.MT_EncoderSel == 0x11)        
            {
                Temp = HDHAbsEnc_SaveThetaOffset(); 
            }
        #endif 
            else
            {
                Temp = 1;
            }

            if(Temp == 1)
            {
                ThetaInitStep = 7;    //正确
            }
            else if(Temp == 2)
            {
                ThetaInitStep = 8;    //故障
            }
            break;

        case 7:         //初始角辨识正确或失败时都参数还原，通过H0d03来区分
        case 8:
            *HostSysCtrl = STR_MTR_Gvar.FPGA.SysCtrl;    //控制寄存器恢复
            *HostPosRst = 0;         //位置偏差清除命令寄存器还原
            TimeDelay = 0;

            if(ThetaInitStep == 7)
            {                  
                AuxFunCodeUnion.code.OEM_ResetABSTheta = 0;
                PostErrMsg(PCHGDWARN);		 //需要重新上电才能生效
            }
            else if(UVWErrFlag == 1)    //UVW接线错误，需要任意调换两相
            {
                AuxFunCodeUnion.code.OEM_ResetABSTheta = 2;
                PostErrMsg(UVWLINKERR);      
            }            
            else if(ThetaInitStep == 8)   //辨识失败
            {
                AuxFunCodeUnion.code.OEM_ResetABSTheta = 2;
                PostErrMsg(ENCDERR_Z6);
            }            

            UVWErrFlag    = 0;    //故障标志复位
            ThetaInitStep = 0;
            break;

        default:
            break;
    }

    if(TimeDelay > 8000)            //延迟时间过长或者发生了复位故障
    {
        PostErrMsg(ENCDERR_Z6);   //延时过长警告 Er.740
        ThetaInitStep = 8;       //故障时参数还原
    }
}


/********************************* END OF FILE *********************************/
