/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_SpiDriver_ST.c
 创建人：童文邹                
 修改人：李浩               创建日期：11.11.28 
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
#include "FUNC_SpiDriver_ST.h"
#include "FUNC_GPIODriver.h"
#include "FUNC_DMADriver.h"
#include "stm32f4xx_gpio.h"
#include "FUNC_InterfaceProcess.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

#define SPI_BUFFER_SIZE    3

#define SPI_MASTER_DR_BASE            0x4000380C


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

STR_FUNC_SPI_Def * FUNC_SPI1  =  (STR_FUNC_SPI_Def *)(0x40013000);
STR_FUNC_SPI_Def * FUNC_SPI2  =  (STR_FUNC_SPI_Def *)(0x40003800);
STR_FUNC_SPI_Def * FUNC_SPI3  =  (STR_FUNC_SPI_Def *)(0x40003C00);

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
UNI_SPI_DI   UNI_SpiDiReg = {0};
UNI_SPI_DO   UNI_SpiDoReg = {0};

/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */
Uint8  Spi_SlaveBuffer_Rx[SPI_BUFFER_SIZE];
Uint8  Spi_SlaveBuffer_Tx[SPI_BUFFER_SIZE];

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitSpi(void); 
void SpiRxTxScan(Uint16 Step);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
#if POWERDRIVER_TYPE==POWDRV_IS650
extern void DiodDisp(Uint8* pTubeSel,Uint8* pDispData);
#endif

/*******************************************************************************
  函数名:  void InitSpi() 
  输入:            
  输出:    
  子函数: GPIO_Init(); SPI_Init(); DMA_Init(); SPI_I2S_DMACmd();SPI_Cmd(); DMA_Cmd()
  描述: 初始化SPI，包含初始化SPI的端口GPIO，以及初始化SPI接收发送所使用的DMA  
  返回值: 
********************************************************************************/ 
void InitSpi()
{
    //配置SPI2 pins: SCK, MISO and MOSI
	FUNC_GPIOB->AFRH.bit.AFR13        = GPIO_AF_SPI2;
	FUNC_GPIOB->MODER.bit.MODER13     = FUNC_GPIO_Mode_AF;
	FUNC_GPIOB->OTYPER.bit.OT13       = FUNC_GPIO_OType_PP;
	FUNC_GPIOB->OSPEEDR.bit.OSPEEDR13 = FUNC_GPIO_Speed_50MHz;

	FUNC_GPIOB->AFRH.bit.AFR14        = GPIO_AF_SPI2;
    FUNC_GPIOB->MODER.bit.MODER14     = FUNC_GPIO_Mode_AF;
	FUNC_GPIOB->OTYPER.bit.OT14       = FUNC_GPIO_OType_PP;
	FUNC_GPIOB->OSPEEDR.bit.OSPEEDR14 = FUNC_GPIO_Speed_50MHz;
	
	
	FUNC_GPIOB->AFRH.bit.AFR15        = GPIO_AF_SPI2;
	FUNC_GPIOB->MODER.bit.MODER15     = FUNC_GPIO_Mode_AF;
	FUNC_GPIOB->OTYPER.bit.OT15       = FUNC_GPIO_OType_PP;
	FUNC_GPIOB->OSPEEDR.bit.OSPEEDR15 = FUNC_GPIO_Speed_50MHz;

	//配置SPI2
    FUNC_SPI2->CR1.bit.BR       = 6 ;
    FUNC_SPI2->CR1.bit.CPOL     = 0 ;
    FUNC_SPI2->CR1.bit.CPHA     = 0 ;
    FUNC_SPI2->CR1.bit.DFF      = 0 ;
    FUNC_SPI2->CR1.bit.LSBFIRST = 0;
    FUNC_SPI2->CR1.bit.SSI      = 1 ;
    FUNC_SPI2->CR1.bit.SSM      = 1 ;
	FUNC_SPI2->CR2.bit.FRF      = 0 ;
	FUNC_SPI2->CR1.bit.BIDIMODE = 0;
	FUNC_SPI2->CR1.bit.RXONLY   = 0;
	FUNC_SPI2->CR1.bit.BIDIOE   = 0;
    FUNC_SPI2->CR1.bit.MSTR = 1;

    FUNC_SPI2->CRCPR.bit.CRCPOLY = 7;

    /*以下是DMA1通道3相关寄存器初始化复位*/
    FUNC_DMA1->Channel3_SCR.bit.EN = 0;   //DMA1通道3选择不使能
	while(FUNC_DMA1->Channel3_SCR.bit.EN)
	{}
    FUNC_DMA1->Channel3_SCR.all = 0;      //复位DMA1通道3控制寄存器
    FUNC_DMA1->Channel3_SNDTR.all = 0;    //复位DMA1通道3的传输数量寄存器
    FUNC_DMA1->Channel3_SPAR.all = 0;     //复位DMA1通道3的外设地址寄存器
    FUNC_DMA1->Channel3_SM0AR.all = 0;    //复位DMA1通道3的存储器地址寄存器
    /*以下是清除DMA1通道3的各种中断标志位*/
   	FUNC_DMA1_IT_REG->LIFCR.all=0;
    FUNC_DMA1_IT_REG->LIFCR.bit.CTCIF3 = 1;
    FUNC_DMA1_IT_REG->LIFCR.bit.CHTIF3 = 1;
    FUNC_DMA1_IT_REG->LIFCR.bit.CTEIF3 = 1;
	FUNC_DMA1_IT_REG->LIFCR.bit.CFEIF3 = 1;
    FUNC_DMA1_IT_REG->LIFCR.bit.CDMEIF3= 1;

   
    /*以下是DMA1 通道 3 相关寄存器初始化,并将该通道配置为SPI数据接收*/
    FUNC_DMA1->Channel3_SPAR.all = (unsigned int)SPI_MASTER_DR_BASE;
    FUNC_DMA1->Channel3_SM0AR.all = (unsigned int)Spi_SlaveBuffer_Rx;
    FUNC_DMA1->Channel3_SCR.bit.DIR = 0; //从外设读
	FUNC_DMA1->Channel3_SCR.bit.CT  = 0; 
    FUNC_DMA1->Channel3_SNDTR.all = SPI_BUFFER_SIZE;
    FUNC_DMA1->Channel3_SCR.bit.PINC = 0;  //不执行外设地址增量操作
    FUNC_DMA1->Channel3_SCR.bit.MINC = 1;  //执行存储器地址增量操作
    FUNC_DMA1->Channel3_SCR.bit.PSIZE = 0; //外设数据宽度为8位宽度
    FUNC_DMA1->Channel3_SCR.bit.MSIZE = 0; //外设数据宽度为8位宽度
    FUNC_DMA1->Channel3_SCR.bit.CIRC = 0;  //不执行循环操作
	FUNC_DMA1->Channel3_SCR.bit.DBM  = 0;  //
	FUNC_DMA1->Channel3_SCR.bit.CHSEL= 0;  //选择SPI_RX
	FUNC_DMA1->Channel3_SCR.bit.PFCTRL= 0;  
  
    FUNC_DMA1->Channel3_SCR.bit.PL = 0;    //低优先级
	FUNC_DMA1->Channel3_SFCR.bit.DMDIS = 0;//直接模式
	FUNC_DMA1->Channel3_SFCR.bit.FEIE =0;


    //DMA1 通道 4 相关寄存器初始化，并将该通道配置为SPI数据发送
    /*以下是DMA1通道5相关寄存器初始化复位*/
    FUNC_DMA1->Channel4_SCR.bit.EN = 0;   //DMA1通道4选择不使能
	while(FUNC_DMA1->Channel4_SCR.bit.EN)
	{}
    FUNC_DMA1->Channel4_SCR.all    = 0;      //复位DMA1通道4控制寄存器
    FUNC_DMA1->Channel4_SNDTR.all  = 0;    //复位DMA1通道4的传输数量寄存器
    FUNC_DMA1->Channel4_SPAR.all   = 0;    //复位DMA1通道4的外设地址寄存器
    FUNC_DMA1->Channel4_SM0AR.all  = 0;   //复位DMA1通道4的存储器地址寄存器
    /* 以下是清除DMA1通道4的各种中断标志位 */
	FUNC_DMA1_IT_REG->HIFCR.all=0;
    FUNC_DMA1_IT_REG->HIFCR.bit.CTCIF4 = 1;
    FUNC_DMA1_IT_REG->HIFCR.bit.CHTIF4 = 1;
    FUNC_DMA1_IT_REG->HIFCR.bit.CTEIF4 = 1;
	FUNC_DMA1_IT_REG->HIFCR.bit.CFEIF4 = 1;
    FUNC_DMA1_IT_REG->HIFCR.bit.CDMEIF4= 1;

    /*以下是DMA1 通道 4 相关寄存器初始化，并将该通道配置为SPI数据发送*/
    FUNC_DMA1->Channel4_SPAR.all = (unsigned int)SPI_MASTER_DR_BASE;
    FUNC_DMA1->Channel4_SM0AR.all = (unsigned int)Spi_SlaveBuffer_Tx;
    FUNC_DMA1->Channel4_SCR.bit.DIR = 0x01;  //从存储器读
    FUNC_DMA1->Channel4_SNDTR.all = SPI_BUFFER_SIZE;
	FUNC_DMA1->Channel4_SCR.bit.DBM  = 0;  //
    FUNC_DMA1->Channel4_SCR.bit.PINC = 0;  //不执行外设地址增量操作
    FUNC_DMA1->Channel4_SCR.bit.MINC = 1;  //执行存储器地址增量操作
    FUNC_DMA1->Channel4_SCR.bit.PSIZE = 0; //外设数据宽度为8位宽度
    FUNC_DMA1->Channel4_SCR.bit.MSIZE = 0; //外设数据宽度为8位宽度
    FUNC_DMA1->Channel4_SCR.bit.CIRC = 0;  //不执行循环操作
	FUNC_DMA1->Channel4_SCR.bit.CHSEL= 0;  //选择SPI_TX
	FUNC_DMA1->Channel4_SCR.bit.PFCTRL= 0;
	FUNC_DMA1->Channel4_SCR.bit.PL = 2;    //高优先级

	FUNC_DMA1->Channel4_SFCR.bit.DMDIS = 0;//直接模式
	FUNC_DMA1->Channel4_SFCR.bit.FEIE =0;


    SPI2_SSOutputEnable();
    SPI2_SSOutputDisable();  //不用NSS管脚

    SPI2_I2S_DMA_RX_Enable();  //RXDMAEN=1  ，启动接收缓冲区DMA 
    SPI2_I2S_DMA_Tx_Enable();  //TXDMAEN=1  ，启动发送缓冲区DMA
 
    SPI2_Enable();  //使能SPI2

    FUNC_DMA1->Channel3_SNDTR.all = 0;
    FUNC_DMA1->Channel4_SNDTR.all = 0;

	DMA1_Channel3_Enable();   //EN =1，Enable DMA1 Channel3 使能DMA接收通道，暂时不使能DMA发送通道
}


/*******************************************************************************
  函数名: void SpiRxTxScan(Uint16 Step)
  输入:  UNI_SpiDoReg.all；STR_FUNC_Gvar.SpiFromPanel.LedTubeSel；STR_FUNC_Gvar.SpiFromPanel.LedDisplayCode;                 
  输出:  UNI_SpiDiReg.all；STR_FUNC_Gvar.SpiFromPanel.InnerKey ;STR_FUNC_Gvar.SpiFromPanel.InnerKey
  子函数: GPIO_WriteSPICSRL()
  描述:   Spi的接收和发送数据更新函数
  返回值: 
********************************************************************************/ 
void SpiRxTxScan(Uint16 Step)
{
	if(Step == SPI_RX_STEP )
    {                
		GPIO_ResetSPICSRL();   //将SN74HC165移位寄存器的R/L信号置低，锁存数据，当启动SPI传输接收时再置高, R/L信号上升沿锁存数据，R/L信号为高时每一个SPICLK周期移一位到MISO
        
    #if POWERDRIVER_TYPE==POWDRV_IS650
        /*CPU读上次接收*/
		UNI_SpiDiReg.all = Spi_SlaveBuffer_Rx[0];
        STR_FUNC_Gvar.SpiDrvOutput.InnerKey = 0xFF - Spi_SlaveBuffer_Rx[1];
        STR_FUNC_Gvar.SpiDrvOutput.OuterKey = 0xFF - Spi_SlaveBuffer_Rx[2];

        /* 通过这里判断是否有外置按键有效，原因无外置按键，读取的值SpiOutDataIn.all为有效，和实际不符 */
        STR_FUNC_Gvar.SpiDrvOutput.OuterKey = (STR_FUNC_Gvar.SpiDrvOutput.OuterKey == 0x0000)? 0xffff : STR_FUNC_Gvar.SpiDrvOutput.OuterKey;		
    #else 		
        /*CPU读上次接收*/
        UNI_SpiDiReg.all = Spi_SlaveBuffer_Rx[0];
        STR_FUNC_Gvar.SpiDrvOutput.InnerKey = Spi_SlaveBuffer_Rx[1];
        STR_FUNC_Gvar.SpiDrvOutput.OuterKey = Spi_SlaveBuffer_Rx[2];

        /* 通过这里判断是否有外置按键有效，原因无外置按键，读取的值SpiOutDataIn.all为有效，和实际不符 */
        STR_FUNC_Gvar.SpiDrvOutput.OuterKey = (STR_FUNC_Gvar.SpiDrvOutput.OuterKey == 0x0000)? 0xffff : STR_FUNC_Gvar.SpiDrvOutput.OuterKey;
    #endif

	}
    else if(Step == SPI_TX_STEP )
    {	
		/* 启动本次发送、接收 */
        if (!FUNC_DMA1->Channel4_SNDTR.all)
        {
			GPIO_SetSPICSRL();   //将SN74HC165移位寄存器的R/L信号置高，将锁存的数据串行输出 	
			DMA1_Channel3_Disable();                //屏蔽DMA_13接收
            while(FUNC_DMA1->Channel3_SCR.bit.EN)
	        {}
			FUNC_DMA1_IT_REG->LIFCR.all = 0xF400000;//清除DMA发送中断标志位

			DMA1_Channel4_Disable();                //屏蔽DMA_14发送
			while(FUNC_DMA1->Channel4_SCR.bit.EN)
	        {}
            FUNC_DMA1_IT_REG->HIFCR.all =0x3D;      //清除DMA接收中断标志位

        #if POWERDRIVER_TYPE==POWDRV_IS650 
			DiodDisp(&Spi_SlaveBuffer_Tx[0],&Spi_SlaveBuffer_Tx[1]);
			Spi_SlaveBuffer_Tx[2] = (UNI_SpiDoReg.all >> 8) & 0x00ff;      //当前DO输出  					
		#else
            /* 要发3次 , 0->数码管段选择 1->显示编码 2->DO */
            Spi_SlaveBuffer_Tx[0] = STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelTubeSel;   //五个LED的数码管的选择输出

            //STO处理
            if(1 == UNI_FUNC_MTRToFUNC_FastList_16kHz.List.StatusFlag.bit.STOState)
            {
                Spi_SlaveBuffer_Tx[0] |= 0x80;
            }
            else
            {
                Spi_SlaveBuffer_Tx[0] &= 0x7F;
            }
            Spi_SlaveBuffer_Tx[1] = STR_FUNC_Gvar.ManageFunCodeOutput.TxPanelDispData;  //当前LED的数码管显示的示数输出
            Spi_SlaveBuffer_Tx[2] = (UNI_SpiDoReg.all >> 8) & 0x00ff;                   //当前DO输出
        #endif            
                      
            FUNC_DMA1->Channel3_SNDTR.all = 0x03;
            FUNC_DMA1->Channel4_SNDTR.all = 0x03;
    
            DMA1_Channel3_Enable();   //使能DMA_13接收
            DMA1_Channel4_Enable();   //使能DMA_14发送 
        }
    }
}
/********************************* END OF FILE *********************************/


