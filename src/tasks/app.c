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

  if (bsp_is_btn_pressed()) {
    app_start(&device.app);
#if !DEBUG
    app_register(&device.app);
#endif
  }

  os_yield();

  while (1) {
    if (app_is_running(&device.app)) {
      app_pulse_process(&device.app);
      os_yield();

      app_pos_process(&device.app);
      os_yield();

      app_gps_process(&device.app);
      os_yield();

      if (timeout_is_expired(&device.app.status_send_timeout)) {
        app_send_status(&device.app);
        timeout_start(&device.app.status_send_timeout, NET_STATUS_SEND_PERIOD);
      }
    }

    os_yield();
  }
}