/*
 * HALUart.cpp
 *
 *  Created on: Aug 14, 2025
 *      Author: MartinA
 */

#include <Config.h>
#include "Globals.h"
#include "SerialPort.h"

extern UART_HandleTypeDef		HUART;

SemaphoreHandle_t 			txCompleted;
StreamBufferHandle_t		USART_RxBuffer;			// handle to buffer
StaticStreamBuffer_t 		USART_StreamBuffer;
char 						USARTRxChar[10];
char						usart_data[BUFFER_SIZE];
//
const char *StartMessage = "\r\nMMDVM TNC V0.1 Oct, 2025\r\n\n";

// constructor: in case
CHALUART::CHALUART()
{
	return;
}

// destructor: likewise
CHALUART::~CHALUART()
{
	return;
}

// initialize
void CHALUART::Init(int speed)
{

	// create the tx completed semaphore
	initTxSemaphore();

	// create the stream buffer
	streamBufferInit();

#if __BOARD_TYPE == ARDUINO_F44
    // send a startup message
    write((const uint8_t *)StartMessage, strlen(StartMessage));
#endif

    // start USART
	rxReset();
    startRx();
}

// write data
UartWriteStatus CHALUART::write(const uint8_t *string, uint16_t len)
{
	// send using DMA
	uint16_t dataLen = (uint16_t)len;
	if((HAL_UART_Transmit_DMA(&HUART, string, dataLen)) != HAL_OK)
		return UART_DMA_ERROR;

	// wait for completion..
	if((txSemaphoreWait(pdMS_TO_TICKS(TX_TIMEOUT))) == pdTRUE)
			return UART_TIMEOUT;

	return UART_WRITE_OK;
}

// read data
uint8_t CHALUART::read(int timeout)
{
	char retval;

	TickType_t tickTimeout;

	if(timeout == 0)
		tickTimeout = portMAX_DELAY;
	else
		tickTimeout = pdMS_TO_TICKS(timeout);

	if(xStreamBufferReceive(USART_RxBuffer, &retval, 1, tickTimeout) == 0)
		return 0;

	return retval;
}

uint16_t CHALUART::available(void)
{
	size_t avail = xStreamBufferBytesAvailable(USART_RxBuffer);
	return avail;
}

// flush the buffer
void CHALUART::flush()
{
	return;
}

/*
 * These are written in C for HAL compatibility
 *
 * Receiver functions
 * Start an interrupt driven receive
 * Put a receive char in the stream buffer
 */
void streamBufferInit(void)
{
	USART_RxBuffer = xStreamBufferCreateStatic(BUFFER_SIZE, 1, (uint8_t *)usart_data, &USART_StreamBuffer);
}

UartReadStatus startRx(void)
{
	HAL_StatusTypeDef rxStat;
	if((rxStat = HAL_UART_Receive_IT(&HUART,(uint8_t *)USARTRxChar,1)) != HAL_OK)
		return UART_READ_ERROR;

	return UART_READ_OK;

}

void rxReset(void)
{
	xStreamBufferReset(USART_RxBuffer);
}

/*
 * Transmitter functions
 */
void initTxSemaphore(void)
{
    txCompleted = xSemaphoreCreateBinary();
}

uint8_t txSemaphoreWait(TickType_t ticks)
{
	return xSemaphoreTake(txCompleted, ticks);
}

// Transmit completed: trigger semaphore
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(txCompleted, &xHigherPriorityTaskWoken);
}

// callback function when rx is completed
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// put the receive char in the streambuffer
	xStreamBufferSendFromISR(USART_RxBuffer, USARTRxChar, 1, NULL);

	startRx();
}





