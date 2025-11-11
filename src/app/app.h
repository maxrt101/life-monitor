/** ========================================================================= *
 *
 * @file app.h
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor Application
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "max3010x/max3010x.h"
#include "mpu6050/mpu6050.h"
#include "uart/uart.h"
#include "sensors/accel/accel.h"
#include "sensors/pulse/pulse.h"
#include "gps/gps.h"
#include "error/error.h"
#include <stdbool.h>

/* Defines ================================================================== */
#define PULSE_SAMPLE_COUNT 16

/* Macros =================================================================== */
/* Enums ==================================================================== */
typedef enum {
  APP_FLAG_PULSE_SENSOR_FAILURE = (1 << 0),
  APP_FLAG_ACCEL_SENSOR_FAILURE = (1 << 1),
  APP_FLAG_GPS_FAILURE          = (1 << 2),
} app_flags_t;

/* Types ==================================================================== */
typedef struct {
  bool is_running;

  app_flags_t flags;

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

    mpu6050_measurement_t sample;

    /** Acceleration monitor */
    acceleration_monitor_t monitor;
  } pos;
} app_t;

typedef struct {
  i2c_t  * pulse_i2c;
  i2c_t  * accel_i2c;
  uint8_t  gps_uart_no;
} app_cfg_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
error_t app_init(app_t * app, app_cfg_t * cfg);

error_t app_set_flag(app_t * app, app_flags_t flag);
error_t app_clear_flag(app_t * app, app_flags_t flag);
bool app_get_flag(app_t * app, app_flags_t flag);

error_t app_pulse_process(app_t * app);
error_t app_pos_process(app_t * app);
error_t app_gps_process(app_t * app);


#ifdef __cplusplus
}
#endif