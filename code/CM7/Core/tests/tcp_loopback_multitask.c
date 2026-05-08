/*
 * tcp_loopback_multitask.c
 *
 */
#include "settings.h"

#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK

/* Includes ----------------------------------------------------------*/
#include <socket.h>
#include <stdio.h>
#include "lwip.h"
#include "FreeRTOS.h"
#include "queue.h" // vQueueAddToRegistry


/* Defines -----------------------------------------------------------*/

/* Typedefs -----------------------------------------------------------*/
typedef struct
{
	enum type_t { DATA , CLOSED } type;
	struct network_mbuf_t
	{
		size_t len;
		uint8_t data[MESSAGE_SIZE];
	} buffer; // buffer for the messages received from the network
} network_message_t;

/* Variables ---------------------------------------------------------*/
int sockfd;

osMessageQueueId_t network_message_free;
osMessageQueueId_t network_message_rx_to_tx;
static network_message_t network_message_pool[NUM_NETWORK_MESSAGES];

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

void tcp_set_up()
{
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

void tcp_rx()
{
	uint8_t msg_prio = 0;

	for( ; ; )
	{
		network_message_t* message;

		// Get free buffer
		if( osMessageQueueGet( network_message_free , &message , &msg_prio , osWaitForever ) != osOK )
			continue;

		// Get message
		size_t received_bytes = 0;

		while( received_bytes < MESSAGE_SIZE )
		{
			int ret = lwip_read( sockfd , message->buffer.data + received_bytes , MESSAGE_SIZE - received_bytes );

			if( ret <= 0 )
			{
				// connection closed, send message to the following task
				message->type = CLOSED;
				osMessageQueuePut( network_message_rx_to_tx , &message , msg_prio , osWaitForever );
				return;
			}
			received_bytes += ret;
		}

		message->type = DATA;
		message->buffer.len = received_bytes;

		// Notify next task that data is available
		osMessageQueuePut( network_message_rx_to_tx , &message , msg_prio , osWaitForever );
	}
}

void tcp_tx()
{
	uint8_t msg_prio = 0;

	for( ; ; )
	{
		network_message_t* message;

		// Wait until a message is available
		if( osMessageQueueGet( network_message_rx_to_tx , &message , &msg_prio , osWaitForever ) != osOK )
			continue;

		if( message->type == CLOSED )
		{
			// connection closed, return message to queue
			osMessageQueuePut( network_message_free , &message , msg_prio , 0 );
			return;
		}

		// Transmit it back
		size_t sent_bytes = 0;

		while( sent_bytes < message->buffer.len )
		{
			int ret = lwip_write( sockfd , message->buffer.data + sent_bytes , message->buffer.len - sent_bytes );

			if( ret <= 0 )
			{
				// connection closed, return message to queue
				osMessageQueuePut( network_message_free , &message , msg_prio , 0 );
				return;
			}

			sent_bytes += ret;
		}

		// Return buffer to pool
		osMessageQueuePut( network_message_free , &message , msg_prio , osWaitForever );
	}
}

#endif
