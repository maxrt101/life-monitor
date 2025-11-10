/** ========================================================================= *
*
 * @file sh_cmd_pulse.c
 * @date 31-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'pulse' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "drv/max3010x/max3010x.h"
#include "shell/shell_util.h"
#include "shell/shell.h"
#include "sensors/pulse/pulse.h"
#include "tty/ansi.h"
#include "log/log.h"
#include "project.h"

/* Defines ================================================================== */
#define LOG_TAG shell

#define SAMPLES_COUNT 16
#define CURRENT       50

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static int8_t cmd_pulse(shell_t * sh, uint8_t argc, const char ** argv) {
  while (1) {
    char c = '\0';

    if (tty_get_char_async(&sh->tty, &c) == E_OK && c != '\0') {
      log_printf("\r\n");
      log_info("Stopping...");
      break;
    }

    max3010x_poll_irq_flags(&device.pulse.max3010x);

    if (max3010x_process(&device.pulse.max3010x) == MAX3010X_STATUS_SAMPLES_READY) {
      led_off(&device.board.leds[BSP_LED_MAIN]);

      size_t size = SAMPLES_COUNT;

      if (max3010x_read_samples(&device.pulse.max3010x, device.pulse.samples, &size) == E_OK) {
        bool beat = false;

        for (size_t i = 0; i < size; ++i) {
          beat |= pulse_process_sample(&device.pulse.ctx, (int32_t) device.pulse.samples[i].ir) == E_OK;
        }
        pulse_report_bpm(&device.pulse.ctx);

        if (beat) {
          led_on(&device.board.leds[BSP_LED_MAIN]);
        }
      }
    }
  }

  log_info("beats:    %u", device.pulse.ctx.total.beats);
  log_info("time(ms): %u", device.pulse.ctx.total.time);
  log_info("bpm(%d):  %u", PULSE_BEAT_APPROX_SAMPLES, device.pulse.ctx.approx.bpm);
  log_info("bpm(abs): %u", PULSE_CALCULATE_BPM_TOTAL_AVG(device.pulse.ctx.total.beats, device.pulse.ctx.total.time));

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(pulse, cmd_pulse, "Pulse Sensor (MAX30102) control");

