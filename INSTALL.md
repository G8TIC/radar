# Installation
This is the quick installation setup guide for 'radar' - the ADS-B feeder software for 1090MHz UK.

## Working assumptions
Some working assumptions:
1. You have a Raspberry Pi or Intel/AMD based Linux box
2. You have a Linux OS based on Debian 10.x or later
3. You have a root access to your machine
4. Your system has an supported ADS-B receiver and radio sub-system
5. You have build envionment (GCC, make and git) installed

## Computer system

### Hardware
You have Raspberry Pi or Intel/AMD based Linux box - this can be a 32-bit or
64- bit system based on ARM, Intel or AMD CPUs.  Anything from an NXP iMX6
upwards including AMD Geode, Intel Atrom/Celeron/Pentium/more should work
fine.

### Operating system
Your harware is running a Linux OS based on Debian 10.x or Debian derived OS like Raspbian,
Raspberry Pi OS, Ubuntu, Arch, Devuan, etc.

### Root access
You need access to your system with either a screen and keyboard or SSH over
the network and the 'su' or 'sudo' command or a root login.

## ADS-B receiver
You have either an RTL-SDR dongle or other supported SDR receiver such as
AirSpy Mini, Mode-S Beast or GNS 5984T connected via USB adapter with the
appropriate software.

Your system has dump1090-mutability, dump1090-fa or readsb installed (or
dedicated sofwtare) and has the Beast protocol is accessible on localhost TCP port 30005.

## Station identification
Drop me an email at info@1090mhz.uk with some basic information about your station:
1. A station name: 3-12 characters (uppercase letters, digits and hyphen) - typically you locality name
2. The GPS Latitude and Longitude of your antenna to at least five decimal places
3. The altitude of your antenna (in metres) above ground

### Sharing key
I will send you a sharing key - a 64-bit number in hexadecimal, for example:
```
    0x7F43A1B7DEF44C7A
```
If you have more than one receiver then you need a separate key for each receiver.

### Pass-phrase
Version 2.0 of the feeder protocol uses digital signatures on the UDP
messages sent to the central aggregator.

I will provide you a 16-character random string which is the pass-phase to
generate the cryptographic key used to digitally sign the messages.


## Install 'RADAR' feeder software

### Build envionment
If you don't have the C compiler and tools installed then use the command:
```
    apt-get install git build-essential
```
to install them.

### Download the sofwtare and install
Download the 'radar' source code from Github and install as follows:
```
    git clone https://github.com/G8TIC/radar
    cd radar
    make
    sudo make install
    sudo make setup
```
and enter the sharing key and pass-phrase when requested.

The setup phase will create /etc/default/radar and start the service.


That's it ;-)
