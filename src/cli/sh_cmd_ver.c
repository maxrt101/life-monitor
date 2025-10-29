/** ========================================================================= *
*
 * @file sh_cmd_ver.c
 * @date 13-11-2024
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'ver' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "log/log.h"

/* Defines ================================================================== */
#define LOG_TAG SHELL

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */

/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */

/* Shared functions ========================================================= */
static int8_t cmd_ver(shell_t * sh, uint8_t argc, const char ** argv) {
  log_printf(
      "Project:     %s\r\n"
      "Version:     %s\r\n"
      "Commit:      %s\r\n"
      "Built on:    %s %s\r\n"
      "Built by:    %s\r\n"
      "Built using: %s %s\r\n",
    PROJECT_NAME,
    PROJECT_VERSION,
    PROJECT_COMMIT,
    __TIME__, __DATE__,
    PROJECT_COMPILED_BY,
    PROJECT_COMPILED_WITH, __VERSION__
  );

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(ver, cmd_ver, "Prints version");
