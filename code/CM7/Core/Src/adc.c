/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include <stdbool.h>
#include "tim.h"
#include "lwl.h"

/* Variables ------------------------------------------------------------------*/
static uint16_t adc_buf0[ADC3_NUM_REGS];
static uint16_t adc_buf1[ADC3_NUM_REGS];
static volatile uint16_t* latest_adc_buf = NULL;
/* USER CODE END 0 */

ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

/* ADC3 init function */
void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 2;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  hadc3.Init.Oversampling.Ratio = 8;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspInit 0 */

  /* USER CODE END ADC3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    PeriphClkInitStruct.PLL2.PLL2N = 25;
    PeriphClkInitStruct.PLL2.PLL2P = 4;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC3 clock enable */
    __HAL_RCC_ADC3_CLK_ENABLE();

    /* ADC3 DMA Init */
    /* ADC3 Init */
    hdma_adc3.Instance = DMA1_Stream0;
    hdma_adc3.Init.Request = DMA_REQUEST_ADC3;
    hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc3.Init.Mode = DMA_CIRCULAR;
    hdma_adc3.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc3) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc3);

    /* ADC3 interrupt Init */
    HAL_NVIC_SetPriority(ADC3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC3_IRQn);
  /* USER CODE BEGIN ADC3_MspInit 1 */

  /* USER CODE END ADC3_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspDeInit 0 */

  /* USER CODE END ADC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC3_CLK_DISABLE();

    /* ADC3 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    /* ADC3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC3_IRQn);
  /* USER CODE BEGIN ADC3_MspDeInit 1 */

  /* USER CODE END ADC3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
static void adc_publish_buffer( uint16_t* buf )
{
	static bool adc_primed = false;

	if( !adc_primed )
	{
		adc_primed = true;     // discard first transfer
		return;
	}

	latest_adc_buf = buf;
}

void DMA_M0_Callback( DMA_HandleTypeDef* hdma )
{
	UNUSED( hdma );
	adc_publish_buffer( adc_buf0 );
}

void DMA_M1_Callback( DMA_HandleTypeDef* hdma )
{
	UNUSED( hdma );
	adc_publish_buffer( adc_buf1 );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef* hadc )
{
	UNUSED( hadc );
}

void start_ADC_DMA()
{
	hadc3.DMA_Handle->XferCpltCallback = DMA_M0_Callback;
	hadc3.DMA_Handle->XferM1CpltCallback = DMA_M1_Callback;

	// Start DMA once
	HAL_DMAEx_MultiBufferStart_IT( hadc3.DMA_Handle , (uint32_t) &ADC3->DR , (uint32_t) adc_buf0 , (uint32_t) adc_buf1 , ADC3_NUM_REGS );

	HAL_ADC_Start( &hadc3 );

	// Start timer
	HAL_TIM_Base_Start( &htim2 );
}

HAL_StatusTypeDef calc_ADC_temp_reduced_div( int64_t* const result )
{
	lwl_enter_record( ADC_LWL_ID , ADC_REDUCED_DIV_LWL_ID , "" );

	// No adc readings yet
	if( latest_adc_buf == NULL )
		return HAL_ERROR;

	// get raw adc measurements and calculate Celsius
	uint16_t* adc_buf = (uint16_t*) latest_adc_buf;

	// calculate constants
	const uint32_t TEMP_NUM = ( TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP ) * 1000;
	const uint32_t TEMP_OFF = TEMPSENSOR_CAL1_TEMP * 1000;
	// use q2.30 fixed point reciprocate to avoid the division
	const uint32_t TEMP_RECIP_Q30 = (uint32_t) ( ( (uint64_t) 1 << 30 ) / ( *TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR ) );

	// calculate temp in milliC
	uint32_t temp = (uint32_t) ( ( (uint64_t) adc_buf[TEMP_REG] * ( *VREFINT_CAL_ADDR ) ) / adc_buf[VDD_REG] );
	int64_t temp_milli = ( ( (int64_t) ( temp - ( *TEMPSENSOR_CAL1_ADDR ) ) * TEMP_NUM * TEMP_RECIP_Q30 ) >> 30 ) + TEMP_OFF;

	// sensor is noisy, smooth the results.
	static int64_t filtered = 0;
	if( filtered == 0 )
		filtered = temp_milli;
	else
		filtered = ( filtered * 7 + temp_milli ) / 8;

	*result = filtered;
	return HAL_OK;
}

HAL_StatusTypeDef calc_ADC_temp_int( int64_t* const result )
{
	lwl_enter_record( ADC_LWL_ID , ADC_INT_LWL_ID , "" );

	// No adc readings yet
	if( latest_adc_buf == NULL )
		return HAL_ERROR;

	// get raw adc measurements and calculate Celsius
	uint16_t* adc_buf = (uint16_t*) latest_adc_buf;

	// calculate temp in milliC
	int64_t temp_milli = ( ( ( (int64_t) adc_buf[TEMP_REG] * ( *VREFINT_CAL_ADDR ) ) / adc_buf[VDD_REG] - ( *TEMPSENSOR_CAL1_ADDR ) )
	        * ( TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP ) * 1000 ) / ( *TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR )
	        + ( (int64_t) TEMPSENSOR_CAL1_TEMP * 1000 );

	// sensor is noisy, smooth the results.
	static int64_t filtered = 0;
	if( filtered == 0 )
		filtered = temp_milli;
	else
		filtered = ( filtered * 7 + temp_milli ) / 8;

	*result = filtered;
	return HAL_OK;
}

HAL_StatusTypeDef calc_ADC_temp_float( float* const result )
{
	lwl_enter_record( ADC_LWL_ID , ADC_FLOAT_LWL_ID , "" );

	// No adc readings yet
	if( latest_adc_buf == NULL )
		return HAL_ERROR;

	// get raw adc measurements and calculate Celsius
	uint16_t* adc_buf = (uint16_t*) latest_adc_buf;

	// calculate temp
	float ratio = (float) adc_buf[TEMP_REG] / (float) adc_buf[VDD_REG];
	float temp = ( ratio * (float) *VREFINT_CAL_ADDR - (float) *TEMPSENSOR_CAL1_ADDR )
	        * ( (float) ( TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP ) / (float) ( *TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR ) )+
	        (float)TEMPSENSOR_CAL1_TEMP;

	// sensor is noisy, smooth the results.
	static float filtered = 0.0f;
	if( filtered == 0.0f )
		filtered = temp;
	else
		filtered = ( filtered * 7.0f + temp ) / 8.0f;

	*result = filtered;
	return HAL_OK;
}

/* USER CODE END 1 */

