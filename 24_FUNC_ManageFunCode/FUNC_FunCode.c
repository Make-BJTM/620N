/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_FunCode.c
 创建人：   童文邹                 创建日期：2009.10
 修改人：   王治国                 修改日期：2011.10.15 
 描述:
    1. 
    2. 
 修改记录：  
    2012.03.15   王治国
    1. 如果驱动器型号(DRIVER_TYPE * 1000)或非标项目号(NONSTANDARD_PROJECT * 100)改变
       Eeprom芯片当做第一次使用处理.
    2. 如果标准项目软件版本号(VERSION_STANDARD)或非标项目软件版本号(VERSION_NONSTANDARD)改变
       上电初始化时,会自动复位H00和H01组之外的功能码
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_FunCode.h"
#include "FUNC_AuxFunCode.h"
#include "FUNC_FunCodeDefault.h" 
#include "FUNC_GlobalVariable.h"
#include "FUNC_OperEeprom.h"
#include "FUNC_DriverPara.h"
#include "FUNC_MotorPara.h"
#include "FUNC_ErrorCode.h"
#include "FUNC_WWDG.h" 
#include "PUB_Table.h"
#include "CANopen_OD.h"
#include "CANopen_InterFace.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/
#define EEPROM_CHECK_WORD1      (0x010E + DRIVER_TYPE * 1000)           // EEPROM校验字1
#define EEPROM_CHECK_WORD2      (Uint16)(0xFFFF - EEPROM_CHECK_WORD1)   // EEPROM校验字2

#define EEPROM_CHECK_WORD3      (0x010E + NONSTANDARD_PROJECT)          // EEPROM校验字3
#define EEPROM_CHECK_WORD4      (Uint16)(0xFFFF - EEPROM_CHECK_WORD3)   // EEPROM校验字4

//用于校验存储是否异常的数组长度
#define MEM_CHECK_NUM     (0x1E - 2)

/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */
//功能码定义
UNI_CODETABLE       FunCodeUnion;

//辅助功能码定义
AUXFUNCODETABLE     AuxFunCodeUnion; 

//每组首个功能码的地址(包含辅助功能码)
const Uint16 * FunCode_GroupStartAddr[AUXFUNGROUP_ENDINDEX + 1] = {
    (Uint16 *)&FunCodeUnion.group.H00[0],
    (Uint16 *)&FunCodeUnion.group.H01[0],
    (Uint16 *)&FunCodeUnion.group.H02[0],
    (Uint16 *)&FunCodeUnion.group.H03[0],
    (Uint16 *)&FunCodeUnion.group.H04[0],
    (Uint16 *)&FunCodeUnion.group.H05[0],
    (Uint16 *)&FunCodeUnion.group.H06[0],
    (Uint16 *)&FunCodeUnion.group.H07[0],
    (Uint16 *)&FunCodeUnion.group.H08[0],
    (Uint16 *)&FunCodeUnion.group.H09[0],
    (Uint16 *)&FunCodeUnion.group.H0A[0],
    (Uint16 *)&AuxFunCodeUnion.group.H0B[0],        //辅助功能码
    (Uint16 *)&FunCodeUnion.group.H0C[0],
    (Uint16 *)&AuxFunCodeUnion.group.H0D[0],        //辅助功能码
    (Uint16 *)&FunCodeUnion.group.H0E[0],
    (Uint16 *)&FunCodeUnion.group.H0F[0],
    (Uint16 *)&FunCodeUnion.group.H10[0],
    (Uint16 *)&FunCodeUnion.group.H11[0],
    (Uint16 *)&FunCodeUnion.group.H12[0],
    (Uint16 *)&FunCodeUnion.group.H13[0],
    (Uint16 *)&FunCodeUnion.group.H14[0],
    (Uint16 *)&FunCodeUnion.group.H15[0],
    (Uint16 *)&FunCodeUnion.group.H16[0],
    (Uint16 *)&FunCodeUnion.group.H17[0],
    (Uint16 *)&FunCodeUnion.group.H18[0], 
    (Uint16 *)&FunCodeUnion.group.H19[0], 
    (Uint16 *)&FunCodeUnion.group.H1A[0], 
    (Uint16 *)&FunCodeUnion.group.H1B[0], 
    (Uint16 *)&FunCodeUnion.group.H1C[0], 
    (Uint16 *)&FunCodeUnion.group.H1D[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H1E[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H1F[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H20[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H21[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H22[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H23[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H24[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H25[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H26[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H27[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H28[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H29[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H2A[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H2B[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H2C[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H2D[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],  //(Uint16 *)&FunCodeUnion.group.H2E[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H2F[0], 
    (Uint16 *)&AuxFunCodeUnion.group.H30[0],
    (Uint16 *)&AuxFunCodeUnion.group.H31[0],
    (Uint16 *)&AuxFunCodeUnion.group.H32[0],
};

//每组功能码末尾功能码序号(包含辅助功能码)
const Uint16 FunCode_GroupEndIndex[AUXFUNGROUP_ENDINDEX + 1] = {
    H00LEN - 1,
    H01LEN - 1,
    H02LEN - 1,
    H03LEN - 1,
    H04LEN - 1,
    H05LEN - 1,
    H06LEN - 1,
    H07LEN - 1,
    H08LEN - 1,
    H09LEN - 1,
    H0ALEN - 1,
    H0BLEN - 1,                //辅助功能码
    H0CLEN - 1,
    H0DLEN - 1,                //辅助功能码
    H0ELEN - 1,
    H0FLEN - 1,
    H10LEN - 1,
    H11LEN - 1,
    H12LEN - 1,
    H13LEN - 1,
    H14LEN - 1,
    H15LEN - 1,
    H16LEN - 1,
    H17LEN - 1,
    H18LEN - 1,
    H19LEN - 1,
    H1ALEN - 1,
    H1BLEN - 1,
    H1CLEN - 1,
    H1DLEN - 1,
    0,  //H1ELEN - 1,
    0,  //H1FLEN - 1,
    0,  //H20LEN - 1,
    0,  //H21LEN - 1,
    0,  //H22LEN - 1,
    0,  //H23LEN - 1,
    0,  //H24LEN - 1,
    0,  //H25LEN - 1,
    0,  //H26LEN - 1,
    0,  //H27LEN - 1,
    0,  //H28LEN - 1,
    0,  //H29LEN - 1,
    0,  //H2ALEN - 1,
    0,  //H2BLEN - 1,
    0,  //H2CLEN - 1,
    0,  //H2DLEN - 1,
    0,  //H2ELEN - 1,
//以下是辅助功能码
    H2FLEN - 1,
    H30LEN - 1,
    H31LEN - 1,
    H32LEN - 1,
};

//功能码属性表中每组首个功能码属性的序号(包含辅助功能码)
const Uint16 FunCodeDeft_GroupStartIndex[AUXFUNGROUP_ENDINDEX + 1] = {
    H0000DFTINDEX,
    H0100DFTINDEX,
    H0200DFTINDEX,
    H0300DFTINDEX,
    H0400DFTINDEX,
    H0500DFTINDEX,
    H0600DFTINDEX,
    H0700DFTINDEX,
    H0800DFTINDEX,
    H0900DFTINDEX,
    H0A00DFTINDEX, 
    H0B00DFTINDEX,
    H0C00DFTINDEX,
    H0D00DFTINDEX,
    H0E00DFTINDEX,
    H0F00DFTINDEX,
    H1000DFTINDEX,
    H1100DFTINDEX,
    H1200DFTINDEX,
    H1300DFTINDEX,
    H1400DFTINDEX,
    H1500DFTINDEX,
    H1600DFTINDEX,
    H1700DFTINDEX,
    H1800DFTINDEX,
    H1900DFTINDEX,
    H1A00DFTINDEX,
    H1B00DFTINDEX,
    H1C00DFTINDEX,
    H1D00DFTINDEX,
    0,  //H1E00DFTINDEX,
    0,  //H1F00DFTINDEX,
    0,  //H2000DFTINDEX,
    0,  //H2100DFTINDEX,
    0,  //H2200DFTINDEX,
    0,  //H2300DFTINDEX,
    0,  //H2400DFTINDEX,
    0,  //H2500DFTINDEX,
    0,  //H2600DFTINDEX,
    0,  //H2700DFTINDEX,
    0,  //H2800DFTINDEX,
    0,  //H2900DFTINDEX,
    0,  //H2A00DFTINDEX,
    0,  //H2B00DFTINDEX,
    0,  //H2C00DFTINDEX,
    0,  //H2D00DFTINDEX,
    0,  //H2E00DFTINDEX,
    H2F00DFTINDEX,
    H3000DFTINDEX,
    H3100DFTINDEX,
    H3200DFTINDEX,
};

//功能码结构体中,每组首个功能码属性的序号(不包含辅助功能码)
const Uint16 FunCode_GroupStartIndex[FUNGROUP_ENDINDEX + 1] = {
    H0000INDEX,
    H0100INDEX,
    H0200INDEX,
    H0300INDEX,
    H0400INDEX,
    H0500INDEX,
    H0600INDEX,
    H0700INDEX,
    H0800INDEX,
    H0900INDEX,
    H0A00INDEX, 
    2048,
    H0C00INDEX,
    2048,
    H0E00INDEX,
    H0F00INDEX,
    H1000INDEX,
    H1100INDEX,
    H1200INDEX,
    H1300INDEX,
    H1400INDEX,
    H1500INDEX,
    H1600INDEX,
    H1700INDEX,
    H1800INDEX,
    H1900INDEX,
    H1A00INDEX,
    H1B00INDEX,
    H1C00INDEX,
    H1D00INDEX,
};

//面板显示每组功能码末尾序号
const Uint8 FunCode_PanelDispLen[AUXFUNGROUP_ENDINDEX + 1] = {
    H00_PANELDISPLEN,
    H01_PANELDISPLEN,
    H02_PANELDISPLEN,
    H03_PANELDISPLEN,
    H04_PANELDISPLEN,
    H05_PANELDISPLEN,
    H06_PANELDISPLEN,
    H07_PANELDISPLEN,
    H08_PANELDISPLEN,
    H09_PANELDISPLEN,
    H0A_PANELDISPLEN,
    H0B_PANELDISPLEN,
    H0C_PANELDISPLEN,
    H0D_PANELDISPLEN,
    H0E_PANELDISPLEN,
    H0F_PANELDISPLEN,
    H10_PANELDISPLEN,
    H11_PANELDISPLEN,
    H12_PANELDISPLEN,
    H13_PANELDISPLEN,
    H14_PANELDISPLEN,
    H15_PANELDISPLEN,
    H16_PANELDISPLEN,
    H17_PANELDISPLEN,
    H18_PANELDISPLEN,
    H19_PANELDISPLEN,
    H1A_PANELDISPLEN,
    H1B_PANELDISPLEN,
    H1C_PANELDISPLEN,
    H1D_PANELDISPLEN,
    0,  //H1E_PANELDISPLEN,
    0,  //H1F_PANELDISPLEN,
    0,  //H20_PANELDISPLEN,
    0,  //H21_PANELDISPLEN,
    0,  //H22_PANELDISPLEN,
    0,  //H23_PANELDISPLEN,
    0,  //H24_PANELDISPLEN,
    0,  //H25_PANELDISPLEN,
    0,  //H26_PANELDISPLEN,
    0,  //H27_PANELDISPLEN,
    0,  //H28_PANELDISPLEN,
    0,  //H29_PANELDISPLEN,
    0,  //H2A_PANELDISPLEN,
    0,  //H2B_PANELDISPLEN,
    0,  //H2C_PANELDISPLEN,
    0,  //H2D_PANELDISPLEN,
    0,  //H2E_PANELDISPLEN,
    H2F_PANELDISPLEN,
    H30_PANELDISPLEN,
    H31_PANELDISPLEN,
    H32_PANELDISPLEN,
};

//用于校验的功能码的位置
const Uint16 CheckWordIndex[MEM_CHECK_NUM] = {
    GetCodeIndex(FunCodeUnion.code.MemCheck00),
    GetCodeIndex(FunCodeUnion.code.MemCheck01),
    GetCodeIndex(FunCodeUnion.code.MemCheck02),
    GetCodeIndex(FunCodeUnion.code.MemCheck03),
    GetCodeIndex(FunCodeUnion.code.MemCheck04),
    GetCodeIndex(FunCodeUnion.code.MemCheck05),
    GetCodeIndex(FunCodeUnion.code.MemCheck06),
    GetCodeIndex(FunCodeUnion.code.MemCheck07),
    GetCodeIndex(FunCodeUnion.code.MemCheck08),
    GetCodeIndex(FunCodeUnion.code.MemCheck09),
    GetCodeIndex(FunCodeUnion.code.MemCheck0A),
    GetCodeIndex(FunCodeUnion.code.MemCheck0C),
    GetCodeIndex(FunCodeUnion.code.MemCheck0E),
    GetCodeIndex(FunCodeUnion.code.MemCheck0F),
    GetCodeIndex(FunCodeUnion.code.MemCheck10),
    GetCodeIndex(FunCodeUnion.code.MemCheck11),
    GetCodeIndex(FunCodeUnion.code.MemCheck12),
    GetCodeIndex(FunCodeUnion.code.MemCheck13),
    GetCodeIndex(FunCodeUnion.code.MemCheck14),
    GetCodeIndex(FunCodeUnion.code.MemCheck15),
    GetCodeIndex(FunCodeUnion.code.MemCheck16),
    GetCodeIndex(FunCodeUnion.code.MemCheck17),
    GetCodeIndex(FunCodeUnion.code.MemCheck18),
    GetCodeIndex(FunCodeUnion.code.MemCheck19),
    GetCodeIndex(FunCodeUnion.code.MemCheck1A),
    GetCodeIndex(FunCodeUnion.code.MemCheck1B),
    GetCodeIndex(FunCodeUnion.code.MemCheck1C),
    GetCodeIndex(FunCodeUnion.code.MemCheck1D),
};

const Uint16 CheckWordDftIndex[MEM_CHECK_NUM] = {
    GetCodeIndex(FunCodeUnion.code.MemCheck00) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck01) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck02) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck03) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck04) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck05) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck06) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck07) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck08) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck09) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck0A) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck0C) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck0E) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck0F) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck10) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck11) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck12) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck13) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck14) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck15) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck16) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck17) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck18) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck19) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck1A) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck1B) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck1C) - H0000INDEX,
    GetCodeIndex(FunCodeUnion.code.MemCheck1D) - H0000INDEX,
};

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void InitFunCode(void);
void ResetFunCode(void);
void InitElcLabel(void);
void ElcLabelDeal(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 
Static_Inline void FindFunCodeGroupOffset(Uint16 Index);



/*******************************************************************************
  函数名:  
  输入:   无 
  输出:   无 
  子函数: 无
  描述：
    1.
    2.
********************************************************************************/
void InitFunCode(void)
{
    Uint32 Counter = 0;
    Uint64 Temp = 0;
    Uint64 Temp_1 = 0;
    Uint8  EepromCheckCnt = 0;
    Uint8  ParaErrFlag = 0;

    //读取EEPROM检验字
    do
    {
        ReadFromEeprom(0,3);
        ReadFromEeprom(CheckWordIndex[0],CheckWordIndex[0]);
        ReadFromEeprom(CheckWordIndex[1],CheckWordIndex[1]);
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 1000000) break;
        }

        if((FunCodeUnion.code.EepromCheckWord1 == EEPROM_CHECK_WORD1) && 
           (FunCodeUnion.code.EepromCheckWord2 == EEPROM_CHECK_WORD2) &&
           (FunCodeUnion.code.EepromCheckWord3 == EEPROM_CHECK_WORD3) && 
           (FunCodeUnion.code.EepromCheckWord4 == EEPROM_CHECK_WORD4) &&
           (FunCodeUnion.code.MemCheck00 == GetCodeDftValue(CheckWordDftIndex[0])) &&
           (FunCodeUnion.code.MemCheck01 == GetCodeDftValue(CheckWordDftIndex[1])) )
        {
            EepromCheckCnt = 10;
        }
        else if((FunCodeUnion.code.EepromCheckWord1 != EEPROM_CHECK_WORD1) || 
           (FunCodeUnion.code.EepromCheckWord2 != EEPROM_CHECK_WORD2) ||
           (FunCodeUnion.code.EepromCheckWord3 != EEPROM_CHECK_WORD3) || 
           (FunCodeUnion.code.EepromCheckWord4 != EEPROM_CHECK_WORD4) ||
           (FunCodeUnion.code.MemCheck00 != GetCodeDftValue(CheckWordDftIndex[0])) ||
           (FunCodeUnion.code.MemCheck01 != GetCodeDftValue(CheckWordDftIndex[1])) )
        {
            EepromCheckCnt ++;
            DELAY_US(100);
        }

    }while(EepromCheckCnt < 4);

    //先初始化辅助功能码
    for(Counter=0;Counter<AUXFUNCODELEN;Counter++)
    {
        AuxFunCodeUnion.all[Counter] = GetCodeDftValue(Counter + H0B00DFTINDEX);
    }

    if(EepromCheckCnt == 4)
    {
        //Eeprom芯片被第一次使用
        //---------------------------- H00[],H01[]...H1E[],H1D[] ----------------------------
        //获取各个组的功能码默认值
        for(Counter=H0000INDEX;Counter<HRsvd00INDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = GetCodeDftValue(Counter - H0000INDEX);
        }

        FunCodeUnion.code.MT_RsdAbsRomMotorModel = 0;
        FunCodeUnion.code.MT_RsdMotorModel = 0;

        //初始化编码器版本号 
        FunCodeUnion.code.MT_ABSEncVer = 0xFFFF;

        //支持绝对编码器
        if(14 == (FunCodeUnion.code.MT_MotorModel / 1000))
        {
            FunCodeUnion.code.MT_RsdMotorModel = FunCodeUnion.code.MT_MotorModel;

            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_RsdMotorModel));

            //处理驱动器参数
            DealDriverPara();
            //带绝对编码器电机
            Pub_AbsRomInit();

            FunCodeUnion.code.MT_RsdAbsRomMotorModel = FunCodeUnion.code.MT_AbsRomMotorModel;
        }
        else
        {
            //处理驱动器参数
            DealDriverPara();
            //处理电机参数
            DealMotorPara();
        }

        //根据编码器分辨率设置电子齿轮比 H05_07 H05_09 H05_011 H05_13        
        //FPGA速度平均值滤波使能H08_22 速度反馈低通滤波截止频率H08_23   速度反馈选择H08_25
        if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0)    //省线式编码器
        {
            FunCodeUnion.code.PL_PosFirCmxLow = 4;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = 0;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 1;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H

            FunCodeUnion.code.PL_PosSecCmxLow = 4;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = 0;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 1;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H 

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率
            FunCodeUnion.code.GN_SpdFbSel = 0;                 //H08_25速度反馈选择
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x10)    //绝对式编码器
        {
            FunCodeUnion.code.PL_PosFirCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 10000;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H
            FunCodeUnion.code.PL_PosSecCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 10000;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H        

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率        
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x20)    //旋变
        {
            FunCodeUnion.code.PL_PosFirCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 10000;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H
            FunCodeUnion.code.PL_PosSecCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 10000;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H 
                    
            FunCodeUnion.code.GN_SpdFdbFilt_On = 4;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择                   
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30)    //光栅尺
        {
            FunCodeUnion.code.PL_PosFirCmxLow = 1;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = 0;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 1;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H

            FunCodeUnion.code.PL_PosSecCmxLow = 1;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = 0;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 1;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H        

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 1000;           //H08_23 速度反馈低通滤波截止频率 
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择                   
        }

        #if NONSTANDARD_PROJECT == LINEARMOT
            FunCodeUnion.code.PL_PosReachValue = 20;            //H05_21 定位完成幅度
            FunCodeUnion.code.ER_PerrFaultVluLow = 32767;       //H0A10 位置偏差过大故障设定值低16位
            FunCodeUnion.code.ER_PerrFaultVluHigh = 0;          //H0A11 位置偏差过大故障设定值高16位 

        #else
            //根据编码器分辨率设置定位完成幅度H05_21   位置偏差过大故障设定值H0A10 H0A11
            Temp = (Uint64)((Uint32)FunCodeUnion.code.MT_EncoderPensH << 16) + FunCodeUnion.code.MT_EncoderPensL;
            if(0 == (FunCodeUnion.code.MT_EncoderSel & 0x0f0)) Temp = Temp << 2;

            Temp_1 = ((Uint64)7L * Temp) / (Uint64)10000L;
            FunCodeUnion.code.PL_PosReachValue = (Uint16)Temp_1;            //H05_21 定位完成幅度
             
            Temp_1 = ((Uint64)32767L * Temp) / (Uint64)10000L;         
            FunCodeUnion.code.ER_PerrFaultVluLow = (Uint16)Temp_1;                      //H0A10 位置偏差过大故障设定值低16位
            FunCodeUnion.code.ER_PerrFaultVluHigh = (Uint16)((Uint32)Temp_1 >> 16);     //H0A11 位置偏差过大故障设定值高16位 
        #endif


#if ECT_ENABLE_SWITCH
        //---------------------------- H1C00_Rsvd[] 和H1D00_Rsvd[]----------------------------
        AppDataToDefultValue();
        //然后付给对象字典
        ReadAppDataFromEeprom();
#endif
        SaveToEepromAll();
        
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }
        
        //---------------------------- HRsvd_Rsvd[] ----------------------------
        //HRsvd_Rsvd组的功能码置零
        for(Counter=HRsvd00INDEX;Counter<HEL00INDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = 0;
        }

        //存储HRsvd_Rsvd组的功能码
        SaveToEepromSeri(HRsvd00INDEX,(HEL00INDEX - 1));
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }

        //---------------------------- EL_SN[] ----------------------------
        //HRsvd_Rsvd组的功能码置零
        for(Counter=HEL00INDEX;Counter<HErrSvINDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = 0;
        }

        //存储HRsvd_Rsvd组的功能码
        SaveToEepromSeri(HRsvd00INDEX,(HErrSvINDEX - 1));
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }

        //---------------------------------- HErrSv[] ----------------------------------
        //HErrSv组的功能码置零
        for(Counter=HErrSvINDEX;Counter<HOffSv00INDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = 0;
        }

        //存储HErrSv组的功能码
        SaveToEepromSeri(HErrSvINDEX,(HOffSv00INDEX - 1));
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }

        //---------------------------------- HOffSv[] ----------------------------------
        //HOffSv组的功能码置零
        for(Counter=HOffSv00INDEX;Counter<HIAP00INDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = 0;
        }

        //存储HOffSv组的功能码
        SaveToEepromSeri(HOffSv00INDEX,(HIAP00INDEX - 1));
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }

        //读取HIAP组的功能码
        ReadFromEeprom(HIAP00INDEX, (HIAP00INDEX + IAPLEN - 1));

        //------------------------ EepromCheckWord[] ------------------------
        //初始化H00组前面的功能码
        FunCodeUnion.code.EepromCheckWord1 = EEPROM_CHECK_WORD1;
        FunCodeUnion.code.EepromCheckWord2 = EEPROM_CHECK_WORD2;
        FunCodeUnion.code.EepromCheckWord3 = EEPROM_CHECK_WORD3;
        FunCodeUnion.code.EepromCheckWord4 = EEPROM_CHECK_WORD4;

        //----------------------------H00组前面的功能码清零----------------------------
        for(Counter=4;Counter<H0000INDEX;Counter++)
        {
            FunCodeUnion.all[Counter] = 0;
        }

        //存储H00组前面的功能码
        SaveToEepromSeri(0,(H0000INDEX - 1));
        Counter = 0;
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 2000000L) break;
        }
    }
    else
    {
        ReadFromEepromAll();


        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 1000000L) break;
        }
#if ECT_ENABLE_SWITCH
        //然后付给对象字典
        ReadAppDataFromEeprom();
#endif  

        //处理驱动器参数 
        if(FunCodeUnion.code.OEM_ServoSeri != FunCodeUnion.code.OEM_RsdServoSeri) 
        {
            DealDriverPara();
            FunCodeUnion.code.MT_RsdAbsRomMotorModel = 0;
            FunCodeUnion.code.MT_RsdMotorModel = 0; 
        }

        //初始化编码器版本号 
        FunCodeUnion.code.MT_ABSEncVer = 0xFFFF;

        //支持绝对编码器
        if(14 == (FunCodeUnion.code.MT_MotorModel / 1000))
        {
            FunCodeUnion.code.MT_RsdMotorModel = FunCodeUnion.code.MT_MotorModel;

            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_RsdMotorModel));

            //带绝对编码器电机
            Pub_AbsRomInit();

            FunCodeUnion.code.MT_RsdAbsRomMotorModel = FunCodeUnion.code.MT_AbsRomMotorModel;
        }
        else
        {
            FunCodeUnion.code.MT_RsdAbsRomMotorModel = 0;

            //处理电机参数
            if(FunCodeUnion.code.MT_MotorModel != FunCodeUnion.code.MT_RsdMotorModel) 
            {
                if(FunCodeUnion.code.MT_MotorModel != 65535)
                {
                    DealMotorPara();
                }
                else
                {
                    FunCodeUnion.code.MT_RsdMotorModel = 65535;
                    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.MT_RsdMotorModel));
                }                
            }
        }

        //存储H00-H02组的功能码
        SaveToEepromSeri(H0000INDEX,(H0300INDEX - 1));
        while(EepromProcess() == 1)
        {
            Counter++;
            if(Counter >= 1000000L) break;
        }

        //电机与驱动器匹配判断
        if(FunCodeUnion.code.MT_RateCurrent > FunCodeUnion.code.OEM_RateCurrent)
        {
            PostErrMsg(OVDRVRATECUR);
        }
    }

    //不存储在Eeprom但需要初始化的变量
    FunCodeUnion.code.OEM_SoftVersion = VERSION_H0100;
    FunCodeUnion.code.MT_NonStandardVerL = VERSION_H0002;
    FunCodeUnion.code.MT_NonStandardVerH = VERSION_H0003;
    FunCodeUnion.code.OEM_DSPVerBD = VERSION_H0150;
    FunCodeUnion.code.OEM_FPGAVerBD = VERSION_H0151;
#if ECT_ENABLE_SWITCH
    FunCodeUnion.code.BP_EtherCatVer = EL_ECAT_VER;
#endif    
    FunCodeUnion.code.BP_UserPass = 0;

    //H00 H01组功能码范围限制判断
    for(Counter=H0000INDEX;Counter<H0200INDEX; )
    {
        if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.Writable < 2)
        {
            if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.DataBits  == ATTRIB_TWO_WORD)
            {
                Temp = A_SHIFT16_PLUS_B(FunCodeUnion.all[Counter + 1], FunCodeUnion.all[Counter]);
                if(LimitCheck_TwoWords((Counter-H0000INDEX), Temp) != 0)
                {
                    PostErrMsg(INITOEMPARAERR);     //Er.111  厂家参数异常
                    FindFunCodeGroupOffset(Counter);
                    ParaErrFlag = 1;
                    break;
                }
                Counter++;   //32位数据
            }
            else
            {
                //16位数据
                if(LimitCheck_0neWord((Counter-H0000INDEX), FunCodeUnion.all[Counter]) != 0)
                {
                    PostErrMsg(INITOEMPARAERR);   //Er.111  厂家参数异常
                    FindFunCodeGroupOffset(Counter);
                    ParaErrFlag = 1;
                    break;
                }
            }
        }
        Counter++;
    }
    
    //比较功能码属性表中各组校验功能码
    if(ParaErrFlag == 0)
    {
        for(Counter=0; Counter< MEM_CHECK_NUM; Counter++)
        {
            if(FunCodeUnion.all[CheckWordIndex[Counter]] 
                          != GetCodeDftValue(CheckWordIndex[Counter] - H0000INDEX))
            {
               PostErrMsg(INITSYSTMPERR);           //Er.101     系统参数异常
               FindFunCodeGroupOffset(CheckWordIndex[Counter]);
               ParaErrFlag = 1;
            }
        }
    }

    //H01组后面的功能码范围限制判断
    if(ParaErrFlag == 0)
    {
        for(Counter=H0200INDEX;Counter<HRsvd00INDEX; )
        {
            if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.Writable < 2)
            {
                if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.DataBits  == ATTRIB_TWO_WORD)
                {
                    Temp = A_SHIFT16_PLUS_B(FunCodeUnion.all[Counter + 1], FunCodeUnion.all[Counter]);
                    if(LimitCheck_TwoWords((Counter-H0000INDEX), Temp) != 0)
                    {
                        PostErrMsg(INITSYSTMPERR);    //Er.101     系统参数异常
                        FindFunCodeGroupOffset(Counter);
                        break;
                    }
                    Counter++;   //32位数据
                }
                else
                {
                    //16位数据
                    if(LimitCheck_0neWord((Counter-H0000INDEX), FunCodeUnion.all[Counter]) != 0)
                    {
                        PostErrMsg(INITSYSTMPERR);    //Er.101     系统参数异常
                        FindFunCodeGroupOffset(Counter);
                        break;
                    }
                }
            }
            Counter++;
        }
    }

#if(DRIVER_TYPE == SERVO_620N)
    if(4000 == FunCodeUnion.code.OEM_CarrWaveFreq)
    {
        //大功率S620N驱动器 载频修改为4K  而电机电流环按5K载频调试  故修正电流环增益功能码 乘以0.8
        FunCodeUnion.code.OEM_CurIdKpSec = ((Uint32)FunCodeUnion.code.OEM_CurIdKpSec * 4L) /5;          //H01_24 D轴电流环比例增益
        FunCodeUnion.code.OEM_CurIqKpSec = ((Uint32)FunCodeUnion.code.OEM_CurIqKpSec * 4L) /5;          //H01_27 Q轴电流环比例增益
        FunCodeUnion.code.OEM_CapIdKp = ((Uint32)FunCodeUnion.code.OEM_CapIdKp * 4L) /5;             //H01_52 性能优先模式D轴比例增益
        FunCodeUnion.code.OEM_CapIqKp = ((Uint32)FunCodeUnion.code.OEM_CapIqKp * 4L) /5;             //H01_54 性能优先模式Q轴比例增益    
    }
#endif

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
Static_Inline void FindFunCodeGroupOffset(Uint16 Index)
{
    int8 i = 0;

    if(Index < H0C00INDEX)
    {
        for(i = 0x0A;i >= 0;i--)
        {
            if(Index > FunCode_GroupStartIndex[i])
            {
                AuxFunCodeUnion.code.DP_ParaErrGroup = i;
                AuxFunCodeUnion.code.DP_ParaErrOffset = Index - FunCode_GroupStartIndex[i];
                break;
            }
        }
    }
    else if(Index < H0E00INDEX)
    {
        AuxFunCodeUnion.code.DP_ParaErrGroup = 0x0C;
        AuxFunCodeUnion.code.DP_ParaErrOffset = Index - H0C00INDEX;
    }
    else if(Index < HRsvd00INDEX)
    {
        for(i = 0x1D;i >= 0x0E;i--)
        {
            if(Index > FunCode_GroupStartIndex[i])
            {
                AuxFunCodeUnion.code.DP_ParaErrGroup = i;
                AuxFunCodeUnion.code.DP_ParaErrOffset = Index - FunCode_GroupStartIndex[i];
                break;
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
void ResetFunCode(void)
{
    Uint16 Counter = 0;
    Uint64 Temp = 0;
    Uint64 Temp_1 = 0;
    static Uint16 DataBuffer[2] = {0,0}; 

    if(STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 0)  //0:未复位
    {
        if(FunCodeUnion.code.BP_InitServo != 1)  return;

        //伺服运行时不可复位功能码
        if(STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN)
        {
            FunCodeUnion.code.BP_InitServo = 0;
            return;
        }

        STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode = 1;

        //锁存密码类的功能码
        DataBuffer[0] = FunCodeUnion.code.MT_EnVisable;         //H02_40 电机组参数可见使能
        DataBuffer[1] = FunCodeUnion.code.OEM_OEMPass;          //H02_41 厂家密码
    
#if ECT_ENABLE_SWITCH
        //将H00组和H01组之外的功能码恢复为默认值
        for(Counter=H0200INDEX;Counter<H1C00INDEX;Counter++)
        {
            if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.Writable != ATTRIB_DISP_WRT)
            {
                FunCodeUnion.all[Counter] = GetCodeDftValue(Counter - H0000INDEX);
            }
            //该循环运行时间要几个ms,需要喂狗
            ServiceDog();
        }

        //将H1C和H1D组功能码恢复至402默认值
        AppDataToDefultValue(); 
        
        //然后付给对象字典
        ReadAppDataFromEeprom();
        
#else
        //将H00组和H01组之外的功能码恢复为默认值
        for(Counter=H0200INDEX;Counter<HRsvd00INDEX;Counter++)
        {
            if(FunCodeDeft[Counter - H0000INDEX].Attrib.bit.Writable != ATTRIB_DISP_WRT)
            {
                FunCodeUnion.all[Counter] = GetCodeDftValue(Counter - H0000INDEX);
            }
            //该循环运行时间要几个ms,需要喂狗
            ServiceDog();
        }
#endif
        //根据编码器分辨率设置电子齿轮比 H05_07 H05_09 H05_011 H05_13        
        //FPGA速度平均值滤波使能H08_22 速度反馈低通滤波截止频率H08_23   速度反馈选择H08_25
        if((FunCodeUnion.code.MT_EncoderSel & 0x0f0) == 0)    //省线式编码器
        {
            FunCodeUnion.code.PL_PosFirCmxLow = 4;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = 0;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 1;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H

            FunCodeUnion.code.PL_PosSecCmxLow = 4;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = 0;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 1;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H 

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率
            FunCodeUnion.code.GN_SpdFbSel = 0;                 //H08_25速度反馈选择
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x10)    //绝对式编码器
        {
            FunCodeUnion.code.PL_PosFirCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 10000;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H
            FunCodeUnion.code.PL_PosSecCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 10000;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H        

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率        
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x20)    //旋变
        {
            FunCodeUnion.code.PL_PosFirCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 10000;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H
            FunCodeUnion.code.PL_PosSecCmxLow = FunCodeUnion.code.MT_EncoderPensL;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = FunCodeUnion.code.MT_EncoderPensH;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 10000;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H 
                    
            FunCodeUnion.code.GN_SpdFdbFilt_On = 4;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 4000;           //H08_23 速度反馈低通滤波截止频率
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择                   
        }
        else if((FunCodeUnion.code.MT_EncoderSel & 0xf0) == 0x30)    //光栅尺
        {
            FunCodeUnion.code.PL_PosFirCmxLow = 1;             //H05_07 电子齿数比1 分子 L
            FunCodeUnion.code.PL_PosFirCmxHigh = 0;            //H05_08 电子齿数比1 分子 H
            FunCodeUnion.code.PL_PosFirCdvLow = 1;             //H05_09 电子齿数比1 分母 L
            FunCodeUnion.code.PL_PosFirCdvHigh = 0;            //H05_10 电子齿数比1 分母 H

            FunCodeUnion.code.PL_PosSecCmxLow = 1;             //H05_11 电子齿数比2 分子 L
            FunCodeUnion.code.PL_PosSecCmxHigh = 0;            //H05_12 电子齿数比2 分子 H
            FunCodeUnion.code.PL_PosSecCdvLow = 1;             //H05_13 电子齿数比2 分母 L
            FunCodeUnion.code.PL_PosSecCdvHigh = 0;            //H05_14 电子齿数比2 分母 H        

            FunCodeUnion.code.GN_SpdFdbFilt_On = 0;            //H08_22 FPGA速度平均值滤波使能  默认为0           
            FunCodeUnion.code.GN_SpdLpFiltFc = 1000;           //H08_23 速度反馈低通滤波截止频率 
            FunCodeUnion.code.GN_SpdFbSel = 1;                 //H08_25速度反馈选择                   
        }

        #if NONSTANDARD_PROJECT == LINEARMOT
            FunCodeUnion.code.PL_PosReachValue = 20;            //H05_21 定位完成幅度
            FunCodeUnion.code.ER_PerrFaultVluLow = 32767;       //H0A10 位置偏差过大故障设定值低16位
            FunCodeUnion.code.ER_PerrFaultVluHigh = 0;          //H0A11 位置偏差过大故障设定值高16位 

        #else
            //根据编码器分辨率设置定位完成幅度H05_21   位置偏差过大故障设定值H0A10 H0A11
            Temp = (Uint64)((Uint32)FunCodeUnion.code.MT_EncoderPensH << 16) + FunCodeUnion.code.MT_EncoderPensL;
            if(0 == (FunCodeUnion.code.MT_EncoderSel & 0x0f0)) Temp = Temp << 2;

            Temp_1 = ((Uint64)7L * Temp) / (Uint64)10000L;
            FunCodeUnion.code.PL_PosReachValue = (Uint16)Temp_1;            //H05_21 定位完成幅度
             
            Temp_1 = ((Uint64)32767L * Temp) / (Uint64)10000L;         
            FunCodeUnion.code.ER_PerrFaultVluLow = (Uint16)Temp_1;                      //H0A10 位置偏差过大故障设定值低16位
            FunCodeUnion.code.ER_PerrFaultVluHigh = (Uint16)((Uint32)Temp_1 >> 16);     //H0A11 位置偏差过大故障设定值高16位 
        #endif

        //恢复校验字
        FunCodeUnion.code.EepromCheckWord1 = 0;
        FunCodeUnion.code.EepromCheckWord2 = 0;
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.EepromCheckWord1));
        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.EepromCheckWord2));
        
        SaveToEepromSeri(H0200INDEX,(HRsvd00INDEX - 1));
        
        EepromProcess();
    }
    else     //1：复位中
    {
        if(EepromProcess() != 1)     //存储完成
        {
            //恢复校验字
            FunCodeUnion.code.EepromCheckWord1 = EEPROM_CHECK_WORD1;
            FunCodeUnion.code.EepromCheckWord2 = EEPROM_CHECK_WORD2;
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.EepromCheckWord1));
            SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.EepromCheckWord2));


            //复位完成
            STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode = 0;

            //恢复锁存的变量
            FunCodeUnion.code.MT_EnVisable = DataBuffer[0];                 //H02_40 电机组参数可见使能
            FunCodeUnion.code.OEM_OEMPass = DataBuffer[1];                  //H02_41 厂家密码

            PostErrMsg(PCHGDWARN);
        }
    }

}

/********************************* END OF FILE *********************************/
