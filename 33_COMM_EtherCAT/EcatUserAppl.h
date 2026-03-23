/*
  ***************(C) COPYRIGHT 2013  Inovance Technology Co., Ltd***************

  * @file	   		: EcatUserAppl.h
  * @author    		:
  * @Description    :
  ******************************************************************************
  */
/*******************************************************************************
  * 修改       		：
  * 版本	  		：V1.00
  * 时间	   		：
  * 说明 	  		：
********************************************************************************/

#ifndef _EcatUserAppl_H_
#define _EcatUserAppl_H_

#include "ecat_def.h"

#include "ecatslv.h"
#include "ecatappl.h"

#include "objdef.h"

#include "ecathw.h"

#include "StmEcatHW.h"


/*****************************************************************************************
*
*                        defines
*
*****************************************************************************************/


/***************************************
*  ControlWord Commands Mask (IEC61800_184e)
***************************************/
#define CONTROLWORD_COMMAND_SHUTDOWN_MASK			0x0087
#define CONTROLWORD_COMMAND_SWITCHON_MASK			0x00C7
#define CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION_MASK	0x008F
#define CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK			0x0082
#define CONTROLWORD_COMMAND_QUICKSTOP_MASK			0x0086
#define CONTROLWORD_COMMAND_DISABLEOPERATION_MASK		0x008F
#define CONTROLWORD_COMMAND_ENABLEOPERATION_MASK		0x008F
#define CONTROLWORD_COMMAND_FAULTRESET_MASK			0x0080


/***************************************
*  ControlWord Commands (IEC61800_184e)
***************************************/
#define CONTROLWORD_COMMAND_SHUTDOWN			0x0006
#define CONTROLWORD_COMMAND_SWITCHON			0x0007
#define CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION	0x000F
#define CONTROLWORD_COMMAND_DISABLEVOLTAGE		0x0000
#define CONTROLWORD_COMMAND_QUICKSTOP			0x0002
#define CONTROLWORD_COMMAND_DISABLEOPERATION		0x0007
#define CONTROLWORD_COMMAND_ENABLEOPERATION		0x000F
#define CONTROLWORD_COMMAND_FAULTRESET			0x0080


/***************************************
*  StatusWord Masks and Flags
***************************************/
#define STATUSWORD_STATE_MASK				0x006F
#define STATUSWORD_VOLTAGE_ENABLED			0x0010          /* 表示启用高压 */
#define STATUSWORD_WARNING				0x0080
#define STATUSWORD_MANUFACTORSPECIFIC			0x0100
#define STATUSWORD_INTERNAL_LIMIT			0x0800
#define STATUSWORD_REMOTE				0x0200          /* 如果控制字设置处理*/
#define STATUSWORD_TARGET_REACHED			0x0400
#define STATUSWORD_INTERNALLIMITACTIVE			0x0800
#define STATUSWORD_DRIVE_FOLLOWS_COMMAND		0x1000          /*用于循环同步模式*/
#define STATUSWORD_DRIVE_FOLLOWINGERRORHOMINGERR	0x2000          /* 用于循环同步模式
 */


/***************************************
*  StatusWord
***************************************/
#define STATUSWORD_STATE_NOTREADYTOSWITCHON	0x0000
#define STATUSWORD_STATE_SWITCHEDONDISABLED	0x0020
#define STATUSWORD_STATE_READYTOSWITCHON	0x0021
#define STATUSWORD_STATE_SWITCHEDON		0x0023
#define STATUSWORD_STATE_OPERATIONENABLED	0x0027
#define STATUSWORD_STATE_QUICKSTOPACTIVE	0x0007
#define STATUSWORD_STATE_FAULTREACTIONACTIVE	0x000F
#define STATUSWORD_STATE_FAULT			0x0008


/***************************************
*  Modes
***************************************/
/* aus IEC61800-7-200_22G_184e_FDIS_ETG.pdf */


/*
 * 0 No mode change/no mode assigned
 * +1 Profile position mode
 * +2 Velocity mode
 * +3 Profile velocity mode
 * +4 Torque profile mode
 * +5 reserved
 * +6 Homing mode
 * +7 Interpolated position mode
 * +8 Cyclic sync position mode
 * +9 Cyclic sync velocity mode
 * +10 Cyclic sync torque mode
 */
#define MODE_NOMODE			0x0000
#define MODE_PROFILEPOSITION		0x0001
#define MODE_VELOCITY			0x0002
#define MODE_PROFILEVELOCITY		0x0003
#define MODE_TORQUEPROFILE		0x0004
#define MODE_RESERVED			0x0005
#define MODE_HOMING			0x0006
#define MODE_INTERPOLATIONPOSITION	0x0007
#define MODE_CYCLICSYNCPOSITION		0x0008
#define MODE_CYCLICSYNCVELOCITY		0x0009
#define MODE_CYCLICSYNCTORQUE		0x000A


/***************************************
*  CiA402 Statemachine
***************************************/
#define STATE_NOT_READY_TO_SWITCH_ON	0x0001          /* O */
#define STATE_SWITCH_ON_DISABLED	0x0002          /* O */
#define STATE_READY_TO_SWITCH_ON	0x0004          /* M */
#define STATE_SWITCHED_ON		0x0008          /* M */
#define STATE_OPERATION_ENABLED		0x0010          /* M */
#define STATE_QUICK_STOP_ACTIVE		0x0020          /* O */
#define STATE_FAULT_REACTION_ACTIVE	0x0040          /* M */
#define STATE_FAULT			0x0080          /* M */


/***************************************
*  CiA402 Modes of Operation (object 0x6060) (IEC61800_184e)
***************************************/
/* -128 to -1 Manufacturer-specific operation modes */
#define NO_MODE			0
#define PROFILE_POSITION_MODE	1
#define VELOCITY_MODE		2
#define PROFILE_VELOCITY_MOCE	3
#define PROFILE_TORQUE_MODE	4
/* 5 reserved */
#define HOMING_MODE			6
#define INTERPOLATION_POSITION_MODE	7
#define CYCLIC_SYNC_POSITION_MODE	8
#define CYCLIC_SYNC_VELOCITY_MODE	9
#define CYCLIC_SYNC_TORQUE_MODE		10
/* +11 to +127 reserved */


/***************************************
*  CiA402 Error Codes (object 0x603F) (IEC61800_184e)
***************************************/
#define ERROR_SHORT_CIRCUIT_EARTH_LEAKAGE_INPUT			0x2110
#define ERROR_EARTH_LEAKAGE_INPUT				0x2120
#define ERROR_EARTH_LEAKAGE_PHASE_L1				0x2121
#define ERROR_EARTH_LEAKAGE_PHASE_L2				0x2122
#define ERROR_EARTH_LEAKAGE_PHASE_L3				0x2123
#define ERROR_SHORT_CIRCUIT_INPUT				0x2130 /* GENERICERROR */
#define ERROR_SHORT_CIRCUIT_PHASES_L1_L2			0x2131
#define ERROR_SHORT_CIRCUIT_PHASES_L2_L3			0x2132
#define ERROR_SHORT_CIRCUIT_PHASES_L3_L1			0x2133
#define ERROR_INTERNAL_CURRENT_NO1				0x2211
#define ERROR_INTERNAL_CURRENT_NO2				0x2212
#define ERROR_OVER_CURRENT_IN_RAMP_FUNCTION			0x2213
#define ERROR_OVER_CURRENT_IN_THE_SEQUENCE			0x2214
#define ERROR_CONTINUOUS_OVER_CURRENT_DEVICE_INTERNAL		0x2220  /* GENERICERROR */
#define ERROR_CONTINUOUS_OVER_CURRENT_DEVICE_INTERNAL_NO1	0x2221
#define ERROR_CONTINUOUS_OVER_CURRENT_DEVICE_INTERNAL_NO2	0x2222
#define ERROR_SHORT_CIRCUIT_EARTH_LEAKAGE_DEVICE_INTERNAL	0x2230
#define ERROR_EARTH_LEAKAGE_DEVICE_INTERNAL			0x2240
#define ERROR_SHORT_CIRCUIT_DEVICE_INTERNAL			0x2250
#define ERROR_CONTINUOUS_OVER_CURRENT				0x2310  /* GENERICERROR */
#define ERROR_CONTINUOUS_OVER_CURRENT_NO1			0x2311
#define ERROR_CONTINUOUS_OVER_CURRENT_NO2			0x2312
#define ERROR_SHORT_CIRCUIT_EARTH_LEAKAGE_MOTOR_SIDE		0x2320
#define ERROR_EARTH_LEAKAGE_MOTOR_SIDE				0x2330
#define ERROR_EARTH_LEAKAGE_PHASE_U				0x2331
#define ERROR_EARTH_LEAKAGE_PHASE_V				0x2332
#define ERROR_EARTH_LEAKAGE_PHASE_W				0x2333
#define ERROR_SHORT_CIRCUIT_MOTOR_SIDE				0x2340  /* GENERICERROR */
#define ERROR_SHORT_CIRCUIT_PHASES_U_V				0x2341
#define ERROR_EARTH_LEAKAGE_PHASE_V_W				0x2342
#define ERROR_EARTH_LEAKAGE_PHASE_W_U				0x2343
#define ERROR_LOAD_LEVEL_FAULT_I2T_THERMAL_STATE		0x2350
#define ERROR_LOAD_LEVEL_WARNING_I2T_THERMAL_STATE		0x2351
#define ERROR_MAINS_OVER_VOLTAGE				0x3110  /* GENERICERROR */
#define ERROR_MAINS_OVER_VOLTAGE_PHASE_L1			0x3111
#define ERROR_MAINS_OVER_VOLTAGE_PHASE_L2			0x3112
#define ERROR_MAINS_OVER_VOLTAGE_PHASE_L3			0x3113
#define ERROR_MAINS_UNDER_VOLTAGE				0x3120  /* GENERICERROR */
#define ERROR_MAINS_UNDER_VOLTAGE_PHASE_L1			0x3121
#define ERROR_MAINS_UNDER_VOLTAGE_PHASE_L2			0x3122
#define ERROR_MAINS_UNDER_VOLTAGE_PHASE_L3			0x3123
#define ERROR_PHASE_FAILURE					0x3130  /* GENERICERROR */
#define ERROR_PHASE_FAILURE_L1					0x3131
#define ERROR_PHASE_FAILURE_L2					0x3132
#define ERROR_PHASE_FAILURE_L3					0x3133
#define ERROR_PHASE_SEQUENCE					0x3134
#define ERROR_MAINS_FREQUENCY					0x3140  /* GENERICERROR */
#define ERROR_MAINS_FREQUENCY_TOO_GREAT				0x3141
#define ERROR_MAINS_FREQUENCY_TOO_SMALL				0x3142
#define ERROR_DC_LINK_OVER_VOLTAGE				0x3210  /* GENERICERROR */
#define ERROR_OVER_VOLTAGE_NO_1					0x3211
#define ERROR_OVER_VOLTAGE_NO_2					0x3212
#define ERROR_DC_LINK_UNDER_VOLTAGE				0x3220  /* GENERICERROR */
#define ERROR_UNDER_VOLTAGE_NO_1				0x3221
#define ERROR_UNDER_VOLTAGE_NO_2				0x3222
#define ERROR_LOAD_ERROR					0x3230
#define ERROR_OUTPUT_OVER_VOLTAGE				0x3310  /* GENERICERROR */
#define ERROR_OUTPUT_OVER_VOLTAGE_PHASE_U			0x3311
#define ERROR_OUTPUT_OVER_VOLTAGE_PHASE_V			0x3312
#define ERROR_OUTPUT_OVER_VOLTAGE_PHASE_W			0x3313
#define ERROR_ARMATURE_CIRCUIT					0x3320  /* GENERICERROR */
#define ERROR_ARMATURE_CIRCUIT_INTERRUPTED			0x3321
#define ERROR_FIELD_CIRCUIT					0x3330
#define ERROR_FIELD_CIRCUIT_INTERRUPTED				0x3331
#define ERROR_EXCESS_AMBIENT_TEMPERATURE			0x4110
#define ERROR_TOO_LOW_AMBIENT_TEMPERATURE			0x4120
#define ERROR_TEMPERATURE_SUPPLY_AIR				0x4130
#define ERROR_TEMPERATURE_AIR_OUTLET				0x4140
#define ERROR_EXCESS_TEMPERATURE_DEVICE				0x4210
#define ERROR_TOO_LOW_TEMPERATURE_DEVICE			0x4220
#define ERROR_TEMPERATURE_DRIVE					0x4300  /* GENERICERROR */
#define ERROR_EXCESS_TEMPERATURE_DRIVE				0x4310
#define ERROR_TOO_LOW_TEMPERATURE_DRIVE				0x4320
#define ERROR_TEMPERATURE_SUPPLY				0x4400  /* GENERICERROR */
#define ERROR_EXCESS_TEMPERATURE_SUPPLY				0x4410
#define ERROR_TOO_LOW_TEMPERATURE_SUPPLY			0x4420
#define ERROR_SUPPLY_ERROR					0x5100  /* GENERICERROR */
#define ERROR_SUPPLY_LOW_VOLTAGE				0x5110
#define ERROR_U1_SUPPLY_15V					0x5111
#define ERROR_U2_SUPPLY_24_V					0x5112
#define ERROR_U3_SUPPLY_5_V					0x5113
#define ERROR_U4_MANUFACTURER_SPECIFIC				0x5114
#define ERROR_U5_MANUFACTURER_SPECIFIC				0x5115
#define ERROR_U6_MANUFACTURER_SPECIFIC				0x5116
#define ERROR_U7_MANUFACTURER_SPECIFIC				0x5117
#define ERROR_U8_MANUFACTURER_SPECIFIC				0x5118
#define ERROR_U9_MANUFACTURER_SPECIFIC				0x5119
#define ERROR_SUPPLY_INTERMEDIATE_CIRCUIT			0x5120
/* #define ERROR_CONTROL                                    0x5200    //GENERICERROR */
#define ERROR_CONTROL_MEASUREMENT_CIRCUIT	0x5210
#define ERROR_CONTROL_COMPUTING_CIRCUIT		0x5220
#define ERROR_OPERATING_UNIT			0x5300
#define ERROR_POWER_SECTION			0x5400
#define ERROR_OUTPUT_STAGES			0x5410
#define ERROR_CHOPPER				0x5420
#define ERROR_INPUT_STAGES			0x5430
#define ERROR_CONTACTS_ERROR			0x5440
#define ERROR_CONTACT_1_MANUFACTURER_SPECIFIC	0x5441
#define ERROR_CONTACT_2_MANUFACTURER_SPECIFIC	0x5442
#define ERROR_CONTACT_3_MANUFACTURER_SPECIFIC	0x5443
#define ERROR_CONTACT_4_MANUFACTURER_SPECIFIC	0x5444
#define ERROR_CONTACT_5_MANUFACTURER_SPECIFIC	0x5445
#define ERROR_FUSES_ERROR			0x5450             /* GENERICERROR */
#define ERROR_S1_L1				0x5451
#define ERROR_S2_L2				0x5452
#define ERROR_S3_L3				0x5453
#define ERROR_S4_MANUFACTURER_SPECIFIC		0x5454
#define ERROR_S5_MANUFACTURER_SPECIFIC		0x5455
#define ERROR_S6_MANUFACTURER_SPECIFIC		0x5456
#define ERROR_S7_MANUFACTURER_SPECIFIC		0x5457
#define ERROR_S8_MANUFACTURER_SPECIFIC		0x5458
#define ERROR_S9_MANUFACTURER_SPECIFIC		0x5459
#define ERROR_HARDWARE_MEMORY			0x5500             /* GENERICERROR */
#define ERROR_RAM				0x5510
#define ERROR_ROM_EPROM				0x5520
#define ERROR_EEPROM				0x5530
#define ERROR_SOFTWARE_RESET_WATCHDOG		0x6010
/* 0x6301_TO_0x630F        ERROR_DATA_RECORD_NO_1_TO_NO_15 */
#define ERROR_LOSS_OF_PARAMETERS			0x6310
#define ERROR_PARAMETER_ERROR				0x6320
#define ERROR_POWER_ERROR				0x7100
#define ERROR_BRAKE_CHOPPER				0x7110          /* GENERICERROR */
#define ERROR_FAILURE_BRAKE_CHOPPER			0x7111
#define ERROR_OVER_CURRENT_BRAKE_CHOPPER		0x7112
#define ERROR_PROTECTIVE_CIRCUIT_BRAKE_CHOPPER		0x7113
#define ERROR_MOTOR_ERROR				0x7120          /* GENERICERROR */
#define ERROR_MOTOR_BLOCKED				0x7121
#define ERROR_MOTOR_ERROR_OR_COMMUTATION_MALFUNC	0x7122
#define ERROR_MOTOR_TILTED				0x7123
#define ERROR_MEASUREMENT_CIRCUIT			0x7200
#define ERROR_SENSOR_ERROR				0x7300          /* GENERICERROR */
#define ERROR_TACHO_FAULT				0x7301
#define ERROR_TACHO_WRONG_POLARITY			0x7302
#define ERROR_RESOLVER_1_FAULT				0x7303
#define ERROR_RESOLVER_2_FAULT				0x7304
#define ERROR_INCREMENTAL_SENSOR_1_FAULT		0x7305
#define ERROR_INCREMENTAL_SENSOR_2_FAULT		0x7306
#define ERROR_INCREMENTAL_SENSOR_3_FAULT		0x7307
#define ERROR_SPEED					0x7310
#define ERROR_POSITION					0x7320
#define ERROR_COMPUTATION_CIRCUIT			0x7400
#define ERROR_COMMUNICATION				0x7500
#define ERROR_SERIAL_INTERFACE_NO_1			0x7510
#define ERROR_SERIAL_INTERFACE_NO_2			0x7520
#define ERROR_DATA_STORAGE_EXTERNAL			0x7600
#define ERROR_TORQUE_CONTROL				0x8300    /* GENERICERROR */
#define ERROR_EXCESS_TORQUE				0x8311
#define ERROR_DIFFICULT_START_UP			0x8312
#define ERROR_STANDSTILL_TORQUE				0x8313
#define ERROR_INSUFFICIENT_TORQUE			0x8321
#define ERROR_TORQUE_FAULT				0x8331
#define ERROR_VELOCITY_SPEED_CONTROLLER			0x8400
#define ERROR_POSITION_CONTROLLER			0x8500
#define ERROR_POSITIONING_CONTROLLER			0x8600    /* GENERICERROR */
#define ERROR_FOLLOWING_ERROR				0x8611
#define ERROR_REFERENCE_LIMIT				0x8612
#define ERROR_SYNC_CONTROLLER				0x8700
#define ERROR_WINDING_CONTROLLER			0x8800
#define ERROR_PROCESS_DATA_MONITORING			0x8900
/* #define ERROR_CONTROL                                    0x8A00 */
#define ERROR_DECELERATION		0xF001
#define ERROR_SUB_SYNCHRONOUS_RUN	0xF002
#define ERROR_STROKE_OPERATION		0xF003
/*
 * #define ERROR_CONTROL                                    0xF004
 * 0xFF00_TO_0xFFFF        MANUFACTURER_SPECIFIC
 */


/*******************************************************************
 * CiA402 generic error option code values
 * Note: Not all values are valid for each error option code.
 * A detailed description of the option code values are listed in the specification IEC 61800-7-200
 * 0x605B    : action in state transition 8
 * 0x605C    : action in state transition 5
 ********************************************************************/
#define DISABLE_DRIVE		0               /* options: 0x605B; 0x605C; 0x605E */
#define SLOW_DOWN_RAMP		1               /* options: 0x605B; 0x605C; 0x605E */
#define QUICKSTOP_RAMP		2               /* options: 0x605E */
#define STOP_ON_CURRENT_LIMIT	3               /* options: 0x605E */
#define STOP_ON_VOLTAGE_LIMIT	4               /* options: 0x605E */


/*******************************************************************
 * Specific values for Quick stop option code (object 0x605A) (IEC61800_184e)
 * indicated the quick stop function
 ********************************************************************/

/* -32768 to -1        MANUFACTURER_SPECIFIC */
#define SLOWDOWN_RAMP_NO_TRANSIT	5       /* Slow down on slow down ramp and stay in Quick Stop Active */
#define QUICKSTOP_RAMP_NO_TRANSIT	6       /* Slow down on quick stop ramp and stay in Quick Stop Active */
#define CURRENT_LIMIT_NO_TRANSIT	7       /* Slow down on current limit and stay in Quick Stop Active */
#define VOLTAGE_LIMIT_NO_TRANSIT	8       /* Slow down on voltage limit and stay in Quick Stop Active */
/* 9 to 32767        RESERVED */

#define MAX_AXES 1

/*Module Identifications*/
#define CSV_CSP_MODULE_ID	0x00119800
#define CSP_MODULE_ID		0x00219800
#define CSV_MODULE_ID		0x00319800


/*****************************************************************************************
*
*                        object definitions (typedef; Object dictionary entry)
*
*****************************************************************************************/
#ifdef _CiA402_
/*PDO assign entry description*/
OBJCONST TSDOINFOENTRYDESC OBJMEM asPDOAssignEntryDesc[] = {
	{ DEFTYPE_UNSIGNED8,  0x08, (ACCESS_READ | ACCESS_WRITE_PREOP) },
	{ DEFTYPE_UNSIGNED16, 0x10, (ACCESS_READ | ACCESS_WRITE_PREOP) }
};
#endif


/*******************************************
** Object 0x1600    -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1600;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1600[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1600[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1701    -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1701;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1701[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1701[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1702    -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1702;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1702[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1702[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif

/*******************************************
** Object 0x1703    -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1703;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1703[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1703[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1704   -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1704;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1704[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1704[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif

/*******************************************
** Object 0x1705    -    csp/csv RxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1705;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1705[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                              /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1705[] = "RxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1A00    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1A00;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1A00[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ | ACCESS_WRITE_PREOP) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1A00[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1B01    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1B01;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1B01[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1B01[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1B02    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1B02;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1B02[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1B02[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1B03    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1B03;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1B03[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1B03[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1B04    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1B04;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1B04[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1B04[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif

/*******************************************
** Object 0x1B05    -    csp/csv TxPDO
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT32	aEntries[10];
} OBJ_STRUCT_PACKED_END
TOBJ1B05;
#ifdef _CiA402_
OBJCONST TSDOINFOENTRYDESC OBJMEM	asEntryDesc0x1B05[11] = {
	{ DEFTYPE_UNSIGNED8,  0x8,  (ACCESS_READ ) },       /* Subindex 000 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 001*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 002*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 003*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 004 */
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 005*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 006*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 007*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 008*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) },       /* SubIndex 009*/
	{ DEFTYPE_UNSIGNED32, 0x20, (ACCESS_READ ) }
};                                                                          /* SubIndex 00A*/
OBJCONST UCHAR OBJMEM			aName0x1B05[] = "TxPDO\000SubIndex001\000SubIndex002\000SubIndex003\000SubIndex004\000SubIndex005\000SubIndex006\000SubIndex007\000SubIndex008\000SubIndex009\000SubIndex00A\000\377";
#endif


/*******************************************
** Object 0x1C12    -    SM2 PDO assign
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT16	aEntries[MAX_AXES];
} OBJ_STRUCT_PACKED_END
TOBJ1C12;
#ifdef _CiA402_
OBJCONST UCHAR OBJMEM aName0x1C12[] = "RxPDO assign";
#endif


/*******************************************
** Object 0x1C13    -    SM3 PDO assign
********************************************/
typedef struct OBJ_STRUCT_PACKED_START {
	UINT16	u16SubIndex0;
	UINT16	aEntries[MAX_AXES];
} OBJ_STRUCT_PACKED_END
TOBJ1C13;
#ifdef _CiA402_
OBJCONST UCHAR OBJMEM aName0x1C13[] = "TxPDO assign";
#endif



/*****************************************************************************************
*
*                        generic Axis structure
*
*****************************************************************************************/

/* ************************************ */

typedef struct OBJ_STRUCT_PACKED_START
{
	TOBJ1600	sRxPDOMap0;                     /* RxPDO */


	TOBJ1701	sRxPDOMap1;                     /* RxPDO */
	TOBJ1702	sRxPDOMap2;                     /* RxPDO */
	TOBJ1703	sRxPDOMap3;                     /* RxPDO */
	TOBJ1704	sRxPDOMap4;                     /* RxPDO */
	TOBJ1705	sRxPDOMap5;                     /* RxPDO */
	TOBJ1A00	sTxPDOMap0;                     /* TxPDO */
	TOBJ1B01	sTxPDOMap1;                     /* TxPDO */
	TOBJ1B02	sTxPDOMap2;                     /* TxPDO */
	TOBJ1B03	sTxPDOMap3; 					/* TxPDO */
	TOBJ1B04	sTxPDOMap4; 					/* TxPDO */

	
}OBJ_STRUCT_PACKED_END
CiA402Objects;

#define CIA402_OBJECTS_SIZE SIZEOF(CiA402Objects)
typedef struct
{
	CiA402Objects	Objects;
	TOBJECT OBJMEM	* ObjDic;
}
TCiA402Axis, *pTCiA402Axis;


#ifdef _CiA402_
#define PROTO
#else
#define PROTO extern
#endif

PROTO TOBJ1C12 sRxPDOassign
#ifdef _CiA402_
	= { 1, { 0x1701 } }
#endif
;

PROTO TOBJ1C13 sTxPDOassign
#ifdef _CiA402_
	= { 1, { 0x1B01 } }
#endif
;

PROTO CiA402Objects DefCiA402ObjectValues
#ifdef _CiA402_
	= {
	{ 3, { 0x60400010, 0x607A0020, 0x60B80010, 0, 0, 0, 0, 0, 0, 0 } },      /* TOBJ1600*/
	{ 4, { 0x60400010, 0x607A0020, 0x60B80010, 0x60FE0020, 0, 0, 0, 0, 0, 0 } },      /* TOBJ1701*/
	{ 7, { 0x60400010, 0x607A0020, 0x60FF0020, 0x60710010, 0x60600008, 0x60B80010, 0x607F0020, 0, 0, 0 } },      /* TOBJ1702*/
	{ 7, { 0x60400010, 0x607A0020, 0x60FF0020, 0x60600008, 0x60B80010, 0x60E00010, 0x60E10010, 0, 0, 0 } },      /* TOBJ1703*/
	{ 9, { 0x60400010, 0x607A0020, 0x60FF0020, 0x60710010, 0x60600008, 0x60B80010, 0x607F0020, 0x60E00010, 0x60E10010, 0 } },      /* TOBJ1704*/
	{ 8, { 0x60400010, 0x607A0020, 0x60FF0020, 0x60600008, 0x60B80010, 0x60E00010, 0x60E10010, 0x60B20010, 0, 0 } },      /* TOBJ1705*/
	{ 7, { 0x60410010, 0x60640020, 0x60B90010, 0x60BA0020, 0x60BC0020, 0x603F0010, 0x60FD0020, 0, 0, 0 } },      /*TOBJ1A00*/
	{ 9, { 0x603F0010, 0x60410010, 0x60640020, 0x60770010, 0x60F40020, 0x60B90010, 0x60BA0020, 0x60BC0020, 0x60FD0020,  0 } },      /*TOBJ1B01*/
	{ 9, { 0x603F0010, 0x60410010, 0x60640020, 0x60770010, 0x60610008, 0x60B90010, 0x60BA0020, 0x60BC0020, 0x60FD0020, 0 } },      /*TOBJ1B02*/
	{ 10, { 0x603F0010,0x60410010, 0x60640020, 0x60770010, 0x60F40020, 0x60610008, 0x60B90010, 0x60BA0020, 0x60BC0020, 0x60FD0020 } },      /*TOBJ1B03*/
	{ 10, { 0x603F0010, 0x60410010, 0x60640020, 0x60770010, 0x60610008, 0x60F40020, 0x60B90010, 0x60BA0020, 0x60BC0020, 0x606C0020 } },      /*TOBJ1B04*/
	}

#endif
;

PROTO TOBJECT OBJMEM ApplicationObjDic[]
#ifdef _CiA402_
	= { /* Object 0x1C12 */
	{ NULL, NULL, 0x1C12, { DEFTYPE_UNSIGNED16, MAX_AXES | (OBJCODE_ARR << 8) }, asPDOAssignEntryDesc, aName0x1C12, &sRxPDOassign, NULL, NULL, 0x0000 },
	/* Object 0x1C13 */
	{ NULL, NULL, 0x1C13, { DEFTYPE_UNSIGNED16, MAX_AXES | (OBJCODE_ARR << 8) }, asPDOAssignEntryDesc, aName0x1C13, &sTxPDOassign, NULL, NULL, 0x0000 },
	{ NULL, NULL, 0xFFFF, { 0,		    0				  }, NULL,		   NULL,	NULL,	       NULL }
	}

#endif
;

PROTO TOBJECT OBJMEM DefCiA402AxisObjDic[]
#ifdef _CiA402_
	= { /* Object 0x1600 */
	{ NULL, NULL, 0x1600, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1600, aName0x1600, NULL, NULL, NULL, 0x0000 },
		
	{ NULL, NULL, 0x1701, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1701, aName0x1701, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1702, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1702, aName0x1702, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1703, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1703, aName0x1703, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1704, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1704, aName0x1704, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1705, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1705, aName0x1705, NULL, NULL, NULL, 0x0000 },
	/* Object 0x1A00 */
	{ NULL, NULL, 0x1A00, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1A00, aName0x1A00, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1B01, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1B01, aName0x1B01, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1B02, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1B02, aName0x1B02, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1B03, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1B03, aName0x1B03, NULL, NULL, NULL, 0x0000 },
	
	{ NULL, NULL, 0x1B04, { DEFTYPE_PDOMAPPING, 10 | (OBJCODE_REC << 8) }, asEntryDesc0x1B04, aName0x1B04, NULL, NULL, NULL, 0x0000 },
	{ NULL, NULL, 0xFFFF, { 0,		    0			    }, NULL,		  NULL,	       NULL, NULL }
	}

#endif
;
extern TCiA402Axis	LocalAxes;


PROTO void APPL_Application(void);

PROTO void APPL_AckErrorInd(UINT16 stateTrans);

PROTO UINT16 APPL_StartMailboxHandler(void);

PROTO UINT16 APPL_StopMailboxHandler(void);

PROTO UINT16 APPL_StartInputHandler(UINT16 *pIntMask);

PROTO UINT16 APPL_StopInputHandler(void);

PROTO UINT16 APPL_StartOutputHandler(void);

PROTO UINT16 APPL_StopOutputHandler(void);

PROTO UINT16 APPL_GenerateMapping(UINT16 *pInputSize, UINT16 *pOutputSize);

PROTO void APPL_InputMapping(UINT16* pData);

PROTO void APPL_OutputMapping(UINT16* pData);

PROTO void LocalError(UINT16 ErrorCode);


PROTO UINT8 EcatAppl_Init(void);

PROTO void APPL_StartIRQHandler(void);




#undef PROTO

#endif
/************************************** The End Of File **************************************/
