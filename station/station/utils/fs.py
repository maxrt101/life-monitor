from pathlib import Path
from typing import Any

# Create path from list of strings and/or other paths
def form_path(*args) -> Path:
    def normalize(p: Any) -> Path | str:
        match p:
            case str():
                return p
            case Path():
                return p
            case _:
                return str(p)

    def path(p: Any) -> Path:
        return p if type(p) is Path else Path(str(p))

    result = None
    for arg in args:
        result = result / normalize(arg) if result else path(arg)
    return result
