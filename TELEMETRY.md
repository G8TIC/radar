# System telemetry

In addition to the ADS-B traffic we also send some telemetry about the operation of the
platform and performance of our software periodically.

We do this to understand how our software is performing in order to detect bugs or unwanted
behaviour that we need to fix.


## What we send

We send the following information:

### Time stamps

Start time and current time so we can work out how long the feeder has been running.

### UTS name strings

The Posix UTS name strings as returned by the `uname` command - this
includes the Linux kernel version info, machine architecture, etc.

### Processor details

Processor architcture (ARM, Intel, etc) and number of CPUs.

### OS statistics

System uptime, number of processes, load averages, CPU temperature, memory
utilisation, swap utilisation.

### GCC version

The GCC compiler version information that built the feeder code.

### GLIBC version

The version number of the Glibc library that radar is running with.

### Data sizes

Sizes of various data types in the C language as these vary on different
architectures as this has caused problems on some platforms.

### Software version

Version number of the radar software.

### Performance metrics

Client protocol (BEAST or AVR), number of times we have connected, disconnected or
had a connection failed or socket fail from the dump1090 or readsb.

Counts of bytes read, good and bad frames and packets per second.


## What we don't send

We do not send any personally identifable information such as usernames,
passwords, IP addresses, network information, files or other data.

We do not send any information outside the UK or Europe.


## Disabling telemetry

By default telemetry is sent 10 seconds after start-up and then every 900
seconds (15 minutes).

You can disable telemetry by setting the telemetry interval to zero by adding `-t 0` to the configuration in /etc/default/radar

## More information

For detailed information on the telemetry and how it is collected the code
is in `telemetry.c` and `telemetry.h`.
