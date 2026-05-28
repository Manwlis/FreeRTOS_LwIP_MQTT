/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c4;

/* USER CODE BEGIN Private defines */
#if LIS3DHTR_OS == FREE_RTOS
#include "cmsis_os2.h"

// The public HAL API IT functions expect one outstanding transfer per peripheral.
// To ensure this, all transactions of a hi2c will be facilitated by a single task.
// It is expected that the task that initializes the peripheral is the one that handles it too.
typedef struct _hi2c_freertos_wrapper_t
{
	I2C_HandleTypeDef* const hi2c;
	osThreadId_t task_handle;
}hi2c_freertos_wrapper_t;

extern hi2c_freertos_wrapper_t hi2c4_wrapper;
#endif
/* USER CODE END Private defines */

void MX_I2C4_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

