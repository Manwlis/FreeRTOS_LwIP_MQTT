/*
 * lwl.h
 *
 *  Created on: 18 Μαΐ 2026
 *      Author: MSI
 */

#ifndef LWL_LWL_H_
#define LWL_LWL_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Macros ------------------------------------------------------------------*/
#define LIS3DHTR_LWL_ID 'L'
#define LIS3DHTR_WRITE_LWL_ID "W"
#define LIS3DHTR_READ_LWL_ID "R"

#define ADC_LWL_ID 'A'
#define ADC_REDUCED_DIV_LWL_ID "R"
#define ADC_INT_LWL_ID "I"
#define ADC_FLOAT_LWL_ID "F"

#define TEST_LWL_ID 'T'
#define TEST_TEST_LWL_ID "TEST"

#define I2C_LWL_ID 'I'
#define I2C_TX_IT_LWL_ID "TX"
#define I2C_RX_IT_LWL_ID "RX"
#define I2C_ER_IT_LWL_ID "ER"

#define SPI_LWL_ID 'S'
#define SPI_RX_IT_LWL_ID "RX"
#define SPI_ER_IT_LWL_ID "TX"

/* Exported functions prototypes ---------------------------------------------*/
void lwl_init();
void lwl_enter_record( uint8_t module_id , char functionality_id[] , const char* fmt , ... );
void dump_log();

#endif /* LWL_LWL_H_ */
