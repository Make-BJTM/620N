/*******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_Fft.h
 创建人：马世贤                创建日期：12.09.09 
 描述：
    1.FFT相关数据结构定义
	2.FFT相关变量声明
            
 修改记录：  

*******************************************************************************/
#ifndef SERVOF_FFT_H
#define SERVOF_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

//宏定义
#define NN 1024      //运算点数
#define MM 10        //蝶形运算级数，2^M = N;

typedef struct 
{ 
    Uint16 Fs;  //采样频率
	Uint16 N;   //傅立叶变换长度
	Uint16 M;   //傅立叶变换长度对应2的M次方，也即蝶形运算级数
	Uint16 OldFreq;		  //陷波器更新前的频率
	Uint16 OldBandW;       //陷波器更新前宽度
	Uint16 OldDepth;       //陷波器更新前深度
	Uint16 CntPerSec;      //每秒内的计数
	Uint16 QuitDelaySec;   //为退出自适应滤波器的延时计数，单位s

	int16 AverageValue;	    //认为平均值是直流偏置
	int16 OldNtchUpdtFlag;  //上次更新陷波器时的标志位，1~更新C，2~更新D   
	int32 DataSum;         //16位数累加512次，用32位肯定不会溢出。
	int32 Freq;     //谐波峰值频率	
	int32 Mag[2];	 //谐波峰值幅度，保留两个以判定设置陷波器后振幅是否加大	   			     		  			  		   
}STR_FFTFUNC;

	
//复数数据结构定义
typedef struct
{
    int32 Real;
	int32 Imag;
}STR_COMPLX;




extern void FftProcess(void);
extern void FftSample(int32 TorqCmdTemp);
extern void FftInit(void);
extern void FftUpdate(void);
extern void FftStopUpdate(void);
extern Uint16 FftCal(void);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of SERVOF_FFT_H definition

//===========================================================================
// End of file.
//===========================================================================
