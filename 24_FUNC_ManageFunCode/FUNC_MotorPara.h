/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.                    
 文件名:    FUNC_MotorPara.c  
 创建人：   XXXXXX                 创建日期：XXXX.XX.XX                     
 修改人：   XXXXXX                 修改日期：XXXX.XX.XX 
 描述： 
    1. 
    2. 
 修改记录：  
    XXXX.XX.XX  XXXXXXX
    1.      
    2. 	   
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */ 
#include "FUNC_GlobalVariable.h"

/* Private_Constants ---------------------------------------------------------*/
/* 不带参数的宏定义 */

/* Private_Macros ------------------------------------------------------------*/
/* 带参数的宏定义 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */  
typedef struct{
    Uint16 RsdMotorModel;       //H00_01 内部电机参数数组的唯一标识
    Uint16 RateVolt;            //H00_09 额定电压
    Uint16 RatePower;           //H00_10 额定功率
    Uint16 RateCurrent;         //H00_11 额定电流
    Uint16 RateToq;             //H00_12 额定转矩
    Uint16 MaxToq;         //H00_13 最大转矩
    Uint16 RateSpd;             //H00_14 额定转速
    Uint16 MaxSpd;              //H00_15 最大转速
    Uint16 Inertia;             //H00_16 转动惯量
    Uint16 PolePair;            //H00_17 永磁同步电机极对数
    Uint16 StatResist;          //H00_18 定子电阻
    Uint16 StatInductQ;         //H00_19 定子电感Lq
    Uint16 StatInductD;         //H00_20 定子电感Ld
    Uint16 RevEleCoe;           //H00_21 反电势系数
    Uint16 ToqCoe;              //H00_22 转矩系数
    Uint16 EleConst;            //H00_23 电气常数
    Uint16 MachConst;           //H00_24 机械常数
    Uint16 EncoderSel;          //H00_30 编码器选择
    Uint16 EncoderPensL;        //H00_31 编码器线数 低
    Uint16 EncoderPensH;        //H00_32 编码器线数 高
	Uint16 InitTheta;           //H00_33 初始电角度
	Uint16 UposedgeTheta;       //H00_34 Uposedge角度
    Uint16 KedGain;                 //H01_22 D轴反电势补偿系数 
	Uint16 KeqGain;                 //H01_23 Q轴反电势补偿系数
    Uint16 CurIdKpSec;              //H01_24 D轴电流环比例增益
    Uint16 CurIdKiSec;              //H01_25 D轴电流环积分补偿因子
    Uint16 CurIqKpSec;              //H01_27 Q轴电流环比例增益
    Uint16 CurIqKiSec;              //H01_28 Q轴电流环积分补偿因子    
    Uint16 CapIdKp;                 //H01_52 性能优先模式D轴比例增益
    Uint16 CapIdKi;                 //H01_53 性能优先模式D轴积分补偿因子
    Uint16 CapIqKp;                 //H01_54 性能优先模式Q轴比例增益
    Uint16 CapIqKi;                 //H01_55 性能优先模式Q轴积分补偿因子
}STR_MOTRORPARA;

//直线电机电机参数定义
typedef struct{
    Uint16 RsdMotorModel;       //H00_01 内部电机参数数组的唯一标识
    Uint16 RateVolt;            //H00_09 额定电压
    Uint16 RatePower;           //H00_10 额定功率
    Uint16 RateCurrent;         //H00_11 连续电流
    Uint16 RateForce;           //H00_12 连续推力
    Uint16 MaxCurrent;          //H00_13 最大电流
    Uint16 RateSpd;             //H00_14 额定速度
    Uint16 MaxSpd;              //H00_15 最大速度
    Uint16 MoverMass;           //H00_16 动子质量
    Uint16 PolePair;            //H00_17 永磁同步电机极对数
    Uint16 StatResist;          //H00_18 定子电阻
    Uint16 StatInductQ;         //H00_19 定子电感Lq
    Uint16 StatInductD;         //H00_20 定子电感Ld
    Uint16 RevEleCoe;           //H00_21 反电势系数
    Uint16 ToqCoe;              //H00_22 转矩系数
    Uint16 EleConst;            //H00_23 电气常数
    Uint16 MachConst;           //H00_24 电机常数
    Uint16 EncoderSel;          //H00_30 编码器选择
    Uint16 PolarLength;         //H00_31 直线电机极距
    Uint16 EncoderRslt;         //H00_32 光栅尺分辨率
	Uint16 InitTheta;           //H00_33 初始电角度
	Uint16 UposedgeTheta;       //H00_34 Uposedge角度
    Uint16 KedGain;                 //H01_22 D轴反电势补偿系数 
	Uint16 KeqGain;                 //H01_23 Q轴反电势补偿系数
    Uint16 CurIdKpSec;              //H01_24 D轴电流环比例增益
    Uint16 CurIdKiSec;              //H01_25 D轴电流环积分补偿因子
    Uint16 CurIqKpSec;              //H01_27 Q轴电流环比例增益
    Uint16 CurIqKiSec;              //H01_28 Q轴电流环积分补偿因子    
    Uint16 CapIdKp;                 //H01_52 性能优先模式D轴比例增益
    Uint16 CapIdKi;                 //H01_53 性能优先模式D轴积分补偿因子
    Uint16 CapIqKp;                 //H01_54 性能优先模式Q轴比例增益
    Uint16 CapIqKi;                 //H01_55 性能优先模式Q轴积分补偿因子
}STR_BLMOT_PARAM;

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */


/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void DealMotorPara(void);

/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 


/*******************************************************************************
  函数名: 
  输入:   无 
  输出:   无 
  子函数: 无
    1.
    2.
********************************************************************************/








/********************************* END OF FILE *********************************/
