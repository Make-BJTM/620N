/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:	FUNC_VirtualDiDo.h
 创建人：李浩                创建日期：11.11.22 
 描述：
     1.功能码类型的定义
	 2.ERRMSGREC类型的定义
            
 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx
*******************************************************************************/

#ifndef FUNC_VIRTUALDIDO_H
#define FUNC_VIRTUALDIDO_H

#ifdef  __cplusplus                     //C++和C语言可兼容要求
extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */

#include "PUB_GlobalPrototypes.h"


/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/




/* Exported_Macros -----------------------------------------------------------*/
/* 宏定义 函数类 */	



/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */

typedef struct{        
    Uint16  Di1:1;   
    Uint16  Di2:1;	  
    Uint16  Di3:1;	  
    Uint16  Di4:1;		
    Uint16  Di5:1;   
    Uint16  Di6:1;   
    Uint16  Di7:1;	  
    Uint16  Di8:1;	  
    Uint16  Di9:1;		
    Uint16  Di10:1;  
    Uint16  Di11:1;  
    Uint16  Di12:1;	
    Uint16  Di13:1;	
    Uint16  Di14:1;	
    Uint16  Di15:1;  
    Uint16  Di16:1;	
}STR_VIR_DI;
 
typedef union{
    volatile Uint16       all;
    volatile STR_VIR_DI   bit;
}UNI_VIR_DI_REGISTER;

typedef struct{
    volatile UNI_VIR_DI_REGISTER    VirDiStateNew;
    volatile UNI_VIR_DI_REGISTER    VirDiStateOld;

    Uint16 VirDiFuncSel[16];       //虚拟DI分配,VirDI1~16 立即有效	   
    Uint16 VirDiLogicSel[16];      //虚拟Di有效方式选择

    Uint16 VirDiPortEnState;       //在虚拟Di使能的情况下，10个Di端口功能是否使能有效的中间运算变量

    Uint16 VirDiPortChangeEn;      //虚拟Di端口变更后使能标志位
    	
}STR_VIR_DI_STATE;

typedef struct{
    Uint16 VirDoFuncSel[16];       //Do分配,Do1~16 立即有效	   
    Uint16 VirDoLogicSel[16];      //有效方式选择,立即有效Do1~16 ->[0]~[15]

    Uint16 VirDoPortEnState;       //在虚拟Do使能的情况下，8个Do端口功能是否使能有效的中间运算变量
    Uint8  BrakeVirDoEn;           //抱闸功能的虚拟Do输出端口分配

}STR_VIR_DO_STATE;


typedef struct{                                
    Uint16 Do1:1;       
    Uint16 Do2:1;       
    Uint16 Do3:1;       
    Uint16 Do4:1;       
    Uint16 Do5:1;       
    Uint16 Do6:1;       
    Uint16 Do7:1;       
    Uint16 Do8:1;       
    Uint16 Do9:1;       
    Uint16 Do10:1;      
    Uint16 Do11:1;      
    Uint16 Do12:1;      
    Uint16 Do13:1;      
    Uint16 Do14:1;      
    Uint16 Do15:1;      
    Uint16 Do16:1;      
}STR_VIR_DO;                       

typedef union  
{
    volatile Uint16        all;
	volatile STR_VIR_DO    bit;
}UNI_VIR_DO_REGISTER;




typedef struct{
    Uint16  VirDiPort1:1;    //Bit0   端口1
    Uint16  VirDiPort2:1;    //Bit1   端口2
    Uint16  VirDiPort3:1;    //Bit2   端口3
    Uint16  VirDiPort4:1;    //Bit3   端口4
    Uint16  VirDiPort5:1;    //Bit4   端口5
    Uint16  VirDiPort6:1;    //Bit5   端口6
    Uint16  VirDiPort7:1;    //Bit6   端口7
    Uint16  VirDiPort8:1;    //Bit7   端口8
    Uint16  VirDiPort9:1;    //Bit8   端口9
    Uint16  VirDiPort10:1;   //Bit9   端口10
    Uint16  VirDiPort11:1;   //Bit10  端口11
    Uint16  VirDiPort12:1;   //Bit11  端口12
    Uint16  VirDiPort13:1;   //Bit12  端口13
    Uint16  VirDiPort14:1;   //Bit13  端口14
    Uint16  VirDiPort15:1;   //Bit14  端口15
    Uint16  VirDiPort16:1;   //Bit15  端口16
}STR_VIR_DI_PORT;
typedef union{
    volatile Uint16                  all;
    volatile STR_VIR_DI_PORT         bit;
}UNI_VIR_DI_PORT;





/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

extern Uint16 * const VirtualDiPortFunSelAddList[16];
extern Uint16 * const VirtualDoPortFunSelAddList[16];

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void InitVirDiFuncAndLogic(STR_VIR_DI_STATE *p);
extern void VirDiFuncSetErr(STR_VIR_DI_STATE *p);
extern void DealVirDi(STR_VIR_DI_STATE *p);

extern void InitVirDoFuncAndLogic(STR_VIR_DO_STATE *p);
extern void VirDoFuncSetErr(STR_VIR_DO_STATE *p);
extern void VirDoProcess(STR_VIR_DO_STATE *p);



#ifdef __cplusplus
}
#endif /* extern "C" */ 

#endif /* end of FUNC_VirtualDiDo.h */


/********************************* END OF FILE *********************************/
