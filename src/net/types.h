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
  NET_CMD_PING      = 0,
  NET_CMD_CONFIRM   = 1,
  NET_CMD_REJECT    = 2,
  NET_CMD_REGISTER  = 3,
  NET_CMD_STATUS    = 4,
  NET_CMD_LOCATION  = 5,
  NET_CMD_ALERT     = 6,
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
 * Network Alarm Trigger
 */
typedef __PACKED_ENUM {
  NET_ALERT_TRIGGER_PULSE_THRESHOLD = 1,
  NET_ALERT_TRIGGER_SUDDEN_MOVEMENT = 2,
} net_alert_trigger_t;

/**
 * Device Reset Reason
 */
typedef __PACKED_ENUM {
  NET_RESET_REASON_UNK    = 0,
  NET_RESET_REASON_HW_RST = 1,
  NET_RESET_REASON_SW_RST = 2,
  NET_RESET_REASON_WDG    = 3,
  NET_RESET_REASON_WWDG   = 4,
  NET_RESET_REASON_POR    = 5,
  NET_RESET_REASON_BOR    = 6,
} net_reset_reason_t;

/**
 * Network status flags
 */
typedef __PACKED_ENUM {
  NET_STATUS_FLAG_PULSE_SENSOR_FAILURE = (1 << 0),
  NET_STATUS_FLAG_ACCEL_SENSOR_FAILURE = (1 << 1),
  NET_STATUS_FLAG_GPS_FAILURE          = (1 << 2),
} net_status_flags_t;

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

