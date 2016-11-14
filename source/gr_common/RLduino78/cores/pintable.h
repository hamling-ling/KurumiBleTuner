/*
  pintable.h
  Copyright (c) 2015 Nozomu Fujita. All right reserved.

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
#ifndef _PINTABLE_H_
#define _PINTABLE_H_

#include <pins_arduino.h>
#include <RLduino78_mcu_depend.h>

#ifdef __cplusplus
extern "C" {
#endif

#define getPortModeRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_MODE_REG + port))	//!< ポート・モード・レジスタ・ベースアドレス
#define getPortPullUpRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_PULL_UP_REG + port))	//!< ポート・プルアップ・レジスタ・ベースアドレス
#define getPortInputModeRegisterAddr(port)	((volatile uint8_t*)(ADDR_PORT_IN_MODE_REG + port))	//!< ポート入力モード・レジスタ・ベースアドレス
#define getPortOutputModeRegisterAddr(port)	((volatile uint8_t*)(ADDR_PORT_OUT_MODE_REG + port))//!< ポート出力モード・レジスタ・ベースアドレス
#define getPortRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_REG + port))			//!< ポート・レジスタ・ベースアドレス

typedef const struct {
	uint8_t mask;
	uint8_t bit;
	uint8_t timer;
	volatile uint8_t* portModeRegisterAddr;
	volatile uint8_t* portPullUpRegisterAddr;
	volatile uint8_t* portInputModeRegisterAddr;
	volatile uint8_t* portOutputModeRegisterAddr;
	volatile uint8_t* portRegisterAddr;
} PinTableType;

#define _PinTable(pin, timer) { \
	DIGITAL_PIN_MASK_##pin, \
	(uint8_t)(__builtin_log((uint8_t)DIGITAL_PIN_MASK_##pin) / __builtin_log(2)), \
	timer, \
	getPortModeRegisterAddr(DIGITAL_PIN_##pin), \
	getPortPullUpRegisterAddr(DIGITAL_PIN_##pin), \
	getPortInputModeRegisterAddr(DIGITAL_PIN_##pin), \
	getPortOutputModeRegisterAddr(DIGITAL_PIN_##pin), \
	getPortRegisterAddr(DIGITAL_PIN_##pin), \
}

extern const PinTableType PinTable[NUM_DIGITAL_PINS] __attribute__((weak));
const PinTableType PinTable[NUM_DIGITAL_PINS] = {
#if defined(REL_GR_KURUMI)
	_PinTable( 0, SWPWM_PIN),
	_PinTable( 1, SWPWM_PIN),
	_PinTable( 2, SWPWM_PIN),
	_PinTable( 3, PWM_PIN_3),
	_PinTable( 4, SWPWM_PIN),
	_PinTable( 5, PWM_PIN_5),
	_PinTable( 6, PWM_PIN_6),
	_PinTable( 7, SWPWM_PIN),
	_PinTable( 8, SWPWM_PIN),
	_PinTable( 9, PWM_PIN_9),
	_PinTable(10, PWM_PIN_10),
	_PinTable(11, SWPWM_PIN),
	_PinTable(12, SWPWM_PIN),
	_PinTable(13, SWPWM_PIN),
	_PinTable(14, SWPWM_PIN),
	_PinTable(15, SWPWM_PIN),
	_PinTable(16, SWPWM_PIN),
	_PinTable(17, SWPWM_PIN),
	_PinTable(18, SWPWM_PIN),
	_PinTable(19, SWPWM_PIN),
	_PinTable(20, SWPWM_PIN),
	_PinTable(21, SWPWM_PIN),
	_PinTable(22, SWPWM_PIN),
	_PinTable(23, SWPWM_PIN),
	_PinTable(24, SWPWM_PIN),
#elif defined(REL_GR_KURUMI_PROTOTYPE)
	_PinTable( 0, SWPWM_PIN),
	_PinTable( 1, SWPWM_PIN),
	_PinTable( 2, SWPWM_PIN),
	_PinTable( 3, PWM_PIN_3),
	_PinTable( 4, SWPWM_PIN),
	_PinTable( 5, PWM_PIN_5),
	_PinTable( 6, PWM_PIN_6),
	_PinTable( 7, SWPWM_PIN),
	_PinTable( 8, SWPWM_PIN),
	_PinTable( 9, PWM_PIN_9),
	_PinTable(10, PWM_PIN_10),
	_PinTable(11, SWPWM_PIN),
	_PinTable(12, SWPWM_PIN),
	_PinTable(13, SWPWM_PIN),
	_PinTable(14, SWPWM_PIN),
	_PinTable(15, SWPWM_PIN),
	_PinTable(16, SWPWM_PIN),
	_PinTable(17, SWPWM_PIN),
	_PinTable(18, SWPWM_PIN),
	_PinTable(19, SWPWM_PIN),
#endif
};

#if 0	// GCC の吐くコードの効率が悪いのでインラインアセンブラを使用
#define getPinTable(u8Pin) (&PinTable[u8Pin])
#else
#define getPinTable(u8Pin) \
({ \
	PinTableType* p; \
	__asm __volatile( \
		"	mov	a, %1	\n" \
		"	mov	x, %2	\n" \
		"	mulu	x	\n" \
		"	addw	ax, %3	\n" \
		"	movw	%0, ax	\n" \
		: "=r"(p) \
		: "r"((uint8_t)(u8Pin)), "i"((unsigned)&PinTable[1] - (unsigned)&PinTable[0]), "i"(PinTable) \
		: "a", "x" \
	); \
	p; \
})
#endif

#ifdef __cplusplus
};
#endif

#endif/*_PINTABLE_H_*/
