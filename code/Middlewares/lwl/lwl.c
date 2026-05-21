/*
 * lwl.c
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "lwl.h"
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "stm32h7xx_nucleo.h"
#include "stm32h7xx_hal.h"

/* Macros ------------------------------------------------------------------*/
#define LWL_BUFFER_SIZE 2048	// in bytes
#define IS_POWER_OF_2( x ) ( ( x ) > 0 && ( ( ( x ) & ( ( x ) - 1 ) ) == 0 ) )
_Static_assert( IS_POWER_OF_2( LWL_BUFFER_SIZE ) , "LWL_BUFFER_SIZE must be a power of 2" );

/* Typedefs -----------------------------------------------------------*/
typedef struct _lwl_data_t
{
	size_t next_entry_index;
	uint8_t buffer[LWL_BUFFER_SIZE];
} lwl_data_t;

typedef struct _lwl_driver_t
{
	lwl_data_t* data;
	bool is_initialized;
}lwl_driver_t;

/* Variables ---------------------------------------------------------*/
static lwl_data_t lwl_data; // TODO: maybe should not directly access lwl_data, but go through lwl_driver
static lwl_driver_t lwl_driver = { .data = &lwl_data , .is_initialized = false };
/* Functions ---------------------------------------------------------*/
void lwl_init()
{
	lwl_data.next_entry_index = 0;
	memset( lwl_data.buffer , 0 , LWL_BUFFER_SIZE * sizeof( uint8_t ) );
	lwl_driver.is_initialized = true;
}

void lwl_enter_record( uint8_t module_id , uint8_t functionality_id , const char* fmt , ... )
{
	if( lwl_driver.is_initialized == false )
		return;

	lwl_data.buffer[lwl_data.next_entry_index] = module_id;
	lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
//	If buffer size is not power of 2 then
//  lwl_data.next_entry_index = (lwl_data.next_entry_index + 1) % LWL_BUFFER_SIZE;

	lwl_data.buffer[lwl_data.next_entry_index] = functionality_id;
	lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );

	va_list args;

	for( va_start( args , fmt ) ; *fmt != '\0' ; ++fmt )
	{
		switch( *fmt )
		{
			case 'd':
			{
				uint32_t temp = (uint32_t) va_arg( args , int );

				for( size_t i = 0 ; i < sizeof(int) ; i++ )
				{
					lwl_data.buffer[lwl_data.next_entry_index] = (uint8_t) temp;
					temp >>= 8;

					lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
				}
				break;
			}
			case 'u':
			{
				uint32_t temp = (uint32_t) va_arg( args , unsigned int );

				for( size_t i = 0 ; i < sizeof(unsigned int) ; i++ )
				{
					lwl_data.buffer[lwl_data.next_entry_index] = (uint8_t) temp;
					temp >>= 8;

					lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
				}
				break;
			}
			case 'c':
			{
				lwl_data.buffer[lwl_data.next_entry_index] = (uint8_t) va_arg( args , int );
				lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
				break;
			}
			case 's':
			case 'h':
			{
				uint16_t temp = (uint16_t) va_arg( args , unsigned int );

				for( size_t i = 0 ; i < sizeof(uint16_t) ; i++ )
				{
					lwl_data.buffer[lwl_data.next_entry_index] = (uint8_t) temp;
					temp >>= 8;

					lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
				}
				break;
			}
			case 'f':
			{
				union
				{
					float f;
					uint8_t b[sizeof(float)];
				} u;
				u.f = (float)va_arg( args , double );
				for( size_t i = 0 ; i < sizeof(float) ; i++ )
				{
					lwl_data.buffer[lwl_data.next_entry_index] = u.b[i];
					lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
				}
				break;
			}
			default:
				assert( 0 );
		}
	}
}

// this doesnt work, because uart is set with 7 data bits. Use debugger for now instead
void dump_log()
{
	static COM_TypeDef COM_ActiveLogPort = COM1;
	uint32_t index = (uint32_t)(lwl_data.next_entry_index);
	HAL_UART_Transmit(&hcom_uart [COM_ActiveLogPort], (uint8_t *) &index , sizeof(index) , COM_POLL_TIMEOUT );
	HAL_UART_Transmit(&hcom_uart [COM_ActiveLogPort], lwl_data.buffer , LWL_BUFFER_SIZE , COM_POLL_TIMEOUT );
}
