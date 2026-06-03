/*
 * udp_test.h
 *
 */

#ifndef TESTS_UDP_TEST_H_
#define TESTS_UDP_TEST_H_

#include "mqtt_client.h"

/* Exported functions prototypes ---------------------------------------------*/
void udp_set_up();
void udp_tx_datahose( osMessageQueueId_t mqtt_queue );
void udp_destroy();

#endif /* TESTS_UDP_TEST_H_ */
