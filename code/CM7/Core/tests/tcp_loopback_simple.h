/*
 * tcp_loopback_simple.h
 *
 */

#ifndef TESTS_TCP_LOOPBACK_SIMPLE_H_
#define TESTS_TCP_LOOPBACK_SIMPLE_H_

#include "settings.h"

#if CURRENT_TEST == TCP_LOOPBACK

/* Exported functions prototypes ---------------------------------------------*/
void tcp_set_up();

#if LWIP_IMPLEMENTATION == SOCKET_API
void tcp_loopback();
#endif

#endif

#endif /* TESTS_TCP_LOOPBACK_SIMPLE_H_ */
