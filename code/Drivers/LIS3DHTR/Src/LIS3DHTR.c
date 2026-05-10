/*
 * LIS3DHTR.c
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "LIS3DHTR.h"
#include "i2c.h"
#include "cmsis_os2.h"

/* Variables ---------------------------------------------------------*/
const LIS3DHTR_reg_t LIS3DHTR_memory_map[LIS3DHTR_NUM_REGS] = {
#ifdef DEBUG_LIS3DHTR
#define X( name , address , access ) { #name , address , access } ,
#else
#define X( name , address , access ) { address , access } ,
#endif
#include "LIS3DHTR_mmap.def"
#undef X
};

/* Functions ---------------------------------------------------------*/
void LIS3DHTR_print_mmap()
{
	for( int i = 0 ; i < LIS3DHTR_NUM_REGS ; i++ )
	{
#ifdef DEBUG_LIS3DHTR
		printf( "%2d    %20s    %2x    %d\n" , i , LIS3DHTR_memory_map[i].name , LIS3DHTR_memory_map[i].address ,
		        LIS3DHTR_memory_map[i].access );
#else
		printf( "%2d    %2x    %d\n" , i , LIS3DHTR_memory_map[i].address , LIS3DHTR_memory_map[i].access );
#endif
	}
}

void LIS3DHTR_read_all_regs( const LIS3DHTR_device_t* const device )
{
	for( uint8_t i = 0 ; i < LIS3DHTR_NUM_REGS ; i++ )
	{
		uint8_t data = 0xFF;

		LIS3DHTR_read_reg( device , i , &data );

#ifdef DEBUG_LIS3DHTR
		printf( "%2d    %20s    %x\n" , i , LIS3DHTR_memory_map[i].name , data );
#else
		printf( "%2d    %x\n" , i , data );
#endif
	}
}

void LIS3DHTR_enable_aux_adcs( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	reg_set_field( &data , TEMP_CFG_ADC_EN , LIS3_ENABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
}

void LIS3DHTR_disable_aux_adcs( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	reg_set_field( &data , TEMP_CFG_ADC_EN , LIS3_DISABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
}

void LIS3DHTR_enable_temp_sensor( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	reg_set_field( &data , TEMP_CFG_TEMP_EN , LIS3_ENABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
}

void LIS3DHTR_disable_temp_sensor( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	reg_set_field( &data , TEMP_CFG_TEMP_EN , LIS3_DISABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
}

void LIS3DHTR_enable_BDU( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	reg_set_field( &data , CTRL_REG4_BDU , LIS3_ENABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
}

void LIS3DHTR_disable_BDU( const LIS3DHTR_device_t* const device )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	reg_set_field( &data , CTRL_REG4_BDU , LIS3_DISABLE );
	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
}

void LIS3DHTR_set_ODR( const LIS3DHTR_device_t* const device , CTRL_REG1_ODR_mode_t ODR_MODE )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG1 , &data );
	reg_set_field( &data , CTRL_REG1_ODR , ODR_MODE );
	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG1 , data );
}

void LIS3DHTR_set_scale( const LIS3DHTR_device_t* const device , CTRL_REG4_FS_setting_t scale )
{
	uint8_t data;

	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	reg_set_field( &data , CTRL_REG4_FS , scale );
	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
}

void LIS3DHTR_set_resolution( const LIS3DHTR_device_t* const device , LIS3DHTR_output_resolution_t resolution )
{
	uint8_t reg1;
	uint8_t reg4;

	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG1 , &reg1 );
	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &reg4 );

	switch( resolution )
	{
		case LIS3DHTR_LOW_POWER:
			reg_set_field( &reg1 , CTRL_REG1_LPen , LIS3_ENABLE );
			reg_set_field( &reg4 , CTRL_REG4_HR , LIS3_DISABLE );

			break;
		case LIS3DHTR_NORMAL:
			reg_set_field( &reg1 , CTRL_REG1_LPen , LIS3_DISABLE );
			reg_set_field( &reg4 , CTRL_REG4_HR , LIS3_DISABLE );
			break;
		case LIS3DHTR_HIGH:
			reg_set_field( &reg1 , CTRL_REG1_LPen , LIS3_DISABLE );
			reg_set_field( &reg4 , CTRL_REG4_HR , LIS3_ENABLE );
			break;
		default:
			return; // erroneous setting
	}

	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG1 , reg1 );
	LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , reg4 );
}

float LIS3DHTR_get_temp( const LIS3DHTR_device_t* const device )
{
	uint8_t ADC3_H;
	uint8_t ADC3_L;
	int16_t temp_raw;

	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_ADC3_H , &ADC3_H );
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_ADC3_L , &ADC3_L );
	temp_raw = (int16_t) ( ( ADC3_H << LIS3DHTR_REG_SIZE_BITS ) | ADC3_L );

#define calibration_temp 2500
#define q8_format_normalizer 256

//	return calibration_temp + ( (int32_t) temp_raw * 100 ) / q8_format_normalizer;
	return 25 + (float) temp_raw / 256;
}

void LIS3DHTR_get_acceleration( const LIS3DHTR_device_t* const device , float* x , float* y , float* z )
{
	uint8_t x_l;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_X_L , &x_l );
	uint8_t x_h;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_X_H , &x_h );
	int16_t x_raw = (int16_t) ( ( x_h << LIS3DHTR_REG_SIZE_BITS ) | x_l );

	uint8_t y_l;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Y_L , &y_l );
	uint8_t y_h;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Y_H , &y_h );
	int16_t y_raw = (int16_t) ( ( y_h << LIS3DHTR_REG_SIZE_BITS ) | y_l );

	uint8_t z_l;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Z_L , &z_l );
	uint8_t z_h;
	LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Z_H , &z_h );
	int16_t z_raw = (int16_t) ( ( z_h << LIS3DHTR_REG_SIZE_BITS ) | z_l );

	uint8_t data;
	LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	uint8_t accuracy = reg_get_field( data , CTRL_REG4_FS );

	static const float accuracy_range[FS_NUM_SETTINGS] = { 16000.0f , 7282.0f , 3968.0f , 1280.0f };

	*x = ( (float) x_raw ) / accuracy_range[accuracy];
	*y = ( (float) y_raw ) / accuracy_range[accuracy];
	*z = ( (float) z_raw ) / accuracy_range[accuracy];

	return;
}
