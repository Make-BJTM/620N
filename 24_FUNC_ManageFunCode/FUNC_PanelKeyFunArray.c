/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_PanelKeyFunArray.c
 创建人:    王治国                创建日期：2011.11.11      
 描述:
    1. 
    2. 
缩写说明：
    1. fun      ->  function 

 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_PanelKeyFunArray.h"
#include "FUNC_FunCode.h"
#include "FUNC_FunCodeDefault.h" 
#include "FUNC_AuxFunCode.h"
#include "FUNC_OperEeprom.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_InterfaceProcess.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
//用户超级密码
#define USER_SUPER_PASS  18181 

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//第3级菜单,UP和DOWN键操作步进变化的数
const Uint32 StepNumber_Page1st[5] ={10000,1000,100,10,1};
const Uint32 StepNumber_Page2nd[5] ={0,10000000,1000000,100000,10000};
const Uint32 StepNumber_Page3rd[5] ={0,0,0,1000000000,100000000};

const Uint32 StepHexNumber_Page1st[5] ={0x10000,0x1000,0x100,0x10,0x1};
const Uint32 StepHexNumber_Page2nd[5] ={0,0x10000000,0x1000000,0x100000,0x10000};


 //显示组内功能码偏移锁存
static Uint8    DispOffsetLatch[FUNGROUP_ENDINDEX + 1];  

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
//空函数
void NullFuntion(STR_PANELOPERRATION *p);

//第0级菜单-按键函数
void KeyModeFunZero(STR_PANELOPERRATION *p);
void KeyLongShiftFunZero(STR_PANELOPERRATION *p);

//第1级菜单-按键函数
void KeyModeFun1st(STR_PANELOPERRATION *p);
void KeyUpFun1st(STR_PANELOPERRATION *p);
void KeyDownFun1st(STR_PANELOPERRATION *p);
void KeyShiftFun1st(STR_PANELOPERRATION *p);
void KeySetFun1st(STR_PANELOPERRATION *p);

//第2级菜单-按键函数
void KeyModeFun2nd(STR_PANELOPERRATION *p);
void KeyUpFun2nd(STR_PANELOPERRATION *p);
void KeyDownFun2nd(STR_PANELOPERRATION *p);
void KeyShiftFun2nd(STR_PANELOPERRATION *p);
void KeySetFun2nd(STR_PANELOPERRATION *p);

//第3级菜单-按键函数
void KeyModeFun3rd(STR_PANELOPERRATION *p);
void KeyUpFun3rd(STR_PANELOPERRATION *p);
void KeyDownFun3rd(STR_PANELOPERRATION *p);
void KeyShiftFun3rd(STR_PANELOPERRATION *p);
void KeyLongShiftFun3rd(STR_PANELOPERRATION *p);
void KeySetFun3rd(STR_PANELOPERRATION *p);

//第4级菜单-按键函数
void KeyModeFun4th(STR_PANELOPERRATION *p);
void KeyUpFun4th(STR_PANELOPERRATION *p);
void KeyDownFun4th(STR_PANELOPERRATION *p);
void KeyShiftFun4th(STR_PANELOPERRATION *p);
void KeySetFun4th(STR_PANELOPERRATION *p);

//第5级菜单-按键函数
void KeyModeFun5th(STR_PANELOPERRATION *p);

//第6级菜单-按键函数
void KeyModeFun6th(STR_PANELOPERRATION *p);

//第7级菜单-按键函数
void KeyModeFun7th(STR_PANELOPERRATION *p);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
//获取第3级菜单,UP和DOWN键操作步进变化的数
Static_Inline Uint32 GetStepNumber(STR_PANELOPERRATION *p);

/*******************************************************************************
  函数名:  void NullFuntion(void)
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
void NullFuntion(STR_PANELOPERRATION *p)
{
    return;
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
void KeyModeFunZero(STR_PANELOPERRATION *p)
{
    //由第0级菜单切换到第1级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_1ST;

    //面板当前处理第2位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_2ND;

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
void KeyLongShiftFunZero(STR_PANELOPERRATION *p)
{
    Uint8   DispBits = 0;
    int8    DispOffset;        //显示功能码组内偏移

    if( (FunCodeUnion.code.BP_DefaultDisplayCode >= H0B_PANELDISPLEN) || 
        (FunCodeUnion.code.BP_DefaultDisplayCode == 4) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 6) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 8) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 14) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 16) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 18) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 20) ||
        (  (FunCodeUnion.code.BP_DefaultDisplayCode > 30)  
         &&(FunCodeUnion.code.BP_DefaultDisplayCode < 53) ) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 54) ||
        (FunCodeUnion.code.BP_DefaultDisplayCode == 56)   )
    {
        return;
    }

    DispOffset = FunCodeUnion.code.BP_DefaultDisplayCode;

    DispBits = GetAttrib_DispBits(0x0B , DispOffset);
    if(DispBits == 0) DispBits = 1;

    //如果显示位数大于5,说明要翻页,LongShift实现翻页功能
    if(DispBits > 5)
    {
        p->Menu.CtrlReg.bit.Page ++;

        //页数范围检查
        DispBits --;
        DispBits = DispBits / 4;

        if(p->Menu.CtrlReg.bit.Page  >  DispBits)
        {
            p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
        }
    }
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
void KeyModeFun1st(STR_PANELOPERRATION *p)
{
    //由第1级菜单切换到第0级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_ZERO; 
    p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
    //面板当前无处理位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
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
void KeyUpFun1st(STR_PANELOPERRATION *p)
{
    //处理位设置出错时,当前处理位复位到第2位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_1ST) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_2ND))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_2ND;
    }

    //组增加
    if(p->Menu.CtrlReg.bit.DealBit == DEALBIT_1ST)
    {
        p->Menu.DispGroup += 0x10;
    }
    else
    {
        p->Menu.DispGroup ++;
    }

    //范围限制
    if(p->Menu.DispGroup > FUNGROUP_ENDINDEX ) p->Menu.DispGroup = 0;

    //如果本组首功能码显示长度为0,跳过该组
    while(FunCode_PanelDispLen[p->Menu.DispGroup] == 0)
    { 
        p->Menu.DispGroup ++; 
        if(p->Menu.DispGroup > FUNGROUP_ENDINDEX ) p->Menu.DispGroup = 0;
    }

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
void KeyDownFun1st(STR_PANELOPERRATION *p)
{
    //处理位设置出错时,当前处理位复位到第2位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_1ST) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_2ND))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_2ND;
    }

    //组减少
    if(p->Menu.CtrlReg.bit.DealBit == DEALBIT_1ST)
    {
        p->Menu.DispGroup -= 0x10;
    }
    else
    {
        p->Menu.DispGroup --;
    }

    //范围限制
    if((int8)p->Menu.DispGroup < 0 ) p->Menu.DispGroup = FUNGROUP_ENDINDEX;

    //如果本组首功能码显示长度为0,跳过该组
    while(FunCode_PanelDispLen[p->Menu.DispGroup] == 0)
    {
        p->Menu.DispGroup --;
        if((int8)p->Menu.DispGroup < 0 ) p->Menu.DispGroup = FUNGROUP_ENDINDEX;
    } 

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
void KeyShiftFun1st(STR_PANELOPERRATION *p)
{
    //当前处理位移位
    p->Menu.CtrlReg.bit.DealBit = p->Menu.CtrlReg.bit.DealBit ^ 0x3;

    //处理位设置出错时,当前处理位复位到第2位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_1ST) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_2ND))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_2ND;
    }
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
void KeySetFun1st(STR_PANELOPERRATION *p)
{
    //由第1级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;
    p->Menu.DispOffset = DispOffsetLatch[p->Menu.DispGroup];

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

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
void KeyModeFun2nd(STR_PANELOPERRATION *p)
{
    //由第2级菜单切换到第1级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_1ST;
    DispOffsetLatch[p->Menu.DispGroup] = p->Menu.DispOffset;

    //面板当前处理第2位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_2ND;

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
void KeyUpFun2nd(STR_PANELOPERRATION *p)
{
    //处理位设置出错时,当前处理位复位到第4位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_3RD) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_4TH))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }

    //偏移量增加
    if(p->Menu.CtrlReg.bit.DealBit == DEALBIT_3RD)
    {
        p->Menu.DispOffset += 10;
    }
    else
    {
        p->Menu.DispOffset ++;
    }

    //范围限制,分成3组H00、H01其它
    if(p->Menu.DispGroup == 0)
    {
        //如果厂家密码通过,或者电机组可见功能码MT_EnVisable >= 10000时,正常处理
        if((FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) || 
           (FunCodeUnion.code.MT_EnVisable > 10000))
        {
            //范围限制
            if(p->Menu.DispOffset > FunCode_PanelDispLen[0] - 1) p->Menu.DispOffset = 0;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(0 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset ++;
                //范围限制
                if(p->Menu.DispOffset > FunCode_PanelDispLen[0] - 1) p->Menu.DispOffset = 0;
            }
        }
        else //只显示3个功能码
        {
            if(p->Menu.DispOffset > H00_PANELDISPLEN_OEM - 1) p->Menu.DispOffset = 0;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(0 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset ++;
                //范围限制
                if(p->Menu.DispOffset > H00_PANELDISPLEN_OEM - 1) p->Menu.DispOffset = 0;
            }
        }
    }
    else if(p->Menu.DispGroup == 1)
    {
        //如果厂家密码通过,正常处理
        if(FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD)
        {
            //范围限制
            if(p->Menu.DispOffset > FunCode_PanelDispLen[1] - 1) p->Menu.DispOffset = 0;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(1 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset ++;
                //范围限制
                if(p->Menu.DispOffset > FunCode_PanelDispLen[1] - 1) p->Menu.DispOffset = 0;
            }
        }
        else //只显示2个功能码
        {
            if(p->Menu.DispOffset > H01_PANELDISPLEN_OEM - 1) p->Menu.DispOffset = 0;
        }
    }
    else
    {
        //范围限制
        if(p->Menu.DispOffset > FunCode_PanelDispLen[p->Menu.DispGroup] - 1) p->Menu.DispOffset = 0;

        //32位功能码高字,跳过该功能码
        if( GetAttrib_DataIndex(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
        { 
            p->Menu.DispOffset ++;
            //范围限制
                if(p->Menu.DispOffset > FunCode_PanelDispLen[p->Menu.DispGroup] - 1) p->Menu.DispOffset = 0;
        }
    }
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
void KeyDownFun2nd(STR_PANELOPERRATION *p)
{
    //处理位设置出错时,当前处理位复位到第4位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_3RD) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_4TH))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }

    //偏移量减少
    if(p->Menu.CtrlReg.bit.DealBit == DEALBIT_3RD)
    {
        p->Menu.DispOffset -= 10;
    }
    else
    {
        p->Menu.DispOffset --;
    }

    //范围限制,分成3组H00、H01其它
    if(p->Menu.DispGroup == 0)
    {
        //如果厂家密码通过,或者电机组可见功能码MT_EnVisable >= 10000时,正常处理
        if((FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) || 
           (FunCodeUnion.code.MT_EnVisable > 10000))
        {
            //范围限制
            if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[0] - 1;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(0 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset --;

                //范围限制
                if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[0] - 1;
            }
        }
        else //只显示3个功能码
        {
            if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = H00_PANELDISPLEN_OEM - 1;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(0 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset --;

                //范围限制
                if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = H00_PANELDISPLEN_OEM - 1;
            }
        }
    }
    else if(p->Menu.DispGroup == 1)
    {
        //如果厂家密码通过,正常处理
        if(FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD)
        {
            //范围限制
            if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[1] - 1;

            //32位功能码高字,跳过该功能码
            if( GetAttrib_DataIndex(1 , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
            { 
                p->Menu.DispOffset --;

                //范围限制
                if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[1] - 1;
            }
        }
        else //只显示3个功能码
        {
            if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = H01_PANELDISPLEN_OEM - 1;
        }
    }
    else
    {
        //范围限制
        if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[p->Menu.DispGroup] - 1;

            //32位功能码高字,跳过该功能码
        if( GetAttrib_DataIndex(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_HIGH_WORD )
        { 
            p->Menu.DispOffset --;

            //范围限制
            if((int8)p->Menu.DispOffset < 0 ) p->Menu.DispOffset = FunCode_PanelDispLen[p->Menu.DispGroup] - 1;
        }
    }
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
void KeyShiftFun2nd(STR_PANELOPERRATION *p)
{
    //当前处理位移位
    p->Menu.CtrlReg.bit.DealBit = p->Menu.CtrlReg.bit.DealBit ^ 0x7;

    //处理位设置出错时,当前处理位复位到第4位
    if((p->Menu.CtrlReg.bit.DealBit != DEALBIT_3RD) && (p->Menu.CtrlReg.bit.DealBit != DEALBIT_4TH))
    {
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }
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
void KeySetFun2nd(STR_PANELOPERRATION *p)
{
    //如果当前非Soff且处理H0D_11，返回 
    if( (p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 11) && 
        (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RDY) ) 
    {    
        return;
    }
    //如果处理H0D_02,直接进入第6级菜单  惯量辨识模式
    if( (p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 0x02) )
    {
        //如果当前非Soff，返回
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RDY)  return;

        //显示H08_15
        p->Menu.DispGroup = 0x08;
        p->Menu.DispOffset =15;
        p->Menu.DispData = AuxFunCodeUnion.code.OnLineInertiaRatio;
        AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x01;

        //由第3级菜单切换到第5级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_6TH;
        //显示页数置0 
        p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
    
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
        return;
    }

    //由第2级菜单切换到第3级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_3RD;
    //显示页数置0 
    p->Menu.CtrlReg.bit.Page = PAGE_ZERO;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

    //获取显示数据
    //如果当前处理的时H0D_11,显示数据为H06_04赋给
    if((p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 11))
    {
        p->Menu.DispData = 200;
        //监控模块锁存H0604使用
        AuxFunCodeUnion.code.FA_Jog = 0x80;
    }
    else if(GetAttrib_PanelAttrib(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_PANEL_UNREAD)
    {
         p->Menu.DispData = 0;
    }
    else if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD) 
    {   //32位数据
        p->Menu.DispData = A_SHIFT16_PLUS_B(GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset + 1),
                                            GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset));
    }
    else
    {   //16位数据
        p->Menu.DispData = (Uint32)GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset);
        p->Menu.DispData &= 0xFFFF;
        //如果是负数,高十六位置1
        if((GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN) && 
           (p->Menu.DispData > 32767))
        {
            p->Menu.DispData |= 0xFFFF0000;
        }
    }

    //第3级菜单用户密码设定模式判断
    //厂家密码状态下 用户密码保护状态自动解除
    //厂家密码状态下 不能修改用户密码 只能清除 
    if( (FunCodeUnion.code.OEM_OEMPass != OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) )
    {
        if(p->Menu.UserPass != FunCodeUnion.code.BP_UserPass_Rsvd)
        {
            p->Menu.CtrlReg.bit.Class3rdMode = USERPROTECT;
         }
        else
        {
            p->Menu.CtrlReg.bit.Class3rdMode = USERDECRYPT;
        }

        return;
    }

    if( (FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) )
    {
        p->Menu.CtrlReg.bit.Class3rdMode = NORMALMODE;
        p->Menu.DispData = 0;
        return;
    }

    //第3级菜单模式判断, 写属性->面板属性->用户密码->第0组保护->用户密码设定
    //根据写属性判断
    switch( GetAttrib_Writable(p->Menu.DispGroup , p->Menu.DispOffset) )
    {
        case ATTRIB_ANY_WRT:           //随时设定
            p->Menu.CtrlReg.bit.Class3rdMode = NORMALMODE;
            break;

        case ATTRIB_POSD_WRT:           //停机时设定
            if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
            {
                p->Menu.CtrlReg.bit.Class3rdMode = RUNREAD;     //伺服运行状态只读,不可以更改
            }
            else
            {
                p->Menu.CtrlReg.bit.Class3rdMode = NORMALMODE;  
            }
            break; 

        case ATTRIB_DISP_WRT:            //仅显示 只读
            p->Menu.CtrlReg.bit.Class3rdMode = READONLY;    //只读属性,不可以更改
            break;

        case ATTRIB_RSVD_WRT:
            p->Menu.CtrlReg.bit.Class3rdMode = READONLY;    //只读属性,不可以更改
            p->Menu.DispData = 0;
            break;
    }

    //如果是厂家保护功能码,未认证时按保留功能码处理
    if( (GetAttrib_OEMProtect(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_OEMPROTECT) &&
        (FunCodeUnion.code.OEM_OEMPass != OEMPASSWORD) && (FunCodeUnion.code.MT_EnVisable <= 10000) )
    {
        p->Menu.CtrlReg.bit.Class3rdMode = READONLY;    //只读属性,不可以更改
        p->Menu.DispData = 0;   
    }

    //根据面板属性判断,如果面板属性是不可改变,模式更改为只读模式
    if(GetAttrib_PanelAttrib(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_PANEL_UNCHANGE)
    {
        p->Menu.CtrlReg.bit.Class3rdMode = READONLY;    //不可以更改
    }
    
    //根据用户密码设定判断 1. 厂家密码未通过认证 H0240<10000 2.当前操作功能码不是H0241 3.用户密码未通过认证
    if( (FunCodeUnion.code.OEM_OEMPass != OEMPASSWORD) &&
        (FunCodeUnion.code.MT_EnVisable <= 10000) &&
        (!((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 41))) &&
        (p->Menu.UserPass != FunCodeUnion.code.BP_UserPass_Rsvd) )
    {
        p->Menu.CtrlReg.bit.Class3rdMode = USERLOCK;
    }

    //第0组保护,H0000设为65535或14XXX时电机参数才可以更改。
    if((p->Menu.DispGroup == 0)&& (p->Menu.DispOffset != 0) &&
       (65535 != FunCodeUnion.code.MT_MotorModel) &&
       (14 != (FunCodeUnion.code.MT_MotorModel/1000)) )
    {
        p->Menu.CtrlReg.bit.Class3rdMode = READONLY;    //不可以更改
    }
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
void KeyModeFun3rd(STR_PANELOPERRATION *p)
{
    //由第3级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

    //如果处理H0D_11,还原H0604 H0605 H0606功能码
    if( (p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 11) )
    {
        AuxFunCodeUnion.code.FA_Jog = 0x81;
    }
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
Static_Inline Uint32 GetStepNumber(STR_PANELOPERRATION *p)
{
    Uint8 DispBits = 0;

    DispBits = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);
    if(DispBits == 0) DispBits = 1;

    if(DispBits < 6)  //1页显示
    {
        if( GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_HNUM_TYP )
        {
            return(StepHexNumber_Page1st[p->Menu.CtrlReg.bit.DealBit]);
        }
        else
        {
            return(StepNumber_Page1st[p->Menu.CtrlReg.bit.DealBit]);
        }
    }
    else   //需要多页页显示
    {
        if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)       //第0页
        {
            if( GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_HNUM_TYP )
            {
                return(StepHexNumber_Page1st[p->Menu.CtrlReg.bit.DealBit]);
            }
            else
            {
                return(StepNumber_Page1st[p->Menu.CtrlReg.bit.DealBit]);
            }
        }
        else if(p->Menu.CtrlReg.bit.Page == PAGE_1ST)   //第1页
        {
            if( GetAttrib_DataType(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_HNUM_TYP )
            {
                return(StepHexNumber_Page2nd[p->Menu.CtrlReg.bit.DealBit]);
            }
            else
            {
                return(StepNumber_Page2nd[p->Menu.CtrlReg.bit.DealBit]);
            }
        }
        else  //第3页
        {
            return(StepNumber_Page3rd[p->Menu.CtrlReg.bit.DealBit]);
        }
    }
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
void KeyUpFun3rd(STR_PANELOPERRATION *p)
{
    Uint32  FunCodeUpperLimit = 0;

    //以下模式,本键无效
    if((p->Menu.CtrlReg.bit.Class3rdMode == READONLY) || 
       (p->Menu.CtrlReg.bit.Class3rdMode == RUNREAD) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERPROTECT) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT_SW))  return;
    
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERLOCK)
    {
        KeySetFun3rd(p);
        return;
    }

    //厂家密码状态下 不能修改用户密码 只能清除
    if( (FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) ) return;
                     
    //当前处理位数超出范围时,返回
    if(p->Menu.CtrlReg.bit.DealBit > 4)  return;

    //步进加
    p->Menu.DispData += GetStepNumber(p);

    if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD)        //32位数据
    {
        //获取上限数据
        FunCodeUpperLimit = A_SHIFT16_PLUS_B(GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset + 1),
                                            GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset));
        //有符号上限判断
        if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
        {
             if((int32)p->Menu.DispData > (int32)FunCodeUpperLimit) 
             {
                p->Menu.DispData = FunCodeUpperLimit;
             }
        }
        else //无符号上限判断
        {
             if((Uint32)p->Menu.DispData > (Uint32)FunCodeUpperLimit) 
             {
                p->Menu.DispData = FunCodeUpperLimit;
             }
        } 
    }
    else //16位数据        
    {
        //获取上限数据
        if(GetAttrib_UpperLmt(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_RLAT_LMTH)
        {
            FunCodeUpperLimit = (Uint32)GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset);
            FunCodeUpperLimit = FunCodeUnion.all[FunCodeUpperLimit];
        }
        else
        {
            FunCodeUpperLimit = (Uint32)GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset);
        }

        //有符号上限判断
        if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
        {
             //补齐符号位
             if(FunCodeUpperLimit & 0x00008000) FunCodeUpperLimit = FunCodeUpperLimit | 0xFFFF0000;

             if((int32)p->Menu.DispData > (int32)FunCodeUpperLimit) 
             {
                p->Menu.DispData = FunCodeUpperLimit;
             }
        }
        else   //无符号上限判断
        {
             if((Uint32)p->Menu.DispData > (Uint32)FunCodeUpperLimit) 
             {
                p->Menu.DispData = FunCodeUpperLimit;
             }
        }
    }
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
void KeyDownFun3rd(STR_PANELOPERRATION *p)
{
    Uint32  FunCodeLowerLimit = 0;
    Uint32  FunCodeUpperLimit = 0;

    //以下模式,本键无效
    if((p->Menu.CtrlReg.bit.Class3rdMode == READONLY) || 
       (p->Menu.CtrlReg.bit.Class3rdMode == RUNREAD) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERPROTECT) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT_SW))  return;
    
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERLOCK)
    {
        KeySetFun3rd(p);
        return;
    }

    //厂家密码状态下 不能修改用户密码 只能清除
    if( (FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) ) return;

    //当前处理位数超出范围时,返回
    if(p->Menu.CtrlReg.bit.DealBit > 4) return;

    //步进减
    p->Menu.DispData -= GetStepNumber(p);

    if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD)        //32位数据
    {
        //获取下限数据
        FunCodeLowerLimit = A_SHIFT16_PLUS_B(GetDftLowerLmt(p->Menu.DispGroup , p->Menu.DispOffset + 1),
                                             GetDftLowerLmt(p->Menu.DispGroup , p->Menu.DispOffset));
        //有符号下限判断
        if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
        {
             if((int32)p->Menu.DispData < (int32)FunCodeLowerLimit) 
             {
                p->Menu.DispData = FunCodeLowerLimit;
             }
        }
        else //无符号下限判断
        {
             if(((Uint32)p->Menu.DispData < (Uint32)FunCodeLowerLimit) || ((int32)p->Menu.DispData < 0) )
             {
                p->Menu.DispData = FunCodeLowerLimit;
             }
        } 
    }
    else //16位数据        
    {
        //获取上限数据
        if(GetAttrib_UpperLmt(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_RLAT_LMTH)
        {
            FunCodeUpperLimit = (Uint32)GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset);
            FunCodeUpperLimit = FunCodeUnion.all[FunCodeUpperLimit];
        }
        else
        {
            FunCodeUpperLimit = (Uint32)GetDftUpperLmt(p->Menu.DispGroup , p->Menu.DispOffset);
        }

        //获取下限数据
        if(GetAttrib_LowerLmt(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_RLAT_LMTL)
        {
            FunCodeLowerLimit = (Uint32)GetDftLowerLmt(p->Menu.DispGroup , p->Menu.DispOffset);
            FunCodeLowerLimit = FunCodeUnion.all[FunCodeLowerLimit];
        }
        else
        {
            FunCodeLowerLimit = (Uint32)GetDftLowerLmt(p->Menu.DispGroup , p->Menu.DispOffset);
        }

        //有符号下限判断
        if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
        {
            //补齐符号位
            if(FunCodeLowerLimit & 0x00008000) FunCodeLowerLimit = FunCodeLowerLimit | 0xFFFF0000;

            if((int32)p->Menu.DispData < (int32)FunCodeLowerLimit) 
            {
               p->Menu.DispData = FunCodeLowerLimit;
            }
        }
        else   //无符号下限判断
        {
             if( ((Uint32)p->Menu.DispData < (Uint32)FunCodeLowerLimit) || ((Uint32)p->Menu.DispData > (Uint32)FunCodeUpperLimit) )
             {
                p->Menu.DispData = FunCodeLowerLimit;
             }
        }
    }
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
void KeyShiftFun3rd(STR_PANELOPERRATION *p)
{
    Uint8  DispBits = 0;

    //以下模式,本键无效
    if((p->Menu.CtrlReg.bit.Class3rdMode == READONLY) || 
       (p->Menu.CtrlReg.bit.Class3rdMode == RUNREAD) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERPROTECT) ||
       (p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT_SW))  return;
    
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERLOCK)
    {
        KeySetFun3rd(p);
        return;
    }

    //厂家密码状态下 不能修改用户密码 只能清除
    if( (FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) ) return;

    DispBits = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);
    if(DispBits == 0) DispBits = 1;

    if(DispBits < 6)  //1页显示
    {
        if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
        {
            DispBits --;    //减掉符号位
        }

        p->Menu.CtrlReg.bit.DealBit --;


        if((p->Menu.CtrlReg.bit.DealBit < (5 - DispBits)) || (p->Menu.CtrlReg.bit.DealBit > 4))
        {
            p->Menu.CtrlReg.bit.DealBit = 4;
        }
    }
    else if(DispBits < 9)    //需要2页显示
    {
        if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)     //第0页
        {
            p->Menu.CtrlReg.bit.DealBit --;
            if(p->Menu.CtrlReg.bit.DealBit < 1)
            {
                p->Menu.CtrlReg.bit.DealBit = 4;
                p->Menu.CtrlReg.bit.Page = PAGE_1ST;
            }
        }
        else   //第1页
        {
            if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
            {
                DispBits --;        //减掉符号位
            }

            p->Menu.CtrlReg.bit.DealBit --;

            if((p->Menu.CtrlReg.bit.DealBit < (9 - DispBits)) || (p->Menu.CtrlReg.bit.DealBit > 4))
            {
                p->Menu.CtrlReg.bit.DealBit = 4;
                p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
            }
        }
    }
    else   //需要3页显示
    {
        if(p->Menu.CtrlReg.bit.Page == PAGE_ZERO)       //第0页
        {
            p->Menu.CtrlReg.bit.DealBit --;
            if(p->Menu.CtrlReg.bit.DealBit < 1)
            {
                p->Menu.CtrlReg.bit.DealBit = 4;
                p->Menu.CtrlReg.bit.Page = PAGE_1ST;
            }
        }
        else if(p->Menu.CtrlReg.bit.Page == PAGE_1ST)   //第1页
        {
            p->Menu.CtrlReg.bit.DealBit --;
            if(p->Menu.CtrlReg.bit.DealBit < 1)
            {
                p->Menu.CtrlReg.bit.DealBit = 4;
                p->Menu.CtrlReg.bit.Page = PAGE_2ND;

                if((DispBits == 9) && (GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN))
                {
                    p->Menu.CtrlReg.bit.DealBit = 0xF;  //显示位数是9时,第3页只显示符号 符号不闪烁
                }
            }
        }
        else  //第3页
        {
            if(GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN)
            {
                DispBits --;      //减掉符号位 
            }

            p->Menu.CtrlReg.bit.DealBit --; 

            if((p->Menu.CtrlReg.bit.DealBit < (12 - DispBits)) || (p->Menu.CtrlReg.bit.DealBit > 4))
            {
                p->Menu.CtrlReg.bit.DealBit = 4;
                p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
            } 
        }
    }
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
void KeyLongShiftFun3rd(STR_PANELOPERRATION *p)
{
    Uint8  DispBits = 0;

    DispBits = GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset);
    if(DispBits == 0) DispBits = 1;

    //如果显示位数大于5,说明要翻页,LongShift实现翻页功能
    if(DispBits > 5)
    {
        p->Menu.CtrlReg.bit.Page ++;

        //页数范围检查
        DispBits --;
        DispBits = DispBits / 4;

        if(p->Menu.CtrlReg.bit.Page  >  DispBits)
        {
            p->Menu.CtrlReg.bit.Page = PAGE_ZERO;
        }

        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH; 

        //显示位数是9时,第2页只显示符号 符号不闪烁
        if((p->Menu.CtrlReg.bit.Page == PAGE_2ND) &&
           (GetAttrib_DispBits(p->Menu.DispGroup , p->Menu.DispOffset) == 9) && 
           (GetAttrib_Sign(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_INT_SIGN))
        {
             p->Menu.CtrlReg.bit.DealBit = 0xF;  
        }
    }
    else
    {
        KeyShiftFun3rd(p);
    } 
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
void KeySetFun3rd(STR_PANELOPERRATION *p)
{
    //如果处理H0D_11,将设定值赋给H06_04,但是不存储,进入第五级菜单
    if( (p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 11) )
    {
        //当前非Soff返回
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RDY) return;

        FunCodeUnion.code.SL_JOGSpdCommand = p->Menu.DispData;
        AuxFunCodeUnion.code.FA_Jog = 0x01;

        //由第3级菜单切换到第5级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_5TH;    
        //面板当前无处理位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
        return;
    }

   //如果处理H0D_06,进入第七级菜单
    if( (p->Menu.DispGroup == 0x0D) && (p->Menu.DispOffset == 6)&& (p->Menu.DispData != 0))
    {
        //当前非Soff返回
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RDY) return;

        AuxFunCodeUnion.code.FA_TorqPiTune = p->Menu.DispData;

        //由第3级菜单切换到第7级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_7TH;    
        //面板当前无处理位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
        return;
    }


    //进入解密过程切换状态
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERPROTECT)
    {
        p->Menu.CtrlReg.bit.Class3rdMode = USERDECRYPT_SW;
        return;
    }

    //进入解密状态
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT_SW)
    {
        p->Menu.CtrlReg.bit.Class3rdMode = USERDECRYPT;
        return;
    }

    //用户密码模式
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERDECRYPT)
    {
        if(p->Menu.UserPass != FunCodeUnion.code.BP_UserPass_Rsvd) //破解密码
        {
           //如果没有输入密码值,返回
            if(p->Menu.DispData == 0)  return;

            //用户输入密码成功判断
            if((Uint16)p->Menu.DispData == FunCodeUnion.code.BP_UserPass_Rsvd)
            {
                p->Menu.UserPass = FunCodeUnion.code.BP_UserPass_Rsvd;
            }
        }
        else  //用户修改密码
        {
           //如果没有修改密码值,返回
            if(p->Menu.UserPass == (Uint16)p->Menu.DispData)  return;

            p->Menu.UserPass = (Uint16)p->Menu.DispData;
            FunCodeUnion.code.BP_UserPass_Rsvd = (Uint16)p->Menu.DispData;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.BP_UserPass_Rsvd));
            PostErrMsg(PCHGDWARN);
        }

       //由第3级菜单切换到第4级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_4TH;
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
        return;
    }

    //厂家密码状态下 不能修改用户密码 只能清除
    if( (FunCodeUnion.code.OEM_OEMPass == OEMPASSWORD) &&
        ((p->Menu.DispGroup == 2) && (p->Menu.DispOffset == 30)) )
    {
        //清除密码
        p->Menu.UserPass = (Uint16)p->Menu.DispData;
        FunCodeUnion.code.BP_UserPass_Rsvd = (Uint16)p->Menu.DispData;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.BP_UserPass_Rsvd));

        //由第3级菜单切换到第4级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_4TH;
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
        return;
    }
    
    //用户锁定状态下 显示-----
    if(p->Menu.CtrlReg.bit.Class3rdMode == USERLOCK)
    {
       //由第3级菜单切换到第4级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_4TH;
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE;
    }

    //非正常模式或用户密码模式,返回
    if(p->Menu.CtrlReg.bit.Class3rdMode != NORMALMODE)  return;

    //判断是否更改功能码的值,如果没有更改功能码的值,返回
    if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD)
    {
        if( ((Uint16)(p->Menu.DispData & 0xFFFF) == GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset) &&
            ((Uint16)(p->Menu.DispData >> 16) == GetFunCode( p->Menu.DispGroup , (p->Menu.DispOffset + 1)))) )
            return;
    }
    else
    { 
        //用户密码保护模式时不进行比较,H0230等于0，防止设定值等于0时,set键无效
        if((Uint16)(p->Menu.DispData & 0xFFFF) == GetFunCode(p->Menu.DispGroup , p->Menu.DispOffset))
            return;
    }

    //由第3级菜单切换到第4级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_4TH;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_NONE; 

    //更改功能码值
    SetFunCode(p->Menu.DispGroup , p->Menu.DispOffset ,  p->Menu.DispData & 0xFFFF);

    if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD)        //32位数据
    {
        SetFunCode(p->Menu.DispGroup ,(p->Menu.DispOffset + 1) ,  p->Menu.DispData >> 16);
    }

    //如果更改H03 H04 H17组参数 STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate置1
    if((p->Menu.DispGroup == 0x03) || (p->Menu.DispGroup == 0x04) || (p->Menu.DispGroup == 0x17))
    {
        STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate = 1;
    }

    //Eeprom存储
    if((GetAttrib_PanelAttrib(p->Menu.DispGroup , p->Menu.DispOffset) == 0) &&
       (p->Menu.DispGroup != 0x0D) && (p->Menu.DispGroup != 0x0B))
    {
        if(GetAttrib_Writable(p->Menu.DispGroup , p->Menu.DispOffset) > 2)  return;         //只有停机设定和任意设定属性存储在Eeprom中

        SaveToEepromOne(GetGroupCodeIndex(p->Menu.DispGroup , p->Menu.DispOffset));

        if(GetAttrib_DataBits(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_TWO_WORD)        //32位数据
        {
            SaveToEepromOne(GetGroupCodeIndex(p->Menu.DispGroup , p->Menu.DispOffset) + 1);
        }

        if(GetAttrib_Active(p->Menu.DispGroup , p->Menu.DispOffset) == ATTRIB_NEXT_ACT)
        {
            PostErrMsg(PCHGDWARN);
            //WarnReg.bit.MCHGDWARNFLG = 1;
        }
    }
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
void KeyModeFun4th(STR_PANELOPERRATION *p)
{
    //由第4级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
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
void KeyUpFun4th(STR_PANELOPERRATION *p)
{
    if(USERLOCK == p->Menu.CtrlReg.bit.Class3rdMode)
    {
        //由第4级菜单切换到第3级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_3RD;
    
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }
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
void KeyDownFun4th(STR_PANELOPERRATION *p)
{
    if(USERLOCK == p->Menu.CtrlReg.bit.Class3rdMode)
    {
        //由第4级菜单切换到第3级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_3RD;
    
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }
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
void KeyShiftFun4th(STR_PANELOPERRATION *p)
{
    if(USERLOCK == p->Menu.CtrlReg.bit.Class3rdMode)
    {
        //由第4级菜单切换到第3级菜单
        p->Menu.CtrlReg.bit.Class = CLASS_3RD;
    
        //面板当前处理第4位
        p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
    }
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
void KeySetFun4th(STR_PANELOPERRATION *p)
{
    //由第4级菜单切换到第3级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_3RD;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
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
void KeyModeFun5th(STR_PANELOPERRATION *p)
{
    //退出速度JOG模式
    AuxFunCodeUnion.code.FA_Jog = 0x41;

    //由第5级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
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
void KeyModeFun6th(STR_PANELOPERRATION *p)
{
    //退出惯量辨识模式
    AuxFunCodeUnion.code.FA_OffLnInrtMod = 0x41;  //zxh

    p->Menu.DispGroup = 0x0D;
    p->Menu.DispOffset =0x02;

    //由第6级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
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
void KeyLongSetFun6th(STR_PANELOPERRATION *p)
{
    //退出速度JOG模式
    AuxFunCodeUnion.code.FA_Jog = 0x41;
    p->Menu.DispGroup = 0x0D;
    p->Menu.DispOffset = 0x02;

    //由第5级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;

    //存储H0815
    FunCodeUnion.code.GN_InertiaRatio = AuxFunCodeUnion.code.OnLineInertiaRatio;
    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_InertiaRatio));

    //第二级菜单显示SAVE标志位使能
    p->Menu.CtrlReg.bit.Class2ndDisp_SAVE = 1;
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
void KeyModeFun7th(STR_PANELOPERRATION *p)
{
    //退出电流环参数自调谐模式
    AuxFunCodeUnion.code.FA_TorqPiTune = 0;

    //由第7级菜单切换到第2级菜单
    p->Menu.CtrlReg.bit.Class = CLASS_2ND;

    //面板当前处理第4位
    p->Menu.CtrlReg.bit.DealBit = DEALBIT_4TH;
}

/********************************* END OF FILE *********************************/
