/** ========================================================================= *
 *
 * @file bsp.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK sleep functions
 *
 * @note Accuracy changes linearly with clock frequency
 *       (more Hz -> more accurate)
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "util/compiler.h"

/* Defines ================================================================== */
/**
 * For M0+, M4, M33 the loop takes 3 cycles:
 * 1 for SUBS, 2 for BHI.
 *
 * @note If needed this can be overridden
 */
#ifndef CORE_DELAY_US_LOOP_CYCLES
#define CORE_DELAY_US_LOOP_CYCLES 3
#endif

/* Macros =================================================================== */
#define SYSCLK_FREQ_FIRST_DENOMINATOR(__multiplicand)                          \
  (8192ULL * (__multiplicand))

#define SYSCLK_FREQ_SECOND_DENOMINATOR(__multiplicand)                         \
  ((1000000ULL * (__multiplicand)) / SYSCLK_FREQ_FIRST_DENOMINATOR(1ULL))

/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
static __NAKED __RAMFUNC
void sleep_impl(uint32_t minuend, uint32_t subtrahend)
{
  __ASM volatile("loop: \n"
                 " subs r0, r0, r1 \n"
                 " bhi loop \n"
                 " bx lr \n"
                 ::: "r0", "r1", "cc");
}

void sleep_us_port(uint16_t time_us)
{
  extern uint32_t SystemCoreClock;

  /**
   * Use faster implementation of division by SYSCLK_FREQ_FIRST_DENOMINATOR and
   * constant subtraction by SYSCLK_FREQ_SECOND_DENOMINATOR multiplied by
   * CORE_DELAY_US_LOOP_CYCLES.
   */
  sleep_impl(
        ((SystemCoreClock / SYSCLK_FREQ_FIRST_DENOMINATOR(1ULL)) * time_us),
        ((CORE_DELAY_US_LOOP_CYCLES
          * SYSCLK_FREQ_SECOND_DENOMINATOR(10000ULL))
         / 10000ULL));
}
