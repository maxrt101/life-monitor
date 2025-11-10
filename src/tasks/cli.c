/** ========================================================================= *
*
 * @file cli.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief CLI Task
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include <project.h>

#include "tasks.h"
#include "shell/shell.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
void cli_task_signal_handler(os_signal_t signal, __UNUSED void * ctx) {
  switch (signal) {
    case OS_SIGNAL_RESUME:
      shell_start(&device.shell);
    break;

    case OS_SIGNAL_PAUSE:
    case OS_SIGNAL_KILL:
      shell_stop(&device.shell);
    break;

    default:
      break;
  }
}

/* Shared functions ========================================================= */
void cli_task_fn(__UNUSED void * ctx) {
  os_signal_register_handler(
    OS_SIGNAL_PAUSE | OS_SIGNAL_RESUME | OS_SIGNAL_KILL,
    cli_task_signal_handler
  );

  shell_init(&device.shell, vfs_open(&vfs, CONSOLE_FILE), NULL);

  shell_start(&device.shell);

  os_yield();

  while (1) {
    shell_process(&device.shell);
    os_yield();
  }
}