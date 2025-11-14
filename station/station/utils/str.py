
ESCAPE_SEQUENCES = {
    '\n': '\\n',
    '\r': '\\r',
    '\t': '\\t',
    '\b': '\\b',
    '\v': '\\v',
}


def escape(s: str) -> str:
    for k, v in ESCAPE_SEQUENCES.items():
        s = s.replace(k, v)
    return s


def unescape(s: str) -> str:
    for k, v in ESCAPE_SEQUENCES.items():
        s = s.replace(v, k)
    return s


def bytes_to_str(data: bytes) -> str:
    return ' '.join([f'{b:02X}' for b in data])


def parse_int(s: str) -> int:
    try:
        return int(s)
    except ValueError:
        return int(s, 16)


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
