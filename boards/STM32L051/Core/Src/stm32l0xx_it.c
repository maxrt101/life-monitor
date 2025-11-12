/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "main.h"
#include "stm32l0xx_it.h"
#include "time/time.h"
#include "os/reset/reset.h"
#include "log/log.h"
#include "tty/ansi.h"
#include "trx/trx.h"
#include "project.h"
#include "os/os.h"
#include "bsp.h"

milliseconds_t global_runtime = 0;

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/

/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
  while (1) {}
}

/**
  * @brief This function handles Hard fault interrupt.
  */
__NAKED void HardFault_Handler(void) {
  // FIXME: For some reason L051 pushes exception frame onto PSP, which is used by tasks
  __asm volatile (
      "mrs r0, psp                 \n"
      "b HardFault_Handler_Wrapped \n"
  );
}

void HardFault_Handler_Wrapped(uint32_t * regs) {
  static const char * registers[] = {
    "R0", "R1", "R2", "R3", "R12", "LR", "PC", "PSR"
  };

  log_fatal("%s        HARD FAULT        %s", ANSI_COLOR_BG_RED, ANSI_TEXT_RESET);

  for (size_t i = 0; i < UTIL_ARR_SIZE(registers); ++i) {
    log_fatal(ANSI_TEXT_BOLD "%-5s" ANSI_TEXT_RESET ANSI_COLOR_FG_MAGENTA "0x%08x" ANSI_TEXT_RESET, registers[i], regs[i]);
  }

  log_fatal("Task: " ANSI_COLOR_FG_RED "%s" ANSI_TEXT_RESET, os_task_current()->name);

  log_fatal(ANSI_TEXT_BOLD "MSP Stacktrace:" ANSI_TEXT_RESET);
  bsp_print_stacktrace((uint32_t *)__get_MSP(), BSP_STACKTRACE_DEPTH);

  log_fatal(ANSI_TEXT_BOLD "Task Stacktrace:" ANSI_TEXT_RESET);
  bsp_print_stacktrace((uint32_t *)__get_PSP(), BSP_STACKTRACE_DEPTH);

  os_reset(OS_RESET_WDG);
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
  runtime_inc(1);
  HAL_IncTick();
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles RTC global interrupt through EXTI lines 17, 19 and 20 and LSE CSS interrupt through EXTI line 19.
 */
void RTC_IRQHandler(void) {
  if (LL_RTC_IsActiveFlag_WUT(RTC)) {
#if USE_RTC_IRQ_DEBUG_LOG
    log_printf("RTC: %d\r\n", LL_RTC_WAKEUP_GetAutoReload(RTC) / 16);
#endif

    // global_runtime += ((LL_RTC_WAKEUP_GetAutoReload(RTC) / 16) * 125) / 128;
    //
    // if (runtime_get() < global_runtime) {
    //   runtime_set(global_runtime);
    // }

    LL_RTC_ClearFlag_WUT(RTC);
    LL_PWR_ClearFlag_WU();
  }

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
}

/**
 * @brief This function handles EXTI line 0 and line 1 interrupts.
 */
void EXTI0_1_IRQHandler(void) {
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET) {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
    /** Call handler for Ra-02 DIO0 pin */
    trx_irq_handler(&device.trx);
  }
}

/**
 * @brief This function handles ADC, COMP1 and COMP2 interrupts (COMP interrupts through EXTI lines 21 and 22).
 */
void ADC1_COMP_IRQHandler(void) {}

/**
 * @brief This function handles SPI1 global interrupt.
 */
void SPI1_IRQHandler(void) { }

