/** ========================================================================= *
 *
 * @file storage.c
 * @date 03-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "storage/storage.h"
#include "error/assertion.h"
#include "hal/nvm/nvm.h"
#include <string.h>

/* Defines ================================================================== */
#define LOG_TAG       storage
#define RAW_CRC_POLY  0x31

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
__STATIC_INLINE uint8_t raw_nvm_crc(const uint8_t * buffer, size_t size) {
  ASSERT_RETURN(buffer && size, 0);

  uint16_t crc = 0;

  for (size_t i = 0; i < size; ++i) {
    crc ^= buffer[i];
    for (uint8_t j = 0; j < 8; ++j) {
      crc = crc & 0x80
          ? (crc << 1) ^ RAW_CRC_POLY
          : (crc << 1);
    }
  }

  return crc;
}

__STATIC_INLINE uint8_t storage_crc(storage_data_t * storage) {
  return raw_nvm_crc(
      (uint8_t *) storage,
      sizeof(storage_data_t) - sizeof(storage->crc)
  );
}

/* Shared functions ========================================================= */
error_t storage_read(storage_data_t * storage) {
#if USE_MOCK_STORAGE
  return E_CORRUPT;
#else
  ASSERT_RETURN(storage, E_NULL);

  memcpy(storage, __storage_start, sizeof(storage_data_t));

  return storage_crc(storage) == storage->crc ? E_OK : E_CORRUPT;
#endif
}

error_t storage_write(storage_data_t * storage) {
#if USE_MOCK_STORAGE
  return E_OK;
#else
  ASSERT_RETURN(storage, E_NULL);

  storage->crc = storage_crc(storage);

  nvm_erase((uint32_t) __storage_start, sizeof(storage_data_t));

  return nvm_write((uint32_t) __storage_start, (uint8_t *) storage, sizeof(storage_data_t));
#endif
}
