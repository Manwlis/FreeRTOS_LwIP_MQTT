/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include "settings.h"
#include "stm32h7xx_it.h" // for the RUN_TIME_STATS

#include "udp_test.h"
#include "tcp_loopback_simple.h"
#include "tcp_loopback_multitask.h"

#include "mqtt_client.h"
#include "i2c.h"
#include "LIS3DHTR.h"
#include "spi.h"
#include "pmodals.h"
#include "adc.h"

// for vQueueAddToRegistry()
#include "FreeRTOS.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
const osThreadAttr_t i2c4_task_attributes = { .name = "i2c4_task" , .stack_size = 2048 , .priority = (osPriority_t) osPriorityNormal , };
const osThreadAttr_t spi1_task_attributes = { .name = "spi1_task" , .stack_size = 2048 , .priority = (osPriority_t) osPriorityNormal , };
const osThreadAttr_t adc3_task_attributes = { .name = "adc3_task" , .stack_size = 2048 , .priority = (osPriority_t) osPriorityNormal , };
const osThreadAttr_t eth_task_attributes = { .name = "eth_task" , .stack_size = 2048 , .priority = (osPriority_t) osPriorityNormal , };

osThreadId_t i2c4_task_handle;
osThreadId_t spi1_task_handle;
osThreadId_t adc3_task_handle;
osThreadId_t eth_task_handle;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
void StartTxTask( void* argument );
#endif

void i2c4_task( void* argument );
void spi1_task( void* argument );
void adc3_task( void* argument );
void eth_task( void* argument );
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0;
	extern TIM_HandleTypeDef htim17;
	HAL_TIM_Base_Start_IT(&htim17);
}

__weak unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
	printf("PANIIIIC!!!!!!!!!!\n");
	for(;;);
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	i2c4_task_handle = osThreadNew( i2c4_task , NULL, &i2c4_task_attributes);
	spi1_task_handle = osThreadNew( spi1_task , NULL, &spi1_task_attributes);
	adc3_task_handle = osThreadNew( adc3_task , NULL, &adc3_task_attributes);
	eth_task_handle = osThreadNew( eth_task , NULL, &eth_task_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
	UNUSED( argument );

	lwl_init();
	mqtt_init();

	// connect to mqtt
	while( mqtt_data.connected != true )
		osDelay( 100 );

	sub_topic_id_t topic_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
	vQueueAddToRegistry( mqtt_queue , "mqtt_lwl" );
	mqtt_sub_topic( MQTT_SUB_LWL_ID , mqtt_queue , &topic_id );

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever );

		if( message == NULL || status != osOK )
			continue; // did we loose a message here. Should this be logged? Can status be not OK but message pointer valid? Then we have memory leak.

		// consume message
		if( compare_mqtt_payload( message , "dump" , true ) )
			dump_log_mqtt();

		// free message
		osMemoryPoolFree( mqtt_data.os_memory_pool , message );
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void i2c4_task( void* argument )
{
	UNUSED( argument );

	// Init peripheral and devices
	LIS3DHTR_device_t LIS3DHTR_handle = LIS3DHTR_create_handle( (void*) &hi2c4 , 0x19 );
	hi2c4_wrapper.task_handle = osThreadGetId();

	// connect to mqtt
	while(mqtt_data.connected != true )
		osDelay( 100 );

	sub_topic_id_t topic_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
	vQueueAddToRegistry( mqtt_queue , "mqtt_i2c4" );
	mqtt_sub_topic( MQTT_SUB_LIS3_ID , mqtt_queue , &topic_id );

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever );

		if( message == NULL || status != osOK )
			continue; // did we loose a message here. Should this be logged? Can status be not OK but message pointer valid? Then we have memory leak.


		if( compare_mqtt_payload( message , "enable aux dacs" , true ) )
		{
			LIS3DHTR_enable_aux_adcs( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "disable aux dacs" , true ) )
		{
			LIS3DHTR_disable_aux_adcs( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "enable temp" , true ) )
		{
			LIS3DHTR_enable_temp_sensor( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "disable temp" , true ) )
		{
			LIS3DHTR_disable_temp_sensor( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "enable bdu" , true ) )
		{
			LIS3DHTR_enable_BDU( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "disable bdu" , true ) )
		{
			LIS3DHTR_disable_BDU( &LIS3DHTR_handle );
		}
		else if( compare_mqtt_payload( message , "set odr" , false ) )
		{
			const uint32_t prefix_len = sizeof("set odr ");	// mqtt command must have an extra numeric digit but no \0
			if( message->len != prefix_len ){ continue; }	// thus same size as with the compare string

			// get the numeric digit and convert it to int
			int32_t option = message->data[prefix_len-1] - '0';

			// check that it is a valid option
			if( option < ODR_POWER_DOWN || option > ODR_5_376KHZ ){ continue; }

			LIS3DHTR_set_ODR( &LIS3DHTR_handle , option );
		}
		else if( compare_mqtt_payload( message , "set res" , false ) )
		{
			const uint32_t prefix_len = sizeof("set res ");	// mqtt command must have an extra numeric digit but no \0
			if( message->len != prefix_len ){ continue; }	// thus same size as with the compare string

			// get the numeric digit and convert it to int
			int32_t option = message->data[prefix_len-1] - '0';

			// check that it is a valid option
			if( option < LIS3DHTR_LOW_POWER || option > LIS3DHTR_HIGH ){ continue; }

			LIS3DHTR_set_resolution( &LIS3DHTR_handle , option );
		}
		else if( compare_mqtt_payload( message , "get temp" , true ) )
		{
			float temp;
			LIS3DHTR_get_temp( &LIS3DHTR_handle , &temp );

			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_LIS3_TEMP_ID , &temp , sizeof( temp ) , 0 , 0 , NULL , NULL );
		}
		else if( compare_mqtt_payload( message , "get accel" , true ) )
		{
			float x;
			float y;
			float z;
			LIS3DHTR_get_acceleration( &LIS3DHTR_handle , &x , &y , &z );

			uint8_t data[ sizeof(x) + sizeof(y) + sizeof(z) ];
			memcpy( &(data[0]) , &(x) , sizeof(x) );
			memcpy( &(data[sizeof(x)]) , &(y) , sizeof(y) );
			memcpy( &(data[sizeof(x)+sizeof(y)]) , &(z) , sizeof(z) );

			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_LIS3_ACCEL_ID , data , sizeof( data ) , 0 , 0 , NULL , NULL );
		}

		// free message
		osMemoryPoolFree( mqtt_data.os_memory_pool , message );
	}
	osThreadExit();
}


void spi1_task( void* argument )
{
	UNUSED( argument );

	// Init peripheral and devices
	pmodals_device_t pmodals_handle = pmodals_create_handle( (void*) &hspi1 , 3.27f , 10000 );
	hspi1_wrapper.task_handle  = osThreadGetId();

	// connect to mqtt
	while(mqtt_data.connected != true )
		osDelay( 100 );

	sub_topic_id_t topic_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
	vQueueAddToRegistry( mqtt_queue , "mqtt_spi1" );
	mqtt_sub_topic( MQTT_SUB_ALS_ID , mqtt_queue , &topic_id );

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever );

		if( status != osOK || message == NULL )
			continue; // did we loose a message here. Should this be logged? Can status be not OK but message pointer valid? Then we have memory leak.

		// consume message
		if( compare_mqtt_payload( message , "get lux" , true ) )
		{
			float lux = 0;
			pmodals_get_lux( &pmodals_handle , &lux );
			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_ALS_LUX_ID , &lux , sizeof( lux ) , 0 , 0 , NULL , NULL );
		}

		// free message
		osMemoryPoolFree( mqtt_data.os_memory_pool , message );
	}
	osThreadExit();
}

void adc3_task( void* argument )
{
	UNUSED( argument );

	// Init peripheral and devices
	start_ADC_DMA();

	// connect to mqtt
	while(mqtt_data.connected != true )
		osDelay( 100 );

	sub_topic_id_t topic_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
	vQueueAddToRegistry( mqtt_queue , "mqtt_adc3" );
	mqtt_sub_topic( MQTT_SUB_TEMP_ID , mqtt_queue , &topic_id );

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever );

		if( message == NULL || status != osOK )
			continue; // did we loose a message here. Should this be logged? Can status be not OK but message pointer valid? Then we have memory leak.

		// consume message
		if( compare_mqtt_payload( message , "int" , true ) )
		{
			int32_t temp = 0;
			calc_ADC_temp_int( &temp );

			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_TEMP_INT_ID , &temp , sizeof( temp ) , 0 , 0 , NULL , NULL );
		}
		else if( compare_mqtt_payload( message , "reduced" , true ) )
		{
			int32_t temp = 0;
			calc_ADC_temp_reduced_div( &temp );

			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_TEMP_INT_ID , &temp , sizeof( temp ) , 0 , 0 , NULL , NULL );
		}
		else if( compare_mqtt_payload( message , "float" , true ) )
		{
			float temp = 0;
			calc_ADC_temp_float( &temp );

			mqtt_publish_wrapper( mqtt_data.client , MQTT_PUB_TEMP_FLOAT_ID , &temp , sizeof( temp ) , 0 , 0 , NULL , NULL );
		}

		// free message
		osMemoryPoolFree( mqtt_data.os_memory_pool , message );
	}
	osThreadExit();
}


void eth_task( void* argument )
{
	UNUSED( argument );


	// connect to mqtt
	while(mqtt_data.connected != true )
		osDelay( 100 );

	sub_topic_id_t topic_id;
	osMessageQueueId_t mqtt_queue = osMessageQueueNew( MQTT_OS_QUEUE_NUM_ELEMENTS , sizeof(mqtt_os_message_t*) , NULL );
	vQueueAddToRegistry( mqtt_queue , "mqtt_lwip" );
	mqtt_sub_topic( MQTT_SUB_ETH_TEST_ID , mqtt_queue , &topic_id );

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* message = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_queue , &message , NULL , osWaitForever );

		if( message == NULL || status != osOK )
			continue; // did we loose a message here. Should this be logged? Can status be not OK but message pointer valid? Then we have memory leak.

		// Copy message locally so we can free it. The test might take a while.
		mqtt_os_message_t local_message;
		memcpy( local_message.data , message->data , MQTT_PAYLOAD_MAX_SIZE );
		local_message.len = message->len;

		osMemoryPoolFree( mqtt_data.os_memory_pool , message );

		if( compare_mqtt_payload( &local_message , "tcp simple" , true ) )
		{
			tcp_simple_set_up();
			tcp_simple_loopback( mqtt_queue );
			tcp_simple_destroy();
		}
		else if( compare_mqtt_payload( &local_message , "udp" , true ) )
		{
			udp_set_up();
			udp_tx_datahose( mqtt_queue );
			udp_destroy();
		}
		else if( compare_mqtt_payload( &local_message , "tcp multi" , true ) )
		{
			tcp_multi_set_up();
			tcp_multi_loopback( mqtt_queue );
			tcp_multi_destroy();
		}


	}
	osThreadExit();
}


/* USER CODE END Application */

