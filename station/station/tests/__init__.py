from station.utils.ansi import (
    ERASE_LINE,
    COLOR_GREEN,
    COLOR_RED,
    COLOR_YELLOW,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_RESET
)

from dataclasses import dataclass
import traceback
import unittest
import time

from .net import RadioPacketTestCase, RadioNetworkTestCase


TEST_CASES = [
    RadioPacketTestCase,
    RadioNetworkTestCase
]


RUNNING_HEADER = f'{COLOR_CYAN}→ Running:{COLOR_RESET}'
PASSED_HEADER  = f'{COLOR_GREEN}✔ Passed:{COLOR_RESET}'
FAILED_HEADER  = f'{COLOR_RED}✖ Failed:{COLOR_RESET}'
ERROR_HEADER   = f'{COLOR_YELLOW}⚠ Error:{COLOR_RESET}'
SKIPPED_HEADER = f'{COLOR_CYAN}⏭ Skipped:{COLOR_RESET}'


class ColoredTestResult(unittest.TextTestResult):
    @dataclass
    class Summary:
        total:   int
        failed:  int
        errors:  int
        skipped: int
        passed:  int

    @staticmethod
    def print_error(err, *, full_stacktrace: bool = False):
        exc_type, exc_value, exc_tb = err

        tb_list = traceback.extract_tb(exc_tb)
        frames = tb_list if full_stacktrace else [tb_list[-1]]

        print(''.join([
            'Traceback (last call):\n',
            *traceback.StackSummary.from_list(frames).format(colorize=True),
            *traceback.format_exception_only(exc_type, exc_value, colorize=True)
        ]))

    def printErrors(self):
        pass

    def addSuccess(self, test):
        super().addSuccess(test)
        print(f'{ERASE_LINE}{PASSED_HEADER} {test}')

    def addSkip(self, test, reason):
        super().addSkip(test, reason)
        print(f'{ERASE_LINE}{SKIPPED_HEADER} {test}')

    def addFailure(self, test, err):
        super().addFailure(test, err)
        print(f'{ERASE_LINE}{FAILED_HEADER} {test}')
        self.print_error(err)

    def addError(self, test, err):
        super().addError(test, err)
        print(f'{ERASE_LINE}{ERROR_HEADER} {test}')
        self.print_error(err, full_stacktrace=True)

    def startTest(self, test):
        super().startTest(test)
        print(f'{RUNNING_HEADER} {test} ...', end='\r')

    def summary(self) -> Summary:
        total   = self.testsRun
        failed  = len(self.failures)
        errors  = len(self.errors)
        skipped = len(getattr(self, 'skipped', []))
        passed  = total - failed - errors - skipped
        return self.Summary(total, failed, errors, skipped, passed)


class ColoredTextTestRunner(unittest.TextTestRunner):
    def run(self, test) -> ColoredTestResult.Summary:
        result = ColoredTestResult(self.stream, self.descriptions, self.verbosity)

        start = time.perf_counter()
        test(result)
        end = time.perf_counter()

        summary = result.summary()

        # TODO: Does magenta look good here?
        print(f'\n{COLOR_MAGENTA}Summary: Ran {summary.total} tests in {end - start:.03}s{COLOR_RESET}')
        print(f'{PASSED_HEADER}  {summary.passed}')
        print(f'{FAILED_HEADER}  {summary.failed}')
        print(f'{ERROR_HEADER}   {summary.errors}')
        print(f'{SKIPPED_HEADER} {summary.skipped}')

        return summary


def run() -> bool:
    suite = unittest.TestSuite()

    for case in TEST_CASES:
        suite.addTests(unittest.TestLoader().loadTestsFromTestCase(case))

    runner = ColoredTextTestRunner(verbosity=0)
    summary = runner.run(suite)

    return summary.total == summary.passed
