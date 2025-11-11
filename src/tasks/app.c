/** ========================================================================= *
*
 * @file app.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief App Task
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include <project.h>
#include "log/log.h"
#include "tasks.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
void app_task_fn(__UNUSED void * ctx) {
  os_yield();

  while (1) {
    if (device.app.is_running) {
      app_pulse_process(&device.app);
    }
    os_yield();

    if (device.app.is_running) {
      app_pos_process(&device.app);
    }
    os_yield();

    if (device.app.is_running) {
      app_gps_process(&device.app);
    }
    os_yield();
  }
}