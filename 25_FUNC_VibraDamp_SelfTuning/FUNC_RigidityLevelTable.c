/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.            
 文件名:    FUNC_RigidityLevelTable.c
 创建人:    朱祥华               创建日期：2012.07.24
 修改：     姚虹      			 修改日期：2014.09.10
 描述:
    1. 
    2. 

 修改记录：    
    1. 将位置增益，积分等参数通过查表直接打到，而不再使用计算的方式     
    2. 加入增益自调整过程程序
********************************************************************************/ 

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "PUB_Main.h"
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ManageFunCode.h"
#include "FUNC_RigidityLevelTable.h"

/* Private_Constants ---------------------------------------------------------*/
/* 宏定义 常数类*/


//计算位置环增益梯度
#define   POS_KP_GRAD       40
/* Private_Macros ------------------------------------------------------------*/
/* 宏定义 函数类 */

/* Private_TypesDefinitions --------------------------------------------------*/ 
/* 结构体变量定义 枚举变量定义 */




//我司原来计算出来的刚性表
const STR_RGDTYTABLE  GainTable[] = 
{//H0802       H0800        H0801        H0705    H0900刚性等级
    24,         15,         50000,        1326,   // 刚性0
    32,         20,         39789,         995,   // 刚性1
    40,         25,         31831,         796,   // 刚性2
    48,         30,         26526,         663,   // 刚性3
    56,         35,         22736,         568,   // 刚性4
    72,         45,         17684,         442,   // 刚性5
    96,         60,         13263,         332,   // 刚性6
   120,         75,         10610,         265,   // 刚性7
   144,         90,          8842,         221,   // 刚性8
   176,        110,          7234,         181,   // 刚性9
   224,        140,          5684,         142,   // 刚性10
   288,        180,          4421,         111,   // 刚性11
   400,        250,          3183,          80,   // 刚性12
   480,        300,          2653,          66,   // 刚性13
   560,        350,          2274,          57,   // 刚性14
   640,        400,          1989,          50,   // 刚性15
   800,        500,          1592,          40,   // 刚性16
   960,        600,          1326,          33,   // 刚性17
  1200,        750,          1061,          27,   // 刚性18
  1440,        900,           884,          22,   // 刚性19
  1840,       1150,           692,          17,   // 刚性20
  2240,       1400,           568,          14,   // 刚性21
  2720,       1700,           468,          12,   // 刚性22
  3360,       2100,           379,           9,   // 刚性23
  4000,       2500,           318,           8,   // 刚性24
  4480,       2800,           284,           7,   // 刚性25
  4960,       3100,           257,           6,   // 刚性26
  5440,       3400,           234,           6,   // 刚性27
  5920,       3700,           215,           5,   // 刚性28
  6400,       4000,           199,           5,   // 刚性29
  7200,       4500,           177,           5,   // 刚性30
  8000,       5000,           159,           5,   // 刚性31
  8000,       5000,           159,           5,   // 多加一组防止溢出    
};

/* Private_Variables ---------------------------------------------------------*/
/* 变量定义 */  

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */
void Rigidity_LevelProcess2(void);


/* Private_Functions ---------------------------------------------------------*/
/* 该文件内部调用的函数的声明 */ 




//根据自动增益调整的模式选择来调整相应的增益参数
void Rigidity_LevelProcess(void)
{
    static Uint16 RomUpdtFlag = 0;        //增益变化标志位，1~启动增益调整
	static Uint16 OldModeSel  = 0;
	static Uint16 InitDone = 0; 
    Uint16 level;

	if(InitDone == 0)     //初次上电给模式赋值，避免反复写ROM
	{
	    OldModeSel = FunCodeUnion.code.AutoTuningModeSel;
		InitDone   = 1;
	}
		
	//判断是否更新Eeprom，从模式0切换到其它模式或者非零模式下增益与刚性表中不一致时
    if(FunCodeUnion.code.AutoTuningModeSel != 0)
    {
	    if((FunCodeUnion.code.GN_Spd_Kp != GainTable[FunCodeUnion.code.Rigidity_Level1st].SpdKp) 
	       || (OldModeSel == 0))   
    	{
	        RomUpdtFlag = 1;		    //需要更新Eeprom中功能码
	    }
    }

    switch(FunCodeUnion.code.AutoTuningModeSel)
	{
	    case 0:				   //为零时保持参数不变，只考虑
			//只有在手动增益自调整切换到0时，才保存一遍功能码
			if(RomUpdtFlag == 1)
			{			
				if(STR_FUNC_Gvar.RigidityLvlTab.AutoTuneFlag == 1)     //只在手动自调整时更新功能码
                {
                    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Gn2ndSetup));
				    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_GnSwModeSel));
				    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_SpdFb_Kp));				
				    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime));
				    SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime2));

				    SaveToEepromSeri(GetCodeIndex(FunCodeUnion.code.GN_Spd_Kp),GetCodeIndex(FunCodeUnion.code.GN_Pos_Kp2));

                    //根据之前的模式决定是否保存补偿值
                    if((OldModeSel == 3) || (OldModeSel == 4))
                    {
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_ConstToqComp));
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_ToqPlusComp));
                        SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.AT_ToqMinusComp));                    
                    }

                    STR_FUNC_Gvar.RigidityLvlTab.AutoTuneFlag = 0;     //手动自调整结束
                }
				
				RomUpdtFlag = 0;
			}        
        	    
		break;

		case 1:	  //标准刚性表模式
		case 3:	  //刚性表模式带摩擦补偿
	        if(AuxFunCodeUnion.code.H2F_Rsvd33 == 1)
            {
		        FunCodeUnion.code.GN_Gn2ndSetup  = 1;   
		        FunCodeUnion.code.GN_GnSwModeSel = 0;	//无增益切换，H0810                
                FunCodeUnion.code.GN_SpdFb_Kp = 0;	//前馈值
            }

			FunCodeUnion.code.TL_ToqFiltTime = GainTable[FunCodeUnion.code.Rigidity_Level1st].ToqFiltTime; 
			FunCodeUnion.code.GN_Spd_Kp	     = GainTable[FunCodeUnion.code.Rigidity_Level1st].SpdKp;
			FunCodeUnion.code.GN_Spd_Ti      = GainTable[FunCodeUnion.code.Rigidity_Level1st].SpdTi;
			FunCodeUnion.code.GN_Pos_Kp		 = GainTable[FunCodeUnion.code.Rigidity_Level1st].PosKp;
			//第二增益设置为默认值
			//FunCodeUnion.code.TL_ToqFiltTime2 =	GainTable[12].ToqFiltTime;
			//FunCodeUnion.code.GN_Spd_Kp2	  = GainTable[12].SpdKp;
		    //FunCodeUnion.code.GN_Pos_Kp2 	  = GainTable[12].PosKp; //位置增益取高一等级的值		    			
			//FunCodeUnion.code.GN_Spd_Ti2      = GainTable[12].SpdTi;

			if((RomUpdtFlag == 1) && (AuxFunCodeUnion.code.H2F_Rsvd33 == 0))    //刚性等级发生变化且不进行自调整  
			{			    			 
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Gn2ndSetup));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_GnSwModeSel));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_SpdFb_Kp));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Spd_Kp));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Spd_Ti));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Pos_Kp));

				RomUpdtFlag = 0;
			}					
		break;
		
		case 2:	  //定位模式		
		case 4:	  //定位模式带摩擦补偿			
			if(OldModeSel != FunCodeUnion.code.AutoTuningModeSel) 
            {
                FunCodeUnion.code.GN_Gn2ndSetup  = 1;
		        FunCodeUnion.code.GN_GnSwModeSel = 10;
                FunCodeUnion.code.GN_SpdFb_Kp = 300;     //带30%的前馈
            }

			FunCodeUnion.code.TL_ToqFiltTime = GainTable[FunCodeUnion.code.Rigidity_Level1st].ToqFiltTime; 
			FunCodeUnion.code.GN_Spd_Kp	     = GainTable[FunCodeUnion.code.Rigidity_Level1st].SpdKp;
			FunCodeUnion.code.GN_Spd_Ti      = GainTable[FunCodeUnion.code.Rigidity_Level1st].SpdTi;
			FunCodeUnion.code.GN_Pos_Kp		 = GainTable[FunCodeUnion.code.Rigidity_Level1st].PosKp;
			FunCodeUnion.code.TL_ToqFiltTime2 =	FunCodeUnion.code.TL_ToqFiltTime;
			FunCodeUnion.code.GN_Spd_Kp2	  = FunCodeUnion.code.GN_Spd_Kp;

            level = FunCodeUnion.code.Rigidity_Level1st+2;
            if(level > 31)
            {
                level = 31;
            }
			FunCodeUnion.code.GN_Pos_Kp2 	  = GainTable[level].PosKp; //位置增益取高一等级的值		    
			
			FunCodeUnion.code.GN_Spd_Ti2 = 51200;


			//刚性等级发生变化且不处于自调整时
			if((RomUpdtFlag == 1) && (AuxFunCodeUnion.code.H2F_Rsvd33 == 0))
			{			
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_Gn2ndSetup));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_GnSwModeSel));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.GN_SpdFb_Kp));				
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime));
				SaveToEepromOne(GetCodeIndex(FunCodeUnion.code.TL_ToqFiltTime2));

				SaveToEepromSeri(GetCodeIndex(FunCodeUnion.code.GN_Spd_Kp),GetCodeIndex(FunCodeUnion.code.GN_Pos_Kp2));
				
				RomUpdtFlag = 0;
			}
		break; 
		
		default:
		break;   
	}
	
	STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc1st = (Uint32)FunCodeUnion.code.TL_ToqFiltTime * 10;
	STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st       = FunCodeUnion.code.GN_Spd_Kp;

    if (FunCodeUnion.code.GN_Spd_Ti >= 51200) //积分时间大于等于512ms时无积分作用
    {
        STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10 = 0;
    }
    else //积分时间小于512ms时才有积分作用
    {
        STR_FUNC_Gvar.RigidityLvlTab.SpdKi1st_Q10 
        = ((Uint64)STR_FUNC_Gvar.RigidityLvlTab.SpdKp1st * STR_FUNC_Gvar.System.SpdPrd_Q10) 
		   / ((Uint32)FunCodeUnion.code.GN_Spd_Ti * 10);                   
    }

    STR_FUNC_Gvar.RigidityLvlTab.ToqRefFltTc2nd = (Uint32)FunCodeUnion.code.TL_ToqFiltTime2 * 10;;  //单位us
    STR_FUNC_Gvar.RigidityLvlTab.SpdKp2nd       = FunCodeUnion.code.GN_Spd_Kp2;

    if (FunCodeUnion.code.GN_Spd_Ti2 >= 51200) //积分时间大于等于512ms时无积分作用
    {
        STR_FUNC_Gvar.RigidityLvlTab.SpdKi2nd_Q10 = 0;
    }
    else //积分时间小于512ms时才有积分作用
    {
        STR_FUNC_Gvar.RigidityLvlTab.SpdKi2nd_Q10 
         =  ((int64)STR_FUNC_Gvar.RigidityLvlTab.SpdKp2nd * STR_FUNC_Gvar.System.SpdPrd_Q10)
		      / ((Uint32)FunCodeUnion.code.GN_Spd_Ti2 * 10);
    }

    //位置环增益梯度处理
    if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN) || 
       ((STR_FUNC_Gvar.RigidityLvlTab.PosKp1st + POS_KP_GRAD) > FunCodeUnion.code.GN_Pos_Kp))
    {
        STR_FUNC_Gvar.RigidityLvlTab.PosKp1st = FunCodeUnion.code.GN_Pos_Kp;
    }
    else
    {
        STR_FUNC_Gvar.RigidityLvlTab.PosKp1st += POS_KP_GRAD;
    }

    if((STR_FUNC_Gvar.MonitorFlag.bit.ServoRunStatus != RUN) || 
       ((STR_FUNC_Gvar.RigidityLvlTab.PosKp2nd + POS_KP_GRAD) > FunCodeUnion.code.GN_Pos_Kp2))
    {
        STR_FUNC_Gvar.RigidityLvlTab.PosKp2nd = FunCodeUnion.code.GN_Pos_Kp2;
    }
    else
    {
        STR_FUNC_Gvar.RigidityLvlTab.PosKp2nd += POS_KP_GRAD;
    }
	
	OldModeSel = FunCodeUnion.code.AutoTuningModeSel;	        
}







/********************************* END OF FILE *********************************/
