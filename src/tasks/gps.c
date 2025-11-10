/** ========================================================================= *
*
 * @file app.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief GPS Task
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include <project.h>
#include <gps/gps.h>

#include "tasks.h"
#include "log/log.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
void gps_task_fn(__UNUSED void * ctx) {
  os_yield();

  while (1) {
    uint8_t byte = 0;
    uart_recv(device.gps.uart, &byte, 1, NULL);

    if (byte == '\r') {
      return;
    }

    if (byte == '\n') {
      device.gps.buffer[device.gps.index] = '\0';
    } else {
      device.gps.buffer[device.gps.index++] = byte;
    }

    gps_parse(&device.gps.last_location, device.gps.buffer, device.gps.index);

    os_yield();
  }
}