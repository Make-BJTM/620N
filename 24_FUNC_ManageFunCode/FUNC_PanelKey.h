/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_PanelKey.h
 创建人：童文邹                创建日期：2008.10
 修改人：王治国                修改日期：2011.11.09
 描述：
       1. 本文件定义了面板操作的结构体及初始化宏定义
       2. 如果按键电路有变化,请在本文件中声明,并更改STR_SPIRXKEY_Bit位结构体
       3. 本文声明了ServoPanel结构体变量,该变量时面板功能内部全局变量
 缩写：
       1. 
 修改记录：  
    1 xx.xx.xx      XX  
       变更内容： xxxxxxxxxxx
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx 
********************************************************************************/
#ifndef FUNC_PANELKEY_H
#define FUNC_PANELKEY_H 

#ifdef __cplusplus
extern "C" {
#endif   

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  
#include "PUB_GlobalPrototypes.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */
//按键电路板宏定义
#define  S5AS5R5ID1_VERB   1

#define  ACCLEVEL1ST    127        //Up键和Down键加速等级1，512ms/4ms = 128
#define  ACCLEVEL2ND    31         //Up键和Down键加速等级2，128ms/4ms = 32
#define  ACCLEVEL3RD    7          //Up键和Down键加速等级3，32ms/4ms = 8

#define  LONG_PRESS_TIME        128         //Shift键长按延迟时间  512ms/4ms = 128


//KEYSCAN结构体默认值
#define PANELOPERRATION_DEFAULT  {                  \
    0xFF,           /*输入信号*/                    \
    0xFF,           /*经滤波后的输入信号*/          \
    NONEKEY,        /*当前键扫描的值*/              \
    INVALIDEDGE,    /*当前键扫描的状态*/            \
    0,              /*MODE键滤波计数器*/            \
    0,              /*UP键滤波计数器*/              \
    0,              /*DOWN键滤波计数器*/            \
    0,              /*SHIFT键滤波计数器*/           \
    0,              /*SET键滤波计数器*/             \
    0,              /*UP键和DOWN键加速计数器*/      \
    ACCLEVEL1ST,    /*UP键和DOWN键加速等级*/        \
    0,              /*UP键和DOWN键加速等级计数器*/  \
    0,              /*面板菜单控制寄存器*/          \
    0,              /*显示功能码组*/                \
    0,              /*显示功能码组内偏移*/          \
    0,              /*LED数码管显示数据*/           \
    0,              /*用户密码*/                    \
    0xaf,0x86,0x92,0x86,0x87,    /*reset*/          \
                    /*LED数码管显示段数据*/         \
    0,              /*控制寄存器*/                  \
    0,              /*定时器,闪烁时使用*/           \
    0,              /*当前显示的数码管序号*/        \
    0,              /*发送当前显示的数码管使能*/    \
    0,              /*发送当前显示的数码管数据*/    }


//面板菜单等级定义
#define    CLASS_ZERO  0
#define    CLASS_1ST   1
#define    CLASS_2ND   2
#define    CLASS_3RD   3
#define    CLASS_4TH   4
#define    CLASS_5TH   5
#define    CLASS_6TH   6
#define    CLASS_7TH   7
//#define    CLASS_8TH   8

//面板菜单页数定义
#define    PAGE_ZERO   0
#define    PAGE_1ST    1
#define    PAGE_2ND    2
#define    PAGE_3RD    3

//面板第三级菜单显示模式
#define    NORMALMODE       0          //可以更改并存储在Eeprom
#define    READONLY         1          //只读属性,不可以更改
#define    USERLOCK         2          //用户密码保护,不可以更改
#define    RUNREAD          3          //停机设定属性,伺服运行状态,不可以更改
#define    USERPROTECT      4          //加密状态
#define    USERDECRYPT_SW   5          //解密过程切换状态
#define    USERDECRYPT      6          //解密状态


//面板当前处理的位
#define    DEALBIT_ZERO   0
#define    DEALBIT_1ST    1
#define    DEALBIT_2ND    2
#define    DEALBIT_3RD    3
#define    DEALBIT_4TH    4
#define    DEALBIT_NONE    0xF
/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */    

/* Exported_Types ------------------------------------------------------------*/ 
/* 常规类型定义 */   

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
//键值定义     
typedef enum{
    MODEKEY = 0,
    UPKEY = 1,
    DOWNKEY = 2,    
    SHIFTKEY = 3,
    LONG_SHIFTKEY = 4,
    SETKEY = 5,
    LONG_SETKEY = 6,
    UPDOWNKEY = 7,
    NONEKEY = 8,
}ENU_KEYVAULE; 

//键状态定义     
typedef enum{
    INVALIDEDGE = 0,
    VALIDEDGE = 1,
}ENU_KEYEDGESTATUS;

//SPI接收字符位定义 
typedef struct{
//按键电路有更改,请定义相关宏并在下面添加位定义
#if POWERDRIVER_TYPE==POWDRV_IS650
    Uint8 Rsvd:3;
    //按键位定义  低电平有效
    Uint8 Mode:1;
    Uint8 Down:1;
	Uint8 Set:1;
    Uint8 Shift:1;
	Uint8 Up:1;
#else 
    Uint8 Rsvd:3;
    //按键位定义  低电平有效
    Uint8 Mode:1;
    Uint8 Set:1;
    Uint8 Up:1;
    Uint8 Down:1;
    Uint8 Shift:1;
#endif
}STR_SPIRXKEY_Bit; 


typedef union{
    volatile Uint8              all;
    volatile STR_SPIRXKEY_Bit    bit; 
}UNI_SPIRXKEY_REG; 

//扫描按键结构体类型定义
typedef struct{
    volatile  UNI_SPIRXKEY_REG    Input;                  //输入信号
    volatile UNI_SPIRXKEY_REG    FltrInput;              //经滤波后的输入信号

    ENU_KEYVAULE        CurrKey;                //当前键扫描的值
    ENU_KEYEDGESTATUS   KeyEdge;                //当前键边沿状态

    Uint8           ModeFltrCnt;            //MODE键滤波计数器
    Uint8           UpFltrCnt;              //UP键滤波计数器
    Uint8           DownFltrCnt;            //DOWN键滤波计数器
    Uint8           ShiftFltrCnt;           //SHIFT键滤波计数器
    Uint8           SetFltrCnt;             //SET键滤波计数器

    Uint8           AccCnt;                 //UP键和DOWN键加速计数器
    Uint8           AccLevel;               //UP键和DOWN键加速等级
    Uint8           AccLevelCnt;            //UP键和DOWN键加速等级计数器
}STR_KEYSCAN;

//面板菜单控制位定义
typedef struct{
    Uint16  ErrShowSure:1;          //故障确认标志位
    Uint16  Class:4;                //面板菜单等级
    Uint16  Page:2;                 //面板菜单显示页数
    Uint16  Class3rdMode:4;         //面板第三级菜单显示模式
    Uint16  DealBit:4;              //面板正在操作的LED数码管位 
    Uint16  Class2ndDisp_SAVE:1;    //第二级菜单显示SAVE标志位 0 不显示  1 显示
}STR_PANELMENUCTRL_BIT; 

typedef union{
    volatile Uint16               all;
    volatile STR_PANELMENUCTRL_BIT   bit; 
}UNI_PANELMENUCTRL_REG;

//面板菜单结构体类型定义
typedef struct{
    volatile UNI_PANELMENUCTRL_REG       CtrlReg;                //面板菜单控制寄存器
    int8                        DispGroup;              //显示功能码组
    int8                        DispOffset;             //显示功能码组内偏移
    Uint32                      DispData;               //LED数码管显示数据
    Uint16                      UserPass;               //用户密码
}STR_PANELMENU;


//LED显示控制寄存器定义
typedef struct{
    Uint8   ZeroFlicker:1;       //第0位数据闪烁控制位  0:不闪烁, 1:闪烁
    Uint8   FirstFlicker:1;      //第1位数据闪烁控制位  0:不闪烁, 1:闪烁
    Uint8   SecondFlicker:1;     //第2位数据闪烁控制位  0:不闪烁, 1:闪烁
    Uint8   ThirdFlicker:1;      //第3位数据闪烁控制位  0:不闪烁, 1:闪烁
    Uint8   ForthFlicker:1;      //第4位数据闪烁控制位  0:不闪烁, 1:闪烁
    Uint8   DotFlicker:1;        //小数点闪烁控制位 0:正常, 1：永远都不闪烁
    Uint8   Rsvd:2;              //保留
}STR_LEDDISPLAYCTRL_BIT;


typedef union{
    volatile Uint8                   all;
    volatile STR_LEDDISPLAYCTRL_BIT  bit;
}UNI_LEDDISPLAYCTRL_REG;


//LED显示结构体类型定义
typedef struct{
    Uint8           DispData[5];            //LED数码管显示段数据
    volatile UNI_LEDDISPLAYCTRL_REG  CtrlReg;        //控制寄存器
    Uint8           Timer;                  //定时器,闪烁时使用
    Uint8           TubeIndex;              //当前显示的数码管序号
    Uint8           TxTubeSel;              //发送当前显示的数码管使能
    Uint8           TxDispData;             //发送当前显示的数码管数据
}STR_LEDDISPLAY;


//面板结构体类型定义
typedef struct{
    STR_KEYSCAN             KeyScan; 
    STR_PANELMENU           Menu;
    STR_LEDDISPLAY          Display;
}STR_PANELOPERRATION;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
extern STR_PANELOPERRATION     ServoPanel;

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */


#ifdef __cplusplus
}
#endif 

#endif /* end of FUNC_PANELKEYOPERATION_H */

/********************************* END OF FILE *********************************/




