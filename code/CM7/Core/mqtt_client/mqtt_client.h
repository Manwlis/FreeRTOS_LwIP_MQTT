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
#define mqtt_get_connection_status() (*mqtt_connection_status)

/* Shared Types ---------------------------------------------------------*/
typedef int32_t sub_topic_id_t;

// This struct is used to move mqtt messages to the freertos tasks
typedef struct _mqtt_os_message_t
{
	uint8_t data[MQTT_PAYLOAD_MAX_SIZE];
	uint32_t len;
	sub_topic_id_t topic_id;
}mqtt_os_message_t;

/* Exported Functions ---------------------------------------------------------*/
err_t mqtt_init();
void mqtt_test();
err_t mqtt_sub_topic( const char* const topic_name , const osMessageQueueId_t os_queue_id , sub_topic_id_t* const topic_id );
err_t mqtt_unsub_topic( sub_topic_id_t* const topic_id );
err_t mqtt_publish_wrapper( const char *topic , const void *payload , u16_t payload_length , u8_t qos , u8_t retain , mqtt_request_cb_t cb , void *arg );
osStatus_t mqtt_free_message( const mqtt_os_message_t* const message );

/* Exported Variables ---------------------------------------------------------*/
extern bool* const mqtt_connection_status;

/* Inline Functions ---------------------------------------------------------*/
inline bool compare_mqtt_payload( const mqtt_os_message_t* const restrict message , const char* const restrict string , bool match_size )
{
	if( message == NULL || string == NULL )
		return false;

	if( match_size == true )
		return ( message->len == strlen(string) ) && ( strncmp( (char*)message->data , string , message->len ) == 0 );
	else
		return strncmp( (char*)message->data , string , strlen(string) ) == 0;
}


#endif /* MQTT_CLIENT_MQTT_CLIENT_H_ */
