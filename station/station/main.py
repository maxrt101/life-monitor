from station.radio.packet import Packet
from station.radio.types import Command, TransportType, ResetReason

def hexdump(data: bytes):
    offset = 0

    print(f'0x{offset:08X}: ', end='')
    while offset < len(data):
        print(f'{data[offset]:02x} ', end='')

        if (offset + 1) % 16 == 0:
            print()
            if offset + 1 != len(data):
                print(f'0x{offset:08X}: ', end='')

        offset += 1

    print()


def main():
    key = bytes([0 for _ in range(16)])

    encrypted_packet = bytes([int(f'0x{x}', 16) for x in '34 6a 6e 6c 6a 6a 6a 6a b4 c7 d4 85 6a 6a 6a 6a 6a 68 6e 95 03 28 2b 7f'.split(' ')])

    hexdump(encrypted_packet)

    packet = Packet.from_bytes(encrypted_packet, key)

    print(packet)

    status = Packet.create(
        command=Command.STATUS,
        transport=TransportType.UNICAST,
        origin=0xEBAC0C42,
        target=0xDA1BA10B,
        key=key,
        flags=0,
        reset_reason=ResetReason.WDG,
        reset_count=8,
        cpu_temp=5,
        bpm=0x42,
        avg_bpm=0x69
    )

    status_encrypted = status.to_bytes()

    hexdump(status_encrypted)

    status_decrypted = Packet.from_bytes(status_encrypted, key)

    print(status_decrypted)

if __name__ == '__main__':
    main()
