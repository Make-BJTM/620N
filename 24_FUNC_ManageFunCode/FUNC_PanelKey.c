/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_PanelKey.c
 创建人:    王治国                创建日期：2011.11.07      
 描述:
    1. 
    2. 
缩写说明：
    1. cnt      ->  counter
    2. fltr     ->  filter
    3. acc      ->  acceleration
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_PanelKey.h"
#include "FUNC_PanelKeyFunArray.h" 


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */  
STR_PANELOPERRATION     ServoPanel = PANELOPERRATION_DEFAULT;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */



/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 

Static_Inline void PanelKeyScan(STR_PANELOPERRATION *p);
Static_Inline void PanelKeyProcess(STR_PANELOPERRATION *p);

Static_Inline Uint8 UpDownKeyScan(STR_PANELOPERRATION *p);

void (*PanelMenuFun[9][7])(STR_PANELOPERRATION *p) = {
KeyModeFunZero, NullFuntion,    NullFuntion,    NullFuntion,    KeyLongShiftFunZero, NullFuntion,   NullFuntion,
KeyModeFun1st,  KeyUpFun1st,    KeyDownFun1st,  KeyShiftFun1st, KeyShiftFun1st,      KeySetFun1st,  KeySetFun1st,
KeyModeFun2nd,  KeyUpFun2nd,    KeyDownFun2nd,  KeyShiftFun2nd, KeyShiftFun2nd,      KeySetFun2nd,  KeySetFun2nd,
KeyModeFun3rd,  KeyUpFun3rd,    KeyDownFun3rd,  KeyShiftFun3rd, KeyLongShiftFun3rd,  KeySetFun3rd,  KeySetFun3rd,
KeyModeFun4th,  KeyUpFun4th,    KeyDownFun4th,  KeyShiftFun4th, KeyShiftFun4th,      KeySetFun4th,  KeySetFun4th,
KeyModeFun5th,  NullFuntion,    NullFuntion,    NullFuntion,    NullFuntion,         NullFuntion,   NullFuntion,
KeyModeFun6th,  NullFuntion,    NullFuntion,    NullFuntion,    NullFuntion,         NullFuntion,   KeyLongSetFun6th,
KeyModeFun7th,  NullFuntion,    NullFuntion,    NullFuntion,    NullFuntion,         NullFuntion,   NullFuntion,
NullFuntion,    NullFuntion,    NullFuntion,    NullFuntion,    NullFuntion,         NullFuntion,   NullFuntion,
};

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 
    2. 
********************************************************************************/
void PanelKey(void)
{
    PanelKeyScan(&ServoPanel);
    PanelKeyProcess(&ServoPanel);
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1. 检测按键的下降沿是否有效
    2. 检测当前按下的键
    3. 下降沿去抖，上升沿不去抖动
********************************************************************************/
Static_Inline void PanelKeyScan(STR_PANELOPERRATION *p)
{
    //读取SPI输入值
    p->KeyScan.Input.all = STR_FUNC_Gvar.SpiDrvOutput.InnerKey & STR_FUNC_Gvar.SpiDrvOutput.OuterKey;

    //初始化未完成,只显示RESET,返回
    if(STR_PUB_Gvar.AllInitDone == 0)   return;

    //功能码复位期间按键无效
    if((STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 1)||(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode == 1)||
        (STR_FUNC_Gvar.ManageFunCodeOutput.ResetOD == 1)) return;
    //面板LED测试时按键无效
    if(1 == AuxFunCodeUnion.code.FA_DisTest) return;

    //电流环PI参数自调谐时按键无效
    if((STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1) && 
       (p->Menu.CtrlReg.bit.Class != CLASS_7TH)) return;

    //JOG结束时,第二级菜单显示SAVE特殊处理,按键无效
    if (p->Menu.CtrlReg.bit.Class2ndDisp_SAVE == 1) return;

    //边沿状态先置无效状态
    p->KeyScan.KeyEdge = INVALIDEDGE;
    p->KeyScan.CurrKey = NONEKEY;


    //Mode键边沿的识别,下降沿有效
    if(p->KeyScan.Input.bit.Mode == 0)
    {
        if(p->KeyScan.FltrInput.bit.Mode == 1)
        {
            p->KeyScan.ModeFltrCnt ++;
            if(p->KeyScan.ModeFltrCnt > 6)
            {
                p->KeyScan.ModeFltrCnt = 0;
                p->KeyScan.FltrInput.bit.Mode = 0;
    
                p->KeyScan.KeyEdge = VALIDEDGE;
                p->KeyScan.CurrKey = MODEKEY; 
                return;                             //Mode键生效时,其它键无效
            }
        }
        else
        {
            p->KeyScan.CurrKey = MODEKEY; 
            return;                                 //按下Mode键时,其它键无效
        }
    }
    else
    {
        p->KeyScan.FltrInput.bit.Mode = 1;
        p->KeyScan.ModeFltrCnt = 0;
    }

    //Up键和Down键的边沿识别  下降沿有效   
    if(UpDownKeyScan(p) == 1) return;  //UP/DOWN键生效时,其它键无效
    
    //Shift键的边沿识别,下降沿有效
    if(p->KeyScan.Input.bit.Shift == 0)
    {
        if(p->KeyScan.FltrInput.bit.Shift == 1)
        {
            p->KeyScan.ShiftFltrCnt ++;
            if(p->KeyScan.ShiftFltrCnt > LONG_PRESS_TIME)    //长按Shift键
            {
                p->KeyScan.ShiftFltrCnt = 0;
                p->KeyScan.FltrInput.bit.Shift = 0;
    
                p->KeyScan.KeyEdge = VALIDEDGE;
                p->KeyScan.CurrKey = LONG_SHIFTKEY;
                return;                                     //SHIFT键生效时,其它键无效
            }
        }
        else
        {
            p->KeyScan.CurrKey = SHIFTKEY;
            return;                                         //SHIFT键按下时,其它键无效
        }
    }
    else 
    {
        if(p->KeyScan.ShiftFltrCnt > 6)              //短按Shift键
        {
            p->KeyScan.ShiftFltrCnt = 0;
            p->KeyScan.FltrInput.bit.Shift = 1;

            p->KeyScan.KeyEdge = VALIDEDGE;
            p->KeyScan.CurrKey = SHIFTKEY;
            return;                                 //SHIFT键生效时,其它键无效
        }
        else
        {
            p->KeyScan.FltrInput.bit.Shift = 1;
            p->KeyScan.ShiftFltrCnt = 0;
        }
    }

    //Set键的边沿识别,下降沿有效
    if(p->KeyScan.Input.bit.Set == 0)
    {
        if(p->KeyScan.FltrInput.bit.Set == 1)
        {
            p->KeyScan.SetFltrCnt ++;
            if(p->KeyScan.SetFltrCnt > LONG_PRESS_TIME)
            {
                p->KeyScan.SetFltrCnt = 0;
                p->KeyScan.FltrInput.bit.Set = 0;
    
                p->KeyScan.KeyEdge = VALIDEDGE;
                p->KeyScan.CurrKey = LONG_SETKEY;
                return;
            }
        }
        else
        {
            p->KeyScan.CurrKey = SETKEY;
            return;
        }
    }
    else
    {
        if(p->KeyScan.SetFltrCnt > 6)
        {
            p->KeyScan.SetFltrCnt = 0;
            p->KeyScan.FltrInput.bit.Set = 1;

            p->KeyScan.KeyEdge = VALIDEDGE;
            p->KeyScan.CurrKey = SETKEY;
            return;
        }
        else
        {
            p->KeyScan.FltrInput.bit.Set = 1;
            p->KeyScan.SetFltrCnt = 0;
        }
    }

    p->KeyScan.CurrKey = NONEKEY;
}

/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
Static_Inline Uint8 UpDownKeyScan(STR_PANELOPERRATION *p)
{
    //同时按下Up键和Down键时,其它键无效,返回
    if((p->KeyScan.Input.bit.Up == 0) && (p->KeyScan.Input.bit.Down == 0))
    {
        p->KeyScan.UpFltrCnt++;

        if(p->KeyScan.UpFltrCnt > 25)    //同时按下200ms后进行处理
        {
            p->KeyScan.DownFltrCnt++;
            
            if(p->KeyScan.DownFltrCnt > 25) 
            {
                p->KeyScan.UpFltrCnt = 0;         //滤波计数器清零
                p->KeyScan.DownFltrCnt = 0;
                p->KeyScan.FltrInput.bit.Up = 0;
                p->KeyScan.FltrInput.bit.Down = 0;
    
                //UP,DOWN键边沿有效
                p->KeyScan.KeyEdge = VALIDEDGE;
                p->KeyScan.CurrKey = UPDOWNKEY;

                //加速等级置为第一级500ms,计数器清零
                p->KeyScan.AccLevel = ACCLEVEL1ST;
                p->KeyScan.AccLevelCnt = 0;
                p->KeyScan.AccCnt = 0;
    
                return 1;   //Up键有效时,shift键和set键无效                 
            }
        }
    }

     //Up键的边沿识别,下降沿有效
    if((p->KeyScan.Input.bit.Up == 0) && (p->KeyScan.FltrInput.bit.Up == 1))   
    {
        p->KeyScan.UpFltrCnt ++;
        
        if(p->KeyScan.UpFltrCnt > 6)
        {
            p->KeyScan.UpFltrCnt = 0;              //滤波计数器清零
            p->KeyScan.FltrInput.bit.Up = 0;
    
            //UP键边沿有效
            p->KeyScan.KeyEdge = VALIDEDGE;
            p->KeyScan.CurrKey = UPKEY;
            //加速等级置为第一级500ms,计数器清零
            p->KeyScan.AccLevel = ACCLEVEL1ST;
            p->KeyScan.AccLevelCnt = 0;
            p->KeyScan.AccCnt = 0;
    
            return 1;   //Up键有效时,shift键和set键无效
        } 
    }
    //Down键下降沿识别
    if((p->KeyScan.Input.bit.Down == 0) && (p->KeyScan.FltrInput.bit.Down == 1))
    {
        p->KeyScan.DownFltrCnt ++;
        
        if(p->KeyScan.DownFltrCnt > 6)
        {
            p->KeyScan.DownFltrCnt = 0;              //滤波计数器清零
            p->KeyScan.FltrInput.bit.Down = 0;
    
            //DOWN键边沿有效
            p->KeyScan.KeyEdge = VALIDEDGE;
            p->KeyScan.CurrKey = DOWNKEY;
    
            //加速等级置为第一级500ms,计数器清零
            p->KeyScan.AccLevel = ACCLEVEL1ST;
            p->KeyScan.AccLevelCnt = 0;
            p->KeyScan.AccCnt = 0;
    
            return 1;   //DOWN键有效时,shift键和set键无效
        } 
    }

    //UP和dOWN键都不按下时，需要清除相关变量
    if((p->KeyScan.Input.bit.Up == 1) && (p->KeyScan.Input.bit.Down == 1))
    {
        //当前键值无效
        p->KeyScan.CurrKey = NONEKEY;

        //复位相关变量
        p->KeyScan.FltrInput.bit.Up = 1;
        p->KeyScan.UpFltrCnt = 0;

        p->KeyScan.FltrInput.bit.Down = 1;
        p->KeyScan.DownFltrCnt = 0;

        //加速等级置为第一级500ms,计数器清零
        p->KeyScan.AccLevel = ACCLEVEL1ST;  
        p->KeyScan.AccLevelCnt = 0;
        p->KeyScan.AccCnt = 0;
    }

    //第一,二,三级菜单下,Up键或Down键加速,低电平有效
    if( ( (p->Menu.CtrlReg.bit.Class == CLASS_1ST) || 
          (p->Menu.CtrlReg.bit.Class == CLASS_2ND) || 
          (p->Menu.CtrlReg.bit.Class == CLASS_3RD) ) &&
        ( (p->KeyScan.FltrInput.bit.Up == 0) || (p->KeyScan.FltrInput.bit.Down == 0)) )
    {
        if(p->KeyScan.AccLevel != 0)
        { 
            if(p->KeyScan.AccCnt <= p->KeyScan.AccLevel)       //等级1, 2, 3
            {
                p->KeyScan.AccCnt ++;
            }
            else 
            {
                p->KeyScan.AccCnt = 0;
                p->KeyScan.AccLevelCnt ++;

                p->KeyScan.KeyEdge = VALIDEDGE;
            }

            //加速等级更新
            if(p->KeyScan.AccLevelCnt == 0) 
            {
                p->KeyScan.AccLevel = ACCLEVEL1ST;                     //等级1，512ms    保持1次
            }            
            else if(p->KeyScan.AccLevelCnt <= 16) 
            {
                p->KeyScan.AccLevel = ACCLEVEL2ND;                     //等级2，128ms    保持16次     2048ms
            }
            else if(p->KeyScan.AccLevelCnt <= 80) 
            {
                p->KeyScan.AccLevel = ACCLEVEL3RD;                     //等级3，32ms     保持64次     2048ms
            }
            else
            {
                p->KeyScan.AccLevel = 0;                               //等级4，4ms      一直保持
            }
        }
        else
        {
            //等级4，4ms      一直有效
            p->KeyScan.KeyEdge = VALIDEDGE; 
        }
    }

    //Up键高时,复位变量
    if(p->KeyScan.Input.bit.Up == 1)
    {
        p->KeyScan.FltrInput.bit.Up = 1;
        p->KeyScan.UpFltrCnt = 0;
    }
    else if(p->KeyScan.FltrInput.bit.Up == 0)
    {
        //Up键按下
        p->KeyScan.CurrKey = UPKEY;
        return 1;       //Up键或Dowm键按下时,shift键和set键无效
    }

    //Down键高时,复位变量
    if(p->KeyScan.Input.bit.Down == 1)
    {
        p->KeyScan.FltrInput.bit.Down = 1;
        p->KeyScan.DownFltrCnt = 0; 
    }
    else if(p->KeyScan.FltrInput.bit.Down == 0)
    {
        //Dowm键按下
        p->KeyScan.CurrKey = DOWNKEY;
        return 1;       //Up键或Dowm键按下时,shift键和set键无效
    }

    return 0;
}
/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
Static_Inline void PanelKeyProcess(STR_PANELOPERRATION *p)
{

    //初始化未完成,只显示RESET,返回
    if(STR_PUB_Gvar.AllInitDone == 0)   return;

    //功能码复位期间按键无效
    if((STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 1)||(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode == 1)||
        (STR_FUNC_Gvar.ManageFunCodeOutput.ResetOD == 1)) return;
    //面板LED测试时按键无效
    if(1 == AuxFunCodeUnion.code.FA_DisTest) return;

    //电流环PI参数自调谐时按键无效
    if((STR_FUNC_Gvar.MonitorFlag.bit.TorqPiTuneEn == 1) && 
       (p->Menu.CtrlReg.bit.Class != CLASS_7TH)) return;

    //JOG结束时,第二级菜单显示SAVE特殊处理,按键无效
    if (p->Menu.CtrlReg.bit.Class2ndDisp_SAVE == 1) return;

    //报错显示处理
    if((STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow == 1) && (STR_FUNC_Gvar.Monitor.HighLevelErrCode != 0x0000))
    {  
        if((p->KeyScan.CurrKey == SETKEY) && (p->Menu.CtrlReg.bit.ErrShowSure == 0) && 
           (p->KeyScan.KeyEdge == VALIDEDGE))
        {
            p->Menu.CtrlReg.bit.ErrShowSure = 1; 
            return;
        }
    }
    else
    {
        p->Menu.CtrlReg.bit.ErrShowSure = 0;
        STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow = 0;
    } 

    //有效按键边沿处理
    if((p->KeyScan.KeyEdge == VALIDEDGE) && (p->KeyScan.CurrKey != NONEKEY) && (p->KeyScan.CurrKey != UPDOWNKEY))  
    {
        if( (STR_FUNC_Gvar.MonitorFlag.bit.ErrorShow) && 
            (STR_FUNC_Gvar.Monitor.HighLevelErrCode != 0x0000) && 
            (p->Menu.CtrlReg.bit.ErrShowSure == 0))
        {  
            return;
        }

        //菜单数组函数
        PanelMenuFun[p->Menu.CtrlReg.bit.Class][p->KeyScan.CurrKey](p);

    }
    else   //有效按键电平处理
    {
        //JOG模式处理
        if(p->Menu.CtrlReg.bit.Class == CLASS_5TH)
        {
            if( (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == NRD) ||
                (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR))
            {
                //故障切换到第2级菜单
                p->Menu.CtrlReg.bit.Class = CLASS_2ND;
            
                //面板当前处理第4位
                p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

                p->Menu.DispGroup = 0x0D;
                p->Menu.DispOffset =11;

                AuxFunCodeUnion.code.FA_Jog = 0x41;
            }
            else if(p->KeyScan.CurrKey == UPKEY)
            {
                AuxFunCodeUnion.code.FA_Jog = 0x11;
            }
            else if(p->KeyScan.CurrKey == DOWNKEY)
            {
                AuxFunCodeUnion.code.FA_Jog = 0x21;
            }
            else
            {
                AuxFunCodeUnion.code.FA_Jog = 0x01;
            }
        }
        else if(p->Menu.CtrlReg.bit.Class == CLASS_6TH)
        {
            if( (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == NRD) ||
                (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == ERR))
            {
                //故障切换到第2级菜单
                p->Menu.CtrlReg.bit.Class = CLASS_2ND;

                //面板当前处理第4位
                p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

                p->Menu.DispGroup = 0x0D;
                p->Menu.DispOffset = 0x02;

                AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x41;
            }
            else if(p->KeyScan.CurrKey == UPKEY)
            {
                AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x11;
            }
            else if(p->KeyScan.CurrKey == DOWNKEY)
            {
                AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x21;
            }
            else
            {
                AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x01;
            }
        }

        //只有在0级菜单下同时按下UpDown键时才进入自调整处理，三级菜单
        if((p->KeyScan.CurrKey == UPDOWNKEY) && (p->Menu.CtrlReg.bit.Class == CLASS_ZERO))
        {
            p->Menu.CtrlReg.bit.Class = CLASS_3RD;

            p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

            p->Menu.DispGroup  = 0x0D;
            p->Menu.DispOffset = 9;
        }
    }
}


/********************************* END OF FILE *********************************/
