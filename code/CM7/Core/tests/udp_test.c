/*
 * udp_test.c
 *
 */

/* Includes ----------------------------------------------------------*/
#include "settings.h"
#include "lwip.h"
#include <socket.h>
#include "mqtt_client.h"

/* Typedef -----------------------------------------------------------*/

/* Variables ---------------------------------------------------------*/
static int sockfd;
static struct sockaddr_in addr;

/* Functions ---------------------------------------------------------*/

void udp_set_up()
{
	sockfd = socket( AF_INET , SOCK_DGRAM , 0 );

	memset( &addr , 0 , sizeof( addr ) );

	addr.sin_family = AF_INET;
	addr.sin_port = htons( ETH_SERVER_PORT );
	addr.sin_addr.s_addr = inet_addr( ETH_SERVER_IP );

	while( !netif_is_up( &gnetif ) || !netif_is_link_up( &gnetif ) )
		osDelay( 250 );
	osDelay( 200 );

	printf( "IP: %s\n"    , ipaddr_ntoa( &gnetif.ip_addr ) );
	printf( "Mask: %s\n"  , ipaddr_ntoa( &gnetif.netmask ) );
	printf( "GW: %s\n"    , ipaddr_ntoa( &gnetif.gw ) );
	printf( "netif: %d\n" , netif_is_up( &gnetif ) );
	printf( "Link: %d\n"  , netif_is_link_up( &gnetif ) );
}
/**
 * @brief  UDP transmit test.
 * @retval None
 */
void udp_tx_datahose( osMessageQueueId_t mqtt_queue )
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
				break;
			}
			osMemoryPoolFree( mqtt_data.os_memory_pool , mqtt_message );
		}

		// send the message 1000 times
		static const char network_message[NETWORK_MESSAGE_SIZE] = { [0 ... ( NETWORK_MESSAGE_SIZE - 1 )] = 1 };

		for( int i = 0 ; i < 1000 ; i++ )
			if( sendto( sockfd , network_message , NETWORK_MESSAGE_SIZE , 0 , (struct sockaddr* )&addr , sizeof( addr ) ) == -1 )
				return;
	}
}

void udp_destroy()
{
	lwip_close( sockfd );
}
