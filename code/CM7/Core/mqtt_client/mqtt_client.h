/*
 * mqtt_client.h
 */

#ifndef MQTT_CLIENT_MQTT_CLIENT_H_
#define MQTT_CLIENT_MQTT_CLIENT_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "settings.h"

/* Macros ---------------------------------------------------------*/
#define MQTT_SUB_TOPICS( X ) \
	X( SENSOR_LIS3	, "sensor/lis3" ) \
	X( SENSOR_ALS	, "sensor/als" ) \
	X( SENSOR_TEMP	, "sensor/temp" ) \
	X( MODULE_LWL	, "module/lwl" ) \
	X( LWIP_TEST	, "lwip/eth_test" )


#define GEN_ENUM( id , topic )  MQTT_TOPIC_##id ,
#define GEN_ARRAY( id , topic ) [MQTT_TOPIC_##id].name = topic ,

/* Shared Types ---------------------------------------------------------*/
typedef enum
{
	MQTT_UNKOWN_TOPIC = -1 ,
	MQTT_SUB_TOPICS( GEN_ENUM )
	NUM_MQTT_SUB_TOPICS
} mqtt_sub_topic_idx_t;

typedef struct _mqtt_sub_topics_t
{
	const char* const name;
	osMessageQueueId_t os_queue_id;

}mqtt_sub_topics_t;

// This struct is used to move mqtt messages to the freertos tasks
typedef struct _mqtt_os_message_t
{
	uint8_t data[MQTT_OS_POOL_ELEMENT_SIZE];
	uint32_t len;
}mqtt_os_message_t;


/* Exported Functions ---------------------------------------------------------*/
void mqtt_init();
void mqtt_test();

/* Exported Variables ---------------------------------------------------------*/
extern mqtt_sub_topics_t mqtt_sub_topics[NUM_MQTT_SUB_TOPICS];
extern osMemoryPoolId_t mqtt_os_message_pool;

#endif /* MQTT_CLIENT_MQTT_CLIENT_H_ */
