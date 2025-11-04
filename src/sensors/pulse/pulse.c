/** ========================================================================= *
*
 * @file pulse.c
 * @date 03-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "pulse/pulse.h"
#include "error/assertion.h"
#include "log/log.h"
#include "tty/ansi.h"

/* Defines ================================================================== */
#define LOG_TAG pulse

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
static int32_t low_pass_filter(pulse_t * pulse, int32_t value) {
  // y[n] = y[n-1] + alpha * (x - y[n-1])
  pulse->filter.lpf.y = pulse->filter.lpf.y + ((PULSE_LPF_ALPHA * ((value << 8) - pulse->filter.lpf.y)) >> 8);
  return pulse->filter.lpf.y >> 8; // scale back to original range
}

static int32_t dc_filter(pulse_t * pulse, int32_t value) {
  int32_t filtered = value - pulse->filter.dc.baseline;

  // Update baseline slowly
  pulse->filter.dc.baseline += (value - pulse->filter.dc.baseline) >> 8; // simple low-pass for DC

  return filtered;
}

static int32_t gauss_filter(pulse_t * pulse, int32_t value) {
  int32_t acc = 0;
  int32_t idx = 0;

  pulse->filter.gauss.buffer[pulse->filter.gauss.index] = value;

  // FIXME: If WINDOW_SIZE != 5 this breaks
  for (int32_t i = -2; i <= 2; ++i) {
    uint32_t buf_idx = (pulse->filter.gauss.index + i + PULSE_GAUSS_WINDOW_SIZE) % PULSE_GAUSS_WINDOW_SIZE;
    acc += pulse->filter.gauss.buffer[buf_idx] * PULSE_GAUSS_COEFFICIENTS[idx++];
  }

  pulse->filter.gauss.index = ++pulse->filter.gauss.index % PULSE_GAUSS_WINDOW_SIZE;

  return acc / PULSE_GAUSS_FACTOR;
}

static int32_t weighted_moving_average(pulse_t * pulse, int32_t value) {
  pulse->filter.wma.buffer[pulse->filter.wma.index] = value;
  pulse->filter.wma.index = ++pulse->filter.wma.index % PULSE_WMA_BUFFER_SIZE;

  int32_t wma = 0;
  int32_t wma_index = pulse->filter.wma.index;
  for (uint32_t i = 0; i < PULSE_WMA_BUFFER_SIZE; ++i) {
    wma += pulse->filter.wma.buffer[wma_index] * pulse->filter.wma.weights[i];

    wma_index = ++wma_index % PULSE_WMA_BUFFER_SIZE;
  }
  wma /= pulse->filter.wma.weights_sum;

  return wma;
}

static void pulse_detected(pulse_t * pulse) {
  pulse->total.beats++;

  if (!pulse->detect.last_beat_timestamp) {
    pulse->detect.last_beat_timestamp = pulse->last_process_timestamp;
  }

  uint32_t dt = runtime_get() - pulse->detect.last_beat_timestamp;

  pulse->approx.beats[pulse->approx.index] = dt < PULSE_MAX_BEAT_TIME_DELTA ? dt : PULSE_MAX_BEAT_TIME_DELTA;
  pulse->approx.index = ++pulse->approx.index % PULSE_BEAT_APPROX_SAMPLES;

  // store/use bpm
  pulse->detect.last_beat_timestamp = runtime_get();
}

/* Shared functions ========================================================= */
error_t pulse_init(pulse_t * pulse, int32_t raw_threshold, int32_t dcf_init_shift) {
  ASSERT_RETURN(pulse, E_NULL);

  memset(pulse, 0, sizeof(pulse_t));

  pulse->detect.raw_threshold = raw_threshold;
  pulse->filter.dc.baseline = raw_threshold - dcf_init_shift;

  log_debug("Raw Threshold: %d", pulse->detect.raw_threshold);

  for (int32_t w = 0; w < PULSE_WMA_BUFFER_SIZE; ++w) {
    pulse->filter.wma.weights[PULSE_WMA_BUFFER_SIZE - w - 1] = w;
    pulse->filter.wma.weights_sum += w;
  }

  return E_OK;
}

error_t pulse_process_sample(pulse_t * pulse, int32_t sample) {
  ASSERT_RETURN(pulse, E_NULL);
  error_t err = E_AGAIN;

  if (!pulse->last_process_timestamp) {
    pulse->last_process_timestamp = runtime_get() - 300;
  }

  pulse->total.time += runtime_get() - pulse->last_process_timestamp;

  if (sample < pulse->detect.raw_threshold) {
    err = E_OUTOFBOUNDS;
    goto exit;
  }

  int32_t lpf_res   = low_pass_filter(pulse, sample);
  int32_t dcf_res   = dc_filter(pulse, lpf_res);
  int32_t gauss_res = gauss_filter(pulse, dcf_res);
  int32_t filtered  = weighted_moving_average(pulse, gauss_res);

#if 0
  uint32_t bpm = 0;
  pulse_approximate_bpm(pulse, &bpm);

  log_printf("\r" ANSI_ERASE_LINE "raw=%d f=%d (bpm=%d/%d total=%d)",
    sample, filtered,
    pulse->approx.bpm,
    PULSE_CALCULATE_BPM_TOTAL_AVG(pulse->total.beats, pulse->total.time),
    pulse->total.beats
  );
#endif

  if (filtered > PULSE_FILTERED_MAX_THRESHOLD) {
    err = E_OUTOFBOUNDS;
    goto exit;
  }

  switch (pulse->detect.state) {
    case PULSE_DETECT_STATE_IDLE: {
      if (filtered > PULSE_FILTERED_MIN_THRESHOLD && filtered > pulse->detect.prev) {
        pulse->detect.state = PULSE_DETECT_STATE_SLOPE_UP;
      }
      break;
    }

    case PULSE_DETECT_STATE_SLOPE_UP: {
      if (filtered < pulse->detect.prev) {
        pulse->detect.state = PULSE_DETECT_STATE_SLOPE_PEAK;
      }
      break;
    }

    case PULSE_DETECT_STATE_SLOPE_PEAK: {
      if (filtered < pulse->detect.prev) {
        pulse_detected(pulse);
        pulse->detect.state = PULSE_DETECT_STATE_COOLDOWN;
        err = E_OK;
      } else {
        pulse->detect.state = PULSE_DETECT_STATE_SLOPE_UP;
      }
      break;
    }

    case PULSE_DETECT_STATE_COOLDOWN: {
      if (runtime_get() + PULSE_MIN_BEAT_TIME_DELTA > pulse->detect.last_beat_timestamp) {
        pulse->detect.state = PULSE_DETECT_STATE_IDLE;
      }
      break;
    }

    default: {
      break;
    }
  }

exit:
#if 0
  if (err != E_OUTOFBOUNDS) {
    log_printf("ir=%d f=%d gauss=%d wma=%d pulse=%d\r\n",
      sample, dcf_res, gauss_res, filtered, err == E_OK ? 300 : 0
    );
  }
#endif

  pulse->detect.prev = filtered;
  pulse->last_process_timestamp = runtime_get();

  return err;
}

error_t pulse_approximate_bpm(pulse_t * pulse, uint32_t * bpm) {
  ASSERT_RETURN(pulse && bpm, E_NULL);

  uint32_t avg_beat_time = 0;
  uint32_t beat_count = 0;

  // TODO: Value needs to stabilize, over at least 10-16 samples
  for (uint32_t i = 0; i < PULSE_BEAT_APPROX_SAMPLES; ++i) {
    if (pulse->approx.beats[i]) {
      avg_beat_time += pulse->approx.beats[i];
      beat_count++;
    }
  }
  avg_beat_time /= beat_count;

  uint32_t bpms = 1000000 / avg_beat_time;
  pulse->approx.bpm = 60 * bpms / 1000;

  *bpm = pulse->approx.bpm;

  return E_OK;
}

error_t pulse_report_bpm(pulse_t * pulse) {
  ASSERT_RETURN(pulse, E_NULL);

  uint32_t bpm = 0;

  ERROR_CHECK_RETURN(pulse_approximate_bpm(pulse, &bpm));

#if PULSE_REPORT_ON_SAME_LINE
  log_printf(ANSI_CURSOR_MOVE_UP(1) "\r" ANSI_ERASE_LINE);
#endif

#if PULSE_REPORT_EXT
  log_info("BPM=%d (dt=%d total=%d avg=%d)",
    bpm,
    pulse->approx.beats[pulse->approx.index ? pulse->approx.index - 1 : PULSE_BEAT_APPROX_SAMPLES - 1],
    pulse->total.beats,
    PULSE_CALCULATE_BPM_TOTAL_AVG(pulse->total.beats, pulse->total.time)
  );
#else
  log_printf(ANSI_CURSOR_MOVE_UP(1) "\r" ANSI_ERASE_LINE "BPM=%d", bpm);
#endif

  return E_OK;
}
