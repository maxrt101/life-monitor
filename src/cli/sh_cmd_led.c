/** ========================================================================= *
*
 * @file sh_cmd_led.c
 * @date 13-11-2024
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief 'led' CLI Command implementation
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "shell/shell.h"
#include "app/app.h"
#include "log/log.h"
#include <string.h>
#include <project.h>

#include "shell/shell_util.h"

/* Defines ================================================================== */
#define LOG_TAG shell

/* Macros =================================================================== */
#define LED_CHECK(led)            \
    do {                          \
      if (!led) {                 \
        log_error("No such LED"); \
        return SHELL_FAIL;        \
      }                           \
    } while (0)

/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
__STATIC_INLINE led_t * get_led(const char * name) {
  ASSERT_RETURN(name[0] >= '0' && name[0] <= '9', NULL);
  int index = name[0] - '0';
  ASSERT_RETURN(index >= 0 && index < BSP_LED_COUNT, NULL);
  return &device.board.leds[index];
}

/* Shared functions ========================================================= */
static int8_t cmd_led(shell_t * sh, uint8_t argc, const char ** argv) {
  SHELL_ASSERT_REPORT_RETURN(argc == 3, "Usage: led NAME on|off");


  if (!strcmp(argv[1], "on") || !strcmp(argv[1], "1")) {
    led_t * led = get_led(argv[2]);
    LED_CHECK(led);
    led_on(led);
  } else if (!strcmp(argv[1], "off") || !strcmp(argv[1], "0")) {
    led_t * led = get_led(argv[2]);
    LED_CHECK(led);
    led_off(led);
  }

  return SHELL_OK;
}

SHELL_DECLARE_COMMAND(led, cmd_led, "LED control");