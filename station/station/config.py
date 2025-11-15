# Path (fie name) of sqlite database file
CONFIG_DB_FILE_PATH: str = 'lifemonitor.db'

# Session key for flask authentication checks
# Code to generate:
#   s16r = lambda: ''.join(random.sample(string.hexdigits, 16));
#   f'{s16r()}-{s16r()}-{s16r()}-{s16r()}'
CONFIG_APP_SESSION_KEY: str = '8d0F9A5a416DBceb-bEe8CDa0d5B3f971-750c16Eb4B9fadF3-06D9b7C2EBecF83f'

# Artificial timeout between radio.Network.cycle calls
CONFIG_RADIO_THREAD_CYCLE_PERIOD: float = 0.5

# Timeframe in which station will wait for device to send NET_CMD_REGISTER packet (in seconds)
CONFIG_REGISTRATION_DURATION: int = 10

# Duration of single packet listen
CONFIG_RADIO_PACKET_LISTEN_DURATION: int = 200

# MAC address of this station (unique identifier of a node in radio network)
CONFIG_STATION_MAC: int = 0xDEADBEEF

# Key for this station's radio network
CONFIG_RADIO_KEY: bytes = bytes([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16])

# Default key for devices (used on registration, before device knows network key)
CONFIG_RADIO_DEFAULT_KEY: bytes = bytes([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

# Radio driver backend. Possible values: 'mock', 'sx1278'
CONFIG_RADIO_DRIVER: str = 'mock'

# Linux SPI Device to use in linux-sx1278
CONFIG_RADIO_SX1278_SPIDEV: str = '/dev/spidev0.0'

# Path to compiled Shared Object of linux-sx1278
CONFIG_RADIO_LINUX_SX1278_LIB_PATH: str = 'linux-sx1278/cmake-build-directory/liblinux-sx1278.so'

# Path to linux-sx1278 python binding
CONFIG_RADIO_LINUX_SX1278_BINDING_PATH: str = 'linux-sx1278/bindings/sx1278.py'
