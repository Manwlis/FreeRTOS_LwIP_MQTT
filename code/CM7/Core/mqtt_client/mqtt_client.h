/*
 * mqtt_client.h
 */

#ifndef MQTT_CLIENT_MQTT_CLIENT_H_
#define MQTT_CLIENT_MQTT_CLIENT_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include "settings.h"
#include "cmsis_os2.h"
#include "mqtt.h"
#include "tcpip.h" // LOCK_TCPIP_CORE() & UNLOCK_TCPIP_CORE()

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

/* Inline Functions ---------------------------------------------------------*/
inline bool compare_mqtt_payload( const mqtt_os_message_t* const message , const char* const string )
{
	if( message == NULL || string == NULL )
		return false;

	return ( message->len == strlen(string) ) && ( strncmp( (char*)message->data , string , message->len ) == 0 );
}


inline err_t mqtt_publish_wrapper( mqtt_client_t *client , const char *topic , const void *payload , u16_t payload_length , u8_t qos , u8_t retain ,
                                  mqtt_request_cb_t cb , void *arg )
{
	LOCK_TCPIP_CORE();
	err_t rv = mqtt_publish( client , topic , payload , payload_length , qos , retain , cb , arg );
	UNLOCK_TCPIP_CORE();

	return rv;
}

/* Exported Variables ---------------------------------------------------------*/
extern mqtt_data_t mqtt_data;

#endif /* MQTT_CLIENT_MQTT_CLIENT_H_ */
