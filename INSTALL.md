# Installation
This is the quick installation setup guide.

## Working assumptions
Some working assumptions:
1. You have a Raspberry Pi or Intel/AMD based Linux box
2. You have Linux OS based on Debian 10.x or later, Debian, Raspberry Pi OS, Ubuntu, Devuan, etc..
3. You have a root access to your machine
4. You have build envionment (GCC, make and git) installed

### Root access
You need access to your system with either a screen and keyboard or SSH over
the network and the 'sudo' command or a root login.

### Build envionment
If you don't have the C compiler and tools installed then use the command:
```
    apt-get install git build-essential
```
to install them.

## Station information
Drop me an email at info@1090mhz.uk with some basic information about your station:
1. A station name: 3-12 characters (uppercase letters, digits and hyphen) - typically you locality name
2. The GPS Latitude and Longitude of your antenna to at least five decimal places
3. The altitude of your antenna (in metres) above ground

## Sharing key
I will send you a sharing key - a 64-bit number in hexadecimal, for example:
```
    0x7F43A1B7DEF44C7A
```

If you have more than one receiver then you need a separate key for each receiver.

## Download and install
Download and install the code as follows:
```
    git clone https://github.com/G8TIC/radar
    cd radar
    make
    sudo make install
    sudo make setup
```
and enter the sharing key when requested.

The setup phase will create /etc/default/radar and start the service.


That's it ;-)
