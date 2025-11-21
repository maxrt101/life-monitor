from station import config, radio, db, utils
from typing import Callable
import copy
import time
import sys


def log_info(msg: str):
    utils.logger.info(f'SCRIPT: {msg}')

def log_debug(msg: str):
    utils.logger.debug(f'SCRIPT: {msg}')


class Device:
    def __init__(self, mac: str, name: str, ver: str):
        self.mac = utils.parse_int(mac)
        self.name = name
        self.ver = ver
        self.key = config.CONFIG_RADIO_DEFAULT_KEY
        self.station_mac = 0

    def __str__(self):
        return f'Device(mac=0x{self.mac:X} name={self.name} ver={self.ver} key={self.key} station_mac={self.station_mac})'


class LoopContext:
    def __init__(self, start_index: int, limit: int):
        self.start_index = start_index
        self.limit = limit
        self.cycles = 0


class Script:
    def __init__(self, net=None):
        self.devices = dict()
        self.variables = dict()
        self.commands = []
        self.index = 0
        self.loops = []

        if net:
            self.net = net
        else:
            db.init()
            self.net = radio.Network(radio.create_driver(''), config.CONFIG_RADIO_KEY, config.CONFIG_RADIO_DEFAULT_KEY)


    def __dev(self, name: str) -> Device:
        if name in self.devices.keys():
            return self.devices[name]
        raise ValueError(f'Unknown device {name}')


    def load(self, src: str):
        self.commands = self.__split_into_tokens(src)
        self.commands = self.__process_escapes(self.commands)


    def load_var(self, var: str):
        decl, val = var.split('=')
        name, kind = decl.split(':')
        self.variables[name] = self.__cast(kind, val)


    def __split_into_tokens(self, txt: str) -> list[list[str]]:
        return [line.split() for line in txt.split('\n') if line and line[0][0] != '#']


    def __process_escapes(self, commands: list[list[str]]) -> list[list[str]]:
        found = True

        while found:
            found = False

            for cmd in commands:
                i = 0
                size = len(cmd)

                while i < size:
                    if len(cmd[i]) > 0 and cmd[i][-1] == '\\':
                        if i + 1 < size:
                            cmd[i] = cmd[i][:-1] + ' ' + cmd[i+1]
                            cmd.pop(i+1)
                            size -= 1
                        else:
                            cmd[i] = cmd[i][:-1]
                        found = True
                    i += 1

        return commands


    def __list_get(self, name: str):
        name, idx = name.split(':')
        if '.' in name:
            obj = self.__process_expr(name)
        else:
            obj = self.variables[name]
        return obj[int(self.__process_expr(idx))]


    def __list_set(self, name: str, value):
        name, idx = name.split(':')
        if '.' in name:
            obj = self.__process_expr(name)
        else:
            obj = self.variables[name]
        obj[int(self.__process_expr(idx))] = value


    def __dict_get(self, name: str):
        chain = name.split('.')
        obj = self.variables
        for k in chain:
            obj = obj[k]
        return obj


    def __dict_set(self, name: str, value):
        chain = name.split('.')
        obj = self.variables
        for k in chain[:-1]:
            obj = obj[k]
        obj[chain[-1]] = value


    def __expr(self, name: str):
        if name[-1] == '?':
            return name[:-1] in self.variables

        if '?' in name:
            cond, rest = name.split('?')
            then_val, else_val = rest.split('!')
            return self.__process_expr(then_val) if cond in self.variables else self.__process_expr(else_val)

        elif '|' in name:
            name, kind = name.split('|')
            return self.__cast(kind, self.__expr(name))

        elif '+' in name:
            a, b = name.split('+')
            return int(self.__process_expr(a)) + int(self.__process_expr(b))

        elif '-' in name:
            a, b = name.split('-')
            return int(self.__process_expr(a)) - int(self.__process_expr(b))

        elif '*' in name:
            a, b = name.split('*')
            return int(self.__process_expr(a)) * int(self.__process_expr(b))

        elif '/' in name:
            a, b = name.split('/')
            return int(self.__process_expr(a)) / int(self.__process_expr(b))

        elif '==' in name:
            a, b = name.split('==')
            return int(self.__process_expr(a)) == int(self.__process_expr(b))

        elif '!=' in name:
            a, b = name.split('!=')
            return int(self.__process_expr(a)) != int(self.__process_expr(b))

        elif ':' in name:
            return self.__list_get(name)

        elif '.' in name:
            return self.__dict_get(name)

        if name in self.variables:
            return self.variables[name]
        else:
            return name


    def __cast(self, kind: str, value: str | list[str]):
        match kind:
            case 'int':
                return utils.parse_int(value[0] if type(value) is list else value)
            case 'str':
                return ' '.join(value)
            case 'strarr':
                if type(value) is list:
                    val = [x for sub in value for x in sub.split(',')]
                else:
                    val = value.split(',')
                return [self.__process_expr(x) for x in val]
            case 'intarr':
                if type(value) is list:
                    val = [x for sub in value for x in sub.split(',')]
                else:
                    val = value.split(',')
                return [utils.parse_int(x) for x in val]
            case 'dict':
                # print(value)
                #
                # val = []
                #
                # if type(value) is list:
                #     for i in range(len(value)):
                #         if not self.__is_expr(value[i]) and ',' in value[i]:
                #             val.extend(value[i].split(','))
                # else:
                #     if not self.__is_expr(value) and ',' in value:
                #         val = value.split(',')
                #
                # print(value)

                if not self.__is_expr(value) and ',' in value:
                    value = value.split(',')

                return {kv[0]: self.__process_expr(kv[1]) for kv in [kv.split('=', 1) for kv in value]}
            case _:
                raise ValueError(f'Invalid type for cast: {kind}')


    @classmethod
    def __convert_type(cls, t, val: str):
        if t is int:
            return utils.parse_int(val)
        elif t is list:
            raise NotImplementedError()
        return val


    def __is_expr(self, token: str) -> bool:
        return len(token) > 0 and token[0] == '{' and token[-1] == '}'


    def __process_expr(self, token: str) -> str:
        if self.__is_expr(token):
            expr = token[1:-1]
            return self.__expr(expr)
        return token


    def __process_exprs(self, tokens: list[str]) -> list[str]:
        for j in range(len(tokens)):
            tokens[j] = self.__process_expr(tokens[j])
        return tokens


    def __parse_pack(self, pack: list[str]) -> list | dict:
        if any([True for arg in pack if '=' in arg]):
            res = dict()
            for arg in pack:
                tokens = arg.split('=')
                res[tokens[0]] = self.__process_expr(tokens[1])
            return res
        else:
            return pack


    def __call_with_pack(self, callee: Callable, pack: list | dict):
        pack = self.__parse_pack(pack)
        if type(pack) is dict:
            return callee(**pack)
        else:
            return callee(*pack)

    def __replace_with_pack(self, dest: dict, pack: list | dict):
        pack = self.__parse_pack(pack)
        if type(pack) is dict:
            for k in dest.keys():
                if k in pack:
                    dest[k] = self.__convert_type(type(dest[k]), pack[k])
        else:
            for i in range(len(pack)):
                dest[i] = self.__convert_type(type(dest[i]), pack[i])
        return dest



    def __packet_from_tokens(self, dev: str, cmd: str, payload: list[str]):
        payload = self.__parse_pack(payload)

        payloads = {
            'PING': {},
            'CONFIRM': {},
            'REJECT': {
                'reason': 0
            },
            'REGISTER': {
                'hw_ver':       0,
                'sw_ver_major': 0,
                'sw_ver_minor': 0,
                'sw_ver_patch': 0
            },
            'REGISTRATION_DATA': {
                'station_mac': config.CONFIG_STATION_MAC,
                'net_key':     config.CONFIG_RADIO_KEY
            },
            'STATUS': {
                'flags':        0,
                'reset_reason': 0,
                'reset_count':  1,
                'cpu_temp':     20,
                'bpm':          70,
                'avg_bpm':      70
            },
            'LOCATION': {
                'lat_dir':  'N',
                'lat':      '49.4397313',
                'long_dir': 'E',
                'long':     '23.4025276'
            },
            'ALERT': {
                'trigger': 0
            }
        }

        header = {
            'transport': radio.TransportType.UNICAST.value,
            'origin': self.__dev(dev).mac,
            'target': self.__dev(dev).station_mac,
            'key': self.__dev(dev).key,
        }

        header = self.__replace_with_pack(header, payload)
        payload = self.__replace_with_pack(payloads[cmd], payload)

        return radio.Packet.create(
            command=radio.Command[cmd],
            transport=header['transport'],
            origin=header['origin'],
            target=header['target'],
            key=header['key'],
            **payload
        )

    @classmethod
    def __packet_unpack(cls, packet: radio.Packet) -> dict:
        data = {
            'command':   packet.header.command.name,
            'size':      packet.header.size,
            'packet_id': packet.header.packet_id,
            'repeat':    packet.header.repeat,
            'transport': packet.header.transport.name,
            'origin':    packet.header.origin,
            'target':    packet.header.target
        }

        payloads = {
            'PING':              lambda _: {},
            'CONFIRM':           lambda _: {},
            'REJECT':            lambda p: {'reason': p.payload.reason},
            'REGISTER':          lambda p: {'hw_ver': p.payload.hw_ver, 'sw_ver_major': p.payload.sw_ver_major, 'sw_ver_minor': p.payload.sw_ver_minor, 'sw_ver_patch': p.payload.sw_ver_patch},
            'REGISTRATION_DATA': lambda p: {'station_mac': p.payload.station_mac, 'net_key': p.payload.net_key},
            'STATUS':            lambda p: {'flags': p.payload.flags, 'reset_reason': p.payload.reset_reason, 'reset_count': p.payload.reset_count, 'cpu_temp': p.payload.cpu_temp, 'bpm': p.payload.bpm, 'avg_bpm': p.payload.avg_bpm},
            'LOCATION':          lambda p: {'lat_dir': p.payload.lat_dir, 'lat': p.payload.lat, 'long_dir': p.payload.long_dir, 'long': p.payload.long},
            'ALERT':             lambda p: {'trigger': p.payload.trigger}
        }

        data.update(payloads[packet.header.command.name](packet))

        return data


    def __cmd_echo(self, subcmd: list[str]):
        log_debug(' '.join([str(x) for x in subcmd]))


    def __cmd_list(self, _subcmd: list[str]):
        log_info('Available devices:')
        for name, dev in self.devices.items():
            log_info(f'  {name}: {dev}')


    def __cmd_vars(self, _subcmd: list[str]):
        log_info('Defined variables:')
        for name, value in self.variables.items():
            log_info(f'  {name}: {value}')


    def __cmd_set(self, subcmd: list[str]):
        name, kind, value = subcmd[0], subcmd[1], subcmd[2:]
        self.variables[name] = self.__cast(kind, value)
        log_debug(f'NEW var {name} ({kind}): {value}')


    def __cmd_upd(self, subcmd: list[str]):
        name, value = subcmd[0], subcmd[1]

        if '.' in name:
            self.__dict_set(name, value)
        elif ':' in name:
            self.__list_set(name, value)
        else:
            self.variables[subcmd[0]] = subcmd[1]
        log_debug(f'UPD var {name}: {value}')


    def __cmd_device(self, subcmd: list[str]):
        if subcmd[0] == 'new':
            self.devices[subcmd[1]] = self.__call_with_pack(Device, subcmd[2:])
            log_info(f'New {self.devices[subcmd[1]]}')
            return

        dev = subcmd[0]

        match subcmd[1]:
            case 'pkt':
                packet = self.__packet_from_tokens(
                    dev=dev,
                    cmd=subcmd[2],
                    payload=subcmd[3:]
                )
                log_debug(f'Packet: {packet}')

            case 'send':
                packet = self.__packet_from_tokens(
                    dev=dev,
                    cmd=subcmd[2],
                    payload=subcmd[3:]
                )
                self.net.driver.next_packet(packet.to_bytes())
                log_info(f'Send {dev}: {packet}')

            case 'recv':
                cmd = subcmd[2]

                if not self.net.driver.last_out_packet:
                    # TODO: Don't advance `i` in run(), wait for packet
                    raise ValueError('No packet to receive')

                packet = radio.Packet.from_bytes(self.net.driver.last_out_packet, self.__dev(dev).key)
                data = self.__packet_unpack(packet)
                self.variables['packet'] = data

                log_info(f'Recv {dev}: {packet}')

                if data['command'] != cmd:
                    raise AssertionError(f'Mismatch in packet command: expected {cmd}, got {data["command"]}')

                match_fields = self.__parse_pack(subcmd[3:])

                if type(match_fields) is dict:
                    for k, v in match_fields.items():
                        if data[k] != self.__convert_type(type(data[k]), v):
                            raise AssertionError(f'Mismatch in packet field {k}: expected {v}, got {data[k]}')

            case 'set-key':
                self.__dev(dev).key = subcmd[2]

            case 'set-station-mac':
                self.__dev(dev).station_mac = subcmd[2]

            case _:
                raise ValueError(f'Unknown subcommand {subcmd[1]} for command device')


    def __cmd_station(self, subcmd: list[str]):
        if subcmd[0] == 'reg':
            dev = subcmd[1]

            self.net.start_registration(self.__dev(dev).name, self.__dev(dev).mac)
        elif subcmd[0] == 'check':
            dev = subcmd[1]

            db.Device.get_by_id(self.__dev(dev).mac)
        else:
            raise ValueError()


    def __cmd_wait(self, subcmd: list[str]):
        time.sleep(int(subcmd[0]) / 1000)


    def __cmd_assert(self, subcmd: list[str]):
        if len(subcmd) == 0:
            raise AssertionError('Manual assert')
        elif len(subcmd) == 1:
            if not subcmd[0]:
                raise AssertionError(f'Assert {subcmd[0]} failed')
        else:
            if subcmd[0] != subcmd[1]:
                raise AssertionError(f'Assert {subcmd[0]} == {subcmd[1]} failed')


    def run_cmd(self, tokens: list[str]):
        tokens = self.__process_exprs(tokens)

        cmd_handlers = {
            'echo':    self.__cmd_echo,
            'list':    self.__cmd_list,
            'vars':    self.__cmd_vars,
            'set':     self.__cmd_set,
            'upd':     self.__cmd_upd,
            'dev':     self.__cmd_device,
            'device':  self.__cmd_device,
            'sta':     self.__cmd_station,
            'station': self.__cmd_station,
            'wait':    self.__cmd_wait,
            'assert':  self.__cmd_assert,
        }

        return cmd_handlers[tokens[0]](tokens[1:])


    def __handle_loop(self, cmd: list[str]):
        self.loops.append(LoopContext(
            start_index=self.index + 1,
            limit=int(cmd[1]) if len(cmd) > 1 else 0
        ))
        self.index += 1


    def __handle_endloop(self, _cmd: list[str]):
        if not len(self.loops):
            raise ValueError('Not in a loop')
        loop = self.loops[-1]
        loop.cycles += 1
        self.index = self.index + 1 if loop.limit and loop.cycles >= loop.limit else loop.start_index


    def __handle_break(self, _cmd: list[str]):
        stack = 1
        while True:
            if self.commands[self.index][0] == 'loop':
                stack += 1
            if self.commands[self.index][0] == 'endloop':
                stack -= 1
            if not stack:
                break
            self.index += 1
        self.index += 1


    def __handle_if(self, cmd: list[str]):
        cond = self.__process_expr(cmd[1])
        self.index += 1
        if not cond:
            stack = 1
            while True:
                if self.commands[self.index][0] == 'if':
                    stack += 1
                if self.commands[self.index][0] == 'endif':
                    stack -= 1
                if not stack:
                    break
                self.index += 1
            self.index += 1


    def __handle_endif(self, _cmd: list[str]):
        self.index += 1


    def __handle_bkpt(self, _cmd: list[str]):
        self.index += 1
        while True:
            inp = input('> ')

            if inp == 'resume':
                return True

            if inp == 'stop':
                return False

            commands = self.__split_into_tokens(inp)
            commands = self.__process_escapes(commands)

            for c in commands:
                self.run_cmd(c)


    def cycle(self):
        if self.index >= len(self.commands):
            return False

        cmd = copy.deepcopy(self.commands[self.index])

        log_debug(f'CYCLE {self.index}: {cmd}')

        match cmd[0]:
            case 'exit':
                return False
            case 'loop':
                self.__handle_loop(cmd)
                return True
            case 'endloop':
                self.__handle_endloop(cmd)
                return True
            case 'break':
                self.__handle_break(cmd)
                return True
            case 'if':
                self.__handle_if(cmd)
                return True
            case 'endif':
                self.__handle_endif(cmd)
                return True
            case 'bkpt':
                return self.__handle_bkpt(cmd)

        self.run_cmd(cmd)
        self.net.cycle()

        self.index += 1

        return True


    def run(self):
        self.index = 0
        while self.index < len(self.commands):
            if not self.cycle():
                return


def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} FILE')
        exit(1)

    with open(sys.argv[1], 'r') as f:
        src = f.read()

    try:
        script = Script()
        script.load(src)

        for var in sys.argv[2:]:
            script.load_var(var)

        script.run()
    except KeyboardInterrupt:
        print('\nStopping...')
    finally:
        db.deinit()


if __name__ == '__main__':
    main()
