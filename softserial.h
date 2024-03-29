/*
  SoftwareSerial.h - Software serial library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

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

#ifndef AFSoftSerial_h
#define AFSoftSerial_h

#include <inttypes.h>

#define SOFTSERIAL_TX_PORT		PORTD
#define SOFTSERIAL_TX_DDR		DDRD
#define SOFTSERIAL_TX_PIN		4

void Softserial_begin(long);
void Softserial_print(const char[]);
void Softserial_println(const char[]);
void Softserial_print_array(const char *s, uint8_t length);
void Softserial_print_flash(const char[]);
void Softserial_println_flash(const char[]);
void Softserial_print_byte(uint16_t n);

#if 0
class AFSoftSerial
{
  private:
    long _baudRate;
    void printNumber(unsigned long, uint8_t);

  public:
    AFSoftSerial(uint8_t, uint8_t);
    void setTX(uint8_t tx);
    void setRX(uint8_t rx);
    void begin(long);
    int read();
    uint8_t available(void);
    void print(char);
    void print(const char[]);
    void print(uint8_t);
    void print(int);
    void print(unsigned int);
    void print(long);
    void print(unsigned long);
    void print(long, int);
    void println(void);
    void println(char);
    void println(const char[]);
    void println(uint8_t);
    void println(int);
    void println(long);
    void println(unsigned long);
    void println(long, int);
};
#endif
#endif

