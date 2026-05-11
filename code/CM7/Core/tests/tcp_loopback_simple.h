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
void tcp_loopback();
void tcp_destroy();

#endif //CURRENT_TEST == TCP_LOOPBACK
#endif /* TESTS_TCP_LOOPBACK_SIMPLE_H_ */
