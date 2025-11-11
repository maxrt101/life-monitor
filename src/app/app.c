/** ========================================================================= *
*
 * @file app.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor Application
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "app/app.h"
#include "error/assertion.h"
#include "log/log.h"

/* Defines ================================================================== */
#define LOG_TAG app

/* Macros =================================================================== */
/**
 * Check if __expr is E_OK, if not - set __flag into app->flags
 *
 * @param __expr Expression to check
 * @param __flag Flag to set if __expr is not E_OK
 */
#define ERR_CHECK_SET_FLAG(__expr, __flag)                                    \
  do {                                                                        \
    error_t err = (__expr);                                                   \
    if (err != E_OK) {                                                        \
      app_set_flag(app, __flag);                                              \
    }                                                                         \
  } while (0)


/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
__STATIC_INLINE void init_pulse(app_t * app, i2c_t * i2c) {
  ERR_CHECK_SET_FLAG(
    max3010x_init(
      &app->pulse.max3010x,
      &(max3010x_cfg_t){
        .i2c = i2c,
        .adc_range.max30102 = MAX30102_ADC_RANGE_2K_nA,
        .pulse_width = {
          .max30100 = MAX30100_PULSE_WIDTH_1600_ADC_16_BIT,
          .max30102 = MAX30102_PULSE_WIDTH_118_ADC_16_BIT,
        },
        .sample_rate = {
          .max30100 = MAX30100_SAMPLE_RATE_100_HZ,
          .max30102 = MAX30102_SAMPLE_RATE_100_HZ,
        },
        .current = {
          .ir  = 50,
          .red = 50
        },
        .mode = MAX3010X_MODE_HEART_RATE
      }
    ),
    APP_FLAG_PULSE_SENSOR_FAILURE
  );

  ERR_CHECK_SET_FLAG(
    pulse_init(
      &app->pulse.ctx,
      max3010x_get_min_ir_adc_voltage(&app->pulse.max3010x),
      500
    ),
    APP_FLAG_PULSE_SENSOR_FAILURE
  );
}

__STATIC_INLINE void init_pos(app_t * app, i2c_t * i2c) {
  ERR_CHECK_SET_FLAG(
    mpu6050_init(
      &app->pos.mpu6050,
      &(mpu6050_cfg_t) {
        .i2c   = i2c,
        .gyro  = MPU6050_GYRO_FS_SEL_1000_DEG_PER_S,
        .accel = MPU6050_ACCEL_AFS_SEL_16G,
      }
    ),
    APP_FLAG_ACCEL_SENSOR_FAILURE
  );

  ERR_CHECK_SET_FLAG(
    acceleration_monitor_init(&app->pos.monitor),
    APP_FLAG_ACCEL_SENSOR_FAILURE
  );
}

__STATIC_INLINE void init_gps(app_t * app, uint8_t uart_no) {
  ERR_CHECK_SET_FLAG(
    uart_init(&app->gps.uart, &(uart_cfg_t){ .uart_no = uart_no }),
    APP_FLAG_GPS_FAILURE
  );

  ERR_CHECK_SET_FLAG(
    uart_set_baudrate(app->gps.uart, 9600),
    APP_FLAG_GPS_FAILURE
  );
}

/* Shared functions ========================================================= */
error_t app_init(app_t * app, app_cfg_t * cfg) {
  ASSERT_RETURN(app, E_NULL);

  memset(app, 0, sizeof(app_t));

  init_pulse(app, cfg->pulse_i2c);
  init_pos(app, cfg->accel_i2c);
  init_gps(app, cfg->gps_uart_no);

  return E_OK;
}

error_t app_set_flag(app_t * app, app_flags_t flag) {
  ASSERT_RETURN(app, E_NULL);

  app->flags |= flag;

  return E_OK;
}

error_t app_clear_flag(app_t * app, app_flags_t flag) {
  ASSERT_RETURN(app, E_NULL);

  app->flags &= ~flag;

  return E_OK;
}

bool app_get_flag(app_t * app, app_flags_t flag) {
 ASSERT_RETURN(app, false);

 return app->flags & flag > 0;
}

error_t app_pulse_process(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  if (app_get_flag(app, APP_FLAG_PULSE_SENSOR_FAILURE)) {
    return E_FAILED;
  }

  max3010x_poll_irq_flags(&app->pulse.max3010x);

  if (max3010x_process(&app->pulse.max3010x) == MAX3010X_STATUS_SAMPLES_READY) {
    // led_off(&app->board.leds[BSP_LED_MAIN]);

    size_t size = PULSE_SAMPLE_COUNT;

    if (max3010x_read_samples(&app->pulse.max3010x, app->pulse.samples, &size) == E_OK) {
      bool beat = false;

      for (size_t i = 0; i < size; ++i) {
        beat |= pulse_process_sample(&app->pulse.ctx, (int32_t) app->pulse.samples[i].ir) == E_OK;
      }
      // pulse_report_bpm(&app->pulse.ctx);

      // if (beat) {
      //   led_on(&app->board.leds[BSP_LED_MAIN]);
      // }
    }
    return E_OK;
  }

  return E_AGAIN;
}

error_t app_pos_process(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  if (app_get_flag(app, APP_FLAG_ACCEL_SENSOR_FAILURE)) {
    return E_FAILED;
  }

  if (mpu6050_measure(&app->pos.mpu6050, &app->pos.sample) == E_OK) {
    // led_off(&app->board.leds[BSP_LED_MAIN]);

    // log_printf("accel {x=%d y=%d z=%d}; gyro {x=%d y=%d z=%d}\r\n",
    //   data.accel.x, data.accel.y, data.accel.z,
    //   data.gyro.x,  data.gyro.y,  data.gyro.z
    // );

    acceleration_pos_t sample = {
      .x = app->pos.sample.accel.x,
      .y = app->pos.sample.accel.y,
      .z = app->pos.sample.accel.z
    };

    if (acceleration_monitor_process_sample(&app->pos.monitor, &sample) == ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED) {
      // led_on(&app->board.leds[BSP_LED_MAIN]);
      return E_OK;
    }

    return E_EMPTY;
  }

  return E_AGAIN;
}

error_t app_gps_process(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  if (app_get_flag(app, APP_FLAG_GPS_FAILURE)) {
    return E_FAILED;
  }

  ASSERT_RETURN(uart_available(app->gps.uart), E_AGAIN);

  TIMEOUT_CREATE(t, 0);

  uint8_t byte = 0;
  if (uart_recv(app->gps.uart, &byte, 1, &t) != E_OK) {
    return E_AGAIN;
  }

  if (byte == '\r') {
    return E_AGAIN;
  }

  if (byte == '\n') {
    app->gps.buffer[app->gps.index] = '\0';
  } else {
    app->gps.buffer[app->gps.index++] = byte;
    return E_AGAIN;
  }

#if 0
  // log_printf("[%d]: %s\r\n", app->gps.index, app->gps.buffer);
  for (uint8_t i = 0; i < app->gps.index; ++i) {
    log_printf("  %d: %d '%c'\r\n", i, app->gps.buffer[i], app->gps.buffer[i]);
  }
#endif

  error_t err = gps_parse(&app->gps.last_location, app->gps.buffer, app->gps.index);;

  app->gps.index = 0;

  return err;
}
