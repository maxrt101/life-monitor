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
#include <gps/gps.h>

#include "shell/shell.h"
#include "drv/trx/trx.h"
#include "drv/max3010x/max3010x.h"
#include "drv/mpu6050/mpu6050.h"
#include "hal/uart/uart.h"
#include "sensors/pulse/pulse.h"
#include "sensors/accel/accel.h"
#include "bsp.h"

/* Defines ================================================================== */
#define PULSE_SAMPLE_COUNT 16

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Device context
 */
typedef struct {
  /** Board context. Peripherals, etc. */
  board_t board;

  /** TRX Driver Handle */
  trx_t trx;

  struct {
    /** GPS (neo6m) uart context */
    uart_t * uart;

    /** GPS message buffer & index */
    char    buffer[128];
    uint8_t index;

    /** Last known location */
    gps_location_t last_location;
  } gps;

  struct {
    /** MAX30100 Pulse sensor context */
    max3010x_t max3010x;

    max3010x_sample_t samples[PULSE_SAMPLE_COUNT];

    /** Pulse detector */
    pulse_t ctx;
  } pulse;

  struct {
    /** MPU6050 Accelerometer/Gyroscope context */
    mpu6050_t mpu6050;

    /** Acceleration monitor */
    acceleration_monitor_t monitor;
  } pos;

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
