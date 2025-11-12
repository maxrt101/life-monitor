/** ========================================================================= *
 *
 * @file hal_nvm.c
 * @date 27-10-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief Port implementation of SDK NVM HAL API
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "hal/nvm/nvm.h"
#include "log/log.h"
#include <string.h>

/* Defines ================================================================== */
#define LOG_TAG        hal_nvm
#define NVM_PAGE_SIZE  128
#define NVM_ALIGN      1UL
#define NVM_ALIGN_MASK ((1 << NVM_ALIGN) - 1)


/* Macros =================================================================== */
#define NVM_ADDR_ROUND_UP(addr) \
    ((addr) - ((addr) % NVM_PAGE_SIZE))

#define NVM_ADDR_ROUND_DOWN(addr) \
    ((addr) + (NVM_PAGE_SIZE - ((addr) % NVM_PAGE_SIZE)))

#define NVM_BYTES_TO_PAGES(size) \
    ((size) / NVM_PAGE_SIZE + ((size) % NVM_PAGE_SIZE ? 1 : 0))

/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
uint32_t nvm_get_page_size(void) {
  return NVM_PAGE_SIZE;
}

error_t nvm_erase_page(uint32_t addr) {
  error_t err = E_OK;
  uint32_t failed_page;

  addr = NVM_ADDR_ROUND_UP(addr);

  FLASH_EraseInitTypeDef init = {
    .TypeErase   = FLASH_TYPEERASE_PAGES,
    .PageAddress = addr,
    .NbPages     = 1,
  };

  HAL_FLASH_Unlock();

  if (HAL_FLASHEx_Erase(&init, &failed_page) != HAL_OK) {
    log_error("Page erase %d@%p failed: error 0x%X", addr, failed_page, HAL_FLASH_GetError());
    err = E_FAILED;
  }

  HAL_FLASH_Lock();

  return err;
}

error_t nvm_erase(uint32_t addr, uint32_t size) {
  addr = NVM_ADDR_ROUND_UP(addr);
  uint32_t pages = NVM_BYTES_TO_PAGES(size);

  for (uint32_t i = 0; i < pages; ++i) {
    ERROR_CHECK_RETURN(nvm_erase_page(addr + i * NVM_PAGE_SIZE));
  }

  return E_OK;
}

error_t nvm_write(uint32_t addr, uint8_t * buffer, uint32_t size) {
  uint32_t buf[1] __ALIGNED(NVM_ALIGN) = {0};

  HAL_FLASH_Unlock();

  while (size) {
    // Calculate offsets and base address in flash.
    uint32_t flash_addr_align = (addr & NVM_ALIGN_MASK);
    uint32_t base_flash_addr  = (addr & ~(NVM_ALIGN_MASK));

    // Copy previous FLASH content
    memcpy(buf, (const void *) base_flash_addr, sizeof(buf));

    // Get the number of bytes to copy
    uint32_t amount = ((sizeof(buf) - flash_addr_align) <= size)
                            ? sizeof(buf) - flash_addr_align
                            : size;

    memcpy(((uint8_t *)buf) + flash_addr_align, buffer, amount);
    buffer += amount;
    size -= amount;
    addr += amount;

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, base_flash_addr, buf[0]) != HAL_OK) {
      log_error("Programming at %p failed: error 0x%X", addr, HAL_FLASH_GetError());
    }
  }

  HAL_FLASH_Lock();

  return E_OK;
}
