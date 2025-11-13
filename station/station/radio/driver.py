from typing import Any

class RadioDriver:
    MAX_PA = 20

    sx1278 = None

    @classmethod
    def __init_sx1278(cls):
        if not cls.sx1278:
            from station.utils import import_from_path
            from station.config import CONFIG_RF_LINUX_SX1278, CONFIG_RF_LINUX_SX1278_BINDING
            cls.sx1278 = import_from_path('sx1278', CONFIG_RF_LINUX_SX1278_BINDING)
            cls.sx1278.__init__(CONFIG_RF_LINUX_SX1278)

    def __init__(self, spidev: str):
        self.__init_sx1278()
        self.spi = self.sx1278.Spi(spidev)
        self.trx = self.sx1278.Sx1278(self.spi)
        self.last_error = ''

        self.trx.set_power(self.MAX_PA)

    def __del__(self):
        if 'spi' in self.__dict__:
            self.spi.deinit()

    def send(self, data: bytes):
        try:
            self.trx.send(data)
        except Exception as ex:
            self.last_error = str(f'{ex.__class__.__name__}: {ex}')

    def recv(self, timeout_ms: int) -> bytes | None:
        try:
            return self.trx.recv(self.sx1278.Timeout(timeout_ms))
        except Exception as ex:
            self.last_error = str(f'{ex.__class__.__name__}: {ex}')
            return None

    def get_last_error(self) -> Any:
        return self.last_error
