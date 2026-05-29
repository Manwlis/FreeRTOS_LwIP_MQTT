/*
 * mqctt.c
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "mqtt.h"
#include "mqtt_priv.h"

#include "lwip.h"
#include "mqtt_client.h"
#include "lwl.h"

/* Variables ---------------------------------------------------------*/
mqtt_sub_topics_t mqtt_sub_topics[NUM_MQTT_SUB_TOPICS] = { MQTT_SUB_TOPICS( GEN_ARRAY ) };
// memory pool for the os messages
osMemoryPoolId_t mqtt_os_message_pool;

mqtt_client_t* client;
static volatile bool mqtt_connected = false;
static volatile mqtt_sub_topic_idx_t mqtt_sub_topic_idx = MQTT_UNKOWN_TOPIC;

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

	for( mqtt_sub_topic_idx_t i = 0 ; i < NUM_MQTT_SUB_TOPICS ; i++ )
	{
		if( strcmp( topic , mqtt_sub_topics[i].name ) == 0 )
		{
			mqtt_sub_topic_idx = i;
			break;
		}
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
		if( mqtt_sub_topic_idx < MQTT_UNKOWN_TOPIC || mqtt_sub_topic_idx >= NUM_MQTT_SUB_TOPICS )
		{	// PANIC! This can only happen by memory corruption!
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_PANIC_LWL_ID , "" );
			return;
		}
		if( mqtt_sub_topic_idx == MQTT_UNKOWN_TOPIC )
		{	// Unknown subscription topic.
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_UNKOWN_LWL_ID , "" );
			return;
		}
		if( len > MQTT_OS_POOL_ELEMENT_SIZE )
		{	// Can't handle such large messages.
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_SIZE_LWL_ID , "" );
			return;
		}

		// message is valid, send it to the task
		mqtt_os_message_t *msg = osMemoryPoolAlloc( mqtt_os_message_pool , 0 );
		if( msg == NULL )
		{
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_ALLOC_LWL_ID , "" );
			return; // loosing a message should probably notify the other side?
		}

		msg->len = len;
		memcpy( msg->data , data , msg->len );

		osStatus_t status = osMessageQueuePut( mqtt_sub_topics[mqtt_sub_topic_idx].os_queue_id , &msg , 0 , 0 );
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
{
	lwl_enter_record( MQTT_LWL_ID , MQTT_CONN_CB_LWL_ID , "d" , status );

	if( status != MQTT_CONNECT_ACCEPTED )
	{
		printf( "mqtt_connection_cb: Disconnected, reason: %d\n" , status );
		return;
	}

	printf( "mqtt_connection_cb: Successfully connected\n" );

	/* Register the callback function for PUB messages & subscribe */
	mqtt_set_inpub_callback( client , mqtt_incoming_publish_cb , mqtt_incoming_data_cb , arg );

	mqtt_connected = true;
}


/**
 * @brief
 */
void init_mqtt()
{
	// create OS infastructure
	for( int i = 0 ; i < NUM_MQTT_SUB_TOPICS ; i++ )
	{
		mqtt_sub_topics[i].os_queue_id = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );

		// If something fails, we should clean up everything. Stall for now.
		if( mqtt_sub_topics[i].os_queue_id == NULL )
			for(;;);
	}
	mqtt_os_message_pool = osMemoryPoolNew( MQTT_OS_QUEUE_NUM_ELEMENTS * NUM_MQTT_SUB_TOPICS , sizeof(mqtt_os_message_t) , NULL );
	if( mqtt_os_message_pool == NULL )
		for(;;);

	// create mqtt connection info
	client = mqtt_client_new();

	ip_addr_t ip_addr;
	IP4_ADDR( &ip_addr , ETH_SERVER_IP_1 , ETH_SERVER_IP_2 , ETH_SERVER_IP_3 , ETH_SERVER_IP_4 );

	struct mqtt_connect_client_info_t ci;
	memset( &ci , 0 , sizeof( ci ) );
	ci.client_id = "lwip_test";

	// TODO: move this to a new function in lwip.c and remove it from all the tests.
	while( !netif_is_up( &gnetif ) || !netif_is_link_up( &gnetif ) )
		osDelay( 250 );

	osDelay( 1000 );

	// try connecting until success
	for( ; ; )
	{
		err_t error = mqtt_client_connect( client , &ip_addr , 1883 , &mqtt_connection_cb , NULL , &ci );
		if( error == ERR_OK )
			break;
	}

	// wait until connected
	while( !mqtt_connected )
		osDelay( 10 );

	// subscribe to topics
	mqtt_sub_topic_idx_t i = 0;
	while( i < NUM_MQTT_SUB_TOPICS )
	{
		err_t error = mqtt_subscribe( client , mqtt_sub_topics[i].name , 1 , NULL , NULL );
		printf( "%d    %s\n" , error , mqtt_sub_topics[i].name );
		// retry failed subscribes
		if( error == ERR_OK )
			i++;
	}

	// send hello
	const char* topic = "devices";
	const char* pub_payload = "STM32H755ZI connected.";
	mqtt_publish( client , topic , pub_payload , strlen( pub_payload ) , 2 , 0 , NULL , NULL );
}

/**
 * @brief Publish an mqtt message every ~200ms, going through all pub topics, and containing a counter.
 */
void test_mqtt()
{
	uint32_t counter = 0;
	mqtt_sub_topic_idx_t i = 0;
	char payload_buffer[18];

	while( counter < 10 )
	{
		utoa( counter , payload_buffer , 10 );
		mqtt_publish( client , mqtt_sub_topics[i].name , payload_buffer , strlen( payload_buffer ) , 2 , 0 , NULL , NULL );

		osDelay( 500 );

		counter++;
		i++;
		if( i == NUM_MQTT_SUB_TOPICS )
			i = 0;
	}
}
