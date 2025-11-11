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
  log_info("Will print samples. Press any key to stop");

  while (1) {
    char c = '\0';

    if (tty_get_char_async(&sh->tty, &c) == E_OK && c != '\0') {
      log_printf("\r\n");
      log_info("Stopping...");
      break;
    }

    error_t err = app_pos_process(&device.app);

    if (err == E_OK || err == E_EMPTY) {
      log_printf("accel {x=%d y=%d z=%d}; gyro {x=%d y=%d z=%d}\r\n",
        device.app.pos.sample.accel.x, device.app.pos.sample.accel.y, device.app.pos.sample.accel.z,
        device.app.pos.sample.gyro.x,  device.app.pos.sample.gyro.y,  device.app.pos.sample.gyro.z
      );
    }

    sleep_ms(200);
  }

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(pos, cmd_pos, "Accel/Gyro (MPU6050) Control");

