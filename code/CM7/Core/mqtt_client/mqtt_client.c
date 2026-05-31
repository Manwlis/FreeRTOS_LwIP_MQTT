/*
 * mqctt.c
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "lwip.h"
#include "mqtt.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h" // taskENTER_CRITICAL()

#include "mqtt_client.h"
#include "lwl.h"

/* Variables ---------------------------------------------------------*/
// memory pool for the os messages
mqtt_data_t mqtt_data = {0};

static volatile int32_t mqtt_sub_topic_idx = MQTT_UNKOWN_TOPIC;

/* Functions ---------------------------------------------------------*/
/**
 * @brief
 * @param
 * @param
 * @param
 */
static void mqtt_incoming_publish_cb( void* arg , const char* topic , u32_t tot_len )
{
	UNUSED(arg);

	mqtt_sub_topic_idx = MQTT_UNKOWN_TOPIC;

	for( uint8_t i = 0 ; i < MQTT_MAX_TOPICS ; i++ )
		if( mqtt_data.sub_topics[i].valid )
			if( strcmp( topic , mqtt_data.sub_topics[i].name ) == 0 )
			{
				mqtt_sub_topic_idx = i;
				break;
			}

	lwl_enter_record( MQTT_LWL_ID , MQTT_IN_PUB_CB_LWL_ID , "du" , mqtt_sub_topic_idx , tot_len );
}

/**
 * @brief
 * @param
 * @param
 * @param
 * @param
 */
static void mqtt_incoming_data_cb( void* arg , const u8_t* data , u16_t len , u8_t flags )
{
	UNUSED(arg);

	lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_LWL_ID , "duc" , mqtt_sub_topic_idx , len , flags );

	// handling only completed payloads
	if( flags & MQTT_DATA_FLAG_LAST )
	{
		// verify topic and payload
		if( mqtt_sub_topic_idx < MQTT_UNKOWN_TOPIC || mqtt_sub_topic_idx >= MQTT_MAX_TOPICS )
		{	// PANIC! This can only happen by memory corruption!
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_PANIC_LWL_ID , "" );
			assert( true );
			return;
		}
		if( mqtt_sub_topic_idx == MQTT_UNKOWN_TOPIC )
		{	// Unknown subscription topic.
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_UNKOWN_LWL_ID , "" );
			return;
		}
		if( len > MQTT_PAYLOAD_MAX_SIZE )
		{	// Can't handle such large messages.
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_SIZE_LWL_ID , "" );
			return;
		}
		if( mqtt_data.sub_topics[mqtt_sub_topic_idx].valid == false )
		{	// invalid topic somehow requested. This should have been averted by mqtt_incoming_publish_cb. Maybe panic?
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_INVALID_LWL_ID , "" );
			assert( true );
			return;
		}

		// message is valid, send it to the task
		mqtt_os_message_t *msg = osMemoryPoolAlloc( mqtt_data.os_memory_pool , 0 );
		if( msg == NULL )
		{
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_ALLOC_LWL_ID , "" );
			return; // loosing a message should probably notify the other side?
		}

		msg->len = len;
		memcpy( msg->data , data , msg->len );

		osStatus_t status = osMessageQueuePut( mqtt_data.sub_topics[mqtt_sub_topic_idx].os_queue_id , &msg , 0 , 0 );
		if( status != osOK )
		{
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_QUEUE_LWL_ID , "d" , status );
			return; // loosing a message should probably notify the other side?
		}
	}
	else { /* Handle payloads that are too long, save them in a buffer or a file. */ }
}


/**
 * @brief
 * @param
 * @param
 * @param
 */
static void mqtt_connection_cb( mqtt_client_t* client , void* arg , mqtt_connection_status_t status )
{// TODO: reconnect when disconnected
	lwl_enter_record( MQTT_LWL_ID , MQTT_CONN_CB_LWL_ID , "d" , status );

	if( status != MQTT_CONNECT_ACCEPTED )
	{
		printf( "mqtt_connection_cb: Disconnected, reason: %d\n" , status );
		return;
	}

	printf( "mqtt_connection_cb: Successfully connected\n" );

	/* Register the callback function for PUB messages & subscribe */
	mqtt_set_inpub_callback( client , mqtt_incoming_publish_cb , mqtt_incoming_data_cb , arg );

	// The user supplied their connected flag through the arg
	mqtt_data.connected = true;
}


/**
 * @brief
 */
void mqtt_init()
{
	// create OS infrastructure
	mqtt_data.os_memory_pool = osMemoryPoolNew( MQTT_OS_QUEUE_NUM_ELEMENTS * MQTT_MAX_TOPICS , sizeof(mqtt_os_message_t) , NULL );
	if( mqtt_data.os_memory_pool == NULL )
		for(;;);

	mqtt_data.num_topics = 0;
	for( uint32_t i = 0 ; i < MQTT_MAX_TOPICS ; i++ )
		mqtt_data.sub_topics[i].valid = false;

	// create mqtt connection info
	mqtt_data.client = mqtt_client_new();

	ip_addr_t ip_addr;
	IP4_ADDR( &ip_addr , ETH_SERVER_IP_1 , ETH_SERVER_IP_2 , ETH_SERVER_IP_3 , ETH_SERVER_IP_4 );

	struct mqtt_connect_client_info_t client_info;
	memset( &client_info , 0 , sizeof( client_info ) );
	client_info.client_id = MQTT_CLIENT_ID;

	client_info.will_topic = MQTT_WILL_TOPIC;
	client_info.will_msg = MQTT_WILL_PAYLOAD;
	client_info.will_msg_len = sizeof( MQTT_WILL_PAYLOAD );

	// TODO: move this to a new function in lwip.c and remove it from here & all the tests.
	while( !netif_is_up( &gnetif ) || !netif_is_link_up( &gnetif ) )
		osDelay( 250 );

	osDelay( 1000 );

	// try connecting until success
	for( ; ; )
	{
		err_t error = mqtt_client_connect( mqtt_data.client , &ip_addr , MQTT_HOST_PORT , &mqtt_connection_cb , NULL , &client_info );
		if( error == ERR_OK )
			break;
	}

	// wait until connected
	while( !mqtt_data.connected )
		osDelay( 10 );

	// announce connection
	mqtt_publish( mqtt_data.client , MQTT_CONNECT_TOPIC , MQTT_CONNECT_PAYLOAD , sizeof( MQTT_CONNECT_PAYLOAD ) , 0 , 0 , NULL , NULL );
}


/**
 * @brief
 * @param
 * @param
 * @retval
 */
err_t mqtt_sub_topic( const char* const topic_name , const osMessageQueueId_t os_queue_id )
{
	// this is a shared resource. Protect any changes on it.
	taskENTER_CRITICAL();

	// check if there are any topic slots available
	if( mqtt_data.num_topics == MQTT_MAX_TOPICS )
		return ERR_MEM;

	// find first available topic slot
	uint8_t i = 0;
	while( mqtt_data.sub_topics[i].valid == true )
	{
		i++;
		assert( i < MQTT_MAX_TOPICS ); // i should never reach MQTT_MAX_TOPICS if num_topics was handled correctly
	}

	// subscribe to that topic
	strcpy( mqtt_data.sub_topics[i].name , topic_name );
	err_t error = mqtt_subscribe( mqtt_data.client , mqtt_data.sub_topics[i].name , 1 , NULL , NULL );
	if( error != ERR_OK ) return error;

	// connect OS queue
	mqtt_data.sub_topics[i].os_queue_id = os_queue_id;

	// update metadata
	mqtt_data.sub_topics[i].valid = true;
	mqtt_data.num_topics++;

	taskEXIT_CRITICAL();

	printf("MQTT subscribed to %s, index = %u , queue = %d\n" , mqtt_data.sub_topics[i].name , i , mqtt_data.sub_topics[i].os_queue_id );
	return ERR_OK;
}

/**
 * @brief Publish an mqtt message every ~200ms, going through all pub topics, and containing a counter.
 */
void mqtt_test()
{
	uint32_t counter = 0;
	char payload_buffer[18];

	while( counter < MQTT_MAX_TOPICS )
	{
		if( mqtt_data.sub_topics[counter].valid == true )
		{
			utoa( counter , payload_buffer , 10 );
			mqtt_publish( mqtt_data.client , mqtt_data.sub_topics[counter].name , payload_buffer , strlen( payload_buffer ) , 2 , 0 , NULL , NULL );

			osDelay( 500 );
		}
		counter++;
	}
}
