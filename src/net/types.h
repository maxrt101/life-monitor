/** ========================================================================= *
*
 * @file types.h
 * @date 11-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network types
 *
 *  ========================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ================================================================= */
#include "util/util.h"
#include <stdbool.h>

/* Defines ================================================================== */
/** Size of MAC in bytes */
#define NET_MAC_SIZE 4

/** Size of KEY in bytes */
#define NET_KEY_SIZE 16

/** Max size of packet payload */
#define NET_PACKET_MAX_PAYLOAD 46

/** Max size of packet (header + payload) */
#define NET_PACKET_MAX_SIZE    62

/* Macros =================================================================== */
/* Enums ==================================================================== */
/**
 * Network Command
 */
typedef __PACKED_ENUM {
  NET_CMD_CONFIRM        = 1,
  NET_CMD_REJECT         = 2,

  NET_CMD_REGISTER       = 16,

  NET_CMD_SYSTEM_DATA    = 32,
  NET_CMD_LOCATION_DATA  = 33,
  NET_CMD_PULSE_DATA     = 34,
  NET_CMD_ACCEL_DATA     = 35,

  NET_CMD_ALARM          = 64,
} net_cmd_t;

/**
 * Network Transport type
 */
typedef enum {
  NET_TRANSPORT_TYPE_UNICAST   = 0,
  NET_TRANSPORT_TYPE_MULTICAST = 1,
  NET_TRANSPORT_TYPE_BROADCAST = 2,
} net_transport_type_t;

/**
 * Network Alarm Type
 */
typedef __PACKED_ENUM {
  NET_ALARM_WARNING  = 1,
  NET_ALARM_CRITICAL = 2,
} net_alarm_type_t;

/**
 * Network Alarm Trigger
 */
typedef __PACKED_ENUM {
  NET_ALARM_TRIGGER_PULSE                 = 1,
  NET_ALARM_TRIGGER_MOVEMENT              = 2,
  NET_ALARM_TRIGGER_MOVEMENT_AND_MOVEMENT = 3,
  NET_ALARM_TRIGGER_CONNECTION_LOST       = 4,
} net_alarm_trigger_t;

/* Types ==================================================================== */
/**
 * Network MAC Address of a node
 */
typedef union {
  uint32_t value;
  uint8_t  raw[NET_MAC_SIZE];
} net_mac_t;

/**
 * Network encryption key
 */
typedef uint8_t net_key_t[NET_KEY_SIZE];

/**
 * Network Frame (used as intermediate buffer for packet data, so to not modify
 *                original packet)
 */
typedef __PACKED_STRUCT {
  uint8_t data[NET_PACKET_MAX_SIZE];
  uint8_t size;
} net_frame_t;

/* Variables ================================================================ */
/* Shared functions ========================================================= */

#ifdef __cplusplus
}
#endif

