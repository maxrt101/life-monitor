/** ========================================================================= *
 *
 * @file accel.c
 * @date 04-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "sensors/accel/accel.h"
#include "error/assertion.h"
#include "tty/ansi.h"
#include "log/log.h"

/* Defines ================================================================== */
#define LOG_TAG accel

/* Macros =================================================================== */
#define ABS_DIFF(__a, __b) ((__a) > (__b) ? (__a) - (__b) : (__b) - (__a))

/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
static int16_t weighted_moving_average(acceleration_wma_t * wma, int16_t value) {
  wma->buffer[wma->index] = value;
  wma->index = ++wma->index % ACCELERATION_WMA_BUFFER_SIZE;

  int32_t avg = 0;
  uint32_t wma_index = wma->index;

  for (uint32_t i = 0; i < ACCELERATION_WMA_BUFFER_SIZE; ++i) {
    avg += wma->buffer[wma_index] * wma->weights[i];
    // avg += wma->buffer[wma_index];

    wma_index = ++wma_index % ACCELERATION_WMA_BUFFER_SIZE;
  }

  // avg /= wma->weights_sum;
  avg /= ACCELERATION_WMA_BUFFER_SIZE;

  return avg;
}

/* Shared functions ========================================================= */
error_t acceleration_monitor_init(acceleration_monitor_t * am) {
  ASSERT_RETURN(am, E_NULL);

  memset(am, 0, sizeof(acceleration_monitor_t));

  for (int16_t w = 0; w < ACCELERATION_WMA_BUFFER_SIZE; ++w) {
    // int16_t index = ACCELERATION_WMA_BUFFER_SIZE - w - 1;
    int16_t index = w;

    am->x.weights[index] = w;
    am->x.weights_sum += w;

    am->y.weights[index] = w;
    am->y.weights_sum += w;

    am->z.weights[index] = w;
    am->z.weights_sum += w;
  }

  return E_OK;
}

acceleration_process_result_t acceleration_monitor_process_sample(acceleration_monitor_t * am, acceleration_pos_t * sample) {
  ASSERT_RETURN(am && sample, ACCELERATION_RESULT_IDLE);

  acceleration_process_result_t res = ACCELERATION_RESULT_IDLE;

  acceleration_pos_t avg = {
    .x = weighted_moving_average(&am->x, sample->x),
    .y = weighted_moving_average(&am->y, sample->y),
    .z = weighted_moving_average(&am->z, sample->z),
  };

  log_printf("accel {x=%d y=%d z=%d}; avg {x=%d y=%d z=%d}\r\n",
    sample->x, sample->y, sample->z, avg.x, avg.y, avg.z
  );

  if (ABS_DIFF(sample->x, avg.x) > ACCELERATION_SUDDEN_MOVEMENT_THRESHOLD) {
    log_printf(ANSI_COLOR_FG_RED "X > THRESHOLD (%d %d)" ANSI_TEXT_RESET "\r\n", sample->x, avg.x);
    res = ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED;
  }

  if (ABS_DIFF(sample->y, avg.y) > ACCELERATION_SUDDEN_MOVEMENT_THRESHOLD) {
    log_printf(ANSI_COLOR_FG_RED "Y > THRESHOLD (%d %d)" ANSI_TEXT_RESET "\r\n", sample->y, avg.y);
    res = ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED;
  }

  if (ABS_DIFF(sample->z, avg.z) > ACCELERATION_SUDDEN_MOVEMENT_THRESHOLD) {
    log_printf(ANSI_COLOR_FG_RED "Z > THRESHOLD (%d %d)" ANSI_TEXT_RESET "\r\n", sample->z, avg.z);
    res = ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED;
  }

  return res;
}