/** ========================================================================= *
 *
 * @file bsp.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "bsp.h"
#include "project.h"
#include "error/assertion.h"
#include "queue/queue.h"
#include "os/alloc/alloc.h"
#include "wdt/wdt.h"
#include "gpio/gpio.h"
#include "tty/ansi.h"
#include "log/log.h"

/* Defines ================================================================== */
#define LOG_TAG bsp

#define LED_QUEUE_SIZE 4

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/** Start of os heap, defined in linker */
extern uint32_t __os_heap_start;

/** End of os heap, defined in linker */
extern uint32_t __os_heap_end;

/** Queue for On-Board Green LED */
QUEUE_DEFINE(led_green_queue, LED_QUEUE_SIZE);

static os_heap_t heap;

/* Private functions ======================================================== */
error_t console_init(vfs_t * vfs);

/* Shared functions ========================================================= */
void bsp_init(void) {
  HAL_MspInit();

  // Initialize OS Heap
  os_heap_create(&heap, &__os_heap_start, (uint8_t *) &__os_heap_end - (uint8_t *) &__os_heap_start);
  os_use_heap(&heap);

  led_init(
    &device.leds[0],
    GPIO_TYPE_BIND(LED_GREEN),
    GPIO_POL_POSITIVE,
    &led_green_queue
  );

  btn_cfg_t cfg = {0};

  gpio_ctx_init(&cfg.gpio, GPIO_TYPE_BIND(BUTTON), GPIO_POL_POSITIVE);
  cfg.press_time = 1000;

  // Initialize BTN instance
  btn_init(&device.btns[0], &cfg);

  // Initialize RTC
  LL_RTC_DisableWriteProtection(RTC);
  LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_2);
  LL_RTC_EnableIT_WUT(RTC);
  LL_RTC_WAKEUP_Enable(RTC);
  LL_RTC_EnableWriteProtection(RTC);

  bsp_set_next_wakeup(300);

  wdt_init();
  wdt_feed();
}

error_t bsp_init_vfs_files(vfs_t * vfs) {
  ERROR_CHECK_RETURN(console_init(vfs));

  return E_OK;
}

void bsp_set_next_wakeup(milliseconds_t period) {
  // RTC has 32768 ticks per second, 1ms = 32 ticks.
  // The RTC divider is 2, so 1ms = 16 ticks.
  int32_t period_ticks = ((int32_t)period) * 16 - 1;

#if USE_RTC_DEBUG_LOG
  log_warn("RTC WUP=%dms (%dt)", period, period_ticks);
#endif

  LL_RTC_DisableWriteProtection(RTC);

  LL_RTC_WAKEUP_Disable(RTC);
  while (!LL_RTC_IsActiveFlag_WUTW(RTC)) {}

  LL_RTC_EnableIT_WUT(RTC);

  /* Reconfigure the Wakeup Timer counter */
  LL_RTC_WAKEUP_SetAutoReload(RTC, period_ticks);
  LL_RTC_WAKEUP_Enable(RTC);

  LL_RTC_EnableWriteProtection(RTC);
}

milliseconds_t bsp_get_wakeup(void) {
  return LL_RTC_WAKEUP_GetAutoReload(RTC) / 16;
}

void bsp_print_stacktrace(uint32_t * sp, uint32_t depth) {
  uint32_t found = 0;

  while ((uint32_t)sp > SRAM_BASE && (uint32_t)sp < SRAM_BASE + SRAM_SIZE_MAX && found < depth) {
    uint32_t value = *sp;
    if (value >= FLASH_BASE && value < FLASH_END) {
      log_print_raw(LOG_FATAL, "#%02d: " ANSI_COLOR_FG_CYAN "0x%08x" ANSI_TEXT_RESET, found++, value);
    }

    sp++;
  }
}
