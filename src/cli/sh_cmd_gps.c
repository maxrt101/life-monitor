/** ========================================================================= *
*
 * @file sh_cmd_gps.c
 * @date 04-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'gps' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell_util.h"
#include "shell/shell.h"
#include "hal/uart/uart.h"
#include "tty/ansi.h"
#include "log/log.h"
#include "gps/gps.h"
#include "project.h"

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static int8_t cmd_gps(shell_t * sh, uint8_t argc, const char ** argv) {
  // if (argc > 1 && !strcmp(argv[1], "test")) {
  //   gps_location_t location;
  //
  //   char buffer[128];
  //
  //   // const char * test = "$GPRMC,134800.00,A,4322.44684,N,01231.03213,E,1.314,,141022,,,A*72";
  //   const char * test = "$GNRMC,060512.00,A,3150.788156,N,11711.922383,E,0.0,,311019,,,A,V*1B";
  //   size_t test_size = strlen(test);
  //
  //   memcpy(buffer, test, test_size + 1);
  //
  //   log_printf("Test sentence: '%s'\r\n", buffer);
  //   gps_parse(&location, buffer, test_size);
  //   return SHELL_OK;
  // }


  // SHELL_ERR_REPORT_RETURN(uart_init(&device.gps.uart, &(uart_cfg_t){ .uart_no = 2, }), "uart_init");
  // uart_set_baudrate(device.gps.uart, 9600);

  log_info("Sniffing NEO6M uart traffic. Press any key to stop...");

  while (1) {
    char c = '\0';

    if (tty_get_char_async(&sh->tty, &c) == E_OK && c != '\0') {
      log_printf("\r\n");
      log_info("Stopping...");
      break;
    }

    if (app_gps_process(&device.app) == E_OK) {
      log_printf("Latitude:  %c %s\r\n",
        device.app.gps.last_location.latitude.direction,
        device.app.gps.last_location.latitude.value
      );

      log_printf("Longitude: %c %s\r\n",
        device.app.gps.last_location.longitude.direction,
        device.app.gps.last_location.longitude.value
      );
    }

  }

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(gps, cmd_gps, "GPS (NEO6M) Control");

