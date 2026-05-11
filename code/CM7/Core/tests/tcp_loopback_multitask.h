/*
 * tcp_loopback_multitask.h
 *
 */

#ifndef TESTS_TCP_LOOPBACK_MULTITASK_H_
#define TESTS_TCP_LOOPBACK_MULTITASK_H_

#include "settings.h"

#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK

/* Includes ----------------------------------------------------------*/
#include "cmsis_os2.h"

/* Exported types ------------------------------------------------------------*/

/* Exported variables ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void set_up_queues();
void tcp_set_up();
void tcp_rx();
void tcp_tx();
void tcp_destroy();

#endif //CURRENT_TEST == TCP_LOOPBACK_MULTITASK

#endif /* TESTS_TCP_LOOPBACK_MULTITASK_H_ */
