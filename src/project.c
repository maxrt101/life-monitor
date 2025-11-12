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
#include "app/app.h"
#include "net/net.h"
#include "project.h"
#include "bsp.h"

/* Defines ================================================================== */
#define LOG_TAG main

/* Macros =================================================================== */
/**
 * Sample macro for printing and handling errors if they occur
 */
#define ERR_CHECK(__expr)                                                     \
  do {                                                                        \
    error_t err = (__expr);                                                   \
    if (err != E_OK) {                                                        \
      log_error("%s:%d %s", __FILE__, __LINE__, error2str(err));              \
      UTIL_IF_1(USE_DEBUG, return, os_reset(OS_RESET_WDG));                   \
    }                                                                         \
  } while (0)

/**
 * Sample macro for printing errors if they occur
 */
#define ERR_CHECK_WARN(__expr, __log_str)                                     \
  do {                                                                        \
    error_t err = (__expr);                                                   \
    if (err != E_OK) {                                                        \
      log_warn(__log_str ": %s", error2str(err));                             \
    }                                                                         \
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

OS_CREATE_TASK(app,   1024, app_task_fn, NULL);
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

static void hexdump(uint8_t * data, size_t size) {
  size_t offset = 0;

  log_printf("0x%08x: ", offset);
  for (; offset < size; ++offset) {
    log_printf("%02x ", data[offset]);

    if ((offset+1) % 16 == 0) {
      log_printf("\r\n");
      if (offset + 1 != size) {
        log_printf("0x%08x: ", offset);
      }
    }
  }

  log_printf("\r\n");
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

  uint32_t vrefint = 0;
  // bsp_adc_get_vrefint(&vrefint);

  net_init(&device.net, &(net_cfg_t){
    .trx         = &device.trx,
    .dev_mac     = 0xEBAC0C42,
    .station_mac = 0xDA1BA10B,
    .key         = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    .rand_seed  = vrefint,
  });

  app_init(&device.app, &(app_cfg_t) {
    .net         = &device.net,
    .pulse_i2c   = &device.board.i2c,
    .accel_i2c   = &device.board.i2c,
    .gps_uart_no = BSP_GPS_UART_NO,
  });

  net_packet_t pkt;

  net_packet_init(&device.net, &pkt, &(net_packet_cfg_t){
    .cmd = NET_CMD_ACCEL_DATA,
    .transport = NET_TRANSPORT_TYPE_UNICAST,
    .target = 0,
  });

  pkt.payload.accel.sudden_movement = true;
  pkt.payload.accel.x = 1001;
  pkt.payload.accel.y = 5002;
  pkt.payload.accel.z = 14003;

  log_printf("Packet (%d):\r\n", pkt.size + NET_HEADER_SIZE);
  hexdump((uint8_t *) &pkt, sizeof(pkt));

  net_frame_t frame;

  net_packet_serialize(&device.net, &frame, &pkt);

  log_printf("Serialized (%d):\r\n", frame.size);
  hexdump((uint8_t *) &frame, sizeof(frame));

  net_packet_t dpkt;

  net_packet_deserialize(&device.net, &frame, &dpkt);

  log_printf("Deserialized (%d):\r\n", dpkt.size + NET_HEADER_SIZE);
  hexdump((uint8_t *) &dpkt, sizeof(dpkt));

  log_info("Starting tasks");

  os_task_start(OS_TASK(app));
  os_task_start(OS_TASK(io));
  os_task_start(OS_TASK(cli));

  log_info("Starting OS");

  // Launch into multitask
  os_launch();
}

