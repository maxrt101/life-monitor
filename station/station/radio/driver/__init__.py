from abc import ABC, abstractmethod
from typing import Any


class Driver(ABC):
    @abstractmethod
    def send(self, data: bytes): ...

    @abstractmethod
    def recv(self, timeout_ms: int) -> bytes | None: ...

    @abstractmethod
    def get_last_error(self) -> Any: ...
