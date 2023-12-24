# RADAR UDP/IP wire format

This describes the wire format for Radar V2 protocol.

## UDP port number

Radar V2 protocol uses UDP port number 5997.

## Message format

Each radar update message comprises the following components in a 50-byte UDP/IP message sent to
the aggregator


69 69 00 00 00 00 43 79    95 B4 4A E4 01 0B 06 00   95 00 00 00    03     1F C4 3F 33 1A D2    27    8D 4C AD E6 99 14 7A 22 18 68 0A 7B F7 F9     10 AC F2 F8 D3 34 49 6F
<-------- key -------->    <--------- ts -------->   <-- seq -->  opcode   <---- mlat ----->   rssi   <-------------- squitter --------------->     <-------- atag ------->


Where:

key:	  is the 64-bit API key or "sharing key", i.e. receiving station identifier

ts:	  is a 64-bit unsigned message time-stamp in uS

seq:	  is a 32-bit unsigned message sequence number (starts at 1 and rolls through zero)

opcode:	  is the messgae operation code (message type)

mlat:	  multi-lateration 48-bit clock measturement

rssi:	  received signal strength indication

squitter: the 14-byte ADS-B extended squitter

atag:	  the 64-bit authentication tag


### API key

The API key is the unique identi

### Opcode

The Opcode is the message type and can be Mode-A/C, Mode-S (short), Mode-S (extended), Telemerty, Radio Stats or Keep-Alive.

### Authentication tag

