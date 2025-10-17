/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (C) 2017,2018,2020,2023 by Jonathan Naylor G4KLX
 *   Copyright (C) 2019,2020 by BG5HHP
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

#include "Config.h"
#include "Globals.h"
#include "IO.h"
#include "main.h"

// capture the DAC output
#define	CAPTURE_DAC_OUTPUT		0				// set to one to capture DAC output
#define	CAPTURE_ADC_SAMPLES		0				// capture samples from the ADC

#if CAPTURE_DAC_OUTPUT || CAPTURE_ADC_SAMPLES
#define	DAC_CAPTURE_SIZE		8000
uint32_t  dacBuf[DAC_CAPTURE_SIZE];
uint32_t  *pdacBuf = dacBuf;
#endif

const uint32_t DC_OFFSET = 2048U;

// Sampling frequency
#define SAMP_FREQ   24000

// callback for ADC interrupt
extern "C" {

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
		uint32_t raw_Sample;
		uint16_t ADC_Sample, DAC_Sample;

		raw_Sample = HAL_ADC_GetValue(hadc);
		ADC_Sample = (uint16_t)(raw_Sample & 0xFFFF);
        DAC_Sample = io.interrupt(ADC_Sample);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL, DAC_ALIGN_12B_R, DAC_Sample);
        HAL_DAC_Start(&hdac, DAC_CHANNEL);
   }
}

void CIO::initInt()
{
   // PTT pin
   HAL_GPIO_WritePin(PTT_GPIO_Port, PTT_Pin, GPIO_PIN_RESET);

   // COSLED pin
   HAL_GPIO_WritePin(COSLED_GPIO_Port, COSLED_Pin, GPIO_PIN_RESET);

   // LED pin
   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

#if defined(MODE_LEDS)
   // Mode 1 pin
   HAL_GPIO_WritePin(MODE1_GPIO_Port, MODE1_Pin, GPIO_PIN_RESET);

   // Mode 2 pin
   HAL_GPIO_WritePin(MODE2_GPIO_Port, MODE2_Pin, GPIO_PIN_RESET);

   // Mode 3 pin
   HAL_GPIO_WritePin(MODE3_GPIO_Port, MODE3_Pin, GPIO_PIN_RESET);

   // Mode 4 pin
   HAL_GPIO_WritePin(MODE4_GPIO_Port, MODE4_Pin, GPIO_PIN_RESET);
#endif

   // Sampler Sync Pin
   HAL_GPIO_WritePin(SYNC_GPIO_Port, SYNC_Pin, GPIO_PIN_RESET);


}

/*
 * Start the A/D running in interrupt mode. The D/A can be serviced at the same time
 */
void CIO::startInt()
{
	// nothing to do here as it is now handled in main()
}

uint16_t CIO::interrupt(uint16_t ADC_Sample)
{

	// toggle sample sync pin
	HAL_GPIO_TogglePin(SYNC_GPIO_Port, SYNC_Pin);

	// toggle hearbeat count
	m_ledCount++;


	// capture A/D Samples when squelch is open
	if(getSquelch())		{
#if CAPTURE_ADC_SAMPLES
		*pdacBuf = ((uint32_t)ADC_Sample) & 0xfff;
		if((pdacBuf - dacBuf) < DAC_CAPTURE_SIZE)	{
			pdacBuf++;
		} else {
			pdacBuf = dacBuf;
		}
#endif
		// Read value from ADC1
		m_rxBuffer.put(ADC_Sample);
	}

	// get next DAC sample
    uint16_t DAC_sample = DC_OFFSET;

#if CAPTURE_DAC_OUTPUT
    if(m_txBuffer.get(DAC_sample))	{
        *pdacBuf = ((uint32_t)DAC_sample) & 0xfff;
        if((pdacBuf - dacBuf) < DAC_CAPTURE_SIZE)
        	pdacBuf++;
    } else {
    	pdacBuf = dacBuf;
    }
#else
    m_txBuffer.get(DAC_sample);
#endif


    return(DAC_sample);

}

void CIO::setLEDInt(bool on)
{
   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void CIO::setPTTInt(bool on)
{
   HAL_GPIO_WritePin(PTT_GPIO_Port, PTT_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void CIO::setCOSInt(bool on)
{
	HAL_GPIO_WritePin(COSLED_GPIO_Port, COSLED_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void CIO::setMode1Int(bool on)
{
#if defined(MODE_LEDS)
	  HAL_GPIO_WritePin(MODE1_GPIO_Port, MODE1_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void CIO::setMode2Int(bool on)
{
#if defined(MODE_LEDS)
	  HAL_GPIO_WritePin(MODE2_GPIO_Port, MODE2_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void CIO::setMode3Int(bool on)
{
#if defined(MODE_LEDS)
	  HAL_GPIO_WritePin(MODE3_GPIO_Port, MODE3_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void CIO::setMode4Int(bool on)
{
#if defined(MODE_LEDS)
	  HAL_GPIO_WritePin(MODE4_GPIO_Port, MODE4_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

bool CIO::getSquelch(void)
{
	uint8_t corin = HAL_GPIO_ReadPin(STAT1_GPIO_Port, STAT1_Pin);

	// get the correct sense
	uint8_t sense = (SQUELCH_SENSE == 1) ? corin : corin ^ 1;

	return sense ? true : false;
}

// Simple delay function for STM32
// Example from: http://thehackerworkshop.com/?p=1209
void CIO::delayInt(unsigned int dly)
{
#if defined(STM32F7XX)
  unsigned int loopsPerMillisecond = (SystemCoreClock/1000);
#else
  unsigned int loopsPerMillisecond = (SystemCoreClock/1000) / 3;
#endif

  for (; dly > 0; dly--)
  {
    asm volatile //this routine waits (approximately) one millisecond
    (
      "mov r3, %[loopsPerMillisecond] \n\t" //load the initial loop counter
      "loop: \n\t"
        "subs r3, #1 \n\t"
        "bne loop \n\t"

      : //empty output list
      : [loopsPerMillisecond] "r" (loopsPerMillisecond) //input to the asm routine
      : "r3", "cc" //clobber list
    );
  }
}

uint8_t CIO::getCPU() const
{
  return 2U;
}

void CIO::getUDID(uint8_t* buffer)
{
#if defined(STM32F4XX)
  ::memcpy(buffer, (void *)0x1FFF7A10, 12U);
#elif defined(STM32F722xx)
  ::memcpy(buffer, (void *)0x1FF07A10, 12U);
#elif defined(STM32F767xx)
  ::memcpy(buffer, (void *)0x1FF0F420, 12U);
#endif
}

