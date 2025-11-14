from station.radio.types import Command, TransportType
from station.utils import validate_enum, assert_raise
import struct


class Header:
    FORMAT = '>BBHBBII'

    def __init__(self, command: Command | int, size: int, packet_id: int, repeat: int, transport: TransportType | int, origin: int, target: int):
        assert_raise(validate_enum(Command, command), ValueError(f'Invalid command {command}'))
        assert_raise(validate_enum(TransportType, transport), ValueError(f'Invalid transport type {transport}'))

        self.command   = command if type(command) is Command else Command(command)
        self.size      = size
        self.packet_id = packet_id
        self.repeat    = repeat
        self.transport = transport if type(transport) is Command else TransportType(transport)
        self.origin    = origin
        self.target    = target

    def __str__(self):
        return f'{self.command.name} #{self.packet_id} r{self.repeat} {self.transport.name} 0x{self.origin:X} -> 0x{self.target:X}'

    def __eq__(self, other):
        return (
            self.command.value   == other.command.value   and
            self.packet_id       == other.packet_id       and
            self.repeat          == other.repeat          and
            self.transport.value == other.transport.value and
            self.origin          == other.origin          and
            self.target          == other.target
        )

    @classmethod
    def get_size(cls):
        return struct.calcsize(cls.FORMAT)

    @classmethod
    def from_bytes(cls, data: bytes) -> 'Header':
        return cls(*struct.unpack(cls.FORMAT, data[:cls.get_size()]))

    def to_bytes(self):
        return struct.pack(
            self.FORMAT,
            self.command.value,
            self.size,
            self.packet_id,
            self.repeat,
            self.transport.value,
            self.origin,
            self.target
        )