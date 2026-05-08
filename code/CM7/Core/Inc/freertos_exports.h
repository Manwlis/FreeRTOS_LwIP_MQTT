/*
 * freertos_exports.h
 *
 *  Created on: 8 Μαΐ 2026
 *      Author: MSI
 */

#ifndef INC_FREERTOS_EXPORTS_H_
#define INC_FREERTOS_EXPORTS_H_

/* Includes ------------------------------------------------------------------*/
#include "settings.h"

#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
#include "cmsis_os2.h"
#endif

/* Exported types ------------------------------------------------------------*/
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
extern osMessageQueueId_t network_message_free;
extern osMessageQueueId_t network_message_rx_to_tx;
#endif

/* Exported variables ------------------------------------------------------------*/
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
typedef struct
{
	enum type_t { DATA , CLOSED } type;
	struct network_mbuf_t
	{
		size_t len;
		uint8_t data[MESSAGE_SIZE];
	} buffer; // buffer for the messages received from the network
} network_message_t;
#endif

#endif /* INC_FREERTOS_EXPORTS_H_ */
