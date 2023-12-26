# Statistics

In addition to the real-time traffic we also gather statistics about the
rest of the ADS-B on the radio channel that we do not forward. These stats
help us understand how busy the radio channel is and will allow us to develop
things like heat maps.


## What we send

We send the following information:

### Time stamps

Start time and current time - so we can work out how long the feeder has been running.

### Messges received

Counts for Mode-A/C, Mode-S (short) and Mode-S (extended) messages received off-air.

### Downlink Formats received

Counts for each of the 32 ADS-B downlink format (DF) message types.

### Duplicates

Counts for the number of duplicate messages received for Mode-A/C, Mode-S (short) and Mode-S (extended)
messages.

### Messages sent

Counts for each type of message sent to the aggregator.

### Total traffic

Total traffic counts in messages and bytes.


## Disabling statistics

By default statistics are sent every 900 seconds (15 minutes).

You can disable stats by setting the interval to zero by adding `-s 0` to the configuration in /etc/default/radar


## More information

For detailed information on the stats  and how it is collected the code is in `stats.c` and `stats.h`.
