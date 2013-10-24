seriallog
=========

seriallog reads data from a serial device and logs to stdout.  I use this to capture output from a benchtop multi-meter but it works fine for just any simple ASCII stream on the serial device.

Sample output:

```
20130928140921 +0.00E-3
20130928140922 +0.00E-3
20130928140924 +5.020E+0
20130928140925 +5.009E+0
20130928140926 +5.006E+0
```

##Building and installing

There are no dependencies (beyond a working C compiler), you should just be able to compile with something like this:

```
# cc seriallog.c -o seriallog
```

Then copy the binary to your favorite 'bin' directory.

##Running

Output from invoking the executable with no arguments:

```
USAGE: seriallog [-dt] [-b <baud>] [-D <serialdevice>]

-b <baud>           baud rate on serial device
                    default=9600
-d                  enable debug output
-D <serialdevice>   serial device connected to the tagger
                    default=/dev/tty.usbserial
-t                  print time stamp at start of each line

Reads data from the serial port and prints on stdout.
```
