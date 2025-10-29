/** ========================================================================= *
 *
 * @file hal_i2c.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK I2C HAL API
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "hal/i2c/i2c.h"
#include "error/assertion.h"
#include "stm32l051xx.h"
#include "stm32_platform.h"
#include "i2c.h"

/* Defines ================================================================== */
#define LOG_TAG hal_i2c

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
error_t i2c_init(i2c_t * i2c, i2c_cfg_t * cfg) {
  ASSERT_RETURN(i2c && cfg, E_NULL);

  switch (cfg->i2c_no) {
    case 1:
      i2c->handle = &hi2c1;
      MX_I2C1_Init();
      HAL_I2C_MspInit(&hi2c1);
      break;
    default:
      return E_INVAL;
  }

  return E_OK;
}

error_t i2c_deinit(i2c_t * i2c) {
  ASSERT_RETURN(i2c, E_NULL);

  I2C_HandleTypeDef * handle = i2c->handle;

  STM32_HAL_ERROR_CHECK_RETURN(HAL_I2C_DeInit(handle));

  return E_OK;
}

error_t i2c_send(i2c_t * i2c, uint16_t addr, uint8_t * data, size_t size) {
  ASSERT_RETURN(i2c && data, E_NULL);

  I2C_HandleTypeDef * handle = i2c->handle;

  STM32_HAL_ERROR_CHECK_RETURN(
    HAL_I2C_Master_Transmit(handle, addr, data, size, BSP_I2C_RECV_TIMEOUT)
  );

  return E_OK;
}

error_t i2c_recv(i2c_t * i2c, uint16_t addr, uint8_t * data, size_t size) {
  ASSERT_RETURN(i2c && data, E_NULL);

  I2C_HandleTypeDef * handle = i2c->handle;

  STM32_HAL_ERROR_CHECK_RETURN(
    HAL_I2C_Master_Receive(handle, addr, data, size, BSP_I2C_RECV_TIMEOUT)
  );

  return E_OK;
}
