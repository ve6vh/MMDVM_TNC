# kissgen - a kiss message generator

This code runs on any linux machine, pc or Pi, to generate kiss messages for test purposes. Invoking the program with no
arguments on the command line will print a help message.

## Building
Copy all the files to a linux machine. Build the code with 'make all'.

## Execution
The general form on the command line is:
kissgen [-bmdakt] -D <val1> <val2>

Where:
-b is the baud rate, default 115200
-m is the serial word format, default is 8N1
-d is the device, default is /dev/ttyAMA0
-a is the kiss address, 0-15
-k is the kiss message type
-t is a test mode which generates a new message type (07) for test purposes.

The message types are:-
0. Sends a canned data message "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ**Test Message**"
1-6. Sends the associate kiss messages as defined in the protocol. 1-5 required a single argument, 6 requires 2.
7. Sends a single character, useful for serial debugging.

The -D switch is a debug mode that dumps the kiss message to stdout in hex format for debugging.

The test modes are not defined in the protocol, and are implementation dependent.
In this verision of MMDVM_TNC, a continuous test message is sent out consisting of the symbols -3, +1, -1, +3.
