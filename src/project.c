/** ========================================================================= *
 *
 * @file project.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "project.h"
#include "bsp.h"
#include "drv/trx/trx.h"
#include "log/log.h"
#include "tty/ansi.h"
#include "shell/shell.h"
#include "vfs/vfs.h"
#include "os/reset/reset.h"
#include "os/os.h"
#include "os/power/power.h"
#include "wdt/wdt.h"

/* Defines ================================================================== */
#define LOG_TAG main

/* Macros =================================================================== */
/**
 * Sample macro for printing and handling errors if they occur
 */
#define ERR_CHECK(expr)                                                   \
  do {                                                                    \
    error_t err = (expr);                                                 \
    if (err != E_OK) {                                                    \
      log_error("%s:%d %s", __FILE__, __LINE__, error2str(err));          \
      UTIL_IF_1(USE_DEBUG, return, os_reset(OS_RESET_WDG));               \
    }                                                                     \
  } while (0)

/**
 * Sample macro for printing errors if they occur
 */
#define ERR_CHECK_WARN(expr, log_str)                                     \
  do {                                                                    \
    error_t err = (expr);                                                 \
    if (err != E_OK) {                                                    \
      log_warn(log_str ": %s", error2str(err));                           \
    }                                                                     \
  } while (0)

/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/** Global device context handle */
device_t device;

/** VFS Node Pool */
VFS_DECLARE_NODE_POOL(vfs_node_pool, 8);

/** VFS Table Pool */
VFS_DECLARE_TABLE_POOL(vfs_table_pool, 8);

/* Private functions ======================================================== */
void trx_on_waiting(__UNUSED trx_t * trx) {
#if USE_TRX_YIELD_ON_WAIT
  wdt_feed();
  os_yield();
#else
  wdt_feed();
  os_power_mode_change(OS_POWER_MODE_FAST_SLEEP);
#endif
}

__STATIC_INLINE void init_radio(void) {
  // Initialize LoRa SPI
  spi_cfg_t lora_spi_cfg = {
    .spi_no = RA02_SPI_INDEX,
    .cs     = GPIO_TO_TYPE(BSP_LORA_CS)
  };

  ERR_CHECK(spi_init(&device.trx_spi, &lora_spi_cfg));

  // Initialize LoRa
  trx_cfg_t trx_cfg = {
    .ra02.spi = &device.trx_spi,
    .ra02.reset = GPIO_TO_TYPE(BSP_LORA_RESET)
  };

  ERR_CHECK(trx_sx1278_init(&device.trx));
  ERR_CHECK(trx_init(&device.trx, &trx_cfg));

  // Set LoRa parameters
  ERR_CHECK(trx_set_freq(&device.trx, 433000));
  ERR_CHECK(trx_set_power(&device.trx, 20));
  ERR_CHECK(trx_set_preamble(&device.trx, 10));
  ERR_CHECK(trx_set_bandwidth(&device.trx, 125000));
}

#include "os/mutex.h"
#include "os/event.h"

#if TEST_OS_MUTEX
OS_CREATE_MUTEX(test_mutex);

void test_task1(void * ctx) {
  while (1) {
    os_mutex_lock(&test_mutex, OS_MUTEX_WAIT_FOREVER);
    log_info("task1 acquired mutex at %d", runtime_get());
    os_delay(100);
    os_mutex_unlock(&test_mutex);
    os_yield();
  }
}

OS_CREATE_TASK(task1, 1024, test_task1, NULL);

void test_task2(void * ctx) {
  while (1) {
    os_mutex_lock(&test_mutex, OS_MUTEX_WAIT_FOREVER);
    log_info("task2 acquired mutex at %d", runtime_get());
    os_delay(100);
    os_mutex_unlock(&test_mutex);
    os_yield();
  }
}

OS_CREATE_TASK(task2, 1024, test_task2, NULL);

void test_task3(void * ctx) {
  while (1) {
    os_mutex_lock(&test_mutex, OS_MUTEX_WAIT_FOREVER);
    log_info("task3 acquired mutex at %d", runtime_get());
    os_delay(100);
    os_mutex_unlock(&test_mutex);
    os_yield();
  }
}

OS_CREATE_TASK(task3, 1024, test_task3, NULL);
#endif

#if TEST_OS_EVENT
OS_CREATE_EVENT(event1);

void test1_task_fn(void * ctx) {
  os_event_subscribe(&event1);

  while (1) {
    os_event_wait(&event1);
    log_info("task1 triggered by event1 (%d)", runtime_get());
  }
}

OS_CREATE_TASK(test1, 1024, test1_task_fn, NULL);

void test2_task_fn(void * ctx) {
  while (1) {
    log_info("Triggering event1 (%d)", runtime_get());
    os_event_trigger(&event1);
    os_delay(500);
  }
}

OS_CREATE_TASK(test2, 1024, test2_task_fn, NULL);
#endif

#if TEST_OS_DYNAMIC_TASKS
void spawned(void * ctx) {
  log_info("Task '%s' is spawned (%u)", os_task_current()->name, runtime_get());
  os_delay(1000);
  log_info("Task '%s' is exiting (%u)", os_task_current()->name, runtime_get());
}

void spawner(void * ctx) {
  OS_CREATE_TASK(spawned, 1024, spawned, NULL);

  log_info("Spawning task (%u)", runtime_get());
  os_task_start(OS_TASK(spawned));

  log_info("Waiting for task to exit (%u)", runtime_get());
  os_wait_task(OS_TASK(spawned));

  log_info("Task exited (%u)", runtime_get());

  while (1) {
    os_yield();
  }
}

OS_CREATE_TASK(spawner, 2048, spawner, NULL);
#endif

void cli_task_fn(__UNUSED void * ctx) {
  shell_init(&device.shell, vfs_open(&vfs, CONSOLE_FILE), NULL);

  shell_start(&device.shell);

  os_yield();

  while (1) {
    shell_process(&device.shell);
    os_yield();
  }
}

OS_CREATE_TASK(cli, 1024, cli_task_fn, NULL);

/* Shared functions ========================================================= */
void project_main(void) {
  // Initialize BSP
  bsp_init();

  // Initialize root vfs
  vfs_init(&vfs, &vfs_node_pool, &vfs_table_pool);

  // Create /dev folder
  vfs_mkdir(&vfs, "/dev");

  bsp_init_vfs_files(&vfs);

  // Initialize log
  log_init(vfs_open(&vfs, CONSOLE_FILE));

#if USE_PRINT_STARTUP_INFO
  log_printf(
    "\r\n\r\n%s----- %s v%s (%s) -----%s\r\n",
     ANSI_COLOR_FG_YELLOW,
     PROJECT_NAME,
     PROJECT_VERSION,
     PROJECT_COMMIT,
     ANSI_TEXT_RESET
  );

  log_printf(
    "\r\nBuilt on %s%s %s%s by %s%s%s with %s%s %s%s\r\n\r\n",
    ANSI_TEXT_BOLD, __TIME__, __DATE__, ANSI_TEXT_RESET,
    ANSI_TEXT_BOLD, PROJECT_COMPILED_BY, ANSI_TEXT_RESET,
    ANSI_TEXT_BOLD, PROJECT_COMPILED_WITH, __VERSION__, ANSI_TEXT_RESET
  );

  // log_info("Resset reason:  %s", os_reset_reason_to_str(os_get_reset_reason()));
#endif

  init_radio();

  // log_info("Starting tasks");

  // os_task_start(&app_task);
  // os_task_start(&io_task);

  // os_task_start(&cli_task);

  // log_info("Starting OS");

  // Launch into multitask
  // os_launch();

  // Shouldn't return
  // os_abort("Scheduler returned (shouldn't happen)");

#if TEST_OS_MUTEX
  os_task_start(&task1_task);
  os_task_start(&task2_task);
  os_task_start(&task3_task);
#endif

#if TEST_OS_EVENT
  os_task_start(OS_TASK(test1));
  os_task_start(OS_TASK(test2));
#endif

#if TEST_OS_DYNAMIC_TASKS
  os_task_start(OS_TASK(spawner));
#endif

  os_task_start(OS_TASK(cli));

  os_launch();
}

