CONFIG_DB_FILE_PATH: str = 'lifemonitor.db'

CONFIG_REGISTRATION_DURATION: int = 10

CONFIG_STATION_MAC: int = 0xBADCAFE0

CONFIG_RADIO_KEY: bytes = bytes([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16])
CONFIG_RADIO_DEFAULT_KEY: bytes = bytes([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

CONFIG_RADIO_DRIVER: str = 'mock'
CONFIG_RADIO_SX1278_SPIDEV: str = '/dev/spidev0.0'
CONFIG_RADIO_LINUX_SX1278_LIB_PATH: str = 'linux-sx1278/cmake-build-directory/liblinux-sx1278.so'
CONFIG_RADIO_LINUX_SX1278_BINDING_PATH: str = 'linux-sx1278/bindings/sx1278.py'
