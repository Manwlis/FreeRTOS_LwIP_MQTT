/*
 * udp_test.c
 *
 */

/* Includes ----------------------------------------------------------*/
#include "settings.h"

#if CURRENT_TEST == UDP_TX_BENCHMARK

#if LWIP_IMPLEMENTATION == RAW_API
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include <string.h> // memcpy
#else
#include <socket.h>
#endif

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
#if LWIP_IMPLEMENTATION == RAW_API
	ip_addr_t PC_IPADDR;
	IP4_ADDR( &PC_IPADDR , ETH_SERVER_IP_1 , ETH_SERVER_IP_2 , ETH_SERVER_IP_3 , ETH_SERVER_IP_4 );

	struct udp_pcb* my_udp = udp_new();
	udp_connect( my_udp , &PC_IPADDR , ETH_SERVER_PORT );

#else
	int sockfd = socket( AF_INET , SOCK_DGRAM , 0 );

	struct sockaddr_in addr;
	memset( &addr , 0 , sizeof( addr ) );

	addr.sin_family = AF_INET;
	addr.sin_port = htons( ETH_SERVER_PORT );
	addr.sin_addr.s_addr = inet_addr( ETH_SERVER_IP );
#endif

	while( !netif_is_up( &gnetif ) || !netif_is_link_up( &gnetif ) )
		osDelay( 250 );
	osDelay( 200 );

	printf( "IP: %s\n"    , ipaddr_ntoa( &gnetif.ip_addr ) );
	printf( "Mask: %s\n"  , ipaddr_ntoa( &gnetif.netmask ) );
	printf( "GW: %s\n"    , ipaddr_ntoa( &gnetif.gw ) );
	printf( "netif: %d\n" , netif_is_up( &gnetif ) );
	printf( "Link: %d\n"  , netif_is_link_up( &gnetif ) );

	for( ; ; )
	{
#if LWIP_IMPLEMENTATION == RAW_API
		struct pbuf* udp_buffer = udp_buffer = pbuf_alloc( PBUF_TRANSPORT , MESSAGE_SIZE , PBUF_RAM );

		if( udp_buffer != NULL )
		{
			memcpy( udp_buffer->payload , message , MESSAGE_SIZE );
			LOCK_TCPIP_CORE();
			udp_send( my_udp , udp_buffer );
			UNLOCK_TCPIP_CORE();
			pbuf_free( udp_buffer );
		}
#else
		sendto( sockfd , message , MESSAGE_SIZE , 0 , (struct sockaddr* )&addr , sizeof( addr ) );
#endif
	}
}
#endif
