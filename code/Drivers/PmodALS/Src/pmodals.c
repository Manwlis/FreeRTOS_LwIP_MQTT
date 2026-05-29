/*
 * pmodals.c
 */
#include "pmodals.h"
#include "cmsis_os2.h"

pmodals_device_t pmodals_create_handle( const void* const phy_handle , float Vcc , float Rload )
{
	return (pmodals_device_t ){
		.phy_handle = phy_handle ,
		.Vcc = Vcc ,
		.Rload = Rload ,
		.adc_max_value = 256.0f ,
		.TEMT6000_transfer_function = 2 * 1000000 // lux ≈ 2 × Iphoto[µA]
	};
}


HAL_StatusTypeDef pmodals_get_lux( const pmodals_device_t* const device , float* const lux )
{
	uint8_t data;
#if PMODALS_OS == NO_OS
	HAL_StatusTypeDef rv = HAL_SPI_Receive( (SPI_HandleTypeDef*) device->phy_handle , (uint8_t *)&data , 1 , HAL_MAX_DELAY );
#else
	HAL_StatusTypeDef rv = HAL_SPI_Receive_IT( (SPI_HandleTypeDef*) device->phy_handle , (uint8_t *)&data , 1 );
#endif

	if( rv != HAL_OK )
		return rv;

#if PMODALS_OS == FREE_RTOS
	osThreadFlagsWait( SPI_MEM_IT_FLAG , osFlagsWaitAny , osWaitForever );
#endif

	*lux = ( (float)data * device->Vcc / device->adc_max_value / device->Rload ) * device->TEMT6000_transfer_function;

	return HAL_OK;
}
