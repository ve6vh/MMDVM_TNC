/*---------------------------------------------------------------------------
	Project:	      TNC simulator

	File Name:	      TNC.h

	Date Created:	  Oct 14, 2025

	Author:			  MartinA

	Description:      <what it does>

					  This program is free software: you can redistribute it and/or modify
					  it under the terms of the GNU General Public License as published by
					  the Free Software Foundation, either version 2 of the License, or
					  (at your option) any later version, provided this copyright notice
					  is included.

	SVN Revision: 	  $Revision: 9584 $
					  $Date$

---------------------------------------------------------------------------*/
#ifndef INC_TNC_H_
#define INC_TNC_H_

// KISS definitions
#define	FESC		0xDB				// frame escape
#define	FEND		0xC0				// frame end
#define	TFEND		0xDC				// transposed frame end
#define	TFESC		0xDD				// transposed frame escape

#define	KISS_CMD(addr,cmd)	((addr<<4) | cmd)

#define	DEF_ADDRESS	0					// default kiss address
#define UNDEFINED	-1					// undefined type

// commands from mainline
#define ID_TNCMESSAGE_SENDDATAMESSAGE		0
#define ID_TNCMESSAGE_SETTXDELAY			1
#define ID_TNCMESSAGE_SETPERSISTENCE		2
#define ID_TNCMESSAGE_SETSLOTTIME			3
#define ID_TNCMESSAGE_SETTXTAILTIMER		4
#define ID_SETDUPLEX						5
#define ID_TNCMESSAGE_SETHARDWARETYPE		6
#define ID_TNCMESSAGE_SENDSINGLECHARACTER	7
#define ID_TXTESTTONE_ON					8
#define ID_TXTESTTONE_OFF					9

#define	MAX_DATA_FRAME						400					// max data frame size

// KISS commands
#define	SEND_DATA_FRAME	0x00			// send a data frame
#define	TX_DELAY		0x01			// set tx delay
#define	PERSISTENCE		0x02			// set persistence
#define SLOT_TIME		0x03			// set slot time
#define TX_TAIL			0x04			// transmit tail
#define	SET_DUPLEX		0x05			// set duplex
#define SET_HARDWARE	0x06			// device dependent hardware
#define	SET_TEST_MODE	0x07			// set test mode
#define	RETURN			0xFF			// exit KISS mode

// duplex modes
#define	HALF_DUPLEX		0				// half duplex
#define	FULL_DUPLEX		1				// full duplex

// test tone
#define	TEST_TONE_OFF	0				// tone off
#define TEST_TONE_ON	1				// test tone on

// TNC parameters
typedef struct tnc_params_t {
	uint8_t		Address;				// Kiss Address
	uint16_t	Tx_Delay; 				// transmit delay
	uint16_t	Tx_Tail;				// transmit tail
	uint8_t		Tx_Gain;				// transmit gain
	uint8_t		Rx_Gain;				// receive gain
	uint8_t		Persistence;			// CSMA Persistence
	uint8_t		SlotTime;				// CSMA Slot time
	uint8_t		Duplex;					// Duplex
} TNC_PARAMS;

// links in
void sendTNCDataMessage(int wmId);
TNC_PARAMS *getTNCParams(void);

// others
void writeSerialPort(char *TNCBuffer, int bufLen);

#endif
