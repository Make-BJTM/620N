/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:	XX.h                                                           
 创建人：XX                创建日期：XX.XX.XX 
 修改人：XX                修改日期：XX.XX.XX 
 修改人：XX                修改日期：XX.XX.XX 
 描述： 
    1.
    2.
/-------------------------------------------------------------/
1.  ETC伺服MCU软件模块：
    FPGA——FPGA配置模块
    SPDTOQ——速度转矩控制模块
    POS——位置控制模块
    TUNE——调整模块
    ECAT——EthetCAT模块
    FUNC——功能模块
    
2.  参考《汇川伺服软件编程规范V2.0》
    模块接口变量命名形式：GTYPE_MODULEEXTERN_MlpName或GTYPE_MODULEEXTERN_IntName
    模块全局变量命名形式：TYPE_MODULE_MlpName或TYPE_MODULE_IntName    
    G：       该前缀表明该变量为全局变量（大写）
    TYPE：    表明该变量对应的结构化数据类型(大写)。
              (如：STR为结构体；UNI为共用体，ARR为数组，ENU为枚举型，
              若该变量为基本数据类型则不用写）
    MODULE：  模块名简称(大写)，其前后需加下划线。(如：FUNC为功能模块)
    Name：    具有功能含义的变量名，名字较长时应考虑缩写，
              单词间没有下划线（单词的首字母大写）
              
    例如：功能模块输出的接口结构体变量命名：
    GSTR_FUNCEXTERN_INTVARIABLE        gSTR_FUNCEx_IntVar;
    GSTR_FUNCEXTERN_MAINLOOPVARIABLE   gSTR_FUNCEx_MlpVar;
    母线电压    gSTR_FUNCEx_IntVar.UdcLive

    例如：功能模块内部用的全局结构体变量命名
    STR_FUNC_INTGLOBALVARIABLE         STR_FUNC_IntGvar;
    STR_FUNC_MAINLOOPGLOBALVARIABLE    STR_FUNC_MlpGvar;
********************************************************************************/

#ifndef __FUNC_CSPCMDBUFFER_H
#define __FUNC_CSPCMDBUFFER_H

#ifdef __cplusplus
 extern "C" {
#endif	


/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义*/
//暂无

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */	
//暂无

/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 
typedef struct{   
    int32   TargetPos;              //目标位置   
    int32   SpdRef;                 //速度前馈
    int32   ToqRef;                 //转矩前馈   
}STR_CSPCMD;

typedef struct{   
    STR_CSPCMD      Cmd[8]; 
    int32           EstimatedDeltaPos;               
    int8            WrAddr;
    int8            RdAddrLatch[3];               
    int8            WrAddrLatch[3];    
    int8            BuffMod;
    int8            BuffState;    
}STR_CSPCMDBUFFERVAR;

typedef struct{
    volatile    int32              CSP_TargetPos;              //目标位置 
    volatile    int32              CSP_SpdRef;                 //速度前馈
    volatile    int32              CSP_ToqRef;                 //转矩前馈        
}STR_CMDBUFFERVARIABLE;


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */
//暂无
extern STR_CMDBUFFERVARIABLE STR_CmdBufferVar;
/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void CSPBufferVarUpdate(void);
extern void WrCSPBufferIrq(void);
extern void RdCSPBufferSync(void);


#ifdef __cplusplus
}
#endif

#endif /* __FUNC_CSPCMDBUFFER_H */	

/********************************* END OF FILE *********************************/
