/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (c) 2017 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Re-written to use UART defined in IOC and standard HAL and OS calls
 */

#include "Config.h"
#include "Globals.h"

#include "SerialPort.h"

// 'real' device handles
// main uar/t
extern UART_HandleTypeDef HUART;
static CHALUART	m_UART_Main;			// main uart

#if SERIAL_DEBUGGING
extern UART_HandleTypeDef HDEBUG;
static CHALUART m_UART_Debug;			// debug, if attached
#endif

#if SERIAL_DEBUGGING
void CSerialPort::beginInt(uint8_t port, int speed)
{

	switch (n) {
		case SERIAL_PORT:
			m_UART_Main.Init(HUART, speed);
			break;

		case DEBUG_PORT:
			m_UART_Debug.Init(HDEBUG, speed);
			break;

		default:
			break;
	}
	// only one port used
	m_UART_Main.Init(HUART, speed);
}

int CSerialPort::availableForReadInt(uint8_t n)
{
   switch (n) {
      case SERIAL_PORT:
         return m_UART_Main.available();	//AvailUSART2();

      case DEBUG_PORT:
         return m_UART_Debug.available(); 	//AvailUSART1();

      default:
         return 0;
   }
}

int CSerialPort::availableForWriteInt(uint8_t n)
{
   switch (n) {
      case SERIAL_PORT:
         return m_UART_Main.availableForWrite();//AvailForWriteUSART2();

      case DEBUG_PORT:
         return m_UART_Debug.availableForWrite(); //AvailForWriteUSART1();

      default:
         return 0;
   }
}

uint8_t CSerialPort::readInt(uint8_t n)
{
   switch (n) {
      case SERIAL_PORT:
         return m_UART_Main.read();//ReadUSART2();

      case DEBUG_PORT:
         return m_UART_Debug.read();//ReadUSART1();

      default:
         return 0U;
   }
}

void CSerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
   switch (n) {
      case SERIAL_PORT:
         m_UART_Main.write(data, length);//WriteUSART2(data, length);
         if (flush)
            m_UART_Main.flush();//TXSerialFlush2();
         break;

      case DEBUG_PORT:
         m_UART_Debug.write(data, length); //WriteUSART1(data, length);
         if (flush)
            m_UART_Debug.flush();
         break;

      default:
         break;
   }
}
#else
void CSerialPort::beginInt(uint8_t port, int speed)
{
	// only one port used
	m_UART_Main.Init(speed);
}

int CSerialPort::availableForReadInt(uint8_t n)
{
    return m_UART_Main.available();				//AvailUSART2();

}

int CSerialPort::availableForWriteInt(uint8_t n)
{
    return m_UART_Main.availableForWrite();		//AvailForWriteUSART2();
}

uint8_t CSerialPort::readInt(uint8_t n)
{
    return m_UART_Main.read(RX_TIMEOUT);					//ReadUSART2();
}

void CSerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
    m_UART_Main.write(data, length);			//WriteUSART2(data, length);
}
#endif


