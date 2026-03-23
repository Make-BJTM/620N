/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_PosFbCalc.c
 创建人：                
 修改人：              创建日期：
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
#include "FUNC_InterfaceProcess.h"
#include "FUNC_PosCtrl.h" 
#include "FUNC_FunCode.h" 
#include "FUNC_ManageFunCode.h"
#include "FUNC_XIntPosition.h"
#include "ECT_Probe.h"
#include "FUNC_FullCloseLoop.h"
#include "ECT_PP.h"
#include "CANopen_Home.h"
/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */



/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
static Uint8 PosFdbClrFlg = 0;     //第一次上电时清位置反馈增量值
static Uint64 AbsMod2PosUpLmt;   //多圈模式2位置上限值

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void PosFbCalc_ToqInt(void);
void PowerOffSave_MainLoop(void); //多圈绝对位置模式更新
void PosFbCalc_PosSched(void);
/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Static_Inline void PosFbCalc_AbsMod1(void);
Static_Inline void PosFbCalc_AbsMod2(void);
Static_Inline void PosFbCalc_IncMod(void); 
Static_Inline void PosFbCalc_ToqSched(void);


/*******************************************************************************
  函数名: 
  输入:  
  输出:	       
  子函数:         
  描述:  
********************************************************************************/ 
void PowerOffSave_MainLoop(void)
{
    //不使能掉电保存功能时 复位掉电保存的功能码
    if(1 != FunCodeUnion.code.ER_PowerOffSaveToEeprom)
    {
        if(0 != FunCodeUnion.code.OS_CurrentPos_L) 
        {
            FunCodeUnion.code.OS_CurrentPos_L = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_CurrentPos_L));
        }
        if(0 != FunCodeUnion.code.OS_CurrentPos_H) 
        {
            FunCodeUnion.code.OS_CurrentPos_H = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_CurrentPos_H));
        }    
    }

    if(2 != UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
    {   
        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffL) 
        {
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffL = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffL));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH) 
        {   
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH32L) 
        {
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH32L = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH32L));
        }

        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222L) 
        {
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222L = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222L));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H) 
        {   
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H32L) 
        {
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H32L = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H32L));
        }
        
        if(0 != FunCodeUnion.code.OS_AbsMod2EncPos_PowOffL) 
        {
            FunCodeUnion.code.OS_AbsMod2EncPos_PowOffL = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2EncPos_PowOffL));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH) 
        {
            FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH));
        }
        if(0 != FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH32L) 
        {
            FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH32L = 0;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH32L));
        }
    } 
}




/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
Static_Inline void PosFbCalc_AbsMod1(void)
{    
    int64 Temp64_1 = 0; 
    int64 Temp64_2 = 0;
            
    if(PosFdbClrFlg == 0)  //上电第一次清除位置反馈增量值
    {       
        // 绝对坐标记忆
        if(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.AbsEncCommInit == 1)
        {
            PosFdbClrFlg = 2;
            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.FpgaPosFdbInit;                
            
            //处理比较重要***********

            Temp64_1 = (int64)(int32)A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_MultiAbsPosOffsetH, FunCodeUnion.code.PL_MultiAbsPosOffsetL);
            Temp64_1 = Temp64_1 & 0xFFFFFFFF; 
            Temp64_2 = (int64)(int32)A_SHIFT16_PLUS_B(FunCodeUnion.code.PL_MultiAbsPosOffsetH32H, FunCodeUnion.code.PL_MultiAbsPosOffsetH32L);
            Temp64_1 = (Uint64)((Uint64)Temp64_2 << 32) | (Uint64)Temp64_1;

            Temp64_2 = (int64)( ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH << 32)
                              + ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL & 0xFFFFFFFF));                       

            STR_InnerGvarPosCtrl.CurrentAbsPos = (int64)Temp64_2 - (int64)Temp64_1;                                     
        }
        else
        {
            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
            STR_InnerGvarPosCtrl.CurrentAbsPos = 0; 
        }             	                
    }

    //增量式位置反馈值，全局变量
    STR_FUNC_Gvar.PosCtrl.PosFdb = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue - STR_InnerGvarPosCtrl.PosFdbAbsValueLast;
    STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;   //锁存当前绝对位置反馈值，以便下次计算增量式位置反馈用
            
    STR_InnerGvarPosCtrl.CurrentAbsPos += STR_FUNC_Gvar.PosCtrl.PosFdb;   //反馈脉冲累加计数器,1个脉冲单位   

    //编码器位置
    AuxFunCodeUnion.code.DP_EncPosH32H = (Uint16)(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH >> 16);
    AuxFunCodeUnion.code.DP_EncPosH32L = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH;    
    AuxFunCodeUnion.code.DP_EncPosH = (Uint16)((Uint64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL >> 16);      
    AuxFunCodeUnion.code.DP_EncPosL = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL;  

    if(1 == STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset)
    {
        //延时一个周期确保多圈数据更新
        STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset = 2;  
    }
    else if(2 == STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset)
    {
        Temp64_1 = (int64)( ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH << 32)
                          + ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL & 0xFFFFFFFF));                         
        Temp64_1 = (int64)Temp64_1 - (int64)STR_InnerGvarPosCtrl.CurrentAbsPos;
    
        STR_InnerGvarPosCtrl.MutexBit.bit.AbsMod1CalcPosOffset = 0; 
                  
        FunCodeUnion.code.PL_MultiAbsPosOffsetL = (Uint16)Temp64_1; 
        FunCodeUnion.code.PL_MultiAbsPosOffsetH = (Uint16)(Temp64_1 >> 16);
        FunCodeUnion.code.PL_MultiAbsPosOffsetH32L = (Uint16)(Temp64_1 >> 32);  
        FunCodeUnion.code.PL_MultiAbsPosOffsetH32H = (Uint16)(Temp64_1 >> 48);                        
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetL));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH32H));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.PL_MultiAbsPosOffsetH32L));
    }
}

/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
void AbsMod2CalcPosUpLmt(void)
{
    int64 Temp64_1 = 0; 
    int64 Temp64_2 = 0;

    if(2 != UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection) return;

    if( (0 == FunCodeUnion.code.PL_MechAbsPosLimUpL)
     && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH)
     && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH32L)
     && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH32H)  )
    {
        //初始化多圈绝对位置模式2 相关变量             
        Temp64_1 = UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
        if(0 == FunCodeUnion.code.PL_AbsMode2MechGearNum) FunCodeUnion.code.PL_AbsMode2MechGearNum = 1;
        if(0 == FunCodeUnion.code.PL_AbsMode2MechGearDen) FunCodeUnion.code.PL_AbsMode2MechGearDen = 1;     

        //乘以机械齿轮比 换算成编码器单位
        Temp64_2 = (Uint64)FunCodeUnion.code.PL_AbsMode2MechGearNum;
        Temp64_2 =  (Uint64)Temp64_1 * (Uint64)Temp64_2;  
        Temp64_1 = (Uint64)FunCodeUnion.code.PL_AbsMode2MechGearDen;
        Temp64_1 = (Uint64)Temp64_2 / (Uint64)Temp64_1;
    }
    else
    {
        Temp64_1 = FunCodeUnion.code.PL_MechAbsPosLimUpH32H;   
        Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpH32L;      
        Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpH;   
        Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpL;         
    }
    
    if(STR_InnerGvarPosCtrl.AbsMod2PosUpLmt != (Uint64)Temp64_1)
    {
        STR_InnerGvarPosCtrl.AbsMod2PosUpLmt = (Uint64)Temp64_1; 
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0; 
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0; 
    } 
}

/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
Static_Inline void PosFbCalc_AbsMod2(void)
{
    int64 Temp64_1 = 0; 
    int64 Temp64_2 = 0;
    int64 CurrentAbsPosTemp = 0;
    int64 CurrentAbsPosTemp222 = 0;
    
    Uint64 AbsMod2MechSAbsPos_PowOff;    //多圈模式2掉电时旋转负载单圈位置    掉电保存
    Uint64 AbsMod2EncPos_PowOff;       //多圈模式2掉电时编码器位置  范围0-编码器总分辨率  掉电保存
    int64  AbsMod2MechSAbsPos_PowOff222;       //多圈模式2掉电时编码器位置  范围-编码器总分辨率-编码器总分辨率  掉电保存
    Uint8 MultTurnBits = 16;
            
    if(PosFdbClrFlg == 0)  //上电第一次清除位置反馈增量值
    {
        PosFdbClrFlg = 1;

        STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
        STR_InnerGvarPosCtrl.CurrentAbsPos = 0; 
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0; 

        if( (0 == FunCodeUnion.code.PL_MechAbsPosLimUpL)
         && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH)
         && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH32L)
         && (0 == FunCodeUnion.code.PL_MechAbsPosLimUpH32H)  )
        {
            //初始化多圈绝对位置模式2 相关变量             
            Temp64_1 = UNI_FUNC_MTRToFUNC_InitList.List.EncRev; 
            if(0 == FunCodeUnion.code.PL_AbsMode2MechGearNum) FunCodeUnion.code.PL_AbsMode2MechGearNum = 1;
            if(0 == FunCodeUnion.code.PL_AbsMode2MechGearDen) FunCodeUnion.code.PL_AbsMode2MechGearDen = 1;
            if((0x11 == FunCodeUnion.code.MT_EncoderSel) && (FunCodeUnion.code.PL_AbsMode2MechGearNum > 4095))
            {
                FunCodeUnion.code.PL_AbsMode2MechGearNum = 4095;        
            } 
    
            //乘以机械齿轮比 换算成编码器单位
            Temp64_2 = (Uint64)(Uint32)(Uint16)FunCodeUnion.code.PL_AbsMode2MechGearNum;
            Temp64_2 =  (Uint64)Temp64_1 * (Uint64)Temp64_2;  
            Temp64_1 = (Uint64)(Uint32)(Uint16)FunCodeUnion.code.PL_AbsMode2MechGearDen;
            Temp64_2 = (Uint64)Temp64_2 / (Uint64)Temp64_1;
            
            AbsMod2PosUpLmt = Temp64_2;       //无符号 
        }
        else
        {
            Temp64_1 = FunCodeUnion.code.PL_MechAbsPosLimUpH32H;   
            Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpH32L;      
            Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpH;   
            Temp64_1 = ((Uint64)Temp64_1 << 16) + FunCodeUnion.code.PL_MechAbsPosLimUpL;

            AbsMod2PosUpLmt = Temp64_1;      
        } 

        STR_InnerGvarPosCtrl.AbsMod2PosUpLmt = AbsMod2PosUpLmt;   
                                	                
    }
    else if (PosFdbClrFlg == 1)
    {    
        if(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.AbsEncCommInit == 1)
        {
            PosFdbClrFlg = 2;

            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.FpgaPosFdbInit;

            CurrentAbsPosTemp = (int64)( ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH << 32)
                                       + ((Uint64)(Uint32)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL & 0xFFFFFFFF));
            
            STR_InnerGvarPosCtrl.CurrentAbsPos = CurrentAbsPosTemp;

            //去掉符号位
            if(0x13 == FunCodeUnion.code.MT_EncoderSel)                                                                
            {
                //汇川编码器多圈16位 单圈20位
                CurrentAbsPosTemp = CurrentAbsPosTemp & 0x0000007FFFFFFFFF;
                MultTurnBits = 16;
            }
            else if(0x11 == FunCodeUnion.code.MT_EncoderSel)    
            {
                //海德汉编码器多圈是12位的 单圈20位
                CurrentAbsPosTemp = CurrentAbsPosTemp & 0x00000000FFFFFFFF; 
                MultTurnBits = 12;               
            }
            else
            {
                //尼康和多摩川编码器多圈多圈16位 单圈20位
                CurrentAbsPosTemp = CurrentAbsPosTemp & 0x0000000FFFFFFFFF;
                MultTurnBits = 16;
            }
             
            
            //多圈模式2掉电时旋转负载单圈位置
            Temp64_1 = FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH32L;   
            Temp64_1 = ((int64)Temp64_1 << 16) + FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH;      
            Temp64_1 = ((int64)Temp64_1 << 16) + FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffL;     
            AbsMod2MechSAbsPos_PowOff = Temp64_1;     //无符号


            //多圈模式2掉电时旋转负载单圈位置2
            Temp64_1 = (int64)(int32)A_SHIFT16_PLUS_B(FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H, FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222L);
            Temp64_1 = Temp64_1 & 0xFFFFFFFF; 
            Temp64_2 = (int64)(int32)(int16)FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H32L;
            Temp64_1 = (Uint64)((Uint64)Temp64_2 << 32) | (Uint64)Temp64_1;
            
            AbsMod2MechSAbsPos_PowOff222 = (int64)Temp64_1;    //有符号

            //去掉符号位
            if(0x13 == FunCodeUnion.code.MT_EncoderSel)                                                                
            {
                //汇川编码器多圈16位 单圈20位
                AbsMod2MechSAbsPos_PowOff = AbsMod2MechSAbsPos_PowOff & 0x0000007FFFFFFFFF;
            }
            else if(0x11 == FunCodeUnion.code.MT_EncoderSel)    
            {
                //海德汉编码器多圈是12位的 单圈20位
                AbsMod2MechSAbsPos_PowOff = AbsMod2MechSAbsPos_PowOff & 0x00000000FFFFFFFF;                
            }
            else
            {
                //尼康和多摩川编码器多圈多圈16位 单圈20位
                AbsMod2MechSAbsPos_PowOff = AbsMod2MechSAbsPos_PowOff & 0x0000000FFFFFFFFF;
            }
            
            //多圈模式2掉电时编码器位置
            Temp64_1 = FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH32L;  
            Temp64_1 = ((int64)Temp64_1 << 16) + FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH;     
            Temp64_1 = ((int64)Temp64_1 << 16) + FunCodeUnion.code.OS_AbsMod2EncPos_PowOffL;   
            AbsMod2EncPos_PowOff = Temp64_1;    //无符号            

            //去掉符号位
            if(0x13 == FunCodeUnion.code.MT_EncoderSel)                                                                
            {
                //汇川编码器多圈16位 单圈20位
                AbsMod2EncPos_PowOff = AbsMod2EncPos_PowOff & 0x0000007FFFFFFFFF;
            }
            else if(0x11 == FunCodeUnion.code.MT_EncoderSel)    
            {
                //海德汉编码器多圈是12位的 单圈20位
                AbsMod2EncPos_PowOff = AbsMod2EncPos_PowOff & 0x00000000FFFFFFFF;                
            }
            else
            {
                //尼康和多摩川编码器多圈多圈16位 单圈20位
                AbsMod2EncPos_PowOff = AbsMod2EncPos_PowOff & 0x0000000FFFFFFFFF;
            }
            if(CurrentAbsPosTemp >= AbsMod2EncPos_PowOff)
            {
                //求差值
                Temp64_1 = (Uint64)CurrentAbsPosTemp - (Uint64)AbsMod2EncPos_PowOff;
                //计算掉电期间旋转圈数
                Temp64_2 = (Uint64)Temp64_1 / (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;

                if(Temp64_2 < (1L << (MultTurnBits - 1)))
                {
                    //正转
                    CurrentAbsPosTemp = AbsMod2MechSAbsPos_PowOff + Temp64_1;
                    CurrentAbsPosTemp = CurrentAbsPosTemp % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;

                    
                    CurrentAbsPosTemp222 = AbsMod2MechSAbsPos_PowOff222 + Temp64_1;
                    if(CurrentAbsPosTemp222<0)//旋转圈数小于1圈，依然为负数
                    {
                    }
                    else//即使掉电前为负，正转旋转位置也到了正值
                    {
                        CurrentAbsPosTemp222 = CurrentAbsPosTemp222 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                    }
                }
                else
                {
                    //反转越界
                    Temp64_2 = (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
                    Temp64_2 = (Uint64)Temp64_2 << MultTurnBits;     
                    Temp64_2 = Temp64_2 - Temp64_1;

                    CurrentAbsPosTemp222 = AbsMod2MechSAbsPos_PowOff222 - (int64)Temp64_1;
                    
                    Temp64_2 = Temp64_2 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;

                    CurrentAbsPosTemp = AbsMod2MechSAbsPos_PowOff - Temp64_2;
                    CurrentAbsPosTemp += STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;    //防止为负数 补偿一下
                    CurrentAbsPosTemp = CurrentAbsPosTemp % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt; 
                }
            }
            else
            {
                //求差值
                Temp64_1 = (Uint64)AbsMod2EncPos_PowOff - (Uint64)CurrentAbsPosTemp;
                //计算掉电期间旋转圈数
                Temp64_2 = (Uint64)Temp64_1 / (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;

                if(Temp64_2 < (1L << (MultTurnBits - 1)))
                {
                    //反转
                    Temp64_1 = Temp64_1 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                    CurrentAbsPosTemp = AbsMod2MechSAbsPos_PowOff - Temp64_1;
                    CurrentAbsPosTemp += STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;   //防止为负数 补偿一下
                    CurrentAbsPosTemp = CurrentAbsPosTemp % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt; 


                    CurrentAbsPosTemp222 = AbsMod2MechSAbsPos_PowOff222 - Temp64_1;
                    if(CurrentAbsPosTemp222<0)//旋转圈数大于1圈
                    {
                        CurrentAbsPosTemp222 = 0 - CurrentAbsPosTemp222;
                        CurrentAbsPosTemp222 = CurrentAbsPosTemp222 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                        CurrentAbsPosTemp222 = 0 - CurrentAbsPosTemp222;
                    }
                    else//依然为正数
                    {
                    }
                }
                else
                {
                    //正转越界
                    Temp64_2 = (Uint64)UNI_FUNC_MTRToFUNC_InitList.List.EncRev;
                    Temp64_2 = (Uint64)Temp64_2 << MultTurnBits;     
                    Temp64_2 = Temp64_2 - Temp64_1;
                    
                    CurrentAbsPosTemp222 = AbsMod2MechSAbsPos_PowOff222 + (int64)Temp64_1;
                    
                    Temp64_2 = Temp64_2 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;

                    CurrentAbsPosTemp = AbsMod2MechSAbsPos_PowOff + Temp64_2;
                    CurrentAbsPosTemp = CurrentAbsPosTemp % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt; 
                }            
            }

            
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = CurrentAbsPosTemp;
            
            if((int64)CurrentAbsPosTemp222 < 0)
            {
                if(((int64)CurrentAbsPosTemp222 + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)<=0)
                {
                    CurrentAbsPosTemp222 = 0 - CurrentAbsPosTemp222;
                    CurrentAbsPosTemp222 = CurrentAbsPosTemp222 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                    CurrentAbsPosTemp222 = 0 - CurrentAbsPosTemp222;
                }

                if(((int64)CurrentAbsPosTemp222 + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt) <= 100)
                {
                    CurrentAbsPosTemp222 = -(CurrentAbsPosTemp222 + STR_InnerGvarPosCtrl.AbsMod2PosUpLmt);
                }
            }
            else 
            {
                if((int64)CurrentAbsPosTemp222 >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
                {
                    CurrentAbsPosTemp222 = CurrentAbsPosTemp222 % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                }

                if(((int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - (int64)CurrentAbsPosTemp222) <= 100)
                {
                    CurrentAbsPosTemp222 = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - CurrentAbsPosTemp222;
                }
            }
            
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = CurrentAbsPosTemp222;
        } 
        else
        {
            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
            STR_InnerGvarPosCtrl.CurrentAbsPos = 0;  
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0;        
        }   
    }

    //增量式位置反馈值，全局变量
    STR_FUNC_Gvar.PosCtrl.PosFdb = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue - STR_InnerGvarPosCtrl.PosFdbAbsValueLast;
    STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;   //锁存当前绝对位置反馈值，以便下次计算增量式位置反馈用
            
    STR_InnerGvarPosCtrl.CurrentAbsPos += STR_FUNC_Gvar.PosCtrl.PosFdb;   //反馈脉冲累加计数器,1个脉冲单位
    STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos += STR_FUNC_Gvar.PosCtrl.PosFdb;         

    if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos < 0)
    {
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2PosUpLmt - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
    }

    if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
    {
        STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
    }

    if(PosFdbClrFlg == 2)
    {
        //多圈模式2掉电时旋转负载单圈位置
        FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH32L = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >> 32); 
        FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffH = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos >> 16); 
        FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOffL = (Uint16)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos;
           
        //多圈模式2编码器掉电时编码器位置
        FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH32L = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH;    
        FunCodeUnion.code.OS_AbsMod2EncPos_PowOffH = (Uint16)((Uint64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL >> 16);     
        FunCodeUnion.code.OS_AbsMod2EncPos_PowOffL = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL;

        //编码器位置
        AuxFunCodeUnion.code.DP_EncPosH32H = (Uint16)(UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH >> 16); 
        AuxFunCodeUnion.code.DP_EncPosH32L = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbH;    
        AuxFunCodeUnion.code.DP_EncPosH = (Uint16)((Uint64)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL >> 16);      
        AuxFunCodeUnion.code.DP_EncPosL = (Uint16)UNI_FUNC_MTRToFUNC_FastList_16kHz.List.MultiAbsPosFdbL;
    }  
}

/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
Static_Inline void PosFbCalc_IncMod(void)
{           
    if(PosFdbClrFlg == 0)  //上电第一次清除位置反馈增量值
    {       
        PosFdbClrFlg = 2;
        if(1 == FunCodeUnion.code.ER_PowerOffSaveToEeprom)
        {            
            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
            STR_InnerGvarPosCtrl.CurrentAbsPos = (int64)((int32)(A_SHIFT16_PLUS_B(FunCodeUnion.code.OS_CurrentPos_H, FunCodeUnion.code.OS_CurrentPos_L)));
        }
        else
        {
            STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;
            STR_InnerGvarPosCtrl.CurrentAbsPos = 0;
        }                	                
    }   

    //增量式位置反馈值，全局变量
    STR_FUNC_Gvar.PosCtrl.PosFdb = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue - STR_InnerGvarPosCtrl.PosFdbAbsValueLast;
    STR_InnerGvarPosCtrl.PosFdbAbsValueLast = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue;   //锁存当前绝对位置反馈值，以便下次计算增量式位置反馈用
            
    STR_InnerGvarPosCtrl.CurrentAbsPos += STR_FUNC_Gvar.PosCtrl.PosFdb;   //反馈脉冲累加计数器,1个脉冲单位 

    if(1 == FunCodeUnion.code.ER_PowerOffSaveToEeprom)
    {
        FunCodeUnion.code.OS_CurrentPos_L = STR_InnerGvarPosCtrl.CurrentAbsPos & 0xFFFF;    //掉电保存参数_停机位置
        FunCodeUnion.code.OS_CurrentPos_H = STR_InnerGvarPosCtrl.CurrentAbsPos >> 16;       //掉电保存参数_停机位置
    } 
}

/*******************************************************************************
  函数名:  PosFbCalc()
  输入:                                      
  输出:  增量式的位置反馈输出脉冲，单位为脉冲  
  子函数:                
  描述:  根据FPGA等返回的当前和上次的位置脉冲反馈值，进行增量式的位置反馈输出值计算 
********************************************************************************/
void PosFbCalc_PosSched(void)
{    
    if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection) PosFbCalc_AbsMod2();
    else if((1 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)||(3 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)) PosFbCalc_AbsMod1();
    else PosFbCalc_IncMod();       
	    
	//by huangxin201711_29 回零的位置清零移到z信号检测
	//by huangxin201711_13 回零结束时在第一个位置环中清零
	if(1 == STR_CanopenHome.HomingStatus.bit.HomingClrFlag)
	{	
		STR_CanopenHome.HomingStatus.bit.HomingClrFlag = 0;		
		HomingClearPos();
	}
	//by huangxin201711_13 回零结束时在第一个位置环中清零

}


/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
Static_Inline void PosFbCalc_ToqSched(void)
{
    static int32 PosFdbAbsValueLastLoc = 0;     //当前位置反馈绝对值
    static int8 PowonDelay = 0;     //延时
    int32 DeltaPosFdb = 0;     //位置反馈增量
    int32 ExDeltaPosFdb = 0;     //外部编码器位置反馈增量
    static int32 ExPosFdbAbsValueLastLoc = 0;     //当前外部编码器位置反馈绝对值

    //位置反馈更新 给上位机反馈使用
    if(PowonDelay < 6)
    {
        STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt = STR_InnerGvarPosCtrl.CurrentAbsPos; 
        STR_InnerGvarPosCtrl.ExCurrentAbsPos_ToqInt = STR_FUNC_Gvar.PosCtrl.ExCurrentAbsPos;
        if((2 == PosFdbClrFlg) && (1 == STR_FUNC_Gvar.ScheldularFlag.bit.PosFlag))  PowonDelay ++;       
    }
    else
    {
        DeltaPosFdb = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue - PosFdbAbsValueLastLoc;
        //STR_InnerGvarPosCtrl.PosFdb_ToqInt = DeltaPosFdb;
        STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt += DeltaPosFdb;
        if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
        {
            STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt += DeltaPosFdb;
        }
            
        ExDeltaPosFdb = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.ExPosFdbAbsValue- ExPosFdbAbsValueLastLoc;
        if(STR_FullCloseLoop.ExCoderDir ==1) ExDeltaPosFdb = -ExDeltaPosFdb;//根据方向调整   
        STR_InnerGvarPosCtrl.ExCurrentAbsPos_ToqInt += ExDeltaPosFdb;
        
    }
        
    if(2 == UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection)
    {
        if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt < 0)
        {
            if(((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt + (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)<=0)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = 0 - STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
            }
        }
        else
        {
            if((int64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt >= (int64)STR_InnerGvarPosCtrl.AbsMod2PosUpLmt)
            {
                STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt = STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt % STR_InnerGvarPosCtrl.AbsMod2PosUpLmt;
            }
        }

        //多圈模式2掉电时旋转负载单圈位置222
        if(6 == PowonDelay)
        {
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H32L = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt >> 32); 
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222H = (Uint16)((Uint64)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt >> 16); 
            FunCodeUnion.code.OS_AbsMod2MechSAbsPos_PowOff222L = (Uint16)STR_InnerGvarPosCtrl.AbsMod2MechSingleAbsPos_ToqInt;
        }
    }
    
    PosFdbAbsValueLastLoc = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.PosFdbAbsValue; 
    ExPosFdbAbsValueLastLoc = UNI_FUNC_MTRToFUNC_FastList_16kHz.List.ExPosFdbAbsValue;
    STR_FUNC_Gvar.OscTarget.CurrentAbsPos = STR_InnerGvarPosCtrl.CurrentAbsPos_ToqInt;                 
#if ((DRIVER_TYPE == SERVO_620N) || (DRIVER_TYPE == SERVO_650N))
    if(FunCodeUnion.code.OEM_LocalModeEn==1)
    {
    //中断定长处理 
        XintPosEnJudgment(DeltaPosFdb);            //获取中断定长是否使能
    }
    else
    {
        TouchProbeFunc(DeltaPosFdb);
    }
#endif
}


/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：    
********************************************************************************/
void PosFbCalc_ToqInt(void)
{    

     PosFbCalc_ToqSched();	//by huangxin201711_14调整电流环的位置反馈计算位置
	if((1 == STR_FUNC_Gvar.ScheldularFlag.bit.PosFlag)&&(STR_EcatPosSync.PosSyncModeConfigDone==0))
    {
        PosFbCalc_PosSched(); //回零的清零在这   		
    }

   //	PosFbCalc_ToqSched(); 
}

/********************************* END OF FILE *********************************/
