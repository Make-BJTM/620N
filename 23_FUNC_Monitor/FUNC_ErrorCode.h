/********************************************************************************
 深圳市汇川技术有限公司 版权所有（C）All rights reserved.
 文件名:    FUNC_ErrorCode.h
 创建人: 王军干
 修改人：李浩                修改日期：12.01.10
 描述：
            
 修改记录：  
    1 2012.10.11      王治国  
       变更内容： 将故障码宏定义改为32位,高16位为内部故障码,
                  低16位为正常显示、存储故障码
                  发生故障时, 将内部故障码赋给H0B45
    2 xx.xx.xx      XX
       变更内容： xxxxxxxxxxx

********************************************************************************/
#ifndef FUNC_ERRORCODE_H
#define FUNC_ERRORCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* 引用头文件 */  

/* Exported_Constants --------------------------------------------------------*/
/* 不带参数的宏定义 */

//--bit15~14: 0--NO.1 ; 1--NO.2 ; 3--警告
//--bit13:    0--不可复位；1--可复位
#define  ERRMSKBIT            13
#define  ERRCODEMSK           0xE000

//----------NO.1 不可复位故障码----------//
#define  INITSYSTMPERR        0x01010101     //系统参数异常
#define  FPGAINITERR          0x01020102     //逻辑配置故障 
#define  VERSIONUNMATCH       0x01030103     //FPGA和MCU产品型号不匹配
#define  FPGAINTERR           0x01040104     //FPGA中断发送故障 运行超时
#define  MCULOST              0x01000100     //MCU访问FPGA间隔超时
#define  EXECTMFLT0           0x12080208     //MCU未及时更新转矩指令引起的超时错误
#define  EXECTMFLT1           0x22080208     //编码器通讯超时引起的错误
#define  EXECTMFLT2           0x32080208     //电流采样（7860）超时引起的错误
#define  EXECTMFLT3           0x42080208     //A/D转换过长引起的错误
#define  EXECTMFLT4           0x02080208     //FPGA系统采样运算超时
#define  INTRUNOVERTIME       0xE9400104     //电流环中断或软中断运行时间超时
#define  DEFUALTERR           0x01050105     //系统参数异常 不可复位,需恢复出厂参数 
#define  PRAMSVERR_WR         0x01080108     //参数存储故障
#define  PRAMSVERR_RD         0x11080108     //参数存储故障
#define  INITOEMPARAERR       0x01110111     //厂家参数异常

#define  ENCMATCHERR          0x01200120     //产品匹配故障,无对应的编码器
#define  NULL_MOTOR           0x11200120     //产品匹配故障,无对应的电机
#define  NULLABSMOTORNUM      0x11200120     //总线电机编码错误 
#define  NULL_DRIVER          0x21200120     //产品匹配故障,无对应的驱动器
#define  OVDRVRATECUR         0x31200120     //电机额定电流大于驱动器额定电流

#define  ABSPOSMATCHERR       0x01220122     //设置绝对位置功能时电机参数不匹配
#define  HC2NDENCMATCHERR     0x11220122     //设定为2代编码器电机型号时参数不匹配  

#define  CHABSROMERR          0x01360136     //电机ROM中数据校验错误或未存入参数
#define  CHABSROMERR1         0x11360136     //电机ROM中数据校验错误或未存入参数
#define  SOVERCURRENT         0x02000200     //软件过流故障
#define  S1OVERCURRENT        0x12000200     //软件过流故障
#define  S2OVERCURRENT        0x22000200     //软件过流故障
#define  S3OVERCURRENT        0x32000200     //软件过流故障
#define  HWOVERCURRENT        0x02010201     //硬件过流故障
#define  UOVERCURRENT         0x12010201     //硬件过流故障
#define  VOVERCURRENT         0x22010201     //硬件过流故障
#define  OUTSHORT2GND0        0x02100210     //输出对地短路
#define  OUTSHORT2GND1        0x12100210     //输出对地短路
#define  UVWLINKERR           0x02200220     //UVW接线错误，需要对调任意两相接线
#define  UVW_FEICHE           0x02340234     //UVW三相接错线后的飞车报警
#define  CTRLPOWERUDVDC1      0x14300430     //控制电源欠电压
#define  CTRLPOWERUDVDC2      0x24300430     //控制电源欠电压
#define  ENCDERR_Z            0x07400740     //编码器Z干扰故障
#define  ENCDERR_Z2           0x27400740     //编码器Z干扰故障
#define  ENCDERR_Z3           0x37400740     //编码器Z干扰故障
#define  ENCDERR_Z4           0x47400740     //编码器Z干扰故障
#define  ENCDERR_Z5           0x57400740     //编码器Z干扰故障
#define  ENCDERR_Z6           0x67400740     //编码器Z干扰故障
#define  ENCDERR_Z7           0x77400740     //编码器Z干扰故障
#define  ENCDERR_Z8           0x87400740     //编码器Z干扰故障
#define  ENCDERR_AB           0x07410740     //编码器AB干扰故障
#define  HDHENCDERR_P         0x67456745     //编码器Z干扰故障
#define  AI_SAMPLE_OV_ERR     0x08340834     //DSP的AD采样过压故障
#define  FPGA_AD_SAMPLE_ERR   0x08350835     //FPGA报出的AD采样故障 
#define  ENCDPMERR            0x0A330A33     //编码器参数异常
#define  ENFBCHKERR           0x0A340A34     //编码器回送校验异常
#define  MULTIRERR            0x0A350A35     //Z断线

//----------NO.1 可复位故障码------------//
#define  DILOADERR            0x21302130     //DI功能分配故障(除了重复分配故障外，还包括分配功能超限，手轮中断定长分配功能不合理等故障)
#define  DOLOADERR            0x21312131     //DO功能分配故障 
#define  DQOVRCUR             0x22072207     //D/Q电流溢出；
#define  UDVDC                0x24102410     //欠电压
#define  OVERVOLTAGE          0x24002400     //过电压 
#define  OVERSPD              0x25002500     //速度超过最高转速 
#define  FPGAOVERSPD          0x25012500     //FPGA内部测速溢出
#define  INERTIAFAIL          0x26002600     //离线惯量辨识失败
#define  ANGINITFAIL          0x26022602     //角度辨识失败
#define  EXENCDSCLERR         0x27702770     //外部编码器标尺故障

//-------NO.2 故障码(可复位)-------------//
#define  SONVALIDERR          0x61216121     //伺服ON指令无效故障
#define  STOINOFF             0x63006300     //STO信号输入保护
#define  POWERPL              0x04206420     //电源线缺相
#define  POWERPL1             0x14206420     //电源线缺相
#define  POWERPL2             0x24206420     //电源线缺相
#if POWERDRIVER_TYPE==POWDRV_IS650
#define  POWERPL3             0x24216421     //继电器故障
#endif
#define  OEMOL                0x66106610     //驱动器过载
#define  PULSOUTOSPD          0x65106510     //分频脉冲输出过速  
#define  MTOL1                0x66206620     //电机过载 
#define  MTOL2                0x66216620     //电机过载
#define  MTOLOCKROTOR         0x66306630     //电机堵转过热保护 
#define  RADIATOROT           0x66506650     //散热片温度过高
#if POWERDRIVER_TYPE==POWDRV_IS650
#define  MOTORTEMPOT          0x66606660     //电机温度过高
#endif
#define  ENCDBATERR           0x07316731     //编码器电池失效
#define  ENCDMULTIERR         0x07336733     //编码器多圈计数错误
#define  ENCDMULTIOV          0x07356735     //编码器多圈计数器溢出  
#define  ENCDMULTIOV1         0x17356735     //编码器多圈计数器溢出 
#define  GEALSETERR0          0x0B036B03     //电子齿轮设定错误
#define  GEALSETERR1          0x1B036B03     //电子齿轮设定错误
#define  GEALSETERR2          0x2B036B03     //电子齿轮设定错误
#define  GEALSETERR3          0x3B036B03     //ECT电子齿轮设定错误
#define  COMMDISCONNECT       0x6D036D03     //通信连接中断
#define  MULTPOSCLASHFULCLOP  0x6B046B04     //全闭环和多段位置绝对模式混用时不能内外切换
//Canopen402添加
#define  COMMCONNECTOVERTIME  0x6D046D04     //Canopen通信超时
#define  COMMINITIALISE       0x6D056D05     //Canopen通信恢复初始化
#define  COMMSTOP             0x6D066D06     //Canopen通信复位
#define  COMBUSOFF            0x6D076D07     //总线脱离错误
#define  COMPDOLENERR         0x6D086D08     //总线PDO传输长度错误
#define  POSLIMITERR          0x6D096D09     //软件位置限制上限小于下限
#define  HOMEOFFSETERR        0x6D106D10     //原点偏置在软件位置限制之外
#define  SYNCPERIODERR        0x6D116D11     //同步帧误差超过同步周期的1/4
//Canopen402通信添加结束
#define  PSTNOERR             0x6B006B00     //位置偏差过大
#define  PULSINERR            0x6B016B01     //位置指令输入异常
#define  FULLCLOSEERR         0x6B026B02     //全闭环位置偏差过大

//通讯错误
#define  ECATLINKFAIL           0x6E016E01  //总线掉线
#define  ECATLINKCLOSE          0x6E026E02  //总线关闭
#define  ECATLINKOVERTIME       0x6E036E03  //总线连接超时
#define  ECATCARDLINKOVERTIME   0x6E046E04  //扩展卡内部通讯超时
#define  ECATADDRCONFLICT       0x6E056E05  //站号冲突
#define  ECATADDROVERLIMIT      0x6E066E06  //站号设置错误
#define  ECATSTATEMACHERRCHANGE 0x1E086E08  //通讯状态机异常
#define  ECATSYNCLOST           0x6E086E08  //同步丢失
#define  ECATPARAERR            0x6E096E09  //系统参数错误
#define  ECATCONFIGUREERR       0x6E0A6E0A  //组态错误
#define  EEPROMNULL             0x6E116E11  //未烧录ESI配置文件
#define  ECATINITFAILURE        0x6E126E12  //ECAT初始化失败
#define  SYNCPERIODSETERR       0x6E136E13  //ECAT同步周期设置不为PWM周期的倍数
#define  SYNCBIASERR            0x1E156E15  //ECAT同步信号偏差过大

//------------警告码-----------------//
#define  PULSOUTSETWARN         0xE110E110    //分频脉冲输出设定故障
#define  ORIGINOVERTIME         0xE601E601    //原点复归回零超时错误
#define  AIZEROBIASOVWARN       0xE831E831    //AI零偏过大
#define  EMERGENCYSTOPWARN      0xE900E900    //紧急停机警告
#define  RBOVERLOADWARN         0xE920E920    //再生制动电阻过载
#define  BLKOVERCURRENT         0xE921E921    //再生制动电阻过流
#define  RBTOOSMALLWARN         0xE922E922    //外接再生制动电阻阻值过小
#define  MTOFFLINE              0xE939E939    //电机动力线断线
#define  ABSMOTALARM            0xE980E980    //编码器算法异常警告
#define  MTOLWARN               0xE909E909    //电机过载警告
#define  PCHGDWARN              0xE941E941    //需要重新接通电源的参数变更
#define  PRAMSVWARN             0xE942E942    //参数存储频繁警告
#define  PNOTWARN               0xE950E950    //正向超程警告
#define  NOTWARN                0xE952E952    //负向超程警告
#define  POWERPLWARN            0xE990E990    //电源缺相警告
#define  COMMADDRCONFLICT       0xE994E994    //CANLINK地址冲突

#define  CANRECOVBUSOFF         0xE995E995    //总线恢复警告
#define  CANPASSIVEERR          0xE996E996    //总线被动错误警告
#define  POSREACHLIMT           0xE997E997    //位置指令或位置反馈达到极限值
#define  ODVALUEERR             0xE998E998    //对象字典数值设置错误

#define  IDENTIFYWARN           0xEA40EA40    //参数辨识失败
#define  IDENTIFYWARN_H0806     0x0806EA40    //参数辨识失败 H0806=2
#define  IDENTIFYWARN_000       0x0000EA40    //参数辨识失败 采样错误
#define  IDENTIFYWARN_001       0x0001EA40    //参数辨识失败 检验上升时间错误
#define  IDENTIFYWARN_002       0x0002EA40    //参数辨识失败 检验超调量错误
#define  IDENTIFYWARN_003       0x0003EA40    //参数辨识失败 Q轴电流稳态误差检验函数
#define  IDENTIFYWARN_004       0x0004EA40    //参数辨识失败 Q轴电流稳态误差检验函数
#define  ENCDBATWARN            0x0730E730    //编码器电池报警
#define  ENCODEROT              0x0760E760    //编码器过热 
#define  ENCDTXCOMMERR          0x0732E732    //编码器TX端通讯校验错误



//尼康编码器报警
#define  NKENCRXERR0          0x07550755     //尼康编码器通讯故障
#define  NKENCRXERR1          0x17550755     //尼康编码器通讯故障
#define  NKENBUSY             0x07670756     //尼康编码器
#define  NKENCOVERLIM         0x0765E765     //尼康编码器超限 （过热 过速 编码器EEPROM访问异常）

//旋变报警
#define  ROTTRANERR_Z2        0x27500750     //编码器Z干扰故障
#define  ROTTRANERR_Z3        0x37500750     //编码器Z干扰故障
#define  ROTTRANERR_Z4        0x47500750     //编码器Z干扰故障
#define  ROTTRANERR_Z5        0x57500750     //编码器Z干扰故障
#define  ROTTRANERR_Z6        0x67500750     //编码器Z干扰故障
//---------未用到的故障和警告------------//
/*
#define  RWABSROMERR          0x0135     //读写ABS ENC中的EEPROM错误
#define  PLSLSSMIN            0x0150     //多段位置脉冲数过少
#define  POSSPDLSSMIN         0x0151     //多段位置速度设定过小
#define  GDERR                0x0320     //GD电路失效故障           
#define  ENCDBAKERR           0x0700     //编码器备份故障    
#define  ENCDSUMERR           0x0720     //编码器和校验故障        
#define  ENCDOSPD             0x0750     //编码器超速
#define  ENCODEROT            0x0760     //编码器过热
#define  AD_SAMPLE_ERR        0x0832     //AD采样故障2 (既 转换未完成故障 对内)
#define  INIT_CURR_SAMPLE_ERR 0x0833     //电流采样故障1  零偏检测故障
#define  SYSTEMERR            0x0880     //系统故障
#define  PREVNTOUTCNTRL       0x0A10     //防止失控检出
#define  ENCDCLRERR           0x0A30     //编码器清除故障
#define  PSTNSONOERR          0x0B01     //伺服ON时位置偏差过大
#define  OUTPLERR             0x0C11     //输出缺相 
#define  ENCDCOMMERR          0x0A20     //编码器通信故障
#define  ENCDACCERR           0x0A31     //编码器通信位置数据加速度异常
#define  ENCDTIMERERR         0x0A32     //编码器通信定时器故障

//----------------------------------------//
#define  INITPCOMERR          0x2104     //参数组合故障
#define  INITAllClSLPERR      0x2112      //全闭环参数设定故障       初始化时
#define  INITCOMBERR          0x2113      //组合错误                初始化时
#define  AISELCTERR           0x21402140     //AI选择故障
#define  SPDLSSMIN            0x2152     //多段速度执行时间太短
#define  PIDSELCTERR          0x2160     //PID控制器输出分配故障
//#define  SDMFLTU            0x2205     //U相Sigma-Delta调制器故障,只报警
//#define  SDMFLTV            0x2206     //V相Sigma-Delta调制器故障
#define  RECRTERR             0x2300      //再生故障 
#define  MAINWIREERR          0x2320      //主回路电源接线故障      初始化时  

#define  PREFOVERSPD          0x2501     //位置指令过速
#define  PULSOUTOSPD          0x2510      //分频脉冲输出过速  
#define  VIBERR               0x2520      //振动故障 
#define  SELFADJERR           0x2521      //自动调谐故障
#define  DBOL                 0x2630      //DB过载
#define  SHTUPCLRES           0x2640      //冲击电流限制电阻 
#define  EXENCDMDERR          0x2771      //外部编码器模块故障
#define  EXENCDSNERR          0x2772      //外部编码器传感器故障
#define  EXENCDPERR           0x2773      //外部编码器位置异常(绝对式) 
#define  EXENCDOSPD           0x2775      //外部编码器超速故障 
#define  EXENCDOT             0x2776      //外部编码器过热故障

//--------------------------------------//
#define  RECTOL              0x6310     //再生过载
#define  MTOROVHEATERR        0x6651     //电机过热故障 
#define  INITSPDADERR        0x6810     //速度指令AD故障             初始化时
#define  INITSPDADCONVR      0x6811     //速度指令AD转换数据异常     初始化时
#define  INITTORQADERR       0x6820     //转矩指令AD故障             初始化时 
#define  POEBYSPDL           0x6B02     //伺服ON时速度限制引起的位置偏差过大故障
#define  ORGINPUT             0x6B05     //原点复归时有指令输入
#define  MLPOE               0x6B10     //电机-负载位置间偏差过大 
#define  RS232CMMERR        0x6D00     //RS232C通信连接故障          初始化时
#define  RS485CMMERR        0x6D01     //RS485通信连接故障           初始化时
#define  DOPCMMERR          0x6E00     //数字操作器通信故障，        初始化时

#define  PERRWARN          0xE900    //偏差位置过大 
#define  SONPERWARN        0xE901    //伺服ON时位置偏差过大
#define  MTOLWARN          0xE909    //电机过载警告
#define  OEMOLWARN         0xE910    //驱动器过载警告
#define  VIBWARN           0xE911    //振动
#define  ABENCDBATWARN     0xE930    //绝对值编码器电池异常
#define  MTHETA_ERR_OVER   0xE960    //编码器角度重新初始化时偏离过大故障
#define  SVUNREADY         0xE970    //伺服未准备好且伺服On警告
#define  RADOTWARN         0xE981    //散热片过热警告
#define  MTOROVHEATWARN    0xE982    //电机过热警告
#define  COMMCHECKWARN     0xE991    //通信模块自检失败告警
#define  COMMTRANSMITWARN  0xE992    //通信模块通信异常告警
#define  UDVDCWARN              0xE971E971    //欠电压警告
*/


//--------------------------------------//
#define  CLRACK        0xFE1F  //清除响应标志位
#define  ALRMRST       0xFFFC  //故障复位时清除相应标志位
#define  VALID         1
#define  INVALID       0
#define  TRUE          1
#define  FALSE         0
//------记录故障信息--显示用-----------// 
#define  ERRMSG_MEMLEN_EACH         9

/* Exported_Macros -----------------------------------------------------------*/
/* 带参数的宏定义 */

  	                                                                     
/* Exported_Types ------------------------------------------------------------*/ 
/* 结构体变量类型定义 枚举变量类型定义 */ 


/* Exported_Variables --------------------------------------------------------*/
/* 可供其它文件调用变量的声明 */

/* Exported_Functions --------------------------------------------------------*/
/* 可供其它文件调用的函数的声明 */ 
extern void PostErrMsg(Uint32 PostErrCode);
extern void WarnAutoClr(Uint32 PostWarnCode);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of FUNC_ERRORCODE_H definition

/********************************* END OF FILE *********************************/






