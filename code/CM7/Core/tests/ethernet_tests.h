/*
 * ethernet_tests.h
 *
 *  Created on: 8 Μαΐ 2026
 *      Author: MSI
 */

#ifndef TESTS_ETHERNET_TESTS_H_
#define TESTS_ETHERNET_TESTS_H_

/* Exported functions prototypes ---------------------------------------------*/
#if CURRENT_TEST == UDP_TX_BENCHMARK
void udp_tx_benchmark();
#endif

#if ( CURRENT_TEST == TCP_LOOPBACK_MULTITASK ) || ( CURRENT_TEST == TCP_LOOPBACK )
void tcp_set_up();
#endif

#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
void tcp_rx();
void tcp_tx();
#endif

#if CURRENT_TEST == TCP_LOOPBACK
#if LWIP_IMPLEMENTATION == SOCKET_API
void tcp_loopback();
#endif
#endif

#endif /* TESTS_ETHERNET_TESTS_H_ */
