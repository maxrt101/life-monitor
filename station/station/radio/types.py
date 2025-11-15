from enum import Enum


# Encryption key size in bytes
KEY_SIZE = 16


class Command(Enum):
    PING              = 0
    CONFIRM           = 1
    REJECT            = 2
    REGISTER          = 3
    REGISTRATION_DATA = 4
    STATUS            = 5
    LOCATION          = 6
    ALERT             = 7


class TransportType(Enum):
    UNICAST     = 0
    MULTICAST   = 1
    BROADCAST   = 2


class AlertTrigger(Enum):
    PULSE_THRESHOLD = 1
    SUDDEN_MOVEMENT = 2


class ResetReason(Enum):
    UNK     = 0
    HW_RST  = 1
    SW_RST  = 2
    WDG     = 3
    WWDG    = 4
    POR     = 5
    BOR     = 6


class StatusFlags:
    PULSE_SENSOR_FAILURE = (1 << 0)
    ACCEL_SENSOR_FAILURE = (1 << 1)
    GPS_FAILURE          = (1 << 2)

