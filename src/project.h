/** ========================================================================= *
 *
 * @file project.h
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "shell/shell.h"
#include "drv/trx/trx.h"
#include "app/app.h"
#include "bsp.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Device context
 */
typedef struct {
  /** Board context. Peripherals, etc. */
  board_t board;

  /** LifeMonitor Application */
  app_t app;

  /** TRX Driver Handle */
  trx_t trx;

  /** CLI Shell Context */
  shell_t shell;
} device_t;

/* Variables ================================================================ */
/**
 * Global device context handle
 */
extern device_t device;

/* Shared functions ========================================================= */
/**
 * Main function of the project, runs on application startup and does all the work
 */
void project_main(void);

#ifdef __cplusplus
}
#endif
