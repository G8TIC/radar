# RADAR UDP/IP wire format

This describes the wire format for Radar V2 protocol.

## UDP port number

Radar V2 protocol uses UDP port number 5997.

## Message format

Each ADS-B message sent to the central aggregator comprises the following components in a 50-byte UDP/IP packet.

```
+-------------------------+-------------------------+-------------+--------+-------------------+------+-------------------------------------------+-------------------------+
|        API Key          |       Time Stamp        |   Sequence  | Opcode |       MLAT        | RSSI |                 ADS-B Data                |         Auth Tag        |
+-------------------------+-------------------------+-------------+--------+-------------------+------+-------------------------------------------+-------------------------+
| 69 69 00 00 00 00 43 79 | 95 B4 4A E4 01 0B 06 00 | 95 00 00 00 |   03   | 1F C4 3F 33 1A D2 |  27  | 8D 4C AD E6 99 14 7A 22 18 68 0A 7B F7 F9 | 10 AC F2 F8 D3 34 49 6F |
+-------------------------+-------------------------+-------------+--------+-------------------+------+-------------------------------------------+-------------------------+
```

Where:

### API key

The API key is the unique identity of the sending station as a 64-bit number (little endian)

### Time Stamp

The timestamp is the number of micro-seconds since the unix epoch on 1st Jan 1970
expressed as a 64-bit unsigned integer (little endian).

### Sequence

Is the message sequence number, an unsigned 32-bit integer that starts at 1
and rolls through zero. (Little endian)

### Opcode

The Opcode is an 8-bit number that indicates the message type and can be Mode-A/C, Mode-S (short), Mode-S (extended), Telemerty, Radio Stats or Keep-Alive.

Opcodes are defined in radar.h

### MLAT

The MLAT is the 48-bit multi-lateration clock counter from readsb/dump1090.

### RSSI

The Receiver Signal Strength Indication (RSSI) is the received signal stength in dBFS.

### ADS-B Data

ADS-B Data is the 112-bit/14-byte ADS-B DF17/DF18/DF19 Extended Squitter as specified in RTCA DO-260 / EuroCAE ED-102.

### Auth Tag

The Authentication tag is a 64-bit truncacted HMAC-SHA256 of the message for integrity checking (little endian).

## Other messges

There are some other mesasges that are sent:

### Keep-alive

If your station receives no traffic for a second then we send a "hello" or
keep-alive so that the aggregator knows you hav not gone away.

### Radio Statistics

The 1090MHz radio channel local to you may contain a huhe amount of traffic
that is meaningless to us inlcuding kegcacy Mode-A, Mode-C and Mode-S
(short) messages.

We count the quantity of these messages to understand how busy the radio
channel is in your locality and send traffic counts every 15 minutes.

See stats.c/stats.h for more details.

### System Telemetry

We monitor the envionment and operation of the receiver platform periodically
in order to detect bugs or operation that we need to fix.

See telemetry.c/telemetry.h for more details.
