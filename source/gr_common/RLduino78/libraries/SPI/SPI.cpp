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

#include "pins_arduino.h"
#include "SPI.h"

SPIClass SPI;

uint8_t SPIClass::initialized = 0;
uint8_t SPIClass::interruptMode = 0;
uint8_t SPIClass::interruptMask = 0;
uint8_t SPIClass::interruptSave = 0;

void SPIClass::begin() {
#ifndef REL_GR_KURUMI
  uint8_t sreg = SREG;
#else
  uint8_t sreg = isNoInterrupts();
#endif
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
#ifndef REL_GR_KURUMI
  if (!initialized) {
    // Set SS to high so a connected chip will be "deselected" by default
    uint8_t port = digitalPinToPort(SS);
    uint8_t bit = digitalPinToBitMask(SS);
    volatile uint8_t *reg = portModeRegister(port);

    // if the SS pin is not already configured as an output
    // then set it high (to enable the internal pull-up resistor)
    if(!(*reg & bit)){
      digitalWrite(SS, HIGH);
    }

    // When the SS pin is set as OUTPUT, it can be used as
    // a general purpose output port (it doesn't influence
    // SPI operations).
    pinMode(SS, OUTPUT);

    // Warning: if the SS pin ever becomes a LOW INPUT then SPI
    // automatically switches to Slave, so the data direction of
    // the SS pin MUST be kept as OUTPUT.
    SPCR |= _BV(MSTR);
    SPCR |= _BV(SPE);

    // Set direction register for SCK and MOSI pin.
    // MISO pin automatically overrides to INPUT.
    // By doing this AFTER enabling SPI, we avoid accidentally
    // clocking in a single bit since the lines go directly
    // from "input" to SPI control.
    // http://code.google.com/p/arduino/issues/detail?id=888
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);
  }
#else
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SS, OUTPUT);

  digitalWrite(SCK, HIGH);
  digitalWrite(MOSI, HIGH);
  digitalWrite(SS, HIGH);

  if (SPI_SAUxEN == 0) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SBI2(SFR2_PER0, SFR2_BIT_SAUxEN);  // クロック供給開始
#else
    SPI_SAUxEN = 1;                    // クロック供給開始
#endif
    NOP();
    NOP();
    NOP();
    NOP();
    SPI_SPSx = 0x0001;                 // 動作クロック設定
  }

#ifdef WORKAROUND_READ_MODIFY_WRITE
  SPI_STx      |= SPI_CHx;             // シリアル通信停止
  SBI(SFR_MKxx,  SFR_BIT_CSIMKxx);     // 割り込み処理禁止
  CBI(SFR_IFxx,  SFR_BIT_CSIIFxx);     // 割り込み要求フラグをクリア
  CBI(SFR_PR1xx, SFR_BIT_CSIPR1xx);    // 割り込み優先順位の設定
  CBI(SFR_PR0xx, SFR_BIT_CSIPR0xx);
#else
  SPI_STx      |= SPI_CHx;             // シリアル通信停止
  SPI_CSIMKxx   = 1;                   // 割り込み処理禁止
  SPI_CSIIFxx   = 0;                   // 割り込み要求フラグをクリア
  SPI_CSIPR1xx  = 0;                   // 割り込み優先順位の設定
  SPI_CSIPR0xx  = 0;
#endif
  SPI_SIRxx     = 0x0007;              // エラーフラグをクリア
  SPI_SMRxx     = 0x0020;              // モード設定
  SPI_SCRxx     = 0xF007;              // シリアル通信動作設定
  SPI_SDRxx     = SPI_CLOCK_DIV4 << 9; // 動作クロックの分周設定

#ifdef WORKAROUND_READ_MODIFY_WRITE
  CBI(SFR_IFxx,   SFR_BIT_CSIIFxx); // 割り込み要求フラグをクリア
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
  initialized++; // reference count
#ifndef REL_GR_KURUMI
  SREG = sreg;
#else
  if (!sreg) {
    interrupts();
  }
#endif
}

void SPIClass::end() {
#ifndef REL_GR_KURUMI
  uint8_t sreg = SREG;
#else
  uint8_t sreg = isNoInterrupts();
#endif
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  // Decrease the reference counter
  if (initialized)
    initialized--;
  // If there are no more references disable SPI
  if (!initialized) {
#ifndef REL_GR_KURUMI
    SPCR &= ~_BV(SPE);
#else
    SPI_STx  |= SPI_CHx;         // シリアル通信停止
    SPI_SOEx &= ~SPI_CHx;        // シリアル出力停止
#ifdef WORKAROUND_READ_MODIFY_WRITE
    CBI2(SFR2_PER0, SFR2_BIT_SAUxEN);  // クロック供給停止
#else
    SPI_SAUxEN = 0;                    // クロック供給停止
#endif
#endif
    interruptMode = 0;
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    inTransactionFlag = 0;
    #endif
  }
#ifndef REL_GR_KURUMI
  SREG = sreg;
#else
  if (!sreg) {
    interrupts();
  }
#endif
}

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
  uint8_t mask = 0;
#ifndef REL_GR_KURUMI
  uint8_t sreg = SREG;
#else
  uint8_t sreg = isNoInterrupts();
#endif
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  #ifdef SPI_INT3_MASK
  case 3: mask = SPI_INT3_MASK; break;
  #endif
  #ifdef SPI_INT4_MASK
  case 4: mask = SPI_INT4_MASK; break;
  #endif
  #ifdef SPI_INT5_MASK
  case 5: mask = SPI_INT5_MASK; break;
  #endif
  #ifdef SPI_INT6_MASK
  case 6: mask = SPI_INT6_MASK; break;
  #endif
  #ifdef SPI_INT7_MASK
  case 7: mask = SPI_INT7_MASK; break;
  #endif
  default:
    interruptMode = 2;
    break;
  }
  interruptMask |= mask;
  if (!interruptMode)
    interruptMode = 1;
#ifndef REL_GR_KURUMI
  SREG = sreg;
#else
  if (!sreg) {
    interrupts();
  }
#endif
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber)
{
  // Once in mode 2 we can't go back to 0 without a proper reference count
  if (interruptMode == 2)
    return;
  uint8_t mask = 0;
#ifndef REL_GR_KURUMI
  uint8_t sreg = SREG;
#else
  uint8_t sreg = isNoInterrupts();
#endif
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  #ifdef SPI_INT3_MASK
  case 3: mask = SPI_INT3_MASK; break;
  #endif
  #ifdef SPI_INT4_MASK
  case 4: mask = SPI_INT4_MASK; break;
  #endif
  #ifdef SPI_INT5_MASK
  case 5: mask = SPI_INT5_MASK; break;
  #endif
  #ifdef SPI_INT6_MASK
  case 6: mask = SPI_INT6_MASK; break;
  #endif
  #ifdef SPI_INT7_MASK
  case 7: mask = SPI_INT7_MASK; break;
  #endif
  default:
    break;
    // this case can't be reached
  }
  interruptMask &= ~mask;
  if (!interruptMask)
    interruptMode = 0;
#ifndef REL_GR_KURUMI
  SREG = sreg;
#else
  if (!sreg) {
    interrupts();
  }
#endif
}
