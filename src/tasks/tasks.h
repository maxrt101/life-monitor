/** ========================================================================= *
 *
 * @file tasks.h
 * @date 27-09-2024
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 * @copyright GrainMole
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "os/os.h"

/* Defines ================================================================== */
/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/** Declare app task handle */
OS_DECLARE_TASK(app);

/** Declare io task handle */
OS_DECLARE_TASK(io);

/** Declare cli task handle */
OS_DECLARE_TASK(cli);

/* Shared functions ========================================================= */
/**
 * Worker for Detector Task
 */
void app_task_fn(void * ctx);

/**
 * Worker for IO Task
 */
void io_task_fn(void * ctx);

/**
 * Worker for Cli Task
 */
void cli_task_fn(void * ctx);

#ifdef __cplusplus
}
#endif