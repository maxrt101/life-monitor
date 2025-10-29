/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  LL_RTC_InitTypeDef RTC_InitStruct = {0};

  /* Peripheral clock enable */
  LL_RCC_EnableRTC();

  /* RTC interrupt Init */
  NVIC_SetPriority(RTC_IRQn, 0);
  NVIC_EnableIRQ(RTC_IRQn);

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC and set the Time and Date
  */
  RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
  RTC_InitStruct.AsynchPrescaler = 127;
  RTC_InitStruct.SynchPrescaler = 255;
  LL_RTC_Init(RTC, &RTC_InitStruct);

  /** Initialize RTC and set the Time and Date
  */

  /** Enable the WakeUp
  */
  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_2);
  /* USER CODE BEGIN RTC_Init 2 */

  LL_RTC_WaitForSynchro(RTC);

  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_WAKEUP_Disable(RTC);

  while (LL_RTC_IsActiveFlag_WUTW(RTC) == 0) {}

  CLEAR_BIT(RTC->ISR, RTC_ISR_WUTF);

  while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1) {}

  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_2);
  LL_RTC_WAKEUP_SetAutoReload(RTC, 1638);

  // Without this WUP IRQ doesn't work
  LL_EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_20;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; //it_or_event
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
  LL_EXTI_Init(&EXTI_InitStruct);

  LL_RTC_EnableIT_WUT(RTC);
  LL_RTC_WAKEUP_Enable(RTC);
  LL_RTC_EnableWriteProtection(RTC);
  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
