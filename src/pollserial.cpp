/*
pollserial.cpp Heavily modified version of:

  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 23 November 2006 by David A. Mellis

  Modified July 2010 by Myles D. Metzler
*/

#include <avr/io.h>
#include <stdlib.h>
#include "pollserial.h"

#define BUFFER_SIZE 256

rbuffer rxbuffer = {0,0,0};

void USART_recieve() {
#if defined ( UDR1 )
	if( UCSR1A & _BV(RXC1)) {
		uint8_t i = (rxbuffer.head + 1) & (BUFFER_SIZE - 1);
		if ( i != rxbuffer.tail) {
			rxbuffer.buffer[rxbuffer.head] = UDR1;
			rxbuffer.head = i;
		}
	}
#else
	if( UCSRA & _BV(RXC)) {
		uint8_t i = (rxbuffer.head + 1) & (BUFFER_SIZE - 1);
		if ( i != rxbuffer.tail) {
			rxbuffer.buffer[rxbuffer.head] = UDR;
			rxbuffer.head = i;
		}
	}
#endif
}

pt2Funct pollserial::begin(long baud) {
	uint16_t baud_setting;
	bool use_u2x;

	rxbuffer.buffer = (unsigned char*)malloc(BUFFER_SIZE*sizeof(unsigned char));

	if (baud > F_CPU / 16) {
		use_u2x = true;
	}
	else {

		uint8_t nonu2x_baud_error = abs((int)(255-((F_CPU/(16*(((F_CPU/8/baud-1)/2)+1))*255)/baud)));
		uint8_t u2x_baud_error = abs((int)(255-((F_CPU/(8*(((F_CPU/4/baud-1)/2)+1))*255)/baud)));

		use_u2x = (nonu2x_baud_error > u2x_baud_error);
	}
	if (use_u2x) {
#if defined ( UDR1 )
		UCSR1A = _BV(U2X1);
#else
		UCSRA = _BV(U2X);
#endif
		baud_setting = (F_CPU / 4 / baud - 1) / 2;
	}
	else {
#if defined ( UDR1 )
		UCSR1A = 0;
#else
		UCSRA = 0;
#endif
		baud_setting = (F_CPU / 8 / baud - 1) / 2;
	}

#if defined ( UDR1 )
	UBRR1 = baud_setting;
	UCSR1B = _BV(RXEN1) | _BV(TXEN1);
#else
	UBRR = baud_setting;
	UCSRB = _BV(RXEN) | _BV(TXEN);
#endif

	return &USART_recieve;
}

void pollserial::end() {
	UCSR1B &= ~(_BV(RXEN1) | _BV(TXEN1));
	free(rxbuffer.buffer);
}

uint8_t pollserial::available() {
	return (BUFFER_SIZE + rxbuffer.head - rxbuffer.tail) & (BUFFER_SIZE-1);
}

int pollserial::read() {
    if (rxbuffer.head == rxbuffer.tail)
        return -1;

    uint8_t c = rxbuffer.buffer[rxbuffer.tail];
    rxbuffer.tail = (rxbuffer.tail + 1) & (BUFFER_SIZE - 1);
    return c;
}


void pollserial::flush() {
	rxbuffer.head = rxbuffer.tail;
}

size_t pollserial::write(uint8_t c) {
#if defined ( UDR1 )
	while (!((UCSR1A) & _BV(UDRE1)));
	UDR1 = c;
#else
	while (!((UCSRA) & _BV(UDRE)));
	UDR = c;
	return 0;
#endif
}