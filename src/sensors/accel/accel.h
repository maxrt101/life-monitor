/** ========================================================================= *
 *
 * @file accel.h
 * @date 04-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Sudden movement detector from accelerometer data
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "lib/error/error.h"
#include <stdint.h>

/* Defines ================================================================== */
/** Size of Simple Moving Average Buffer */
#define ACCELERATION_SMA_BUFFER_SIZE 16

/**
 * Threshold that difference of avg & sample value has to cross to be
 * considered a sudden movement
 */
#ifndef ACCELERATION_SUDDEN_MOVEMENT_THRESHOLD
#define ACCELERATION_SUDDEN_MOVEMENT_THRESHOLD 10000
#endif

/* Macros =================================================================== */
/* Enums ==================================================================== */
/**
 * Acceleration Process Result
 */
typedef enum {
  ACCELERATION_RESULT_IDLE = 0,
  ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED,
} acceleration_process_result_t;

/* Types ==================================================================== */
/**
 * Single value (x, y or z)
 */
typedef int16_t acceleration_point_t;

/**
 * Value vector (x, y & z)
 */
typedef struct {
  acceleration_point_t x;
  acceleration_point_t y;
  acceleration_point_t z;
} acceleration_pos_t;

/**
 * Simple Moving Average Context
 */
typedef struct {
  int16_t  buffer[ACCELERATION_SMA_BUFFER_SIZE];
  uint32_t index;
} acceleration_sma_t;

/**
 * Acceleration Monitor Context
 */
typedef struct {
  acceleration_sma_t x;
  acceleration_sma_t y;
  acceleration_sma_t z;
} acceleration_monitor_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initialized Acceleration Monitor
 *
 * @param am Acceleration Monitor Context
 */
error_t acceleration_monitor_init(acceleration_monitor_t * am);

/**
 * Process accelerometer sample
 *
 * @param am Acceleration Monitor Context
 * @param sample Accelerometer Sample
 * @returns ACCELERATION_RESULT_SUDDEN_MOVEMENT_DETECTED if sudden movement detected
 */
acceleration_process_result_t acceleration_monitor_process_sample(
  acceleration_monitor_t * am, acceleration_pos_t * sample
);

#ifdef __cplusplus
}
#endif
