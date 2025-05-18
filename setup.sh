#!/bin/bash
#
# setup.sh -- Setup for 1090MHz UK ADS-B Aggrgator
# Copyright (C) 2023 by Michael J Tubby B.Sc. MIET mike@tubby.org
#
# This script attemtps to determine settings a configure the 'radar'
# ADS-B sofwtare for your environment.
#

#
# name of default file
#
DEFAULT=/etc/default/radar

#
# Check we're running as root
#
if [ "$EUID" -ne 0 ]
	then echo "Please run setup as root, try 'sudo bash' for a root shell"
	exit
fi

#
# Determine init system
#
INIT=`ps --no-headers -o comm 1`


#
# Check if we have 'netstat' available - we need it to probe protocol support
#
NETSTAT=`which netstat`
if [[ $NETSTAT == "" ]]; then
	echo "The 'netstat' program doesn't appear to be installed on your system"
	echo "We need 'netstat' to probe for protocol support - please install with:"
	echo ""
	echo "   apt-get install net-tools"
	echo ""
	echo "or similar for your OS and/or package manager."
	echo ""
	exit
fi


#
# Determine which dump1090 protocols are supported
#
BEAST=`netstat -pan | grep LISTEN | grep -Eo '30005' | uniq`

# we no longer support AVR since it doesn't give us RSSI and MLAT
#AVR=`netstat -pan | grep LISTEN | grep -Eo '30002' | uniq`



#
# Ask user for their info ...
#
echo ""
echo "Setup for 1090MHz UK ADS-B Aggregator"
echo "====================================="
echo ""
echo "This script sets up the 'radar' ADS-B forwarder send traffic to the"
echo "1090MHz UK ADS-B aggregator."
echo ""
echo "Additional information at: https://1090mhz.uk/sharing.html"
echo ""
echo ""
echo "Step 1: Determine BEAST protocol support"
echo "----------------------------------------"
echo ""

if [[ $BEAST == "30005" ]]; then
	echo "We found BEAST protocol on this system ;-)"
else
	echo "Cannot find BEAST protocol on this system - is dump1090 or readsb running?"
	echo ""
	echo "If you normally run dump1090/readsb on this machine say No here, re-start"
	echo "dump1090/readsb and then run setup again."
	echo ""
	echo "If you want to use a feed from another machine on your local network say"
	echo "Yes here and provide the IP address of the remote machine."
	echo ""
	
	while true; do
		read -p "Do you want to to configure a remote feed? (Y/N) : " yn
		case $yn in
			[Yy]* ) break;;
			[Nn]* ) exit;;
			* ) echo "Please answer 'Y' for yes or 'N' for no.";;
		esac
	done

	read -p "Enter the IP address remote system with ADS-B service? : " REMOTE
fi


echo ""
echo ""
echo "Step 2: Setup the API/sharing key"
echo "---------------------------------"
echo ""
echo "To forward ADS-B messages to the 1090MHz UK aggregator you need an API key or"
echo "\"sharing key\" that is unique to your installation."
echo ""
echo "API keys are 64-bit hex numbers like:"
echo ""
echo "       0x78001234E301A27F"
echo ""
echo "If you don't alreay have an API key you can get one by sending an email to info@1090mhz.uk"
echo "and ask for a Radar sharing key or find me, Mike Tubby, on Discord."
echo ""

while :; do
	echo -n "Please enter your API key: "
	read KEY
	if [[ $KEY =~ ^0x[0-9a-fA-F]{16}$ ]]; then
		break;
	else
	        echo ""
        	echo "Key looks incorrect ... try again?"
	        echo ""
	fi
done

echo ""
echo ""
echo "Step 3: Signing pass-phrase (optional)"
echo "--------------------------------------"
echo ""
echo "Radar v2 protocol uses digital signatures on messages to the central aggregator"
echo "to protect data in transit from corruption, alteration, forgery and spoofing."
echo ""
echo "You may have been provided with a pass-phrase/secret for use with the system"
echo "in which case please enter it here.  If you don't have a PSK then leave this"
echo "blank (just press enter) and we will use the default value."
echo ""

read -p "Please enter your pass-phrase or just hit [RETURN] : " PASSPHRASE

if [[ ${#PASSPHRASE} -lt 1 ]]; then
	PASSPHRASE="secret"
fi


#
# tell the user which init system we found
#
echo ""
echo ""
echo "Step 4: Identify init system"
echo "----------------------------"
echo ""
echo "This system uses \"$INIT\" as it's init system..."
echo ""


#
# build options
#
OPTIONS="-k $KEY"

#
# add signing key
#
OPTIONS="${OPTIONS} -p ${PASSPHRASE}"

#
# add remote ADS-B host if specified
#
if [ -z ${REMOTE+x} ]; then
	:
else
	OPTIONS="${OPTIONS} -r ${REMOTE}"
fi


echo ""
echo ""
echo "Step 5: Install configuration and start-up scripts"
echo "--------------------------------------------------"
echo ""

#
# sort out start-up script and options for systemd or sysv-init
#
if [[ $INIT == "systemd" ]]; then
	echo "Configuring for Systemd ..."

	# make /etc/default/radar *without* the daemon option as systemd does this!
	cp radar.default $DEFAULT

	echo "OPTIONS=\"${OPTIONS}\"" >> $DEFAULT

        echo "Stop previous instance of radar forwarder..."
        systemctl stop radar

        echo "Copy systemd unit file ..."
	cp radar.unit /etc/systemd/system/radar.service

	echo "Enabling radar service on boot ..."
	systemctl enable radar

	echo "Starting radar service ..."
	systemctl start radar
fi

if [[ $INIT == "init" ]]; then
	echo "Configuring for traditional Sysv init ..."
	
	# make /etc/default/radar *with* the daemon option
	cp radar.default $DEFAULT
	
	echo "OPTIONS=\"${OPTIONS} -d\"" >> $DEFAULT

	cp radar.init /etc/init.d/radar
	chmod 755 /etc/init.d/radar

	echo "Enabling radar service on boot ..."
	/usr/sbin/update-rc.d radar defaults
	service radar start
fi

echo ""
echo "Finished."
echo ""

