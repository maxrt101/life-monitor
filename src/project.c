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

#if USE_SHELL_HISTORY
error_t tty_process_ansi_csi_custom(tty_t * tty, tty_line_t * line, char c) {
  // Call shell history handler to parse UP/DOWN arrow keys
  return shell_history_process_ansi_csi(tty, line, c);
}
#endif

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
#endif

  log_info("Initializing...");
  log_info("Reset reason: %s", os_reset_reason_to_str(os_get_reset_reason()));

  init_radio();

  log_info("Starting tasks");

  os_task_start(OS_TASK(cli));

  log_info("Starting OS");

  // Launch into multitask
  os_launch();
}

