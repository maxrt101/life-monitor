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
#include "pulse/pulse.h"
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
  max3010x_t max3010x;

  log_info("Initializing MAX3010X...");

  SHELL_ERR_REPORT_RETURN(
    max3010x_init(
      &max3010x,
      &(max3010x_cfg_t){
        .i2c = &device.board.i2c,
        .adc_range.max30102 = MAX30102_ADC_RANGE_2K_nA,
        .pulse_width = {
          .max30100 = MAX30100_PULSE_WIDTH_1600_ADC_16_BIT,
          .max30102 = MAX30102_PULSE_WIDTH_118_ADC_16_BIT,
        },
        .sample_rate = {
          .max30100 = MAX30100_SAMPLE_RATE_100_HZ,
          .max30102 = MAX30102_SAMPLE_RATE_100_HZ,
        },
        .current = {
          .ir  = CURRENT,
          .red = CURRENT
        },
        .mode = MAX3010X_MODE_HEART_RATE
      }
    ),
    "max3010x_init");

  log_info("Will print read samples. Press any key to stop");

  max3010x_sample_t samples[SAMPLES_COUNT];
  pulse_t pulse;

  pulse_init(
    &pulse,
    max3010x_get_min_ir_adc_voltage(&max3010x),
    500
  );

  while (1) {
    char c = '\0';

    if (tty_get_char_async(&sh->tty, &c) == E_OK && c != '\0') {
      log_printf("\r\n");
      log_info("Stopping...");
      break;
    }

    max3010x_poll_irq_flags(&max3010x);

    if (max3010x_process(&max3010x) == MAX3010X_STATUS_SAMPLES_READY) {
      led_off(&device.board.leds[BSP_LED_MAIN]);

      size_t size = SAMPLES_COUNT;

      if (max3010x_read_samples(&max3010x, samples, &size) == E_OK) {
        bool beat = false;

        for (size_t i = 0; i < size; ++i) {
          beat |= pulse_process_sample(&pulse, (int32_t) samples[i].ir) == E_OK;
        }
        pulse_report_bpm(&pulse);

        if (beat) {
          led_on(&device.board.leds[BSP_LED_MAIN]);
        }
      }
    }
  }

  SHELL_ERR_REPORT_RETURN(max3010x_shutdown(&max3010x), "max3010x_shutdown");

  log_info("beats:    %u", pulse.total.beats);
  log_info("time(ms): %u", pulse.total.time);
  log_info("bpm(%d):  %u", PULSE_BEAT_APPROX_SAMPLES, pulse.approx.bpm);
  log_info("bpm(abs): %u", PULSE_CALCULATE_BPM_TOTAL_AVG(pulse.total.beats, pulse.total.time));

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(pulse, cmd_pulse, "Pulse Sensor (MAX30102) control");

