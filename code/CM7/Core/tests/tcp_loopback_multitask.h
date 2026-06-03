/*
 * tcp_loopback_multitask.h
 *
 */

#ifndef TESTS_TCP_LOOPBACK_MULTITASK_H_
#define TESTS_TCP_LOOPBACK_MULTITASK_H_

/* Includes ----------------------------------------------------------*/
#include "mqtt_client.h"

/* Exported types ------------------------------------------------------------*/

/* Exported variables ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void tcp_multi_set_up();
void tcp_multi_loopback( osMessageQueueId_t mqtt_queue );
void tcp_multi_destroy();

#endif /* TESTS_TCP_LOOPBACK_MULTITASK_H_ */
