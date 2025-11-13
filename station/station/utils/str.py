
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
