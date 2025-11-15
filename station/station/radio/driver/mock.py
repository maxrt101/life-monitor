# from station.utils import logger, bytes_to_str
from . import Driver
from typing import Any


class MockDriver(Driver):
    def __init__(self):
        self.packets         = []  # Queue of packets, to be returned by 'recv()'
        self.errors          = []  # Queue of errors, to be returned by 'get_last_error()'
        self.last_in_packet  = b'' # Raw data of last 'received' packet
        self.last_out_packet = b'' # Raw data of last 'send' packet

    def next_packet(self, data: bytes):
        # Push packet data into queue
        self.packets.append(data)

    def next_error(self, err: str):
        # Push error into queue
        self.errors.append(err)

    def send(self, data: bytes):
        # Save packet data
        self.last_out_packet = data
        # Logs are turned off for now
        # logger.debug(f'RF Send Packet: {bytes_to_str(data)}')

    def recv(self, timeout_ms: int) -> bytes | None:
        # Pop from queue, of return None, if no packets are preset
        data = self.packets.pop(0) if len(self.packets) else None

        self.last_in_packet = data

        # Logs are turned off for now
        # if data:
        #     logger.debug(f'RF Recv Packet: {bytes_to_str(data)}')

        return data

    def get_last_error(self) -> Any:
        # Pop from queue, or return 'OK', if no errors are present
        return self.errors.pop(0) if len(self.errors) else 'OK'
