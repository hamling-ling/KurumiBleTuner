/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * Modified 28 February 2013 by masahiko.nagata.cj@renesas.com
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <RLduino78_mcu_depend.h>

// SPI_HAS_TRANSACTION means SPI has beginTransaction(), endTransaction(),
// usingInterrupt(), and SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_NOTUSINGINTERRUPT means that SPI has notUsingInterrupt() method
#define SPI_HAS_NOTUSINGINTERRUPT 1

// SPI_ATOMIC_VERSION means that SPI has atomicity fixes and what version.
// This way when there is a bug fix you can check this define to alert users
// of your code if it uses better version of this library.
// This also implies everything that SPI_HAS_TRANSACTION as documented above is
// available too.
#define SPI_ATOMIC_VERSION 1

// Uncomment this line to add detection of mismatched begin/end transactions.
// A mismatch occurs if other libraries fail to use SPI.endTransaction() for
// each SPI.beginTransaction().  Connect an LED to this pin.  The LED will turn
// on if any mismatch is ever detected.
//#define SPI_TRANSACTION_MISMATCH_LED 5

#define SPI_CLOCK_DIV2   0x00
#define SPI_CLOCK_DIV4   0x01
#define SPI_CLOCK_DIV8   0x03
#define SPI_CLOCK_DIV16  0x07
#define SPI_CLOCK_DIV32  0x0F
#define SPI_CLOCK_DIV64  0x1F
#define SPI_CLOCK_DIV128 0x3F

#define SPI_MODE0 0x30
#define SPI_MODE1 0x10
#define SPI_MODE2 0x20
#define SPI_MODE3 0x00

#define SPI_MODE_MASK 0x3000  // DAP = bit 13, CKP = bit 12 on SCR

class SPISettings {
public:
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    if (__builtin_constant_p(clock)
) {
      init_AlwaysInline(clock, bitOrder, dataMode);
    } else {
      init_MightInline(clock, bitOrder, dataMode);
    }
  }
  SPISettings() {
    init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
  }
private:
  void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    init_AlwaysInline(clock, bitOrder, dataMode);
  }
  void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    __attribute__((__always_inline__)) {
#ifndef REL_GR_KURUMI
      // Clock settings are defined as follows. Note that this shows SPI2X
    // inverted, so the bits form increasing numbers. Also note that
    // fosc/64 appears twice
    // SPR1 SPR0 ~SPI2X Freq
    //   0    0     0   fosc/2
    //   0    0     1   fosc/4
    //   0    1     0   fosc/8
    //   0    1     1   fosc/16
    //   1    0     0   fosc/32
    //   1    0     1   fosc/64
    //   1    1     0   fosc/64
    //   1    1     1   fosc/128

    // We find the fastest clock that is less than or equal to the
    // given clock rate. The clock divider that results in clock_setting
    // is 2 ^^ (clock_div + 1). If nothing is slow enough, we'll use the
    // slowest (128 == 2 ^^ 7, so clock_div = 6).
    uint8_t clockDiv;

    // When the clock is known at compiletime, use this if-then-else
    // cascade, which the compiler knows how to completely optimize
    // away. When clock is not known, use a loop instead, which generates
    // shorter code.
    if (__builtin_constant_p(clock)) {
      if (clock >= F_CPU / 2) {
        clockDiv = 0;
      } else if (clock >= F_CPU / 4) {
        clockDiv = 1;
      } else if (clock >= F_CPU / 8) {
        clockDiv = 2;
      } else if (clock >= F_CPU / 16) {
        clockDiv = 3;
      } else if (clock >= F_CPU / 32) {
        clockDiv = 4;
      } else if (clock >= F_CPU / 64) {
        clockDiv = 5;
      } else {
        clockDiv = 6;
      }
    } else {
      uint32_t clockSetting = F_CPU / 2;
      clockDiv = 0;
      while (clockDiv < 6 && clock < clockSetting) {
        clockSetting /= 2;
        clockDiv++;
      }
    }

    // Compensate for the duplicate fosc/64
    if (clockDiv == 6)
    clockDiv = 7;

    // Invert the SPI2X bit
    clockDiv ^= 0x1;

    // Pack into the SPISettings class
    spcr = _BV(SPE) | _BV(MSTR) | ((bitOrder == LSBFIRST) ? _BV(DORD) : 0) |
      (dataMode & SPI_MODE_MASK) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
    spsr = clockDiv & SPI_2XCLOCK_MASK;
#else
    if (clock >= configCPU_CLOCK_HZ / 2) {
      spdr = (SPI_CLOCK_DIV2 << 9);
    } else if (clock >= configCPU_CLOCK_HZ / 4) {
      spdr = (SPI_CLOCK_DIV4 << 9);
    } else if (clock >= configCPU_CLOCK_HZ / 8) {
      spdr = (SPI_CLOCK_DIV8 << 9);
    } else if (clock >= configCPU_CLOCK_HZ / 16) {
      spdr = (SPI_CLOCK_DIV16 << 9);
    } else if (clock >= configCPU_CLOCK_HZ / 32) {
      spdr = (SPI_CLOCK_DIV32 << 9);
    } else if (clock >= configCPU_CLOCK_HZ / 64) {
      spdr = (SPI_CLOCK_DIV64 << 9);
    } else {
      spdr = SPI_CLOCK_DIV128 << 9;
    }

    spcr = 0xF007;; // default value
    if(bitOrder == LSBFIRST) {
    	spcr |=  (1 << 7);
    } else {
        spcr &= ~(1 << 7);
    }

    spcr = (spcr & ~SPI_MODE_MASK) | ((uint16_t)dataMode << 8);

#endif
  }
#ifndef REL_GR_KURUMI
  uint8_t spcr;
  uint8_t spsr;
#else
  uint16_t spcr;
  uint16_t spdr;
#endif
  friend class SPIClass;
};


class SPIClass {
public:
	  // Initialize the SPI library
	  static void begin();

	  // If SPI is used from within an interrupt, this function registers
	  // that interrupt with the SPI library, so beginTransaction() can
	  // prevent conflicts.  The input interruptNumber is the number used
	  // with attachInterrupt.  If SPI is used from a different interrupt
	  // (eg, a timer), interruptNumber should be 255.
	  static void usingInterrupt(uint8_t interruptNumber);
	  // And this does the opposite.
	  static void notUsingInterrupt(uint8_t interruptNumber);
	  // Note: the usingInterrupt and notUsingInterrupt functions should
	  // not to be called from ISR context or inside a transaction.
	  // For details see:
	  // https://github.com/arduino/Arduino/pull/2381
	  // https://github.com/arduino/Arduino/pull/2449

	  // Before using SPI.transfer() or asserting chip select pins,
	  // this function is used to gain exclusive access to the SPI bus
	  // and configure the correct settings.
	  inline static void beginTransaction(SPISettings settings) {
	    if (interruptMode > 0) {
	#ifndef REL_GR_KURUMI
	      uint8_t sreg = SREG;
	#else
	      uint8_t sreg = isNoInterrupts();
	#endif
	      noInterrupts();

	      #ifdef SPI_AVR_EIMSK
	      if (interruptMode == 1) {
	        interruptSave = SPI_AVR_EIMSK;
	        SPI_AVR_EIMSK &= ~interruptMask;
	        SREG = sreg;
	      } else
	      #endif
	      {
	        interruptSave = sreg;
	      }
	    }

	    #ifdef SPI_TRANSACTION_MISMATCH_LED
	    if (inTransactionFlag) {
	      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
	      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
	    }
	    inTransactionFlag = 1;
	    #endif

	#ifndef REL_GR_KURUMI
	    SPCR = settings.spcr;
	    SPSR = settings.spsr;
	#else
	    if(((SPI_MODE_MASK & SPI_SDRxx) != (SPI_MODE_MASK & settings.spdr)) || (SPI_SCRxx != settings.spcr)){
		    SPI_STx  |= SPI_CHx;         // シリアル通信停止
		    SPI_SOEx &= ~SPI_CHx;        // シリアル出力停止
		    SPI_SDRxx = settings.spdr;
		    SPI_SCRxx = settings.spcr;
	#ifdef WORKAROUND_READ_MODIFY_WRITE
	        CBI(SFR_IFxx, SFR_BIT_CSIIFxx); // 割り込み要求フラグをクリア
	       SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
	        SPI_SOx     &= ~SPI_CHx;
	        SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
	        SPI_SSx     |= SPI_CHx;      // シリアル通信開始
	#else
	        SPI_CSIIFxx  = 0;            // 割り込み要求フラグをクリア
	        SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
	        SPI_SOx     &= ~SPI_CHx;
	        SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
	        SPI_SSx     |= SPI_CHx;      // シリアル通信開始
	#endif

	    }
	#endif
	  }

	  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
	  inline static uint8_t transfer(uint8_t data) {
	#ifndef REL_GR_KURUMI
	    SPDR = data;
	    /*
	     * The following NOP introduces a small delay that can prevent the wait
	     * loop form iterating when running at the maximum speed. This gives
	     * about 10% more speed, even if it seems counter-intuitive. At lower
	     * speeds it is unnoticed.
	     */
	    asm volatile("nop");
	    while (!(SPSR & _BV(SPIF))) ; // wait
	    return SPDR;
	#else
	    SPI_SIOxx = data;
	    while (!SPI_CSIIFxx);
#ifdef WORKAROUND_READ_MODIFY_WRITE
	    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
#else
        SPI_CSIIFxx = 0;
#endif
        return SPI_SIOxx;
	#endif //GRSAKURA
	  }
	#ifndef REL_GR_KURUMI
	  inline static uint16_t transfer16(uint16_t data) {
	    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
	    in.val = data;
	    if (!(SPCR & _BV(DORD))) {
	      SPDR = in.msb;
	      asm volatile("nop"); // See transfer(uint8_t) function
	      while (!(SPSR & _BV(SPIF))) ;
	      out.msb = SPDR;
	      SPDR = in.lsb;
	      asm volatile("nop");
	      while (!(SPSR & _BV(SPIF))) ;
	      out.lsb = SPDR;
	    } else {
	      SPDR = in.lsb;
	      asm volatile("nop");
	      while (!(SPSR & _BV(SPIF))) ;
	      out.lsb = SPDR;
	      SPDR = in.msb;
	      asm volatile("nop");
	      while (!(SPSR & _BV(SPIF))) ;
	      out.msb = SPDR;
	    }
	    return out.val;
	  }
	  inline static void transfer(void *buf, size_t count) {
	    if (count == 0) return;
	    uint8_t *p = (uint8_t *)buf;
	    SPDR = *p;
	    while (--count > 0) {
	      uint8_t out = *(p + 1);
	      while (!(SPSR & _BV(SPIF))) ;
	      uint8_t in = SPDR;
	      SPDR = out;
	      *p++ = in;
	    }
	    while (!(SPSR & _BV(SPIF))) ;
	    *p = SPDR;
	  }
#else
	  inline static void transfer(void *buf, size_t count) {
	    if (count == 0) return;
	    uint8_t *p = (uint8_t *)buf;
	    SPI_SIOxx = *p;
	    while (--count > 0) {
	    	uint8_t out = *(p + 1);
	    	while (!SPI_CSIIFxx);
#ifdef WORKAROUND_READ_MODIFY_WRITE
            CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
#endif
	    	uint8_t in = SPI_SIOxx;
	        SPI_SIOxx = out;
            *p++ = in;
	    }
        while (!SPI_CSIIFxx);
#ifdef WORKAROUND_READ_MODIFY_WRITE
        CBI(SFR_IFxx, SFR_BIT_CSIIFxx);*p = SPI_SIOxx;
#endif
	  }
	#endif //REL_GR_KURUMI
	  // After performing a group of transfers and releasing the chip select
	  // signal, this function allows others to access the SPI bus
	  inline static void endTransaction(void) {
	    #ifdef SPI_TRANSACTION_MISMATCH_LED
	    if (!inTransactionFlag) {
	      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
	      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
	    }
	    inTransactionFlag = 0;
	    #endif

	    if (interruptMode > 0) {
	      #ifdef SPI_AVR_EIMSK
	      uint8_t sreg = SREG;
	      #endif
	      noInterrupts();
	      #ifdef SPI_AVR_EIMSK
	      if (interruptMode == 1) {
	        SPI_AVR_EIMSK = interruptSave;
	        SREG = sreg;
	      } else
	      #endif
	      {
	#ifndef REL_GR_KURUMI
	        SREG = interruptSave;
	#else
	        if (!interruptSave) {
	          interrupts();
	        }
	#endif
	      }
	    }
	  }

	  // Disable the SPI bus
	  static void end();

	  // This function is deprecated.  New applications should use
	  // beginTransaction() to configure SPI settings.
	  inline static void setBitOrder(uint8_t bitOrder) {
	#ifndef REL_GR_KURUMI
	    if (bitOrder == LSBFIRST) SPCR |= _BV(DORD);
	    else SPCR &= ~(_BV(DORD));
	#else
	    SPI_STx  |= SPI_CHx;         // シリアル通信停止
	    SPI_SOEx &= ~SPI_CHx;        // シリアル出力停止
	    if(bitOrder == LSBFIRST) {
	      SPI_SCRxx |= (1 << 7);
	    } else {
	      SPI_SCRxx &= ~(1 << 7);
	    }
#ifdef WORKAROUND_READ_MODIFY_WRITE
        CBI(SFR_IFxx,
		SFR_BIT_CSIIFxx); // 割り込み要求フラグをクリア
        SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
        SPI_SOx     &= ~SPI_CHx;
        SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
        SPI_SSx     |= SPI_CHx;      // シリアル通信開始
#else
        SPI_CSIIFxx  = 0;            // 割り込み要求フラグをクリア
        SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
        SPI_SOx     &= ~SPI_CHx;
        SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
        SPI_SSx     |= SPI_CHx;      // シリアル通信開始
#endif

	#endif
	  }
  inline static void setDataMode(uint8_t dataMode) {
#ifndef REL_GR_KURUMI
    SPCR = (SPCR & ~SPI_MODE_MASK) | dataMode;
#else
    SPI_STx  |= SPI_CHx;         // シリアル通信停止
    SPI_SOEx &= ~SPI_CHx;        // シリアル出力停止
    SPI_SCRxx = (SPI_SCRxx & ~SPI_MODE_MASK) | ((uint16_t)dataMode << 8);
#ifdef WORKAROUND_READ_MODIFY_WRITE
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);// 割り込み要求フラグをクリア
    SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
    SPI_SOx     &= ~SPI_CHx;
    SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
    SPI_SSx     |= SPI_CHx;      // シリアル通信開始
#else
    SPI_CSIIFxx  = 0;            // 割り込み要求フラグをクリア
    SPI_SOx     |= SPI_CHx << 8; // シリアル出力バッファ設定
    SPI_SOx     &= ~SPI_CHx;
    SPI_SOEx    |= SPI_CHx;      // シリアル出力許可
    SPI_SSx     |= SPI_CHx;      // シリアル通信開始
#endif
#endif
  }
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setClockDivider(uint8_t clockDiv) {
#ifndef REL_GR_KURUMI
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (clockDiv & SPI_CLOCK_MASK);
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((clockDiv >> 2) & SPI_2XCLOCK_MASK);
#else
    SPI_STx  |= SPI_CHx;         // シリアル通信停止
    SPI_SOEx &= ~SPI_CHx;        // シリアル出力停止
    SPI_SDRxx = clockDiv << 9;
#ifdef WORKAROUND_READ_MODIFY_WRITE
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);// 割り込み要求フラグをクリア
    SPI_SOx |= SPI_CHx << 8; // シリアル出力バッファ設定
    SPI_SOx &= ~SPI_CHx;
    SPI_SOEx |= SPI_CHx;// シリアル出力許可
    SPI_SSx |= SPI_CHx;// シリアル通信開始
#else
    SPI_CSIIFxx = 0;            // 割り込み要求フラグをクリア
    SPI_SOx |= SPI_CHx << 8;// シリアル出力バッファ設定
    SPI_SOx &= ~SPI_CHx;
    SPI_SOEx |= SPI_CHx;// シリアル出力許可
    SPI_SSx |= SPI_CHx;// シリアル通信開始
#endif
#endif
}
            // These undocumented functions should not be used.  SPI.transfer()
			// polls the hardware flag which is automatically cleared as the
			// AVR responds to SPI's interrupt

#ifndef REL_GR_KURUMI
inline static void attachInterrupt() {SPCR |= _BV(SPIE);}
inline static void detachInterrupt() {SPCR &= ~_BV(SPIE);}
#endif
private:
static uint8_t initialized;
static uint8_t interruptMode; // 0=none, 1=mask, 2=global
static uint8_t interruptMask;// which interrupts to mask
static uint8_t interruptSave;// temp storage, to restore state
#ifdef SPI_TRANSACTION_MISMATCH_LED
static uint8_t inTransactionFlag;
#endif
};

extern SPIClass SPI;

#endif
