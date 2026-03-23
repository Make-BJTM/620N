/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_OperEeprom.c
 创建人：童文邹                
 修改人：李浩               创建日期：11.11.01 
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
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_OperEeprom.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_WWDG.h"
#include "FUNC_InterfaceProcess.h" 
#include "FUNC_FunCodeDefault.h"

#define I2C_STM32F20X 
#ifdef  I2C_STM32F20X
#include "FUNC_I2cDriver_ST.h"
#endif

#ifdef  I2C_DSP2833X
#include "FUNC_I2cDrive_Ti.h"
#endif

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/

#define EEPROM_ADDRESS_RANGE_LIMIT         2032   //功能码存入Eeprom时的首末地址，不能大于Eeprom存储限制--2048！

#define I2C_STM32F20X 

#define I2C_READ    0    //IIC 读状态
#define I2C_WRITE   1    //IIC 写状态


#define FUNCODE_RW_STATE_READING           0   // Eeprom 正在读取
#define FUNCODE_RW_STATE_READ_OK           1   // 读取Eeprom完毕且成功，读取的数据放在DataReadArr开始的地址里面

#define FUNCODE_RW_STATE_WRITING           0   //  Eeprom正在写入
#define FUNCODE_RW_STATE_WRITE_OK          1   // 写入Eeprom完毕且成功

#define FUNCODE_RW_STATE_PARA_ERROR        4   // Eeprom 读写功能码参数错误，目前仅用于读取个数错误


#define FUNCODE_READ_VERIFY_RIGHT_TIME_MAX     3             // 在读Eeprom功能码状态下，读取Eeprom功能码验证次数为3次
#define FUNCODE_READ_FOR_WRITE_VERIFY_RIGHT_TIME_MAX     2   // 在写Eeprom功能码状态下，读取Eeprom功能码验证次数为2次


#define FUNCODE_EEPROM_START_INDEX        16   // Eeprom的前面(_*2)个byte被误写的可能性比较大，所以不使用

#if (16 == EEPROM_PAGE_BYTE)
#define EEPROM_PAGE_OFFSET_FUNCODE       0x07
#elif (32 == EEPROM_PAGE_BYTE)
#define EEPROM_PAGE_OFFSET_FUNCODE       0x0f
#endif


/*Eeprom操作故障宏定义*/

#define SERI_UPDATE_MAX                200  //  Eeprom连续读写时可以允许的最大更新数组范围
#define SINGLE_UPDATE_MAX              700  //  Eeprom 同时允许修改的不连续的功能码个数

#define EEPROM_READ_SINGLE_TIME_MAX    500  // _*1ms，单次读EEPROM的超时时间
#define EEPROM_WRITE_SINGLE_TIME_MAX   750  // _*1ms，单次写EEPROM的超时时间


/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */
#define GET_PAGE_OFFSET_FUNCODE(Index)    (((Index) + FUNCODE_EEPROM_START_INDEX) & EEPROM_PAGE_OFFSET_FUNCODE)


/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */ 
STR_EEPROM_RW_TIME    STR_EepromRwTime;

ENU_FUNCODE_EEPROM_RW_FLAG   FunCodeRwWorkFlag = FUNCODE_RW_STATE_READ_DATA;  // 读写功能码过程的几个工作标志,是枚举类型

ENU_ALL_OPERATE_MODE    EepromOperateMode = ALL_NO_OPERATE;   // Eeprom所有读取方式的枚举型变量定义
ENU_SERI_OPERATE_MODE   SeriOperateMode = SERI_NO_OPERATE;    // Eeprom连续读取方式的枚举型变量定义

ENU_EEPROM_RW_ERROR    EepromRwError = EEPROM_NONE_ERROR;     //Eeprom读取故障类型变量
/* Private_Variables ---------------------------------------------------------*/
/* 文件内变量定义 */

Uint16  SeriRwNum;                          //Eeprom连续读写个数
Uint16  SeriRwBuffArr[SERI_UPDATE_MAX][3];  //Eeprom连续读写缓冲数组
Uint16  SingleWriteNum;                     //Eeprom 单个不连续写数据的个数
Uint16  SingleWriteArr[SINGLE_UPDATE_MAX];

Uint16  EepromRwStartIndex;    // Eeprom最终操作所用到的读写地址INDEX,周期调度需要使用
Uint16  EepromRwFinalIndex;    // Eeprom最终操作所用到的读写地址INDEX,周期调度需要使用
Uint16  EepromRwFunCodeNum;    // 最多有EEPROM_PAGE_FUNCODE_NUM个

Uint8 CutoffStutas = 0;   //掉电保状态 0 正常 1 准备 2 存储 3 存储完

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
void SaveToEepromOne(Uint16 Index);
void SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex);
void SaveToEepromAll(void);
void ReadFromEeprom(Uint16 Start,Uint16 End);
void ReadFromEepromAll(void);
void EepromRwWatchDog(void);
Uint16 EepromProcess(void);
void InitI2c(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */
Uint16 ReadFunCode(STR_EEPROM_FUNCODE_RW *p);
Uint16 WriteFunCode(STR_EEPROM_FUNCODE_RW *p);
void UpdateEepromIndex(void);
void SeriRwCheck(void);


/*******************************************************************************
  函数名: void InitI2c() 
  输入:                 
  输出:    
  子函数: InitI2c_ST()，InitI2c_Ti()
  描述:  两种平台的I2C初始化选择函数
  返回值:  
********************************************************************************/ 
void InitI2c()
{
#ifdef  I2C_STM32F20X
    InitI2c_ST();
#endif

#ifdef  I2C_TI28XXX
     InitI2c_Ti();
#endif
}

/*******************************************************************************
  函数名: void EepromRwWatchDog()
  输入:             
  输出:    
  子函数:         
  描述:  
********************************************************************************/ 
void EepromRwWatchDog()
{
    if (STR_EepromRwTime.ReadFlag)   // 正在进行EEPROM的读操作
    {
        STR_EepromRwTime.ReadTicker++;
        if (STR_EepromRwTime.ReadTicker > EEPROM_READ_SINGLE_TIME_MAX)   // 单次读超时，报错
        {
            STR_EepromRwTime.ReadTicker = 0;
            EepromRwError = EEPROM_READ_ERROR;
        }
    }

    if (STR_EepromRwTime.WriteFlag)   // 正在进行EEPROM的写操作
    {
        STR_EepromRwTime.WriteTicker++;
        if (STR_EepromRwTime.WriteTicker > EEPROM_WRITE_SINGLE_TIME_MAX)   // 单次写超时，报错
        {
            STR_EepromRwTime.WriteTicker = 0;
            EepromRwError = EEPROM_WRITE_ERROR;
        }
    }
}

/*******************************************************************************
  函数名: SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex) 
  输入: 需要操作的连续功能码的首地址StartIndex和末地址EndIndex            
  输出:    
  子函数:         
  描述:  保存连续的功能码到EEPROM中
********************************************************************************/ 
void SaveToEepromSeri(Uint16 StartIndex,Uint16 EndIndex)
{
    if ((StartIndex >= EEPROM_ADDRESS_RANGE_LIMIT) || (EndIndex >= EEPROM_ADDRESS_RANGE_LIMIT) || (StartIndex > EndIndex))
    {
        PostErrMsg(DEFUALTERR);            //系统参数出故障了
        return;
    }
    else
    {
        SeriRwCheck();

        SeriRwBuffArr[SeriRwNum-1][0] = SERI_WRITE_SERI;
        SeriRwBuffArr[SeriRwNum-1][1] = StartIndex;
        SeriRwBuffArr[SeriRwNum-1][2] = EndIndex;
    }
}

/*******************************************************************************
  函数名: SaveToEepromAll() 
  输入: 需要输入功能码的起始地址FUNCODE_START_INDEX，和可操作功能码得个数FUNC_OPER_LEN_SAVE           
  输出:    
  子函数: 无        
  描述:  把所有可操作的功能码保存到EEPROM中
********************************************************************************/ 
void SaveToEepromAll()
{
    SeriRwCheck();

    SeriRwBuffArr[SeriRwNum-1][0] = SERI_WRITE_ALL;       // 对应ALL_WRITE_ALL
    //此处仅只能操作的功能码不包括 EepromCheckWord[]  HErrSv[] HRsvd[] HOffSv[]
    // 功能码的起始地址
    SeriRwBuffArr[SeriRwNum-1][1] = H0000INDEX;  
    //功能码的末尾地址
    SeriRwBuffArr[SeriRwNum-1][2] = HRsvd00INDEX - 1;  
    
}

/*******************************************************************************
  函数名: SaveToEepromOne(Uint16 Index)
  输入:  单个写操作的功能码个数，以及其相应的地址          
  输出:    
  子函数:         
  描述:  把将要执行单个写操作的功能码的地址放入缓冲数组中，并且当单个写操作的功能码
         个数超限时，应进行报错处理
********************************************************************************/ 
void SaveToEepromOne(Uint16 Index)
{
    if ( Index >= EEPROM_ADDRESS_RANGE_LIMIT )
    {
        return;
    }
    else
    {
        SingleWriteNum++;

        if(SingleWriteNum > SINGLE_UPDATE_MAX)          // 若将要执行单个写操作的功能码的个数超过限制，则应进行超限报错
        {
            SingleWriteNum = SINGLE_UPDATE_MAX;

            if(EepromRwError == EEPROM_NONE_ERROR)
            {
                EepromRwError = EEPROM_FUNCODE_NUM_OVER_ERROR;
            }
        }
        else
        {
            SingleWriteArr[(SingleWriteNum-1)] = Index; // 把将要执行单个写操作的功能码的地址放入缓冲数组中
        }
    }
}

/*******************************************************************************
  函数名: ReadFromEeprom(Uint16 Start,Uint16 End)
  输入:  需要操作的功能码段的首尾地址以及操作方式          
  输出:    
  子函数: 无        
  描述:  用于功能码第一次初始化时从EEPROM中读取Flash校验到RAM中
********************************************************************************/ 
void ReadFromEeprom(Uint16 Start,Uint16 End)
{
    if ((Start >= EEPROM_ADDRESS_RANGE_LIMIT) || (End >= EEPROM_ADDRESS_RANGE_LIMIT) || (Start > End) )
    {
        PostErrMsg(DEFUALTERR);            //系统参数出故障了
        return;
    }
    else
    {
        SeriRwCheck();

        SeriRwBuffArr[SeriRwNum-1][0] = SERI_READ_SERI;
        SeriRwBuffArr[SeriRwNum-1][1] = Start;
        SeriRwBuffArr[SeriRwNum-1][2] = End;
    }

}

/*******************************************************************************
  函数名: ReadFromEepromAll()
  输入:  需要操作的功能码首尾地址以及功能码操作方式       
  输出:    
  子函数: 无       
  描述: 用于功能码初始化过程中从EEPROM中读取功能码到RAM中 
********************************************************************************/ 
void ReadFromEepromAll()
{
    SeriRwCheck();

    SeriRwBuffArr[SeriRwNum-1][0] = SERI_READ_ALL;
    SeriRwBuffArr[SeriRwNum-1][1] = 0;
    SeriRwBuffArr[SeriRwNum-1][2] = 2031; 
}   

/*******************************************************************************
  函数名: Uint16 EepromProcess() 
  输入:                 
  输出:    
  子函数: ReadFunCode()(Eeprom读功能码函数)，WriteFunCode()(Eeprom写功能码函数)
          UpdateEepromIndex()(下一次读写操作前的更新)       
  描述: 不断调度用的EEPROM处理程序,默认状态应该是EEPROM无操作,底层操作需要的标志位应该在正常工作前置好位
        感觉很多操作在调度中反复运行
  返回值: 0--EEPROM 空闲或上次操作完成  1--EEPROM正在忙  2-故障态 
********************************************************************************/  
Uint16 EepromProcess()
{
    static    STR_EEPROM_FUNCODE_RW   FunCodeRead;
    static    STR_EEPROM_FUNCODE_RW   FunCodeWrite;
    Uint16    SeriStartIndex;                // EEPROM操作当前被操作功能码首INDEX
    Uint16    SeriFinalIndex;                // EEPROM操作当前被操作功能码尾INDEX

    /*第1阶段：Eeprom故障报错处理*/
    if (EepromRwError == EEPROM_WRITE_ERROR)  // 当Eeprom写故障时，则进入故障状态
    {
        PostErrMsg(PRAMSVERR_WR); 
        EepromOperateMode = ALL_ERR_STATE;
        return 2; // 0 空闲 1忙 2故障  (EEPROM忙碌状态返回值,返回故障状态)
    }
    else if (EepromRwError == EEPROM_READ_ERROR)  // 当Eeprom读故障时，则进入故障状态
    {
        PostErrMsg(PRAMSVERR_RD); 
        EepromOperateMode = ALL_ERR_STATE;
        return 2; // 0 空闲 1忙 2故障  (EEPROM忙碌状态返回值,返回故障状态)
    }
    else if (EepromRwError == EEPROM_FUNCODE_NUM_OVER_ERROR )  // 当Eeprom读写功能码个数超限时，则进入故障报警
    {
        PostErrMsg(PRAMSVWARN);
        EepromRwError = EEPROM_NONE_ERROR;
    }
#ifdef  I2C_DSP2833X    /* Ti平台检测I2C的STOP和READY_ACCESS的两个中断标志位*/
    I2cInterrputPoll();
#endif 

//-----------------------------------------------------------------------------------//
    /*第2阶段：Eeprom读写模式选择，以及读写地址保存*/
    if((0 == CutoffStutas) && (1 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave))
    {
        CutoffStutas = 1;
    }

    if((3 == CutoffStutas) && (0 == STR_FUNC_Gvar.Monitor2Flag.bit.CutoffPowerSave))
    {
        CutoffStutas = 0;
    }

    if(3 == CutoffStutas) 
    {
        SeriRwNum = 0;
        SingleWriteNum = 0;
        return 0;
    }

    if(1 == CutoffStutas)
    {
        if(!EepromOperateMode)   //如果当前没有执行EEPROM操作任务
        {
            SeriRwNum = 0;     //只执行一次掉电保存写操作
            SingleWriteNum = 0;
            
            if((1 != FunCodeUnion.code.ER_PowerOffSaveToEeprom) && (2 != UNI_FUNC_MTRToFUNC_InitList.List.AbsPosDetection))
            {
                //不进行掉电保存
                CutoffStutas = 3;
                return 0;
            }
            else
            {
                CutoffStutas = 2;
            }

            SeriOperateMode = (ENU_SERI_OPERATE_MODE)SERI_CUT_POWER_WRITE;  //掉电写操作
            SeriStartIndex  = HOffSv00INDEX;   //掉电写首地址
            SeriFinalIndex  = HOffSv00INDEX + OffSvLEN - 1;   //掉电写末地址

            EepromOperateMode  = (ENU_ALL_OPERATE_MODE)SeriOperateMode;
            EepromRwStartIndex = SeriStartIndex ;
            EepromRwFinalIndex = SeriFinalIndex + 1;
        }
    }
    else if(2 == CutoffStutas)
    {
        if(!EepromOperateMode)
        {
            // 若当前没有Eeprom读写操作请求(即SeriRwNum=0;SingleWriteNum=0时),则返回空闲状态
            CutoffStutas = 3;
            return 0;   // 0 空闲 1忙 2故障  (EEPROM忙碌状态返回值,返回闲)
        }
    }
    else     //非掉电保存情况下的处理
    {
        if(!EepromOperateMode)   //如果当前没有执行EEPROM操作任务
        {
            if(SeriRwNum)  //如果外部有连续EEPROM操作的请求
            {
                SeriRwNum--;
    
                SeriOperateMode = (ENU_SERI_OPERATE_MODE)SeriRwBuffArr[SeriRwNum][0];
                SeriStartIndex  = SeriRwBuffArr[SeriRwNum][1];
                SeriFinalIndex  = SeriRwBuffArr[SeriRwNum][2];
    
                EepromOperateMode  = (ENU_ALL_OPERATE_MODE)SeriOperateMode;
                EepromRwStartIndex = SeriStartIndex ;
                EepromRwFinalIndex = SeriFinalIndex + 1;
            }
            else if(SingleWriteNum)  // 如果有单个EEPROM操作滞留缓冲区
            {
                EepromOperateMode = ALL_WRITE_ONE; 
                SingleWriteNum--;                                 // 需要保存的功能码的数量减少1
                EepromRwStartIndex = SingleWriteArr[SingleWriteNum];   
            }
            else    // 若当前没有Eeprom读写操作请求(即SeriRwNum=0;SingleWriteNum=0时),则返回空闲状态
            {
                return 0;   // 0 空闲 1忙 2故障  (EEPROM忙碌状态返回值,返回闲)
            }
        }
    }

//----------------------------------------------------------------------------------------------//

    /*第3阶段：准备读取功能码在E2PROM的当前数据,即准备要写入功能码的起始地址(index)和要写入的功能码个数(Number)*/
    if (FunCodeRwWorkFlag == FUNCODE_RW_STATE_READ_DATA)
    {
        if (EepromOperateMode == ALL_WRITE_ONE) // 保存1个功能码
        {
            EepromRwFunCodeNum = 1;    
        }
        else   //保存多个功能码
        {
            EepromRwFunCodeNum = EepromRwFinalIndex - EepromRwStartIndex;   //Eeprom 读取功能码个数
            if ( EepromRwFunCodeNum > EEPROM_PAGE_FUNCODE_NUM - GET_PAGE_OFFSET_FUNCODE(EepromRwStartIndex) ) //不额外带进其他的功能码!
            {
                EepromRwFunCodeNum = EEPROM_PAGE_FUNCODE_NUM - GET_PAGE_OFFSET_FUNCODE(EepromRwStartIndex);    
            }
        }

        FunCodeRead.RwIndex = EepromRwStartIndex;
        FunCodeRead.RwNumber = EepromRwFunCodeNum;

        if ( EepromOperateMode == ALL_WRITE_ALL ) // 恢复全部出厂参数,功能码全部写入时，没有必要先读取
        {
            FunCodeRwWorkFlag = FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE;
        }

        if(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode  == 1)
        {
            FunCodeRwWorkFlag = FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE;
        }

        if ( EepromOperateMode == ALL_CUT_POWER_WRITE )  //掉电保存时，不进行读过程，直接进行预写入过程
        {
            FunCodeRwWorkFlag = FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE;
        }
        else  //其他模式下都需要先读取再写入
        {
            //停止看门狗
            if(STR_PUB_Gvar.AllInitDone == 1)
            {
                DisableWDog();
            }

            if( FUNCODE_RW_STATE_READ_OK == ReadFunCode(&FunCodeRead) )//msx+
            {
                FunCodeRwWorkFlag = FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE;
            }

            //启动看门狗
            if(STR_PUB_Gvar.AllInitDone == 1)
            {
                FUNC_InitAndEnableWatchDog();
            }
        }
    }

    /*第4阶段：读取FunCodeRead.data[i]的数据到RAM区,如果有写入,准备要写入EEPROM的数据*/
    if ( FunCodeRwWorkFlag == FUNCODE_RW_STATE_PREPARE_DATA_FOR_WRITE)
    {
        int16  i;
        Uint16 IndexTmp;     // 临时循环地址变量
        for (i = 0; i < EepromRwFunCodeNum; i++)
        {
            IndexTmp = EepromRwStartIndex + i;
            if ( (EepromOperateMode == ALL_WRITE_ONE)||(EepromOperateMode == ALL_WRITE_SERI) )  //写一个功能码或者连续写多个功能码
            {
                if(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode  == 1)
                {
                    FunCodeWrite.RwData[i] = FunCodeDeft[IndexTmp - 16].Value;
                    FunCodeRwWorkFlag = FUNCODE_RW_STATE_WRITE_DATA; 
                }
                else
                {
                    FunCodeWrite.RwData[i] = FunCodeUnion.all[IndexTmp];    //将相应的功能码存入要写入Eeprom的写入数组中
                    if (FunCodeRead.RwData[i] != FunCodeWrite.RwData[i])      //如果写入的数组和读取的数组有任何一个不一致，则确定需要写入
                    {
                        FunCodeRwWorkFlag = FUNCODE_RW_STATE_WRITE_DATA;
                    }
                }
            }
            else if (EepromOperateMode == ALL_WRITE_ALL)              // 所有功能码恢复出厂参数,不用先读，直接写入，写所有的功能码,目前特指在恢复出厂参数,或者第一次使用EEPROM，所有功能码恢复出厂参数，一定需要先将参数从FLASH读入到RAM,然后再写入EEPROM
            {
                FunCodeWrite.RwData[i] = FunCodeUnion.all[IndexTmp];    // 不用读取后比较，直接写入
                FunCodeRwWorkFlag = FUNCODE_RW_STATE_WRITE_DATA;                
            }
            else if (EepromOperateMode == ALL_CUT_POWER_WRITE)
            {
                FunCodeWrite.RwData[i] = FunCodeUnion.all[IndexTmp];    // 不用读取后比较，直接写入
                FunCodeRwWorkFlag = FUNCODE_RW_STATE_WRITE_DATA;                
            }
            else if ( (EepromOperateMode == ALL_READ_ALL)||(EepromOperateMode == ALL_READ_SERI) )  //如果只是读取功能码(即读取一个或一串时)，将从Eeprom中读出来的功能码存入到RAM区去
            {
                 FunCodeUnion.all[IndexTmp] = FunCodeRead.RwData[i];	 		               
            }
            else  // 如果EepromOperateMode == ALL_NO_OPERATE，(即无任何读写操作时，则不进行相关处理)
            {
               ;  //无操作
            }
        }
        FunCodeWrite.RwIndex = EepromRwStartIndex;    //如果是需要写入功能码的情形下，还需存入，写入功能码的首地址
        FunCodeWrite.RwNumber = EepromRwFunCodeNum;   //如果是需要写入功能码的情形下，还需存入，写入功能码的个数
         
        if (FunCodeRwWorkFlag != FUNCODE_RW_STATE_WRITE_DATA)  // 若EEPROM当前值与需要写入的值完全一致, 或仅仅是读取状态时, 则不需要真正的写EEPROM
        {
            UpdateEepromIndex();   // 为下一次的Eeprom的读取或写入做准备，读写状态复归到读状态，读写地址相应的增加
        }
    }

    /*第5阶段：将FunCodeWrite.data[i]中的数据写入到Eeprom里面去，写入操作需要用写入函数WriteFunCode()来完成*/
    if (FunCodeRwWorkFlag == FUNCODE_RW_STATE_WRITE_DATA)
    {
        //停止看门狗
        if(STR_PUB_Gvar.AllInitDone == 1)
        {
            DisableWDog();
        }

        if (FUNCODE_RW_STATE_WRITE_OK == WriteFunCode(&FunCodeWrite)) 
        {
            UpdateEepromIndex();  // 在本次写入功能码完成后，需要为下一次的写入操作，来更新相应的读写地址，和读写状态
        }

        //启动看门狗
        if(STR_PUB_Gvar.AllInitDone == 1)
        {
            FUNC_InitAndEnableWatchDog();
        }
    }

    return 1;   // 0 空闲 1忙 2故障  (EEPROM忙碌状态返回值,返回忙)
}

/*******************************************************************************
  函数名: Uint16 EepromReadFunCode(STR_EEPROM_FUNCODE_RW *p)
  输入:   p->RwIndex  -- 读取数据的起始index
          p->RwNumber -- 读取数据的个数, (0, EEPROM_PAGE_FUNCODE_NUM]           
  输出:   p->RwData[] -- 保存读取数据
  返回值：FUNCODE_RW_STATE_READING    -- 正在读取
          FUNCODE_RW_STATE_READ_OK    -- 读取完毕，读取的数据放在DataReadArr开始的地址里面
          FUNCODE_RW_STATE_PARA_ERROR -- 参数错误，目前仅是readNumber错误       
  子函数: I2cRwDeal() I2C读取处理函数                                       
  描述: 从EEPROM(24LC08/32/64)中读取n个16bit数据
        读取过程如下：
        1. 开始读取，若连续读取(FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX)次每次数据一致，读取完毕；
        2. 若在读取中发现有不一致，重新再连续读取(FUNCCODE_READ_VERIFY_RIGHT_TIME_MAX)次，每次数据一致，读取完毕；
        3. 否则，重复第2步。
        4. 当前有读写EEPROM错误，不读取而直接退出。 
********************************************************************************/ 
Uint16 ReadFunCode(STR_EEPROM_FUNCODE_RW *p)
{
    static Uint8 FristRead = 0;        // 读取次数，仅用来判断是否是第一次读取
    static Uint8 ReadRightCnt = 0;     // 连续读取且数据一致的次数
    static Uint8 ReadVerifyWrong = 0;  // 验证不一致标志，延时使用
    static Uint16 DataReadArr[EEPROM_PAGE_FUNCODE_NUM];  //功能码数据读取数组
    int8 ReadRightCntMax = 0;              //连续正确读取数据次数最大值
    int16 Count = 0;         //I2C 读取功能码后存放数据的循环计数值
    Uint8 I2cReadState;      //I2C 读功能码函数返回的状态

    STR_EepromRwTime.ReadFlag = 1;  //读超时计数器的读取标志位置1

    if ((0 == p->RwNumber) || (p->RwNumber > EEPROM_PAGE_FUNCODE_NUM)) // 检查输入参数
    {
        return FUNCODE_RW_STATE_PARA_ERROR;  //Eeprom中的功能码读个数取超限，返回参数故障
    }

    if (ReadVerifyWrong == 1) 
    {   
        ReadVerifyWrong = 0;
        return FUNCODE_RW_STATE_READING;  //继续读取Eeprom中功能码数据
    }  
#ifdef  I2C_STM32F20X    /* ST平台通过I2C从EEPROM中读取功能码数据*/
   
    I2cReadState = I2cRwDeal( (Uint8 *)DataReadArr, 
                   ((p->RwIndex + FUNCODE_EEPROM_START_INDEX) << 1),
                   (p->RwNumber << 1),
                   I2C_READ );    
#endif

#ifdef  I2C_DSP2833X    /* Ti平台通过I2C从EEPROM中读取功能码数据*/

    STR_I2cReadOprate.HighAddress = (((p->RwIndex + FUNCODE_EEPROM_START_INDEX) << 1) & 0xff00);
    STR_I2cReadOprate.LowAddress  = ((p->RwIndex + FUNCODE_EEPROM_START_INDEX) << 1) - STR_I2cReadOprate.HighAddress;
    STR_I2cReadOprate.HighAddress = STR_I2cReadOprate.HighAddress >> 8;
    STR_I2cReadOprate.Number      =  p->RwNumber<< 1;

    if(!STR_I2cReadOprate.Status)  //读无效状态时，发送读控制字
    {
        STR_I2cReadOprate.Status   = I2C_READ_NOSTOP; //I2C发送读控制字标志(地址信息)
    }

    I2cReadState = I2cRwOprate();

    for(i = p->RwNumber - 1; i >= 0 ; i--)
    {    
        DataReadArr[i] = STR_I2cReadOprate.Buffer[(i<<1)] + ((STR_I2cReadOprate.Buffer[(i<<1) + 1]) << 8);  //高BYTE存高8位，低BYTE存低8位
    }
#endif

    if (I2cReadState == 1)  //I2C读取Eeprom中的功能码数据完成
    {
        FristRead ++;
        for (Count = p->RwNumber - 1; Count >= 0 ; Count--) 
        {
            if (FristRead == 1) //对第一次I2C读取的数据进行保存
            {
                p->RwData[Count] = DataReadArr[Count];
            }
            else  //对第一次之后，再读取的数据进行比较
            {
                if (p->RwData[Count] != DataReadArr[Count])
                    break;
            }
        }

        if (FristRead == 1)
        {
            ReadRightCnt = 1;  // 验证不一致，则再连续读取_次，全部一致，认为OK. 
            return FUNCODE_RW_STATE_READING;  //继续读取Eeprom中功能码数据
        }

        if (Count < 0)
        {
            if (FunCodeRwWorkFlag == FUNCODE_RW_STATE_WRITE_DATA)  // 如果是写状态，则读取验证次数为2次
                ReadRightCntMax = FUNCODE_READ_FOR_WRITE_VERIFY_RIGHT_TIME_MAX;
            else                                                   // 如果是其他状态，则读取验证次数为3次
                ReadRightCntMax = FUNCODE_READ_VERIFY_RIGHT_TIME_MAX;

            ReadRightCnt++;                            //读取验证次数自加
            if (ReadRightCnt >= ReadRightCntMax)       // 从开始读取验证，连续读取_次，全部一致则OK，若验证不一致，则再连续读取_次，全部一致则OK
            {
                FristRead = 0;
                STR_EepromRwTime.ReadFlag = 0;     // 读完成
                STR_EepromRwTime.ReadTicker = 0;   // ticker清零
                return FUNCODE_RW_STATE_READ_OK;
            }
        }
        else                      // 与上次读取的数据不一致
        {
            FristRead = 0;        // 第一次的读取就有可能不正确，要重新读取
            ReadVerifyWrong = 1;  // 验证不一致标志，延时使用
            
            return FUNCODE_RW_STATE_READING; // FUNCCODE_READ_RET_WAIT_FOR_REPEAT;
        }
    }   
   return FUNCODE_RW_STATE_READING;   // 如果当前I2C没有读取好，或没有准备好，或I2C总忙，则会自动重读取Eeprom中功能码数据   
}

/*******************************************************************************
  函数名: Uint16 WriteFunCode(STR_EEPROM_FUNCODE_RW *p)
  输入:   p->RwIndex  -- 读取数据的起始index
          p->RwNumber -- 读取数据的个数, (0, EEPROM_PAGE_FUNCODE_NUM]           
  输出:   p->RwData[] -- 保存读取数据
  返回值：FUNCODE_RW_STATE_WRITING    -- 正在读取
          FUNCODE_RW_STATE_WRITE_OK    -- 读取完毕，读取的数据放在DataReadArr开始的地址里面
          FUNCODE_RW_STATE_PARA_ERROR -- 参数错误，目前仅是readNumber错误       
  子函数: I2cRwDeal() I2C读取处理函数                                       
  描述: 从EEPROM(24LC08/32/64)中写入n个16bit数据
        写入过程如下：
        1. 开始写入
        2. 读取验证，若一致，写入完毕。
        3. 若验证不一致，返回第1步(重新写入，再读取验证)。
        4. 当前有写入EEPROM错误，不写入而直接退出 
********************************************************************************/ 
Uint16 WriteFunCode(STR_EEPROM_FUNCODE_RW *p)
{
    static ENU_FUNCODE_WRITE_STATE    WriteState = FUNCODE_STATE_WRITE;   //定义写Eeprom过程状态的枚举型变量
    static STR_EEPROM_FUNCODE_RW    FunCodeReadAfterWrite;      //定义写完后，读取检验结构体
    static Uint8  WriteVerifyWrong;   // 验证不一致标志
    int16 Count = 0;
    Uint8 I2cWriteState;  //I2C写入功能码函数返回的状态

    STR_EepromRwTime.WriteFlag = 1;  //写超时计数器的读取标志位置1

    if ((p->RwIndex + p->RwNumber) > (p->RwIndex + EEPROM_PAGE_FUNCODE_NUM) // 写入Eeprom的数据不能超过一页
       || (0 == p->RwNumber) || (p->RwNumber > EEPROM_PAGE_FUNCODE_NUM))
    {
        return FUNCODE_RW_STATE_PARA_ERROR;
    }
    if (WriteVerifyWrong == 1) 
    {
        WriteVerifyWrong = 0;
        return FUNCODE_RW_STATE_WRITING;    
    }

    if (WriteState == FUNCODE_STATE_WRITE) // 写入EEPROM数据
    {          
#ifdef  I2C_STM32F20X     /* ST平台通过I2C向EEPROM中写入功能码数据*/
   
        I2cWriteState = I2cRwDeal((Uint8 *)p->RwData,
                                 ((p->RwIndex + FUNCODE_EEPROM_START_INDEX) << 1),
                                 (p->RwNumber << 1),
                                 I2C_WRITE);
#endif

#ifdef  I2C_DSP2833X    /* Ti平台通过I2C向EEPROM中写入功能码数据*/
    
        for(i = 0; i <= p->RwNumber - 1; i++)
        {
            STR_I2cWriteOprate.Buffer[i << 1]       = p->RwData[i] & 0x00ff;  //低位
            STR_I2cWriteOprate.Buffer[(i << 1) + 1] = p->RwData[i] >> 8;      //高位
        }
        STR_I2cWriteOprate.HighAddress = (((p->RwIndex + FUNCODE_EEPROM_START_INDEX) << 1) & 0xff00);
        STR_I2cWriteOprate.LowAddress  = ((p->RwIndex  + FUNCODE_EEPROM_START_INDEX) << 1) - STR_I2cWriteOprate.HighAddress;
        STR_I2cWriteOprate.HighAddress =  STR_I2cWriteOprate.HighAddress >> 8;
        STR_I2cWriteOprate.Number      = (p->RwNumber) << 1;   

        if(!STR_I2cWriteOprate.Status)
		{
            STR_I2cWriteOprate.Status  = I2C_WRITE_WITH_STOP;
        }

        I2cWriteState = I2cRwOprate(); 
#endif
        if (I2cWriteState == 1)     
            WriteState = FUNCODE_STATE_READ_FOR_VERIFY_WRITE;   //如果当前I2C已经写完，则转为读取检验状态
        else 
            return FUNCODE_RW_STATE_WRITING;     // 如果当前I2C没有准备好，或没有写完，或I2C总线忙，会自动重写
    }

    if (WriteState == FUNCODE_STATE_READ_FOR_VERIFY_WRITE) // 写入完成之后，再读取进行验证
    {
        FunCodeReadAfterWrite.RwIndex = p->RwIndex; 
        FunCodeReadAfterWrite.RwNumber = p->RwNumber;
         
        if ( FUNCODE_RW_STATE_READ_OK == ReadFunCode(&FunCodeReadAfterWrite) )  //写完之后，再完成读取
        {
            for (Count = p->RwNumber - 1; Count >= 0; Count--)
            {
                if (FunCodeReadAfterWrite.RwData[Count] != p->RwData[Count])   //若写完之后读取的值不等以写入的值，则退出循环，此时Count>0
                    break;
            }

            if (Count < 0)              // 读取的数据与写入的数据一致
            { 
                STR_EepromRwTime.WriteFlag = 0;    // 写完成
                STR_EepromRwTime.WriteTicker = 0;  // ticker清零
                WriteState = FUNCODE_STATE_WRITE;  // 写入成功，则再复位写状态
                
                return FUNCODE_RW_STATE_WRITE_OK;  // 写入Eeprom完毕且成功
            }
            else                        // 读取的数据与写入的数据不一致
            {
                WriteVerifyWrong = 1;   // 验证不一致标志，延时使用
                WriteState = FUNCODE_STATE_WRITE; 
                return FUNCODE_RW_STATE_WRITING;  // 验证不一致，重新写入
            }
        }
        else    //读取不成功，也要从新写入
        {    
            return FUNCODE_RW_STATE_WRITING; 
        } 
    }
    return FUNCODE_RW_STATE_WRITING;
}

/*******************************************************************************
  函数名: UpdateEepromIndex()
  输入:             
  输出:    
  子函数: 无        
  描述: 为下次EepromProcess()函数操作更新所需要的操作变量,即更新读写地址和读写状态 
********************************************************************************/ 
void UpdateEepromIndex()
{
    FunCodeRwWorkFlag = FUNCODE_RW_STATE_READ_DATA;  // 回到EepromProcess()第一个工作状态

    if(1 == CutoffStutas)
    {
        EepromOperateMode = ALL_NO_OPERATE;      // 读写完成后，应将操作模式恢复到无操作状态
        SeriOperateMode   = SERI_NO_OPERATE;     // 读写完成后，应将操作模式恢复到无操作状态
        return;
    }

    if (EepromOperateMode == ALL_WRITE_ONE)         
    {
        EepromOperateMode = ALL_NO_OPERATE;          // 如果Eeprom的为写一个功能码操作模式的话，则在写入完成后，Eeprom的操作模式应恢复到无操作状态
    }
    else
    {
        EepromRwStartIndex += EepromRwFunCodeNum;    // 下一次准备写入的功能码首地址
        if(EepromRwStartIndex >= EepromRwFinalIndex) // 当需要读取或写入的功能码全部写完后，即每次读取或写入时都更新的首地址大于末尾地址时，则表明写完
        {                                            // 若未读取或写入完成，则相应的Eeprom操作模式保持不变，不用复位到无操作状态
            EepromOperateMode = ALL_NO_OPERATE;      // 读写完成后，应将操作模式恢复到无操作状态
            SeriOperateMode   = SERI_NO_OPERATE;     // 读写完成后，应将操作模式恢复到无操作状态 
        }
    }
}


/*******************************************************************************
  函数名: SeriRwCheck()
  输入:             
  输出:    
  子函数:         
  描述: Eeprom连续读写时，读写个数检测，当连续读写个数超过SERI_UPDATE_MAX，应进行
        个数超限报错 
********************************************************************************/ 
void SeriRwCheck()
{
    SeriRwNum++;
    if(SeriRwNum > SERI_UPDATE_MAX)
    {
        SeriRwNum = SERI_UPDATE_MAX;
        if(STR_PUB_Gvar.AllInitDone == 1)  //未初始化或初始化不正确可能会导致朝限
        {
            if(EepromRwError == EEPROM_NONE_ERROR)
            {
                EepromRwError = EEPROM_FUNCODE_NUM_OVER_ERROR;
            }
        }
    }
}


/********************************* END OF FILE *********************************/
