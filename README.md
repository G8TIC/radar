# Radar :: 1090MHz UK ADS-B forwarder

This project is the "radar" forwarder for the 1090MHz UK ADS-B aircraft tracking network.

Radar allows a wide range of ADS-B receivers based on SDR dongles to receive 1090MHz ADS-B and
forward interesting traffic to the 1090MHz UK aggregator using an optimised, efficient and secure
real-time UDP/IP protocol.

Radar is designed to sit along side other feeders for flight tracking networks such as
FlightRadar24, FlightAware, OARC and Radar360 and has no known problems sharing resources with them.

Our forwarding protocol has been carefully designed and optimised to minimise traffic
(only send relevant messages), be efficient, be very low latency for MLAT and real-time
operation, and to be secure and as a consequence  we prefer it over protocols like BEAST.

If you're looking for a pre-built Docker container image then Ramon KX1T maintains this project as a
docker image over on his site at: https://github.com/sdr-enthusiasts/docker-radar1090

If you want to send a feed to 1090MHz UK without using our dedicated feeder
protocol then we also accept BEAST Reduce Plus from Ultrafeeder and similar
systems, see: https://www.1090mhz.uk/sharing.html for info.


## Supported systems

Radar is designed to run 32-bit or 64-bit systems based on ARM or Intel/AMD processors
and support a wide range of hardware when used with a reasonably modern Linux OS.

### Processors

Radar has been tested on Raspberry Pi3, Pi4, NXP iMX8, Intel Atom 3000, Celeron, Pentium G4000
family, Core i3, Core i5, E-1200 Xeon and other processors.

### ADS-B receivers

Radar works with a wide range of SDR dongles such as the RTL-SDR Blog V3, Nooelec RTL-SDR, NESDR, and others
when used with dump1090, dump1090-mutability, dump1090-fa or readsb. That provide BEAST protocol on TCP port
30005 (preferred) or AVR protocol on TCP port 30002 (fallback).

Your ADS-B receiver/dump1090/readsb can be on the same machine as radar or
can be remote using the `-r [remote ip]` option.

### Linux OS

Radar works on reasonably modern Linux operating systems that are Debian 10.x based or later including
Debian, Raspbian, Raspberry Pi OS, Ubuntu and Devuan -- it will probably work many others as it has minimal
requirements other than GCC compiler and make.

If you have a problem running radar on another platform please [raise an issue](https://github.com/G8TIC/radar/issues)


### Init systems

Radar works with both systemd and traditional sysv-init systems, the setup script will figure out which to use.


## Installation and Setup

### Installation

Obtain the code from github:

```
git clone https://github.com/G8TIC/radar.git
```

Compile the code:

```
cd radar
make
```

Install the code:

```
sudo make install
```

Configure your system (see below for info needed):

```
sudo make setup
```


## Sharing keys and station info

Each receiver station is identified by an API key or 'sharing key' which is a 64-bit hex number like:

```
0x7B432017356401A3
````

In addition to the sharing key we need a station name and the GPS location of your receiver's
antenna in order to perform range checks and MLAT.

### Station name

Your station name will be visible on the website stats page and is 3-12
characters long comprising uppercase letters, digits and underscore ("_") or
hyphen ("-").

Station names are commonly a locality name like FERNHILLL, HEATHROW, READING
or a nickname like ELMARKO or BIGCAT.


### Antenna location

In order to perform range checks and MLAT calculations we need the location
of your receiver antenna as GPS latitude and longitude to six decimal
places.  We would also like your antenna height above ground in metres.


### Obtain a Sharing Key

You can get a sharing key from the 1090MHz website at https://1090mhz.uk or
by emailing info@1090mhz.uk or by messaging me (Mike Tubby) on Discord.


### Pass phrase

The UDP/IP forwarding protocol digitally signs each packet using a truncated
HMAC-SHA256 which protects against message corruption in transit, forgery and
replay attacks.

If you haven't been given a pass phrase when you run setup you can skip this
and the system will use the default pass-phrase of 'secret'.

We might contact you to ask you to use a new pass-phrase (known only to you and
us) to secure the feed in which case you'll need to run setup again or manually
edit the configuration file `/etc/default/radar` and edit the `-p` option to
change the pass phrase.


## Configuration

### Default configuration

The configuration for radar forwarder is stored in:

```
	/etc/default/radar
```

where the minimum is your sharing key provided by the `-k <key>` option.

Other options are described below.


### Command line options

The radar code has several command line options that can be used in the
OPTIONS= line of the config file - here is the full list:

```
  -k <key>           : sharing key (identity) of this receiver station
  -h <hostname>      : hostname of central aggregator (default: adsb-in.1090mhz.uk)
  -p <psk>           : pre-shared key for HMAC authentication (signing of messages)
  -c                 : enable sending Mode-A/C message (not recommended)
  -y                 : enable sending Mode-S Short messages (not recommended)
  -e <level>         : control which Mode-S Extended Squitter DF codes are sent (default = 1)
  -r <ip addr>       : IP address of dump1090/readsb server (default: 127.0.0.1)
  -s <seconds>       : Set the radio stats interval (default 900)
  -t <seconds>       : Set the telemetry interval (default 900)
  -d                 : run as daemon (detach from controlling tty)
  -f                 : print forwarding stats once per second
  -u <uid|username>  : set the UID or username for the process
  -g <gid|group>     : set the GID or group name for the process
  -q <qos>           : set the DSCP/IP ToS quality of service
  -v                 : display version information and exit
  -x|xx|xxx          : set debug level
  -?                 : help (this output)
```

however most have sensible defaults and are therefore not needed in operation.

## Logging

The feed protocol is real-time and does not maintain a logfile as there is
nothing to log.


## Receiver Status

The website now provide a status page which you can use to check your
receiver operation. See [STATUS.md](STATUS.md)


## Wire protocol

A description of the wire protocol is provided in [PROTOCOL.md](PROTOCOL.md)


## Telemetry and Statistics

In addition to the ADS-B data, we send some telemetry about the software and the platform that
it is running on to understand performance and discover bugs.

We also gather statistics about the radio channel and message types and rates
of reception to understand radio channel utilisation.

For details see [TELEMETRY.md](TELEMETRY.md) and [STATS.md](STATS.md)


## Contributing

You are welcome to suggest improvements and raise issues or bugs through Github.


## Legal stuff

### Copyright

This software ("Radar") is Copyright (C) 2023-2025 by Michael J. Tubby B.Sc. MIET G8TIC. All Rights Reserved.

### Acknowledgements

Portions of this project (sha256, hmac-sha256) use open source code contributed by Apple Inc. and are duly acknowledged.

Portions of this project (sha512) use open source code contributed by Aaron D. Gifford and Jelte Jansen and are duly acknowledged.

### License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Refer to file LICENSE supplied with the source code.


### No Warranty

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
CONTRIBTUORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
