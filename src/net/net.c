/** ========================================================================= *
*
 * @file net.c
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network Protocol
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "net/net.h"
#include "error/assertion.h"
#include <stdlib.h>

/* Defines ================================================================== */
#define LOG_TAG net
#define CRC16_INIT 0x42

/* Macros =================================================================== */
#define STATUS_LED_CTL(__net, __on)                                           \
  do {                                                                        \
    if (__net->status_led) {                                                  \
      if (__on) {                                                             \
        led_on(__net->status_led);                                            \
      } else {                                                                \
        led_off(__net->status_led);                                           \
      }                                                                       \
    }                                                                         \
  } while (0)

/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
/* Shared functions ========================================================= */
error_t net_init(net_t * net, net_cfg_t * cfg) {
  ASSERT_RETURN(net && cfg, E_NULL);

  memset(net, 0, sizeof(net_t));

  net->dev_mac.value     = cfg->dev_mac.value;
  net->station_mac.value = cfg->station_mac.value;
  net->trx               = cfg->trx;
  net->packet_id         = 0;
  net->status_led        = cfg->status_led;

  memcpy(net->key, cfg->key, NET_KEY_SIZE);

  ERROR_CHECK_RETURN(net_hopping_init(&net->hopping));

  srand(cfg->rand_seed);

  return E_OK;
}

error_t net_packet_send(net_t * net, net_packet_t * packet) {
  ASSERT_RETURN(net && packet, E_NULL);

  net_frame_t frame;

  ERROR_CHECK_RETURN(net_packet_serialize(net, &frame, packet));

  STATUS_LED_CTL(net, true);
  error_t err = trx_send(net->trx, frame.data, frame.size);
  STATUS_LED_CTL(net, false);

  return err;
}

error_t net_packet_recv(net_t * net, net_packet_t * packet, timeout_t * timeout) {
  ASSERT_RETURN(net && packet && timeout, E_NULL);

  net_frame_t frame;
  size_t size = TRX_MAX_PACKET_SIZE;

  STATUS_LED_CTL(net, true);
  error_t err = trx_recv(net->trx, frame.data, &size, timeout);
  STATUS_LED_CTL(net, false);

  ERROR_CHECK_RETURN(err);

  frame.size = size;

  return net_packet_deserialize(net, &frame, packet);
}

error_t net_send(net_t * net, net_packet_t * packet, net_packet_t * response, uint8_t repeats) {
  ASSERT_RETURN(net && packet, E_NULL);

  bool base = true;
  trx_set_freq(net->trx, net_hopping_get_base_freq(&net->hopping));

  do {
    ERROR_CHECK_RETURN(net_packet_send(net, packet));

    TIMEOUT_CREATE(t, NET_RECV_TIMEOUT);

    if (net_packet_recv(net, response, &t) == E_OK) {
      return E_OK;
    }

    packet->repeat++;

    base = !base;

    trx_set_freq(
      net->trx,
      base
        ? net_hopping_get_base_freq(&net->hopping)
        : net_hopping_get_hop_freq(&net->hopping)
    );
  } while (packet->repeat != repeats);

  return E_NORESP;
}

uint32_t net_rand(net_t * net, uint32_t min, uint32_t max) {
  ASSERT_RETURN(net, 0);
  ASSERT_RETURN(max > min, 0);

  return min + (rand() % (max - min - 1));
}

uint16_t net_crc(uint8_t * data, uint8_t size) {
  ASSERT_RETURN(data, 0);
  ASSERT_RETURN(size, 0);

  uint8_t  x;
  uint16_t crc = CRC16_INIT;

  while (size--) {
    x = crc >> 8 ^ *data++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }

  return crc;
}

error_t net_frame_encrypt(net_t * net, net_frame_t * frame) {
  ASSERT_RETURN(net && frame, E_NULL);
  ASSERT_RETURN(frame->size < NET_HEADER_SIZE + 2, E_INVAL); // Header + CRC

  uint8_t data[NET_PACKET_MAX_SIZE];

  memcpy(data, frame->data, frame->size);

  uint8_t salt[2] = {
    net_rand(net, 1, 255),
    net_rand(net, 1, 255),
  };

  for (uint8_t i = 0; i < frame->size; ++i) {
    data[i] ^= net->key[(salt[0] + i) % NET_KEY_SIZE] ^ salt[1];
  }

  frame->data[0] = salt[0];
  frame->data[1] = salt[1];

  memcpy(frame->data + 2, data, frame->size);

  frame->size += 2;

  return E_OK;
}

error_t net_frame_decrypt(net_t * net, net_frame_t * frame) {
  ASSERT_RETURN(net && frame, E_NULL);
  ASSERT_RETURN(frame->size < NET_HEADER_SIZE + 4, E_INVAL); // Header + CRC + salt

  uint8_t data[NET_PACKET_MAX_SIZE];

  memcpy(data, frame->data + 2, frame->size - 2);

  uint8_t salt[2] = {
    frame->data[0],
    frame->data[1],
  };

  for (uint8_t i = 0; i < frame->size; ++i) {
    data[i] ^= net->key[(salt[0] + i) % NET_KEY_SIZE] ^ salt[1];
  }

  memcpy(frame->data, data, frame->size - 2);

  frame->size -= 2;

  return E_OK;
}
