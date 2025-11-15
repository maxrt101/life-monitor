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

from station.utils import logger

def create_driver(spidev: str) -> Driver:
    from station.config import CONFIG_RADIO_DRIVER

    if CONFIG_RADIO_DRIVER == 'mock':
        from .driver.mock import MockDriver
        logger.info('Initializing MOCK radio driver')
        return MockDriver()
    elif CONFIG_RADIO_DRIVER == 'sx1278':
        from .driver.sx1278 import SX1278Driver
        logger.info(f'Initializing SX1278 radio driver on {spidev}')
        return SX1278Driver(spidev)
    else:
        raise ValueError(f'Invalid rf driver requested: {CONFIG_RADIO_DRIVER}')
