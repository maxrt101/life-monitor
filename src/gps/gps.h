/** ========================================================================= *
*
 * @file gps.h
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief GPS NMEA Sentence Parser (only parses latitude/logitude)
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "error/error.h"
#include <stdlib.h>

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Raw location from GPS
 *
 * @note Must be converted using DDMM.MMMM format, where D is degrees & M is minutes
 */
typedef struct {
  struct {
    /** Direction N/S - North/South */
    char direction;

    /** Raw latitude value in DDMM.MMMM format */
    char value[16];
  } latitude;

  struct {
    /** Direction W/E - West/East */
    char direction;

    /** Raw longitude value in DDMM.MMMM format */
    char value[16];
  } longitude;
} gps_location_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Parse GPS location from NMEA sentence produced by GPS
 *
 * @param location Result location
 * @param buffer NMEA sentence data
 * @param size NMEA sentence size
 */
error_t gps_parse(gps_location_t * location, char * buffer, size_t size);

#ifdef __cplusplus
}
#endif