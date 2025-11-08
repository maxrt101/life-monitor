/** ========================================================================= *
*
 * @file gps.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief GPS
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "gps/gps.h"
#include "error/assertion.h"
#include <stdbool.h>

#include "log/log.h"

/* Defines ================================================================== */
#define LOG_TAG gps

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
typedef struct {
  struct {
    char * buffer[20];
    size_t size;
  } tokens;
  char * checksum;
} gps_parser_t;

/* Variables ================================================================ */
static const char * TALKERS[] = {
  "GA", "GB", "GI", "GL", "GN", "GP", "GQ"
};

/* Private functions ======================================================== */
static bool gps_validate_talker(const char * header) {
  ASSERT_RETURN(header, false);

  for (size_t i = 0; i < UTIL_ARR_SIZE(TALKERS); ++i) {
    if (!strcmp(header, TALKERS[i])) {
      return true;
    }
  }

  return false;
}

static error_t gps_tokenize(gps_parser_t * ctx, char * buffer, size_t size) {
  ASSERT_RETURN(ctx && buffer, E_NULL);
  ASSERT_RETURN(size, E_EMPTY);

  size_t i = 0;
  char * start = buffer;

  while (i < size) {
    char c = buffer[i];

    if (c == ',' || c == '*') {
      ctx->tokens.buffer[ctx->tokens.size++] = start;
      buffer[i] = '\0';
      start = &buffer[i+1];

      if (c == '*') {
        // TODO: Check CRC
        break;
      }
    }

    i++;
  }

  ctx->checksum = start;

  return E_OK;
}

/* Shared functions ========================================================= */
error_t gps_parse(gps_location_t * location, char * buffer, size_t size) {
  ASSERT_RETURN(location && buffer, E_NULL);
  ASSERT_RETURN(size, E_EMPTY);

  gps_parser_t ctx = {0};

  ERROR_CHECK_RETURN(gps_tokenize(&ctx, buffer, size));

#if 1
  log_printf("Tokens (%d):\r\n", ctx.tokens.size);

  for (size_t i = 0; i < ctx.tokens.size; ++i) {
    log_printf("  '%s'\r\n", ctx.tokens.buffer[i]);
  }

  // TODO: Check checksum
  log_printf("Checksum: '%s'\r\n", ctx.checksum);
#endif

  ASSERT_RETURN(ctx.tokens.size, E_EMPTY);

  const char * header = ctx.tokens.buffer[0];

  ASSERT_RETURN(header[0] == '$', E_INVAL);

  char talker[3] = {0};
  char type[5]   = {0};

  if (strlen(header) > 3) {
    ASSERT_RETURN(gps_validate_talker(header), E_INVAL);

    memcpy(talker, &header[1], 2);
    talker[2] = '\0';

    strcpy(type, &header[3]);
    type[4] = '\0';
  } else {
    // TODO: Check overflow
    strcpy(type, &header[1]);
    type[strlen(&header[1])] = '\0';
  }

  if (!strcmp("GLL", type)) {
    ASSERT_RETURN(ctx.tokens.size > 4, E_UNDERFLOW);

    strcpy(location->latitude.value, ctx.tokens.buffer[1]);
    location->latitude.direction = ctx.tokens.buffer[2][0];

    strcpy(location->longitude.value, ctx.tokens.buffer[3]);
    location->longitude.direction = ctx.tokens.buffer[4][0];

    // Check Status field, where A = valid data, V = invalid data
    ASSERT_RETURN(ctx.tokens.buffer[6][0] != 'A', E_INVAL);
  } else if (!strcmp("RMC", type)) {
    ASSERT_RETURN(ctx.tokens.size > 6, E_UNDERFLOW);

    strcpy(location->latitude.value, ctx.tokens.buffer[3]);
    location->latitude.direction = ctx.tokens.buffer[4][0];

    strcpy(location->longitude.value, ctx.tokens.buffer[5]);
    location->longitude.direction = ctx.tokens.buffer[6][0];

    // Check Status field, where A = valid data, V = invalid data
    ASSERT_RETURN(ctx.tokens.buffer[2][0] != 'A', E_INVAL);
  } else {
    return E_INVAL;
  }

#if 1
  log_printf("Talker:    %s\r\n", talker);
  log_printf("Type:      %s\r\n", type);
  log_printf("Latitude:  %c %s\r\n", location->latitude.direction, location->latitude.value);
  log_printf("Longitude: %c %s\r\n", location->longitude.direction, location->longitude.value);
#endif

  return E_OK;
}

/*
# gps test
Test sentence: '$GNRMC,060512.00,A,3150.788156,N,11711.922383,E,0.0,,311019,,,A,V*1B'
Tokens (14):
  '$GNRMC'
  '060512.00'
  'A'
  '3150.788156'
  'N'
  '11711.922383'
  'E'
  '0.0'
  ''
  '311019'
  ''
  ''
  'A'
  'V'
Checksum: '1B'
Talker: 'GN'
Type: 'RMC'
Latitude: N '3150.788156'
Longitude: E '11711.922383'
*/

// Talker IDs
// GA - European Global Navigation System (Galileo)              - Europe
// GB - BeiDou Navigation Satellite System (BDS)                 - China
// GI - Navigation Indian Constellation (NavIC)                  - India
// GL - Globalnaya Navigazionnaya Sputnikovaya Sistema (GLONASS) - Russia
// GN - Global Navigation Satellite System (GNSS)                - multiple
// GP - Global Positioning System (GPS)                          - US
// GQ - Quasi-Zenith Satellite System (QZSS)                     - Japan

// Protocol Header (message type)
// GGA – Global Positioning System Fix Data
//     Time, position, and fix related data for a GNSS receiver.
//     $<TalkerID>GGA,<Timestamp>,<Lat>,<N/S>,<Long>,<E/W>,<GPSQual>,<Sats>,<HDOP>,<Alt>,<AltVal>,<GeoSep>,<GeoVal>,<DGPSAge>,<DGPSRef>*<checksum><CR><LF>
// RMC – Recommended minimum specific GPS/Transit data
//     Time, date, position, course, and speed data provided by a GNSS receiver.
//     $<TalkerID>RMC,<Timestamp>,<Status>,<Lat>,<N/S>,<Long>,<E/W>,<SOG>,<COG>,<Date>,<MagVar>,<MagVarDir>,<mode>,<NavStatus>*<checksum><CR><LF>
// GLL – Geographic position – latitude and longitude
//     Latitude and longitude of vessel position, time of position fix and status.
//     $<TalkerID>GLL,<Lat>,<N/S>,<Long>,<E/W>,<Timestamp>,<Status>,<ModeInd>*<checksum><CR><LF>
//

// $GPRMC Sentence (Position and time)
// $GPVTG Sentence (Course over ground)
// $GPGGA Sentence (Fix data)
// $GPGSA Sentence (Active satellites)
// $GPGSV Sentence (Satellites in view)
// $GPGLL Sentence (Position)

// $GPRMC - GP RMC
// $GPGGA - GP GGA
// $PGSA  - PG SA
// $GLL   - GLL
// $PVTG  - PVTG

/*
$GPRMC,,V,,,,,,,,,,N*53
$PVTG,,,,,,,,,N*30
$GPGGA,,,,,,0,00,99.99,,,,,,*48
$PGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
$GLL,,,,,,V,N*64
$GPRMC,,V,,,,,,,,,,N*53
$PVTG,,,,,,,,,N*30
$GPGGA,,,,,,0,00,99.99,,,,,,*48
$PGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
$GLL,,,,,,V,N*64
$GPRMC,,V,,,,,,,,,,N*53
$PVTG,,,,,,,,,N*30
$GPGGA,,,,,,0,00,99.99,,,,,,*48
$PGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
$GLL,,,,,,V,N*64
$GPRMC,,V,,,,,,,,,,N*53
$PVTG,,,,,,,,,N*30
$GPGGA,,,,,,0,00,99.99,,,,,,*48
$PGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30
$GLL,,,,,,V,N*64
*/
