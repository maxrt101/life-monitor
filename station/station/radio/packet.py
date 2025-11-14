from station.radio.header import Header
from station.radio.payload import Payload, EmptyPayload
from station.radio.types import Command, TransportType
from station.radio import crc, crypt

import struct

class Packet:
    __packet_id = 0

    def __init__(self, header: Header, payload: Payload = EmptyPayload(), key: bytes = None):
        self.header = header
        self.payload = payload
        self.key = key

    def __str__(self):
        return '{}: {}'.format(self.header, self.payload)

    def __eq__(self, other):
        # print(f'header={self.header == other.header} payload={self.payload == other.payload}')
        return self.header == other.header and self.payload == other.payload

    @classmethod
    def reset_packet_id(cls):
        cls.__packet_id = 0

    @classmethod
    def get_increment_packet_id(cls):
        result = cls.__packet_id
        cls.__packet_id += 1
        return result

    @classmethod
    def create(cls, command: Command, transport: TransportType, origin: int, target: int, key: bytes = None, **kwargs):
        return cls(header=Header(
            command, 0, cls.get_increment_packet_id(), 0, transport, origin, target),
            payload=Payload.get_handler_for(command)(**kwargs),
            key=key
        )

    # @classmethod
    # def create_ack(cls, command: Command, device_id: int, packet_id: int, key: bytes = None, **kwargs):
    #     return cls(header=Header(
    #         command, 0, packet_id, device_id),
    #         payload=Payload.get_handler_for(command)(**kwargs),
    #         key=key
    #     )
    #
    # def ack(self, command: Command, **kwargs):
    #     return Packet(header=Header(
    #         command, 0, self.header.packet_id, self.header.device_id),
    #         payload=Payload.get_handler_for(command)(**kwargs),
    #         key=self.key
    #     )
    #
    # def send(self, trx: Driver):
    #     trx.send(self.to_bytes())

    @staticmethod
    def get_header_size():
        return Header.get_size()

    @classmethod
    def get_min_size(cls):
        return cls.get_header_size() + crc.SIZE

    @classmethod
    def get_max_size(cls):
        return 64

    @classmethod
    def from_bytes(cls, data: bytes, key: bytes = None) -> 'Packet':
        if len(data) < cls.get_min_size():
            raise ValueError(f'Packet too small (min={cls.get_min_size()} size={len(data)})')

        if len(data) > cls.get_max_size():
            raise ValueError(f'Packet too big (max={cls.get_max_size()} size={len(data)})')

        data = crypt.decrypt(data, key)

        if not crc.check(data):
            raise ValueError(f'CRC not matching (expected={crc.extract(data)} actual={crc.raw_crc(data[:-2])})')

        header = Header.from_bytes(data)

        payload = data[cls.get_header_size():-crc.SIZE]

        if len(payload) != header.size:
            raise ValueError(f'Mismatching payload size (expected={header.size} actual={len(payload)})')

        return cls(header, Payload.get_handler_for(header.command).from_bytes(payload), key)

    def to_bytes(self, key: bytes = None) -> bytes:
        self.header.size = self.payload.get_size()
        packet = self.header.to_bytes() + self.payload.to_bytes()
        packet += struct.pack(crc.FORMAT, crc.raw_crc(packet))

        return crypt.encrypt(packet, key if key else self.key)
