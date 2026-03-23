/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_I2cDriver_ST.c
 创建人：童文邹
 修改人：李浩               创建日期：11.11.14
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
#include "stm32f4xx.h"
#include "FUNC_GlobalVariable.h"

//#include "FUNC_GPIODriver.h"
#include "FUNC_GPIODriver.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_dma.h"
#include "FUNC_AuxFunCode.h"
#include "FUNC_I2cDriver_ST.h"
#include "FUNC_DMADriver.h"
#include "core_cm4.h"     //嘀嗒时钟配置使用20120227

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define I2C_REMAP       1                   // I2C是否是否重映射

#define I2Cx            I2C1                //ST有两个I2C即(I2C1和I2C2)，此程序中只使用I2C1

#define TIME_1US        (168UL<<8)           // 时钟为42MHz
#define TIME_1MS        (TIME_1US * 1000)

#define I2C_SPEED               160000      // I2C波特率    ,MSX 160000   ->200000
#define TIME_I2C_10_CLK         (1000000UL*10 / I2C_SPEED * TIME_1US)

#define RW_I2C_MAX                  10      // _*2ms，操作I2C的时间，即调用I2cRwDeal()函数的超时时间
#define RW_I2C_WRONG_DELAY_TIME     20      // _*2ms，操作I2C超时后，重新初始化I2C，延迟一段时间后，重新开始

#define I2C_READ    0    //IIC 读状态
#define I2C_WRITE   1    //IIC 写状态

#define DMA_DIRECTION_TX    0
#define DMA_DIRECTION_RX    1

#define EEPROM_I2C_DMA_CHANNEL_TX           DMA1_Stream6
#define EEPROM_I2C_DMA_CHANNEL_RX           DMA1_Stream0
#define EEPROM_I2C_DMA_CLK                  RCC_AHBPeriph_DMA1
#define EEPROM_I2C_DR_Address               ((Uint32)0x40005410)

/* For M24C32 and M24C64 devices, E0,E1 and E2 pins are all used for device
  address selection (ne need for additional address lines). According to the
  Harware connection on the board (on STM3210C-EVAL board E0 = E1 = E2 = 0) */
#define EEPROM_HW_ADDRESS      0xA0          /* E0 = E1 = E2 = 0 */ //Eeprom 硬件地址
#define I2C_SLAVE_ADDRESS7     0xA0
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

#define  GetTime()          ((SysTick->VAL) << 8)
#define  GetTimeEeprom()    GetTime()


//#if (!I2C_REMAP)        // 使用PB6,PB7
//#define SCL_H         (GPIOB->BSRR = GPIO_Pin_6)
//#define SCL_L         (GPIOB->BRR  = GPIO_Pin_6)
//
//#define SDA_H         (GPIOB->BSRR = GPIO_Pin_7)
//#define SDA_L         (GPIOB->BRR  = GPIO_Pin_7)
//
//#define SCL_READ      (GPIOB->IDR  & GPIO_Pin_6)
//#define SDA_READ      (GPIOB->IDR  & GPIO_Pin_7)
//#elif 1                 // PB8,PB9
#define SCL_H         (GPIOB->BSRRL = GPIO_Pin_8)
#define SCL_L         (GPIOB->BSRRH  = GPIO_Pin_8)

#define SDA_H         (GPIOB->BSRRL = GPIO_Pin_9)
#define SDA_L         (GPIOB->BSRRH  = GPIO_Pin_9)

#define SCL_READ      (GPIOB->IDR  & GPIO_Pin_8)
#define SDA_READ      (GPIOB->IDR  & GPIO_Pin_9)
//#endif

/* Private_TypesDefinitions --------------------------------------------------*/
/* 结构体变量定义 枚举类型定义 */
/* I2C的读写过程中的几个状态 */
typedef enum {

    I2C_RW_OK = 0,
    I2C_RW_DELAY_BEFORE_START = 1,                  // 开始之前延时5us
    I2C_RW_BUSY_CHECK = 2,                          // 检查I2C总线是否空闲
    I2C_RW_MASTER_MODE_SELECT = 3,                  // After sending START condition, Test on EV5 and clear it
    I2C_RW_MASTER_TRANSMITTER_MODE_SELECTED = 4,    // After Send EEPROM address for write, Test on EV6 and clear it
    I2C_RW_MASTER_TRANSMITTED_ADDRESS_HIGH = 5,     // After Send the EEPROM's internal address to write to : MSB of the address first, Test on EV8 and clear it
    I2C_RW_MASTER_TRANSMITTED_ADDRESS_LOW = 6,      // After Send the EEPROM's internal address to write to : LSB of the address, Test on EV8 and clear it
    I2C_WRITE_WAIT_DMA_END = 7,                     // 等待DMA传输完成
    I2C_WRITE_DMA_END = 8,                          // DMA传输已经完成
    I2C_WRITE_WAIT_END = 9,                         // 等待EEPROM内部写完成。发送命令，检查ACK信号
    I2C_WRITE_WAIT_END_STEP1 = 10,
    I2C_WRITE_WAIT_END_STEP2_DELAY_BEFORE = 11,
    I2C_WRITE_WAIT_END_STEP2 = 12,
    I2C_READ_MASTER_MODE_SELECT = 13,               // 读，restart
    I2C_READ_MASTER_TRANSMITTER_MODE_SELECTED = 14,
    I2C_READ_WAIT_DMA_END = 15,
    I2C_RW_WRONG_REPEAT = 16,                        // 操作I2C超时，重新初始化I2C，延时后重新开始
    I2C_RW_MASTER_MODE_STOP = 17                   //发送STOP命令
} ENU_I2C_RW_STATE;

/* Private_TypesDefinitions --------------------------------------------------*/
/* 结构体变量定义 枚举变量定义 */
DMA_InitTypeDef     STR_DMA_InitStructure_Tx;
DMA_InitTypeDef     STR_DMA_InitStructure_Rx;
I2C_InitTypeDef  I2C_InitStructure;
/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

__IO uint16_t EepromAddress = 0;  //Eeprom作为从设备的地址

Uint32 I2cTimeLast;
Uint32 I2cTimeWrong;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitI2c_ST(void);
Uint8 I2cRwDeal(Uint8* DataBuffer, Uint16 Address, Uint8 DataNum, Uint8 RwMode);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
void DMA_LowLevelConfig(Uint32 Buffer, Uint32 BufferSize, Uint32 Direction);
void InitEepromLowLevel(void);
void I2cDealBeforeInit(void);
void I2cDelay(void);
void I2cStop(void);
void I2cReceiveLastByte(void);

/*******************************************************************************
  函数名: void InitI2c_ST()
  输入:
  输出:
  子函数:  I2C操作的ST库中相应的库函数
  描述:   该函数提供给main()进行初始化调用
  返回值:
********************************************************************************/
void InitI2c_ST()
{
    if(SysTick_Config(0x00FFFFFF))   //LZ 20120226 FUNCSystem ??I2C???????             //msx+ I2C?????
    {
        /* Capture error */
        while (1);
    }


	I2cDealBeforeInit();

  InitEepromLowLevel();

  I2C_SoftwareResetCmd(I2Cx, ENABLE);
  I2C_SoftwareResetCmd(I2Cx, DISABLE);

  /*!< I2C configuration */
  /* I2Cx configuration */
  I2C_InitStructure.I2C_Mode        = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle   = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;   /* user parameter */
  I2C_InitStructure.I2C_Ack         = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed  = I2C_SPEED;             /* user parameter */

  /* I2Cx Peripheral Enable */
  I2C_Cmd(I2Cx, ENABLE);
  /* Apply I2Cx configuration after enabling it */
  I2C_Init(I2Cx, &I2C_InitStructure);

  /* Enable the I2Cx peripheral DMA requests */
  I2C_DMACmd(I2Cx, ENABLE);

  /*!< Select the EEPROM address according to the state of E0, E1, E2 pins */
  EepromAddress = EEPROM_HW_ADDRESS;

}
/*******************************************************************************
  函数名: Uint8 I2cRwDeal(Uint8* DataBuffer, Uint16 Address, Uint8 DataNum, Uint8 RwMode)
  输入:  读写缓冲数组地址，需要读写功能码对应的地址， 需要读写功能码的个数， 读写两种操作方式
  输出:  读写是否成功
  子函数: GetTimeEeprom()(由滴答时钟获取的时间计数值)，I2C操作的相应的库函数
  描述:  根据I2C操作的时序，进行I2C读取过程中不同状态下的处理
  返回值: RwSuccess
********************************************************************************/
Uint8 I2cRwDeal(Uint8* DataBuffer, Uint16 Address, Uint8 DataNum, Uint8 RwMode)
{
	static ENU_I2C_RW_STATE    I2cRwState;    //定义I2C读写过程状态的枚举型变量
    Uint8 RwSuccess = 0;
    switch (I2cRwState)
    {
        case I2C_RW_OK:
            I2cTimeLast = GetTimeEeprom();    // I2c读取过程的计时初始化
            I2cTimeWrong = I2cTimeLast;       // I2c读取Eeprom超时计时初始化
            I2cRwState = I2C_RW_DELAY_BEFORE_START;
            break;

        case I2C_RW_DELAY_BEFORE_START:  // 读写之前延时5us。STM32的I2C模块没有遵循I2C规范中的“bus free time”
            if ( (I2cTimeLast - GetTimeEeprom()) >= (TIME_1US * 5) )
            {
                I2cRwState = I2C_RW_BUSY_CHECK;
            }
            break;

        case I2C_RW_BUSY_CHECK:         // 检测I2C的总线上是否繁忙，若不繁忙则产生起始(Strat)状态，在起始条件发送完成后SB置1，之后将硬件自动置为主模式
            if (!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
            {
                I2C_GenerateSTART(I2Cx, ENABLE);
                I2cRwState = I2C_RW_MASTER_MODE_SELECT;
            }
            break;

        case I2C_RW_MASTER_MODE_SELECT: // 检测I2c的主模式是否已被选择(若被选择则发送Eeprom从设备的地址，以及主设备配置成发送器))
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
            {
                I2C_Send7bitAddress(I2Cx, EepromAddress, I2C_Direction_Transmitter);
                I2cRwState = I2C_RW_MASTER_TRANSMITTER_MODE_SELECTED;
            }
            break;

        case I2C_RW_MASTER_TRANSMITTER_MODE_SELECTED:  //检测I2c的主设备的发送器模式是否被选择(若被选择则发送所需读取的功能码对应的首地址高8位)
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
            {
                I2C_SendData(I2Cx, (Uint8)((Address & 0xFF00) >> 8));
                I2cRwState = I2C_RW_MASTER_TRANSMITTED_ADDRESS_HIGH;
            }
            break;

        case I2C_RW_MASTER_TRANSMITTED_ADDRESS_HIGH:  //检测I2c所需读取的功能码对应的首地址高8位是否发送完成(若完成则传送相应的低8位)
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            {
                I2C_SendData(I2Cx, (Uint8)(Address & 0x00FF));
                I2cRwState = I2C_RW_MASTER_TRANSMITTED_ADDRESS_LOW;
            }
            break;

        case I2C_RW_MASTER_TRANSMITTED_ADDRESS_LOW:  //检测I2c所需读取的功能码对应的首地址低8位是否发送完成(若完成则是根据是读取还是写入功能码的操作，进行相应的处理)
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            {
                if (I2C_READ == RwMode)   // 从Eeprom中读取功能码
                {
                    I2C_GenerateSTART(I2Cx, ENABLE);    //若是读取模式时，则重新产生起始条件
                    I2cRwState = I2C_READ_MASTER_MODE_SELECT;
                }
                else                      // 向Eeprom中写入功能码
                {
                    DMA_LowLevelConfig((Uint32)DataBuffer, DataNum, DMA_DIRECTION_TX);  /* Configure the DMA Tx Channel with the buffer address and the buffer size */
                    DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_TX,DMA_IT_FEIF6);
                    DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_TX,DMA_IT_DMEIF6);
                    DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_TX,DMA_IT_TEIF6);
                    DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_TX,DMA_IT_HTIF6);
                    DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_TX,DMA_IT_TCIF6);
                    DMA_Cmd(EEPROM_I2C_DMA_CHANNEL_TX, ENABLE);   /* Enable the DMA Tx Channel */
                    I2cRwState = I2C_WRITE_WAIT_DMA_END;
                }
            }
            break;

        case I2C_READ_MASTER_MODE_SELECT:  // 检测I2c在读取方式下的主模式是否已被选择(若被选择则发送Eeprom从设备的地址，以及主设备配置成接收器))
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
            {
                I2C_Send7bitAddress(I2Cx, EepromAddress, I2C_Direction_Receiver);
                I2cRwState = I2C_READ_MASTER_TRANSMITTER_MODE_SELECTED;
            }
            break;

        case I2C_READ_MASTER_TRANSMITTER_MODE_SELECTED:
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
            {
                DMA_LowLevelConfig((Uint32)DataBuffer, DataNum, DMA_DIRECTION_RX);    /* Configure the DMA Rx Channel with the buffer address and the buffer size */
                I2C_DMALastTransferCmd(I2Cx, ENABLE);   /* Inform the DMA that the next End Of Transfer Signal will be the last one */
                DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_RX,DMA_IT_FEIF0);
                DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_RX,DMA_IT_DMEIF0);
                DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_RX,DMA_IT_TEIF0);
                DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_RX,DMA_IT_HTIF0);
                DMA_ClearITPendingBit(EEPROM_I2C_DMA_CHANNEL_RX,DMA_IT_TCIF0);
                DMA_Cmd(EEPROM_I2C_DMA_CHANNEL_RX, ENABLE);   /* Enable the DMA Rx Channel */
                I2cRwState = I2C_READ_WAIT_DMA_END;
            }
            break;

        case I2C_READ_WAIT_DMA_END:
            if (!EEPROM_I2C_DMA_CHANNEL_RX->NDTR)     /* Check if the DMA transfer is complete */
            {
                DMA_Cmd(EEPROM_I2C_DMA_CHANNEL_RX, DISABLE);  /* Disable the DMA Rx Channel and Clear all its Flags */
                I2cRwState = I2C_RW_MASTER_MODE_STOP;
            }
            break;

        case I2C_WRITE_WAIT_DMA_END:
            if (!EEPROM_I2C_DMA_CHANNEL_TX->NDTR)   /* Check if the DMA transfer is complete */
            {
                DMA_Cmd(EEPROM_I2C_DMA_CHANNEL_TX, DISABLE);  /* Disable the DMA Tx Channel and Clear all its Flags */
                I2cRwState = I2C_WRITE_DMA_END;
            }
            break;

        case I2C_WRITE_DMA_END:
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            {
                I2C_GenerateSTOP(I2Cx, ENABLE);
                I2cTimeLast = GetTimeEeprom();
                I2cRwState = I2C_WRITE_WAIT_END;
            }
            break;

        case I2C_WRITE_WAIT_END:
            if (I2cTimeLast - GetTimeEeprom() >= (TIME_1US * 5))      // STOP条件产生之后，5us后将Stop标志位清楚，并产生Start条件，开始之前延时5us
            {
                if (I2Cx->CR1 & 0x200)
                {
                    I2Cx->CR1 &= 0xFDFF;
                }
                I2C_GenerateSTART(I2Cx, ENABLE);
                I2cRwState = I2C_WRITE_WAIT_END_STEP1;
            }
            break;

        case I2C_WRITE_WAIT_END_STEP1:
            if (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
            {
                //wzg Uint16 Temp = 0;
                /*wzg Temp = */I2C_ReadRegister(I2Cx, I2C_Register_SR1);
                /*wzg Temp = */I2C_ReadRegister(I2Cx, I2C_Register_SR2);
                I2C_Send7bitAddress(I2Cx, EepromAddress, I2C_Direction_Transmitter);
                I2cTimeLast = GetTimeEeprom();

                I2cRwState = I2C_WRITE_WAIT_END_STEP2_DELAY_BEFORE;

            }
            break;

        case I2C_WRITE_WAIT_END_STEP2_DELAY_BEFORE:
            if (I2cTimeLast - GetTimeEeprom() >= TIME_I2C_10_CLK)     // 等到第九个时钟脉冲(对应ACK或者NACK)之后再查询ADDR bit
            {
                I2cRwState = I2C_WRITE_WAIT_END_STEP2;
            }
            break;

        case I2C_WRITE_WAIT_END_STEP2:
            if ((I2C_ReadRegister(I2Cx, I2C_Register_SR1) & 0x0002))
            {
                I2C_ClearFlag(I2Cx, I2C_FLAG_AF);    /*!< Clear AF flag */
                I2cRwState = I2C_RW_MASTER_MODE_STOP;
            }
            else
            {
                I2cTimeLast = GetTimeEeprom();
                I2cRwState = I2C_WRITE_WAIT_END;
            }
            break;

        case I2C_RW_WRONG_REPEAT:   // 发生错误，重新初始化I2C，延时后重新开始读写
            if (I2cTimeWrong - GetTimeEeprom() >= (TIME_1MS * 2 * RW_I2C_WRONG_DELAY_TIME))
            {
                if((I2C_READ == RwMode))
                {
                    I2C_GenerateSTART(I2Cx, ENABLE);
                    I2cRwState = I2C_RW_MASTER_MODE_STOP;
                }
                else    I2cRwState = I2C_RW_OK;
            }
            break;

        case I2C_RW_MASTER_MODE_STOP:
            
			
			I2C_GenerateSTOP(I2Cx, ENABLE);     /*!< Send STOP Condition */
            DELAY_US(10);
            if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
            {
                AuxFunCodeUnion.code.DP_Rsvd63++;
				if(I2C_READ == RwMode)
                {
                    I2cReceiveLastByte();
                    I2cTimeWrong = GetTimeEeprom();
                    I2cRwState = I2C_RW_WRONG_REPEAT;
                }
            }
            else
            {
                RwSuccess = 1;
                I2cRwState = I2C_RW_OK;
            }
            break;

        default:
            break;
    }
    /*操作I2C总线超时,故障产生*/
    if ( (I2cRwState != I2C_RW_OK) &&
         (I2cRwState != I2C_RW_WRONG_REPEAT) &&
         (I2cTimeWrong - GetTimeEeprom() >= (TIME_1MS * 6 * RW_I2C_MAX))
       )
    {
        InitI2c_ST();  // 重新初始化I2C模块
        I2cTimeWrong = GetTimeEeprom();
        I2cRwState = I2C_RW_WRONG_REPEAT;
    }
	return RwSuccess;
}
/*******************************************************************************
  函数名:  void DMA_LowLevelConfig(Uint32 Buffer, Uint32 BufferSize, Uint32 Direction)
  输入:
  输出:
  子函数: DMA_Init()(DMA库函数中的初始化函数)
  描述:  I2C在读写过程中，功能码数据的传输是采用DMA方式的，因此需要对DMA进行配置
  返回值:

********************************************************************************/

void DMA_LowLevelConfig(Uint32 Buffer, Uint32 BufferSize, Uint32 Direction)
{
    /* Initialize the DMA with the new parameters */
    if (Direction == DMA_DIRECTION_TX)
    {
        /* Configure the DMA Tx Channel with the buffer address and the buffer size */
        STR_DMA_InitStructure_Tx.DMA_Memory0BaseAddr = (Uint32)Buffer;
        STR_DMA_InitStructure_Tx.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        STR_DMA_InitStructure_Tx.DMA_BufferSize = (Uint32)BufferSize;
        DMA_Init(DMA1_Stream6, &STR_DMA_InitStructure_Tx);
    }
    else
    {
        /* Configure the DMA Rx Channel with the buffer address and the buffer size */
        STR_DMA_InitStructure_Rx.DMA_Memory0BaseAddr = (Uint32)Buffer;
        STR_DMA_InitStructure_Rx.DMA_DIR = DMA_DIR_PeripheralToMemory;
        STR_DMA_InitStructure_Rx.DMA_BufferSize = (Uint32)BufferSize;        
        DMA_Init(DMA1_Stream0, &STR_DMA_InitStructure_Rx);
    }
}


/*******************************************************************************
  函数名: void I2cDealBeforeInit()
  输入:
  输出:
  子函数:
  描述: 在EEPROM初始化之前，EEPROM有可能占用I2C总线，直接初始化不会成功。
        执行本函数之后，I2C总线由DSP控制。
  返回值:
********************************************************************************/

void I2cDealBeforeInit()
{
    Uint8 Cnt  = 0;      //循环变量
    Uint8 Temp = 0;

	GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //PB8  SCL   //PB9 SDA   ，初始配置作为普通IO，不作为外设
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	     //普通输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    //无上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);			   //管脚初始化配置


    SCL_L;
    DELAY_US(2);
    SDA_H;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    SDA_L;
    DELAY_US(2);
    SCL_L;
    DELAY_US(2);
    SDA_H;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    for (Cnt=0; Cnt < 9; Cnt++)
    {
        SCL_L;
        DELAY_US(2);
        SCL_H;
        DELAY_US(2);
    }
    SDA_L;
    DELAY_US(2);
    SCL_L;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    SDA_H;
    DELAY_US(2);
    SDA_L;
    DELAY_US(2);
    //software reset i2c end
    
    SCL_L;
    DELAY_US(2);
    SDA_H;
    DELAY_US(2);
    for (Cnt=0; Cnt < 10; Cnt++)
    {
        if (SDA_READ)
        {
            SDA_L;
            DELAY_US(2);
            if (!SDA_READ)     // SDA被CPU控制，说明已经是ACK/NACK位
            {
                SDA_H;         // 给NACK
                DELAY_US(2);
                Temp = 1;      // 退出
            }
        }
        SCL_H;
        DELAY_US(2);
        SCL_L;
        DELAY_US(2);
        if (Temp == 1)
            break;
    }
    I2cStop();
}

/*******************************************************************************
  函数名: void InitEepromLowLevel()
  输入:
  输出:
  子函数:
  描述: 主要对I2C管脚，时钟，以及DMA等进行初始化
  返回值:
********************************************************************************/

void InitEepromLowLevel()
{
	GPIO_InitTypeDef   GPIO_InitStructure;
    	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); /*!< RCC_APB2Periph_GPIOB Periph clock enable */

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);  /*!< I2C1 Periph clock enable */

    //IIC2复用功能映射到AF4
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);
    
    //PB8  SCL   //PB9 SDA   ，作为外设管脚配置为复用功能
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	     //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    //无上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);			   //管脚初始化配置


  /*!< I2C DMA TX and RX channels configuration */
  /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	DMA_DeInit((DMA_Stream_TypeDef*) DMA1_Stream0_BASE);
    DMA_DeInit((DMA_Stream_TypeDef*) DMA1_Stream6_BASE);
    DMA_Cmd(DMA1_Stream0, DISABLE);
    DMA_Cmd(DMA1_Stream6, DISABLE);
    //配置I2C1  DMA1 STREAM0 channel1 配置为I2C_RX
    STR_DMA_InitStructure_Rx.DMA_Channel = DMA_Channel_1; 
	STR_DMA_InitStructure_Rx.DMA_PeripheralBaseAddr = EEPROM_I2C_DR_Address;
	STR_DMA_InitStructure_Rx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	STR_DMA_InitStructure_Rx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	STR_DMA_InitStructure_Rx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	STR_DMA_InitStructure_Rx.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	STR_DMA_InitStructure_Rx.DMA_Mode = DMA_Mode_Normal; 
	STR_DMA_InitStructure_Rx.DMA_Priority = DMA_Priority_High;
	STR_DMA_InitStructure_Rx.DMA_FIFOMode = DMA_FIFOMode_Disable;        
	STR_DMA_InitStructure_Rx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	STR_DMA_InitStructure_Rx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream0, &STR_DMA_InitStructure_Rx);

    //配置I2C1  DMA1 STREAM6 channel1 配置为I2C_TX
    STR_DMA_InitStructure_Tx.DMA_Channel = DMA_Channel_1; 
	STR_DMA_InitStructure_Tx.DMA_PeripheralBaseAddr = EEPROM_I2C_DR_Address;
	STR_DMA_InitStructure_Tx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	STR_DMA_InitStructure_Tx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	STR_DMA_InitStructure_Tx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	STR_DMA_InitStructure_Tx.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	STR_DMA_InitStructure_Tx.DMA_Mode = DMA_Mode_Normal; 
	STR_DMA_InitStructure_Tx.DMA_Priority = DMA_Priority_High;
	STR_DMA_InitStructure_Tx.DMA_FIFOMode = DMA_FIFOMode_Disable;        
	STR_DMA_InitStructure_Tx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	STR_DMA_InitStructure_Tx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &STR_DMA_InitStructure_Tx);

}


/*******************************************************************************
  函数名:  void I2cDelay()
  输入:
  输出:
  子函数:
  描述:
  返回值:

********************************************************************************/
void I2cDelay()
{
    Uint8 Count = 64;     // 这里可以优化速度，经测试最低到5还能写入
    while (Count > 0)
    {
        Count--;
    }
}

/*******************************************************************************
  函数名:  void I2cStop()
  输入:
  输出:
  子函数:
  描述:
  返回值:

********************************************************************************/
void I2cStop()
{
    SCL_L;
    DELAY_US(2);
    SDA_L;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    SDA_H;
    DELAY_US(2);
}


/*******************************************************************************
  函数名: void I2cReceiveLastByte()
  输入:
  输出:
  子函数:
  描述: 
  返回值:
********************************************************************************/

void I2cReceiveLastByte()
{
    Uint8 Cnt  = 0;      //循环变量

	GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //PB8  SCL   //PB9 SDA   ，初始配置作为普通IO，不作为外设
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	     //普通输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    //无上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);			   //管脚初始化配置

    //外部硬件上上拉
    DELAY_US(2);
    SCL_L;
    DELAY_US(2);
    
    while(Cnt<7)
    {
        SCL_H;
        DELAY_US(2);
        SCL_L;
        DELAY_US(2);
        Cnt++;
    }

    //置NACK
    SDA_H;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    
    //Set Stop
    SCL_L;
    DELAY_US(2);
    SDA_L;
    DELAY_US(2);
    SCL_H;
    DELAY_US(2);
    SDA_H;
    
    InitEepromLowLevel();
//    I2C_SoftwareResetCmd(I2Cx, ENABLE);
//    I2C_SoftwareResetCmd(I2Cx, DISABLE);
//    
//    /*!< I2C configuration */
//    /* I2Cx configuration */
//    I2C_InitStructure.I2C_Mode        = I2C_Mode_I2C;
//    I2C_InitStructure.I2C_DutyCycle   = I2C_DutyCycle_2;
//    I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;   /* user parameter */
//    I2C_InitStructure.I2C_Ack         = I2C_Ack_Enable;
//    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
//    I2C_InitStructure.I2C_ClockSpeed  = I2C_SPEED;             /* user parameter */
    /* I2Cx Peripheral Enable */
    I2C_Cmd(I2Cx, ENABLE);
    /* Apply I2Cx configuration after enabling it */
    I2C_Init(I2Cx, &I2C_InitStructure);

    /* Enable the I2Cx peripheral DMA requests */
    I2C_DMACmd(I2Cx, ENABLE);
}


//********************************* END OF FILE *********************************

