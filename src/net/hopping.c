/** ========================================================================= *
*
 * @file hopping.c
 * @date 11-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "net/hopping.h"
#include "error/assertion.h"

/* Defines ================================================================== */
#define LOG_TAG net

/* Macros =================================================================== */
/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
#if NET_USE_FULL_CHANNEL_TABLE
/** From 408000 to 458000 */
static const int8_t HOPPING_OFFSET_TABLE[32] = {
  -35,  12,  43, -25, -18, -33,  11,  22,
   -5, -20, -21,  45, -38,   4, -32,  25,
   30, -22, -28,  26, -34,  29,  39,  32,
  -41,  47, -14, -40, -48,  13,  31,  10
};
#else
/** Single band 433000 */
static const uint8_t HOPPING_OFFSET_TABLE[1] = {
  0
};
#endif

/* Private functions ======================================================== */
/* Shared functions ========================================================= */
error_t net_hopping_init(net_hopping_t * hopping) {
  ASSERT_RETURN(hopping, E_NULL);

  hopping->base_freq_khz    = NET_CHANNEL_BASE_FREQ_KHZ;
  hopping->channels.offsets = HOPPING_OFFSET_TABLE;
  hopping->channels.size    = UTIL_ARR_SIZE(HOPPING_OFFSET_TABLE);
  hopping->channels.index   = 0;

  return E_OK;
}

uint32_t net_hopping_get_hop_freq(net_hopping_t * hopping) {
  ASSERT_RETURN(hopping, 0);

  return hopping->base_freq_khz + hopping->channels.offsets[hopping->channels.index] * NET_CHANNEL_STEP_KHZ;
}

uint32_t net_hopping_get_base_freq(net_hopping_t * hopping) {
  ASSERT_RETURN(hopping, 0);

  return hopping->base_freq_khz;
}

error_t net_hopping_switch_channel(net_hopping_t * hopping) {
  ASSERT_RETURN(hopping, E_NULL);

  hopping->channels.index = ++hopping->channels.index % hopping->channels.size;

  return E_OK;
}
