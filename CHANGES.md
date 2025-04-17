# Radar :: 1090MHz UK ADS-B forwarder

This CHANGES document provides details on changes to released versions of
the code.


## Change Log


### Version 2.05-0 15th April 2025

Re-wrote the UDP sending code in it's own module because the previous code
was brittle and did not recover from errors such as removing the internet
connection or routers being rebooted.

New module has it's own finite state machine and will recover after errors.


### Version 2.05-1 16th April 2025

Internal tidy-up only, not released.


### Version 2.05-2 17th April 2025

Discovered that UDP can still fail with CGNAT or double NAT where an ICMP
error message doesn't get generated or doesn't get back to user-space still
leaving the UDP sender stranded.

Added a SIGHUP handler and udp_reset() so that the CLI or a cron job can
reset the UDP periodically.
