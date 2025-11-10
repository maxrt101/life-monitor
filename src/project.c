/** ========================================================================= *
 *
 * @file project.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "drv/trx/trx.h"
#include "shell/shell.h"
#include "tty/ansi.h"
#include "log/log.h"
#include "vfs/vfs.h"
#include "os/reset/reset.h"
#include "os/power/power.h"
#include "os/os.h"
#include "wdt/wdt.h"
#include "tasks/tasks.h"
#include "project.h"
#include "bsp.h"

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

OS_CREATE_TASK(app,   256,  app_task_fn, NULL);
OS_CREATE_TASK(pulse, 256,  pulse_task_fn, NULL);
OS_CREATE_TASK(accel, 256,  accel_task_fn, NULL);
OS_CREATE_TASK(gps,   256,  gps_task_fn, NULL);
OS_CREATE_TASK(cli,   1024, cli_task_fn, NULL);
OS_CREATE_TASK(io,    128,  io_task_fn,  NULL);

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
  // Initialize LoRa
  trx_cfg_t trx_cfg = {
    .sx1278.spi = &device.board.trx_spi,
    .sx1278.reset = GPIO_TO_TYPE(BSP_LORA_RESET)
  };

  ERR_CHECK(trx_sx1278_init(&device.trx));
  ERR_CHECK(trx_init(&device.trx, &trx_cfg));

  // Set LoRa parameters
  ERR_CHECK(trx_set_freq(&device.trx, 433000));
  ERR_CHECK(trx_set_power(&device.trx, 20));
  ERR_CHECK(trx_set_preamble(&device.trx, 10));
  ERR_CHECK(trx_set_bandwidth(&device.trx, 125000));
}

__STATIC_INLINE void init_pulse(void) {
  ERR_CHECK(
    max3010x_init(
      &device.pulse.max3010x,
      &(max3010x_cfg_t){
        .i2c = &device.board.i2c,
        .adc_range.max30102 = MAX30102_ADC_RANGE_2K_nA,
        .pulse_width = {
          .max30100 = MAX30100_PULSE_WIDTH_1600_ADC_16_BIT,
          .max30102 = MAX30102_PULSE_WIDTH_118_ADC_16_BIT,
        },
        .sample_rate = {
          .max30100 = MAX30100_SAMPLE_RATE_100_HZ,
          .max30102 = MAX30102_SAMPLE_RATE_100_HZ,
        },
        .current = {
          .ir  = 50,
          .red = 50
        },
        .mode = MAX3010X_MODE_HEART_RATE
      }
    )
  );

  ERR_CHECK(
    pulse_init(
      &device.pulse.ctx,
      max3010x_get_min_ir_adc_voltage(&device.pulse.max3010x),
      500
    )
  );
}

__STATIC_INLINE void init_pos(void) {
  ERR_CHECK(
    mpu6050_init(
      &device.pos.mpu6050,
      &(mpu6050_cfg_t) {
        .i2c   = &device.board.i2c,
        .gyro  = MPU6050_GYRO_FS_SEL_1000_DEG_PER_S,
        .accel = MPU6050_ACCEL_AFS_SEL_16G,
      }
    )
  );

  ERR_CHECK(
    acceleration_monitor_init(&device.pos.monitor)
  );
}

__STATIC_INLINE void init_gps(void) {
  ERR_CHECK(uart_init(&device.gps.uart, &(uart_cfg_t){ .uart_no = 2 }));
  ERR_CHECK(uart_set_baudrate(device.gps.uart, 9600));
}

/* Shared functions ========================================================= */
void project_main(void) {
  // Initialize BSP
  bsp_init(&device.board);

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
  init_pulse();
  init_pos();
  init_gps();

  log_info("Starting tasks");

  os_task_start(OS_TASK(app));
  os_task_start(OS_TASK(pulse));
  os_task_start(OS_TASK(accel));
  os_task_start(OS_TASK(gps));
  os_task_start(OS_TASK(io));
  os_task_start(OS_TASK(cli));

  log_info("Starting OS");

  // Launch into multitask
  os_launch();
}

