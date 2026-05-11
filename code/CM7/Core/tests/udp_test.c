/*
 * udp_test.c
 *
 */

/* Includes ----------------------------------------------------------*/
#include "settings.h"

#if CURRENT_TEST == UDP_TX_BENCHMARK

#include <socket.h>
#include <stdio.h>
#include "lwip.h"

/* Typedef -----------------------------------------------------------*/

/* Variables ---------------------------------------------------------*/

/* Functions ---------------------------------------------------------*/
/**
 * @brief  UDP transmit test.
 * @retval None
 */
void udp_tx_benchmark()
{
	static const char message[MESSAGE_SIZE] = { [0 ... ( MESSAGE_SIZE - 1 )] = 1 };

	/* Init UDP connection */
	int sockfd = socket( AF_INET , SOCK_DGRAM , 0 );

	struct sockaddr_in addr;
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

	for( ; ; )
		if( sendto( sockfd , message , MESSAGE_SIZE , 0 , (struct sockaddr* )&addr , sizeof( addr ) ) == -1 )
			break;

	lwip_close( sockfd );
}
#endif
