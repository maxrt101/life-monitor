/** ========================================================================= *
 *
 * @file project.h
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "led/led.h"
#include "btn/btn.h"
#include "shell/shell.h"
#include "hal/spi/spi.h"
#include "drv/trx/trx.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Device context
 */
typedef struct {
  /** TRX SPI Handle */
  spi_t trx_spi;

  led_t leds[BSP_LED_COUNT];

  /** Buttons */
  btn_t btns[BSP_BTN_COUNT];

  /** TRX Driver Handle */
  trx_t trx;

  /** CLI Shell Context */
  shell_t shell;
} device_t;

/* Variables ================================================================ */
/**
 * Global device context handle
 */
extern device_t device;

/* Shared functions ========================================================= */
/**
 * Main function of the project, runs on application startup and does all the work
 */
void project_main(void);

#ifdef __cplusplus
}
#endif
