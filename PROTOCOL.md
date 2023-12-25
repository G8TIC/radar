# RADAR UDP/IP wire format

This describes the wire format for Radar V2 protocol.

## UDP port number

Radar V2 protocol uses UDP port number 5997.

## Message format

Each ADS-B message send to the central aggregator comprises the following components in a 50-byte UDP/IP
packet.

Message formats are defined in radar.h

The most frequently used message is the Mode-S(ES) update:

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

