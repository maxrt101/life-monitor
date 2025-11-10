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
void accel_task_fn(__UNUSED void * ctx) {
  os_yield();

  while (1) {
    mpu6050_measurement_t data;

    if (mpu6050_measure(&device.pos.mpu6050, &data) == E_OK) {
      led_off(&device.board.leds[BSP_LED_MAIN]);

      // log_printf("accel {x=%d y=%d z=%d}; gyro {x=%d y=%d z=%d}\r\n",
      //   data.accel.x, data.accel.y, data.accel.z,
      //   data.gyro.x,  data.gyro.y,  data.gyro.z
      // );

      acceleration_pos_t sample = { .x = data.accel.x, .y = data.accel.y, .z = data.accel.z };

      if (acceleration_monitor_process_sample(&device.pos.monitor, &sample) == ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED) {
        led_on(&device.board.leds[BSP_LED_MAIN]);
      }
    }

    os_yield();
  }
}