from station.utils import validate_enum, assert_raise, bytes_to_str
from station.radio.types import (
    KEY_SIZE,
    Command,
    TransportType,
    ResetReason,
    AlertTrigger,
    StatusFlags,
)

from dataclasses import dataclass
from abc import ABC, abstractmethod
from enum import Enum
from typing import Type
import struct


class Payload(ABC):
    __subclasses = {}

    @classmethod
    def register_handler(cls, command: Command, handler: Type['Payload']):
        if command not in cls.__subclasses:
            cls.__subclasses[command] = handler

    @classmethod
    def get_handler_for(cls, command: Command) -> Type['Payload']:
        return cls.__subclasses[command]

    @abstractmethod
    def get_size(self) -> int: ...

    @abstractmethod
    def to_bytes(self) -> bytes: ...

    @classmethod
    @abstractmethod
    def from_bytes(cls, data: bytes): ...


class EmptyPayload(Payload):
    def __init__(self):
        ...

    def __str__(self):
        return ''

    def get_size(self) -> int:
        return 0

    def to_bytes(self) -> bytes:
        return b''

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls()


class ConfirmPayload(Payload):
    FORMAT = '>I'

    def __init__(self, station_mac: int, key: bytes):
        self.station_mac = station_mac
        self.key = key

    def __str__(self):
        return f'station_mac=0x{self.station_mac:X} key={self.key}'

    def get_size(self) -> int:
        return struct.calcsize(self.FORMAT) + KEY_SIZE

    def to_bytes(self) -> bytes:
        return struct.pack(self.FORMAT, self.station_mac) + self.key

    @classmethod
    def from_bytes(cls, data: bytes):
        sz = struct.calcsize(cls.FORMAT)
        return cls(*struct.unpack(cls.FORMAT, data[:sz]), data[sz:])


class RejectPayload(Payload):
    FORMAT = '>B'

    def __init__(self, reason: int):
        self.reason = reason

    def __str__(self):
        return f'reason={self.reason}'

    def get_size(self) -> int:
        return struct.calcsize(self.FORMAT)

    def to_bytes(self) -> bytes:
        return struct.pack(self.FORMAT, self.reason)

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(*struct.unpack(cls.FORMAT, data))


class RegisterPayload(Payload):
    FORMAT = '>BBBB'

    def __init__(self, hw_ver: int, sw_ver_major: int, sw_ver_minor: int, sw_ver_patch: int):
        self.hw_ver = hw_ver
        self.sw_ver_major = sw_ver_major
        self.sw_ver_minor = sw_ver_minor
        self.sw_ver_patch = sw_ver_patch

    def __str__(self):
        return f'ver={self.hw_ver}.{self.sw_ver_major}.{self.sw_ver_minor}.{self.sw_ver_patch}'

    def get_size(self) -> int:
        return struct.calcsize(self.FORMAT)

    def to_bytes(self) -> bytes:
        return struct.pack(self.FORMAT, self.hw_ver, self.sw_ver_major, self.sw_ver_minor, self.sw_ver_patch)

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(*struct.unpack(cls.FORMAT, data))


class StatusPayload(Payload):
    FORMAT = '>BBBbBB'

    def __init__(self, flags: int, reset_reason: ResetReason | int, reset_count: int, cpu_temp: int, bpm: int, avg_bpm: int):
        assert_raise(validate_enum(ResetReason, reset_reason), ValueError(f'Invalid reset reason {reset_reason}'))

        self.flags = flags
        self.reset_reason = reset_reason if type(reset_reason) is ResetReason else ResetReason(reset_reason)
        self.reset_count = reset_count
        self.cpu_temp = cpu_temp
        self.bpm = bpm
        self.avg_bpm = avg_bpm

    def __str__(self):
        return f'flags={self.flags} reset=({self.reset_reason.name} {self.reset_count}) cpu={self.cpu_temp} bpm=({self.bpm} {self.avg_bpm})'

    def get_size(self) -> int:
        return struct.calcsize(self.FORMAT)

    def to_bytes(self) -> bytes:
        return struct.pack(self.FORMAT, self.flags, self.reset_reason.value, self.reset_count, self.cpu_temp, self.bpm, self.avg_bpm)

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(*struct.unpack(cls.FORMAT, data))


class LocationPayload(Payload):
    DIR_SIZE = 1
    VAL_SIZE = 14

    def __init__(self, lat_dir: str, lat: str, long_dir: str, long: str):
        self.lat_dir = lat_dir
        self.lat = lat
        self.long_dir = long_dir
        self.long = long

    def __str__(self):
        return f'{self.lat_dir} {self.lat} {self.long_dir} {self.long}'

    def get_size(self) -> int:
        return (self.DIR_SIZE + self.VAL_SIZE) * 2

    # Latitude:  N 4943.97313
    # Longitude: E 02340.25276
    # N 49.4397313 E 23.4025276
    def to_bytes(self) -> bytes:
        return (
                self.lat_dir[:self.DIR_SIZE].encode('utf-8') +
                self.lat.encode('utf-8').ljust(self.VAL_SIZE, b'\x00') +
                self.long_dir[:self.DIR_SIZE].encode('utf-8') +
                self.long.encode('utf-8').ljust(self.VAL_SIZE, b'\x00')
        )

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(
            data[:cls.DIR_SIZE].decode('utf-8'),
            data[cls.DIR_SIZE:cls.DIR_SIZE+cls.VAL_SIZE].decode('utf-8').rstrip('\x00'),
            data[cls.DIR_SIZE+cls.VAL_SIZE:cls.DIR_SIZE+cls.VAL_SIZE+cls.DIR_SIZE].decode('utf-8'),
            data[cls.DIR_SIZE+cls.VAL_SIZE+cls.DIR_SIZE:].decode('utf-8').rstrip('\x00')
        )


class AlertPayload(Payload):
    FORMAT = '>B'

    def __init__(self, trigger: AlertTrigger | int):
        assert_raise(validate_enum(AlertTrigger, trigger), ValueError(f'Invalid alert trigger {trigger}'))

        self.trigger = trigger if type(trigger) is AlertTrigger else AlertTrigger(trigger)

    def __str__(self):
        return f'trigger={self.trigger.name}'

    def get_size(self) -> int:
        return struct.calcsize(self.FORMAT)

    def to_bytes(self) -> bytes:
        return struct.pack(self.FORMAT, self.trigger)

    @classmethod
    def from_bytes(cls, data: bytes):
        return cls(*struct.unpack(cls.FORMAT, data))


Payload.register_handler(Command.PING,      EmptyPayload)
Payload.register_handler(Command.CONFIRM,   ConfirmPayload)
Payload.register_handler(Command.REJECT,    RejectPayload)
Payload.register_handler(Command.REGISTER,  RegisterPayload)
Payload.register_handler(Command.STATUS,    StatusPayload)
Payload.register_handler(Command.LOCATION,  LocationPayload)
Payload.register_handler(Command.ALERT,     AlertPayload)
