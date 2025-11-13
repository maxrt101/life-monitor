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

  switch (pkt.cmd) {
    case NET_CMD_PING:
      break;
    case NET_CMD_CONFIRM:
      break;
    case NET_CMD_REJECT:
      break;
    case NET_CMD_REGISTER:
      break;
    case NET_CMD_STATUS:
      break;
    case NET_CMD_ALERT:
      break;
    default:
      return E_INVAL;
  }

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

  switch (packet->cmd) {
    case NET_CMD_PING:
      break;
    case NET_CMD_CONFIRM:
      break;
    case NET_CMD_REJECT:
      break;
    case NET_CMD_REGISTER:
      break;
    case NET_CMD_STATUS:
      break;
    case NET_CMD_LOCATION:
      break;
    case NET_CMD_ALERT:
      break;
    default:
      return E_INVAL;
  }

  return E_OK;
}
