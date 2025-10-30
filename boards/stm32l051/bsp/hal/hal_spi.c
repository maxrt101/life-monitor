/** ========================================================================= *
 *
 * @file hal_spi.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK SPI HAL API
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "hal/spi/spi.h"
#include "error/assertion.h"
#include "time/timeout.h"
#include "stm32l051xx.h"
#include "stm32l0xx_ll_spi.h"
#include "spi.h"

/* Defines ================================================================== */
#define LOG_TAG hal_spi

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
error_t spi_init(spi_t * spi, spi_cfg_t * cfg) {
  ASSERT_RETURN(spi && cfg, E_NULL);

  spi->cs = cfg->cs;

  switch (cfg->spi_no) {
    case 1:
      spi->handle = SPI1;
      MX_SPI1_Init();
      break;
    case 2:
      spi->handle = SPI2;
      MX_SPI2_Init();
      break;
    default:
      return E_INVAL;
  }

  /* FIXME: On GS.MBR.0.1 first read on SPI is out of sync
   *        (clock starts sooner that MOSI line is pulled down) which
   *        could cause a bad value to be read
   *        Read dummy data at initialization to prevent this
   */
#if USE_SPI_FIRST_READ_OUT_OF_SYNC_FIX
  uint8_t dummy_tx_data[2] = {0};
  uint8_t dummy_rx_data[2] = {0};
  spi_select(spi);
  spi_send_recv(spi, dummy_tx_data, 2, dummy_rx_data, 2);
  spi_unselect(spi);
#endif

  return E_OK;
}

error_t spi_deinit(spi_t * spi) {
  ASSERT_RETURN(spi, E_NULL);

  SPI_TypeDef * handle = (SPI_TypeDef *) spi;
  LL_SPI_DeInit(handle);
  LL_SPI_Disable(handle);

  return E_OK;
}

error_t spi_select(spi_t * spi) {
  ASSERT_RETURN(spi, E_NULL);
  gpio_clear(spi->cs);
  return E_OK;
}

error_t spi_unselect(spi_t * spi) {
  ASSERT_RETURN(spi, E_NULL);
  gpio_set(spi->cs);
  return E_OK;
}

error_t spi_send_recv(
    spi_t * spi,
    uint8_t * tx_buf, size_t tx_size,
    uint8_t * rx_buf, size_t rx_size
) {
  ASSERT_RETURN(spi, E_NULL);

  SPI_TypeDef * handle = (SPI_TypeDef *) spi->handle;

  if (!LL_SPI_IsEnabled(handle)) {
    LL_SPI_Enable(handle);
  }

  size_t remaining_tx = tx_buf ? tx_size : 0;
  size_t remaining_rx = rx_buf  ? rx_size : 0;
  uint8_t * tx_buf_ptr = (uint8_t *) tx_buf;
  uint8_t * rx_buf_ptr = (uint8_t *) rx_buf;

  while (remaining_rx || remaining_tx) {
    uint8_t data = 0;

    if (remaining_tx) {
      data = *tx_buf_ptr++;
      remaining_tx--;
    }

    LL_SPI_TransmitData8(handle, data);

    TIMEOUT_CREATE(t, BSP_SPI_RECV_TIMEOUT);

    while (!LL_SPI_IsActiveFlag_TXE(handle) || !LL_SPI_IsActiveFlag_RXNE(handle)) {
      if (timeout_is_expired(&t)) {
        return E_NORESP;
      }
    }

    data = LL_SPI_ReceiveData8(handle);

    if (remaining_rx) {
      *rx_buf_ptr++ = data;
      remaining_rx--;
    }
  }

  /* Clear flags. */
  LL_SPI_ClearFlag_CRCERR(handle);
  LL_SPI_ClearFlag_OVR(handle);
  LL_SPI_ClearFlag_FRE(handle);

  return E_OK;
}
