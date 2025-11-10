/** ========================================================================= *
*
 * @file pulse.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Pulse Task
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include <project.h>
#include <gps/gps.h>

#include "tasks.h"
#include "log/log.h"

/* Defines ================================================================== */
#define SAMPLE_COUNT 16

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */

/* Private functions ======================================================== */
/* Shared functions ========================================================= */
void pulse_task_fn(__UNUSED void * ctx) {
  os_yield();

  while (1) {
    max3010x_poll_irq_flags(&device.pulse.max3010x);

    if (max3010x_process(&device.pulse.max3010x) == MAX3010X_STATUS_SAMPLES_READY) {
      led_off(&device.board.leds[BSP_LED_MAIN]);

      size_t size = SAMPLE_COUNT;

      if (max3010x_read_samples(&device.pulse.max3010x, device.pulse.samples, &size) == E_OK) {
        bool beat = false;

        for (size_t i = 0; i < size; ++i) {
          beat |= pulse_process_sample(&device.pulse.ctx, (int32_t) device.pulse.samples[i].ir) == E_OK;
        }
//        pulse_report_bpm(&device.pulse.ctx);

        if (beat) {
          led_on(&device.board.leds[BSP_LED_MAIN]);
        }
      }
    }

    os_yield();
  }
}