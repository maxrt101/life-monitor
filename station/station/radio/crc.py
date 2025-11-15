import binascii
import struct

# Initial CRC value
CRC_INIT = 0x42

# CRC size in bytes
SIZE = 2

# CRC format for 'struct'
FORMAT = '>H'

# Calculate CRC on bytes. CRC is CRC16-CCIT
def raw_crc(data: bytes) -> int:
    return binascii.crc_hqx(data, CRC_INIT)

# Extract CRC from raw (decrypted) packet data
def extract(packet: bytes) -> int:
    return struct.unpack(FORMAT, packet[-2:])[0]

# Check CRC of packet, by extracting embedded CRC value
# and comparing it to calculated CRC
def check(packet: bytes) -> bool:
    expected = extract(packet)
    return raw_crc(packet[:-2]) == expected
