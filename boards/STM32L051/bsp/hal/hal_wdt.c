/** ========================================================================= *
*
 * @file hal_wdt.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK WDT HAL
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "hal/wdt/wdt.h"
#include "iwdg.h"
#include "bsp.h"

/* Defines ================================================================== */
#define LOG_TAG hal_wdt

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
void wdt_init(void) {
#if USE_WDG
  MX_IWDG_Init();
#endif
}

void wdt_feed(void) {
#if USE_WDG
  LL_IWDG_ReloadCounter(IWDG);
#endif
}

__NORETURN void wdt_reboot(void) {
#if USE_WDG
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetReloadCounter(IWDG, 0);
  LL_IWDG_DisableWriteAccess(IWDG);
  LL_IWDG_ReloadCounter(IWDG);
#endif
  while (1) {}
}

