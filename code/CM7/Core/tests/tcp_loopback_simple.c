/*
 * tcp_loopback.c
 *
 */
#include "settings.h"

#if CURRENT_TEST == TCP_LOOPBACK
/* Includes ----------------------------------------------------------*/
#include "tcp_loopback_simple.h"
#include "lwip.h"
#include <socket.h>
#include <stdio.h>

/* Defines -----------------------------------------------------------*/

/* Typedef -----------------------------------------------------------*/

/* Variables ---------------------------------------------------------*/
int sockfd;

/* Function prototypes -----------------------------------------------*/

/* Functions -----------------------------------------------*/
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

/**
 * @brief  TCP loopback test.
 * @retval None
 */
void tcp_loopback()
{
	for( ; ; )
	{
		static char recv_message[MESSAGE_SIZE];
		// TODO: convert this to use select
		volatile ssize_t read_len = lwip_read( sockfd , recv_message , MESSAGE_SIZE );
		volatile ssize_t write_len = lwip_write( sockfd , recv_message , read_len );
	}
}

void tcp_destroy()
{
	lwip_shutdown( sockfd , SHUT_RDWR );   // optional but recommended
	lwip_close( sockfd );
	sockfd = -1;
}

#endif
