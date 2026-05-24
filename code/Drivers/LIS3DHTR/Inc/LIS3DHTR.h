/*
 * LIS3DHTR.h
 *
 *  Created on: 3 Μαΐ 2026
 *      Author: MSI
 */

#ifndef INC_LIS3DHTR_H_
#define INC_LIS3DHTR_H_

/* Includes ------------------------------------------------------------------*/
#include "LIS3DHTR_types.h"
#ifdef DEBUG_LIS3DHTR
#include <stdio.h>
#endif

#if LIS3DHTR_IO_MODE == IO_MODE_NON_BLOCKING
#include "cmsis_os2.h"
#endif


#if __has_include("lwl.h")
#include "lwl.h"
#else
#define lwl_enter_record(...)
#endif

/* Macros ------------------------------------------------------------------*/
#define LIS3DHTR_REG_SIZE_BYTES 1
#define LIS3DHTR_REG_SIZE_BITS ( LIS3DHTR_REG_SIZE_BYTES * 8 )

#define LIS3DHTR_API_CHECK(rv) if( rv != HAL_OK ){ return rv; }

/* Exported variables ---------------------------------------------*/
extern const LIS3DHTR_reg_t LIS3DHTR_memory_map[LIS3DHTR_NUM_REGS];

/* Exported functions prototypes ---------------------------------------------*/
LIS3DHTR_device_t LIS3DHTR_create_handle( const void* const phy_handle , const uint8_t phy_address );
// low level API
HAL_StatusTypeDef LIS3DHTR_read_reg( const LIS3DHTR_device_t* const device , uint8_t reg_index , uint8_t* const reg_value );
HAL_StatusTypeDef LIS3DHTR_write_reg( const LIS3DHTR_device_t* const device , uint8_t reg_index , uint8_t reg_value );
// high level API
void LIS3DHTR_read_all_regs( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_enable_aux_adcs( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_disable_aux_adcs( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_enable_temp_sensor( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_disable_temp_sensor( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_enable_BDU( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_disable_BDU( const LIS3DHTR_device_t* const device );
HAL_StatusTypeDef LIS3DHTR_set_ODR( const LIS3DHTR_device_t* const device , CTRL_REG1_ODR_mode_t ODR_MODE );
HAL_StatusTypeDef LIS3DHTR_get_temp( const LIS3DHTR_device_t* const device , float* temp );
HAL_StatusTypeDef LIS3DHTR_get_acceleration( const LIS3DHTR_device_t* const device , float* x , float* y , float* z );
HAL_StatusTypeDef LIS3DHTR_set_resolution( const LIS3DHTR_device_t* const device , LIS3DHTR_output_resolution_t resolution );

/* Inline functions ---------------------------------------------*/
// Bit manipulation helper functions
/**
 * @brief	Reads a field of a register
 * @param	reg		Register to be read
 * @param	field	Field to be read
 * @retval	Value of the field
 */
#define reg_get_field( reg , field ) ( _reg_get_field( reg , field##_mask , field##_pos ) )
inline uint8_t _reg_get_field( uint8_t reg , uint8_t field_mask , uint8_t field_pos )
{
	return ( reg & field_mask ) >> field_pos;
}

/**
 * @brief	Sets a field of a register
 * @param	reg		Register to be written
 * @param	field	Field to be written
 */
#define reg_set_field( reg , field , value )( _reg_set_field ( reg , field##_pos , value ) )
inline void _reg_set_field( uint8_t* const reg , uint8_t field_pos , uint8_t value )
{
	*reg |= value << field_pos;
}

#endif /* INC_LIS3DHTR_H_ */
