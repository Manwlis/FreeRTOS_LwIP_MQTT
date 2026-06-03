/*
 * tcp_loopback_multitask.c
 *
 */


/* Includes ----------------------------------------------------------*/
#include "settings.h"
#include "lwip.h"
#include <socket.h>
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h" // vQueueAddToRegistry
#include "mqtt_client.h"

/* Defines -----------------------------------------------------------*/

/* Typedefs -----------------------------------------------------------*/
typedef struct
{
	enum type_t { DATA , CLOSED } type;
	struct network_mbuf_t
	{
		size_t len;
		uint8_t data[NETWORK_MESSAGE_SIZE];
	} buffer; // buffer for the messages received from the network
} network_message_t;

/* Variables ---------------------------------------------------------*/
static int sockfd;

static osThreadId_t rx_task_handle;
static osThreadId_t tx_task_handle;

static osMessageQueueId_t network_message_free;
static osMessageQueueId_t network_message_rx_to_tx;
static network_message_t network_message_pool[NUM_NETWORK_MESSAGES]; // using osMemoryPool would be more idiomatic and safe

/* Functions ---------------------------------------------------------*/
void set_up_queues()
{
	network_message_free = osMessageQueueNew( NUM_NETWORK_MESSAGES , sizeof(network_message_t*) , NULL );
	network_message_rx_to_tx = osMessageQueueNew( NUM_NETWORK_MESSAGES , sizeof(network_message_t*) , NULL );

	for( int i = 0 ; i < NUM_NETWORK_MESSAGES ; i++ )
	{
		network_message_t* message = &network_message_pool[i];
		osMessageQueuePut( network_message_free , &message , 0 , 0 ); // this calls xQueueSendToBack, maybe we need xQueueSend?
	}

	// So we can monitor them with the debugger
	vQueueAddToRegistry( network_message_free , "network_msg_free" );
	vQueueAddToRegistry( network_message_rx_to_tx , "network_msg_rx_to_tx" );
}

void tcp_multi_set_up()
{
	set_up_queues();

	struct sockaddr_in addr;
	memset( &addr , 0 , sizeof( addr ) );

	addr.sin_family = AF_INET;
	addr.sin_port = htons( ETH_SERVER_PORT );
	addr.sin_addr.s_addr = inet_addr( ETH_SERVER_IP );

	sockfd = lwip_socket( AF_INET , SOCK_STREAM , IPPROTO_TCP );
	if( sockfd == -1 )
		printf( "failed to create socket, errno = %d\n" , errno );

	while( !netif_is_up( &gnetif ) || !netif_is_link_up( &gnetif ) )
		osDelay( 250 );

	osDelay( 1000 );

	// TODO: Investigate why sometimes blocks indefinitely here. Increasing the above osDelay seems to alleviate the issue.
	// Maybe we need for something else to be set up before trying to connect
	int ret = lwip_connect( sockfd , (const struct sockaddr*) &addr , sizeof( addr ) );
	if( ret < 0 )
		printf( "failed to connect socket, errno = %d\n" , errno );

	printf( "IP: %s\n" , ipaddr_ntoa( &gnetif.ip_addr ) );
	printf( "Mask: %s\n" , ipaddr_ntoa( &gnetif.netmask ) );
	printf( "GW: %s\n" , ipaddr_ntoa( &gnetif.gw ) );
	printf( "netif: %d\n" , netif_is_up( &gnetif ) );
	printf( "Link: %d\n" , netif_is_link_up( &gnetif ) );
}

void tcp_multi_rx( osMessageQueueId_t mqtt_queue )
{
	uint8_t msg_prio = 0;
	network_message_t* network_message;

	for( ; ; )
	{
		// check for stop command
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , 0 );

		if( status == osOK && message != NULL )
		{
			if( compare_mqtt_payload( message , "stop" , true ) )
			{
				// connection closed, send message to the tx task
				network_message->type = CLOSED;
				osMessageQueuePut( network_message_rx_to_tx , &network_message , msg_prio , osWaitForever );

				osMemoryPoolFree( mqtt_data.os_memory_pool , message );
				return;
			}
			osMemoryPoolFree( mqtt_data.os_memory_pool , message );
		}

		for( int i = 0 ; i < 1000 ; i++ )
		{
			// Get free buffer
			if( osMessageQueueGet( network_message_free , &network_message , &msg_prio , osWaitForever ) != osOK )
				continue;

			// Get network message
			size_t received_bytes = 0;

			while( received_bytes < NETWORK_MESSAGE_SIZE )
			{
				int ret = lwip_read( sockfd , network_message->buffer.data + received_bytes , NETWORK_MESSAGE_SIZE - received_bytes );

				if( ret <= 0 )
				{
					// connection closed, send message to the tx task
					network_message->type = CLOSED;
					osMessageQueuePut( network_message_rx_to_tx , &network_message , msg_prio , osWaitForever );
					return;
				}
				received_bytes += ret;
			}

			network_message->type = DATA;
			network_message->buffer.len = received_bytes;

			// Notify next task that data is available
			osMessageQueuePut( network_message_rx_to_tx , &network_message , msg_prio , osWaitForever );
		}
	}
}

void tcp_multi_tx()
{
	uint8_t msg_prio = 0;
	network_message_t* network_message;

	for( ; ; )
	{
		// Wait until a message is available
		if( osMessageQueueGet( network_message_rx_to_tx , &network_message , &msg_prio , osWaitForever ) != osOK )
			continue;

		if( network_message->type == CLOSED )
		{
			// connection closed, return message to queue
			osMessageQueuePut( network_message_free , &network_message , msg_prio , 0 );
			osThreadExit();
		}

		// Transmit it back
		size_t sent_bytes = 0;

		while( sent_bytes < network_message->buffer.len )
		{
			int ret = lwip_write( sockfd , network_message->buffer.data + sent_bytes , network_message->buffer.len - sent_bytes );

			if( ret <= 0 )
			{
				// connection closed, return message to queue
				osMessageQueuePut( network_message_free , &network_message , msg_prio , 0 );
				osThreadExit();
			}

			sent_bytes += ret;
		}

		// Return buffer to pool
		osMessageQueuePut( network_message_free , &network_message , msg_prio , osWaitForever );
	}
}


void tcp_multi_loopback( osMessageQueueId_t mqtt_queue )
{
	rx_task_handle = osThreadGetId();

	const osThreadAttr_t tx_task_attributes = { .name = "tx_task" , .stack_size = 2048 , .priority = (osPriority_t) osPriorityNormal1 , };
	tx_task_handle = osThreadNew( tcp_multi_tx , NULL , &tx_task_attributes );

	tcp_multi_rx( mqtt_queue );
}


void tcp_multi_destroy()
{
	lwip_shutdown( sockfd , SHUT_RDWR );
	lwip_close( sockfd );
	sockfd = -1;

	osMessageQueueDelete( network_message_free );
	osMessageQueueDelete( network_message_rx_to_tx );
}
