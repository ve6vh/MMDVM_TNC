/*
 *   Copyright (C) 2023,2024 by Jonathan Naylor G4KLX
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

#include "Mode2Defines.h"
#include "Globals.h"
#include "Mode2TX.h"

// Gaussian BT 0.6 convolved with 5 sample unit step function.
static q15_t TX_PULSE_FILTER[] = {  \
      0, 0, 0, 0, 0, \
      0, 0, 0, 0, 0, \
      0, 0, 0, 0, 0, \
      0, 17, 319, 2659, 10668, \
      22736, 30728, 32767, 30728, 22736, \
      10668, 2659, 319, 17, 0, \
      0, 0, 0, 0, 0, \
      0, 0, 0, 0, 0, \
      0, 0, 0, 0, 0 };
const uint16_t TX_PULSE_FILTER_PHASE_LEN = 9U; // phaseLength = numTaps/L

const q15_t LEVELA =  1362;
const q15_t LEVELB =  454;
const q15_t LEVELC = -454;
const q15_t LEVELD = -1362;

const uint8_t BIT_MASK_TABLE1[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE1[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE1[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE1[(i)&7])

const uint8_t BIT_MASK_TABLE2[] = { 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U };

#define WRITE_BIT2(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE2[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE2[(i)&7])
#define READ_BIT2(p,i)    (p[(i)>>3] & BIT_MASK_TABLE2[(i)&7])

// test modem symbol definitions
#define	SYM_P3		0x3			// +3
#define	SYM_P1		0x2			// +1
#define	SYM_M1		0x0			// -1
#define	SYM_M3		0x1			// -3

#define	UINT8_SYM(a,b,c,d)	(((a<<6) | (b<<4) | (c<<2) | d))

// test patterns
enum test_patterns_e	{
	TEST_SYM=0,					// test symbol +3, -1, +1, -3
	TEST_PATTERN_3,				// +3,-3, +3, -3
	TEST_PATTERN_1,				// +1, -1, +1, -1
	TEST_SYNC,					// sync pattern
	N_TEST_PATTERNS
};

// test symbol +3, -1, +1, -3
uint8_t test_sym	= UINT8_SYM(SYM_P3,SYM_M1,SYM_P1,SYM_M3);

// unmodulated test patterns
uint8_t test_patt_3	= UINT8_SYM(SYM_P3,SYM_M3,SYM_P3,SYM_M3);
uint8_t test_patt_1	= UINT8_SYM(SYM_P1,SYM_M1,SYM_P1,SYM_M1);

// sync pattern +3, +3, -3, +3,    +3, +3, +3, -3,   -3, +3, -3, -3,  +3, -3, -3, -3};
#define	N_SYNC_SYMS		4		// number of sync symbols
uint8_t	test_sync[N_SYNC_SYMS] = {
		UINT8_SYM(SYM_P3, SYM_P3, SYM_M3, SYM_P3),
		UINT8_SYM(SYM_P3, SYM_P3, SYM_P3, SYM_M3),
		UINT8_SYM(SYM_M3, SYM_P3, SYM_M3, SYM_M3),
		UINT8_SYM( SYM_P3, SYM_M3, SYM_M3, SYM_M3)
};

// struct to hold test patterns
struct test_patt_t	{
	uint8_t		*pattern;		// pointer to pattern
	uint8_t		nSyms;			// number of symbols
} testPatterns[N_TEST_PATTERNS] = {
		{ &test_sym, 1},
		{ &test_patt_3, 1},
		{ &test_patt_1, 1},
		{ &test_sync[0], N_SYNC_SYMS }
};


CMode2TX::CMode2TX() :
m_fifo(3000U),
m_playOut(0U),
m_modFilter(),
m_modState(),
m_frame(),
m_level(MODE2_TX_LEVEL * 128),
m_txDelay((TX_DELAY / 10U) * 12U),
m_txTail((TX_TAIL / 10U) * 12U),
m_tokens(),
m_testmode(0U)
{
  ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));

  m_modFilter.L           = MODE2_RADIO_SYMBOL_LENGTH;
  m_modFilter.phaseLength = TX_PULSE_FILTER_PHASE_LEN;
  m_modFilter.pCoeffs     = TX_PULSE_FILTER;
  m_modFilter.pState      = m_modState;
}

void CMode2TX::process()
{
  // in test mode
  if(m_testmode)		{

	  // see if the test is within bounds
	   if(m_testmode > N_TEST_PATTERNS)	{
		   m_testmode = 0;						// invalid test mode
		   return;
	   }

	   // keep filling the buffer with test symbols
	    uint8_t testNum = m_testmode -1;
	    uint8_t nSyms = testPatterns[testNum].nSyms;
	    uint16_t space = io.getSpace();

	    // put the pattern in the buffer
	    while (space > nSyms*(MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH)) {
		    uint8_t *pattern = testPatterns[testNum].pattern;
	    	for(int k=0;k<nSyms;k++)	{
	    		uint8_t c = *pattern++;
	    		writeByte(c);
	    	}
	      space -= nSyms * ( MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH);
	    }
	    return;
  	  }

  if (!m_duplex) {
    // Nothing left to transmit, send the packet tokens back
    if (!m_tx && m_fifo.getData() == 0U) {
      m_tokens.reset();
      uint16_t token;
      while (m_tokens.next(token))
        serial.writeKISSAck(token);
      m_tokens.clear();
    }
  } else {
    // Send the tokens back immediately as the packets can be transmitted immediately too
    m_tokens.reset();
    uint16_t token;
    while (m_tokens.next(token))
      serial.writeKISSAck(token);
    m_tokens.clear();
  }

  // Transmit is off but we have data to send
  if (!m_tx && m_fifo.getData() > 0U) {
    bool tx = io.canTX();
    if (!tx)
      return;
  }

  // Are we sending the trailer?
  if (m_playOut > 0U) {
    uint16_t space = io.getSpace();
    while (space > (MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH)) {
      writeSilence();

      space -= MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH;
      m_playOut--;

      if (m_playOut == 0U)
        break;
    }

    return;
  }

  if (m_fifo.getData() > 0U) {
    uint16_t space = io.getSpace();
    while (space > (MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH)) {
      uint8_t c = 0U;
      m_fifo.get(c);

      writeByte(c);

      space -= MODE2_SYMBOLS_PER_BYTE * MODE2_RADIO_SYMBOL_LENGTH;

      if (m_fifo.getData() == 0U) {
        m_playOut = m_txTail;
        return;
      }
    }
  }
}

uint8_t CMode2TX::writeData(const uint8_t* data, uint16_t length)
{
  uint16_t space = m_fifo.getSpace();
  if (space < (length + MODE2_SYNC_LENGTH_BYTES)) {
    DEBUG1("Mode2TX: no space for the packet");
    return 5U;
  }

  // Add the preamble symbols
  if (!m_tx && (m_fifo.getData() == 0U)) {
    for (uint16_t i = 0U; i < m_txDelay; i++)
      m_fifo.put(MODE2_PREAMBLE_BYTE);
  }

  // Add the IL2P sync vector
  for (uint8_t i = 0U; i < MODE2_SYNC_LENGTH_BYTES; i++)
    m_fifo.put(MODE2_SYNC_BYTES[i]);

  uint8_t buffer[2000U];
  uint16_t len = m_frame.process(data, length, buffer);

  for (uint16_t i = 0U; i < len; i++)
    m_fifo.put(buffer[i]);

  // Insert some spacer
  for (uint8_t i = 0U; i < 10U; i++)
    m_fifo.put(MODE2_PREAMBLE_BYTE);

  return 0U;
}

uint8_t CMode2TX::writeDataAck(uint16_t token, const uint8_t* data, uint16_t length)
{
  m_tokens.add(token);

  return writeData(data, length);
}

void CMode2TX::writeByte(uint8_t c)
{
  q15_t inBuffer[MODE2_SYMBOLS_PER_BYTE];

  const uint8_t MASK = 0xC0U;

  for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
    q15_t value = 0;

    switch (c & MASK) {
      case 0xC0U:
        value = LEVELA;
        break;
      case 0x80U:
        value = LEVELB;
        break;
      case 0x00U:
        value = LEVELC;
        break;
      default:
        value = LEVELD;
        break;
    }

    q31_t res = value * m_level;

    inBuffer[i] = q15_t(__SSAT((res >> 15), 16));
  }

  q15_t outBuffer[MODE2_RADIO_SYMBOL_LENGTH * 4U];
  ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, MODE2_SYMBOLS_PER_BYTE);

  io.write(outBuffer, MODE2_RADIO_SYMBOL_LENGTH * MODE2_SYMBOLS_PER_BYTE);
}

void CMode2TX::writeSilence()
{
  q15_t inBuffer[MODE2_SYMBOLS_PER_BYTE] = {0, 0, 0, 0};
  q15_t outBuffer[MODE2_RADIO_SYMBOL_LENGTH * 4U];

  ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, MODE2_SYMBOLS_PER_BYTE);

  io.write(outBuffer, MODE2_RADIO_SYMBOL_LENGTH * MODE2_SYMBOLS_PER_BYTE);
}

void CMode2TX::setTXDelay(uint8_t value)
{
  m_txDelay = value * 12U;
}
  
void CMode2TX::setTXTail(uint8_t value)
{
  m_txTail = value * 12U;
}
  
void CMode2TX::setLevel(uint8_t value)
{
  m_level = q15_t(value * 128);
}

void CMode2TX::setTestMode(uint8_t mode)
{
	m_testmode = mode;
}
