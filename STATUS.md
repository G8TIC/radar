# Receiver Status

Once you have your receiver confiogured and working you can check it's
status at the aggregator using the 'mystatus' facility.

You send a request to the main website inclduing your charing key and the
website will return a JSON object containing the current status of your
receiver.


## Request

The request is a HTTPS GET on the following URL:

...
    https://www.1090mhz.uk/mystatus.php?key=0x7000000000000000
...

Where 'key' is your 64-bit sharing key expressed as a hexadecimal number.


## Result

The server resturns a JSON object as follows

...
{
    "id": 1,
    "name": "NAME",
    "key": "0x7000000000000000",
    "secret": "***REDACTED***",
    "version": "2.03-2",
    "lat": 52.2*****,
    "lng": -2.1*****,
    "alt": 75,
    "max_range": 360,
    "ipaddr": "82.68.***.***",
    "online": 1,
    "first_heard": "2024-01-15 22:54:48",
    "last_heard": "2024-01-16 10:40:52",
    "lost_comms": "1970-01-01 00:00:00",
    "msg_total": 4201147,
    "msg_unique": 1746156,
    "msg_dupe_intra": 0,
    "msg_dupe_inter": 2385151,
    "msg_dupe_system": 69840,
    "crc_good": 633388,
    "crc_fail": 0,
    "df16": 0,
    "df17": 633185,
    "df18": 203,
    "df19": 0,
    "df20": 883245,
    "df21": 229523,
    "df22": 0,
    "positions": 120523,
    "ave_dist": 62.54,
    "max_dist": 343,
    "percent": 41.56,
    "contrib": 4.56
}
...

Most of the fields are self explanitory.

The DF numbers refer to the various ADS-B Downlink Formats (message types).

Positions are the number of unique positions we received and decoded from you.

The ave_dist is the average distance you are receiving aircraft from (higher indicates
better).

The max_distance is the furthest distance you have received an aircraft from
which will always be less than max_range.

The percent figure indicates the percentage of messages your station sent
the aggregator and the aggregator used.

The contrib value is a percentage of how many messages that teh aggregator
used from yout station compared with everyone else.

