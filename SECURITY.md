# Security Position

The following statements describe the security posture for this project:

## Information Sent
The system sends only information received off-air from publically available sources,
statistics about the radio channel/packet counts and some telemetry about the device
it is running on.

Telemetry is described in [TELEMETRY.md](TELEMETRY.md) and statistics are
described in [STATS.md](STATS.md)


## Personally Identifable Information (PII)
The system collects no Personally Identifable Information (PII) from you or anyone else.

## No network information
While we collect a small amount of telemetry duirectly about the host running our software
this is in order to detect bugs, performance issues an security issues. We do not collect
any information about your internal network, computers, IP addresses, programs, data or
any other information.

## Encryption
All information we send is transmitted un-encrypted (in the clear) since it is collected from public
sources that are readily available (off-air) so its seems pointless to waste processing
time, electrons, energy and bandwidth encrypting something that is publically available.

## Authentication
We implement cryptographic authentication tags which are a type of digital signature on
each message transmitted in order to protect against message corruption,
forgery or replay attacks.

## Information you provide
Information that you provide about your receiving station includes:

a) a name which you choose and is 3-12 characters long and can be fictional/anonymous

b) the latitude/longitude of your antenna - needed for surface position reports and
MLAT but we don't display this to anyone

c) your contact details (name and email address) which are only accessible to the
system operators and only used in the event of a problem

## UK/EU privacy rights
No personal information is transferred anywhere as we do not use any form of
PII and hence no personal information is outside the UK or Europe.

You are free to delete this sofwtare and your station and details at any
time.

This system complies with UK and EU privacy regulations such as the GDPR.

## Transmit only
By using UDP/IP "transmit only" and having no packet receive processing the
data we send leaves your system and passes through out-bound NAT on your
firewall/router.

Nno inbound traffic: no UDP packets are sent by our server(s) to you and there is
no packet receive handling in the code.

## Monitor your traffic
You can monitor the traffic sent by this software using network tools such as
'tcpdump' or 'wireshark' and observing UDP/IP port 5997.

The document [PROTOCOL.md](PROTOCOL.md) provides a description of the wire protocol
and relevant information is in the header file radar.h
