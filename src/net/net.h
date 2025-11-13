/** ========================================================================= *
*
 * @file net.h
 * @date 08-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network Protocol
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "error/error.h"
#include "net/hopping.h"
#include "net/packet.h"
#include "net/types.h"
#include "trx/trx.h"
#include <stdint.h>

#include "led/led.h"

/* Defines ================================================================== */
/** Default number of repeats for packets, that require an answer */
#ifndef NET_REPEATS
#define NET_REPEATS 6
#endif

/** Default listening timeout for packets, that require an answer */
#ifndef NET_RECV_TIMEOUT
#define NET_RECV_TIMEOUT 100
#endif

/** Default period for sending statuses */
#ifndef NET_STATUS_SEND_PERIOD
#define NET_STATUS_SEND_PERIOD 5000
#endif

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/**
 * Network context
 */
typedef struct net_t {
  net_mac_t     dev_mac;      /** Device MAC */
  net_mac_t     station_mac;  /** Station MAC */
  net_key_t     key;          /** Encryption Key */
  uint16_t      packet_id;    /** Current Packet ID */
  trx_t *       trx;          /** TRX Interface to send/recv data through */
  net_hopping_t hopping;      /** Network Hopping Context */
  led_t *       status_led;   /** LED instance that signals TRX work */
} net_t;

/**
 * Network Config
 */
typedef struct net_cfg_t {
  trx_t *   trx;          /** TRX Interface to send/recv data through */
  net_mac_t dev_mac;      /** Device MAC */
  net_mac_t station_mac;  /** Station MAC */
  net_key_t key;          /** Encryption Key */
  led_t *   status_led;   /** LED instance that signals TRX work */
  uint32_t  rand_seed;    /** Seed for RNG */
} net_cfg_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Initialize Network Context
 *
 * @param net Network Context
 * @param cfg Network Config
 */
error_t net_init(net_t * net, net_cfg_t * cfg);

/**
 * Send packet
 *
 * @param net    Network Context
 * @param packet Packet to send
 */
error_t net_packet_send(net_t * net, net_packet_t * packet);

/**
 * Receive packet
 *
 * @param net     Network Context
 * @param packet  Buffer for received packet
 * @param timeout Timeout to listen for packet for
 */
error_t net_packet_recv(net_t * net, net_packet_t * packet, timeout_t * timeout);

/**
 * Send packet and listen for response
 *
 * @param net      Network Context
 * @param packet   Packet to send
 * @param response Buffer for received packet
 * @param repeats  Number of repeats
 */
error_t net_send(net_t * net, net_packet_t * packet, net_packet_t * response, uint8_t repeats);

/**
 * Return random number in range [min; max]
 *
 * @param net Network Context
 * @param min Lower random number range bound
 * @param max Higher random number range bound
 */
uint32_t net_rand(net_t * net, uint32_t min, uint32_t max);

/**
 * Calculate CRC of a packet
 *
 * @param data Data to calculate CRC on
 * @param size Size of data
 */
uint16_t net_crc(uint8_t * data, uint8_t size);

/**
 * Encrypt frame
 *
 * @param net   Network Context
 * @param frame Frame to encrypt
 */
error_t net_frame_encrypt(net_t * net, net_frame_t * frame);

/**
 * Decrypt frame
 *
 * @param net   Network Context
 * @param frame Frame to decrypt
 */
error_t net_frame_decrypt(net_t * net, net_frame_t * frame);

#ifdef __cplusplus
}
#endif
