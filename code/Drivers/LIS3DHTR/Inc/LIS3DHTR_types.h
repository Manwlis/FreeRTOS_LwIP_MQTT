/*
 * LIS3DHTR_types.h
 */

#ifndef LIS3DHTR_INC_LIS3DHTR_TYPES_H_
#define LIS3DHTR_INC_LIS3DHTR_TYPES_H_


#include <stdint.h>
#include "i2c.h"


//#define DEBUG_LIS3DHTR

enum
{
	LIS3_DISABLE ,
	LIS3_ENABLE
};

enum
{
	LIS3_NO_OVERRUN ,
	LIS3_OVERRUN
};

enum
{
	LIS3_NO_NEW_DATA ,
	LIS3_NEW_DATA
};

/* Register mapping ------------------------------------------------------------*/
// Enumeration of the memory map of the LIS3DHTR
typedef enum
{
#define X( name , address , access ) LIS3DHTR_##name ,
#include "LIS3DHTR_mmap.def"
#undef X
	LIS3DHTR_NUM_REGS
} LIS3DHTR_reg_id_t;

// Access types of the LIS3DHTR registers
typedef enum
{
	REG_R , REG_W , REG_RW
} LIS3DHTR_reg_access_t;

// Holds all information on a register of the peripheral
typedef struct
{
#ifdef DEBUG_LIS3DHTR
	const char* const name;
#endif
	const uint8_t address;
	const LIS3DHTR_reg_access_t access;
} LIS3DHTR_reg_t;

// Device information
typedef struct
{
#ifdef DEBUG_LIS3DHTR
	const char* const name;
#endif
	const LIS3DHTR_reg_t* const memory_map;
	const I2C_HandleTypeDef* const i2c_handle;
	const uint8_t i2c_address;
} LIS3DHTR_device_t;


/* Register descriptions ------------------------------------------------------------*/
// used to generate the register field position and width enums
#define GEN_POS(name, position , width) \
    name##_pos = position ,

#define GEN_MASK(name, position , width) \
    name##_mask = ((1U << width) - 1) << position ,

// STATUS_REG_AUX (07h) ----------
#define STATUS_REG_AUX_FIELDS( X ) \
	X( STATUS_REG_AUX_321OR , 0 , 1 ) \
	X( STATUS_REG_AUX_3OR   , 1 , 1 ) \
	X( STATUS_REG_AUX_2OR   , 2 , 1 ) \
	X( STATUS_REG_AUX_1OR   , 3 , 1 ) \
	X( STATUS_REG_AUX_321DA , 4 , 1 ) \
	X( STATUS_REG_AUX_3DA   , 5 , 1 ) \
	X( STATUS_REG_AUX_2DA   , 6 , 1 ) \
	X( STATUS_REG_AUX_1DA   , 7 , 1 )

typedef enum
{
	STATUS_REG_AUX_FIELDS( GEN_POS )
} STATUS_REG_AUX_field_positions;

typedef enum
{
	STATUS_REG_AUX_FIELDS( GEN_MASK )
} STATUS_REG_AUX_field_masks;

// CTRL_REG0 (1Eh) ----------
#define CTRL_REG0_FIELDS( X ) \
		X( CTRL_REG0_SDO_PU_DISC , 7 , 1 )
// rest of the bits should be left in their default state x0010000

typedef enum
{
	CTRL_REG0_FIELDS( GEN_POS )
} CTRL_REG0_field_positions;

typedef enum
{
	CTRL_REG0_FIELDS( GEN_MASK )
} CTRL_REG0_field_masks;


typedef enum
{
	PULLUP_CONNECTED = 0 ,
	PULLUP_DISCONECTED = 1
} CTRL_REG0_SDO_PU_DISC_state_t;

// TEMP_CFG_REG (1Fh) ----------
#define TEMP_CFG_REG_FIELDS( X ) \
	X( TEMP_CFG_TEMP_EN , 6 , 1 ) \
	X( TEMP_CFG_ADC_EN  , 7 , 1 )

typedef enum
{
	TEMP_CFG_REG_FIELDS( GEN_POS )
} TEMP_CFG_REG_field_positions;

typedef enum
{
	TEMP_CFG_REG_FIELDS( GEN_MASK )
} TEMP_CFG_REG_field_masks;

// CTRL_REG1 (20h) ----------
#define CTRL_REG1_FIELDS( X ) \
	X( CTRL_REG1_Xen  , 0 , 1 ) \
	X( CTRL_REG1_Yen  , 1 , 1 ) \
	X( CTRL_REG1_Zen  , 2 , 1 ) \
	X( CTRL_REG1_LPen , 3 , 1 ) \
	X( CTRL_REG1_ODR  , 4 , 4 )

typedef enum
{
	CTRL_REG1_FIELDS( GEN_POS )
} CTRL_REG1_field_positions;

typedef enum
{
	CTRL_REG1_FIELDS( GEN_MASK )
} CTRL_REG1_field_masks;

typedef enum
{
	LIS3DHTR_LOW_POWER ,
	LIS3DHTR_NORMAL ,
	LIS3DHTR_HIGH
}LIS3DHTR_output_resolution_t;

typedef enum
{
	ODR_POWER_DOWN ,
	ODR_1HZ ,
	ODR_10HZ ,
	ODR_25HZ ,
	ODR_50HZ ,
	ODR_100HZ ,
	ODR_200HZ ,
	ODR_400HZ ,
	ODR_1_6KHZ , // available only in low power mode
	ODR_1_344KHZ , // available only in  HR / normal modes
	ODR_5_376KHZ = ODR_1_344KHZ // available only in low power mode
} CTRL_REG1_ODR_mode_t;

// CTRL_REG2 (21h) ----------
#define CTRL_REG2_FIELDS( X ) \
	X( CTRL_REG2_HP_IA1  , 0 , 1 ) \
	X( CTRL_REG2_HP_IA2  , 1 , 1 ) \
	X( CTRL_REG2_HPCLICK , 2 , 1 ) \
	X( CTRL_REG2_FDS     , 3 , 1 ) \
	X( CTRL_REG2_HPCF    , 4 , 2 ) \
	X( CTRL_REG2_HPM     , 6 , 2 )

typedef enum
{
	CTRL_REG2_FIELDS( GEN_POS )
} CTRL_REG2_field_positions;

typedef enum
{
	CTRL_REG2_FIELDS( GEN_MASK )
} CTRL_REG2_field_masks;

typedef enum
{
	// todo: find what each value does
	none
} CTRL_REG2_HP_FILTER_CUTTOF_t;

typedef enum
{
	HP_FILTER_NORMAL_MODE_RESET ,
	HP_FILTER_REFERENCE ,
	HP_FILTER_NORMAL_MODE ,
	HP_FILTER_AUTORESET
} CTRL_REG2_HP_FILTER_mode_t;

// CTRL_REG3 (22h) ----------
#define CTRL_REG3_FIELDS( X ) \
	X( CTRL_REG3_I1_OVERRUN , 1 , 1 ) \
	X( CTRL_REG3_I1_WTM     , 2 , 1 ) \
	X( CTRL_REG3_I1_321DA   , 3 , 1 ) \
	X( CTRL_REG3_I1_ZYXDA   , 4 , 1 ) \
	X( CTRL_REG3_I1_IA2     , 5 , 1 ) \
	X( CTRL_REG3_I1_IA1     , 6 , 1 ) \
	X( CTRL_REG3_I1_CLICK   , 7 , 1 )

typedef enum
{
	CTRL_REG3_FIELDS( GEN_POS )
} CTRL_REG3_field_positions;

typedef enum
{
	CTRL_REG3_FIELDS( GEN_MASK )
} CTRL_REG3_field_masks;

// CTRL_REG4 (23h) ----------
#define CTRL_REG4_FIELDS( X ) \
	X( CTRL_REG4_SIM , 0 , 1 ) \
	X( CTRL_REG4_ST  , 1 , 1 ) \
	X( CTRL_REG4_HR  , 2 , 2 ) \
	X( CTRL_REG4_FS  , 4 , 2 ) \
	X( CTRL_REG4_BLE , 6 , 1 ) \
	X( CTRL_REG4_BDU , 7 , 1 )

typedef enum
{
	CTRL_REG4_FIELDS( GEN_POS )
} CTRL_REG4_field_positions;

typedef enum
{
	CTRL_REG4_FIELDS( GEN_MASK )
} CTRL_REG4_field_masks;

typedef enum
{
	CONTINUOUS_UPDATE ,
	NON_CONTINUOUS_UPDATE // update only after read
} CTRL_REG4_BDU_setting_t;

typedef enum
{
	LITTLE_ENDIAN ,
	BIG_ENDIAN
} CTRL_REG4_BLE_setting_t;

typedef enum
{
	FS_2G ,
	FS_4G ,
	FS_8G ,
	FS_16G ,
	FS_NUM_SETTINGS
} CTRL_REG4_FS_setting_t;

typedef enum
{
	SELF_TEST_DISABLED ,
	SELF_TEST_0 ,
	SELF_TEST_1
} CTRL_REG4_ST_setting_t;

// CTRL_REG5 (24h) ----------
#define CTRL_REG5_FIELDS( X ) \
	X( CTRL_REG5_D4D_INT2 , 0 , 1 ) \
	X( CTRL_REG5_LIR_INT2 , 1 , 1 ) \
	X( CTRL_REG5_D4D_INT1 , 2 , 1 ) \
	X( CTRL_REG5_LIR_INT1 , 3 , 1 ) \
	X( CTRL_REG5_FIFO_EN  , 6 , 1 ) \
	X( CTRL_REG5_BOOT     , 7 , 1 )

typedef enum
{
	CTRL_REG5_FIELDS( GEN_POS )
} CTRL_REG5_field_positions;

typedef enum
{
	CTRL_REG5_FIELDS( GEN_MASK )
} CTRL_REG5_field_masks;

typedef enum
{
	BOOT_NORMAL_MODE ,
	REBOOT_MEM_CONTENT
} CTRL_REG5_BOOT_setting_t;

// CTRL_REG6 (25h) ----------
#define CTRL_REG6_FIELDS( X ) \
	X( CTRL_REG6_INT_POLARITY , 1 , 1 ) \
	X( CTRL_REG6_I2_ACT       , 3 , 1 ) \
	X( CTRL_REG6_I2_BOOT      , 4 , 1 ) \
	X( CTRL_REG6_I2_IA2       , 5 , 1 ) \
	X( CTRL_REG6_I2_IA1       , 6 , 1 ) \
	X( CTRL_REG6_I2_CLICK     , 7 , 1 )

typedef enum
{
	CTRL_REG6_FIELDS( GEN_POS )
} CTRL_REG6_field_positions;

typedef enum
{
	CTRL_REG6_FIELDS( GEN_MASK )
} CTRL_REG6_field_masks;

// REFERENCE (26h) ----------
#define REFERENCE_FIELDS( X ) \
	X( REF , 0 , 8 )

typedef enum
{
	REFERENCE_FIELDS( GEN_POS )
} REFERENCE_field_positions;

typedef enum
{
	REFERENCE_FIELDS( GEN_MASK )
} REFERENCE_field_masks;


// STATUS_REG (27h) ----------
#define STATUS_REG_FIELDS( X ) \
	X( STATUS_REG_XDA   , 0 , 1 ) \
	X( STATUS_REG_YDA   , 1 , 1 ) \
	X( STATUS_REG_ZDA   , 2 , 1 ) \
	X( STATUS_REG_ZYXDA , 3 , 1 ) \
	X( STATUS_REG_XOR   , 4 , 1 ) \
	X( STATUS_REG_YOR   , 5 , 1 ) \
	X( STATUS_REG_ZOR   , 6 , 1 ) \
	X( STATUS_REG_ZYXOR , 7 , 1 )

typedef enum
{
	STATUS_REG_FIELDS( GEN_POS )
} STATUS_REG_field_positions;

typedef enum
{
	STATUS_REG_FIELDS( GEN_MASK )
} STATUS_REG_field_masks;

// FIFO_CTRL_REG (2Eh) ----------
#define FIFO_CTRL_REG_FIELDS( X ) \
	X( FIFO_CTRL_REG_FTH , 0 , 5 ) \
	X( FIFO_CTRL_REG_TR  , 5 , 1 ) \
	X( FIFO_CTRL_REG_FM  , 6 , 2 )

typedef enum
{
	FIFO_CTRL_REG_FIELDS( GEN_POS )
} FIFO_CTRL_REG_field_positions;

typedef enum
{
	FIFO_CTRL_REG_FIELDS( GEN_MASK )
} FIFO_CTRL_REG_field_masks;


typedef enum
{
	BYPASS_MODE ,
	FIFO_MODE ,
	STREAM_MODE ,
	STREAM_TO_FIFO
} FIFO_CTRL_REG_FM_setting_t;

// FIFO_SRC_REG (2Fh) ----------
#define FIFO_SRC_REG_FIELDS( X ) \
	X( FIFO_SRC_REG_FSS      , 0 , 5 ) \
	X( FIFO_SRC_REG_EMPTY     , 5 , 1 ) \
	X( FIFO_SRC_REG_OVRN_FIFO , 6 , 1 ) \
	X( FIFO_SRC_REG_WTM       , 7 , 1 ) \

typedef enum
{
	FIFO_SRC_REG_FIELDS( GEN_POS )
} FIFO_SRC_REG_field_positions;

typedef enum
{
	FIFO_SRC_REG_FIELDS( GEN_MASK )
} FIFO_SRC_REG_field_masks;

// INT1_CFG (30h) & INT2_CFG (34h) ----------
#define INT_CFG_FIELDS( X ) \
	X( INT_CFG_XLIE   , 0 , 1 ) \
	X( INT_CFG_XHIE   , 1 , 1 ) \
	X( INT_CFG_YLIE   , 2 , 1 ) \
	X( INT_CFG_YHIE   , 3 , 1 ) \
	X( INT_CFG_ZLIE   , 4 , 1 ) \
	X( INT_CFG_ZHIE   , 5 , 1 ) \
	X( INT_CFG_AOI_6D , 6 , 2 )

typedef enum
{
	INT_CFG_FIELDS( GEN_POS )
} INT_CFG_field_positions;

typedef enum
{
	INT_CFG_FIELDS( GEN_MASK )
} INT_CFG_field_masks;

typedef enum
{
	OR_COMBINATION ,
	MOVEMENT_RECOGNITION ,
	AND_COMBINATION ,
	POSITION_RECOGNITION
} INT_CFG_AOI_6D_setting_t;

// INT1_SRC (31h) & INT2_SRC (35h) ----------
#define INT_SRC_FIELDS( X ) \
	X( INT_SRC_XL , 0 , 1 ) \
	X( INT_SRC_XH , 1 , 1 ) \
	X( INT_SRC_YL , 2 , 1 ) \
	X( INT_SRC_YH , 3 , 1 ) \
	X( INT_SRC_ZL , 4 , 1 ) \
	X( INT_SRC_ZH , 5 , 1 ) \
	X( INT_SRC_IA , 6 , 1 )

typedef enum
{
	INT_SRC_FIELDS( GEN_POS )
} INT_SRC_field_positions;

typedef enum
{
	INT_SRC_FIELDS( GEN_MASK )
} INT_SRC_field_masks;

// INT1_THS (32h) & INT2_THS (36h) ----------
#define INT_THS_FIELDS( X ) \
	X( INT_THS , 0 , 7 )

typedef enum
{
	INT_THS_FIELDS( GEN_POS )
} INT_THS_field_positions;

typedef enum
{
	INT_THS_FIELDS( GEN_MASK )
} INT_THS_field_masks;

// INT1_DURATION (33h) & INT2_DURATION (37h) ----------
#define INT_DURATION_FIELDS( X ) \
	X( INT_DURATION_D , 0 , 7 )

typedef enum
{
	INT_DURATION_FIELDS( GEN_POS )
} INT_DURATION_field_positions;

typedef enum
{
	INT_DURATION_FIELDS( GEN_MASK )
} INT_DURATION_field_masks;

// CLICK_CFG (38h) ----------
#define CLICK_CFG_FIELDS( X ) \
	X( CLICK_CFG_XS , 0 , 1 ) \
	X( CLICK_CFG_XD , 1 , 1 ) \
	X( CLICK_CFG_YS , 2 , 1 ) \
	X( CLICK_CFG_YD , 3 , 1 ) \
	X( CLICK_CFG_ZS , 4 , 1 ) \
	X( CLICK_CFG_ZD , 5 , 1 )

typedef enum
{
	CLICK_CFG_FIELDS( GEN_POS )
} CLICK_CFG_field_positions;

typedef enum
{
	CLICK_CFG_FIELDS( GEN_MASK )
} CLICK_CFG_field_masks;

// CLICK_SRC (39h) ----------
#define CLICK_SRC_FIELDS( X ) \
	X( CLICK_SRC_X      , 0 , 1 ) \
	X( CLICK_SRC_Y      , 1 , 1 ) \
	X( CLICK_SRC_Z      , 2 , 1 ) \
	X( CLICK_SRC_Sign   , 3 , 1 ) \
	X( CLICK_SRC_SCLICK , 4 , 1 ) \
	X( CLICK_SRC_DCLICK , 5 , 1 ) \
	X( CLICK_SRC_IA     , 6 , 1 )

typedef enum
{
	CLICK_SRC_FIELDS( GEN_POS )
} CLICK_SRC_field_positions;

typedef enum
{
	CLICK_SRC_FIELDS( GEN_MASK )
} CLICK_SRC_field_masks;

typedef enum
{
	POSITIVE ,
	NEGATIVE
} CLICK_SRC_sign_t;

// CLICK_THS (3Ah) ----------
#define CLICK_THS_FIELDS( X ) \
	X( CLICK_THS_THS       , 0 , 7 ) \
	X( CLICK_THS_LIR_Click , 1 , 1 )

typedef enum
{
	CLICK_THS_FIELDS( GEN_POS )
} CLICK_THS_field_positions;

typedef enum
{
	CLICK_THS_FIELDS( GEN_MASK )
} CLICK_THS_field_masks;

// TIME_LIMIT (3Bh) ----------
#define TIME_LIMIT_FIELDS( X ) \
	X( TIME_LIMIT_TLI , 0 , 7 )

typedef enum
{
	TIME_LIMIT_FIELDS( GEN_POS )
} TIME_LIMIT_field_positions;

typedef enum
{
	TIME_LIMIT_FIELDS( GEN_MASK )
} TIME_LIMIT_field_masks;

// TIME_LATENCY (3Ch) ----------
#define TIME_LATENCY_FIELDS( X ) \
	X( TIME_LATENCY_TLA , 0 , 8 )

typedef enum
{
	TIME_LATENCY_FIELDS( GEN_POS )
} TIME_LATENCY_field_positions;

typedef enum
{
	TIME_LATENCY_FIELDS( GEN_MASK )
} TIME_LATENCY_field_masks;

// TIME WINDOW (3Dh) ----------
#define TIME_WINDOW_FIELDS( X ) \
	X( TIME_WINDOW_TW , 0 , 8 )

typedef enum
{
	TIME_WINDOW_FIELDS( GEN_POS )
} TIME_WINDOW_field_positions;

typedef enum
{
	TIME_WINDOW_FIELDS( GEN_MASK )
} TIME_WINDOW_field_masks;

// ACT_THS (3Eh) ----------
#define ACT_THS_FIELDS( X ) \
	X( ACT_THS_ACTH , 0 , 7 )

typedef enum
{
	ACT_THS_FIELDS( GEN_POS )
} ACT_THS_field_positions;

typedef enum
{
	ACT_THS_FIELDS( GEN_MASK )
} ACT_THS_field_masks;

// ACT_DUR (3Fh) ----------
#define ACT_DUR_FIELDS( X ) \
	X( ACT_DUR_ACTD , 0 , 8 )

typedef enum
{
	ACT_DUR_FIELDS( GEN_POS )
} ACT_DUR_field_positions;

typedef enum
{
	ACT_DUR_FIELDS( GEN_MASK )
} ACT_DUR_field_masks;


#endif /* LIS3DHTR_INC_LIS3DHTR_TYPES_H_ */
