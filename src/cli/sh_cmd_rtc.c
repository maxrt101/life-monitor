/** ========================================================================= *
*
 * @file sh_cmd_rtc.c
 * @date 07-03-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'rtc' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "shell/shell_util.h"
#include "log/log.h"
#include "bsp.h"
#include <string.h>

/* Defines ================================================================== */
#define LOG_TAG SHELL

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static int8_t cmd_rtc(shell_t * sh, uint8_t argc, const char ** argv) {
  if (argc < 2) {
    log_error("Usage: rtc wup [MS]");
    return SHELL_FAIL;
  }

  if (!strcmp(argv[1], "wup")) {
    if (argc > 2) {
      bsp_set_next_wakeup(shell_parse_int(argv[2]));
    } else {
      log_info("wup: %d", bsp_get_wakeup());
    }
  } else {
    log_error("Usage: rtc wup [MS]");
    return SHELL_FAIL;
  }

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(rtc, cmd_rtc, "RTC control");

