/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "atomic/atomic.h"
#include "error/assertion.h"
#include "error/error.h"
#include "time/sleep.h"
#include "log/log.h"

#define LOG_TAG bsp_adc

/** ADC_BLOCK state */
typedef enum {
  ADC_STATE_BEGIN = 0,
  ADC_STATE_END
} adc_state_t;

/** Same as ATOMIC_BLOCK in principle */
#define ADC_BLOCK(ch) \
    for (adc_state_t s = adc_begin(ch); s != ADC_STATE_END; s = adc_end())

/* USER CODE END 0 */

/* ADC init function */
void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_InitTypeDef ADC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**ADC GPIO Configuration
  PA0   ------> ADC_IN0
  */
  GPIO_InitStruct.Pin = BAT_ADC_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(BAT_ADC_GPIO_Port, &GPIO_InitStruct);

  /* ADC interrupt Init */
  NVIC_SetPriority(ADC1_COMP_IRQn, 0);
  NVIC_EnableIRQ(ADC1_COMP_IRQn);

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_0);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_TEMPSENSOR);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_TEMPSENSOR);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_VREFINT);
  LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT);

  /** Common config
  */
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
  ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_79CYCLES_5);
  LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);
  LL_ADC_REG_SetSequencerScanDirection(ADC1, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);
  LL_ADC_SetCommonFrequencyMode(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_FREQ_MODE_HIGH);
  LL_ADC_DisableIT_EOC(ADC1);
  LL_ADC_DisableIT_EOS(ADC1);
  ADC_InitStruct.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);

  /* Enable ADC internal voltage regulator */
  LL_ADC_EnableInternalRegulator(ADC1);
  /* Delay for ADC internal voltage regulator stabilization. */
  /* Compute number of CPU cycles to wait for, from delay in us. */
  /* Note: Variable divided by 2 to compensate partially */
  /* CPU processing cycles (depends on compilation optimization). */
  /* Note: If system core clock frequency is below 200kHz, wait time */
  /* is only a few CPU processing cycles. */
  uint32_t wait_loop_index;
  wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  /* USER CODE BEGIN ADC_Init 2 */

  // Calibrate ADC
  LL_ADC_StartCalibration(ADC1);
  while (LL_ADC_IsCalibrationOnGoing(ADC1))
    ;

  /* USER CODE END ADC_Init 2 */

}

/* USER CODE BEGIN 1 */
static void adc_init(void)
{
  LL_ADC_EnableInternalRegulator(ADC1);

  sleep_us(LL_ADC_DELAY_INTERNAL_REGUL_STAB_US);

  while (!LL_PWR_IsActiveFlag_VREFINTRDY()) {
    // TODO: Add timeout
  }

  LL_ADC_Enable(ADC1);

  while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {
    // TODO: Add timeout
  }
}

static adc_state_t adc_begin(uint32_t ch) {
  // LL_ADC_ClearFlag_OVR(ADC1);

  LL_ADC_REG_SetSequencerChannels(ADC1, ch);
  LL_ADC_REG_StartConversion(ADC1);

  while (!LL_ADC_IsActiveFlag_EOC(ADC1)) {
    // TODO: Add timeout
  }

  return ADC_STATE_BEGIN;
}

static adc_state_t adc_end(void) {
  LL_ADC_REG_StopConversion(ADC1);

  while (LL_ADC_REG_IsStopConversionOngoing(ADC1)) {
    // TODO: Add timeout
  }

  return ADC_STATE_END;
}

error_t bsp_adc_get_vrefint(uint32_t * mv) {
  ASSERT_RETURN(mv, E_NULL);

  uint16_t raw = 0;

  ATOMIC_BLOCK() {
    adc_init();

    LL_ADC_SetCommonPathInternalCh(
            __LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT);

    sleep_us(LL_ADC_DELAY_VREFINT_STAB_US);

    ADC_BLOCK(LL_ADC_CHANNEL_VREFINT) {
      raw = LL_ADC_REG_ReadConversionData12(ADC1);
    }

    LL_ADC_SetCommonPathInternalCh(
        __LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);

    *mv = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw, LL_ADC_RESOLUTION_12B);
  }

  log_info("adc: vref=%dmv", *mv);

  return E_OK;
}

error_t bsp_adc_get_temp(int32_t * temp) {
  ASSERT_RETURN(temp, E_NULL);

  uint32_t vref = 0;
  uint16_t raw = 0;

  ATOMIC_BLOCK() {
    adc_init();

    LL_ADC_SetCommonPathInternalCh(
            __LL_ADC_COMMON_INSTANCE(ADC1),
            LL_ADC_PATH_INTERNAL_TEMPSENSOR | LL_ADC_PATH_INTERNAL_VREFINT);

    sleep_us(UTIL_MAX(LL_ADC_DELAY_VREFINT_STAB_US, LL_ADC_DELAY_TEMPSENSOR_STAB_US));

    ADC_BLOCK(LL_ADC_CHANNEL_VREFINT) {
      raw = LL_ADC_REG_ReadConversionData12(ADC1);
    }

    ADC_BLOCK(LL_ADC_CHANNEL_TEMPSENSOR) {
      vref = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw, LL_ADC_RESOLUTION_12B);
      raw = LL_ADC_REG_ReadConversionData12(ADC1);
    }

    LL_ADC_SetCommonPathInternalCh(
            __LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);

    *temp = __LL_ADC_CALC_TEMPERATURE(vref, raw, LL_ADC_RESOLUTION_12B);
  }

  log_info("adc: t=%dC", *temp);

  return E_OK;
}

// FIXME: Calculates lower values then vref (which shouldn't be the case)
//        e.g. - vref=3443 vbat=2198
error_t bsp_adc_get_vbat(uint32_t * mv) {
  ASSERT_RETURN(mv, E_NULL);

  uint32_t vref = 0;
  uint32_t raw = 0;

  ATOMIC_BLOCK() {
    adc_init();

    LL_ADC_SetCommonPathInternalCh(
            __LL_ADC_COMMON_INSTANCE(ADC1),
            LL_ADC_PATH_INTERNAL_VREFINT);

    sleep_us(LL_ADC_DELAY_VREFINT_STAB_US);

    ADC_BLOCK(LL_ADC_CHANNEL_VREFINT) {
      raw = LL_ADC_REG_ReadConversionData12(ADC1);
    }

    ADC_BLOCK(LL_ADC_CHANNEL_0) {
      vref = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw, LL_ADC_RESOLUTION_12B);
      raw = LL_ADC_REG_ReadConversionData12(ADC1);
    }

    LL_ADC_SetCommonPathInternalCh(
            __LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);

    *mv = __LL_ADC_CALC_DATA_TO_VOLTAGE(vref, raw, LL_ADC_RESOLUTION_12B);
  }

  log_info("adc: bat=%dmv", *mv);

  return E_OK;
}
/* USER CODE END 1 */
