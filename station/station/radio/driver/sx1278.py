from station.config import CONFIG_RADIO_LINUX_SX1278_LIB_PATH, CONFIG_RADIO_LINUX_SX1278_BINDING_PATH
from station.utils import import_from_path
from . import Driver
from typing import Any


class SX1278Driver(Driver):
    # Max output power amplifier value in dBm
    MAX_PA = 20

    # linux-sx1278 binding module handle
    sx1278 = None

    @classmethod
    def __init_sx1278(cls):
        # Load linux-sx1278 binding (and shared lib), if not loaded
        if not cls.sx1278:
            cls.sx1278 = import_from_path('sx1278', CONFIG_RADIO_LINUX_SX1278_BINDING_PATH)
            cls.sx1278.__init__(CONFIG_RADIO_LINUX_SX1278_LIB_PATH)

    def __init__(self, spidev: str):
        # Initialize linux-sx1278
        self.__init_sx1278()

        # Connect to SPI device
        self.spi = self.sx1278.Spi(spidev)

        # Create transceiver driver instance
        self.trx = self.sx1278.Sx1278(self.spi)

        # Create last error field
        self.last_error = ''

        # Set max output power for highest reliability
        self.trx.set_power(self.MAX_PA)

    def __del__(self):
        # If 'spi' is present in 'self'
        # This check needs to be here, as __init__ may
        # raise an exception before spi is created
        if 'spi' in self.__dict__:
            # Deinitialize SPI
            self.spi.deinit()

    def send(self, data: bytes):
        try:
            self.trx.send(data)
        except Exception as ex:
            # Save exception name & message into 'last_error'
            self.last_error = str(f'{ex.__class__.__name__}: {ex}')

    def recv(self, timeout_ms: int) -> bytes | None:
        try:
            return self.trx.recv(self.sx1278.Timeout(timeout_ms))
        except Exception as ex:
            # Save exception name & message into 'last_error'
            self.last_error = str(f'{ex.__class__.__name__}: {ex}')
            return None

    def get_last_error(self) -> Any:
        return self.last_error
