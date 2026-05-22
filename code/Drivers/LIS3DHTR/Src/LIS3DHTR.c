/*
 * LIS3DHTR.c
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "LIS3DHTR.h"

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


/**
 * @brief	Prints the current value of all registers of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 */
void LIS3DHTR_read_all_regs( const LIS3DHTR_device_t* const device )
{
	for( uint8_t i = 0 ; i < LIS3DHTR_NUM_REGS ; i++ )
	{
		uint8_t data = 0xFF;

		HAL_StatusTypeDef rv = LIS3DHTR_read_reg( device , i , &data );
		if( rv != HAL_OK )
		{
			printf( "Failed reading register: %2d\n" , i );
			return;
		}

#ifdef DEBUG_LIS3DHTR
		printf( "%2d    %20s    %x\n" , i , LIS3DHTR_memory_map[i].name , data );
#else
		printf( "%2d    %x\n" , i , data );
#endif
	}
}

/**
 * @brief	Enables the auxiliary ADCs of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_enable_aux_adcs( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , TEMP_CFG_ADC_EN , LIS3_ENABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Disables the auxiliary ADCs of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_disable_aux_adcs( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , TEMP_CFG_ADC_EN , LIS3_DISABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Disables the temperature sensor of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_enable_temp_sensor( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , TEMP_CFG_TEMP_EN , LIS3_ENABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Disables the temperature sensor of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_disable_temp_sensor( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_TEMP_CFG_REG , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , TEMP_CFG_TEMP_EN , LIS3_DISABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_TEMP_CFG_REG , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Enables the Block Data Update of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_enable_BDU( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , CTRL_REG4_BDU , LIS3_ENABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Disables the Block Data Update of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_disable_BDU( const LIS3DHTR_device_t* const device )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , CTRL_REG4_BDU , LIS3_DISABLE );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Set the Output Data Rate of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @param	odr		Requested Output Data Rate
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_set_ODR( const LIS3DHTR_device_t* const device , CTRL_REG1_ODR_mode_t odr )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG1 , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , CTRL_REG1_ODR , odr );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG1 , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Set the scale of a LIS3DHTR device
 * @param	device		Handler of the LIS3DHTR device
 * @param	resolution	Requested scale
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_set_scale( const LIS3DHTR_device_t* const device , CTRL_REG4_FS_setting_t scale )
{
	uint8_t data;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	LIS3DHTR_API_CHECK( rv );

	reg_set_field( &data , CTRL_REG4_FS , scale );

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , data );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}

/**
 * @brief	Set the output resolution of a LIS3DHTR device
 * @param	device		Handler of the LIS3DHTR device
 * @param	resolution	Requested resolution
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_set_resolution( const LIS3DHTR_device_t* const device , LIS3DHTR_output_resolution_t resolution )
{
	uint8_t reg1;
	uint8_t reg4;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG1 , &reg1 );
	LIS3DHTR_API_CHECK( rv );
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &reg4 );
	LIS3DHTR_API_CHECK( rv );

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
			return HAL_ERROR; // erroneous setting
	}

	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG1 , reg1 );
	LIS3DHTR_API_CHECK( rv );
	rv = LIS3DHTR_write_reg( device , LIS3DHTR_CTRL_REG4 , reg4 );
	LIS3DHTR_API_CHECK( rv );

	return HAL_OK;
}


/**
 * @brief	Read the temperature of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @param	Temperature of the device
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_get_temp( const LIS3DHTR_device_t* const device , float* temp )
{
	uint8_t ADC3_H;
	uint8_t ADC3_L;
	int16_t temp_raw;
	HAL_StatusTypeDef rv;

	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_ADC3_H , &ADC3_H );
	LIS3DHTR_API_CHECK( rv );
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_ADC3_L , &ADC3_L );
	LIS3DHTR_API_CHECK( rv );

	temp_raw = (int16_t) ( ( ADC3_H << LIS3DHTR_REG_SIZE_BITS ) | ADC3_L );

#define calibration_temp 25.0f
#define q8_format_normalizer 256.0f

	*temp = calibration_temp + (float) temp_raw / q8_format_normalizer;

	return HAL_OK;
}


/**
 * @brief	Read the acceleration of a LIS3DHTR device
 * @param	device	Handler of the LIS3DHTR device
 * @param	x		X axis acceleration
 * @param	y		Y axis acceleration
 * @param	z		Z axis acceleration
 * @retval	HAL status
 */
HAL_StatusTypeDef LIS3DHTR_get_acceleration( const LIS3DHTR_device_t* const device , float* x , float* y , float* z )
{
	HAL_StatusTypeDef rv;

	uint8_t x_l;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_X_L , &x_l );
	LIS3DHTR_API_CHECK( rv );
	uint8_t x_h;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_X_H , &x_h );
	LIS3DHTR_API_CHECK( rv );
	int16_t x_raw = (int16_t) ( ( x_h << LIS3DHTR_REG_SIZE_BITS ) | x_l );

	uint8_t y_l;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Y_L , &y_l );
	LIS3DHTR_API_CHECK( rv );
	uint8_t y_h;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Y_H , &y_h );
	LIS3DHTR_API_CHECK( rv );
	int16_t y_raw = (int16_t) ( ( y_h << LIS3DHTR_REG_SIZE_BITS ) | y_l );

	uint8_t z_l;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Z_L , &z_l );
	LIS3DHTR_API_CHECK( rv );
	uint8_t z_h;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_OUT_Z_H , &z_h );
	LIS3DHTR_API_CHECK( rv );
	int16_t z_raw = (int16_t) ( ( z_h << LIS3DHTR_REG_SIZE_BITS ) | z_l );

	uint8_t data;
	rv = LIS3DHTR_read_reg( device , LIS3DHTR_CTRL_REG4 , &data );
	LIS3DHTR_API_CHECK( rv );
	uint8_t accuracy = reg_get_field( data , CTRL_REG4_FS );

	static const float accuracy_range[FS_NUM_SETTINGS] = { 16000.0f , 7282.0f , 3968.0f , 1280.0f };

	*x = ( (float) x_raw ) / accuracy_range[accuracy];
	*y = ( (float) y_raw ) / accuracy_range[accuracy];
	*z = ( (float) z_raw ) / accuracy_range[accuracy];

	return HAL_OK;
}
