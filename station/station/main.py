from station import tests
import argparse

__VERSION__ = '0.1'

def main():
    print(f'LifeMonitorStation v{__VERSION__}')

    parser = argparse.ArgumentParser(prog='LifeMonitorStation')

    parser.add_argument('-t', '--tests', action='store_true', help='Run tests', dest='tests', default=False)

    args = parser.parse_args()

    if args.tests:
        tests.run()

if __name__ == '__main__':
    main()
