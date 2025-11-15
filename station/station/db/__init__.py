from station.utils import logger
from station.config import CONFIG_DB_FILE_PATH
from werkzeug.security import generate_password_hash
from peewee import *
import datetime


# Global database connection
# Should be re-opened in threads
conn = SqliteDatabase(CONFIG_DB_FILE_PATH)


class BaseModel(Model):
    class Meta:
        database = conn


class User(BaseModel):
    username      = CharField(unique=True)
    password_hash = CharField()


class Device(BaseModel):
    mac     = PrimaryKeyField()
    name    = CharField()
    version = CharField()


class Status(BaseModel):
    device    = ForeignKeyField(Device, backref='status')
    timestamp = DateTimeField(default=datetime.datetime.now)

    flags   = IntegerField()
    bpm     = IntegerField()
    avg_bpm = IntegerField()


class Location(BaseModel):
    device    = ForeignKeyField(Device, backref='locations')
    timestamp = DateTimeField(default=datetime.datetime.now)

    latitude_direction  = CharField()
    latitude            = DoubleField()
    longitude_direction = CharField()
    longitude           = DoubleField()


class Alert(BaseModel):
    device    = ForeignKeyField(Device, backref='alerts')
    timestamp = DateTimeField(default=datetime.datetime.now)

    trigger = IntegerField()


def init():
    conn.connect()
    conn.create_tables([User, Device, Status, Location, Alert])

    # Unconditionally create 'admin' user
    if not User.select().where(User.username == 'admin').exists():
        logger.info("Creating default admin user...")
        hashed_password = generate_password_hash('admin')
        User.create(username='admin', password_hash=hashed_password)
        logger.info("User 'admin' created with password 'password'")


def deinit():
    if not conn.is_closed():
        conn.close()
