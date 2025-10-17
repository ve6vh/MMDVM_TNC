/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023,2024 by Jonathan Naylor G4KLX
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

#if !defined(SERIALPORT_H)
#define  SERIALPORT_H

#include <cmsis_os.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <config.h>
#include <stream_buffer.h>

#include "Config.h"
#include "Globals.h"

#include "stm32f4xx_hal_uart.h"

#if !defined(SERIAL_SPEED)
#define SERIAL_SPEED 115200
#endif

// port definitions
#define	SERIAL_PORT		0
#define	DEBUG_PORT		1

// timeouts
#define RX_TIMEOUT      100     	 // 100ms second rx timeout
#define	TX_TIMEOUT		100			 // 100ms transmit timeout

const uint16_t BUFFER_SIZE = 2048U; //needs to be a power of 2 !
const uint16_t BUFFER_MASK = BUFFER_SIZE - 1;

class CSerialPort {
public:
  CSerialPort();

  void start();

  void process();

  void writeKISSData(uint8_t type, const uint8_t* data, uint16_t length);
  void writeKISSAck(uint16_t token);

  void writeDebug(const char* text);
  void writeDebug(const char* text, int16_t n1);
  void writeDebug(const char* text, int16_t n1, int16_t n2);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4, int16_t n5);

private:
  uint8_t  m_buffer[2000U];
  uint16_t m_ptr;
  bool     m_inFrame;
  bool     m_isEscaped;

  void processMessage();

#if defined(SERIAL_DEBUGGING)
  void writeDebugInt(const char* text);
  void writeDebugInt(int16_t num);
  void reverse(char* buffer, uint8_t length) const;
#endif

  // Hardware versions
  void    beginInt(uint8_t n, int speed);
  int     availableForReadInt(uint8_t n);
  int     availableForWriteInt(uint8_t n);
  uint8_t readInt(uint8_t n);
  void    writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush = false);
};

/*
 * FIFO Class used in old UART code
 */

class CSTMUARTFIFO {
public:
  CSTMUARTFIFO() :
  m_head(0U),
  m_tail(0U)
  {
  }

  void put(uint8_t data)
  {
    m_buffer[BUFFER_MASK & (m_head++)] = data;
  }

  uint8_t get()
  {
    return m_buffer[BUFFER_MASK & (m_tail++)];
  }

  void reset()
  {
    m_tail = 0U;
    m_head = 0U;
  }

  bool isEmpty()
  {
    return m_tail == m_head;
  }

  bool isFull()
  {
    return ((m_head + 1U) & BUFFER_MASK) == (m_tail & BUFFER_MASK);
  }

private:
  volatile uint8_t  m_buffer[BUFFER_SIZE];
  volatile uint16_t m_head;
  volatile uint16_t m_tail;
};

// return codes
typedef enum write_ret_e	{
	UART_WRITE_OK=0,
	UART_DMA_ERROR,
	UART_TIMEOUT
} UartWriteStatus;

typedef enum read_ret_e	{
	UART_READ_OK=0,
	UART_READ_ERROR,
	UART_READ_TIMEOUT
} UartReadStatus;


// UAR/T HAL driven code

#ifdef __cplusplus
extern "C"	{

extern SemaphoreHandle_t 		txCompleted;
extern StreamBufferHandle_t		USART_RxBuffer;			// handle to buffer
extern StaticStreamBuffer_t 	USART_StreamBuffer;
extern char 					USARTRxChar[10];
extern char						usart_data[BUFFER_SIZE];
//
void initTxSemaphore(void);
uint8_t txSemaphoreWait(TickType_t ticks);

void streamBufferInit(void);
UartReadStatus startRx(void);
void rxReset(void);
}
#endif

class CHALUART	{
public:
	CHALUART();
	~CHALUART();

	void 			   	Init(int speed);
	UartWriteStatus  	write(const uint8_t * data, uint16_t length);
	uint8_t				read(int timeout);
	void 				flush(void);
	uint16_t 			available(void);
	uint16_t availableForWrite()
	{
		 return BUFFER_SIZE;
	}
};

#endif
