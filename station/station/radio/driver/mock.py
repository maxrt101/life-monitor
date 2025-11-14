from station.utils import logger, bytes_to_str
from . import Driver
from typing import Any


class MockDriver(Driver):
    def __init__(self):
        self.packets = []
        self.errors = []
        self.last_in_packet = b''
        self.last_out_packet = b''

    def next_packet(self, data: bytes):
        self.packets.append(data)

    def next_error(self, err: str):
        self.errors.append(err)

    def send(self, data: bytes):
        self.last_out_packet = data
        # logger.debug(f'RF Send Packet: {bytes_to_str(data)}')

    def recv(self, timeout_ms: int) -> bytes | None:
        data = self.packets.pop(0) if len(self.packets) else None

        self.last_in_packet = data

        # if data:
        #     logger.debug(f'RF Recv Packet: {bytes_to_str(data)}')

        return data

    def get_last_error(self) -> Any:
        return self.errors.pop(0) if len(self.errors) else 'OK'
