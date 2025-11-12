/** ========================================================================= *
 *
 * @file packet.h
 * @date 11-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network Packet Definitions
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "error/error.h"
#include "net/types.h"
#include <stdbool.h>

/* Defines ================================================================== */
/** Will send data from sensors alongside default payload in some packets */
#ifndef NET_SEND_EXT_DATA
#define NET_SEND_EXT_DATA 1
#endif

/** Size of packet header (which is a mandatory part of a packet) */
#define NET_HEADER_SIZE 14

/* Macros =================================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/** NET_CMD_CONFIRM Payload */
typedef __PACKED_STRUCT {} net_confirm_payload_t;

/** NET_CMD_REJECT Payload */
typedef __PACKED_STRUCT {} net_reject_payload_t;

/** NET_CMD_REGISTER Payload */
typedef __PACKED_STRUCT {
  net_mac_t station_mac;
  net_key_t key;
} net_register_payload_t;

/** NET_CMD_SYSTEM_DATA Payload */
typedef __PACKED_STRUCT {
  uint8_t cpu_temp;

  __PACKED_STRUCT {
    uint8_t  percent;
    uint16_t mv;
  } bat;
} net_system_data_payload_t;

/** NET_CMD_LOCATION_DATA Payload */
typedef __PACKED_STRUCT {
  __PACKED_STRUCT {
    char direction;
    char value[10];
  } latitude;

  __PACKED_STRUCT {
    char direction;
    char value[10];
  } longitude;
} net_location_data_payload_t;

/** NET_CMD_PULSE_DATA Payload */
typedef __PACKED_STRUCT {
  uint8_t bpm;

#if NET_SEND_EXT_DATA
  uint8_t avg_bpm;
//  uint16_t sec;

  uint32_t raw;
  uint32_t lpf;
  uint32_t gauss;
  uint32_t filtered;
#endif
} net_pulse_data_payload_t;

/** NET_CMD_ACCEL_DATA Payload */
typedef __PACKED_STRUCT {
  bool sudden_movement;

#if NET_SEND_EXT_DATA
  int16_t x;
  int16_t y;
  int16_t z;
#endif
} net_accel_data_payload_t;

/** NET_CMD_ALARM Payload */
typedef __PACKED_STRUCT {
  net_alarm_type_t    type;
  net_alarm_trigger_t trigger;
} net_alarm_payload_t;

/**
 * Network packet
 */
typedef __PACKED_STRUCT {
  net_cmd_t            cmd;       /** Network Command */
  uint8_t              size;      /** Packet payload size */
  uint16_t             packet_id; /** Packet ID/Number */
  uint8_t              repeat;    /** Packet Repeat number */
  net_transport_type_t transport; /** Packet Transport type */
  net_mac_t            origin;    /** Packet Origin MAC (sender) */
  net_mac_t            target;    /** Packet Target MAC (receiver) */

  /** Packet payload union */
  __PACKED_UNION {
    net_confirm_payload_t       confirm;
    net_reject_payload_t        reject;
    net_register_payload_t      reg;
    net_system_data_payload_t   system;
    net_location_data_payload_t location;
    net_pulse_data_payload_t    pulse;
    net_accel_data_payload_t    accel;
    net_alarm_payload_t         alarm;
    uint8_t                     raw[0];
  } payload;
} net_packet_t;

/**
 * Packet Config
 */
typedef struct {
  net_cmd_t            cmd;       /** Network Command */
  net_transport_type_t transport; /** Packet Transport type */
  net_mac_t            target;    /** Packet Target MAC. Pass 0 to set station_mac */
} net_packet_cfg_t;

/** Forward declaration of net_t */
typedef struct net_t net_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */
/**
 * Inititalize packet
 *
 * Will set values from 'cfg', and automatically fill 'origin', 'target',
 * 'packet_id', 'repeat' & 'size'.
 *
 * @param net    Network context
 * @param packet Packet
 * @param cfg    Packet Config
 */
error_t net_packet_init(net_t * net, net_packet_t * packet, net_packet_cfg_t * cfg);

/**
 * Serializes packet
 *
 * Will convert 16 & 32 bit values to big endian, calculate CRC and encrypt the packet.
 * Doesn't modify `packet`.
 *
 * @param net    Network context
 * @param frame  Network frame. Encrypted packet will be put here
 * @param packet Packet to serialize
 */
error_t net_packet_serialize(net_t * net, net_frame_t * frame, net_packet_t * packet);

/**
 * Deserializes packet
 *
 * @param net    Network context
 * @param frame  Network frame. Encrypted packet must be here
 * @param packet Decrypted packet will be put here
 */
error_t net_packet_deserialize(net_t * net, net_frame_t * frame, net_packet_t * packet);

#ifdef __cplusplus
}
#endif
