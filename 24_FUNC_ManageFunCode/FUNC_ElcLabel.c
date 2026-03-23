/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_ElcLabel.c
 创建人：   王治国                 修改日期：2013.08.15 
 描述:
    1. 
    2. 
 修改记录：  
    1. 

********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_FunCode.h"
#include "FUNC_AuxFunCode.h"
#include "FUNC_FunCodeDefault.h" 
#include "FUNC_OperEeprom.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_WWDG.h"
#include "stm32f4xx.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//电子标签定义
UNI_ELECTRONIC_LABEL  UNI_EleLabel;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitElcLabel(void);
void ElcLabelDeal(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 

/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
void InitElcLabel(void)
{
    Uint8 i = 0;

    for(i=0;i<32;i++)
    {
        UNI_EleLabel.all[i] = 0;
    }

    UNI_EleLabel.Label.JumpFlag = 0;
    UNI_EleLabel.Label.DelayCnt = 0;

    UNI_EleLabel.Label.ServoStatus = 0x0300;             //伺服状态:1 运行状态;  3停止状态

    UNI_EleLabel.Label.Manufacturer = EL_OEM;                   //厂商
    UNI_EleLabel.Label.ProductLine = EL_PRODUCT_LINE;           //产品线
    UNI_EleLabel.Label.Model = EL_MODEL;                        //产品型号
    UNI_EleLabel.Label.ProductVer = EL_PRODUCT_VER;             //产品版本
    UNI_EleLabel.Label.CanVer = EL_CAN_VER;                     //通信版本
    UNI_EleLabel.Label.BootVer = EL_BOOT_VER;                   //BOOT版本
    UNI_EleLabel.Label.ChipModel = EL_CHIP_MODEL;               //芯片型号
    UNI_EleLabel.Label.NonstandardVer = EL_NONSTANDARD_VER;     //非标号
}


/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
    #define PERIPH_BASE           ((uint32_t)0x40000000) 
    #define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
    #define RCC_BASE              (AHB1PERIPH_BASE + 0x3800)
    RCC AHB1 peripheral reset register (RCC_AHB1RSTR)
    Address offset: 0x10
    RCC AHB2 peripheral reset register (RCC_AHB2RSTR)
    Address offset: 0x14
    RCC AHB3 peripheral reset register (RCC_AHB3RSTR)
    Address offset: 0x18
    RCC APB1 peripheral reset register (RCC_APB1RSTR)
    Address offset: 0x20
    RCC APB2 peripheral reset register (RCC_APB2RSTR)
    Address offset: 0x24 
********************************************************************************/
#define ApplicationAddress    0x08000000
typedef  void (*pFunction)(void);
volatile pFunction Jump_To_Application;

void ElcLabelDeal(void)
{
    Uint32 JumpAddress = 0; 

    //伺服状态:1 运行状态;  3停止状态或故障状态
    if( STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN )
    {
        UNI_EleLabel.Label.ServoStatus = 0x0100;
    }
    else
    {
        UNI_EleLabel.Label.ServoStatus = 0x0300;                    
    }

    //烧写程序使能后,H0D05置1
    if(1 < UNI_EleLabel.Label.JumpFlag)
    {
        AuxFunCodeUnion.code.FA_EmergencyStop = 1;
    }
     
    //跳转到Boot区处理
    if(1 == UNI_EleLabel.Label.JumpFlag) 
    {
        //等待500ms后自动清零
        UNI_EleLabel.Label.DelayCnt ++;

        if(UNI_EleLabel.Label.DelayCnt > 500)
        {
            UNI_EleLabel.Label.JumpFlag = 0;
            UNI_EleLabel.Label.DelayCnt = 0;
        }
    }
    else if(4 == UNI_EleLabel.Label.JumpFlag)
    {
        //等待串口返回帧发送完成
        UNI_EleLabel.Label.DelayCnt ++;
        if(UNI_EleLabel.Label.DelayCnt > 200)
        {
            UNI_EleLabel.Label.DelayCnt = 0;
            UNI_EleLabel.Label.JumpFlag = 2;
        }    
    }
    else if(2 == UNI_EleLabel.Label.JumpFlag)
    {
        UNI_EleLabel.Label.DelayCnt ++;
        if(UNI_EleLabel.Label.DelayCnt > 100)
        {
            UNI_EleLabel.Label.DelayCnt = 0;
            UNI_EleLabel.Label.JumpFlag = 3;
        }    
    }                

    else if(3 == UNI_EleLabel.Label.JumpFlag)
    {
        if(EepromProcess() != 1) 
        {            
            DINT;   //关中断

            //关闭所有的外设            
            RCC->AHB1RSTR |= 0xFFFFFFFF;  //RCC AHB1 peripheral reset register (RCC_AHB1RSTR)
            RCC->AHB2RSTR |= 0xFFFFFFFF;  //RCC AHB2 peripheral reset register (RCC_AHB2RSTR)
            RCC->AHB3RSTR |= 0xFFFFFFFF;  //RCC AHB3 peripheral reset register (RCC_AHB3RSTR)
            RCC->APB1RSTR |= 0xFFFFFFFF;  //RCC APB1 peripheral reset register (RCC_APB1RSTR)
            RCC->APB2RSTR |= 0xFFFFFFFF;  //RCC APB2 peripheral reset register (RCC_APB2RSTR)
            RCC->AHB1RSTR &= 0;  //RCC AHB1 peripheral reset register (RCC_AHB1RSTR)
            RCC->AHB2RSTR &= 0;  //RCC AHB2 peripheral reset register (RCC_AHB2RSTR)
            RCC->AHB3RSTR &= 0;  //RCC AHB3 peripheral reset register (RCC_AHB3RSTR)
            RCC->APB1RSTR &= 0;  //RCC APB1 peripheral reset register (RCC_APB1RSTR)
            RCC->APB2RSTR &= 0;  //RCC APB2 peripheral reset register (RCC_APB2RSTR)

            SysTick->CTRL = 0;

            EXTI->IMR = 0;
            EXTI->EMR = 0;

            NVIC->ICER[0] = 0xFFFFFFFF;
            NVIC->ICER[1] = 0xFFFFFFFF;
            NVIC->ICER[2] = 0xFFFFFFFF;
            NVIC->ICER[3] = 0xFFFFFFFF;
            NVIC->ICER[4] = 0xFFFFFFFF;
            NVIC->ICER[5] = 0xFFFFFFFF;
            NVIC->ICER[6] = 0xFFFFFFFF;
            NVIC->ICER[7] = 0xFFFFFFFF;

            JumpAddress = *(volatile Uint32*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;
//            __set_MSP(*(volatile Uint32*) ApplicationAddress);
            Jump_To_Application();

            UNI_EleLabel.Label.JumpFlag = 5;
        }
    }
}



/********************************* END OF FILE *********************************/
