/*
 * pmodals_types.h
 */

#ifndef PMODALS_INC_PMODALS_TYPES_H_
#define PMODALS_INC_PMODALS_TYPES_H_

#define NO_OS		0
#define FREE_RTOS	1
#define PMODALS_OS	FREE_RTOS

#define SPI_MEM_IT_FLAG		0x0008U
#define SPI_ERR_IT_FLAG		0x0010U

// overload the definitions in settings.h
#if __has_include("settings.h")
#include "settings.h"
#endif

typedef struct
{
	const void* const phy_handle;
	const float Vcc;
	const float Rload;
	const float adc_max_value;
	const float TEMT6000_transfer_function;
}pmodals_device_t;


#endif /* PMODALS_INC_PMODALS_TYPES_H_ */
