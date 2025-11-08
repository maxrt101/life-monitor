/** ========================================================================= *
*
 * @file gps.h
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief GPS
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
typedef struct {
  struct {
   char direction;
   char value[16];
  } latitude;

 struct {
  char direction;
  char value[16];
 } longitude;
} gps_location_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 *
 *
 */
error_t gps_parse(gps_location_t * location, char * buffer, size_t size);

#ifdef __cplusplus
}
#endif