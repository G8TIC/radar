# Radar :: 1090MHz UK ADS-B forwarder
This CHANGES document provides details on changes to released versions of the code.

## Version 2.05-0 15th April 2025
Re-wrote the UDP sending code and put it in its own module udp.[c,h] because the previous
UDP code was brittle and did not recover from errors such as removing the internet
connection or routers being rebooted.

New module has it's own finite state machine and errors cause it to back off
for five seconds, create a new socket and start up again.

## Version 2.05-1 16th April 2025
Internal tidy-up only, not released.

## Version 2.05-2 17th April 2025
Discovered that UDP can still fail with 4G modems through CGNAT or systems with double NAT where an ICMP error message
doesn't get generated or doesn't get back to user-space still leaving the UDP sender stranded.

Added a SIGHUP handler and udp_reset() so that the CLI or a cron job can reset the UDP
state-machine and re-start the UDP sender periodically.
