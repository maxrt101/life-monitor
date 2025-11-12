/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#define LED1_Pin LL_GPIO_PIN_0
#define LED1_GPIO_Port GPIOC
#define LED2_Pin LL_GPIO_PIN_1
#define LED2_GPIO_Port GPIOC
#define LED3_Pin LL_GPIO_PIN_2
#define LED3_GPIO_Port GPIOC
#define BTN_Pin LL_GPIO_PIN_3
#define BTN_GPIO_Port GPIOC
#define GPS_UART_TX_Pin LL_GPIO_PIN_2
#define GPS_UART_TX_GPIO_Port GPIOA
#define GPS_UART_RX_Pin LL_GPIO_PIN_3
#define GPS_UART_RX_GPIO_Port GPIOA
#define TRX_SPI_CS_Pin LL_GPIO_PIN_4
#define TRX_SPI_CS_GPIO_Port GPIOA
#define TRX_SPI_SCK_Pin LL_GPIO_PIN_5
#define TRX_SPI_SCK_GPIO_Port GPIOA
#define TRX_SPI_MISO_Pin LL_GPIO_PIN_6
#define TRX_SPI_MISO_GPIO_Port GPIOA
#define TRX_SPI_MOSI_Pin LL_GPIO_PIN_7
#define TRX_SPI_MOSI_GPIO_Port GPIOA
#define ACCEL_I2C_SCL_Pin LL_GPIO_PIN_13
#define ACCEL_I2C_SCL_GPIO_Port GPIOB
#define ACCEL_I2C_SDA_Pin LL_GPIO_PIN_14
#define ACCEL_I2C_SDA_GPIO_Port GPIOB
#define TRX_DIO0_Pin LL_GPIO_PIN_9
#define TRX_DIO0_GPIO_Port GPIOC
#define TRX_DIO0_EXTI_IRQn EXTI4_15_IRQn
#define TRX_RESET_Pin LL_GPIO_PIN_8
#define TRX_RESET_GPIO_Port GPIOA
#define PULSE_I2C_SCL_Pin LL_GPIO_PIN_9
#define PULSE_I2C_SCL_GPIO_Port GPIOA
#define PULSE_I2C_SDA_Pin LL_GPIO_PIN_10
#define PULSE_I2C_SDA_GPIO_Port GPIOA
#define SWDIO_Pin LL_GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin LL_GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define LOG_UART_TX_Pin LL_GPIO_PIN_6
#define LOG_UART_TX_GPIO_Port GPIOB
#define LOG_UART_RX_Pin LL_GPIO_PIN_7
#define LOG_UART_RX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
