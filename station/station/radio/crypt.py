import random
from station.radio.types import KEY_SIZE

def encrypt(data: bytes | list, key: bytes | list) -> bytes:
    data = list(data)

    salt = [
        random.randint(1, 255),
        random.randint(1, 255),
    ]

    for i in range(len(data)):
        data[i] ^= key[(salt[0] + i) % KEY_SIZE] ^ salt[1]

    return bytes(salt) + bytes(data)

def decrypt(data: bytes | list, key: bytes | list) -> bytes:
    salt = [
        int(data[0]),
        int(data[1])
    ]

    data = list(data[2:])

    for i in range(len(data)):
        data[i] ^= key[(salt[0] + i) % KEY_SIZE] ^ salt[1]

    return bytes(data)
