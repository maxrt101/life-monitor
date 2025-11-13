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
#include "storage/storage.h"
#include "led/led.h"
#include "net/net.h"
#include <stdbool.h>

/* Defines ================================================================== */
#define PULSE_SAMPLE_COUNT        16
#define PULSE_MIN_ALERT_THRESHOLD 40
#define PULSE_MAX_ALERT_THRESHOLD 180

/* Macros =================================================================== */
/* Enums ==================================================================== */
/**
 * Application Status Flags
 */
typedef enum {
  APP_FLAG_PULSE_SENSOR_FAILURE = (1 << 0),
  APP_FLAG_ACCEL_SENSOR_FAILURE = (1 << 1),
  APP_FLAG_GPS_FAILURE          = (1 << 2),
} app_flags_t;

/* Types ==================================================================== */
/**
 * Application Context
 */
typedef struct {
  /** Application Running Flag */
  bool is_running;

  /** Application Status Flags */
  app_flags_t flags;

  /** Last Reset Reason */
  net_reset_reason_t reset_reason;

  /** Last Reset Counter */
  uint8_t reset_count;

  /** Timeout for sending statuses */
  timeout_t status_send_timeout;

  /** Storage Data */
  storage_data_t storage;

  /** Network Context */
  net_t net;

  /** LED Contexts for various events signalling */
  struct {
    led_t * pulse;
    led_t * trx;
    led_t * error;
  } led;

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

    /** Buffer for MAX30100 Samples */
    max3010x_sample_t samples[PULSE_SAMPLE_COUNT];

    /** Pulse detector */
    pulse_t ctx;
  } pulse;

  struct {
    /** MPU6050 Accelerometer/Gyroscope context */
    mpu6050_t mpu6050;

    /** Buffer for MPU6050 Sample */
    mpu6050_measurement_t sample;

    /** Acceleration monitor */
    acceleration_monitor_t monitor;
  } pos;
} app_t;

/**
 * Application Config
 */
typedef struct {
  /** TRX Context */
  trx_t * trx;

  /** I2C Handle For MAX30100 Pulse Sensor */
  i2c_t  *  pulse_i2c;

  /** I2C Handle For MPU6050 Accelerometer/Gyroscope */
  i2c_t  *  accel_i2c;

  /** UART Number for GPS (NEO6M) */
  uint8_t   gps_uart_no;

  /** LED Contexts for various events signalling */
  struct {
    led_t * pulse;
    led_t * trx;
    led_t * error;
  } led;

  /** Last Reset Reason */
  net_reset_reason_t reset_reason;
} app_cfg_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initialized Application
 *
 * @param app Application Context
 * @param cfg Application Config
 */
error_t app_init(app_t * app, app_cfg_t * cfg);

/**
 * Set Application Flag
 *
 * @param app  Application Context
 * @param flag Flag to set
 */
error_t app_set_flag(app_t * app, app_flags_t flag);

/**
 * Clear Application Flag
 *
 * @param app  Application Context
 * @param flag Flag to clear
 */
error_t app_clear_flag(app_t * app, app_flags_t flag);

/**
 * Get Application Flag
 *
 * @param app  Application Context
 * @param flag Flag to get
 */
bool app_get_flag(app_t * app, app_flags_t flag);

/**
 * Returns true if application is running
 *
 * @param app Application Context
 */
bool app_is_running(app_t * app);

/**
 * Start Application
 *
 * @param app Application Context
 */
error_t app_start(app_t * app);

/**
 * Stops Application
 *
 * @param app Application Context
 */
error_t app_stop(app_t * app);

/**
 * Start registration process
 *
 * @param app Application Context
 */
error_t app_register(app_t * app);

/**
 * Process MAX30100 Pulse sensor data
 *
 * @param app Application Context
 */
error_t app_pulse_process(app_t * app);

/**
 * Process MPU6050 Position/Acceleration sensor data
 *
 * @param app Application Context
 */
error_t app_pos_process(app_t * app);

/**
 * Process NEO6M GPS location
 *
 * @param app Application Context
 */
error_t app_gps_process(app_t * app);

/**
 * Send status
 *
 * @param app Application Context
 */
error_t app_send_status(app_t * app);

/**
 * Send location
 *
 * @param app Application Context
 */
error_t app_send_location(app_t * app);

/**
 * Send alert
 *
 * @param app Application Context
 */
error_t app_send_alert(app_t * app, net_alert_trigger_t trigger);

#ifdef __cplusplus
}
#endif
