/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_COMMInterface.c
 创建人:    邓开余,马世贤,韦水平
 修改人:    王治国          创建日期：2011.10.31
 描述:
    1.功能模块与通信模块的接口有功能码和示波器缓冲区数组
    2.通信模块通过本文件的COMMWrRdFuncode来读取功能码
    3.示波器缓冲数组作为全局变量,通信模块直接进行访问
缩写：
    1. comm -> communication
    2. wr -> write
    3. rd -> read
    4. addr -> address
    5. cyc -> cycle
    6. cnt -> count
    7. lmt -> limit
    8. *p -> *pointer
    9. reg -> register
 修改记录：
    1.2012.04.12      王治国
        变更内容：返回故障标志意义更改
        * 0x0000               操作成功        Sucess
        * 0x10                 操作模式错误    OperationModeErr

        * 1.1读操作（命令码0x03）
        * 0x02：地址不存在（读数量为0，超出地址范围，未经过认证时访问厂家参数）
        * 0x04：数据类型不符合（主要针对读32位功能码，32位功能码低16位在前，高16位在后。如果读起始地址是32位功能码高16位或者读末尾地址是32位功能码低16位）
        * 
        * 32位功能码读操作时，由H0C_26控制低16位高16位顺序。
        *   H0C_26=0时，高十六位在前，低十六位在后；
        *   H0C_26=1时，低十六位在前，高十六位在后。
    
        * 1.2写一个功能码操作（命令码0x06）
        * 0x02：地址不存在（超出地址范围，未经过认证时访问厂家参数，只读属性功能码写入数据，中间保留参数写入数据）
        * 0x03：超出上下限，H0241写入错误
        * 0x04：数据类型不符合（写32位功能码高16位或者写32位功能码低16位，停机设定参数在运行状态下写入数据）
    
        * 1.3写多个个功能码操作（命令码0x10）
        * 0x02：地址不存在（超出地址范围，未经过认证时访问厂家参数，只读属性功能码写入数据，中间保留参数写入数据）
        * 0x03：超出上下限，H0241写入错误
        * 0x04：数据类型不符合（主要针对写32位功能码，32位功能码低16位在前，高16位在后。如果写起始地址是32位功能码高16位或者写末尾地址是32位功能码低16位 ，停机设定参数在运行状态下写入数据）
    
        * 32位功能码写操作时，由H0C_26控制低16位高16位顺序。
        *   H0C_26=0时，高十六位在前，低十六位在后；
        *   H0C_26=1时，低十六位在前，高十六位在后。

     2.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_COMMInterface.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_FunCodeDefault.h"
#include "FUNC_OperEeprom.h" 
#include "FUNC_ManageFunCode.h"
#include "FUNC_ErrorCode.h"


/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */

//电子标签数组地址，读时使用
const Uint16 * EleLabelAddr[48] = {
    (Uint16 *)&UNI_EleLabel.all[0],
    (Uint16 *)&UNI_EleLabel.all[1],
    (Uint16 *)&UNI_EleLabel.all[2],
    (Uint16 *)&UNI_EleLabel.all[3],
    (Uint16 *)&UNI_EleLabel.all[4],
    (Uint16 *)&UNI_EleLabel.all[5],
    (Uint16 *)&UNI_EleLabel.all[6],
    (Uint16 *)&UNI_EleLabel.all[7],
    (Uint16 *)&UNI_EleLabel.all[8],
    (Uint16 *)&UNI_EleLabel.all[9],
    (Uint16 *)&UNI_EleLabel.all[10],
    (Uint16 *)&UNI_EleLabel.all[11],
    (Uint16 *)&UNI_EleLabel.all[12],
    (Uint16 *)&UNI_EleLabel.all[13],
    (Uint16 *)&UNI_EleLabel.all[14],
    (Uint16 *)&UNI_EleLabel.all[15],
    (Uint16 *)&UNI_EleLabel.all[16],
    (Uint16 *)&UNI_EleLabel.all[17],
    (Uint16 *)&UNI_EleLabel.all[18],
    (Uint16 *)&UNI_EleLabel.all[19],
    (Uint16 *)&UNI_EleLabel.all[20],
    (Uint16 *)&UNI_EleLabel.all[21],
    (Uint16 *)&UNI_EleLabel.all[22],
    (Uint16 *)&UNI_EleLabel.all[23],
    (Uint16 *)&UNI_EleLabel.all[24],
    (Uint16 *)&UNI_EleLabel.all[25],
    (Uint16 *)&UNI_EleLabel.all[26],
    (Uint16 *)&UNI_EleLabel.all[27],
    (Uint16 *)&UNI_EleLabel.all[28],
    (Uint16 *)&UNI_EleLabel.all[29],
    (Uint16 *)&UNI_EleLabel.all[30],
    (Uint16 *)&UNI_EleLabel.all[31],
    (Uint16 *)&FunCodeUnion.code.EL_SN[0],
    (Uint16 *)&FunCodeUnion.code.EL_SN[1],
    (Uint16 *)&FunCodeUnion.code.EL_SN[2],
    (Uint16 *)&FunCodeUnion.code.EL_SN[3],
    (Uint16 *)&FunCodeUnion.code.EL_SN[4],
    (Uint16 *)&FunCodeUnion.code.EL_SN[5],
    (Uint16 *)&FunCodeUnion.code.EL_SN[6],
    (Uint16 *)&FunCodeUnion.code.EL_SN[7],
    (Uint16 *)&FunCodeUnion.code.EL_SN[8],
    (Uint16 *)&FunCodeUnion.code.EL_SN[9],
    (Uint16 *)&FunCodeUnion.code.EL_SN[10],
    (Uint16 *)&FunCodeUnion.code.EL_SN[11],
    (Uint16 *)&FunCodeUnion.code.EL_SN[12],
    (Uint16 *)&FunCodeUnion.code.EL_SN[13],
    (Uint16 *)&FunCodeUnion.code.EL_SN[14],
    (Uint16 *)&FunCodeUnion.code.EL_SN[15],
};

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
//功能码读、写操作
Uint16 COMMWrRdFuncode(const Uint8 Group_Input,const Uint8 Offset,const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode,Uint8 CommType);

Uint16 COMMWrRdEleLabel(const Uint8 Offset, const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode);


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


/*******************************************************************************
  函数名: extern int32 RxFuncodeFromFUNC(Uint16 Group,  Uint16 Offset, Uint16 Total,
                                         Uint16 *pAddr, Uint8 OperationMode)
  输入:   无 
  输出:   无 
  子函数: 无
    描述：
    1.
    2.
********************************************************************************/
/* OperationMode:
 * NULL          0
 * READSERIES    1
 * WRITESERIES   2
 * WRITEONE      3
*/
/* 返回故障标志
 * 0x0000               操作成功        Sucess
 * 0x10                 操作模式错误    OperationModeErr

 * 1.1读操作（命令码0x03）
 * 0x02：地址不存在（读数量为0，超出地址范围，未经过认证时访问厂家参数）
 * 0x04：数据类型不符合（主要针对读32位功能码，32位功能码低16位在前，高16位在后。如果读起始地址是32位功能码高16位或者读末尾地址是32位功能码低16位）
 * 
 * 32位功能码读操作时，由H0C_26控制低16位高16位顺序。
 *   H0C_26=0时，高十六位在前，低十六位在后；
 *   H0C_26=1时，低十六位在前，高十六位在后。

 * 1.2写一个功能码操作（命令码0x06）
 * 0x02：地址不存在（超出地址范围，未经过认证时访问厂家参数，只读属性功能码写入数据，中间保留参数写入数据）
 * 0x03：超出上下限，H0241写入错误
 * 0x04：数据类型不符合（写32位功能码高16位或者写32位功能码低16位，停机设定参数在运行状态下写入数据）

 * 1.3写多个个功能码操作（命令码0x10）
 * 0x02：地址不存在（超出地址范围，未经过认证时访问厂家参数，只读属性功能码写入数据，中间保留参数写入数据）
 * 0x03：超出上下限，H0241写入错误
 * 0x04：数据类型不符合（主要针对写32位功能码，32位功能码低16位在前，高16位在后。如果写起始地址是32位功能码高16位或者写末尾地址是32位功能码低16位 ，停机设定参数在运行状态下写入数据）

 * 32位功能码写操作时，由H0C_26控制低16位高16位顺序。
 *   H0C_26=0时，高十六位在前，低十六位在后；
 *   H0C_26=1时，低十六位在前，高十六位在后。
 */
/*
 * 例如：组3，有10个成员时
 * Group = 3；Offset 0-9；  
 * Total > 1 且 (Offset + Total - 1) < 9
 * Offset = 9时，Total = 1.
*/

Uint16 COMMWrRdFuncode(const Uint8 Group_Input,const Uint8 Offset,const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode,Uint8 CommType)
{
    Uint16 *pFunCodeAddr = 0;                   //功能码地址
    Uint16 *pWrRdAddr = 0;                      //写入数据地址或读数据来源地址
    Uint16  IndexTemp = 0;                     //临时序号
    Uint16  CycCnt = 0;                     //for循环计数器
    Uint32  FunCodeTemp = 0;                   //功能码
    Uint8   LmtCheck = 0;                     //限值比较结果
    Uint8   Group = 0;
    Uint8   PostEr941_Flag = 0;

    //Group_Input最高位等于1时,写入数据时不存储到Eeprom
    Group = Group_Input & 0x3F;

    if(1 == UNI_EleLabel.Label.JumpFlag) UNI_EleLabel.Label.JumpFlag = 0;

    //--------------------地址范围检查--------------------
    //1. 读写数据量是否为零或组号有没有超出范围
    if(Total == 0) return 0x02;

    if((Group > FUNGROUP_ENDINDEX) &&(Group < AUXFUNGROUP_STARTINDEX)) return 0x02;
    else if(Group > AUXFUNGROUP_ENDINDEX) return 0x02;

    //2. 末尾地址有没有超出本组功能码的范围
    if((Offset + Total) > FunCode_PanelDispLen[Group])   return 0x02;
   
    //H0241厂家密码认证检查
    if(FunCodeUnion.code.OEM_OEMPass != OEMPASSWORD)
    { 
        if((Group == 1) && ((Offset + Total) > H01_PANELDISPLEN_OEM)) return 0x02;

		if((Group == 0x0D)&&(Offset == 3))return 0x02;//by huangxin201711_19 0D03需要厂家密码

        //H0240电机参数可见认证检查
        if( (FunCodeUnion.code.MT_EnVisable <= 10000) &&
            (Group == 0) && ((Offset + Total) > H00_PANELDISPLEN_OEM))return 0x02;
    }

    //如果H0000!=65535或14000,00组除了H0000外都不可以改写
    if( (Group == 0) && (FunCodeUnion.code.MT_MotorModel != 65535) &&
        (14 != (FunCodeUnion.code.MT_MotorModel/1000)) &&
        (OperationMode != READSERIES) && 
        ((Offset + Total) > H00_PANELDISPLEN_OEM) )
    {
        return 0x02;
    }

    //--------------------数据类型判断--------------------
    if(Total > 1)
    {
        //1. 首个地址有没有指向32位功能码的高十六位
        IndexTemp = GetGroupCodeDftIndex(Group ,Offset);
        if(FunCodeDeft[IndexTemp].Attrib.bit.DataIndex == ATTRIB_HIGH_WORD)  return 0x04;

        //2. 末尾地址有没有指向32位功能码的低十六位       
        IndexTemp = IndexTemp + Total - 1;
        if((FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD)&&
           (FunCodeDeft[IndexTemp].Attrib.bit.DataIndex == ATTRIB_LOW_WORD))  return 0x04;
    }

    switch(OperationMode)
    {
        //================================================== 读数据 ==================================================
        case READSERIES:

            //复位功能码地址、读写数据地址、功能码属性序号
            pFunCodeAddr = (Uint16 *)FunCode_GroupStartAddr[Group] + Offset;
            pWrRdAddr = pAddr;
            IndexTemp = GetGroupCodeDftIndex(Group ,Offset);

            //读一个数据
            if(Total == 1) 
            {
                //32位功能码不允许单独读高16位或低16位
                if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD) return 0x04;
                 
                if( (FunCodeDeft[IndexTemp].Attrib.bit.Writable == 3) ||
                    (FunCodeDeft[IndexTemp].Attrib.bit.CommReadEn == ATTRIB_COMM_READ_DISABVLE) )
                {
                    *pWrRdAddr = 0;
                }
                else
                {
                    *pWrRdAddr = *pFunCodeAddr;
                }
                return 0;
            }

            //读多个数据
            for(CycCnt=0;CycCnt<Total;)
            {
                //读32位数据
                if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD)
                {
                    if(FunCodeDeft[IndexTemp].Attrib.bit.CommReadEn == ATTRIB_COMM_READ_DISABVLE)
                    {
                        *pWrRdAddr = 0;
                        *(pWrRdAddr + 1) = 0;
                    }
                    else if((FunCodeUnion.code.CM_Modbus32BitsSeq == 0) && (CommType == COMM_TYPE_MODBUS))
                    {
                        //高十六位在前,低十六位在后
                        *pWrRdAddr = *(pFunCodeAddr + 1);
                        *(pWrRdAddr + 1) = *pFunCodeAddr;
                        //while(1);
                    }
                    else if((FunCodeUnion.code.CM_Modbus32BitsSeq == 1) && (CommType == COMM_TYPE_MODBUS))
                    {
                        //高十六位在后,低十六位在前
                        *pWrRdAddr = *pFunCodeAddr ;
                        *(pWrRdAddr + 1) = *(pFunCodeAddr + 1);
                    }
                    else if(CommType == COMM_TYPE_CANLINK)
                    {
                        //高十六位在后,低十六位在前
                        *pWrRdAddr = *pFunCodeAddr ;
                        *(pWrRdAddr + 1) = *(pFunCodeAddr + 1);
                    }
                    else if(CommType == COMM_TYPE_CANOPEN)
                    {
                        //高十六位在后,低十六位在前
                        *pWrRdAddr = *pFunCodeAddr ;
                        *(pWrRdAddr + 1) = *(pFunCodeAddr + 1);
                    }
                    pWrRdAddr += 2;
                    pFunCodeAddr += 2;
                    CycCnt += 2;
                    IndexTemp += 2;
                }
                //读16位数据
                else
                {
                    if( (FunCodeDeft[IndexTemp].Attrib.bit.Writable == 3) ||
                        (FunCodeDeft[IndexTemp].Attrib.bit.CommReadEn == ATTRIB_COMM_READ_DISABVLE) )
                    {
                        *pWrRdAddr = 0;
                    }
                    else
                    {
                        *pWrRdAddr = *pFunCodeAddr;
                    }

                    pWrRdAddr ++;
                    pFunCodeAddr++;
                    CycCnt++;
                    IndexTemp++;
                }
            }
            return 0;

        //================================================== 写一串数据 ==================================================
        case WRITESERIES:
            //功能码复位期间不允许写功能码
            if((STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 1)||(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode == 1)||
                (STR_FUNC_Gvar.ManageFunCodeOutput.ResetOD == 1)) return 0x02;

            //写数据量是否为1
            if(Total == 1) return 0x02;

            //复位读写数据地址、功能码属性序号
            pWrRdAddr = pAddr;
            IndexTemp = GetGroupCodeDftIndex(Group ,Offset);
            
            for(CycCnt=0;CycCnt<Total;)
            {    
                //----------------------------超出限值检查   32位功能码----------------------------
                if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD)
                {
                    if(FunCodeUnion.code.CM_Modbus32BitsSeq == 0)
                    {
                        //高十六位在前,低十六位在后
                        FunCodeTemp = A_SHIFT16_PLUS_B(*pWrRdAddr,*(pWrRdAddr+1));
                    }
                    else
                    {
                        //高十六位在后,低十六位在前
                        FunCodeTemp = A_SHIFT16_PLUS_B(*(pWrRdAddr+1),*pWrRdAddr);
                    }

                    LmtCheck = LimitCheck_TwoWords(IndexTemp, FunCodeTemp);
                    if(LmtCheck != 0) return 0x03;

                    //写入数据地址指针/序号/循环计数器加2,进入到下一次判断
                    pWrRdAddr += 2;
                    IndexTemp += 2;
                    CycCnt += 2;
                } 
                //----------------------------超出限值检查   16位功能码----------------------------
                else
                {
                    //调用内联比较函数
                    LmtCheck = LimitCheck_0neWord(IndexTemp,*pWrRdAddr);
                    if(LmtCheck != 0) return 0x03;

                    //写入数据地址指针/序号/循环计数器加1,进入到下一次判断
                    pWrRdAddr ++;
                    IndexTemp ++;
                    CycCnt ++;
                }
            }

            //----------停机设定参数/保留参数/显示参数检查----------
            IndexTemp = GetGroupCodeDftIndex(Group ,Offset);
            for(CycCnt=0;CycCnt<Total;CycCnt++)
            {    
                if((FunCodeDeft[IndexTemp].Attrib.bit.Writable == 1) && 
                   (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))  return 0x04;

                if(FunCodeDeft[IndexTemp].Attrib.bit.Writable > 1) return 0x04; 
                IndexTemp++;
            }

            //----------H0241设定检查----------
            if((Group == 0x02) && (Offset >= 41) && ((Offset + Total -1) <= 41))
            {
                //复位写入数据地址指针
                pWrRdAddr = pAddr;
                pWrRdAddr += 41- Offset;
                if(*pWrRdAddr != OEMPASSWORD) return 0x03;
            }

            //----------H0240设定检查----------
            if((Group == 0x02) && (Offset >= 40) && ((Offset + Total -1) <= 40))
            {
                //复位写入数据地址指针
                pWrRdAddr = pAddr;
                pWrRdAddr += 40- Offset;
                if(*pWrRdAddr < 10000) return 0x03;
            }

            //--------------------校验成功开始写数据--------------------
            //复位功能码地址、读写数据地址、功能码属性序号
            pFunCodeAddr = (Uint16 *)FunCode_GroupStartAddr[Group] + Offset;
            pWrRdAddr = pAddr;
            IndexTemp = GetGroupCodeDftIndex(Group ,Offset);
            
            if(Total>2)
            {
                for(CycCnt=0;CycCnt<Total;)
                {
                    //---------------------------- 写32位功能码 ----------------------------
                    if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD)
                    {
                        if(FunCodeUnion.code.CM_Modbus32BitsSeq == 0)
                        {
                            //如果更改下次上电生效属性功能码，报941标志位置1
                            if( (PostEr941_Flag == 0)&&(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) && 
                                ((*pFunCodeAddr != *(pWrRdAddr + 1)) || (*(pFunCodeAddr + 1) != *pWrRdAddr)) )
                            {
                                PostEr941_Flag = 1;
                            }
    
                            //高十六位在前,低十六位在后
                            *pFunCodeAddr = *(pWrRdAddr + 1);
                            *(pFunCodeAddr + 1) = *pWrRdAddr;
                        }
                        else
                        {
                            //如果更改下次上电生效属性功能码，报941标志位置1
                            if( (PostEr941_Flag == 0)&&(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) && 
                                ((*pFunCodeAddr != *pWrRdAddr) || (*(pFunCodeAddr + 1) != *(pWrRdAddr + 1))) )
                            {
                                PostEr941_Flag = 1;
                            }
    
                            //高十六位在后,低十六位在前
                            *pFunCodeAddr = *pWrRdAddr;
                            *(pFunCodeAddr + 1) = *(pWrRdAddr + 1);
                        }
                        pWrRdAddr += 2;
                        pFunCodeAddr += 2;
                        CycCnt += 2;
                        IndexTemp += 2;
                    }
                    //---------------------------- 写16位功能码 ----------------------------
                    else
                    {
                        //如果更改下次上电生效属性功能码，报941标志位置1
                        if( (PostEr941_Flag == 0)&&(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) && 
                            (*pFunCodeAddr != *pWrRdAddr))
                        {
                            PostEr941_Flag = 1;
                        }
                        *pFunCodeAddr = *pWrRdAddr;
                        pWrRdAddr ++;
                        pFunCodeAddr++;
                        CycCnt++;
                        IndexTemp++;
                    }
                }
            }
            else  //写两个功能码时,需要判断功能码有没有更改,没有更改返回0
            {
                //---------------------------- 写32位功能码 ----------------------------
                if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD)
                {
                    if(FunCodeUnion.code.CM_Modbus32BitsSeq == 0)
                    {
                        if((*pFunCodeAddr == *(pWrRdAddr + 1)) && ( *(pFunCodeAddr + 1) == *pWrRdAddr)) return 0;

                        //如果更改下次上电生效属性功能码，报941标志位置1
                        if(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) PostEr941_Flag = 1;

                        //高十六位在前,低十六位在后
                        *pFunCodeAddr = *(pWrRdAddr + 1);
                        *(pFunCodeAddr + 1) = *pWrRdAddr;
                    }
                    else
                    {
                        if((*pFunCodeAddr == *pWrRdAddr) && ( *(pFunCodeAddr + 1) == *(pWrRdAddr + 1) )) return 0;

                        //如果更改下次上电生效属性功能码，报941标志位置1
                        if(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) PostEr941_Flag = 1;

                        //高十六位在后,低十六位在前
                        *pFunCodeAddr = *pWrRdAddr;
                        *(pFunCodeAddr + 1) = *(pWrRdAddr + 1);
                    }
                }
                //---------------------------- 写16位功能码 ----------------------------
                else
                {
                    if((*pFunCodeAddr == *pWrRdAddr) && ( *(pFunCodeAddr + 1) == *(pWrRdAddr + 1) )) return 0;

                    if(*pFunCodeAddr != *pWrRdAddr)
                    {
                        *pFunCodeAddr = *pWrRdAddr;
                        if(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) PostEr941_Flag = 1;
                    }

                    pWrRdAddr ++;
                    pFunCodeAddr++;
                    IndexTemp++;

                    if(*pFunCodeAddr != *pWrRdAddr)
                    {
                        *pFunCodeAddr = *pWrRdAddr;
                        if(FunCodeDeft[IndexTemp].Attrib.bit.Active == 1) PostEr941_Flag = 1;
                    }
                }
            }

            //如果更改H03 H04 H17组参数 STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate置1
            if((Group == 0x03) || (Group == 0x04) || (Group == 0x17))
            {
                STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate = 1;
            }

            //-------------------- Eeprom存储数据 --------------------
            if(((FunCodeUnion.code.CM_WriteEepromEnable == 1)||(FunCodeUnion.code.CM_WriteEepromEnable == 3)) 
				&&(Group_Input <= FUNGROUP_ENDINDEX) &&(Group != 0x0D) && (Group != 0x0B))
            {
                IndexTemp = GetGroupCodeIndex(Group ,Offset);
                SaveToEepromSeri(IndexTemp,(IndexTemp + Total - 1));

                // 再次上电生效的警告
                if(PostEr941_Flag == 1)
                {
                    PostErrMsg(PCHGDWARN);
                }
            }
            return 0;
        //================================================== 写一个数据 ==================================================
        case WRITEONE:
            //功能码复位期间不允许写功能码
            if((STR_FUNC_Gvar.ManageFunCodeOutput.ResetFunCode == 1)||(STR_FUNC_Gvar.ManageFunCodeOutput.CANResetFunCode == 1)||
                (STR_FUNC_Gvar.ManageFunCodeOutput.ResetOD == 1)) return 0x02;

            //写数据量是否大于1
            if(Total > 1) return 0x02;

            //复位功能码地址、读写数据地址、功能码属性序号
            pFunCodeAddr = (Uint16 *)FunCode_GroupStartAddr[Group] + Offset;
            pWrRdAddr = pAddr;
            IndexTemp = GetGroupCodeDftIndex(Group ,Offset);

            //----------32位功能码检查----------
            if(FunCodeDeft[IndexTemp].Attrib.bit.DataBits == ATTRIB_TWO_WORD) return 0x04;

            //----------超出限值检查   16位功能码----------
            //调用内联比较函数
            LmtCheck = LimitCheck_0neWord(IndexTemp,*pWrRdAddr);
            if(LmtCheck != 0) return 0x03; 

            //----------保留参数和显示参数检查----------
            if(FunCodeDeft[IndexTemp].Attrib.bit.Writable > 1) return 0x04;
            
            if((FunCodeUnion.code.AT_LowOscMod == 1) && (Group == 0x09)      //H0904置1时，H0925和H0926只做显示
                && (Offset == 38 || (Offset == 39)))
            {
                return 0x04;
            } 

            //----------停机设定参数检查----------
            if((FunCodeDeft[IndexTemp].Attrib.bit.Writable == 1) &&
               (STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus == RUN))
               return 0x04;

            //----------H0241设定检查----------
            if((Group == 0x02) && (Offset == 41) && (*pWrRdAddr != OEMPASSWORD)) return 0x03;

            //----------H0240设定检查----------
            if((Group == 0x02) && (Offset == 40) && (*pWrRdAddr < 10000)) return 0x03;

            if(*pFunCodeAddr != *pWrRdAddr)     //如果写入数据与原来数据相等,退出
            {
                *pFunCodeAddr = *pWrRdAddr;

                //如果更改H03 H04 H17组参数 STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate置1
                if((Group == 0x03) || (Group == 0x04) || (Group == 0x17))
                {
                    STR_FUNC_Gvar.ManageFunCodeOutput.AiAoDiDoUpdate = 1;
                }

                //-------------------- Eeprom存储数据 --------------------
                if((Group == 0x0C) && (Offset == 13))
                {
                    SaveToEepromOne(GetGroupCodeIndex(Group ,Offset));               
                }
                else if(((FunCodeUnion.code.CM_WriteEepromEnable == 1)||(FunCodeUnion.code.CM_WriteEepromEnable == 3)) 
					&&(Group_Input <= FUNGROUP_ENDINDEX) &&
                   (FunCodeDeft[IndexTemp].Attrib.bit.CommSaveEn == ATTRIB_COMM_SAVE_EEPROM) &&
                   (Group != 0x0D) && (Group != 0x0B))
                {
                    SaveToEepromOne(GetGroupCodeIndex(Group ,Offset));
                    // 再次上电生效的警告
                    if(FunCodeDeft[GetGroupCodeDftIndex(Group ,Offset)].Attrib.bit.Active == ATTRIB_NEXT_ACT)
                    {
                        PostErrMsg(PCHGDWARN);
                    }
                }
            }
            return 0;

        default:
            break;
    }
    return 0x10;
}


/*******************************************************************************
  函数名: Uint16 COMMWrRdFuncode(const Uint8 Offset, const Uint16 Total, Uint16 *pAddr,
             const Uint8 OperationMode,Uint8 CommType);
  输入:   无 
  输出:   无 
  子函数: 无
    描述：
    1.
    2.
********************************************************************************/
/* OperationMode:
 * NULL          0
 * READSERIES    1
 * WRITESERIES   2
*/
/*
 返回故障标志
 * 0x00  操作成功        Sucess
 * 0x10  操作模式错误    OperationModeErr

 * 0x01  命令字错误码
 * 0x02  地址不存在，超出地址范围
*/

Uint16 COMMWrRdEleLabel(const Uint8 Offset, const Uint16 Total, Uint16 *pAddr,const Uint8 OperationMode)
{
    Uint16 *pWrRdAddr = 0;          //写入数据地址或读数据来源地址
    Uint16  CycCnt = 0;             //for循环计数器
    Uint16  Temp = 0;

    //1. 读写数据量是否为零
    if(Total == 0) return 0x02;

    //2. 末尾地址有没有超出范围
    if((Offset + Total) > 0x30)   return 0x02;

    pWrRdAddr = pAddr;

    switch(OperationMode)
    {
        //================================================== 读数据 ==================================================
        case READSERIES:
            if((Offset == 4) && (Total == 1)) //跳转到Boot区
            {
                if(0x0300 == UNI_EleLabel.Label.ServoStatus)
                {
                    UNI_EleLabel.Label.JumpFlag = 1;
                    UNI_EleLabel.Label.DelayCnt = 0;
                    *pWrRdAddr = 0x0300;
                    return 0;
                }
                else
                {
                    UNI_EleLabel.Label.JumpFlag = 0;
                    
                    if(0 <= STR_FUNC_Gvar.OscTarget.SpdFdbFlt) *pWrRdAddr = 0x0100;
                    else *pWrRdAddr = 0x0200;
                    return 0;                
                }                          
            }
            if((Offset == 5) && (Total == 1)) //跳转到Boot区
            {
                if( (0x0300 == UNI_EleLabel.Label.ServoStatus) && (1 == UNI_EleLabel.Label.JumpFlag) )
                {
                    *pWrRdAddr = 0x0001;
                    UNI_EleLabel.Label.JumpFlag = 4;
                    UNI_EleLabel.Label.DelayCnt = 0;
                    return 0;
                }
                else
                {
                    *pWrRdAddr = 0;
                    UNI_EleLabel.Label.JumpFlag = 0;
                    return 1;                
                }                         
            }
            else
            {
                for(CycCnt=0;CycCnt<Total;CycCnt++)
                {
                    *pWrRdAddr = (Uint16) * EleLabelAddr[Offset + CycCnt];
                    pWrRdAddr ++; 
                }
                UNI_EleLabel.Label.JumpFlag = 0;
            }   

            return 0;

        //================================================== 写一串数据 ==================================================
        case WRITESERIES:
            if(1 == UNI_EleLabel.Label.JumpFlag) UNI_EleLabel.Label.JumpFlag = 0;
            
            //地址判断
            if(Offset < 0x20) return 0x02;
            
            for(CycCnt=0;CycCnt<Total;CycCnt++)
            {
                FunCodeUnion.code.EL_SN[Offset + CycCnt - 0x20] = (*pWrRdAddr);
                pWrRdAddr ++; 
            }
            
            //存储Eeprom中            
            Temp = GetCodeIndex(FunCodeUnion.code.EL_SN[Offset - 0x20]); 
            SaveToEepromSeri(Temp, (Temp + Total - 1));
            return 0;
            
        default:
            break;
    }

    return 0x10;
}

/********************************* END OF FILE *********************************/
