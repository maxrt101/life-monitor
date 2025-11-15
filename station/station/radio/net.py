from station.utils import logger
from station import db, config
from .packet import Packet
from .types import Command, TransportType
from .driver import Driver
from datetime import datetime, timedelta


class RegistrationContext:
    def __init__(self, name: str = '', dev_mac: int = 0, duration: int = 0):
        self.name     = name
        self.dev_mac  = dev_mac
        self.duration = duration
        self.start    = datetime.now()

    def in_progress(self) -> bool:
        # Consider registration invalid if dev_mac is 0 - which can only happen if
        # RegistrationContext was created with default values
        return self.dev_mac != 0

    def expired(self) -> bool:
        if not self.start:
            return True
        return datetime.now() - self.start >= timedelta(seconds=self.duration)


class Network:
    def __init__(self, driver: Driver, key: bytes, default_key: bytes):
        self.driver       = driver
        self.key          = key
        self.default_key  = default_key
        self.registration = RegistrationContext()


    def start_registration(self, name: str, dev_mac: int):
        # Start the registration
        self.registration = RegistrationContext(name, dev_mac, config.CONFIG_REGISTRATION_DURATION)
        logger.info(f'Starting registration for "{name}" (0x{dev_mac:X}) for {config.CONFIG_REGISTRATION_DURATION}s')


    def __send_confirm(self, dev_mac: int, key: bytes):
        self.driver.send(Packet.create(
            command=Command.CONFIRM,
            transport=TransportType.UNICAST,
            origin=config.CONFIG_STATION_MAC,
            target=dev_mac,
            key=key,
        ).to_bytes())


    def __send_reject(self, dev_mac: int, key: bytes):
        self.driver.send(Packet.create(
            command=Command.REJECT,
            transport=TransportType.UNICAST,
            origin=config.CONFIG_STATION_MAC,
            target=dev_mac,
            key=key,
            # Payload
            reason=0
        ).to_bytes())


    def __handle_ping(self, packet: Packet):
        # Check packet's target to correspond to station's node MAC
        if packet.header.target != config.CONFIG_STATION_MAC:
            logger.warning(f'PING addressed to another node (0x{packet.header.target:X}), ignoring...')
            return

        # PING must be confirmed
        self.__send_confirm(packet.header.origin, packet.key)

        logger.info(f'Received PING from 0x{packet.header.origin:X}')


    def __handle_registration(self, packet: Packet):
        # Registration flow:
        # User triggers registration on station
        # User turns on device in registration mode (with BTN held)
        # Device broadcasts REGISTER, encrypted with default key
        # Station checks registration context & responds with REGISTRATION_DATA, encrypted with default key
        # Device sends PING, encrypted with station key, to confirm that connection within the network is established
        # Station responds with CONFIRM, encrypted with station key
        # Device saves registration data (station mac & key)
        # Station saved device mac into the DB, marking registration complete

        dev_mac = packet.header.origin

        # Ignore, if no registration is in progress (maybe it's meant for another station in radio range)
        if not self.registration.in_progress():
            logger.warning(f'Received registration from 0x{dev_mac:X}, but no registration is in progress, rejecting...')
            return

        # Ignore, if registration MAC & device MAC are not matching (maybe another station has registration in progress in radio range)
        if dev_mac != self.registration.dev_mac:
            logger.error(f'Mismatching registration device MACs: 0x{self.registration.dev_mac:X} != 0x{dev_mac:X}')
            return

        # Try to get device from DB
        try:
            dev = db.Device.get(dev_mac)
            logger.warning(f'Device 0x{dev_mac:X} already registered')
            # If present - remove it
            db.Device.delete_by_id(dev_mac)
        except Exception:
            # Ignore if not present - it's the standard flow
            pass

        # Crate REGISTRATION_DATA
        reg_data = Packet.create(
            command=Command.REGISTRATION_DATA,
            transport=TransportType.UNICAST,
            origin=config.CONFIG_STATION_MAC,
            target=dev_mac,
            key=packet.key,
            # Payload
            station_mac=config.CONFIG_STATION_MAC,
            net_key=config.CONFIG_RADIO_KEY
        ).to_bytes()

        self.driver.send(reg_data)

        # Wait for ping
        ping = self.__recv_packet(config.CONFIG_RADIO_PACKET_LISTEN_DURATION)

        if ping:
            self.__handle_ping(ping)

            # Save device into the DB, only if ping is received
            db.Device.create(
                mac=dev_mac,
                name=self.registration.name,
                version=f'{packet.payload.hw_ver}.{packet.payload.sw_ver_major}.{packet.payload.sw_ver_minor}.{packet.payload.sw_ver_patch}'
            ).save()

            logger.info(f'Registered 0x{dev_mac:X}')

            # Reset registration
            self.registration = RegistrationContext()
        else:
            logger.error(f'Failed to register 0x{dev_mac:X}, no response')


    def __handle_status(self, packet: Packet):
        # Check packet's target to correspond to station's node MAC
        if packet.header.target != config.CONFIG_STATION_MAC:
            logger.warning(f'STATUS addressed to another node (0x{packet.header.target:X}), ignoring...')
            return

        try:
            # Save status record into DB
            dev = db.Device.get_by_id(packet.header.origin)
            db.Status.create(
                flags=packet.payload.flags,
                bpm=packet.payload.bpm,
                avg_bpm=packet.payload.avg_bpm,
                device=dev
            ).save()
            # STATUS shouldn't be confirmed
            logger.info(f'Received STATUS from 0x{packet.header.origin:X}: {packet.payload}')
        except Exception as e:
            logger.error(f'Failed to save STATUS data from 0x{packet.header.origin:X}: {e}')


    def __handle_location(self, packet: Packet):
        # Check packet's target to correspond to station's node MAC
        if packet.header.target != config.CONFIG_STATION_MAC:
            logger.warning(f'LOCATION addressed to another node (0x{packet.header.target:X}), ignoring...')
            return

        try:
            # Save location record into DB
            dev = db.Device.get_by_id(packet.header.origin)
            db.Location.create(
                latitude_direction=packet.payload.lat_dir,
                latitude=float(packet.payload.lat)/100,
                longitude_direction=packet.payload.long_dir,
                longitude=float(packet.payload.long)/100,
                device=dev
            ).save()
            # LOCATION shouldn't be confirmed
            logger.info(f'Received LOCATION from 0x{packet.header.origin:X}: {packet.payload}')
        except Exception as e:
            logger.error(f'Failed to save LOCATION data from 0x{packet.header.origin:X}: {e}')


    def __handle_alert(self, packet: Packet):
        # Check packet's target to correspond to station's node MAC
        if packet.header.target != config.CONFIG_STATION_MAC:
            logger.warning(f'ALERT addressed to another node (0x{packet.header.target:X}), ignoring...')
            return

        try:
            # Save alert record into DB
            dev = db.Device.get_by_id(packet.header.origin)
            db.Alert.create(
                trigger=packet.payload.trigger.value,
                device=dev
            ).save()
            # ALERT shouldn't be confirmed
            logger.info(f'Received ALERT from 0x{packet.header.origin:X}: {packet.payload}')
        except Exception as e:
            logger.error(f'Failed to save ALERT data from 0x{packet.header.origin:X}: {e}')


    def __handle_packet(self, packet: Packet):
        match packet.header.command:
            case Command.PING:
                self.__handle_ping(packet)
            case Command.REGISTER:
                self.__handle_registration(packet)
            case Command.STATUS:
                self.__handle_status(packet)
            case Command.LOCATION:
                self.__handle_location(packet)
            case Command.ALERT:
                self.__handle_alert(packet)
            case _:
                logger.warning(f'Unexpected command: {packet.header.command.name} ({packet.header.command.value}) from 0x{packet.header.origin:X}')
                # TODO: Send reject?


    def __recv_packet(self, timeout_ms: int) -> Packet | None:
        packet = self.driver.recv(timeout_ms)

        if packet is None:
            return None

        try:
            return Packet.from_bytes(packet, config.CONFIG_RADIO_KEY)
        except Exception as e1:
            try:
                return Packet.from_bytes(packet, config.CONFIG_RADIO_DEFAULT_KEY)
            except Exception as e2:
                logger.error(f'Failed to parse packet: {e1}; {e2}')
                return None


    def cycle(self):
        # Listen for packet
        packet = self.__recv_packet(config.CONFIG_RADIO_PACKET_LISTEN_DURATION)

        # If packet is received - handle it
        if packet:
            self.__handle_packet(packet)

        # Check for expired registration
        if self.registration.in_progress() and self.registration.expired():
            logger.error(f'Registration for 0x{self.registration.dev_mac:X} expired')
            self.registration = RegistrationContext()
