/*
 * tcp_loopback.c
 *
 */

/* Includes ----------------------------------------------------------*/
#include "settings.h"
#include "tcp_loopback_simple.h"
#include <socket.h>
#include "lwip.h"
#include "mqtt_client.h"

/* Defines -----------------------------------------------------------*/

/* Typedef -----------------------------------------------------------*/

/* Variables ---------------------------------------------------------*/
static int sockfd;

/* Function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------*/
void tcp_simple_set_up()
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

/**
 * @brief  TCP loopback test.
 * @param mqtt_queue	Queue where the stop command is expected to come from
 * @retval None
 */
void tcp_simple_loopback( osMessageQueueId_t mqtt_queue )
{
	for( ; ; )
	{
		// check for stop command
		mqtt_os_message_t* mqtt_message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &mqtt_message , NULL , 0 );

		if( status == osOK && mqtt_message != NULL )
		{
			if( compare_mqtt_payload( mqtt_message , "stop" , true ) )
			{
				osMemoryPoolFree( mqtt_data.os_memory_pool , mqtt_message );
				return;
			}
			osMemoryPoolFree( mqtt_data.os_memory_pool , mqtt_message );
		}

		 // check the queue ever 1000 packets
		for( int i = 0 ; i < 1000 ; i++ )
		{
			static char network_message[NETWORK_MESSAGE_SIZE];
			volatile ssize_t read_len = lwip_read( sockfd , network_message , NETWORK_MESSAGE_SIZE );
			if( read_len == -1 ) return;
			volatile ssize_t write_len = lwip_write( sockfd , network_message , read_len );
			if( write_len == -1 ) return;
		}
	}
}

void tcp_simple_destroy()
{
	lwip_shutdown( sockfd , SHUT_RDWR );
	lwip_close( sockfd );
	sockfd = -1;
}
