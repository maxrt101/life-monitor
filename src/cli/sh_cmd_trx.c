/** ========================================================================= *
 *
 * @file sh_cmd_trx.c
 * @date 13-11-2024
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'trx' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "shell/shell_util.h"
#include "log/log.h"
#include "trx/trx.h"
#include "project.h"
#include <string.h>

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
__STATIC void cmd_trx_usage(void) {
  log_error("Usage: trx rssi|pa|bw|baud|preamble|send|recv ...");
}

/* Shared functions ========================================================= */
static int8_t cmd_trx(shell_t * sh, uint8_t argc, const char ** argv) {
  if (argc < 2) {
    cmd_trx_usage();
    return SHELL_FAIL;
  }

  if (!strcmp(argv[1], "rssi")) {
    int16_t rssi = 0;
    trx_get_rssi(&device.trx, &rssi);
    log_info("%d dBm", rssi);
  } else if (!strcmp(argv[1], "pa")) {
    if (argc == 2) {
      uint8_t power = 0;
      trx_get_power(&device.trx, &power);
      log_info("PA: %d", power);
    } else {
      trx_set_power(&device.trx, shell_parse_int(argv[2]));
    }
  } else if (!strcmp(argv[1], "bw")) {
    if (argc < 3) {
      cmd_trx_usage();
      return SHELL_FAIL;
    }
    trx_set_bandwidth(&device.trx, shell_parse_int(argv[2]));
  } else if (!strcmp(argv[1], "baud")) {
    if (argc < 3) {
      cmd_trx_usage();
      return SHELL_FAIL;
    }
    trx_set_baudrate(&device.trx, shell_parse_int(argv[2]));
  } else if (!strcmp(argv[1], "preamble")) {
    if (argc < 3) {
      cmd_trx_usage();
      return SHELL_FAIL;
    }
    trx_set_preamble(&device.trx, shell_parse_int(argv[2]));
  } else if (!strcmp(argv[1], "send")) {
    uint8_t size = argc - 2;
    uint8_t data[TRX_MAX_PACKET_SIZE];

    for (uint8_t i = 0; i < size; ++i) {
      data[i] = shell_parse_int(argv[i+2]);
    }

    SHELL_ERR_REPORT_RETURN(trx_send(&device.trx, data, size), "trx_send");
  } else if (!strcmp(argv[1], "recv")) {
    if (argc != 3) {
      log_error("Usage: trx recv TIMEOUT");
      return SHELL_FAIL;
    }

    size_t size = TRX_MAX_PACKET_SIZE;
    uint8_t data[TRX_MAX_PACKET_SIZE];

    TIMEOUT_CREATE(t, shell_parse_int(argv[2]));

    SHELL_ERR_REPORT_RETURN(trx_recv(&device.trx, data, &size, &t), "trx_recv");

    for (size_t i = 0; i < size; ++i) {
      log_printf("%02X ", data[i]);
    }
    log_printf("\r\n");
  } else {
    cmd_trx_usage();
    return SHELL_FAIL;
  }
}

SHELL_DECLARE_COMMAND(trx, cmd_trx, "TRX control");
