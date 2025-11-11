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

    app_pulse_process(&device.app);
    // pulse_report_bpm(&device.app.pulse.ctx);
  }

  log_info("beats:    %u", device.app.pulse.ctx.total.beats);
  log_info("time(ms): %u", device.app.pulse.ctx.total.time);
  log_info("bpm(%d):  %u", PULSE_BEAT_APPROX_SAMPLES, device.app.pulse.ctx.approx.bpm);
  log_info("bpm(abs): %u", PULSE_CALCULATE_BPM_TOTAL_AVG(device.app.pulse.ctx.total.beats, device.app.pulse.ctx.total.time));

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(pulse, cmd_pulse, "Pulse Sensor (MAX30102) control");

