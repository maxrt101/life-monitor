/** ========================================================================= *
 *
 * @file os_port.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK sys library
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "os/irq/irq.h"
#include "os/power/power.h"
#include "os/reset/reset.h"
#include "util/util.h"
#include "log/log.h"
#include "wdt/wdt.h"
#include "stm32l073xx.h"
#include "stm32l0xx.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_adc.h"
#include "project.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* From main.c */
void SystemClock_Config(void);

/* Shared functions ========================================================= */
void os_irq_enable_port(uint8_t irq) {
  if (irq == OS_IRQ_ALL) {
    __enable_irq();
  } else {
    HAL_NVIC_EnableIRQ(irq);
  }
}

void os_irq_disable_port(uint8_t irq) {
  if (irq == OS_IRQ_ALL) {
    __disable_irq();
  } else {
    HAL_NVIC_DisableIRQ(irq);
  }
}

void os_on_abort(void) {
#if USE_LED_ERROR_ON_ABORT
  // led_stop(APP_LED(&device.app));
  // led_flush(APP_LED(&device.app));
  // led_allow_repeat(APP_LED(&device.app), false);
  // led_schedule(APP_LED(&device.app), LED_PATTERN(error));
  // led_run(APP_LED(&device.app));
#endif
}

__NO_RETURN void os_reset_port(os_reset_method_t method) {
  switch (method) {
    case OS_RESET_WDG:
      UTIL_IF_1(USE_DEBUG, __BKPT(0));
      wdt_reboot();
      break;

    case OS_RESET_HARD:
    case OS_RESET_SOFT:
    default:
      NVIC_SystemReset();
  }

  while (1) {
    /* Wait for reset */
  }
}

os_reset_reason_t os_get_reset_reason_port(void) {
  os_reset_reason_t reason = OS_RESET_REASON_UNK;

  if (LL_RCC_IsActiveFlag_IWDGRST()) {
    reason = OS_RESET_REASON_WDG;
  } else if (LL_RCC_IsActiveFlag_WWDGRST()) {
    reason = OS_RESET_REASON_WWDG;
  } else if (LL_RCC_IsActiveFlag_SFTRST()) {
    reason = OS_RESET_REASON_SW_RST;
  } else if (LL_RCC_IsActiveFlag_PINRST()) {
    reason = OS_RESET_REASON_HW_RST;
  } else if (LL_RCC_IsActiveFlag_PORRST()) {
    reason = OS_RESET_REASON_POR;
  }

  return reason;
}

error_t os_power_mode_change_port(os_power_mode_t mode) {
  switch (mode) {
    case OS_POWER_MODE_FAST_SLEEP:
      HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      break;

    case OS_POWER_MODE_DEEP_SLEEP:
#if USE_ULTRALOWPOWER
      // TODO: Check if ULTRALOWPOWER works
      if (LL_ADC_IsEnabled(ADC1))
      {
        // TODO: Should save ADC state?
        LL_ADC_Disable(ADC1);

        /* TODO: Check this on L051
         * ADC regulator consumes power in STOP mode, so we need to disable it
         * before entering STOP mode.
         */
        LL_ADC_DisableInternalRegulator(ADC1);
        LL_PWR_EnableUltraLowPower();
      }
#endif
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

      /* Clocks are disabled in deep sleep, so re-enable them
       * TODO: Try LL_RCC_SetClkAfterWakeFromStop */
      SystemClock_Config();
#if USE_ULTRALOWPOWER
      // TODO: Should call LL_ADC_Enable?
      LL_PWR_DisableUltraLowPower();
#endif
      break;

    default:
      return E_INVAL;
  }

  return E_OK;
}

void os_prepare_scheduler_stack_port(void) {
  extern uint32_t _estack;

  // Switch MSP & PSP
  __set_PSP(__get_MSP());

  // Use PSP as SP
  __set_CONTROL(1 << CONTROL_SPSEL_Pos);

  // Set MSP to IRQ stack
  __set_MSP((uint32_t) &_estack);

  // Flush pipeline
  __ISB();
}

void os_set_stack_port(void * stack) {
  // Set PSP to new value
  __set_PSP((uint32_t) stack);

  // Flush pipeline
  __ISB();
}
