#!/usr/bin/python3

import argparse
import json
import sqlite3
import sys
import time

import serial
import yaml


class surveyJson:
    def __init__(self, raw: str) -> None:
        self._json = None
        self._wifiInx = 0
        self._wifiCount = 0
        self._bleInx = 0
        self._bleCount = 0

        self._loadSurvey(raw)

    def _loadSurvey(self, data: str) -> bool:
        self._json = json.loads(data)

        self._wifiCount = len(self._json["WiFiDevices"])
        self._bleCount = len(self._json["BLEDevices"])
        return True

    def getWiFiDevCount(self) -> int:
        return self._wifiCount

    def getBLEDevCount(self) -> int:
        return self._bleCount

    def getItem(self, key: str):
        try:
            return self._json[key]
        except KeyError:
            return None

    def getHeader(self):
        device = self.getItem("Device")
        notes = self.getItem("SurveyNotes")
        dateTime = self.getItem("DateTime")
        longitude = self.getItem("LocationLongLat")[1]
        lattitude = self.getItem("LocationLongLat")[0]
        satCount = self.getItem("SatelliteCount")
        version = self.getItem("DataVersion")

        if (
            notes == None
            or device == None
            or dateTime == None
            or longitude == None
            or lattitude == None
            or satCount == None
        ):
            return None

        else:
            return (notes, device, version, dateTime, longitude, lattitude, satCount)

    def getNextWifiDevice(self):
        if self._wifiInx >= self._wifiCount:
            return None

        try:
            type = self._json["WiFiDevices"][self._wifiInx]["Type"]
            subtype = self._json["WiFiDevices"][self._wifiInx]["Subtype"]
            ssid = self._json["WiFiDevices"][self._wifiInx]["SSID"]
            sourceAddr = self._json["WiFiDevices"][self._wifiInx]["SourceAddr"]
            destAddr = self._json["WiFiDevices"][self._wifiInx]["DestAddr"]
            channel = self._json["WiFiDevices"][self._wifiInx]["Channel"]
            rssi = self._json["WiFiDevices"][self._wifiInx]["RSSI"]
        except KeyError as e:
            print(e)
            return None

        self._wifiInx = self._wifiInx + 1

        return (type, subtype, ssid, sourceAddr, destAddr, channel, rssi)

    def getWifiDevice(self, inx: int):
        if inx >= self._wifiCount:
            return None

        try:
            type = self._json["WiFiDevices"][inx]["Type"]
            subtype = self._json["WiFiDevices"][inx]["Subtype"]
            ssid = self._json["WiFiDevices"][inx]["SSID"]
            sourceAddr = self._json["WiFiDevices"][inx]["SourceAddr"]
            destAddr = self._json["WiFiDevices"][inx]["DestAddr"]
            channel = self._json["WiFiDevices"][inx]["Channel"]
            rssi = self._json["WiFiDevices"][inx]["RSSI"]
        except KeyError:
            return None

        return (type, subtype, ssid, sourceAddr, destAddr, channel, rssi)

    def getNextBTDevice(self):
        if self._bleInx >= self._bleCount:
            return None

        try:
            name = self._json["BLEDevices"][self._bleInx]["Name"]
            mac = self._json["BLEDevices"][self._bleInx]["MAC"]
            rssi = self._json["BLEDevices"][self._bleInx]["RSSI"]

            uuid16 = self._json["BLEDevices"][self._bleInx]["UUID16bit"]
            uuid128 = self._json["BLEDevices"][self._bleInx]["UUID128bit"]
        except KeyError:
            return None

        self._bleInx = self._bleInx + 1

        return (name, mac, rssi, uuid16, uuid128)

    def getBTDevice(self, inx: int):
        if self.inx >= self._bleInx:
            return None

        try:
            name = self._json["BLEDevices"][inx]["name"]
            mac = self._json["BLEDevices"][inx]["mac"]
            rssi = self._json["BLEDevices"][inx]["rssi"]

            uuid16 = self._json["BLEDevices"][inx]["UUID16bit"]
            uuid128 = self._json["BLEDevices"][inx]["UUID128bit"]
        except KeyError:
            return None

        self._bleInx = self._bleInx + 1

        return (name, mac, rssi, uuid16, uuid128)


class sq3db:
    def __init__(self, fileName: str) -> None:
        self._dbConnection = sqlite3.connect(fileName)
        self._cursor = self._dbConnection.cursor()
        self._membersExists = False
        self._servicesExists = False

        self._createTables()
        self._loadYaml()
        self._loadMac()

    def _createTables(self) -> bool:
        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS surveys (
                        surveyInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        version INTEGER NOT NULL,
                        device TEXT NOT NULL,
                        notes TEXT NOT NULL,
                        dateTime TEXT NOT NULL,
                        longitude REAL NOT NULL,
                        lattitude REAL NOT NULL,
                        satCount INTEGER NOT NULL
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS wifi_data (
                        wifiInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        surveyInx INTEGER NOT NULL,
                        subtype TEXT NOT NULL,
                        sourceAddr TEXT NOT NULL,
                        destAddr TEXT NOT NULL,
                        rssi INTEGER NOT NULL,
                        channel INTEGER NOT NULL,
                        FOREIGN KEY (surveyInx)
                            REFERENCES surveys (surveyInx)
                                ON DELETE CASCADE
                                ON UPDATE NO ACTION
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS wifi_management (
                        wifiInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        surveyInx INTEGER NOT NULL,
                        subtype TEXT NOT NULL,
                        ssid TEXT NOT NULL,
                        bssid TEXT NOT NULL,
                        rssi INTEGER NOT NULL,
                        channel INTEGER NOT NULL,
                        FOREIGN KEY (surveyInx)
                            REFERENCES surveys (surveyInx)
                                ON DELETE CASCADE
                                ON UPDATE NO ACTION
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS btle (
                        btInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        surveyInx INTEGER NOT NULL,
                        name TEXT NOT NULL,
                        mac TEXT NOT NULL,
                        rssi INTEGER NOT NULL,
                        FOREIGN KEY (surveyInx)
                            REFERENCES surveys (surveyInx)
                                ON DELETE CASCADE
                                ON UPDATE NO ACTION
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS uuid16 (
                        uuidInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        btInx INTEGER NOT NULL,
                        uuid16 INTEGER NULL,
                        FOREIGN KEY (btInx)
                            REFERENCES btle (btInx)
                                ON DELETE CASCADE
                                ON UPDATE NO ACTION
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS uuid128 (
                        uuidInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        btInx INTEGER NOT NULL,
                        uuid128 TEXT NULL,
                        FOREIGN KEY (btInx)
                            REFERENCES btle (btInx)
                                ON DELETE CASCADE
                                ON UPDATE NO ACTION
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS bluetooth_uuids (
                        svcInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        uuidHex TEXT NOT NULL,
                        uuidDec INT NOT NULL,
                        name TEXT NOT NULL,
                        id TEXT NOT NULL
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        try:
            self._cursor.execute("""
                    CREATE TABLE IF NOT EXISTS mac_vendor (
                        macInx INTEGER PRIMARY KEY AUTOINCREMENT,
                        prefix TEXT NOT NULL,
                        vendorName TEXT NOT NULL
                    );
            """)
        except sqlite3.OperationalError as ex:
            print(ex)

        return True

    def _loadYaml(self) -> None:
        yamlFile = open("./service_uuids.yaml", "r")
        services = (yaml.load(yamlFile, Loader=yaml.SafeLoader))["uuids"]
        yamlFile.close()
        serviceCnt = len(services)

        yamlFile = open("./member_uuids.yaml", "r")
        members = (yaml.load(yamlFile, Loader=yaml.SafeLoader))["uuids"]
        yamlFile.close()
        memberCnt = len(members)

        yamlFile = open("./service_class_uuids.yaml", "r")
        serviceClasses = (yaml.load(yamlFile, Loader=yaml.SafeLoader))["uuids"]
        yamlFile.close()
        serviceClassCnt = len(serviceClasses)

        totalUUIDCount = int(
            self._cursor.execute("select count(1) from bluetooth_uuids").fetchone()[0]
        )

        # make sure we only load this table once.
        if totalUUIDCount != (serviceCnt + memberCnt + serviceClassCnt):
            print("Reloading YAML data from Bluetooth assigned numbers")
            self._cursor.execute("DELETE FROM bluetooth_uuids")
            for service in services:
                qstring = 'INSERT INTO bluetooth_uuids (uuidHex, uuidDec, name, id) values ("{}",{},"{}","{}");'.format(
                    hex(int(service["uuid"])),
                    service["uuid"],
                    service["name"],
                    service["id"],
                )

                self._cursor.execute(qstring)
            self.doCommit()

            for member in members:
                qstring = 'INSERT INTO bluetooth_uuids (uuidHex, uuidDec, name, id) values ("{}",{},"{}","{}");'.format(
                    hex(int(member["uuid"])),
                    member["uuid"],
                    member["name"],
                    member["name"],
                )

                self._cursor.execute(qstring)
            self.doCommit()

            for serviceClass in serviceClasses:
                qstring = 'INSERT INTO bluetooth_uuids (uuidHex, uuidDec, name, id) values ("{}",{},"{}", "{}");'.format(
                    hex(int(serviceClass["uuid"])),
                    serviceClass["uuid"],
                    serviceClass["name"],
                    serviceClass["id"],
                )

                self._cursor.execute(qstring)
            self.doCommit()
        else:
            print("Skipping YAML data from Bluetooth assigned numbers")

    def _loadMac(self) -> None:
        f = open("./mac-vendors-export.json", "r")
        macJson = json.load(f)
        f.close()

        totalVendors = int(
            self._cursor.execute("select count(1) from mac_vendor").fetchone()[0]
        )

        count = 0

        if totalVendors != len(macJson):
            print("Reloading mac-vendors-export.json")
            self._cursor.execute("DELETE FROM mac_vendor;")

            for macItem in macJson:
                qstring = 'INSERT INTO mac_vendor (prefix, vendorName) values ("{}","{}")'.format(
                    macItem["macPrefix"], macItem["vendorName"]
                )

                try:
                    self._cursor.execute(qstring)
                except sqlite3.OperationalError as ex:
                    print("Error {} on item {}".format(ex, macItem))

                    qstring = 'INSERT INTO mac_vendor (prefix, vendorName) values ("{}","{}")'.format(
                        macItem["macPrefix"], "Error in vendor name"
                    )

                    self._cursor.execute(qstring)

                count += 1
                if count >= 1000:
                    self.doCommit()
                    count = 0

            self.doCommit()
        else:
            print("Skipping reload mac-vendors-export.json")

    def doCommit(self) -> None:
        self._dbConnection.commit()

    def insertSurveyHeader(self, header: tuple):
        qstring = 'INSERT INTO surveys (notes, device, version, dateTime, longitude, lattitude, satCount) values ("{}", "{}", {},"{}",{},{},{});'.format(
            header[0], header[1], header[2], header[3], header[4], header[5], header[6]
        )

        self._cursor.execute(qstring)
        self._dbConnection.commit()

        return self._cursor.execute(
            'select seq from sqlite_sequence where name = "surveys"'
        ).fetchone()[0]

    def insertWiFiDevice(self, pk: int, wifi: tuple, delayCommit: bool = False) -> None:
        qstring = ""
        if wifi[0] == "Data":
            qstring = 'INSERT INTO wifi_data (surveyInx, subtype, sourceAddr, destAddr, channel, rssi) values ({},"{}","{}","{}",{},{});'.format(
                pk, wifi[1], wifi[3], wifi[4], wifi[5], wifi[6]
            )
        else:
            qstring = 'INSERT INTO wifi_management (surveyInx, subtype, ssid, bssid, channel, rssi) values ({},"{}","{}","{}",{},{});'.format(
                pk, wifi[1], wifi[2], wifi[3], wifi[5], wifi[6]
            )

        self._cursor.execute(qstring)

        if not delayCommit == True:
            self._dbConnection.commit()

    def insertBLEDevice(self, pk: int, bt: tuple, delayCommit: bool = False) -> None:
        btFK = 0
        qstring = 'INSERT INTO btle (surveyInx, name, mac, rssi) values ({},"{}","{}",{});'.format(
            pk, bt[0], bt[1], bt[2]
        )

        self._cursor.execute(qstring)

        if not len(bt[3]) == 0 or not len(bt[4]) == 0:
            self._dbConnection.commit
            btFK = self._cursor.execute(
                'select seq from sqlite_sequence where name = "btle"'
            ).fetchone()[0]

            for uuid16 in bt[3]:
                qstring = "INSERT INTO uuid16 (btInx, uuid16) values ({},{})".format(
                    btFK, uuid16
                )
                self._cursor.execute(qstring)

            for uuid16 in bt[4]:
                qstring = "INSERT INTO uuid16 (btInx, uuid16) values ({},{})".format(
                    btFK, uuid16
                )
                self._cursor.execute(qstring)

        if not delayCommit == True:
            self._dbConnection.commit()


def stripReturn(b: bytes) -> str:
    b = b.decode("utf-8")
    first = b.find("\n") + 1
    second = b.rfind("\n")
    return b[first:second]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p", "--port", dest="port", required=True, help="Flocker serial port"
    )
    parser.add_argument("--dir", dest="ls", action="store_true", help="list files")
    parser.add_argument(
        "--cat", dest="cat", required=False, type=str, default=None, help="file to cat"
    )
    parser.add_argument(
        "--load",
        dest="load",
        required=False,
        type=str,
        default=None,
        help="file to load",
    )
    parser.add_argument(
        "--download",
        dest="download",
        required=False,
        type=str,
        default=None,
        help="Save file locally",
    )
    parser.add_argument(
        "-v", "--verbose", dest="verbose", action="store_true", help="Extra output"
    )
    parser.add_argument(
        "--dbfile",
        dest="dbfile",
        required=False,
        type=str,
        default=None,
        help="Sqlite3 DB filename",
    )
    args = parser.parse_args()

    # start with a connection to the device
    s = serial.Serial(
        port=args.port, baudrate=115200, bytesize=8, parity="N", stopbits=1, timeout=1.0
    )

    if args.ls == True:
        s.write(b"ls -d\r\n")
        s.read_until()
        time.sleep(1.0)
        print(s.read_all().decode("utf-8"))
        sys.exit(0)

    if not args.download is None:
        cmd = "cat -d {}\r\n".format(args.download)
        s.write(bytes(cmd.encode("utf-8")))
        # read firs line (echo of command), and discard
        tmp = s.read_until().decode("utf-8")
        # read the real thing
        tmp = s.read_until().decode("utf-8")
        f = open(args.download, "w")
        f.write(tmp)
        f.close()
        print("Wrote {} bytes to local file '{}'".format(len(tmp), args.download))
        sys.exit(0)

    if not args.cat is None:
        cmd = "cat -d {}\r\n".format(args.cat)
        s.write(bytes(cmd.encode("utf-8")))
        # read firs line (echo of command), and discard
        tmp = s.read_until().decode("utf-8")
        # read the real thing
        tmp = s.read_until().decode("utf-8")
        print(tmp)
        sys.exit(0)

    raw = ""

    if not args.load is None:
        cmd = "cat -d {}\r\n".format(args.load)
        s.write(bytes(cmd.encode("utf-8")))
        # read firs line (echo of command), and discard
        tmp = s.read_until().decode("utf-8")
        # read the real thing
        raw = s.read_until().decode("utf-8")

    if len(raw) < 20:
        print("bad read from file {}".format(args.load))
        sys.exit(1)

    surv = surveyJson(raw)

    if args.dbfile is None:
        print("Database file required, use '--dbfile' argument")
        sys.exit(1)

    db = sq3db(args.dbfile)

    wcount = 0
    bcount = 0

    h = surv.getHeader()
    if not h is None:
        surveyPK = db.insertSurveyHeader(h)

        w = surv.getNextWifiDevice()
        while not w is None:
            wcount += 1
            db.insertWiFiDevice(surveyPK, w, True)
            w = surv.getNextWifiDevice()

        b = surv.getNextBTDevice()
        while not b is None:
            bcount += 1
            db.insertBLEDevice(surveyPK, b, True)
            b = surv.getNextBTDevice()

        db.doCommit()

        print(
            'Loaded {} WiFi devices, {} BTLE devices from survey "{}" at {}'.format(
                wcount, bcount, surv.getItem("SurveyNotes"), surv.getItem("DateTime")
            )
        )
