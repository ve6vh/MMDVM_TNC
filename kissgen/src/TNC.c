/*---------------------------------------------------------------------------
	Project:	      DAQ imlementation for Allstar

	Module:		      TNC serial messages

	File Name:	      TNC.c

	Author:		      Martin C. Alcock, VE6VH

	Revision:	      1.05

	Description:      Header file for project

					This program is free software: you can redistribute it and/or modify
					it under the terms of the GNU General Public License as published by
					the Free Software Foundation, either version 2 of the License, or
					(at your option) any later version, provided this copyright notice
					is included.

					Copyright (c) 2018-2022 Praebius Communications Inc.

	Revision History:

---------------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "TNC.h"

char *Test_Message = {
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ**Test Message**"
};

TNC_PARAMS TncParams = {
	DEF_ADDRESS,	// Address
	300,			// transmit delay
	50,				// transmit tail
	128,			// transmit gain
	128,			// receive gain
	63,				// CSMA Persistence
	0,				// CSMA Slot time
	HALF_DUPLEX		// Duplex
};

char TNCBuffer[200];

// fwd refs
void sendDataMessage(void);
void sendSingleByteMessage(int msgType);
void sendHWParamsMessage(void);
void sendTestMode(int msgType);

// message dispatcher
void sendTNCDataMessage(int wmId)
{
	switch (wmId) {

	case ID_TNCMESSAGE_SENDDATAMESSAGE:
		sendDataMessage();
		return;

	// send parameters
	case ID_TNCMESSAGE_SETTXDELAY:
	case ID_TNCMESSAGE_SETPERSISTENCE:
	case ID_TNCMESSAGE_SETSLOTTIME:
	case ID_TNCMESSAGE_SETTXTAILTIMER:
	case ID_SETDUPLEX:
		sendSingleByteMessage(wmId);
		return;

	// set hardware (for MMDVM-TNC)
	case ID_TNCMESSAGE_SETHARDWARETYPE:
		sendHWParamsMessage();
		return;

	// test modes
	case ID_TNCMESSAGE_SENDSINGLECHARACTER:
	case ID_TXTESTTONE_ON:
	case ID_TXTESTTONE_OFF:
		sendTestMode(wmId);
		return;
	}

	return;
}

// send a data message (00)
void sendDataMessage(void)
{
	char *tBuf = TNCBuffer;

	*tBuf++ = FEND;
	*tBuf++ = KISS_CMD(TncParams.Address, SEND_DATA_FRAME);

	strcpy(tBuf, Test_Message);
	tBuf += strlen(Test_Message);

	*tBuf++ = FEND;
	size_t bufLen = tBuf - TNCBuffer;
	writeSerialPort(TNCBuffer, bufLen);
	
}

// send a single byte message (01-05)
void sendSingleByteMessage(int msgType)
{
	char *tBuf = TNCBuffer;

	*tBuf++ = FEND;

	switch (msgType) {

	case ID_TNCMESSAGE_SETTXDELAY:
		*tBuf++ = KISS_CMD(TncParams.Address, TX_DELAY);
		*tBuf++ = TncParams.Tx_Delay & 0xff;
		break;

	case ID_TNCMESSAGE_SETPERSISTENCE:
		*tBuf++ = KISS_CMD(TncParams.Address, PERSISTENCE);
		*tBuf++ = TncParams.Persistence & 0xff;
		break;

	case ID_TNCMESSAGE_SETSLOTTIME:
		*tBuf++ = KISS_CMD(TncParams.Address, SLOT_TIME);
		*tBuf++ = TncParams.SlotTime & 0xff;
		break;

	case ID_TNCMESSAGE_SETTXTAILTIMER:
		*tBuf++ = KISS_CMD(TncParams.Address, TX_TAIL);
		*tBuf++ = TncParams.Tx_Tail & 0xff;
		break;

	case ID_SETDUPLEX:
		*tBuf++ = KISS_CMD(TncParams.Address, SET_DUPLEX);
		*tBuf++ = TncParams.Duplex & 0x1;
		break;

	default:		// uknown type
		return;
	}

	*tBuf++ = FEND;
	size_t bufLen = tBuf - TNCBuffer;
	writeSerialPort(TNCBuffer, bufLen);
}

// send a set slot time message: sets to zero
void sendHWParamsMessage(void)
{
	char *tBuf = TNCBuffer;

	*tBuf++ = FEND;
	*tBuf++ = KISS_CMD(TncParams.Address, SET_HARDWARE);

	*tBuf++ = TncParams.Rx_Gain;
	*tBuf++ = TncParams.Tx_Gain;			// mode 1 is not used
	*tBuf++ = TncParams.Tx_Gain;			// mode 2 is higher speed mode

	*tBuf++ = FEND;
	size_t bufLen = tBuf - TNCBuffer;
	writeSerialPort(TNCBuffer, bufLen);
}

// send single character: debug the uar/t
void sendTestMode(int msgType)
{
	char *tBuf = TNCBuffer;

	switch (msgType) {

	case ID_TNCMESSAGE_SENDSINGLECHARACTER:
		*tBuf++ = 'A';
		break;

	case ID_TXTESTTONE_ON:
	case ID_TXTESTTONE_OFF:
		*tBuf++ = FEND;
		*tBuf++ = KISS_CMD(TncParams.Address, SET_TEST_MODE);
		*tBuf++ = msgType == ID_TXTESTTONE_OFF ? TEST_TONE_OFF : TEST_TONE_ON;
		*tBuf++ = FEND;
	}

	size_t bufLen = tBuf - TNCBuffer;
	writeSerialPort(TNCBuffer, bufLen);
}

TNC_PARAMS *getTNCParams(void)
{
	return &TncParams;
}

