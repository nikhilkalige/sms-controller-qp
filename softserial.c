/*
  SoftwareSerial.cpp - Software serial library
  Copyright (c) 2006 David A. Mellis.  All right reserved. - hacked by ladyada

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
*/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <avr/interrupt.h>
//#include "WConstants.h"
#include "softserial.h"
#include <avr/pgmspace.h>

/******************************************************************************
 * Definitions
 ******************************************************************************/

#define AFSS_MAX_RX_BUFF 64

/******************************************************************************
 * Statics
 ******************************************************************************/

typedef struct Softserial_tag
{
    uint32_t _baudRate;
    int _bitDelay;
    //static char _receive_buffer[AFSS_MAX_RX_BUFF];
    //static uint8_t _receive_buffer_index;
} Softserial;

//static void printNumber(unsigned long, uint8_t);
static void send_byte(uint8_t b);
void printNumber(uint16_t n, uint8_t base);
static uint16_t whackDelay2(uint16_t delay);


Softserial softserial_dev;




#if (F_CPU == 16000000)
void whackDelay(uint16_t delay)
{
    uint8_t tmp = 0;

    asm volatile("sbiw    %0, 0x01 \n\t"
                 "ldi %1, 0xFF \n\t"
                 "cpi %A0, 0xFF \n\t"
                 "cpc %B0, %1 \n\t"
                 "brne .-10 \n\t"
                 : "=r" (delay), "+a" (tmp)
                 : "0" (delay)
                );
}
#endif

/******************************************************************************
 * Interrupts
 ******************************************************************************/
#if 0
SIGNAL(SIG_PIN_CHANGE0)
{
    if ((_receivePin >= 8) && (_receivePin <= 13))
    {
        recv();
    }
}
SIGNAL(SIG_PIN_CHANGE2)
{
    if (_receivePin < 8)
    {
        recv();
    }
}
#endif

#if 0
void recv(void)
{
    char i, d = 0;
    if (digitalRead(_receivePin))
        return;       // not ready!
    whackDelay(_bitDelay - 8);
    for (i = 0; i < 8; i++)
    {
        //PORTB |= _BV(5);
        whackDelay(_bitDelay * 2 - 6); // digitalread takes some time
        //PORTB &= ~_BV(5);
        if (digitalRead(_receivePin))
            d |= (1 << i);
    }
    whackDelay(_bitDelay * 2);
    if (_receive_buffer_index >=  AFSS_MAX_RX_BUFF)
        return;
    _receive_buffer[_receive_buffer_index] = d; // save data
    _receive_buffer_index++;  // got a byte
}
#endif


/******************************************************************************
 * Constructors
 ******************************************************************************/
#if 0
AFSoftSerial::AFSoftSerial(uint8_t receivePin, uint8_t transmitPin)
{
    _receivePin = receivePin;
    _transmitPin = transmitPin;
    _baudRate = 0;
}

void AFSoftSerial::setTX(uint8_t tx)
{
    _transmitPin = tx;
}
void AFSoftSerial::setRX(uint8_t rx)
{
    _receivePin = rx;
}
#endif
/******************************************************************************
 * User API
 ******************************************************************************/

void Softserial_begin(long speed)
{
    //  pinMode(_transmitPin, OUTPUT);
    SOFTSERIAL_TX_DDR |= (1 << SOFTSERIAL_TX_PIN);
    //digitalWrite(_transmitPin, HIGH);
    SOFTSERIAL_TX_PORT |= (1 << SOFTSERIAL_TX_PIN);
    //pinMode(_receivePin, INPUT);
    //digitalWrite(_receivePin, HIGH);  // pullup!
    softserial_dev._baudRate = speed;
    switch (softserial_dev._baudRate)
    {
        case 115200: // For xmit -only-!
            softserial_dev._bitDelay = 4; break;
        case 57600:
            softserial_dev._bitDelay = 14; break;
        case 38400:
            softserial_dev._bitDelay = 24; break;
        case 31250:
            softserial_dev._bitDelay = 31; break;
        case 19200:
            softserial_dev._bitDelay = 54; break;
        case 9600:
            softserial_dev._bitDelay = 113; break;
        case 4800:
            softserial_dev. _bitDelay = 232; break;
        case 2400:
            softserial_dev._bitDelay = 470; break;
        default:
            softserial_dev._bitDelay = 0;
    }
#if 0
    if (_receivePin < 8)
    {
        // a PIND pin, PCINT16-23
        PCMSK2 |= _BV(_receivePin);
        PCICR |= _BV(2);
    }
    else if (_receivePin <= 13)
    {
        // a PINB pin, PCINT0-5
        PCICR |= _BV(0);
        PCMSK0 |= _BV(_receivePin - 8);
    }
#endif
    whackDelay(softserial_dev._bitDelay * 2); // if we were low this establishes the end
}

#if 0
int AFSoftSerial::read(void)
{
    uint8_t d, i;

    if (! _receive_buffer_index)
        return -1;

    d = _receive_buffer[0]; // grab first byte
    // if we were awesome we would do some nifty queue action
    // sadly, i dont care
    for (i = 0; i < _receive_buffer_index; i++)
    {
        _receive_buffer[i] = _receive_buffer[i + 1];
    }
    _receive_buffer_index--;
    return d;
}

uint8_t AFSoftSerial::available(void)
{
    return _receive_buffer_index;
}
#endif

void send_byte(uint8_t b)
{
    if (softserial_dev._baudRate == 0)
        return;
    uint8_t mask;

    cli();  // turn off interrupts for a clean txmit

    //digitalWrite(_transmitPin, LOW);  // startbit
    SOFTSERIAL_TX_PORT &= ~(1 << SOFTSERIAL_TX_PIN);
    whackDelay(softserial_dev._bitDelay * 2);

    for (mask = 0x01; mask; mask <<= 1)
    {
        if (b & mask)  // choose bit
        {
            //digitalWrite(_transmitPin,HIGH); // send 1
            SOFTSERIAL_TX_PORT |= (1 << SOFTSERIAL_TX_PIN);
        }
        else
        {
            //digitalWrite(_transmitPin,LOW); // send 1
            SOFTSERIAL_TX_PORT &= ~(1 << SOFTSERIAL_TX_PIN);
        }
        whackDelay(softserial_dev._bitDelay * 2);
    }

    //digitalWrite(_transmitPin, HIGH);
    SOFTSERIAL_TX_PORT |= (1 << SOFTSERIAL_TX_PIN);
    sei();  // turn interrupts back on. hooray!
    whackDelay(softserial_dev._bitDelay * 2);
}

void Softserial_print(const char *s)
{
    while (*s)
        send_byte(*s++);
}

void Softserial_println(const char *s)
{
    Softserial_print(s);
    send_byte('\r');
    send_byte('\n');
}

void Softserial_print_array(const char *s, uint8_t length)
{
    uint8_t i = 0;
    while(i < length)
    {
        send_byte(s[i++]);
    }
}

void Softserial_print_flash(const char *s)
{
    while (pgm_read_byte(s))
        send_byte(pgm_read_byte(s++));
}

void Softserial_println_flash(const char *s)
{
    Softserial_print_flash(s);
    send_byte('\r');
    send_byte('\n');
}

void Softserial_print_byte(uint8_t n)
{
  printNumber((uint16_t)n, 10);
}

void printNumber(uint16_t n, uint8_t base)
{
    unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
    unsigned long i = 0;

    if (n == 0)
    {
        send_byte('0');
        return;
    }

    while (n > 0)
    {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
        send_byte((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}

#if 0
void AFSoftSerial::print(int n)
{
    print((long) n);
}

void AFSoftSerial::print(unsigned int n)
{
    print((unsigned long) n);
}

void AFSoftSerial::print(long n)
{
    if (n < 0)
    {
        print('-');
        n = -n;
    }
    printNumber(n, 10);
}

void AFSoftSerial::print(unsigned long n)
{
    printNumber(n, 10);
}

void AFSoftSerial::print(long n, int base)
{
    if (base == 0)
        print((char) n);
    else if (base == 10)
        print(n);
    else
        printNumber(n, base);
}

void AFSoftSerial::println(void)
{
    print('\r');
    print('\n');
}

void AFSoftSerial::println(char c)
{
    print(c);
    println();
}

void AFSoftSerial::println(const char c[])
{
    print(c);
    println();
}

void AFSoftSerial::println(uint8_t b)
{
    print(b);
    println();
}

void AFSoftSerial::println(int n)
{
    print(n);
    println();
}

void AFSoftSerial::println(long n)
{
    print(n);
    println();
}

void AFSoftSerial::println(unsigned long n)
{
    print(n);
    println();
}

void AFSoftSerial::println(long n, int base)
{
    print(n, base);
    println();
}
#endif
// Private Methods /////////////////////////////////////////////////////////////

#if 0
void AFSoftSerial::printNumber(unsigned long n, uint8_t base)
{
    unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
    unsigned long i = 0;

    if (n == 0)
    {
        print('0');
        return;
    }

    while (n > 0)
    {
        buf[i++] = n % base;
        n /= base;
    }

    for (; i > 0; i--)
        print((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}
#endif
