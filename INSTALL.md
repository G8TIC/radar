# Installation

This is the quick installation setup guide.

## Working assumptions
Some working assumptions:
1. You have a Raspberry Pi or Intel/AMD based Linux box
2. You have Linux OS based on Debian 10.x r later, Debian, Raspberry Pi OS, Ubuntu, Devuan, etc..
3. You have GCC, make and git installed
If you don't have a build envionment then:
```
    apt-get install git build-essential
```
shoudl get what you need.

## Station information
Email me at info@1090mhz.uk with some basic information:
1. A name for your station: 3-12 characters (uppercase letters, digits and hyphen) - typically you locality name
2. The GPS Latitude and Longitude of your antenna to at least five decimal places
3. The altitude of your antenna (in metres)

## Sharing key
I will send you a sharing key - a 64-bit number in hexadecimal, for example: 0x7F43A1B7DEF44C7A

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

That's it ;-)
