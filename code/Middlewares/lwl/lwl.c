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

#include "FreeRTOS.h"
#include "task.h" // taskENTER_CRITICAL()

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
// In an real world application, this should be stored in external Flash or FRAM to survive black out events.
static lwl_data_t lwl_data; // TODO: maybe should not directly access lwl_data, but go through lwl_driver
static lwl_driver_t lwl_driver = { .data = &lwl_data , .is_initialized = false };
/* Functions ---------------------------------------------------------*/
void lwl_init()
{
	lwl_data.next_entry_index = 0;
	memset( lwl_data.buffer , 0 , LWL_BUFFER_SIZE * sizeof( uint8_t ) );
	lwl_driver.is_initialized = true;
}

void lwl_enter_record( uint8_t module_id , char functionality_id[] , const char* fmt , ... )
{
	if( lwl_driver.is_initialized == false )
		return;

	lwl_data.buffer[lwl_data.next_entry_index] = module_id;
	lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
//	If buffer size is not power of 2 then
//  lwl_data.next_entry_index = (lwl_data.next_entry_index + 1) % LWL_BUFFER_SIZE;

	for( int i = 0 ; functionality_id[i] != '\0' ; i++)
	{
		lwl_data.buffer[lwl_data.next_entry_index] = (uint8_t)(functionality_id[i]);
		lwl_data.next_entry_index = ( lwl_data.next_entry_index + 1 ) & ( LWL_BUFFER_SIZE - 1 );
	}

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


#if __has_include("mqtt_client.h")
#include "mqtt_client.h"
#include "tcpip.h"
void dump_log_mqtt()
{
	// take a snapshot of the lwl data & metadata. Make sure that nothing change while coping
	taskENTER_CRITICAL( );

	uint8_t metadata[ sizeof(lwl_data.next_entry_index) + sizeof(uint32_t) ];
	memcpy( &(metadata[0]) , &(lwl_data.next_entry_index) , sizeof(lwl_data.next_entry_index) );
	memcpy( &(metadata[sizeof(lwl_data.next_entry_index)]) , &(uint32_t){LWL_BUFFER_SIZE} , sizeof(uint32_t) );
	static uint8_t data[LWL_BUFFER_SIZE];
	memcpy( data , lwl_driver.data , sizeof(data) );

	taskEXIT_CRITICAL( );

	// send the snapshot through mqtt
	mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_LWL_INDEX_ID , metadata , sizeof(metadata) , 1 , 0 , NULL , NULL );

	int32_t current_sent_size = 0;
	for( int32_t remaining_data = LWL_BUFFER_SIZE ; remaining_data > 0 ; remaining_data -= current_sent_size )
	{
		const int32_t payload_max_size = MQTT_OUTPUT_RINGBUF_SIZE - sizeof(MQTT_PUB_LWL_DATA_ID) - 8; // dont know why -8. According to mqtt, largest outgoing publish message = topic+payloads
		current_sent_size = ( remaining_data > payload_max_size ) ? payload_max_size : remaining_data ;

		osDelay(5); // mqtt looses data when sending a burst after idling. This small delay seems to fix it.
		mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_LWL_DATA_ID , &(lwl_data.buffer[LWL_BUFFER_SIZE - remaining_data]) , current_sent_size , 1 , 0 , NULL , NULL );
	}
}
#endif
