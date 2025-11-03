/** ========================================================================= *
*
 * @file sh_cmd_i2c.c
 * @date 31-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'i2c' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "log/log.h"
#include "project.h"

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static int8_t cmd_i2c(shell_t * sh, uint8_t argc, const char ** argv) {
  i2c_detect_result_t detect;

  i2c_detect(&device.board.i2c, detect);
  i2c_detect_dump(detect);

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(i2c, cmd_i2c, "I2C bus control");

