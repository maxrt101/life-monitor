from station.radio.packet import Packet
from station.radio.types import Command, TransportType, ResetReason, AlertTrigger
from station.radio import Network, create_driver
from station.config import CONFIG_RADIO_KEY, CONFIG_RADIO_DEFAULT_KEY, CONFIG_DB_FILE_PATH, CONFIG_STATION_MAC
from station import db
from pathlib import Path
import unittest



class RadioPacketTestCase(unittest.TestCase):
    def setUp(self):
        Packet.reset_packet_id()


    def tearDown(self):
        ...


    def test_deserialize_packet_from_device(self):
        encrypted_packet = bytes([int(f'0x{x}', 16) for x in '34 6a 6f 6c 6a 6a 6a 6a b4 c7 d4 85 6a 6a 6a 6a 6a 68 6e 95 03 28 1e cc'.split(' ')])

        status = Packet.create(
            command=Command.STATUS,
            transport=TransportType.UNICAST,
            origin=0xDEADBEEF,
            target=0,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Payload
            flags=0,
            reset_reason=ResetReason.SW_RST,
            reset_count=4,
            cpu_temp=-1,
            bpm=105,
            avg_bpm=66
        )

        packet = Packet.from_bytes(encrypted_packet, CONFIG_RADIO_DEFAULT_KEY)

        self.assertEqual(packet, status)


    def test_serialize_deserialize_ping(self):
        packet = Packet.create(
            command=Command.PING,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_confirm(self):
        packet = Packet.create(
            command=Command.CONFIRM,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_reject(self):
        packet = Packet.create(
            command=Command.REJECT,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Payload
            reason=44
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_register(self):
        packet = Packet.create(
            command=Command.REGISTER,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Payload
            hw_ver=1,
            sw_ver_major=2,
            sw_ver_minor=3,
            sw_ver_patch=4,
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_registration_data(self):
        packet = Packet.create(
            command=Command.REGISTRATION_DATA,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Payload
            station_mac=0xCAFEBABE,
            net_key=CONFIG_RADIO_KEY
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_status(self):
        packet = Packet.create(
            command=Command.STATUS,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Status payload
            flags=0,
            reset_reason=ResetReason.WDG,
            reset_count=8,
            cpu_temp=5,
            bpm=0x42,
            avg_bpm=0x69
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_location(self):
        # Latitude:  N 4943.97313
        # Longitude: E 02340.25276
        # N 49.4397313 E 23.4025276
        packet = Packet.create(
            command=Command.LOCATION,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Status payload
            lat_dir='N',
            lat='4943.97313',
            long_dir='E',
            long='02340.25276'
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)


    def test_serialize_deserialize_alert(self):
        packet = Packet.create(
            command=Command.ALERT,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0xDA1BA10B,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Payload
            trigger=AlertTrigger.PULSE_THRESHOLD
        )

        packet_encrypted = packet.to_bytes()
        packet_decrypted = Packet.from_bytes(packet_encrypted, CONFIG_RADIO_DEFAULT_KEY)
        self.assertEqual(packet, packet_decrypted)



class RadioNetworkTestCase(unittest.TestCase):
    def setUp(self):
        Packet.reset_packet_id()
        self.net = Network(create_driver(), CONFIG_RADIO_KEY, CONFIG_RADIO_DEFAULT_KEY)
        Path(CONFIG_DB_FILE_PATH).unlink(missing_ok=True)
        db.init()


    def tearDown(self):
        db.deinit()


    def test_registration(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.REGISTER,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0x0,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Status payload
            hw_ver=1,
            sw_ver_major=2,
            sw_ver_minor=3,
            sw_ver_patch=4,
        ).to_bytes())

        self.net.driver.next_packet(Packet.create(
            command=Command.PING,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=CONFIG_STATION_MAC,
            key=CONFIG_RADIO_KEY,
        ).to_bytes())

        self.net.start_registration('Test', 0xEBAC0C42)
        self.net.cycle()

        # No need to assert, since get_by_id will raise an exception on failure
        print(db.Device.get_by_id(0xEBAC0C42).__dict__['__data__'])


    def test_registration_not_started(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.REGISTER,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0x0,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Status payload
            hw_ver=1,
            sw_ver_major=2,
            sw_ver_minor=3,
            sw_ver_patch=4,
        ).to_bytes())

        self.net.cycle()

        try:
            db.Device.get_by_id(0xEBAC0C42)
            self.assertTrue(False)
        except Exception:
            self.assertTrue(True)


    def test_registration_bad_mac(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.REGISTER,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=0x0,
            key=CONFIG_RADIO_DEFAULT_KEY,
            # Status payload
            hw_ver=1,
            sw_ver_major=2,
            sw_ver_minor=3,
            sw_ver_patch=4,
        ).to_bytes())

        self.net.start_registration('Test', 0xEBAC0C43)
        self.net.cycle()

        try:
            db.Device.get_by_id(0xEBAC0C42)
            self.assertTrue(False)
        except Exception:
            self.assertTrue(True)


    def test_ping(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.PING,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=CONFIG_STATION_MAC,
            key=CONFIG_RADIO_KEY,
        ).to_bytes())

        db.Device.create(
            mac=0xEBAC0C42,
            name='Test',
            version='1.0.1.0'
        ).save()

        self.net.cycle()

        deserialized = Packet.from_bytes(self.net.driver.last_out_packet, CONFIG_RADIO_KEY)
        confirm = Packet.create(
            command=Command.CONFIRM,
            transport=TransportType.UNICAST,
            origin=CONFIG_STATION_MAC,
            target=0xEBAC0C42,
            key=CONFIG_RADIO_KEY
        )
        confirm.header.packet_id = 1

        self.assertEqual(deserialized, confirm)


    def test_status(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.STATUS,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=CONFIG_STATION_MAC,
            key=CONFIG_RADIO_KEY,
            # Payload
            flags=0,
            reset_reason=ResetReason.WDG,
            reset_count=8,
            cpu_temp=5,
            bpm=0x42,
            avg_bpm=0x69
        ).to_bytes())

        db.Device.create(
            mac=0xEBAC0C42,
            name='Test',
            version='1.0.1.0'
        ).save()

        self.net.cycle()

        print(db.Status.get_by_id(1).__dict__['__data__'])


    def test_location(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.LOCATION,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=CONFIG_STATION_MAC,
            key=CONFIG_RADIO_KEY,
            # Payload
            lat_dir='N',
            lat='4943.97313',
            long_dir='E',
            long='02340.25276'
        ).to_bytes())
        #49.4397313 23.4025276
        db.Device.create(
            mac=0xEBAC0C42,
            name='Test',
            version='1.0.1.0'
        ).save()

        self.net.cycle()

        print(db.Location.get_by_id(1).__dict__['__data__'])


    def test_alert(self):
        self.net.driver.next_packet(Packet.create(
            command=Command.ALERT,
            transport=TransportType.UNICAST,
            origin=0xEBAC0C42,
            target=CONFIG_STATION_MAC,
            key=CONFIG_RADIO_KEY,
            # Payload
            trigger=AlertTrigger.PULSE_THRESHOLD
        ).to_bytes())

        db.Device.create(
            mac=0xEBAC0C42,
            name='Test',
            version='1.0.1.0'
        ).save()

        self.net.cycle()

        print(db.Alert.get_by_id(1).__dict__['__data__'])
