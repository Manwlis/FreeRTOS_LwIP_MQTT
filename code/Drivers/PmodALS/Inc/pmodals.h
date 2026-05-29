/*
 * pmodals.h
 */

#ifndef INC_PMODALS_H_
#define INC_PMODALS_H_

#include "stm32h7xx_hal.h"
#include "pmodals_types.h"

#if __has_include("lwl.h")
#include "lwl.h"
#else
#define lwl_enter_record(...)
#endif

pmodals_device_t pmodals_create_handle( const void* const phy_handle , float Vcc , float Rload );
HAL_StatusTypeDef pmodals_get_lux( const pmodals_device_t* const device , float* const lux );


#endif /* INC_PMODALS_H_ */
