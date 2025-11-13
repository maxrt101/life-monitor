/** ========================================================================= *
*
 * @file storage.h
 * @date 13-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include <stdint.h>
#include "error/error.h"
#include "util/compiler.h"
#include "net/types.h"

/* Defines ================================================================== */

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Persistent data
 */
typedef __PACKED_STRUCT {
  uint8_t   reset_count;
  net_mac_t station_mac;
  net_key_t key;
  uint8_t   crc;
} storage_data_t;

/* Variables ================================================================ */
/** Defined in LD script */
extern void * __storage_start;

/* Shared functions ========================================================= */
/**
 * Reads storage from flash at storage_start, and checks CRC
 *
 * @param[in] storage Storage instance
 */
error_t storage_read(storage_data_t * storage);

/**
 * Calculates new CRC and saves storage to NVM using hal/nvm
 *
 * @param[in] storage Storage instance
 */
error_t storage_write(storage_data_t * storage);

#ifdef __cplusplus
}
#endif
