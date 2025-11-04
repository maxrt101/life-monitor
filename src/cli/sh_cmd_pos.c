/** ========================================================================= *
*
 * @file sh_cmd_pos.c
 * @date 04-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'pos' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "drv/mpu6050/mpu6050.h"
#include "sensors/accel/accel.h"
#include "shell/shell_util.h"
#include "shell/shell.h"
#include "tty/ansi.h"
#include "log/log.h"
#include "project.h"
#include "time/sleep.h"

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */

/* Shared functions ========================================================= */
static int8_t cmd_pos(shell_t * sh, uint8_t argc, const char ** argv) {
  acceleration_monitor_t am;
  mpu6050_t mpu6050;

  log_info("Initializing MPU6050...");

  SHELL_ERR_REPORT_RETURN(
    mpu6050_init(
      &mpu6050,
      &(mpu6050_cfg_t) {
        .i2c   = &device.board.i2c,
        .gyro  = MPU6050_GYRO_FS_SEL_1000_DEG_PER_S,
        .accel = MPU6050_ACCEL_AFS_SEL_16G,
      }
    ),
    "mpu6050_init"
  );

  acceleration_monitor_init(&am);

  log_info("Will print samples. Press any key to stop");

  while (1) {
    char c = '\0';

    if (tty_get_char_async(&sh->tty, &c) == E_OK && c != '\0') {
      log_printf("\r\n");
      log_info("Stopping...");
      break;
    }

    mpu6050_measurement_t data;

    if (mpu6050_measure(&mpu6050, &data) == E_OK) {
      led_off(&device.board.leds[BSP_LED_MAIN]);

      // log_printf("accel {x=%d y=%d z=%d}; gyro {x=%d y=%d z=%d}\r\n",
      //   data.accel.x, data.accel.y, data.accel.z,
      //   data.gyro.x,  data.gyro.y,  data.gyro.z
      // );

      acceleration_pos_t sample = { .x = data.accel.x, .y = data.accel.y, .z = data.accel.z };

      if (acceleration_monitor_process_sample(&am, &sample) == ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED) {
        led_on(&device.board.leds[BSP_LED_MAIN]);
      }
    }

    sleep_ms(200);
  }

  SHELL_ERR_REPORT_RETURN(mpu6050_sleep(&mpu6050), "mpu6050_sleep");


  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(pos, cmd_pos, "Accel/Gyro (MPU6050) Control");

