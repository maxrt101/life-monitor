/** ========================================================================= *
 *
 * @file pulse.h
 * @date 03-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Pulse (Heart Beat) Detector. Detects beats from raw ADC samples
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "error/error.h"
#include "time/time.h"
#include <stdint.h>

/* Defines ================================================================== */
/**
 * Print each BPM report on the same line
 */
#define PULSE_REPORT_ON_SAME_LINE 1

/**
 * Print additional info in pulse_report_bpm
 */
#define PULSE_REPORT_EXT 1

/**
 * Number of approximation samples
 * Each sample consists of a time difference with previous heartbeat
 */
#define PULSE_BEAT_APPROX_SAMPLES 32

/** Low Pass Filter Config  */
#define PULSE_LPF_SCALE 256
#define PULSE_LPF_ALPHA 16 // alpha = 16/256 ~ 0.0625

/** Gauss Filter Config */

#if 0
#define PULSE_GAUSS_WINDOW_SIZE 5
#define PULSE_GAUSS_COEFFICIENTS ((int32_t[]){1, 4, 6, 4, 1})
#define PULSE_GAUSS_FACTOR      16
#endif

#if 1
#define PULSE_GAUSS_WINDOW_SIZE 5
#define PULSE_GAUSS_COEFFICIENTS ((int32_t[]){16, 64, 96, 64, 16})
#define PULSE_GAUSS_FACTOR      256
#endif

#if 0
#define PULSE_GAUSS_WINDOW_SIZE  7
#define PULSE_GAUSS_COEFFICIENTS ((int32_t[]){1, 6, 15, 20, 15, 6, 1})
#define PULSE_GAUSS_FACTOR       64
#endif

#if 0
#define PULSE_GAUSS_WINDOW_SIZE  9
#define PULSE_GAUSS_COEFFICIENTS ((int32_t[]){1, 4, 11, 20, 26, 20, 11, 4, 1})
#define PULSE_GAUSS_FACTOR       64 // original: 98
#endif

/**
 * Weighted Moving Average Window(Buffer) Size
 */
#define PULSE_WMA_BUFFER_SIZE 32

/**
 * Thresholds for filtered ADC values
 */
#define PULSE_FILTERED_MIN_THRESHOLD 210
#define PULSE_FILTERED_MAX_THRESHOLD 260

/**
 * Beat time delta min/max values
 */
#define PULSE_MIN_BEAT_TIME_DELTA 250  // Max BPM ~ 240
#define PULSE_MAX_BEAT_TIME_DELTA 1200 // Min BPM ~ 50

/* Macros =================================================================== */
/**
 * Calculate average BPM from total beats count & total time(in ms) in which
 * those beats happened
 *
 * @param __total_beats   Beats count
 * @param __total_time_ms Milliseconds
 */
#define PULSE_CALCULATE_BPM_TOTAL_AVG(__total_beats, __total_time_ms) \
  (60 * (1000000 / (__total_time_ms / __total_beats)) / 1000)

/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Pulse detector context
 */
typedef struct {
  /** Timestamp of last time pulse_process_sample was called */
  milliseconds_t last_process_timestamp;

  /** Total counters */
  struct {
    uint32_t beats;
    uint32_t time;
  } total;

  /** Beat Detection Context */
  struct {
    enum {
      PULSE_DETECT_STATE_IDLE = 0,
      PULSE_DETECT_STATE_SLOPE_UP,
      PULSE_DETECT_STATE_SLOPE_PEAK,
      PULSE_DETECT_STATE_COOLDOWN,
    } state;

    /** Previous filtered value */
    uint32_t prev;

    /** Threshold that raw ADC value has to surpass */
    uint32_t raw_threshold;

    /** Timestamp of last detect heartbeat */
    milliseconds_t last_beat_timestamp;
  } detect;

  /** BPM Approximation Context */
  struct {
    uint32_t beats[PULSE_BEAT_APPROX_SAMPLES];
    uint32_t index;

    /** Last approximated value */
    uint32_t bpm;
  } approx;

  /** Filters Context */
  struct {
    /** Low Pass Filter */
    struct {
      int32_t y;
    } lpf;

    /** DC Filter */
    struct {
      int32_t baseline;
    } dc;

    /** Gauss Filter */
    struct {
      int32_t  buffer[PULSE_GAUSS_WINDOW_SIZE];
      uint32_t index;
    } gauss;

    /** Weighted Moving Average */
    struct {
      int32_t  buffer[PULSE_WMA_BUFFER_SIZE];
      uint32_t index;

      int32_t weights[PULSE_WMA_BUFFER_SIZE];
      int32_t weights_sum;
    } wma;
  } filter;
} pulse_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initializes Pulse Detector Context
 *
 * @param pulse Pulse Detector Context
 * @param raw_threshold Threshold that raw ADC sample has to surpass
 * @param dcf_init_shift Negative shift to raw_threshold. Used to initialize
 *                       DC Filter baseline, for faster init-to-first-beat time
 */
error_t pulse_init(pulse_t * pulse, int32_t raw_threshold, int32_t dcf_init_shift);

/**
 * Process single raw ADC sample
 *
 * @param pulse Pulse Detector Context
 * @param sample Raw ADC value
 * @return E_OK if heartbeat was detected, E_AGAIN if not, E_OUTOFBOUNDS if value
 *         didn't pass thresholds
 */
error_t pulse_process_sample(pulse_t * pulse, int32_t sample);

/**
 * Approximate BPM based on last PULSE_BEAT_APPROX_SAMPLES heartbeats
 *
 * Uses an array of time differences from previous heartbeats:
 * x[i] = HB_TS - x[i-1]
 * x - Approximation sample buffer
 * HB_TS - HeatBeat TimeStamp (current)
 *
 * @param pulse Pulse Detector Context
 * @param bpm Where to put approximated BPM
 * @return E_OK if approximation was successful, E_AGAIN - if buffer isn't full
 */
error_t pulse_approximate_bpm(pulse_t * pulse, uint32_t * bpm);

/**
 * Print approximated BPM
 *
 * @param pulse Pulse Detector Context
 */
error_t pulse_report_bpm(pulse_t * pulse);

#ifdef __cplusplus
}
#endif