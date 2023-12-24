# Radar :: 1090MHz UK ADS-B forwarder

This is the ADS-B "radar" forwarder for the 1090MHZ UK ADS-B aircraft tracking network.

Radar allows a wide range of ADS-B receivers based on SDR dongles to receive 1090MHz ADS-B and
forward interesting traffic to the 1090MHz UK aggregator using an optimised, efficient and secure
real-time UDP/IP protocol.

This radar forwarder needs access to dump1090/dump1090-mutability/dump1090-fa or readsb
on the same machine (or another machine on your LAN) and access to the BEAST protocol on
TCP port 30005.

Radar is designed to sit along side other feeders for flight tracking netwoprks such as
FlightRadar24, FlightAware, OARC and Radar360 and has no known problems sharing resources with them.

## Supported systems

Radar is designed to run 32-bit or 64-bit systems based on ARM or Intel/AMD processors
and support a wide range of hardware when used with a reasonably modern Linux OS.

### Processors

Radar has been tested on Raspberry Pi3, Pi4, NXP iMX8, Intel Atom 3000, Celeron, Pentium G4000
family, Core i3, Core i5, E-1200 Xeon and other processors.

### ADS-B receivers

Radar works with a wide range of SDR dongles such as the RTL-SDR Blog V3, Nooelec RTL-SDR, NESDR, and others
when used with dump1090, dump1090-mutability, dump1090-fa or readsb.

### Linux OS

Radar works on reasonably modern Linux operating systems that are Debian 10.x based or later including
Debian, Raspbian, Raspberry Pi OS, Ubuntu and Devuan -- it will probably work many others as it has minimal
requirements other than GCC compiler and make.

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

Intsall the code:

```
sudo make install
```

Configure your system:




## Sharing keys and station info

Each receiver station is identified by an API key or 'sharing key' which is a 64-bit hex number like:

```
0x7B432017356401A3
````

Along with the sharing key we need a station name (3-12 characters) and the
GPS location of your receiver's antenna in order to perform range checks and
MLAT.

### Station name

Your station name will be visible on the website stats page and is 3-12
characters long comprising uppercase letters, digits and underscore ("_") or
hyphen ("-").

Station names are commonly a locality name like FERNHILLL, HEATHROW, READING
or a nickname like ELMARKO or BIGCAT.


### Antenna location

In order to perform range checks and MLAT calculations we need the location
of your receiver antenna as GPS latitude and longitude to five decimal
places.  We would also like you to provide your antenna height above ground
in metres.


### Obtain a Sharing Key

You can get a sharing key from the 1090MHz website at https://1090mhz.uk or
by emailing info@1090mhz.uk or by messaging me (Mike Tubby) on Discord.


### Use a secure pass-phrase

The UDP/IP forwarding protocol digitally signs each packet using a truncated
HMAC-SHA256 which protects against message corruption, forgery and replay
attacks.

By default the messages are signed with the pass-phrase 'secret' - an
additional configuration step is requred at both the sender and aggregator
to set up a custom pass-phrase.



## Command line arguements






## Contributing

You are welcome to suggest improvements and raise issues or bugs through Github.



## Legal stuff

### Copyright

Radar is Copyright (C) 2023 by Michael J. Tubby B.Sc. MIET G8TIC but is free and open source and is
available for you to use, modify, update and change subject to the GPL license (below).

### Acknowledgements

Portions of this project (sha256, hmac-sha256) use open source code contributed by Apple Inc. and which are duly acknowleged.

### License

This is an Open Source project and is Licensed under the GNU Public License (GPL) Version 2.0, or at your option, any newer version of the GPL.

The GNU Publuic License is here: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

### NO WARRANTY

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND WITHOUT WARRANTY OF ANY KIND.
