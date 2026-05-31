/*
 * mqtt_client.h
 */

#ifndef MQTT_CLIENT_MQTT_CLIENT_H_
#define MQTT_CLIENT_MQTT_CLIENT_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include <err.h>
#include "cmsis_os2.h"
#include "mqtt.h"

/* Defines ---------------------------------------------------------*/
#define MQTT_UNKOWN_TOPIC -1

/* Macros ---------------------------------------------------------*/

/* Shared Types ---------------------------------------------------------*/
typedef struct _mqtt_sub_topic_t
{
	char name[32];
	osMessageQueueId_t os_queue_id;
	bool valid;
}mqtt_sub_topic_t;


typedef struct _mqtt_data_t
{
	mqtt_sub_topic_t sub_topics[MQTT_MAX_TOPICS];
	osMemoryPoolId_t os_memory_pool;
	mqtt_client_t* client;
	uint8_t num_topics;
	bool connected;
}mqtt_data_t;



// This struct is used to move mqtt messages to the freertos tasks
typedef struct _mqtt_os_message_t
{
	uint8_t data[MQTT_PAYLOAD_MAX_SIZE];
	uint32_t len;
}mqtt_os_message_t;


/* Exported Functions ---------------------------------------------------------*/
void mqtt_init();
void mqtt_test();
err_t mqtt_sub_topic( const char* const topic_name , const osMessageQueueId_t os_queue_id );

/* Exported Variables ---------------------------------------------------------*/
extern mqtt_data_t mqtt_data;

#endif /* MQTT_CLIENT_MQTT_CLIENT_H_ */
