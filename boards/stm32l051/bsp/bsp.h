/** ========================================================================= *
 *
 * @file bsp.h
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "main.h"
#include "vfs/vfs.h"
#include "led/led.h"
#include "btn/btn.h"
#include "i2c/i2c.h"
#include "spi/spi.h"

/* Defines ================================================================== */
/* RA-02 GPIO Defines */
#define BSP_LORA_CS GPIO_BIND(LORA_CS)
#define BSP_LORA_DIO0 GPIO_BIND(LORA_DIO0)
#define BSP_LORA_RESET GPIO_BIND(LORA_RST)

/* Default stacktrace depth */
#define BSP_STACKTRACE_DEPTH 16

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Peripherals
 */
typedef struct {
  /** SPI Handle for SX1278 */
  spi_t trx_spi;

  /** Unused SPI Handle */
  spi_t free_spi;

  /** I2C Handle */
  i2c_t i2c;

  /** Handles for all LEDs present */
  led_t leds[BSP_LED_COUNT];

  /** Handles for all buttons present */
  btn_t btns[BSP_BTN_COUNT];
} board_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initialize board
 *
 * @param board Board context
 */
void bsp_init(board_t * board);

/**
 * Initialize VFS files
 */
error_t bsp_init_vfs_files(vfs_t * vfs);

/**
 * Returns true if button is pressed
 */
bool bsp_is_btn_pressed(void);

/**
 * Sets period, which when expired will wake up the MCU
 */
void bsp_set_next_wakeup(milliseconds_t period);

/**
 * Returns current wakeup period in ms
 */
milliseconds_t bsp_get_wakeup(void);

/**
 * Calculates VrefInt
 */
error_t bsp_adc_get_vrefint(uint32_t * mv);

/**
 * Calculates Internal Temperature
 */
error_t bsp_adc_get_temp(int32_t * temp);

/**
 * Calculates Battery Voltage
 */
error_t bsp_adc_get_vbat(uint32_t * mv);

/**
 * Prints stacktrace, given stack pointer and max depth
 *
 * Use `arm-none-eabi-addr2line -e <ELF> <ADDR>` to print source line (can be unreliable)
 */
void bsp_print_stacktrace(uint32_t * sp, uint32_t depth);

#ifdef __cplusplus
}
#endif

