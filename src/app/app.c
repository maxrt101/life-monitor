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

#include "boards/STM32L051/bsp/bsp.h"
#include "error/assertion.h"
#include "led/led.h"
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

  log_info("Initializing Application");

  memset(app, 0, sizeof(app_t));

  app->led.pulse    = cfg->led.pulse;
  app->led.trx      = cfg->led.trx;
  app->led.error    = cfg->led.error;
  app->reset_reason = cfg->reset_reason;

  uint32_t vrefint = 1488;
  // bsp_adc_get_vrefint(&vrefint);

  net_cfg_t net_cfg = {
    .trx           = cfg->trx,
    .dev_mac.value = PROJECT_DEVICE_MAC,
    .rand_seed     = vrefint,
  };

  if (storage_read(&app->storage) == E_OK) {
    log_info("Registered to 0x%x", app->storage.station_mac.value);

    net_cfg.station_mac.value = app->storage.station_mac.value;
    memcpy(net_cfg.key, app->storage.key, NET_KEY_SIZE);
  } else {
    log_warn("Storage is corrupt");

    net_cfg.station_mac.value = 0;
    memset(net_cfg.key, 0, NET_KEY_SIZE);

    memset(&app->storage, 0, sizeof(storage_data_t));
  }

  app->storage.reset_count += 1;
  storage_write(&app->storage);

  app->reset_count = app->storage.reset_count;

  net_init(&app->net, &net_cfg);

  init_pulse(app, cfg->pulse_i2c);
  init_pos(app, cfg->accel_i2c);
  init_gps(app, cfg->gps_uart_no);

  timeout_start(&app->status_send_timeout, NET_STATUS_SEND_PERIOD);

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

bool app_is_running(app_t * app) {
  ASSERT_RETURN(app, false);

  return app->is_running;
}

error_t app_start(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  app->is_running = true;

  return E_OK;
}

error_t app_stop(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  app->is_running = false;

  return E_OK;
}

error_t app_register(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  log_info("Registration");

  while (1) {
    memset(app->net.key, 0, NET_KEY_SIZE);
    app->net.station_mac.value = 0;

    net_packet_t reg = {0};

    ERROR_CHECK_RETURN(net_packet_init(&app->net, &reg, &(net_packet_cfg_t){
      .cmd          = NET_CMD_REGISTER,
      .transport    = NET_TRANSPORT_TYPE_BROADCAST,
      .target.value = 0,
    }));

    reg.payload.reg.hw_version = PROJECT_VERSION_HW;
    reg.payload.reg.sw_version_major = PROJECT_VERSION_SW_MAJOR;
    reg.payload.reg.sw_version_minor = PROJECT_VERSION_SW_MINOR;
    reg.payload.reg.sw_version_patch = PROJECT_VERSION_SW_PATCH;

    net_packet_t reg_data = {0};

    if (net_send(&app->net, &reg, &reg_data, NET_REPEATS) == E_OK) {
      if (reg_data.cmd != NET_CMD_REGISTRATION_DATA) {
        continue;
      }

      memcpy(app->net.key, reg_data.payload.reg_data.key, NET_KEY_SIZE);
      app->net.station_mac.value = reg_data.payload.reg_data.station_mac.value;

      net_packet_t ping = {0};

      ERROR_CHECK_RETURN(net_packet_init(&app->net, &ping, &(net_packet_cfg_t){
         .cmd          = NET_CMD_PING,
         .transport    = NET_TRANSPORT_TYPE_UNICAST,
         .target.value = 0,
      }));

      if (net_send(&app->net, &ping, &reg_data, NET_REPEATS) == E_OK && reg_data.cmd == NET_CMD_CONFIRM) {
        log_info("Registered to 0x%x", app->net.station_mac.value);

        memcpy(app->storage.key, reg_data.payload.reg_data.key, NET_KEY_SIZE);
        app->storage.station_mac.value = reg_data.payload.reg_data.station_mac.value;
        storage_write(&app->storage);

        break;
      }
    }
  }

  return E_OK;
}

error_t app_pulse_process(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  if (app_get_flag(app, APP_FLAG_PULSE_SENSOR_FAILURE)) {
    return E_FAILED;
  }

  max3010x_poll_irq_flags(&app->pulse.max3010x);

  if (max3010x_process(&app->pulse.max3010x) == MAX3010X_STATUS_SAMPLES_READY) {
    led_off(app->led.pulse);

    size_t size = PULSE_SAMPLE_COUNT;

    if (max3010x_read_samples(&app->pulse.max3010x, app->pulse.samples, &size) == E_OK) {
      bool beat = false;

      for (size_t i = 0; i < size; ++i) {
        beat |= pulse_process_sample(&app->pulse.ctx, (int32_t) app->pulse.samples[i].ir) == E_OK;
      }

      if (beat) {
        led_on(app->led.pulse);

        if (app_is_running(app)) {
          uint32_t bpm;
          pulse_approximate_bpm(&app->pulse.ctx, &bpm);

          if (bpm < PULSE_MIN_ALERT_THRESHOLD || bpm > PULSE_MAX_ALERT_THRESHOLD) {
            app_send_alert(app, NET_ALERT_TRIGGER_PULSE_THRESHOLD);
          }
        }
      }
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
    led_off(app->led.error);

#if 0
    log_printf("accel {x=%d y=%d z=%d}; gyro {x=%d y=%d z=%d}\r\n",
      app->pos.sample.accel.x, app->pos.sample.accel.y, app->pos.sample.accel.z,
      app->pos.sample.gyro.x,  app->pos.sample.gyro.y,  app->pos.sample.gyro.z
    );
#endif

    acceleration_pos_t sample = {
      .x = app->pos.sample.accel.x,
      .y = app->pos.sample.accel.y,
      .z = app->pos.sample.accel.z
    };

    acceleration_process_result_t res = acceleration_monitor_process_sample(&app->pos.monitor, &sample);

    if (res == ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED) {
      led_on(app->led.error);

      app_send_alert(app, NET_ALERT_TRIGGER_SUDDEN_MOVEMENT);
    }

    return E_OK;
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

error_t app_send_status(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  net_packet_t status = {0};

  ERROR_CHECK_RETURN(net_packet_init(&app->net, &status, &(net_packet_cfg_t){
    .cmd          = NET_CMD_STATUS,
    .transport    = NET_TRANSPORT_TYPE_UNICAST,
    .target.value = 0,
  }));

  status.payload.status.flags        = 0;
  status.payload.status.reset_reason = app->reset_reason;
  status.payload.status.reset_count  = app->reset_count;

  uint32_t bpm = 80;
  pulse_approximate_bpm(&app->pulse.ctx, &bpm);
  status.payload.status.bpm = (uint8_t) bpm;

  status.payload.status.avg_bpm = PULSE_CALCULATE_BPM_TOTAL_AVG(app->pulse.ctx.total.beats, app->pulse.ctx.total.time);

  int32_t temp = 20;
  // bsp_adc_get_temp(&temp);
  status.payload.status.cpu_temp = (int8_t) temp;

  if (app_get_flag(app, APP_FLAG_PULSE_SENSOR_FAILURE)) {
    status.payload.status.flags |= NET_STATUS_FLAG_PULSE_SENSOR_FAILURE;
  }

  if (app_get_flag(app, APP_FLAG_ACCEL_SENSOR_FAILURE)) {
    status.payload.status.flags |= NET_STATUS_FLAG_ACCEL_SENSOR_FAILURE;
  }

  if (app_get_flag(app, APP_FLAG_GPS_FAILURE)) {
    status.payload.status.flags |= NET_STATUS_FLAG_GPS_FAILURE;
  }

  return net_packet_send(&app->net, &status);
}

error_t app_send_location(app_t * app) {
  ASSERT_RETURN(app, E_NULL);

  net_packet_t location = {0};

  ERROR_CHECK_RETURN(net_packet_init(&app->net, &location, &(net_packet_cfg_t){
    .cmd          = NET_CMD_LOCATION,
    .transport    = NET_TRANSPORT_TYPE_UNICAST,
    .target.value = 0,
  }));

  location.payload.location.latitude.direction = app->gps.last_location.latitude.direction;
  location.payload.location.longitude.direction = app->gps.last_location.longitude.direction;

  strcpy(location.payload.location.latitude.value, app->gps.last_location.latitude.value);
  strcpy(location.payload.location.longitude.value, app->gps.last_location.longitude.value);

  return net_packet_send(&app->net, &location);
}

error_t app_send_alert(app_t * app, net_alert_trigger_t trigger) {
  ASSERT_RETURN(app, E_NULL);

  net_packet_t alert = {0};

  ERROR_CHECK_RETURN(net_packet_init(&app->net, &alert, &(net_packet_cfg_t){
    .cmd          = NET_CMD_ALERT,
    .transport    = NET_TRANSPORT_TYPE_UNICAST,
    .target.value = 0,
  }));

  alert.payload.alert.trigger = trigger;

  return net_packet_send(&app->net, &alert);
}
