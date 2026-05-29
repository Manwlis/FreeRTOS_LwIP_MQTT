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
#include "settings.h"
#include "stm32h7xx_it.h" // for the RUN_TIME_STATS

#if CURRENT_TEST == UDP_TX_BENCHMARK
#include "udp_test.h"
#elif CURRENT_TEST == TCP_LOOPBACK
#include "tcp_loopback_simple.h"
#elif CURRENT_TEST == TCP_LOOPBACK_MULTITASK
#include "tcp_loopback_multitask.h"
#endif

#include "mqtt_client.h"
#include "i2c.h"
#include "LIS3DHTR.h"
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
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
osThreadId_t tx_task_handle;
const osThreadAttr_t tx_task_attributes = { .name = "tx_task" , .stack_size = 256 * 4 , .priority = (osPriority_t) osPriorityNormal1 , };
#endif

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
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

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
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
	set_up_queues();
#endif
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
#if CURRENT_TEST == TCP_LOOPBACK_MULTITASK
	tx_task_handle = osThreadNew( StartTxTask , NULL , &tx_task_attributes );
#endif
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

	LIS3DHTR_device_t LIS3DHTR_handle = LIS3DHTR_create_handle( (void*) &hi2c4 , 0x19 );
	hi2c4_wrapper.task_handle = osThreadGetId();

	lwl_init();

	mqtt_init();
//	mqtt_test();

	while(1)
	{
		// wait for queue message
		mqtt_os_message_t* msg = NULL;
		osStatus_t status = osMessageQueueGet( mqtt_sub_topics[MQTT_TOPIC_SENSOR_ALS].os_queue_id , &msg, NULL, osWaitForever);

		printf("osMessageQueueGet status = %d\n" , status );
		if( msg == NULL )
			for(;;);

		printf("%s %lu\n\n" , (char*)(msg->data) , msg->len );

		osMemoryPoolFree( mqtt_os_message_pool , msg );

		// choose correct function
	}



	osThreadExit();
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
#if ( CURRENT_TEST == TCP_LOOPBACK_MULTITASK )
void StartTxTask( void* argument )
{
	UNUSED( argument );

	tcp_tx();
	osThreadFlagsSet( defaultTaskHandle , 0x0001U );
	osThreadExit();
}
#endif
/* USER CODE END Application */

