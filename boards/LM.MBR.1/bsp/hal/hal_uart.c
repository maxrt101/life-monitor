/** ========================================================================= *
 *
 * @file hal_uart.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK UART HAL API
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "hal/uart/uart.h"
#include "error/assertion.h"
#include "stm32l073xx.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_rcc.h"
#include "usart.h"
#include "time/sleep.h"

/* Defines ================================================================== */
#define LOG_TAG hal_uart

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
error_t uart_init(uart_t ** uart, uart_cfg_t * cfg) {
  ASSERT_RETURN(uart && cfg, E_NULL);

  switch (cfg->uart_no) {
    case 1:
      *uart = USART1;
      MX_USART1_UART_Init();
      break;
    case 2:
      *uart = USART2;
      MX_USART2_UART_Init();
      break;
    default:
      return E_INVAL;
  }

  return E_OK;
}

error_t uart_deinit(uart_t * uart) {
  ASSERT_RETURN(uart, E_NULL);
  USART_TypeDef * handle = (USART_TypeDef *) uart;
  LL_USART_DeInit(handle);
  return E_OK;
}

error_t uart_set_baudrate(uart_t * uart, uint32_t baudrate) {
  ASSERT_RETURN(uart, E_NULL);

  LL_RCC_ClocksTypeDef rcc_clocks;
  LL_RCC_GetSystemClocksFreq(&rcc_clocks);

  USART_TypeDef * handle = (USART_TypeDef *) uart;

  LL_USART_Disable(handle);

  LL_USART_SetBaudRate(
        handle, rcc_clocks.PCLK1_Frequency, LL_USART_OVERSAMPLING_16, baudrate);

  LL_USART_Enable(handle);

  return E_OK;
}

error_t uart_reset(uart_t * uart) {
  ASSERT_RETURN(uart, E_NULL);

  ERROR_CHECK_RETURN(uart_deinit(uart));

  USART_TypeDef * handle = (USART_TypeDef *) uart;

  if (handle == USART1) {
    MX_USART1_UART_Init();
  } else if (handle == USART2) {
    MX_USART2_UART_Init();
  } else {
    return E_INVAL;
  }

  return E_OK;
}

bool uart_available(uart_t * uart) {
  ASSERT_RETURN(uart, E_NULL);
  USART_TypeDef * handle = (USART_TypeDef *) uart;
  return LL_USART_IsActiveFlag_RXNE(handle);
}

error_t uart_send(uart_t * uart, const uint8_t * buf, size_t size) {
  ASSERT_RETURN(uart && buf, E_NULL);

  USART_TypeDef * handle = (USART_TypeDef *) uart;

  for (size_t i = 0; i < size; i++) {
    LL_USART_TransmitData8(handle, buf[i]);

    while (!LL_USART_IsActiveFlag_TXE(handle)) {
      // TODO: Static timeout
    }
  }

  while (!LL_USART_IsActiveFlag_TC(handle)) {}

  LL_USART_ClearFlag_TC(handle);

  return E_OK;
}

error_t uart_recv(uart_t * uart, uint8_t * buf, size_t size, timeout_t * timeout) {
  ASSERT_RETURN(uart && buf, E_NULL);

  USART_TypeDef * handle = (USART_TypeDef *) uart;

  for (size_t i = 0; i < size; ++i) {
    while (!LL_USART_IsActiveFlag_RXNE(handle)) {
      if (timeout && timeout_is_expired(timeout)) {
        return E_TIMEOUT;
      }
    }

    buf[i] = LL_USART_ReceiveData8(handle);
  }

  LL_USART_ClearFlag_ORE(handle);
  LL_USART_ClearFlag_RTO(handle);

  return E_OK;
}
