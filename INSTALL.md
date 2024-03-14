# Installation
This is the quick installation setup guide for 'radar' - the ADS-B feeder software for 1090MHz UK.

## Working assumptions
Some working assumptions:
1. You have a Raspberry Pi or Intel/AMD based Linux box
2. You have a Linux OS based on Debian 10.x or later
3. You have a root access to your machine
4. Your system has a supported ADS-B receiver and radio sub-system
5. You have build envionment (GCC, make and git) installed

## Computer system

### Hardware
You have Raspberry Pi 3/4/5 or an Intel/AMD based system - this can be a 32-bit or 64-bit system based on ARM, Intel or AMD CPUs.

Processors can be anything from an NXP iMX6 upwards including Raspberry Pi 3/3/5, small industrial systems based on AMD Geode,
Intel Atom/Celeron/Pentium or more should work fine.

I have feeders running on re-purposed Sophos XG85 firewall boxes based on Atom 3000, custom NXP iMX8 systems, Intel Core i5 desktops and other devices like Intel NUC.

### Operating system
Your hardware is running a Linux OS based on Debian 10.x or Debian derived OS like Raspbian, Raspberry Pi OS, Ubuntu, Arch, Devuan, etc.

### Root access
You need access to your system with either a screen and keyboard or SSH over the network and the 'su' or 'sudo' command or a root login in order to install the code.

### System time
The feeder protocol uses precision timestamps and if your computer's clock drifts by more than a second then your packets won't be processed.

Use systemd's 'timedatectl' or install Network Time Protocol (NTP) to keep your clock accurate.

## ADS-B receiver
You have either an RTL-SDR dongle, an other supported SDR receiver such as AirSpy Mini, or a hardware/dedicated receiver such as a Mode-S Beast or
GNS 5984T connected via USB with the appropriate software.

Your system has the Beast protocol accessible on localhost TCP port 30005 which is typically provided by dump1090-mutability,
dump1090-fa or readsb or a serial port connection.

Some hardware has dedicated software like 'airspy_adsb' for the Airspy Mini.

The Mode-S Beast by DL4MEA has a serial over USB interface and can be directly connected if you don't need dump1090/readsb to act as a multiplexer for feeding other systems.

### Remote receiver
It is possible to run the feeder on one machine and connect to the Beast protocol on a different computer on your LAN using the "-r" CLI option.

By default radar assumes the Beast protocol is on the same box and reachable via IPv4 localhost (127.0.0.1).

## Station Identification (Sharing Key)
You need a Sharing Key to identify your receiver. A Sharing Key is a 64-bit number in hexadecimal that uniquely identifies your receiver, for example:
```
    0x7F43A1B7DEF44C7A
```
If you have more than one receiver then you need a separate key for each receiver.

Drop me an email at info@1090mhz.uk with some basic information about your station:
1. A station name: 3-12 characters (uppercase letters, digits and hyphen) - typically you locality name
2. The GPS Latitude and Longitude of your antenna to at least five decimal places
3. The altitude of your antenna (in metres) above ground
and I will configure your receiver and send you a Sharing Key.

### Pass-phrase
Version 2.xx of the feeder protocol uses digital signatures (authentication tags) on UDP messages sent
to the aggregator to guard against message corruption, forgery and replay attacks.

I will provide you a 16-character random string which is the pass-phase used to generate the cryptographic
key that in turn is used to digitally sign the messages.


## Install 'RADAR' feeder software

### Build environment
If you don't have the C compiler and tools installed then use the command:
```
    apt-get install git build-essential
```
to install them.

### Download the source code and install
As an ordinary user (not root) download the 'radar' source code from Github and install as follows:
```
    git clone https://github.com/G8TIC/radar
    cd radar
    make
    sudo make install
    sudo make setup
```
and enter the sharing key and pass-phrase when requested.

The setup script will create /etc/default/radar, work out which init system you have (systemd ot sysv-init)
and install the appropriate start-up script, and start the service.


That's it ;-)
