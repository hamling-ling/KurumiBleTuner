/*
  HardwareSerial.h - Hardware serial library for Wiring
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

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 7 March 2013 by masahiko.nagata.cj@renesas.com
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>

#include "Stream.h"

#include "RLduino78.h"
#include "RLduino78_mcu_depend.h"

struct ring_buffer;

class HardwareSerial : public Stream
{
  private:
    ring_buffer *_rx_buffer;
    ring_buffer *_tx_buffer;
#if (UART_CHANNEL==0) || (UART_CHANNEL==2) || (UART1_CHANNEL==1) || (UART2_CHANNEL==2)
    uint8_t _urt_channel;
    void uart_init1(void);
    void uart_init2(void);
#else
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_ucsrc;
    volatile uint8_t *_udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
    uint8_t _udrie;
    uint8_t _u2x;
#endif
    bool transmitting;
  public:
    HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer,
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x);
    void begin(unsigned long);
    void begin(unsigned long, uint8_t);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool();
};

// Define config for Serial.begin(baud, config);
#if (UART_CHANNEL==0) || (UART_CHANNEL==2) || (UART1_CHANNEL==2)
#define SERIAL_5N1  0x0013	//	5bit->8bit
#define SERIAL_6N1  0x0013	//  6bit->8bit
#define SERIAL_7N1  0x0012
#define SERIAL_8N1  0x0013
#define SERIAL_5N2  0x0023	//	5bit->8bit
#define SERIAL_6N2  0x0023	//  6bit->8bit
#define SERIAL_7N2  0x0022
#define SERIAL_8N2  0x0023
#define SERIAL_5E1  0x0213	//	5bit->8bit
#define SERIAL_6E1  0x0213	//  6bit->8bit
#define SERIAL_7E1  0x0212
#define SERIAL_8E1  0x0213
#define SERIAL_5E2  0x0223	//	5bit->8bit
#define SERIAL_6E2  0x0223	//  6bit->8bit
#define SERIAL_7E2  0x0222
#define SERIAL_8E2  0x0223
#define SERIAL_5O1  0x0313	//	5bit->8bit
#define SERIAL_6O1  0x0313	//  6bit->8bit
#define SERIAL_7O1  0x0312
#define SERIAL_8O1  0x0313
#define SERIAL_5O2  0x0323	//	5bit->8bit
#define SERIAL_6O2  0x0323	//  6bit->8bit
#define SERIAL_7O2  0x0322
#define SERIAL_8O2  0x0323
#define SERIAL_MASK 0xFCCC
#else
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E
#endif

#if (UART_CHANNEL==0) || (UART_CHANNEL==2) || (UART1_CHANNEL==2) || (UART2_CHANNEL==2)
    extern HardwareSerial Serial;
#if (UART1_CHANNEL==1)
    extern HardwareSerial Serial1;
#endif
#if (UART2_CHANNEL==2)
    extern HardwareSerial Serial2;
#endif
#else
#if defined(UBRRH) || defined(UBRR0H)
  extern HardwareSerial Serial;
#elif defined(USBCON)
  #include "USBAPI.h"
//  extern HardwareSerial Serial_;  
#endif
#if defined(UBRR1H)
  extern HardwareSerial Serial1;
#endif
#if defined(UBRR2H)
  extern HardwareSerial Serial2;
#endif
#if defined(UBRR3H)
  extern HardwareSerial Serial3;
#endif
#endif

extern void serialEventRun(void) __attribute__((weak));

#endif
