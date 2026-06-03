/*
 * tcp_loopback_simple.h
 *
 */

#ifndef TESTS_TCP_LOOPBACK_SIMPLE_H_
#define TESTS_TCP_LOOPBACK_SIMPLE_H_

#include "mqtt_client.h"

/* Exported functions prototypes ---------------------------------------------*/
void tcp_simple_set_up();
void tcp_simple_loopback( osMessageQueueId_t mqtt_queue );
void tcp_simple_destroy();

#endif /* TESTS_TCP_LOOPBACK_SIMPLE_H_ */
