/** ========================================================================= *
 *
 * @file console.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port for VFS Console
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include <string.h>

#include "bsp.h"
#include "log/log.h"
#include "vfs/vfs.h"
#include "hal/uart/uart.h"
#include "atomic/atomic.h"
#include "time/sleep.h"

/* Defines ================================================================== */
#define LOG_TAG                 console
#define CONSOLE_READ_TIMEOUT_MS 100

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
typedef struct {
  uart_t *  uart;         /** Console UART handle */
  struct {
    bool write_happend;   /** Indicates that a write happened */
  } flags;
} console_ctx_t;

static console_ctx_t console_ctx = {NULL, false};

/* Private functions ======================================================== */
error_t console_open(void * ctx, vfs_file_t * file) {
  ASSERT_RETURN(ctx, E_NULL);
  console_ctx_t * console = (console_ctx_t *) ctx;

  if (console->uart == NULL) {
    uart_cfg_t cfg = {.uart_no = CONSOLE_UART_INDEX};
    return uart_init(&console->uart, &cfg);
  }

  return E_OK;
}

error_t console_read(void * ctx, vfs_file_t * file, uint8_t * buffer, size_t size, vfs_read_flags_t flags) {
  ASSERT_RETURN(ctx, E_NULL);
  console_ctx_t * console = (console_ctx_t *) ctx;

  timeout_t t;

  if (flags & VFS_READ_FLAG_NOBLOCK) {
    timeout_start(&t, 0);
  }

  return uart_recv(
      console->uart, buffer, size,
      flags & VFS_READ_FLAG_NOBLOCK ? &t : NULL
  );
}

error_t console_write(void * ctx, vfs_file_t * file, const uint8_t * buffer, size_t size) {
  ASSERT_RETURN(ctx, E_NULL);

  console_ctx_t * console = (console_ctx_t *) ctx;

  console->flags.write_happend = true;

  error_t err = E_OK;

  ATOMIC_BLOCK() {
    err = uart_send(console->uart, buffer, size);
  }

  return err;
}

size_t console_tell(void * ctx, vfs_file_t * file) {
  ASSERT_RETURN(ctx, E_NULL);
  console_ctx_t * console = (console_ctx_t *) ctx;
  return uart_available(console->uart);
}

error_t console_ioctl(void * ctx, vfs_file_t * file, int cmd, va_list args) {
  ASSERT_RETURN(ctx, E_NULL);
  console_ctx_t * console = (console_ctx_t *) ctx;

  switch (cmd) {
    case VFS_IOCTL_SEEK: {
      break;
    }

    case VFS_IOCTL_TELL: {
      size_t * result = va_arg(args, size_t *);
      ASSERT_RETURN(result, E_INVAL);
      *result = uart_available(console->uart);
      break;
    }

    case VFS_IOCTL_RESET_DEVICE: {
      uart_reset(console->uart);
      break;
    }

    case VFS_IOCTL_WRITE_DETECTED: {
      bool * result = va_arg(args, bool*);
      *result = console->flags.write_happend;
      break;
    }

    case VFS_IOCTL_WRITE_DETECTED_CLEAR: {
      console->flags.write_happend = false;
      break;
    }

    default:
      return E_NOTIMPL;
  }

  return E_OK;
}

/* Shared functions ========================================================= */
error_t console_init(vfs_t * vfs) {
  ERROR_CHECK_RETURN(vfs_create_block(vfs, CONSOLE_FILE, &(vfs_block_data_t){
    &console_ctx,
    console_open,
    NULL,
    console_read,
    console_write,
    console_ioctl,
  }));

  VFS_WITH(vfs, console, CONSOLE_FILE) {
    vfs_set_multi_open_flag(console, true);
  }

  return E_OK;
}
