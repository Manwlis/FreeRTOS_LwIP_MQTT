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

/* Private types ---------------------------------------------------------*/
typedef struct _mqtt_sub_topic_t
{
	char name[MQTT_TOPIC_NAME_MAX_SIZE];    // Name of the topic
	osMessageQueueId_t os_queue_id;         // Queue of the topic
	bool valid;                             // Valid flag
}mqtt_sub_topic_t;

typedef struct _mqtt_data_t
{
	mqtt_sub_topic_t sub_topics[MQTT_MAX_SUBBED_TOPICS];    // Array of subbed topics
	osMemoryPoolId_t os_memory_pool;    // Memory pool of the incoming messages
	mqtt_client_t* client;              // LwIP's MQTT API data structure
	uint8_t num_topics;                 // Number of subbed topics
	bool connected;                     // Connected to broker flag
}mqtt_data_t;

/* Variables ---------------------------------------------------------*/
static mqtt_data_t mqtt_data = { 0 };

// These pointers are used to externalize any data required by the user
bool* const mqtt_connection_status = &(mqtt_data.connected);

// used to pass topic info from mqtt_incoming_publish_cb() to mqtt_incoming_data_cb()
static volatile int32_t mqtt_sub_topic_idx = MQTT_UNKOWN_TOPIC;

/* Functions ---------------------------------------------------------*/
/**
 * @brief	Topic Callback for the subscribed topics. Determines from which topic the incoming message originates.
 * @param	arg
 * @param	topic	Topic of the message
 * @param	tot_len	Total length of the message
 */
static void mqtt_incoming_publish_cb( void* arg , const char* topic , u32_t tot_len )
{
	UNUSED( arg );

	mqtt_sub_topic_idx = MQTT_UNKOWN_TOPIC;

	for( uint8_t i = 0 ; i < MQTT_MAX_SUBBED_TOPICS ; i++ )
		if( mqtt_data.sub_topics[i].valid )
			if( strcmp( topic , mqtt_data.sub_topics[i].name ) == 0 )
			{
				mqtt_sub_topic_idx = i;
				break;
			}

	lwl_enter_record( MQTT_LWL_ID , MQTT_IN_PUB_CB_LWL_ID , "du" , mqtt_sub_topic_idx , tot_len );
}

/**
 * @brief	Data callback for the subscribed topics. Pushes the payload to the appropriate task.
 * @param	arg
 * @param	data	Payload of the message
 * @param	len		Size of the payload
 * @param	flags	Data callback flags
 */
static void mqtt_incoming_data_cb( void* arg , const u8_t* data , u16_t len , u8_t flags )
{
	UNUSED( arg );

	lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_LWL_ID , "duc" , mqtt_sub_topic_idx , len , flags );

	// handling only completed payloads
	if( flags & MQTT_DATA_FLAG_LAST )
	{
		// verify topic and payload
		if( mqtt_sub_topic_idx < MQTT_UNKOWN_TOPIC || mqtt_sub_topic_idx >= MQTT_MAX_SUBBED_TOPICS )
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
		mqtt_os_message_t* msg = osMemoryPoolAlloc( mqtt_data.os_memory_pool , 0 );
		if( msg == NULL )
		{
			lwl_enter_record( MQTT_LWL_ID , MQTT_IN_DATA_CB_ALLOC_LWL_ID , "" );
			return; // loosing a message should probably notify the other side?
		}

		msg->len = len;
		memcpy( msg->data , data , msg->len );
		msg->topic_id = mqtt_sub_topic_idx;

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
 * @brief	Callback for when we get connected to the broker, or we failed to. Sets the sub callbacks
 * @param	client
 * @param	arg
 * @param	status	Connection status code
 */
static void mqtt_connection_cb( mqtt_client_t* client , void* arg , mqtt_connection_status_t status )
{ // TODO: reconnect when disconnected
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
 * @brief Initializes everything needed by the mqtt client. Mqtt internals, memory pool, mqtt_data struct.
 */
err_t mqtt_init()
{
	// create OS infrastructure
	if( mqtt_data.os_memory_pool == NULL )
		mqtt_data.os_memory_pool = osMemoryPoolNew( MQTT_OS_QUEUE_NUM_ELEMENTS * MQTT_MAX_SUBBED_TOPICS , sizeof(mqtt_os_message_t) , NULL );

	assert( mqtt_data.os_memory_pool != NULL );

	mqtt_data.num_topics = 0;
	for( uint32_t i = 0 ; i < MQTT_MAX_SUBBED_TOPICS ; i++ )
		mqtt_data.sub_topics[i].valid = false;

	// create mqtt connection info
	if( mqtt_data.client == NULL )
	{
		LOCK_TCPIP_CORE();
		mqtt_data.client = mqtt_client_new();
		UNLOCK_TCPIP_CORE();
	}

	assert( mqtt_data.client != NULL );

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

	// try connecting
	LOCK_TCPIP_CORE();
	err_t error = mqtt_client_connect( mqtt_data.client , &ip_addr , MQTT_HOST_PORT , &mqtt_connection_cb , NULL , &client_info );
	UNLOCK_TCPIP_CORE();
	return error;
}

/**
 * @brief	Subscribe to a mqtt topic
 * @param	topic_name	Name of the topic to subscribe
 * @param	os_queue_id	Id of the os queue where the topic payload will be put into
 * @param	topic_id	Id of the topic. -1 if failure
 * @retval	ERR_OK if success, error code if failure
 */
err_t mqtt_sub_topic( const char* const topic_name , const osMessageQueueId_t os_queue_id , sub_topic_id_t* const topic_id )
{
	// this is a shared resource. Protect any changes on it.
	taskENTER_CRITICAL( );

	*topic_id = -1;

	// check if there are any topic slots available
	if( mqtt_data.num_topics == MQTT_MAX_SUBBED_TOPICS )
		return ERR_MEM;

	// find first available topic slot
	uint8_t i = 0;
	while( mqtt_data.sub_topics[i].valid == true )
	{
		i++;
		assert( i < MQTT_MAX_SUBBED_TOPICS ); // i should never reach MQTT_MAX_TOPICS if num_topics was handled correctly
	}

	// subscribe to that topic
	strcpy( mqtt_data.sub_topics[i].name , topic_name );
	LOCK_TCPIP_CORE();
	err_t error = mqtt_subscribe( mqtt_data.client , mqtt_data.sub_topics[i].name , 1 , NULL , NULL );
	UNLOCK_TCPIP_CORE();
	if( error != ERR_OK ) return error;

	// connect OS queue
	mqtt_data.sub_topics[i].os_queue_id = os_queue_id;

	// update metadata
	mqtt_data.sub_topics[i].valid = true;
	mqtt_data.num_topics++;

	*topic_id = i;

	taskEXIT_CRITICAL( );

	printf( "MQTT subscribed to %s, index = %u , queue = %d , id = %ld\n" , mqtt_data.sub_topics[i].name , i , (int)mqtt_data.sub_topics[i].os_queue_id , *topic_id );
	return ERR_OK;
}

/**
 * @brief	Unsubs from a mqtt topic
 * @param	topic_id	Topic to unsub from
 * @retval	ERR_OK if success, error code if failure
 */
err_t mqtt_unsub_topic( sub_topic_id_t* const topic_id )
{
	if( *topic_id < 0 || *topic_id >= MQTT_MAX_SUBBED_TOPICS )
		return ERR_VAL;

	if( mqtt_data.sub_topics[*topic_id].valid == false )
		return ERR_ALREADY;

	LOCK_TCPIP_CORE();
	err_t error = mqtt_unsubscribe( mqtt_data.client , mqtt_data.sub_topics[*topic_id].name , NULL , NULL );
	UNLOCK_TCPIP_CORE();
	if( error != ERR_OK ) { return error ; }

	mqtt_data.sub_topics[*topic_id].valid = false;
	*topic_id = -1;

	return ERR_OK;
}

/**
 * @brief	Publish function.
 * @param	topic			Publish topic string
 * @param	payload			Data to publish (NULL is allowed)
 * @param	payload_length	Length of payload (0 is allowed)
 * @param	qos				Quality of service, 0 1 or 2
 * @param	retain			MQTT retain flag
 * @param	cb				Callback to call when publish is complete or has timed out
 * @param	arg				User supplied argument to publish callback
 * @retval	ERR_OK if success, error code if failure
 */
err_t mqtt_publish_wrapper( const char *topic , const void *payload , u16_t payload_length , u8_t qos , u8_t retain , mqtt_request_cb_t cb , void *arg )
{
	if( mqtt_data.connected == false )
		return ERR_CONN;

	LOCK_TCPIP_CORE();
	err_t rv = mqtt_publish( mqtt_data.client , topic , payload , payload_length , qos , retain , cb , arg );
	UNLOCK_TCPIP_CORE();

	return rv;
}

/**
 * @brief	Return an allocated message back to the memory pool
 * @param	message	Message to be freed
 * @retval	ERR_OK if success, error code if failure
 */
osStatus_t mqtt_free_message( const mqtt_os_message_t* const message )
{
	return osMemoryPoolFree( mqtt_data.os_memory_pool , message );
}


/**
 * @brief Publish an mqtt message every ~200ms, going through all sub topics, and containing a counter.
 */
void mqtt_test()
{
	uint32_t counter = 0;
	char payload_buffer[18];

	while( counter < MQTT_MAX_SUBBED_TOPICS )
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
