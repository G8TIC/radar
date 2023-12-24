# RADAR UDP/IP wire format

This describes the wire format for Radar V2 protocol.

## UDP port number

Radar V2 protocol uses UDP port number 5997.

## Message format

Each radar update message comprises the following components in a 50-byte UDP/IP message sent to
the aggregator

```
69 69 00 00 00 00 43 79    95 B4 4A E4 01 0B 06 00   95 00 00 00    03     1F C4 3F 33 1A D2    27    8D 4C AD E6 99 14 7A 22 18 68 0A 7B F7 F9     10 AC F2 F8 D3 34 49 6F
<------ API key ------>    <------timestamp ----->   <-- seq -->  opcode   <---- mlat ----->   rssi   <-------------- squitter --------------->     <-------- atag ------->
```

Where:

### API key

The API key is the unique identity of your station as a 64-bit number (little endian)

### Timestamp

The timestamp is the number of micro-seconds since the unix epoch on 1st Jan 1970
expressed as a 64-bit unsigned integer (little endian).

### Seq

Is the message sequence number, an unsigned 32-bit integer that starts at 1
and rolls through zero. (Little endian)

### Opcode

The Opcode is an 8-bit number that indicates the message type and can be Mode-A/C, Mode-S (short), Mode-S (extended), Telemerty, Radio Stats or Keep-Alive.

### MLAT

The MLAT is the 48-bit multi-lateration clock counter from readsb/dump1090.

### RSSI

The RSSI is the Receiver Signal Strength Indication (RSSI) in dBFS.

### Squitter

Squitter is the 112-bit/14-byte ADS-B Extended Squitter according to RTCA DO-260 / EuroCAD ED-102.

### Atag

The Authentication tag (Atag) is a 64-bit truncacted HMAX-SHA256 of the message for integrity checking (little endian).


