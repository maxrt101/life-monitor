import struct

CRC_INIT = 0x42
SIZE = 2
FORMAT = '>H'

def raw_crc(data: bytes) -> int:
    import binascii

    return binascii.crc_hqx(data, CRC_INIT)

def extract(packet: bytes) -> int:
    return struct.unpack(FORMAT, packet[-2:])[0]

def check(packet: bytes) -> bool:
    expected = extract(packet)
    return raw_crc(packet[:-2]) == expected
