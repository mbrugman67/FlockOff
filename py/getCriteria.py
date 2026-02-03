import argparse
import json
import sqlite3


class sq3db:
    def __init__(self, fileName: str) -> None:
        self._dbConnection = sqlite3.connect(fileName)
        self._cursor = self._dbConnection.cursor()

    def getShit(self) -> str:
        ret = "const match_mac_t wifiDefMacs[] = {"
        self._cursor.execute(
            "select prefix, vendorName from mac_vendor "
            "where vendorName like 'meta platforms%' "
            "or vendorName like 'flock%' "
            "or vendorName like 'avigilon%' "
            "or vendorName like 'axis%' "
            "or vendorName like 'dahua%' "
            "or vendorName like 'flir%' "
            "or vendorName like 'geovision%' "
            "or vendorName like 'hanwha%' "
            "or vendorName like 'hikvision' "
            "or vendorName like 'mobotix%' "
            "or vendorName like 'march networks%' "
            "or vendorName like 'sunell%' "
            "or vendorName like 'vivotek' "
            "or vendorName like 'uniview%' "
            "or vendorName like 'wbox%' "
            "or vendorName like 'china dragon%' "
            "or vendorName like 'shenzhen bil%' "
            "order by vendorName, prefix"
        )

        done = False
        count = 0
        while not done:
            try:
                (p, v) = self._cursor.fetchone()
                if len(p) == 8:
                    ret += '{{"{}", "{}"}}, '.format(p, v)
                    count += 1
                    if count > 3:
                        count = 0
                        ret += "\r\n                       "
            except TypeError:
                done = True

        ret = ret[: len(ret) - 2]
        ret += " };   // WiFi MAC match criteria\r\n"
        return ret


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--dbfile",
        dest="dbfile",
        required=False,
        type=str,
        default=None,
        help="Sqlite3 DB filename",
    )

    args = parser.parse_args()

    db = sq3db(args.dbfile)
    print(db.getShit())
