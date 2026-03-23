/******************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_MotorOverLoad.c
 创建人：匡两传                 创建日期：2013.9.10

 描述： 
     1.电机过载保护

 修改记录：  
     1.xx.xx.xx      XX  
        变更内容： xxxxxxxxxxx
     2.xx.xx.xx      XX
        变更内容： xxxxxxxxxxx

******************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */
#include "FUNC_GlobalVariable.h"
#include "FUNC_FunCode.h"
#include "FUNC_ErrorCode.h" 
#include "FUNC_MotorOverLoad.h"


void InitMotorOverLoadProc(void);
void MotorHeatCalc( int32 current);

static int32 HeatOld_Q20=0;       //初始温升0
static int32 Heat_Q20=0;          //实时温升
static int32 TimeCoef=40000;     //ms
static int32 coefA_Q20;
static int32 coefB_Q20;

static int32 OverLoadRate;
static int32 OverLoadWarn;

#define SAMPLEPD 1            //ms
#define INV_TWODOTSIXQ20 394   //(10/26Q10)Q20

//===========================================================

//===========================================================
void InitMotorOverLoadProc(void)
{
	int32 temp;
	
	TimeCoef = 40000;
	temp = (SAMPLEPD<<22)/TimeCoef;
	coefA_Q20 = (1<<22)- temp;
	coefB_Q20 = temp;
					   
	if(FunCodeUnion.code.ER_MotorLoadRate != 0)
	{
		OverLoadRate = ((int32)FunCodeUnion.code.ER_MotorLoadRate*FunCodeUnion.code.ER_MotorLoadRate)*105;
		OverLoadWarn = (OverLoadRate *114)>>7; //90%为警告点
	}
	else
	{
		//if( ((FunCodeUnion.code.MT_RatePower == 40) ||(FunCodeUnion.code.MT_RatePower == 20))
		if( (14 == (FunCodeUnion.code.MT_MotorModel / 1000)) 
		 && ( (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 703) 
           || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 704)
           || (FunCodeUnion.code.MT_RsdAbsRomMotorModel == 804) ) )
       	{
		    OverLoadRate = 1388625;// 115%过载电流
		    OverLoadWarn = 1050000;//100%过载警告
		}
		else if( (FunCodeUnion.code.MT_MotorModel == 703) 
              || (FunCodeUnion.code.MT_MotorModel == 704)
              || (FunCodeUnion.code.MT_MotorModel == 804) )
	    {
		    OverLoadRate = 1388625;// 115%过载电流
		    OverLoadWarn = 1050000;//100%过载警告	
	    }
		else
		{
            OverLoadRate = 1388625;//115% 1157625;// 105%过载电流
		    OverLoadWarn = 1275750;//110% 1050000;//100%过载警告	
		}
	}		           
}

void UpdataTimCoef(int32 timeCoef)
{
	int32 temp;

	if(TimeCoef == timeCoef) return;
	
	TimeCoef = timeCoef;
	temp = (SAMPLEPD<<22)/timeCoef;
	coefA_Q20 = (1<<22)- temp;
	coefB_Q20 = temp;	
}

//===============
//speed 0.01rpm
//current 0.01A
//=================
void MotorHeatCalc( int32 current)
{
	Uint64 tempCurrent; //归一值
	static int32 rsvdHeat=0;
	int64 tempHeatSum;
	int32 currentlocal ;
	currentlocal = ABS(current);

	tempCurrent = ((int64)currentlocal*currentlocal);//*10000)>>20;//20;	   //Q10

	tempHeatSum =((int64)HeatOld_Q20 * coefA_Q20 + (int64)tempCurrent * coefB_Q20);
	Heat_Q20 = (tempHeatSum + rsvdHeat)>>22;
	HeatOld_Q20 = Heat_Q20;
	rsvdHeat = rsvdHeat + tempHeatSum - (Heat_Q20<<22);

	if(FunCodeUnion.code.ER_MTOLClose != 1)
	{
		if((FunCodeUnion.code.ER_MTOLClose == 0)&& (Heat_Q20 >= OverLoadWarn))   //警告值100%有效负载
		{
			PostErrMsg(MTOLWARN);
		}
		else
		{		
			WarnAutoClr(MTOLWARN);
		}

		if(Heat_Q20 >= OverLoadRate)	//故障	
		{		   
			  PostErrMsg(MTOL2);
	    }
	}
	if(currentlocal < (1<<10)) //小于100%
	{
	
		tempCurrent = 35000;
		UpdataTimCoef(tempCurrent);
	}
	else
	{
		tempCurrent = 40000;
		UpdataTimCoef(tempCurrent);

	}
}

/*
过载时间表
转矩% 时间S
115	82.635
116	79.468
117	76.531
118	73.798
119	71.247
120	68.857
121	66.614
122	64.502
123	62.51
124	60.627
125	58.843
126	57.151
127	55.543
128	54.013
129	52.555
130	51.164
131	49.834
132	48.563
133	47.345
134	46.179
135	45.059
136	43.985
137	42.952
138	41.958
139	41.002
140	40.081
141	39.193
142	38.336
143	37.51
144	36.711
145	35.94
146	35.194
147	34.472
148	33.773
149	33.097
150	32.441
151	31.806
152	31.19
153	30.592
154	30.012
155	29.449
156	28.902
157	28.37
158	27.853
159	27.351
160	26.862
161	26.386
162	25.923
163	25.472
164	25.033
165	24.605
166	24.188
167	23.782
168	23.386
169	22.999
170	22.622
171	22.254
172	21.895
173	21.545
174	21.202
175	20.868
176	20.541
177	20.222
178	19.91
179	19.605
180	19.307
181	19.015
182	18.73
183	18.45
184	18.177
185	17.91
186	17.648
187	17.392
188	17.141
189	16.895
190	16.655
191	16.419
192	16.188
193	15.961
194	15.739
195	15.521
196	15.308
197	15.099
198	14.894
199	14.692
200	14.495
201	14.301
202	14.111
203	13.924
204	13.741
205	13.561
206	13.385
207	13.211
208	13.041
209	12.874
210	12.71
211	12.548
212	12.39
213	12.234
214	12.081
215	11.931
216	11.783
217	11.638
218	11.495
219	11.354
220	11.216
221	11.08
222	10.947
223	10.815
224	10.686
225	10.559
226	10.434
227	10.31
228	10.189
229	10.07
230	9.9527
231	9.8372
232	9.7235
233	9.6116
234	9.5014
235	9.3928
236	9.286
237	9.1807
238	9.0771
239	8.975
240	8.8744
241	8.7753
242	8.6777
243	8.5816
244	8.4868
245	8.3935
246	8.3015
247	8.2108
248	8.1214
249	8.0333
250	7.9465
251	7.8609
252	7.7765
253	7.6933
254	7.6112
255	7.5303
256	7.4506
257	7.3719
258	7.2943
259	7.2178
260	7.1423
261	7.0678
262	6.9943
263	6.9219
264	6.8504
265	6.7798
266	6.7102
267	6.6415
268	6.5737
269	6.5068
270	6.4408
271	6.3756
272	6.3113
273	6.2478
274	6.1851
275	6.1232
276	6.0622
277	6.0018
278	5.9423
279	5.8835
280	5.8254
281	5.7681
282	5.7114
283	5.6555
284	5.6003
285	5.5457
286	5.4918
287	5.4386
288	5.386
289	5.3341
290	5.2827
291	5.232
292	5.1819
293	5.1324
294	5.0835
295	5.0352
296	4.9874
297	4.9402
298	4.8936
299	4.8475
300	4.8019
301	4.7569
302	4.7124
303	4.6684
304	4.6249
305	4.5819
306	4.5394
307	4.4973
308	4.4558
309	4.4147
310	4.3741
311	4.3339
312	4.2942
313	4.2549
314	4.2161
315	4.1776
316	4.1397
317	4.1021
318	4.0649
319	4.0282
320	3.9918
321	3.9559
322	3.9203
323	3.8851
324	3.8503
325	3.8159
326	3.7818
327	3.7481
328	3.7147
329	3.6817
330	3.6491
331	3.6168
332	3.5848
333	3.5532
334	3.5219
335	3.4909
336	3.4602
337	3.4299
338	3.3999
339	3.3701
340	3.3407
341	3.3116
342	3.2828
343	3.2542
344	3.226
345	3.198
346	3.1704
347	3.143
348	3.1158
349	3.089
350	3.0624
351	3.0361
352	3.01
353	2.9842
354	2.9587
355	2.9334
356	2.9083
357	2.8835
358	2.8589
359	2.8346
360	2.8105 
*/



