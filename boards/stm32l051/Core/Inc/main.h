/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_ll_iwdg.h"
#include "stm32l0xx_ll_crs.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"
#include "stm32l0xx_ll_rtc.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BAT_ADC_Pin LL_GPIO_PIN_0
#define BAT_ADC_GPIO_Port GPIOA
#define LORA_CS_Pin LL_GPIO_PIN_4
#define LORA_CS_GPIO_Port GPIOA
#define LORA_SCK_Pin LL_GPIO_PIN_5
#define LORA_SCK_GPIO_Port GPIOA
#define LORA_MISO_Pin LL_GPIO_PIN_6
#define LORA_MISO_GPIO_Port GPIOA
#define LORA_MOSI_Pin LL_GPIO_PIN_7
#define LORA_MOSI_GPIO_Port GPIOA
#define LORA_DIO0_Pin LL_GPIO_PIN_0
#define LORA_DIO0_GPIO_Port GPIOB
#define LORA_DIO0_EXTI_IRQn EXTI0_1_IRQn
#define LORA_RST_Pin LL_GPIO_PIN_1
#define LORA_RST_GPIO_Port GPIOB
#define LED_GREEN_Pin LL_GPIO_PIN_15
#define LED_GREEN_GPIO_Port GPIOA
#define LED_R_Pin LL_GPIO_PIN_3
#define LED_R_GPIO_Port GPIOB
#define LED_G_Pin LL_GPIO_PIN_4
#define LED_G_GPIO_Port GPIOB
#define LED_B_Pin LL_GPIO_PIN_5
#define LED_B_GPIO_Port GPIOB
#define BUTTON_Pin LL_GPIO_PIN_8
#define BUTTON_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
