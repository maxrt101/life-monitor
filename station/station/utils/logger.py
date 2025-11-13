from .ansi import ERASE_LINE, COLOR_GREY, COLOR_BLUE, COLOR_YELLOW, COLOR_RED, COLOR_BOLD_RED, COLOR_CYAN, COLOR_RESET
import logging


class CustomFormatter(logging.Formatter):
    FORMAT = ERASE_LINE + '\r[{}%(levelname)s{}]: %(message)s [{}%(filename)s:%(lineno)d{}]'

    FORMATS = {
        logging.DEBUG:    FORMAT.format(COLOR_GREY, COLOR_RESET, COLOR_CYAN, COLOR_RESET),
        logging.INFO:     FORMAT.format(COLOR_BLUE, COLOR_RESET, COLOR_CYAN, COLOR_RESET),
        logging.WARNING:  FORMAT.format(COLOR_YELLOW, COLOR_RESET, COLOR_CYAN, COLOR_RESET),
        logging.ERROR:    FORMAT.format(COLOR_RED, COLOR_RESET, COLOR_CYAN, COLOR_RESET),
        logging.CRITICAL: FORMAT.format(COLOR_BOLD_RED, COLOR_RESET, COLOR_CYAN, COLOR_RESET)
    }

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)

logger = logging.getLogger('app')
logger.setLevel(logging.DEBUG)

# Console handler
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.DEBUG)
console_handler.setFormatter(CustomFormatter())
logger.addHandler(console_handler)
