from abc import ABC, abstractmethod
from typing import Any


# Radio Driver API
class Driver(ABC):
    # Send bytes object (data/payload) via radio channel
    @abstractmethod
    def send(self, data: bytes): ...

    # Listen radio for 'timeout_ms', if packet found - return it as raw
    # bytes object, if not - return None
    @abstractmethod
    def recv(self, timeout_ms: int) -> bytes | None: ...

    # Radio driver doesn't raise exceptions, instead it stores last error
    # in driver context, for user to decide if error needs handling
    @abstractmethod
    def get_last_error(self) -> Any: ...
