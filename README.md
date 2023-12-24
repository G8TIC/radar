# Radar

This is the ADS-B "radar" forwarder for the 1090MHZ UK ADS-B aircraft tracking network.

Radar allows a wide range of ADS-B receivers using SDR dongles to forward interesting traffic
to the 1090MHz UK aggregator using an optimised, efficient and secure real-time UDP/IP protocol.

Your system needs to have dump1090/dump1090-mutability/dump1090-fa or readsb
installed and with access to the BEAST protocol on local port 30005.

Radar is designed to sit along side other feeders for FlightRadar24, FlightAware, OARC and Radar360
and show have no problem sharing with them.


## Supported systems

Radar is designed to run 32-bit or 64-bit ARM and 32-bit or 64-bit Intel with a reasonably modern Linux OS.

### Tested hardware

Radar has been tested on Raspberry Pi3, Pi4, NXP iMX8, Intel Atom 3000, Celeron, Pentium G4000,
Core i3, Core i5 and others.

### Linux OS

Radar works on reasonably modern Linux operating systems that are Debian 10.x based or later including
Debian, Raspbian, Raspberry Pi OS, Ubuntu and Devuan -- it will probably work on
several others as it has minimal requirements other than GCC compiler and make.

### Init systems

Radar works with both systemd and traditional sysv-init systems, the setup script will figure out which to use.


## Configururation and Info

To use the Radar forwarder you need an API key/sharing key which is a 64-bit hex number like:
`0x7B432017356401A3`

You can get a sharding key from the 1090MHz website at https://1090mhz.uk or
my emailing info@1090mhz.uk or by messaging me (Mike Tubby) on Discord.



## Contributing

You are welcome to suggest improvements and raise issues through Github.


## Legal stuff

### Copyright

Radar is Copyright (C) 2023 by Michael J. Tubby B.Sc. MIET G8TIC but is free
for you to use, modify, update and change subject to the GPL license (below).

### Acknowledgements

Portions of this project (sha256, hmac-sha256) use open source code
contributed by Apple Inc. and which are duely acknowleged.

### License

This code is Open Source and is Licensed under the GNU Public License (GPL)
Version 2.0, or at your option, any later version of the GPL.

https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

### NO WARRANTY

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND WITHOUT WARRANTY OF ANY KIND.

