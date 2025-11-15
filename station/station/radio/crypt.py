from station.radio.types import KEY_SIZE
import random


def encrypt(data: bytes | list, key: bytes | list) -> bytes:
    data = list(data)

    # Create 'salt' - 2 random bytes that are used later on, to make
    # same data be different when encrypted
    salt = [
        random.randint(1, 255),
        random.randint(1, 255),
    ]

    for i in range(len(data)):
        # Apply encryption/decryption function to each byte
        data[i] ^= key[(salt[0] + i) % KEY_SIZE] ^ salt[1]

    # Encrypting increases size of packet by 2 bytes - 'salt'
    return bytes(salt) + bytes(data)


def decrypt(data: bytes | list, key: bytes | list) -> bytes:
    # Extract 'salt' from packet
    salt = [
        int(data[0]),
        int(data[1])
    ]

    # Decrypting decreases size of packet by 2 bytes - 'salt'
    data = list(data[2:])

    for i in range(len(data)):
        # Apply encryption/decryption function to each byte
        data[i] ^= key[(salt[0] + i) % KEY_SIZE] ^ salt[1]

    return bytes(data)
