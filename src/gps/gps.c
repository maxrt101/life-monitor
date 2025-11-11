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
/* Private functions ======================================================== */
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

#if 0
  log_printf("%.*s\r\n", size, buffer);
#endif

  gps_parser_t ctx = {0};

  ERROR_CHECK_RETURN(gps_tokenize(&ctx, buffer, size));

#if 0
  log_printf("Tokens (sz=%d crc='%s'): ", ctx.tokens.size, ctx.checksum);

  for (size_t i = 0; i < ctx.tokens.size; ++i) {
    log_printf("'%s' ", ctx.tokens.buffer[i]);
  }

  log_printf("\r\n");
#endif

  ASSERT_RETURN(ctx.tokens.size, E_EMPTY);

  const char * header = ctx.tokens.buffer[0];

  ASSERT_RETURN(header[0] == '$', E_INVAL);

  if (strstr(header, "GLL") != NULL) {
    ASSERT_RETURN(ctx.tokens.size > 4, E_UNDERFLOW);

    strcpy(location->latitude.value, ctx.tokens.buffer[1]);
    location->latitude.direction = ctx.tokens.buffer[2][0];

    strcpy(location->longitude.value, ctx.tokens.buffer[3]);
    location->longitude.direction = ctx.tokens.buffer[4][0];

    // Check Status field, where A = valid data, V = invalid data
    ASSERT_RETURN(ctx.tokens.buffer[6][0] == 'A', E_INVAL);
  } else if (strstr(header, "RMC") != NULL) {
    ASSERT_RETURN(ctx.tokens.size > 6, E_UNDERFLOW);

    strcpy(location->latitude.value, ctx.tokens.buffer[3]);
    location->latitude.direction = ctx.tokens.buffer[4][0];

    strcpy(location->longitude.value, ctx.tokens.buffer[5]);
    location->longitude.direction = ctx.tokens.buffer[6][0];

    // Check Status field, where A = valid data, V = invalid data
    ASSERT_RETURN(ctx.tokens.buffer[2][0] == 'A', E_INVAL);
  } else {
    return E_INVAL;
  }

#if 0
  log_printf("Latitude:  %c %s\r\n", location->latitude.direction, location->latitude.value);
  log_printf("Longitude: %c %s\r\n", location->longitude.direction, location->longitude.value);
#endif

  return E_OK;
}
