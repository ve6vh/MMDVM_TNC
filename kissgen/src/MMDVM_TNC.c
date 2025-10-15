/*
 ============================================================================
 Name        : MMDVM_TNC.c
 Author      : VE6VH
 Version     :
 Copyright   : (C) 2025, ADRCS
 Description : Mainline for MMDVM TNC Frame generator
 	 	 	 : reuses serial code from stm32flash and
 	 	 	 : tnc frame generator from PC code
 ============================================================================
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "serial.h"
#include "port.h"
#include "TNC.h"

/* settings */
struct port_options port_opts = {
	.device			= "/dev/ttyAMA0",
	.baudRate		= SERIAL_BAUD_115200,
	.serial_mode	= "8N1",
	.bus_addr		= 0,
	.rx_frame_max	= MAX_DATA_FRAME,
	.tx_frame_max	= MAX_DATA_FRAME,
};

struct port_interface *port = NULL;

#define	MAX_VALS		5			// max values
int cmd_vals[MAX_VALS];				// command line values
int8_t debugMode = UNDEFINED;			// debug mode off

// fwd refs
void show_help(char *name);

int main(int argc, char *argv[]) {

	int c;
	TNC_PARAMS *tncParams = getTNCParams();
	int kissMsg = UNDEFINED;
	int testTone = UNDEFINED;

	while ((c = getopt(argc, argv, "b:m:Dd:a:k:t:")) != -1) {
		switch(c) {

			// bit rate
			case 'b':
				port_opts.baudRate = serial_get_baud(strtoul(optarg, NULL, 0));
				if (port_opts.baudRate == SERIAL_BAUD_INVALID) {
					serial_baud_t baudrate;
					fprintf(stderr,	"Invalid baud rate, valid options are:\n");
					for (baudrate = SERIAL_BAUD_1200; baudrate != SERIAL_BAUD_INVALID; ++baudrate)
						fprintf(stderr, " %d\n", serial_get_baud_int(baudrate));
					return 1;
				}
				break;

			// word format (8N1 by default)
			case 'm':
				if (strlen(optarg) != 3
					|| serial_get_bits(optarg) == SERIAL_BITS_INVALID
					|| serial_get_parity(optarg) == SERIAL_PARITY_INVALID
					|| serial_get_stopbit(optarg) == SERIAL_STOPBIT_INVALID) {
					fprintf(stderr, "Invalid serial mode\n");
					return 1;
				}
				port_opts.serial_mode = optarg;
				break;

			// device
			case 'd':
				port_opts.device = optarg;
				break;

			case 'D':
				debugMode++;
				break;

			// Kiss address
			case 'a':
				sscanf(optarg, "%hhd", &tncParams->Address);
				break;

			// message type
			case 'k':
				sscanf(optarg, "%d", &kissMsg);
				break;

			// test mode
			case 't':
				sscanf(optarg,"%d", &testTone);
				break;
		}
	}

	if((testTone == UNDEFINED) && (kissMsg == UNDEFINED))	{
		fprintf(stderr, "No command specified\n");
		show_help(argv[0]);
		return(-1);
	}

	int nVals = argc-optind;
	if(nVals > MAX_VALS)		{
		fprintf(stderr, "Too many values specified\n");
		show_help(argv[0]);
		return -1;
	}

	// get the command arguments
	for(int i=optind;i<argc;i++)	{
		sscanf(argv[i], "%d", &cmd_vals[i-optind]);
	}

	// open the serial port
	if(debugMode == UNDEFINED)	{
		if (port_open(&port_opts, &port) != PORT_ERR_OK) {
			fprintf(stderr, "Failed to open port: %s\n", port_opts.device);
			return -1;
		}
	} else {
		fprintf(stderr, "Running in Debug mode\n");
	}

	// do test tone first
	if(testTone != UNDEFINED)	{
		sendTNCDataMessage(testTone ? ID_TXTESTTONE_ON : ID_TXTESTTONE_OFF);
	} else {
		if(kissMsg != UNDEFINED)	{
			switch(kissMsg)	{

				// messages with no args
				case ID_TNCMESSAGE_SENDDATAMESSAGE:
				case ID_TNCMESSAGE_SENDSINGLECHARACTER:
					break;

				case ID_TNCMESSAGE_SETTXDELAY:
					if(nVals < 1)	{
						fprintf(stderr, "Delay value not specified\n");
						show_help(argv[0]);
					} else {
						tncParams->Tx_Delay = cmd_vals[0];
					}
					break;

				case ID_TNCMESSAGE_SETPERSISTENCE:
					if(nVals < 1)	{
						fprintf(stderr, "Persistence value not specified\n");
						show_help(argv[0]);
					} else {
						tncParams->Persistence = cmd_vals[0];
					}
					break;

				case ID_TNCMESSAGE_SETSLOTTIME:
					if(nVals < 1)	{
						fprintf(stderr, "Persistence value not specified\n");
						show_help(argv[0]);
					} else {
						tncParams->SlotTime = cmd_vals[0];
					}
					break;

				case ID_TNCMESSAGE_SETTXTAILTIMER:
					if(nVals < 1)	{
						fprintf(stderr, "Tail timer value not specified\n");
						show_help(argv[0]);
					} else {
						tncParams->Tx_Tail = cmd_vals[0];
					}
					break;

				case ID_SETDUPLEX:
					if(nVals < 1)	{
						fprintf(stderr, "Duplex value not specified\n");
						show_help(argv[0]);
					} else {
						tncParams->Duplex = cmd_vals[0];
					}
					break;

				case ID_TNCMESSAGE_SETHARDWARETYPE:
					if(nVals < 2)	{
						fprintf(stderr, "Two gain values required\n");
						show_help(argv[0]);
					} else {
						tncParams->Rx_Gain = cmd_vals[0];
						tncParams->Tx_Gain = cmd_vals[1];
					}
					break;
			}
		}
		sendTNCDataMessage(kissMsg);
	}
}

void writeSerialPort(char *TNCBuffer, int bufLen)
{
	if(debugMode != UNDEFINED)	{
		for(int i=0;i<bufLen;i++)
			fprintf(stdout, "%02X ", TNCBuffer[i]&0xff);
		fprintf(stdout, "\n");
		return;
	}
	port->write(port, TNCBuffer, bufLen);
}

void show_help(char *name)
{
	fprintf(stderr,
		"Usage: %s [-bmdakt] -D \n"
		"	-b rate		Baud rate (default 115200)\n"
		"	-a kiss address	(default 0)\n"
		"	-m mode		Serial port mode (default 8e1)\n"
		"	-d device 	tty device\n"
		"	-k Kiss message type: \n"
		"	   0			Send data message\n"
		"	   1	<n>		Set Tx Delay\n"
		"	   2	<n>		Set Persistence\n"
		"	   3	<n>		Set CSMA slot time\n"
		"	   4	<n>		Set TX Tail\n"
		"	   5	<n>		Set Duplex\n"
		"	   6	<r> <t>	Set rx and tx gain\n"
		"	-t 1/0	Set test tone on/off\n"
		"   -D debug mode\n"
		"\n"
		"Examples:\n"
		"	Send a data message:\n"
		"		%s -d /dev/ttyAMA0 -k 0\n"
		"\n"
		"	Set Tx Delay\n"
		"		%s -d /dev/ttyAMA0 -k 1 nn \n"
		"\n"
		"	Set test mode on\n"
		"		%s -d /dev/ttyAMA0 -t 1\n"
		"\n"
		"	Read 100 bytes of flash from 0x1000 to stdout:\n"
		"		%s -r - -S 0x1000:100 /dev/ttyS0\n",
		name,
		name,
		name,
		name,
		name
	);
}
