from .fs import form_path
from .str import escape, unescape, bytes_to_str, parse_int
from .ansi import *
from .logger import logger

from typing import Type, Any
from enum import Enum
import importlib.util
import sys

def assert_raise(value: bool, exc: Exception):
    if not value:
        raise exc

# Validates enum value using enum type
def validate_enum(cls: Type[Enum], value: Any):
    v = value

    if isinstance(value, Enum):
        v = value.value

    return v in set(item.value for item in cls)

def import_from_path(module_name: str, file_path: str):
    spec = importlib.util.spec_from_file_location(module_name, file_path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module