/*
 * LIS3DHTR.h
 *
 *  Created on: 3 Μαΐ 2026
 *      Author: MSI
 */

#ifndef INC_LIS3DHTR_H_
#define INC_LIS3DHTR_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "LIS3DHTR_types.h"
#ifdef DEBUG_LIS3DHTR
#include <stdio.h>
#endif

/* Macros ------------------------------------------------------------------*/
#define LIS3DHTR_REG_SIZE_BYTES 1
#define LIS3DHTR_REG_SIZE_BITS ( LIS3DHTR_REG_SIZE_BYTES * 8 )

/* Exported variables ---------------------------------------------*/
extern const LIS3DHTR_reg_t LIS3DHTR_memory_map[LIS3DHTR_NUM_REGS];

/* Exported functions prototypes ---------------------------------------------*/
void init_LIS3DHTR_device( const LIS3DHTR_device_t* device , I2C_HandleTypeDef* i2c_handle , uint8_t i2c_address );
void LIS3DHTR_print_mmap();
void LIS3DHTR_read_all_regs( const LIS3DHTR_device_t* const device );
void LIS3DHTR_enable_aux_adcs( const LIS3DHTR_device_t* const device );
void LIS3DHTR_disable_aux_adcs( const LIS3DHTR_device_t* const device );
void LIS3DHTR_enable_temp_sensor( const LIS3DHTR_device_t* const device );
void LIS3DHTR_disable_temp_sensor( const LIS3DHTR_device_t* const device );
void LIS3DHTR_enable_BDU( const LIS3DHTR_device_t* const device );
void LIS3DHTR_disable_BDU( const LIS3DHTR_device_t* const device );
void LIS3DHTR_set_ODR( const LIS3DHTR_device_t* const device , CTRL_REG1_ODR_mode_t ODR_MODE );
void LIS3DHTR_get_temp( const LIS3DHTR_device_t* const device );
void LIS3DHTR_get_acceleration( const LIS3DHTR_device_t* const device , float* x , float* y , float* z );
void LIS3DHTR_set_resolution( const LIS3DHTR_device_t* const device , LIS3DHTR_output_resolution_t resolution );

/* Inline functions ---------------------------------------------*/

#define reg_get_field( reg , field ) ( _reg_get_field( reg , field##_mask , field##_pos ) )
inline uint8_t _reg_get_field( uint8_t reg , uint8_t field_mask , uint8_t field_pos )
{
	return ( reg & field_mask ) >> field_pos;
}

#define reg_set_field( reg , field , value )( _reg_set_field ( reg , field##_pos , value ) )
inline void _reg_set_field( uint8_t* const reg , uint8_t field_pos , uint8_t value )
{
	*reg |= value << field_pos;
}

inline HAL_StatusTypeDef LIS3DHTR_read_reg( const LIS3DHTR_device_t* const device , uint8_t reg_address , uint8_t* const reg_value )
{
#ifdef DEBUG_LIS3DHTR
	printf("I2C mem read  @ %s (%x), %-15s (%2x)\n" , device->name , device->i2c_address , device->memory_map[reg_address].name , reg_address );
#endif

	if( ( device->memory_map[reg_address].access != REG_R ) && ( device->memory_map[reg_address].access != REG_RW ) )
		return HAL_ERROR;

	return HAL_I2C_Mem_Read( (I2C_HandleTypeDef*) device->i2c_handle , device->i2c_address << 1 , device->memory_map[reg_address].address ,
	                         LIS3DHTR_REG_SIZE_BYTES , reg_value , sizeof( *reg_value ) , HAL_MAX_DELAY );
	// if I try to read a region larger than a register, it returns REG_ADRESS | REG_ADRESS | ...
	// problem with the API, how I use it, the device?
}

inline HAL_StatusTypeDef LIS3DHTR_write_reg( const LIS3DHTR_device_t* const device , uint8_t reg_address , uint8_t reg_value )
{
#ifdef DEBUG_LIS3DHTR
	printf("I2C mem write @ %s (%x), %-15s (%2x) , value %x\n" , device->name , device->i2c_address , device->memory_map[reg_address].name , reg_address , reg_value );
#endif

	if( ( device->memory_map[reg_address].access != REG_W ) && ( device->memory_map[reg_address].access != REG_RW ) )
		return HAL_ERROR;

	return HAL_I2C_Mem_Write( (I2C_HandleTypeDef*) device->i2c_handle , device->i2c_address << 1 , device->memory_map[reg_address].address ,
	                          LIS3DHTR_REG_SIZE_BYTES , &reg_value , sizeof( reg_value ) , HAL_MAX_DELAY );
}

inline LIS3DHTR_device_t LIS3DHTR_create_handle( const I2C_HandleTypeDef* const i2c_handle , const uint8_t i2c_address )
{
	return (LIS3DHTR_device_t ) {
#ifdef DEBUG_LIS3DHTR
			.name = "LIS3DHTR",
#endif
			.memory_map = LIS3DHTR_memory_map ,
			.i2c_handle = i2c_handle ,
			.i2c_address = i2c_address
		};
}
#endif /* INC_LIS3DHTR_H_ */
