from station.config import CONFIG_DB_FILE_PATH
from peewee import *
import datetime


db = SqliteDatabase(CONFIG_DB_FILE_PATH)


class BaseModel(Model):
    class Meta:
        database = db


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
    db.connect()
    db.create_tables([User, Device, Status, Location, Alert])

def deinit():
    db.close()
