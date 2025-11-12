/** ========================================================================= *
*
 * @file hopping.h
 * @date 11-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network Frequency Hopping
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "util/util.h"
#include "error/error.h"

/* Defines ================================================================== */
/** Single step of frequency corresponding to 1 step in hopping table */
#define NET_CHANNEL_STEP_KHZ      500
#define NET_CHANNEL_BASE_FREQ_KHZ 433000

/** Use full 32 channel hopping table */
#ifndef NET_USE_FULL_CHANNEL_TABLE
#define NET_USE_FULL_CHANNEL_TABLE 0
#endif

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Network Hopping Context
 */
typedef struct {
  uint32_t          base_freq_khz;  /** Base frequency */
  struct {
    const uint8_t * offsets;        /** Frequency offsets table */
    uint8_t         size;           /** Frequency offsets table size */
    uint8_t         index;          /** Index info frequency offsets table */
  } channels;                       /** Channels Context */
} net_hopping_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initialize hopping
 *
 * @param hopping Network Hopping Context
 */
error_t net_hopping_init(net_hopping_t * hopping);

/**
 * Return current hopping frequency
 *
 * @param hopping Network Hopping Context
 * @return Current hopping frequency in KHz
 */
uint32_t net_hopping_get_hop_freq(net_hopping_t * hopping);

/**
 * Return base frequency
 *
 * @param hopping Network Hopping Context
 * @return Base frequency in KHz
 */
uint32_t net_hopping_get_base_freq(net_hopping_t * hopping);

/**
 * Switch to next hopping frequency
 *
 * @param hopping Network Hopping Context
 */
error_t net_hopping_switch_channel(net_hopping_t * hopping);

#ifdef __cplusplus
}
#endif

