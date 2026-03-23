/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_FunCodeDefault.h           
 创建人：童文邹                创建日期：2008.10.09 
 修改人：王治国                创建日期：2011.11.15 
 描述： 
    1. 速度控制头文件
    
 修改记录：  
    xx.xx.xx      XX
    1.      
    2. 
********************************************************************************/
#ifndef __FUNC_FUNCODEDEFAULT_H
#define __FUNC_FUNCODEDEFAULT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_GlobalPrototypes.h"
#include "FUNC_FunCode.h"
#include "FUNC_AuxFunCode.h"

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

//功能码属性表中每组首个功能码序号、
#define      H0000DFTINDEX       0
#define      H0100DFTINDEX       (H0000DFTINDEX + H00LEN)
#define      H0200DFTINDEX       (H0100DFTINDEX + H01LEN)
#define      H0300DFTINDEX       (H0200DFTINDEX + H02LEN)
#define      H0400DFTINDEX       (H0300DFTINDEX + H03LEN)
#define      H0500DFTINDEX       (H0400DFTINDEX + H04LEN)
#define      H0600DFTINDEX       (H0500DFTINDEX + H05LEN)
#define      H0700DFTINDEX       (H0600DFTINDEX + H06LEN)
#define      H0800DFTINDEX       (H0700DFTINDEX + H07LEN)
#define      H0900DFTINDEX       (H0800DFTINDEX + H08LEN)
#define      H0A00DFTINDEX       (H0900DFTINDEX + H09LEN)
//H0B组在辅助功能码中定义
#define      H0C00DFTINDEX       (H0A00DFTINDEX + H0ALEN)
//H0D组在辅助功能码中定义
#define      H0E00DFTINDEX       (H0C00DFTINDEX + H0CLEN)
#define      H0F00DFTINDEX       (H0E00DFTINDEX + H0ELEN)
#define      H1000DFTINDEX       (H0F00DFTINDEX + H0FLEN)
#define      H1100DFTINDEX       (H1000DFTINDEX + H10LEN)
#define      H1200DFTINDEX       (H1100DFTINDEX + H11LEN)
#define      H1300DFTINDEX       (H1200DFTINDEX + H12LEN)
#define      H1400DFTINDEX       (H1300DFTINDEX + H13LEN)
#define      H1500DFTINDEX       (H1400DFTINDEX + H14LEN)
#define      H1600DFTINDEX       (H1500DFTINDEX + H15LEN)
#define      H1700DFTINDEX       (H1600DFTINDEX + H16LEN)
#define      H1800DFTINDEX       (H1700DFTINDEX + H17LEN)
#define      H1900DFTINDEX       (H1800DFTINDEX + H18LEN)
#define      H1A00DFTINDEX       (H1900DFTINDEX + H19LEN)
#define      H1B00DFTINDEX       (H1A00DFTINDEX + H1ALEN)
#define      H1C00DFTINDEX       (H1B00DFTINDEX + H1BLEN)
#define      H1D00DFTINDEX       (H1C00DFTINDEX + H1CLEN)

//辅助功能码
#define      H0B00DFTINDEX       (H1D00DFTINDEX + H1DLEN)
#define      H0D00DFTINDEX       (H0B00DFTINDEX + H0BLEN)
#define      H2F00DFTINDEX       (H0D00DFTINDEX + H0DLEN)
#define      H3000DFTINDEX       (H2F00DFTINDEX + H2FLEN)
#define      H3100DFTINDEX       (H3000DFTINDEX + H30LEN)
#define      H3200DFTINDEX       (H3100DFTINDEX + H31LEN)

//功能码属性表长度
#define      FUNCODEDFTLEN       (H3200DFTINDEX + H32LEN)

//bit.Writable
#define     ATTRIB_ANY_WRT     0      //随时设定
#define     ATTRIB_POSD_WRT    1      //停机时设定
#define     ATTRIB_DISP_WRT    2      //仅显示 只读
#define     ATTRIB_RSVD_WRT    3      //保留参数
//bit.UpperLmt 
#define     ATTRIB_RLAT_LMTH   1      //上限 关联限制
//bit.LowerLmt
#define     ATTRIB_RLAT_LMTL   1      //下限 关联限制
//bit.DataBits
#define     ATTRIB_ONE_WORD    0      //16位数据 
#define     ATTRIB_TWO_WORD    1      //32位数据

//bit.DotBit
#define     ATTRIB_ONE_DOT     1      //1位小数位
#define     ATTRIB_TWO_DOT     2      //2位小数位
#define     ATTRIB_THREE_DOT   3      //3位小数位

//bit.Sign
#define     ATTRIB_INT_SIGN    1      //有符号
//bit.Active
#define     ATTRIB_NEXT_ACT    1      //下次上电生效
//bit.DataType
#define     ATTRIB_YNUM_TYP    0     //0:十进制数据
#define     ATTRIB_HNUM_TYP    1     //1:十六进制数据
#define     ATTRIB_NNUM_TYP    2     //2:非数字数据(Di Do显示)
//bit.DataIndex
#define     ATTRIB_LOW_WORD     0         //32位数据低十六位或16位数据
#define     ATTRIB_HIGH_WORD    1        //32位数据高十六位

//bit.OEMProtect
#define     ATTRIB_OEMPROTECT      1    //1:厂家保护
//bit.PanelAttrib
#define     ATTRIB_PANEL_UNCHANGE   1            //面板不可更改
#define     ATTRIB_PANEL_UNSAVE     2            //面板可更改功能码但是不存Eeprom
#define     ATTRIB_PANEL_UNREAD     3            //密码类参数,不可以读取,操作后不存Eeprom
//bit.CommSaveEn
#define     ATTRIB_COMM_SAVE_EEPROM     0       //COMM更改功能码后可以存储到EEPROM
#define     ATTRIB_COMM_UNSAVE_EEPROM   1       //COMM更改功能码后不存储到EEPROM
//bit.CommReadEn
#define     ATTRIB_COMM_READ_EN     0          //读时返回真实值
#define     ATTRIB_COMM_READ_DISABVLE   1       //读时返回0

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */
//获取功能码设定属性值
#define     GetAttrib_Writable(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.Writable

//获取功能码上限限制方式属性值
#define     GetAttrib_UpperLmt(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.UpperLmt

//获取功能码下限限制方式属性值
#define     GetAttrib_LowerLmt(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.LowerLmt

//获取功能码字长属性值(16bits/32bits)
#define     GetAttrib_DataBits(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.DataBits

//获取功能码显示位数属性值
#define     GetAttrib_DispBits(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.DispBits

//获取功能码小数点后位数属性值
#define     GetAttrib_DotBit(Group , Offset)            FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.DotBit

//获取功能码符号属性值
#define     GetAttrib_Sign(Group , Offset)              FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.Sign

//获取功能码设定生效属性值
#define     GetAttrib_Active(Group , Offset)            FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.Active

//获取功能码数据显示类型属性值(二进制显示/十进制显示/十六进制显示)
#define     GetAttrib_DataType(Group , Offset)          FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.DataType

//获取功能码数据字序号属性值(低十六位/高十六位)
#define     GetAttrib_DataIndex(Group , Offset)         FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.DataIndex

//获取功能码OEM密码保护属性值(H0241=1430时面板可见)
#define     GetAttrib_OEMProtect(Group , Offset)        FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.OEMProtect

//获取功能码面板设置属性值
#define     GetAttrib_PanelAttrib(Group , Offset)       FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].Attrib.bit.PanelAttrib

//获取功能码默认属性值
#define     GetCodeDftValue(Index)                              FunCodeDeft[Index].Value

//获取功能码下限属性值
#define     GetDftLowerLmt(Group , Offset)                 FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].LowerLmt

//获取功能码上限属性值
#define     GetDftUpperLmt(Group , Offset)                 FunCodeDeft[FunCodeDeft_GroupStartIndex[(Group)] + (Offset)].UpperLmt

//获取功能码值
#define     GetFunCode(Group , Offset)                 (*((Uint16 *)FunCode_GroupStartAddr[(Group)] + (Offset)))

//设置功能码值
#define     SetFunCode(Group , Offset , Value)           (*((Uint16 *)FunCode_GroupStartAddr[(Group)] + (Offset))) = (Uint16)(Value)

//获取功能码结构体成员的序号,提供给Eeprom存储操作
#define     GetGroupCodeIndex(Group , Offset)               (FunCode_GroupStartIndex[(Group)] + (Offset))

//获取功能码属性表数组成员序号
#define     GetGroupCodeDftIndex(Group , Offset)           (FunCodeDeft_GroupStartIndex[(Group)] + (Offset))

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */
typedef struct{            // bits    description
    Uint32  Writable:2;    //0-1     00:RW;01:停机设定  02:显示  03:保留参数
    Uint32  rsvd:1;        //2       保留
    Uint32  UpperLmt:1;    //3       0:直接由上限限制 1:参数由上限相关功能码限制
    Uint32  LowerLmt:1;    //4       0:直接由下限限制 1:参数由下限相关功能码限制
    Uint32  DataBits:1;    //5       0:16位   1:32位
    Uint32  DispBits:4;    //6-9     5个数码管要显示的位数。0:显示0位，1:显示1位...5:显示5位
    Uint32  DotBit:3;      //10-12    0:无小数位  1:1位小数位...4:4位小数位
    Uint32  Sign:1;        //13      0:无符号   1：有符号
    Uint32  Active:1;      //14      0：立即生效   1：再次上电生效
    Uint32  DataType:2;    //15-16   0:非数字数据  1:十进制数据  2:十六进制数据
    Uint32  DataIndex:1;   //17      0:32位数据低十六位或16位数据  1:32位数据高十六位
    Uint32  OEMProtect:1;  //18      0:不用保护  1:厂家保护
    Uint32  PanelAttrib:2; //19-20  0: 无 1:(PANEL_UNCHANGE)面板显示但不可更改
                           // 2:(PANEL_UNSAVE)面板可更改功能码但是不存Eeprom
                           // 3: 密码类参数,不可以读取,操作后不存Eeprom
    Uint32  CommSaveEn:1;  //21  0: 写时存Eeprom 1:写时不存Eeprom
    Uint32  CommReadEn:1;  //22  0: 读时返回真实值 1:读时返回0
    Uint32  Rsvd:9;        //23-31   Rsvd10
}STR_FUNCODE_ATTRIBUTE_BIT;
//Writable = 2时,通讯和面板均不可以更改
//PanelAttrib = 1时,通讯可以更改但面板不可以更改

typedef union
{
    volatile Uint32                       all;
    volatile STR_FUNCODE_ATTRIBUTE_BIT     bit;
}UNI_FUNCODE_ATTRIBUTE;

//在非数字显示类参数中:下限表示显示类别 上限表示显示需要的页数
typedef struct{
    Uint16 Value;
    Uint16 LowerLmt;                 //下限
    Uint16 UpperLmt;                 //上限
    volatile UNI_FUNCODE_ATTRIBUTE Attrib;      //功能码属性
}STR_FUNCODE_DEFAULT;

/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//功能码属性表(包含辅助功能码)
extern const  STR_FUNCODE_DEFAULT     FunCodeDeft[FUNCODEDFTLEN];

//每组首个功能码的地址(包含辅助功能码)
extern const Uint16 *   FunCode_GroupStartAddr[AUXFUNGROUP_ENDINDEX + 1];

//每组功能码末尾功能码序号(包含辅助功能码)
extern const Uint16     FunCode_GroupEndIndex[AUXFUNGROUP_ENDINDEX + 1];

//功能码属性表中每组首个功能码属性的序号(包含辅助功能码)
extern const Uint16     FunCodeDeft_GroupStartIndex[AUXFUNGROUP_ENDINDEX + 1];

//功能码结构体中,每组首个功能码属性的序号(不包含辅助功能码)
extern const Uint16     FunCode_GroupStartIndex[FUNGROUP_ENDINDEX + 1];

//面板显示每组功能码末尾序号
extern const Uint8 FunCode_PanelDispLen[AUXFUNGROUP_ENDINDEX + 1];

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern Uint8 LimitCheck_0neWord(Uint16 FunCodeDeftIndex, Uint16 DataInput);
extern Uint8 LimitCheck_TwoWords(Uint16 FunCodeDeftIndex, Uint32 DataInput);


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_FUNCODEDEFAULT_H */

/********************************* END OF FILE *********************************/
