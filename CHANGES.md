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

## Version 2.06-0 20th April 2025
Internal testing only.

## Version 2.07-0 22nd April 2025
Implement multiframe support.  Multiframe gathers Mode-S Extended Squitter, MLAT and RSSI
and bufferes them sending up to 32 messages in a single UDP.
Multiframe messages are sent when either (a) the buffer is full or (b) the forwarding timeout expires (default 50mS).
Multiframe increases network efficiency (reduces bandwidth) and the expense of latency.
Where senders use multiframe they can appear lower in the system's ranking tables because the latency means that their messages arrive later
(behind another station with the same traffic) so get considered a duplicate more of the time.
Multiframe is considered experimental and defaults to off - to turn it on add "-m" to the OPTIONS in /etc/default/radar

Fixed SIGHUP handler to that UDP sending can be restarted on systems that have CGNAT and/or double NAT without
error feedback (eg. ICMP port unreachable etc.).

## Version 2.07-1 22nd April 2025
Handle EINTR (interrupted system call) correctly when SIGHUP received by the process.

## Version 2.07-2 22nd April 2025
Implement UDP rebind interval timer which closes and re-opens the UDP send socket with a new ephemeral port
number periodically to overcome limitations of double/treble (or more) NAT and/or CGNAT used on 4G/5G networks
that times out and leaves traffic going down a blackhole.
New option "-n [seconds]" allows this to be specified over range 0-3600 seconds. Suggested value 295 for networks with aggressive CGNAT at 5
minutes.
