/** ========================================================================= *
*
 * @file packet.c
 * @date 11-11-2025
 * @author Maksym Tkachuk <max.r.tkachuk@gmail.com>
 *
 * @brief LifeMonitor RF Network Packet Definitions
 *
 *  ========================================================================= */

/* Includes ================================================================= */
#include "net/packet.h"
#include "net/net.h"
#include "error/assertion.h"
#include "log/log.h"
#include "util/endianness.h"

/* Defines ================================================================== */
#define LOG_TAG net

/* Macros =================================================================== */
#define TO_BIG_ENDIAN16(value)   value = endian_to_big_u16(value)
#define TO_BIG_ENDIAN32(value)   value = endian_to_big_u32(value)
#define FROM_BIG_ENDIAN16(value) value = endian_from_big_u16(value)
#define FROM_BIG_ENDIAN32(value) value = endian_from_big_u32(value)

/* Exposed macros =========================================================== */
/* Enums ==================================================================== */
/* Types ==================================================================== */
/* Variables ================================================================ */
/* Private functions ======================================================== */
#if USE_NET_PACKET_DUMP
__STATIC_INLINE const char * net_cmd2str(net_cmd_t cmd) {
  switch (cmd) {
    case NET_CMD_PING:              return "PING";
    case NET_CMD_CONFIRM:           return "CONFIRM";
    case NET_CMD_REJECT:            return "REJECT";
    case NET_CMD_REGISTER:          return "REGISTER";
    case NET_CMD_REGISTRATION_DATA: return "REGISTRATION_DATA";
    case NET_CMD_STATUS:            return "STATUS";
    case NET_CMD_LOCATION:          return "LOCATION";
    case NET_CMD_ALERT:             return "ALERT";
    default:                        return "?";
  }
}

__STATIC_INLINE const char * net_transport2str(net_transport_type_t type) {
  switch (type) {
    case NET_TRANSPORT_TYPE_UNICAST:   return "UNICAST";
    case NET_TRANSPORT_TYPE_MULTICAST: return "MULTICAST";
    case NET_TRANSPORT_TYPE_BROADCAST: return "BROADCAST";
    default:                           return "?";
  }
}

__STATIC_INLINE const char * net_reset_reason2str(net_reset_reason_t reset_reason) {
  switch (reset_reason) {
    case NET_RESET_REASON_UNK:    return "UNK";
    case NET_RESET_REASON_HW_RST: return "HW_RST";
    case NET_RESET_REASON_SW_RST: return "SW_RST";
    case NET_RESET_REASON_WDG:    return "WDG";
    case NET_RESET_REASON_WWDG:   return "WWDG";
    case NET_RESET_REASON_POR:    return "POR";
    case NET_RESET_REASON_BOR:    return "BOR";
    default:                      return "?";
  }
}

__STATIC_INLINE const char * net_alert_trigger2str(net_alert_trigger_t trigger) {
  switch (trigger) {
    case NET_ALERT_TRIGGER_PULSE_THRESHOLD: return "PULSE_THRESHOLD";
    case NET_ALERT_TRIGGER_SUDDEN_MOVEMENT: return "SUDDEN_MOVEMENT";
    default:                                return "?";
  }
}
#endif

/* Shared functions ========================================================= */
error_t net_packet_init(net_t * net, net_packet_t * packet, net_packet_cfg_t * cfg) {
  ASSERT_RETURN(net && packet && cfg, E_NULL);

  packet->cmd       = cfg->cmd;
  packet->repeat    = 0;
  packet->packet_id = net->packet_id++;
  packet->transport = cfg->transport;

  packet->origin.value = net->dev_mac.value;

  if (cfg->transport == NET_TRANSPORT_TYPE_UNICAST && !cfg->target.value) {
    packet->target.value = net->station_mac.value;
  }

  switch (cfg->cmd) {
    case NET_CMD_PING:
      packet->size = 0;
      break;
    case NET_CMD_CONFIRM:
      packet->size = sizeof(net_confirm_payload_t);
      break;
    case NET_CMD_REJECT:
      packet->size = sizeof(net_reject_payload_t);
      break;
    case NET_CMD_REGISTER:
      packet->size = sizeof(net_register_payload_t);
      break;
    case NET_CMD_REGISTRATION_DATA:
      packet->size = sizeof(net_registration_data_t);
      break;
    case NET_CMD_STATUS:
      packet->size = sizeof(net_status_payload_t);
      break;
    case NET_CMD_LOCATION:
      packet->size = sizeof(net_location_payload_t);
      break;
    case NET_CMD_ALERT:
      packet->size = sizeof(net_alert_payload_t);
      break;
    default:
      return E_INVAL;
  }

  return E_OK;
}

error_t net_packet_serialize(net_t * net, net_frame_t * frame, net_packet_t * packet) {
  ASSERT_RETURN(net && frame && packet, E_NULL);

  net_packet_t pkt;
  memcpy(&pkt, packet, sizeof(net_packet_t));

  TO_BIG_ENDIAN16(pkt.packet_id);
  TO_BIG_ENDIAN32(pkt.target.value);
  TO_BIG_ENDIAN32(pkt.origin.value);

  // switch (pkt.cmd) {
  //   case NET_CMD_PING:
  //     break;
  //   case NET_CMD_CONFIRM:
  //     break;
  //   case NET_CMD_REJECT:
  //     break;
  //   case NET_CMD_REGISTER:
  //     break;
  //   case NET_CMD_STATUS:
  //     break;
  //   case NET_CMD_ALERT:
  //     break;
  //   default:
  //     return E_INVAL;
  // }

  frame->size = pkt.size + NET_HEADER_SIZE;
  memcpy(frame->data, &pkt, frame->size);

  u16_buffer_t crc = { ._u16 = net_crc(frame->data, frame->size) };

  TO_BIG_ENDIAN16(crc._u16);

  frame->data[frame->size++] = crc._u8[0];
  frame->data[frame->size++] = crc._u8[1];

  return net_frame_encrypt(net, frame);
}

error_t net_packet_deserialize(net_t * net, net_frame_t * frame, net_packet_t * packet) {
  ASSERT_RETURN(net && frame && packet, E_NULL);

  ERROR_CHECK_RETURN(net_frame_decrypt(net, frame));

  u16_buffer_t crc = { ._u8 = { frame->data[frame->size - 2], frame->data[frame->size - 1] } };

  FROM_BIG_ENDIAN16(crc._u16);

  ASSERT_RETURN(crc._u16, net_crc(frame->data, frame->size-2), E_CORRUPT);

  frame->size -= 2;

  memcpy(packet, frame->data, frame->size);

  FROM_BIG_ENDIAN16(packet->packet_id);
  FROM_BIG_ENDIAN32(packet->target.value);
  FROM_BIG_ENDIAN32(packet->origin.value);

  // switch (packet->cmd) {
  //   case NET_CMD_PING:
  //     break;
  //   case NET_CMD_CONFIRM:
  //     break;
  //   case NET_CMD_REJECT:
  //     break;
  //   case NET_CMD_REGISTER:
  //     break;
  //   case NET_CMD_STATUS:
  //     break;
  //   case NET_CMD_LOCATION:
  //     break;
  //   case NET_CMD_ALERT:
  //     break;
  //   default:
  //     return E_INVAL;
  // }

  return E_OK;
}

#if USE_NET_PACKET_DUMP
error_t net_packet_dump(net_packet_t * packet) {
  ASSERT_RETURN(packet, E_NULL);

  log_printf("%s #%d r%d %s 0x%X -> 0x%X: ",
    net_cmd2str(packet->cmd),
    packet->packet_id,
    packet->repeat,
    net_transport2str(packet->transport),
    packet->origin.value,
    packet->target.value
  );

  switch (packet->cmd) {
    case NET_CMD_PING:
      break;
    case NET_CMD_CONFIRM:
      break;
    case NET_CMD_REJECT:
      log_printf("reason=%d", packet->payload.reject.reason);
      break;
    case NET_CMD_REGISTER:
      log_printf("ver=%d.%d.%d.%d",
        packet->payload.reg.hw_version,
        packet->payload.reg.sw_version_major,
        packet->payload.reg.sw_version_minor,
        packet->payload.reg.sw_version_patch
      );
      break;
    case NET_CMD_REGISTRATION_DATA:
      log_printf("station_mac=0x%X key=", packet->payload.reg_data.station_mac);
      for (uint8_t i = 0; i < NET_KEY_SIZE; ++i) {
        log_printf("%02x ", packet->payload.reg_data.key[i]);
      }
      break;
    case NET_CMD_STATUS:
      log_printf("flags=%d reset=(%s %d) cpu=%d bpm=(%d %d)",
        packet->payload.status.flags,
        net_reset_reason2str(packet->payload.status.reset_reason),
        packet->payload.status.reset_count,
        packet->payload.status.cpu_temp,
        packet->payload.status.bpm,
        packet->payload.status.avg_bpm
      );
      break;
    case NET_CMD_LOCATION:
      log_printf("%c %s %c %s",
        packet->payload.location.latitude.direction,
        packet->payload.location.latitude.value,
        packet->payload.location.longitude.direction,
        packet->payload.location.longitude.value
      );
      break;
    case NET_CMD_ALERT:
      log_printf("trigger=%s", net_alert_trigger2str(packet->payload.alert.trigger));
      break;
    default:
      return E_INVAL;
  }

  log_printf("\r\n");

  return E_OK;
}
#endif
