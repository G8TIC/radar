# Radar :: 1090MHz UK ADS-B forwarder

This is the ADS-B "radar" forwarder for the 1090MHZ UK ADS-B aircraft tracking network.

Radar allows a wide range of ADS-B receivers based on SDR dongles to receive 1090MHz ADS-B and
forward interesting traffic to the 1090MHz UK aggregator using an optimised, efficient and secure
real-time UDP/IP protocol.

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
when used with dump1090, dump1090-mutability, dump1090-fa or readsb. That provide BEAST protocol on TCP port
30005 (preferred) or AVR protocol on TCP port 30002 (fallback).

Your ADS-B receiver/dump1090/readsb can be on the same machine as radar or
can be remote using the `-r [remote ip]` option.

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

Configure your system (see below for info needed):

```
sudo make setup
```


## Sharing keys and station info

Each receiver station is identified by an API key or 'sharing key' which is a 64-bit hex number like:

```
0x7B432017356401A3
````

Along with the sharing key we need a station name (3-12 characters) and the GPS location of your receiver's
antenna in order to perform range checks and MLAT.

### Station name

Your station name will be visible on the website stats page and is 3-12
characters long comprising uppercase letters, digits and underscore ("_") or
hyphen ("-").

Station names are commonly a locality name like FERNHILLL, HEATHROW, READING
or a nickname like ELMARKO or BIGCAT.


### Antenna location

In order to perform range checks and MLAT calculations we need the location
of your receiver antenna as GPS latitude and longitude to five decimal
places.  We would also like our antenna height above ground in metres.


### Obtain a Sharing Key

You can get a sharing key from the 1090MHz website at https://1090mhz.uk or
by emailing info@1090mhz.uk or by messaging me (Mike Tubby) on Discord.


### Secure pass-phrases

The UDP/IP forwarding protocol digitally signs each packet using a truncated
HMAC-SHA256 which protects against message corruption, forgery and replay
attacks.

By default the messages are signed with the pass-phrase 'secret' - an
additional configuration step is requred at both the sender and aggregator
to set up a custom pass-phrase.



## Configuration

### Default configuration

The radar forwarder stores its configuration in:

```
	/etc/default/radar
```

where the minimum is your sharing key provided by the `-k <sharing key>`
option.

Other options are provided below.


### Command line options

The radar code has several command line options:

```
  -k <key>           : sharing key (identity) of this receiver station
  -h <hostname>      : hostname of central aggregator
  -p <psk>           : pre-shared key for HMAC authwentication (signing of messages)
  -c                 : enable sending Mode-A/C message (not recommended)
  -y                 : enable sending Mode-S Short messages (not recommended)
  -e <level>         : control which Mode-S Extended Squitter DF codes are sent (default = 1)
  -l <ip addr>       : IP address of local dump1090/readsb server (default: 127.0.0.1)
  -s <seconds>       : Set the radio stats interval (default 900)
  -t <seconds>       : Set the telemetry interval (default 900)
  -d                 : run as daemon (detach from controlling tty)
  -f                 : print forwarding stats once per second
  -u <uid|username>  : set the UID or username for the process
  -g <gid|group>     : set the GID or group name for the process
  -q <qos>           : set the DSCP/IP ToS quality of service
  -n <number>        : number of DNS lookup attempts (default: infinite)
  -v                 : display version information and exit
  -x|xx|xxx          : set debug level
  -?                 : help (this output)
```

however most have sensible defaults and are therefore not required in operation.


## Contributing

You are welcome to suggest improvements and raise issues or bugs through Github.


## Legal stuff

### Copyright

Radar is Copyright (C) 2023 by Michael J. Tubby B.Sc. MIET G8TIC but is open source and
free for you to use, modify, update and change as you see fit, subject to the
conditions of the GPL license (below).

### Acknowledgements

Portions of this project (sha256, hmac-sha256) use open source code contributed by Apple Inc. and which are duly acknowleged.

### License

This project is licensed under the GNU Public License (GPL) Version 2.0, or at your option, any newer version of the GPL.

The GNU Public License is here: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

### NO WARRANTY

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
CONTRIBTUORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

