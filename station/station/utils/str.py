

# Create formatted string from bytes object
def bytes_to_str(data: bytes) -> str:
    return ' '.join([f'{b:02X}' for b in data])


# Parse integer. First try in decimal, if failed try again in hex
def parse_int(s: str) -> int:
    try:
        return int(s)
    except ValueError:
        return int(s, 16)


# Generic hexdump for bytes objects
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
