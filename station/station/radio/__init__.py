from .types import (
    Command,
    TransportType,
    ResetReason,
    StatusFlags,
    AlertTrigger
)

from .header import Header
from .packet import Packet

from .payload import (
    Payload,
    EmptyPayload,
    RejectPayload,
    RegisterPayload,
    RegistrationDataPayload,
    StatusPayload,
    LocationPayload,
    AlertPayload
)

from .net import Network

from .driver import Driver

def create_driver() -> Driver:
    from station.config import CONFIG_RADIO_DRIVER, CONFIG_RADIO_SX1278_SPIDEV

    if CONFIG_RADIO_DRIVER == 'mock':
        from .driver.mock import MockDriver
        return MockDriver()
    elif CONFIG_RADIO_DRIVER == 'sx1278':
        from .driver.sx1278 import SX1278Driver
        return SX1278Driver(CONFIG_RADIO_SX1278_SPIDEV)
    else:
        raise ValueError(f'Invalid rf driver requested: {CONFIG_RADIO_DRIVER}')
