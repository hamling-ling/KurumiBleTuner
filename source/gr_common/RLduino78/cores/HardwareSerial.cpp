/*
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
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 7 March 2013 by masahiko.nagata.cj@renesas.com
  Modified 30 June 2013 by yuuki.okamiya.yn@renesas.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "RLduino78.h"
#include "RLduino78_mcu_depend.h"
#include "iodefine.h"
#include "iodefine_ext.h"

#if ( UART_CHANNEL == 2 )
#define URT_RXDx     RXD2.rxd2
#define URT_TXDx     TXD2.txd2
#define URT_PERx     PER0.per0
#define URT_SAUxEN	 SAU1EN //:PER0.3
#define URT_SPSx     SPS1.sps1
#define URT_STx      ST1.st1
#define URT_MKxx     MK0H.mk0h
#define URT_IFxx     IF0H.if0h
#define URT_PR1xx    PR10H.pr10h
#define URT_PR0xx    PR00H.pr00h
#define URT_STMKx    STMK2  //:MK0H.0
#define URT_STIFx    STIF2  //:IF0H.0
#define URT_SRMKx    SRMK2  //:MK0H.1
#define URT_SRIFx    SRIF2  //:IF0H.1
#define URT_SREMKx   SREMK2 //:MK0H.2
#define URT_SREIFx   SREIF2 //:IF0H.2
#define URT_STPR1x   STPR12 //:PR10H.0
#define URT_STPR0x   STPR02 //:PR00H.0
#define URT_SRPR1x   SRPR12 //:PR10H.1
#define URT_SRPR0x   SRPR02 //:PR00H.1
#define URT_SREPR1x  SREPR12//:PR10H.2
#define URT_SREPR0x  SREPR02//:PR00H.2
#define URT_SMRx0    SMR10.smr10
#define URT_SCRx0    SCR10.scr10
#define URT_SDRx0    SDR10.sdr10
#define URT_NFENx    NFEN0.nfen0
#define URT_SNFENxx  SNFEN20//:NFEN0.4
#define URT_SIRx1    SIR11.sir11
#define URT_SMRx1    SMR11.smr11
#define URT_SCRx1    SCR11.scr11
#define URT_SDRx1    SDR11.sdr11
#define URT_SOx      SO1.so1
#define URT_SOxx     SO10   //:SO1.0
#define URT_SOLx     SOL1.sol1
#define URT_SOLxx    SOL10  //:SOL1.0
#define URT_SOEx     SOE1.soe1
#define URT_SOExx    SOE10  //:SOE1.0
#define URT_SSx      SS1.ss1
#define URT_SSxx     SS10   //:SS1.0
#define URT_SSRx     SSR10.ssr10
#define URT_Px       P1.p1
#define URT_PMx      PM1.pm1
#define URT_TXD_Px   0x08
#define URT_RXD_Px   0x10
#elif ( UART_CHANNEL == 0 )
#define URT_RXDx     RXD0.rxd0
#define URT_TXDx     TXD0.txd0
#define URT_PERx     PER0.per0
#define URT_SAUxEN   SAU0EN //:PER0.2
#define URT_SPSx     SPS0.sps0
#define URT_STx      ST0.st0
#define URT_MKxx     MK0H.mk0h
#define URT_IFxx     IF0H.if0h
#define URT_PR1xx    PR10H.pr10h
#define URT_PR0xx    PR00H.pr00h
#define URT_STMKx    STMK0  //:MK0H.5
#define URT_STIFx    STIF0  //:IF0H.5
#define URT_SRMKx    SRMK0  //:MK0H.6
#define URT_SRIFx    SRIF0  //:IF0H.6
#define URT_SREMKx   SREMK0 //:MK0H.7
#define URT_SREIFx   SREIF0 //:IF0H.7
#define URT_STPR1x   STPR10 //:PR10H.5
#define URT_STPR0x   STPR00 //:PR00H.5
#define URT_SRPR1x   SRPR10 //:PR10H.6
#define URT_SRPR0x   SRPR00 //:PR00H.6
#define URT_SREPR1x  SREPR10//:PR10H.7
#define URT_SREPR0x  SREPR00//:PR00H.7
#define URT_SMRx0    SMR00.smr00
#define URT_SCRx0    SCR00.scr00
#define URT_SDRx0    SDR00.sdr00
#define URT_NFENx    NFEN0.nfen0
#define URT_SNFENxx  SNFEN00//:NFEN0.0
#define URT_SIRx1    SIR01.sir01
#define URT_SMRx1    SMR01.smr01
#define URT_SCRx1    SCR01.scr01
#define URT_SDRx1    SDR01.sdr01
#define URT_SOx      SO0.so0
#define URT_SOxx     SO00   //:SO0.0
#define URT_SOLx     SOL0.sol0
#define URT_SOLxx    SOL00  //:SOL0.0
#define URT_SOEx     SOE0.soe0
#define URT_SOExx    SOE00  //:SOE0.0
#define URT_SSx      SS0.ss0
#define URT_SSxx     SS00   //:SS0.0
#define URT_SSRx     SSR00.ssr00
#define URT_Px       P1.p1
#define URT_PMx      PM1.pm1
#define URT_TXD_Px   0x04
#define URT_RXD_Px   0x02
#endif

#if ( UART1_CHANNEL == 1 )
#define URT1_RXDx     RXD1.rxd1
#define URT1_TXDx     TXD1.txd1
#define URT1_PERx     PER0.per0
#define URT1_SAUxEN   SAU0EN //:PER0.2
#define URT1_SPSx     SPS0.sps0
#define URT1_STx      ST0.st0
#define URT1_MKxx     MK1L.mk1l
#define URT1_IFxx     IF1L.if1l
#define URT1_PR1xx    PR11L.pr11l
#define URT1_PR0xx    PR01L.pr01l
#define URT1_STMKx    STMK1  //:MK1L.0
#define URT1_STIFx    STIF1  //:IF1L.0
#define URT1_SRMKx    SRMK1  //:MK1L.1
#define URT1_SRIFx    SRIF1  //:IF1L.1
#define URT1_SREMKx   SREMK1 //:MK1L.2
#define URT1_SREIFx   SREIF1 //:IF1L.2
#define URT1_STPR1x   STPR11 //:PR11L.0
#define URT1_STPR0x   STPR01 //:PR01L.0
#define URT1_SRPR1x   SRPR11 //:PR11L.1
#define URT1_SRPR0x   SRPR01 //:PR01L.1
#define URT1_SREPR1x  SREPR11//:PR11L.2
#define URT1_SREPR0x  SREPR01//:PR01L.2
#define URT1_SMRx0    SMR02.smr02
#define URT1_SCRx0    SCR02.scr02
#define URT1_SDRx0    SDR02.sdr02
#define URT1_NFENx    NFEN0.nfen0
#define URT1_SNFENxx  SNFEN10//:NFEN0.2
#define URT1_SIRx1    SIR03.sir03
#define URT1_SMRx1    SMR03.smr03
#define URT1_SCRx1    SCR03.scr03
#define URT1_SDRx1    SDR03.sdr03
#define URT1_SOx      SO0.so0
#define URT1_SOxx     SO02   //:SO0.2
#define URT1_SOLx     SOL0.sol0
#define URT1_SOLxx    SOL02  //:SOL0.2
#define URT1_SOEx     SOE0.soe0
#define URT1_SOExx    SOE02  //:SOE0.2
#define URT1_SSx      SS0.ss0
#define URT1_SSxx     SS02   //:SS0.2
#define URT1_SSRx     SSR02.ssr02
#define URT1_Px       P0.p0
#define URT1_PMx      PM0.pm0
#define URT1_TXD_Px   0x01
#define URT1_RXD_Px   0x02
#endif

#if ( UART2_CHANNEL == 2 )
#define URT2_RXDx     RXD2.rxd2
#define URT2_TXDx     TXD2.txd2
#define URT2_PERx     PER0.per0
#define URT2_SAUxEN   SAU1EN //:PER0.3
#define URT2_SPSx     SPS1.sps1
#define URT2_STx      ST1.st1
#define URT2_MKxx     MK0H.mk0h
#define URT2_IFxx     IF0H.if0h
#define URT2_PR1xx    PR10H.pr10h
#define URT2_PR0xx    PR00H.pr00h
#define URT2_STMKx    STMK2  //:MK0H.0
#define URT2_STIFx    STIF2  //:IF0H.0
#define URT2_SRMKx    SRMK2  //:MK0H.1
#define URT2_SRIFx    SRIF2  //:IF0H.1
#define URT2_SREMKx   SREMK2 //:MK0H.2
#define URT2_SREIFx   SREIF2 //:IF0H.2
#define URT2_STPR1x   STPR12 //:PR10H.0
#define URT2_STPR0x   STPR02 //:PR00H.0
#define URT2_SRPR1x   SRPR12 //:PR10H.1
#define URT2_SRPR0x   SRPR02 //:PR00H.1
#define URT2_SREPR1x  SREPR12//:PR10H.2
#define URT2_SREPR0x  SREPR02//:PR00H.2
#define URT2_SMRx0    SMR10.smr10
#define URT2_SCRx0    SCR10.scr10
#define URT2_SDRx0    SDR10.sdr10
#define URT2_NFENx    NFEN0.nfen0
#define URT2_SNFENxx  SNFEN20//:NFEN0.4
#define URT2_SIRx1    SIR11.sir11
#define URT2_SMRx1    SMR11.smr11
#define URT2_SCRx1    SCR11.scr11
#define URT2_SDRx1    SDR11.sdr11
#define URT2_SOx      SO1.so1
#define URT2_SOxx     SO10   //:SO1.0
#define URT2_SOLx     SOL1.sol1
#define URT2_SOLxx    SOL10  //:SOL1.0
#define URT2_SOEx     SOE1.soe1
#define URT2_SOExx    SOE10  //:SOE1.0
#define URT2_SSx      SS1.ss1
#define URT2_SSxx     SS10   //:SS1.0
#define URT2_SSRx     SSR10.ssr10
#define URT2_Px       P1.p1
#define URT2_PMx      PM1.pm1
#define URT2_TXD_Px   0x08
#define URT2_RXD_Px   0x10
#endif


#if ( UART_CHANNEL == 2 )
#define URT_SFR_RXDx         0xfff4a    // RXD2.rxd2
#define URT_SFR_TXDx         0xfff48    // TXD2.txd2
#define URT_SFR2_PERx        0xf00f0    // PER0.per0
#define URT_SFR2_BIT_SAUxEN  3          // SAU1EN :PER0.3
#define URT_SFR2_SPSx        0xf0166	// SPS1.sps1
#define URT_SFR2_STx         0xf0164    // ST1.st1
#define URT_SFR2_BIT_STxx0   0          // ST10   :ST1.0
#define URT_SFR2_BIT_STxx1   1          // ST11   :ST1.1
#define URT_SFR_MKxx         0xfffe5    // MK0H.mk0h
#define URT_SFR_IFxx         0xfffe1    // IF0H.if0h
#define URT_SFR_PR1xx        0xfffed    // PR10H.pr10h
#define URT_SFR_PR0xx        0xfffe9    // PR00H.pr00h
#define URT_SFR_BIT_STMKx    0          // STMK2  :MK0H.0
#define URT_SFR_BIT_STIFx    0          // STIF2  :IF0H.0
#define URT_SFR_BIT_SRMKx    1          // SRMK2  :MK0H.1
#define URT_SFR_BIT_SRIFx    1          // SRIF2  :IF0H.1
#define URT_SFR_BIT_SREMKx   2          // SREMK2 :MK0H.2
#define URT_SFR_BIT_SREIFx   2          // SREIF2 :IF0H.2
#define URT_SFR_BIT_STPR1x   0          // STPR12 :PR10H.0
#define URT_SFR_BIT_STPR0x   0          // STPR02 :PR00H.0
#define URT_SFR_BIT_SRPR1x   1          // SRPR12 :PR10H.1
#define URT_SFR_BIT_SRPR0x   1          // SRPR02 :PR00H.1
#define URT_SFR_BIT_SREPR1x  2          // SREPR12:PR10H.2
#define URT_SFR_BIT_SREPR0x  2          // SREPR02:PR00H.2
#define URT_SFR2_SMRx0       0xf0150    // SMR10.smr10
#define URT_SFR2_SCRx0       0xf0158    // SCR10.scr10
#define URT_SFR_SDRx0        0xfff48    // SDR10.sdr10
#define URT_SFR2_NFENx       0xf0070    // NFEN0.nfen0
#define URT_SFR2_BIT_SNFENxx 4          // SNFEN20:NFEN0.4
#define URT_SFR2_SIRx1       0xf014a    // SIR11.sir11
#define URT_SFR2_SMRx1       0xf0152    // SMR11.smr11
#define URT_SFR2_SCRx1       0xf015a    // SCR11.scr11
#define URT_SFR_SDRx1        0xfff4a    // SDR11.sdr11
#define URT_SFR2_SOx         0xf0168    // SO1.so1
#define URT_SFR2_BIT_SOxx    0          // SO10   :SO1.0
#define URT_SFR2_SOLx        0xf0174    // SOL1.sol1
#define URT_SFR2_BIT_SOLxx   0          // SOL10  :SOL1.0
#define URT_SFR2_SOEx        0xf016a    // SOE1.soe1
#define URT_SFR2_BIT_SOExx   0          // SOE10  :SOE1.0
#define URT_SFR2_SSx         0xf0162    // SS1.ss1
#define URT_SFR2_BIT_SSxx0   0          // SS10   :SS1.0
#define URT_SFR2_BIT_SSxx1   1          // SS11   :SS1.1
#elif ( UART_CHANNEL == 0 )
#define URT_SFR_RXDx         0xfff12    // RXD0.rxd0
#define URT_SFR_TXDx         0xfff10    // TXD0.txd0
#define URT_SFR2_PERx        0xf00f0    // PER0.per0
#define URT_SFR2_BIT_SAUxEN  2          // SAU0EN :PER0.2
#define URT_SFR2_SPSx        0xf0126    // SPS0.sps0
#define URT_SFR2_STx         0xf0124    // ST0.st0
#define URT_SFR2_BIT_STxx0   0          // ST00   :ST0.0
#define URT_SFR2_BIT_STxx1   1          // ST01   :ST0.1
#define URT_SFR_MKxx         0xfffe5    // MK0H.mk0h
#define URT_SFR_IFxx         0xfffe1    // IF0H.if0h
#define URT_SFR_PR1xx        0xfffed    // PR10H.pr10h
#define URT_SFR_PR0xx        0xfffe9    // PR00H.pr00h
#define URT_SFR_BIT_STMKx    5          // STMK0  :MK0H.5
#define URT_SFR_BIT_STIFx    5          // STIF0  :IF0H.5
#define URT_SFR_BIT_SRMKx    6          // SRMK0  :MK0H.6
#define URT_SFR_BIT_SRIFx    6          // SRIF0  :IF0H.6
#define URT_SFR_BIT_SREMKx   7          // SREMK0 :MK0H.7
#define URT_SFR_BIT_SREIFx   7          // SREIF0 :IF0H.7
#define URT_SFR_BIT_STPR1x   5          // STPR10 :PR10H.5
#define URT_SFR_BIT_STPR0x   5          // STPR00 :PR00H.5
#define URT_SFR_BIT_SRPR1x   6          // SRPR10 :PR10H.6
#define URT_SFR_BIT_SRPR0x   6          // SRPR00 :PR00H.6
#define URT_SFR_BIT_SREPR1x  7          // SREPR10:PR10H.7
#define URT_SFR_BIT_SREPR0x  7          // SREPR00:PR00H.7
#define URT_SFR2_SMRx0       0xf0110    // SMR00.smr00
#define URT_SFR2_SCRx0       0xf0118    // SCR00.scr00
#define URT_SFR_SDRx0        0xfff10    // SDR00.sdr00
#define URT_SFR2_NFENx       0xf0070    // NFEN0.nfen0
#define URT_SFR2_BIT_SNFENxx 0          // SNFEN00:NFEN0.0
#define URT_SFR2_SIRx1       0xf010a    // SIR01.sir01
#define URT_SFR2_SMRx1       0xf0112    // SMR01.smr01
#define URT_SFR2_SCRx1       0xf011a    // SCR01.scr01
#define URT_SFR2_SDRx1       0xfff12    // SDR01.sdr01
#define URT_SFR2_SOx         0xf0128    // SO0.so0
#define URT_SFR2_BIT_SOxx    0          // SO00   :SO0.0
#define URT_SFR2_SOLx        0xf0134    // SOL0.sol0
#define URT_SFR2_BIT_SOLxx   0          // SOL00  :SOL0.0
#define URT_SFR2_SOEx        0xf012a    // SOE0.soe0
#define URT_SFR2_BIT_SOExx   0          // SOE00  :SOE0.0
#define URT_SFR2_SSx         0xf0122    // SS0.ss0
#define URT_SFR2_BIT_SSxx0   0          // SS00   :SS0.0
#define URT_SFR2_BIT_SSxx1   1          // SS01   :SS0.1
#endif

#if ( UART1_CHANNEL == 1 )
#define URT1_SFR_RXDx         0xfff46    // RXD1.rxd1
#define URT1_SFR_TXDx         0xfff44    // TXD1.txd1
#define URT1_SFR2_PERx        0xf00f0    // PER0.per0
#define URT1_SFR2_BIT_SAUxEN  2          // SAU0EN :PER0.2
#define URT1_SFR2_SPSx        0xf0126    // SPS0.sps0
#define URT1_SFR2_STx         0xf0124    // ST0.st0
#define URT1_SFR2_BIT_STxx0   2          // ST02   :ST0.2
#define URT1_SFR2_BIT_STxx1   3          // ST03   :ST0.3
#define URT1_SFR_MKxx         0xfffe6    // MK1L.mk1l
#define URT1_SFR_IFxx         0xfffe2    // IF1L.if1l
#define URT1_SFR_PR1xx        0xfffee    // PR11L.pr11l
#define URT1_SFR_PR0xx        0xfffea    // PR01L.pr01l
#define URT1_SFR_BIT_STMKx    0          // STMK1  :MK1L.0
#define URT1_SFR_BIT_STIFx    0          // STIF1  :IF1L.0
#define URT1_SFR_BIT_SRMKx    1          // SRMK1  :MK1L.1
#define URT1_SFR_BIT_SRIFx    1          // SRIF1  :IF1L.1
#define URT1_SFR_BIT_SREMKx   2          // SREMK1 :MK1L.2
#define URT1_SFR_BIT_SREIFx   2          // SREIF1 :IF1L.2
#define URT1_SFR_BIT_STPR1x   0          // STPR11 :PR11L.0
#define URT1_SFR_BIT_STPR0x   0          // STPR01 :PR01L.0
#define URT1_SFR_BIT_SRPR1x   1          // SRPR11 :PR11L.1
#define URT1_SFR_BIT_SRPR0x   1          // SRPR01 :PR01L.1
#define URT1_SFR_BIT_SREPR1x  2          // SREPR11:PR11L.2
#define URT1_SFR_BIT_SREPR0x  2          // SREPR01:PR01L.2
#define URT1_SFR2_SMRx0       0xf0114    // SMR02.smr02
#define URT1_SFR2_SCRx0       0xf011c    // SCR02.scr02
#define URT1_SFR_SDRx0        0xfff44    // SDR02.sdr02
#define URT1_SFR2_NFENx       0xf0070    // NFEN0.nfen0
#define URT1_SFR2_BIT_SNFENxx 2          // SNFEN10:NFEN0.2
#define URT1_SFR2_SIRx1       0xf010e    // SIR03.sir03
#define URT1_SFR2_SMRx1       0xf0116    // SMR03.smr03
#define URT1_SFR2_SCRx1       0xf011e    // SCR03.scr03
#define URT1_SFR2_SDRx1       0xfff46    // SDR03.sdr03
#define URT1_SFR2_SOx         0xf0128    // SO0.so0
#define URT1_SFR2_BIT_SOxx    2          // SO02   :SO0.2
#define URT1_SFR2_SOLx        0xf0134    // SOL0.sol0
#define URT1_SFR2_BIT_SOLxx   2          // SOL02  :SOL0.2
#define URT1_SFR2_SOEx        0xf012a    // SOE0.soe0
#define URT1_SFR2_BIT_SOExx   2          // SOE02  :SOE0.2
#define URT1_SFR2_SSx         0xf0122    // SS0.ss0
#define URT1_SFR2_BIT_SSxx0   2          // SS02   :SS0.2
#define URT1_SFR2_BIT_SSxx1   3          // SS03   :SS0.3
#endif

#if ( UART2_CHANNEL == 2 )
#define URT2_SFR_RXDx         0xfff4a    // RXD2.rxd2
#define URT2_SFR_TXDx         0xfff48    // TXD2.txd2
#define URT2_SFR2_PERx        0xf00f0    // PER0.per0
#define URT2_SFR2_BIT_SAUxEN  3          // SAU1EN :PER0.3
#define URT2_SFR2_SPSx        0xf0166    // SPS1.sps1
#define URT2_SFR2_STx         0xf0164    // ST1.st1
#define URT2_SFR2_BIT_STxx0   0          // ST10   :ST1.0
#define URT2_SFR2_BIT_STxx1   1          // ST11   :ST1.1
#define URT2_SFR_MKxx         0xfffe5    // MK0H.mk0h
#define URT2_SFR_IFxx         0xfffe1    // IF0H.if0h
#define URT2_SFR_PR1xx        0xfffed    // PR10H.pr10h
#define URT2_SFR_PR0xx        0xfffe9    // PR00H.pr00h
#define URT2_SFR_BIT_STMKx    0          // STMK2  :MK0H.0
#define URT2_SFR_BIT_STIFx    0          // STIF2  :IF0H.0
#define URT2_SFR_BIT_SRMKx    1          // SRMK2  :MK0H.1
#define URT2_SFR_BIT_SRIFx    1          // SRIF2  :IF0H.1
#define URT2_SFR_BIT_SREMKx   2          // SREMK2 :MK0H.2
#define URT2_SFR_BIT_SREIFx   2          // SREIF2 :IF0H.2
#define URT2_SFR_BIT_STPR1x   0          // STPR12 :PR10H.0
#define URT2_SFR_BIT_STPR0x   0          // STPR02 :PR00H.0
#define URT2_SFR_BIT_SRPR1x   1          // SRPR12 :PR10H.1
#define URT2_SFR_BIT_SRPR0x   1          // SRPR02 :PR00H.1
#define URT2_SFR_BIT_SREPR1x  2          // SREPR12:PR10H.2
#define URT2_SFR_BIT_SREPR0x  2          // SREPR02:PR00H.2
#define URT2_SFR2_SMRx0       0xf0150    // SMR10.smr10
#define URT2_SFR2_SCRx0       0xf0158    // SCR10.scr10
#define URT2_SFR_SDRx0        0xfff48    // SDR10.sdr10
#define URT2_SFR2_NFENx       0xf0070    // NFEN0.nfen0
#define URT2_SFR2_BIT_SNFENxx 4          // SNFEN20:NFEN0.4
#define URT2_SFR2_SIRx1       0xf014a    // SIR11.sir11
#define URT2_SFR2_SMRx1       0xf0152    // SMR11.smr11
#define URT2_SFR2_SCRx1       0xf015a    // SCR11.scr11
#define URT2_SFR_SDRx1        0xfff4a    // SDR11.sdr11
#define URT2_SFR2_SOx         0xf0168    // SO1.so1
#define URT2_SFR2_BIT_SOxx    0          // SO10   :SO1.0
#define URT2_SFR2_SOLx        0xf0174    // SOL1.sol1
#define URT2_SFR2_BIT_SOLxx   0          // SOL10  :SOL1.0
#define URT2_SFR2_SOEx        0xf016a    // SOE1.soe1
#define URT2_SFR2_BIT_SOExx   0          // SOE10  :SOE1.0
#define URT2_SFR2_SSx         0xf0162    // SS1.ss1
#define URT2_SFR2_BIT_SSxx0   0          // SS10   :SS1.0
#define URT2_SFR2_BIT_SSxx1   1          // SS11   :SS1.1
#endif

// this next line disables the entire HardwareSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(UBRRH) || defined(UBRR0H) || defined(UBRR1H) || defined(UBRR2H) || defined(UBRR3H)|| ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 )

#include "HardwareSerial.h"

/*
 * on ATmega8, the uart and its bits are not numbered, so there is no "TXC0"
 * definition.
 */
#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
#else
#if !defined(TXC0)
#if defined(TXC)
#define TXC0 TXC
#elif defined(TXC1)
// Some devices have uart1 but no uart0
#define TXC0 TXC1
#else
#error TXC0 not definable in HardwareSerial.h
#endif
#endif
#endif

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.
#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
 #define SERIAL_BUFFER_SIZE 256
#else
#if (RAMEND < 1000)
  #define SERIAL_BUFFER_SIZE 16
#else
  #define SERIAL_BUFFER_SIZE 256
#endif
#endif

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
};

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
ring_buffer rx_buffer = { { 0 }, 0, 0};
ring_buffer tx_buffer = { { 0 }, 0, 0};
#if ( UART1_CHANNEL == 1 )
ring_buffer rx_buffer1 = { { 0 }, 0, 0};
ring_buffer tx_buffer1 = { { 0 }, 0, 0};
#endif
#if ( UART2_CHANNEL == 2 )
ring_buffer rx_buffer2 = { { 0 }, 0, 0};
ring_buffer tx_buffer2 = { { 0 }, 0, 0};
#endif
#else
#if defined(USBCON)
  ring_buffer rx_buffer = { { 0 }, 0, 0};
  ring_buffer tx_buffer = { { 0 }, 0, 0};
#endif
#if defined(UBRRH) || defined(UBRR0H)
  ring_buffer rx_buffer  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR1H)
  ring_buffer rx_buffer1  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer1  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR2H)
  ring_buffer rx_buffer2  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer2  =  { { 0 }, 0, 0 };
#endif
#if defined(UBRR3H)
  ring_buffer rx_buffer3  =  { { 0 }, 0, 0 };
  ring_buffer tx_buffer3  =  { { 0 }, 0, 0 };
#endif
#endif

inline void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

#if ( UART_CHANNEL == 0 )||( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart_rx_interrupt(void)
{
    unsigned char c = URT_RXDx;
    store_char(c, &rx_buffer);
}
#if ( UART1_CHANNEL == 1 )
extern "C"
INTERRUPT void uart1_rx_interrupt(void)
{
    unsigned char c = URT1_RXDx;
    store_char(c, &rx_buffer1);
}
#endif
#if ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart2_rx_interrupt(void)
{
    unsigned char c = URT2_RXDx;
    store_char(c, &rx_buffer2);
}
#endif
#else
#if !defined(USART0_RX_vect) && defined(USART1_RX_vect)
// do nothing - on the 32u4 the first USART is USART1
#else
#if !defined(USART_RX_vect) && !defined(SIG_USART0_RECV) && \
    !defined(SIG_UART0_RECV) && !defined(USART0_RX_vect) && \
    !defined(SIG_UART_RECV)
  #error "Don't know what the Data Received vector is called for the first UART"
#else
  void serialEvent() __attribute__((weak));
  void serialEvent() {}
  #define serialEvent_implemented
#if defined(USART_RX_vect)
  SIGNAL(USART_RX_vect)
#elif defined(SIG_USART0_RECV)
  SIGNAL(SIG_USART0_RECV)
#elif defined(SIG_UART0_RECV)
  SIGNAL(SIG_UART0_RECV)
#elif defined(USART0_RX_vect)
  SIGNAL(USART0_RX_vect)
#elif defined(SIG_UART_RECV)
  SIGNAL(SIG_UART_RECV)
#endif
  {
  #if defined(UDR0)
    if (bit_is_clear(UCSR0A, UPE0)) {
      unsigned char c = UDR0;
      store_char(c, &rx_buffer);
    } else {
      unsigned char c = UDR0;
    };
  #elif defined(UDR)
    if (bit_is_clear(UCSRA, PE)) {
      unsigned char c = UDR;
      store_char(c, &rx_buffer);
    } else {
      unsigned char c = UDR;
    };
  #else
    #error UDR not defined
  #endif
  }
#endif
#endif

#if defined(USART1_RX_vect)
  void serialEvent1() __attribute__((weak));
  void serialEvent1() {}
  #define serialEvent1_implemented
  SIGNAL(USART1_RX_vect)
  {
    if (bit_is_clear(UCSR1A, UPE1)) {
      unsigned char c = UDR1;
      store_char(c, &rx_buffer1);
    } else {
      unsigned char c = UDR1;
    };
  }
#elif defined(SIG_USART1_RECV)
  #error SIG_USART1_RECV
#endif

#if defined(USART2_RX_vect) && defined(UDR2)
  void serialEvent2() __attribute__((weak));
  void serialEvent2() {}
  #define serialEvent2_implemented
  SIGNAL(USART2_RX_vect)
  {
    if (bit_is_clear(UCSR2A, UPE2)) {
      unsigned char c = UDR2;
      store_char(c, &rx_buffer2);
    } else {
      unsigned char c = UDR2;
    };
  }
#elif defined(SIG_USART2_RECV)
  #error SIG_USART2_RECV
#endif

#if defined(USART3_RX_vect) && defined(UDR3)
  void serialEvent3() __attribute__((weak));
  void serialEvent3() {}
  #define serialEvent3_implemented
  SIGNAL(USART3_RX_vect)
  {
    if (bit_is_clear(UCSR3A, UPE3)) {
      unsigned char c = UDR3;
      store_char(c, &rx_buffer3);
    } else {
      unsigned char c = UDR3;
    };
  }
#elif defined(SIG_USART3_RECV)
  #error SIG_USART3_RECV
#endif
#endif

void serialEventRun(void)
{
#ifdef serialEvent_implemented
  if (Serial.available()) serialEvent();
#endif
#ifdef serialEvent1_implemented
  if (Serial1.available()) serialEvent1();
#endif
#ifdef serialEvent2_implemented
  if (Serial2.available()) serialEvent2();
#endif
#ifdef serialEvent3_implemented
  if (Serial3.available()) serialEvent3();
#endif
}

#if ( UART_CHANNEL == 0 )||( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart_tx_interrupt(void)
{
    if (tx_buffer.head != tx_buffer.tail) {
        // There is more data in the output buffer. Send the next byte
    	URT_TXDx = tx_buffer.buffer[tx_buffer.tail];
        tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    }
}
#if ( UART1_CHANNEL == 1 )
extern "C"
INTERRUPT void uart1_tx_interrupt(void)
{
    if (tx_buffer1.head != tx_buffer1.tail) {
        // There is more data in the output buffer. Send the next byte
    	URT1_TXDx = tx_buffer1.buffer[tx_buffer1.tail];
        tx_buffer1.tail = (tx_buffer1.tail + 1) % SERIAL_BUFFER_SIZE;
    }
}
#endif
#if ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart2_tx_interrupt(void)
{
    if (tx_buffer2.head != tx_buffer2.tail) {
        // There is more data in the output buffer. Send the next byte
    	URT2_TXDx = tx_buffer2.buffer[tx_buffer2.tail];
        tx_buffer2.tail = (tx_buffer2.tail + 1) % SERIAL_BUFFER_SIZE;
    }
}
#endif
#else
#if !defined(USART0_UDRE_vect) && defined(USART1_UDRE_vect)
// do nothing - on the 32u4 the first USART is USART1
#else
#if !defined(UART0_UDRE_vect) && !defined(UART_UDRE_vect) && !defined(USART0_UDRE_vect) && !defined(USART_UDRE_vect)
  #error "Don't know what the Data Register Empty vector is called for the first UART"
#else
#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#endif
{
  if (tx_buffer.head == tx_buffer.tail) {
    // Buffer empty, so disable interrupts
#if defined(UCSR0B)
    cbi(UCSR0B, UDRIE0);
#else
    cbi(UCSRB, UDRIE);
#endif
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer.buffer[tx_buffer.tail];
    tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    
  #if defined(UDR0)
    UDR0 = c;
  #elif defined(UDR)
    UDR = c;
  #else
    #error UDR not defined
  #endif
  }
}
#endif
#endif

#ifdef USART1_UDRE_vect
ISR(USART1_UDRE_vect)
{
  if (tx_buffer1.head == tx_buffer1.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR1B, UDRIE1);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer1.buffer[tx_buffer1.tail];
    tx_buffer1.tail = (tx_buffer1.tail + 1) % SERIAL_BUFFER_SIZE;
    
    UDR1 = c;
  }
}
#endif

#ifdef USART2_UDRE_vect
ISR(USART2_UDRE_vect)
{
  if (tx_buffer2.head == tx_buffer2.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR2B, UDRIE2);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer2.buffer[tx_buffer2.tail];
    tx_buffer2.tail = (tx_buffer2.tail + 1) % SERIAL_BUFFER_SIZE;
    
    UDR2 = c;
  }
}
#endif

#ifdef USART3_UDRE_vect
ISR(USART3_UDRE_vect)
{
  if (tx_buffer3.head == tx_buffer3.tail) {
    // Buffer empty, so disable interrupts
    cbi(UCSR3B, UDRIE3);
  }
  else {
    // There is more data in the output buffer. Send the next byte
    unsigned char c = tx_buffer3.buffer[tx_buffer3.tail];
    tx_buffer3.tail = (tx_buffer3.tail + 1) % SERIAL_BUFFER_SIZE;
    
    UDR3 = c;
  }
}
#endif
#endif

#if ( UART_CHANNEL == 0 )||( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart_error_interrupt(void)
{
    unsigned char c = URT_RXDx;
    URT_SIRx1 = 0x0007;     /* clear error flag */
}
#if ( UART1_CHANNEL == 1 )
extern "C"
INTERRUPT void uart1_error_interrupt(void)
{
    unsigned char c = URT1_RXDx;
    URT1_SIRx1 = 0x0007;     /* clear error flag */
}
#endif
#if ( UART2_CHANNEL == 2 )
extern "C"
INTERRUPT void uart2_error_interrupt(void)
{
    unsigned char c = URT2_RXDx;
    URT2_SIRx1 = 0x0007;     /* clear error flag */
}
#endif
#endif

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer,
  volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
  volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
  volatile uint8_t *ucsrc, volatile uint8_t *udr,
  uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x)
{

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )

  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
  if( rxen == 0 ){
    _urt_channel = 0;
  }
#if( UART1_CHANNEL == 1 )
  else if( rxen == 1 ){
    _urt_channel = 1;
  }
#endif
#if( UART2_CHANNEL == 2 )
  else{
    _urt_channel = 2;
  }
#endif

#else
  _rx_buffer = rx_buffer;
  _tx_buffer = tx_buffer;
  _ubrrh = ubrrh;
  _ubrrl = ubrrl;
  _ucsra = ucsra;
  _ucsrb = ucsrb;
  _ucsrc = ucsrc;
  _udr = udr;
  _rxen = rxen;
  _txen = txen;
  _rxcie = rxcie;
  _udrie = udrie;
  _u2x = u2x;
#endif
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned long baud)
{
#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
    uint16_t baud_setting;
    uint16_t clock_setting;

    for(clock_setting=0; clock_setting<16; clock_setting++){
        baud_setting = ((configCPU_CLOCK_HZ/(1<<clock_setting))/baud -1)/2;
        if( baud_setting < 128 ){
            break;
        }
    }

    if ( _urt_channel == 0 ){
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
    	URT_SPSx &= 0xfff0;
        URT_SPSx |= clock_setting & 0x000f;

        URT_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#else
        URT_SPSx &= 0xfff0;
        URT_SPSx |= clock_setting & 0x000f;

        URT_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#endif
    	uart_init2();
    }
#if ( UART1_CHANNEL == 1 )
    else if ( _urt_channel == 1 ) {
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
        URT1_SPSx &= 0xff0f;
        URT1_SPSx |= (clock_setting & 0x000f) << 4;

        URT1_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT1_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#else
        URT1_SPSx &= 0xff0f;
        URT1_SPSx |= (clock_setting & 0x000f) << 4;

        URT1_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT1_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#endif
    	uart_init2();
    }
#endif
#if ( UART2_CHANNEL == 2 )
    else{
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
    	URT2_SPSx &= 0xfff0;
        URT2_SPSx |= clock_setting & 0x000f;

        URT2_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT2_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#else
        URT2_SPSx &= 0xfff0;
        URT2_SPSx |= clock_setting & 0x000f;

        URT2_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT2_SDRx1 = ( baud_setting << 9 ) & 0xfe00;
#endif
    	uart_init2();
    }
#endif


#else
  uint16_t baud_setting;
  bool use_u2x = true;

#if F_CPU == 16000000UL
  // hardcoded exception for compatibility with the bootloader shipped
  // with the Duemilanove and previous boards and the firmware on the 8U2
  // on the Uno and Mega 2560.
  if (baud == 57600) {
    use_u2x = false;
  }
#endif

try_again:
  
  if (use_u2x) {
    *_ucsra = 1 << _u2x;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  
  if ((baud_setting > 4095) && use_u2x)
  {
    use_u2x = false;
    goto try_again;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  transmitting = false;

  sbi(*_ucsrb, _rxen);
  sbi(*_ucsrb, _txen);
  sbi(*_ucsrb, _rxcie);
  cbi(*_ucsrb, _udrie);
#endif
}

void HardwareSerial::begin(unsigned long baud, byte config)
{
#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
    uint16_t baud_setting;
    uint16_t clock_setting;

    for(clock_setting=0; clock_setting<16; clock_setting++){
        baud_setting = ((configCPU_CLOCK_HZ/(1<<clock_setting))/baud -1)/2;
        if( baud_setting < 128 ){
            break;
        }
    }

    if ( _urt_channel == 0 ){
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
    	URT_SPSx &= 0xfff0;
        URT_SPSx |= clock_setting & 0x000f;

        URT_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT_SCRx0 &= SERIAL_MASK;
        URT_SCRx0 |= config;
#else
        URT_SPSx &= 0xfff0;
        URT_SPSx |= clock_setting & 0x000f;

        URT_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT_SCRx0 &= SERIAL_MASK;
        URT_SCRx0 |= config;
#endif
    	uart_init2();
    }
#if ( UART1_CHANNEL == 1 )
    else if ( _urt_channel == 1 ) {
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
        URT1_SPSx &= 0xff0f;
        URT1_SPSx |= (clock_setting & 0x000f) << 4;

        URT1_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT1_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT1_SCRx0 &= SERIAL_MASK;
        URT1_SCRx0 |= config;
#else
        URT1_SPSx &= 0xff0f;
        URT1_SPSx |= (clock_setting & 0x000f) << 4;

        URT1_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT1_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT1_SCRx0 &= SERIAL_MASK;
        URT1_SCRx0 |= config;
#endif
    	uart_init2();
    }
#endif
#if ( UART2_CHANNEL == 2 )
    else{
    	uart_init1();
#ifdef WORKAROUND_READ_MODIFY_WRITE
    	URT2_SPSx &= 0xfff0;
        URT2_SPSx |= clock_setting & 0x000f;

        URT2_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT2_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT2_SCRx0 &= SERIAL_MASK;
        URT2_SCRx0 |= config;
#else
        URT2_SPSx &= 0xfff0;
        URT2_SPSx |= clock_setting & 0x000f;

        URT2_SDRx0 = ( baud_setting << 9 ) & 0xfe00;
        URT2_SDRx1 = ( baud_setting << 9 ) & 0xfe00;

        URT2_SCRx0 &= SERIAL_MASK;
        URT2_SCRx0 |= config;
#endif
    	uart_init2();
    }
#endif
#else
  uint16_t baud_setting;
  uint8_t current_config;
  bool use_u2x = true;

#if F_CPU == 16000000UL
  // hardcoded exception for compatibility with the bootloader shipped
  // with the Duemilanove and previous boards and the firmware on the 8U2
  // on the Uno and Mega 2560.
  if (baud == 57600) {
    use_u2x = false;
  }
#endif

try_again:
  
  if (use_u2x) {
    *_ucsra = 1 << _u2x;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  
  if ((baud_setting > 4095) && use_u2x)
  {
    use_u2x = false;
    goto try_again;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;
  
  sbi(*_ucsrb, _rxen);
  sbi(*_ucsrb, _txen);
  sbi(*_ucsrb, _rxcie);
  cbi(*_ucsrb, _udrie);
#endif
}

void HardwareSerial::end()
{
  // wait for transmission of outgoing data
  flush();

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
  if ( _urt_channel == 0 ){
#ifdef WORKAROUND_READ_MODIFY_WRITE
	URT_STx |= 0x0003;    /* disable UARTx receive and transmit */
	URT_SOEx &= ~0x0001;   /* disable UARTx output */
    SBI( URT_SFR_MKxx, URT_SFR_BIT_STMKx );    /* disable INTSTx interrupt */
    CBI( URT_SFR_IFxx, URT_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
    SBI( URT_SFR_MKxx, URT_SFR_BIT_SRMKx );    /* disable INTSRx interrupt */
    CBI( URT_SFR_IFxx, URT_SFR_BIT_SRIFx );    /* clear INTSRx interrupt flag */
    SBI( URT_SFR_MKxx, URT_SFR_BIT_SREMKx );   /* disable INTSREx interrupt */
    CBI( URT_SFR_IFxx, URT_SFR_BIT_SREIFx );   /* clear INTSREx interrupt flag */

    if( URT_SFR2_SOEx == 0x0000 ){
        CBI2( URT_SFR2_PERx, URT_SFR2_BIT_SAUxEN ); /* stop SAUx clock */
    }
#else
    URT_STx |= 0x0003;    /* disable UARTx receive and transmit */
    URT_SOEx &= ~0x0001;   /* disable UARTx output */
    URT_STMKx = 1U;    /* disable INTSTx interrupt */
    URT_STIFx = 0U;    /* clear INTSTx interrupt flag */
    URT_SRMKx = 1U;    /* disable INTSRx interrupt */
    URT_SRIFx = 0U;    /* clear INTSRx interrupt flag */
    URT_SREMKx = 1U;   /* disable INTSREx interrupt */
    URT_SREIFx = 0U;   /* clear INTSREx interrupt flag */

    if( URT_SFR2_SOEx == 0x0000 ){
        URT_SAUxEN = 0U;    /* stop SAUx clock */
    }
#endif
  }
#if ( UART1_CHANNEL == 1 )
  else if ( _urt_channel == 1 ) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
    URT1_STx |= 0x000c;    /* disable UARTx receive and transmit */
    URT1_SOEx &= ~0x0004;   /* disable UARTx output */
    SBI( URT1_SFR_MKxx, URT1_SFR_BIT_STMKx );    /* disable INTSTx interrupt */
    CBI( URT1_SFR_IFxx, URT1_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
    SBI( URT1_SFR_MKxx, URT1_SFR_BIT_SRMKx );    /* disable INTSRx interrupt */
    CBI( URT1_SFR_IFxx, URT1_SFR_BIT_SRIFx );    /* clear INTSRx interrupt flag */
    SBI( URT1_SFR_MKxx, URT1_SFR_BIT_SREMKx );   /* disable INTSREx interrupt */
    CBI( URT1_SFR_IFxx, URT1_SFR_BIT_SREIFx );   /* clear INTSREx interrupt flag */

    if( URT1_SFR2_SOEx == 0x0000 ){
        CBI2( URT1_SFR2_PERx, URT1_SFR2_BIT_SAUxEN ); /* stop SAUx clock */
    }
#else
    URT1_STx |= 0x000c;    /* disable UARTx receive and transmit */
    URT1_SOEx &= ~0x0004;   /* disable UARTx output */
    URT1_STMKx = 1U;    /* disable INTSTx interrupt */
    URT1_STIFx = 0U;    /* clear INTSTx interrupt flag */
    URT1_SRMKx = 1U;    /* disable INTSRx interrupt */
    URT1_SRIFx = 0U;    /* clear INTSRx interrupt flag */
    URT1_SREMKx = 1U;   /* disable INTSREx interrupt */
    URT1_SREIFx = 0U;   /* clear INTSREx interrupt flag */

    if( URT1_SFR2_SOEx == 0x0000 ){
        URT1_SAUxEN = 0U;    /* stop SAUx clock */
    }
#endif
  }
#endif
#if ( UART2_CHANNEL == 2 )
  else{
#ifdef WORKAROUND_READ_MODIFY_WRITE
    URT2_STx |= 0x0003;    /* disable UARTx receive and transmit */
    URT2_SOEx &= ~0x0001;   /* disable UARTx output */
    SBI( URT2_SFR_MKxx, URT2_SFR_BIT_STMKx );    /* disable INTSTx interrupt */
    CBI( URT2_SFR_IFxx, URT2_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
    SBI( URT2_SFR_MKxx, URT2_SFR_BIT_SRMKx );    /* disable INTSRx interrupt */
    CBI( URT2_SFR_IFxx, URT2_SFR_BIT_SRIFx );    /* clear INTSRx interrupt flag */
    SBI( URT2_SFR_MKxx, URT2_SFR_BIT_SREMKx );   /* disable INTSREx interrupt */
    CBI( URT2_SFR_IFxx, URT2_SFR_BIT_SREIFx );   /* clear INTSREx interrupt flag */

    if( URT2_SFR2_SOEx == 0x0000 ){
        CBI2( URT2_SFR2_PERx, URT2_SFR2_BIT_SAUxEN ); /* stop SAUx clock */
    }
#else
    URT2_STx |= 0x0003;    /* disable UARTx receive and transmit */
    URT2_SOEx &= ~0x0001;   /* disable UARTx output */
    URT2_STMKx = 1U;    /* disable INTSTx interrupt */
    URT2_STIFx = 0U;    /* clear INTSTx interrupt flag */
    URT2_SRMKx = 1U;    /* disable INTSRx interrupt */
    URT2_SRIFx = 0U;    /* clear INTSRx interrupt flag */
    URT2_SREMKx = 1U;   /* disable INTSREx interrupt */
    URT2_SREIFx = 0U;   /* clear INTSREx interrupt flag */

    if( URT2_SFR2_SOEx == 0x0000 ){
        URT2_SAUxEN = 0U;    /* stop SAUx clock */
    }
#endif
  }
#endif

#else
  cbi(*_ucsrb, _rxen);
  cbi(*_ucsrb, _txen);
  cbi(*_ucsrb, _rxcie);  
  cbi(*_ucsrb, _udrie);
#endif
  // clear any received data
  _rx_buffer->head = _rx_buffer->tail;
}

int HardwareSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
}

int HardwareSerial::peek(void)
{
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[_rx_buffer->tail];
  }
}

int HardwareSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    return c;
  }
}

void HardwareSerial::flush()
{
#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
  while (_tx_buffer->head != _tx_buffer->tail)
    ;
  if (_urt_channel == 0) {
    while (URT_SSRx & 0x0060) {  /* check TSF00 and BFF00 */
      ;
    }
  }
#if ( UART1_CHANNEL == 1 )
  else if (_urt_channel == 1) {
    while (URT1_SSRx & 0x0060) {  /* check TSF02 and BFF02 */
      ;
    }
  }
#endif
#if ( UART2_CHANNEL == 2 )
  else {
    while (URT2_SSRx & 0x0060) {  /* check TSF10 and BFF10 */
      ;
    }
  }
#endif
#else
  // UDR is kept full while the buffer is not empty, so TXC triggers when EMPTY && SENT
  while (transmitting && ! (*_ucsra & _BV(TXC0)));
  transmitting = false;
#endif
}

size_t HardwareSerial::write(uint8_t c)
{
  int i = (_tx_buffer->head + 1) % SERIAL_BUFFER_SIZE;
    
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  // ???: return 0 here instead?
  while (i == _tx_buffer->tail)
      ;

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
  noInterrupts();
#endif
  _tx_buffer->buffer[_tx_buffer->head] = c;
  _tx_buffer->head = i;

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
  if ( _urt_channel == 0 ){
    if (( URT_SSRx & 0x0040 ) == 0 ){  /* check TSF00 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
      CBI( URT_SFR_IFxx, URT_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
#else
      URT_STIFx = 0;    /* clear INTSTx interrupt flag */
#endif
      URT_TXDx = _tx_buffer->buffer[_tx_buffer->tail];
      _tx_buffer->tail = (_tx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    }
  }
#if ( UART1_CHANNEL == 1 )
  else if ( _urt_channel == 1 ) {
    if (( URT1_SSRx & 0x0040 ) == 0 ){  /* check TSF02 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
      CBI( URT1_SFR_IFxx, URT1_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
#else
	  URT1_STIFx = 0;    /* clear INTSTx interrupt flag */
#endif
	  URT1_TXDx = _tx_buffer->buffer[_tx_buffer->tail];
	  _tx_buffer->tail = (_tx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    }
  }
#endif
#if ( UART2_CHANNEL == 2 )
  else{
    if (( URT2_SSRx & 0x0040 ) == 0 ){  /* check TSF10 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
      CBI( URT2_SFR_IFxx, URT2_SFR_BIT_STIFx );    /* clear INTSTx interrupt flag */
#else
      URT2_STIFx = 0;    /* clear INTSTx interrupt flag */
#endif
	  URT2_TXDx = _tx_buffer->buffer[_tx_buffer->tail];
	  _tx_buffer->tail = (_tx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
    }
  }
#endif
  interrupts();
#else
  sbi(*_ucsrb, _udrie);
  // clear the TXC bit -- "can be cleared by writing a one to its bit location"
  transmitting = true;
  sbi(*_ucsra, TXC0);
#endif
  return 1;
}

HardwareSerial::operator bool() {
    return true;
}

void HardwareSerial::uart_init1(void)
{
    if ( _urt_channel == 0 ){
#ifdef WORKAROUND_READ_MODIFY_WRITE
        SBI2( URT_SFR2_PERx, URT_SFR2_BIT_SAUxEN ); /* supply SAUx clock */

        URT_STx |= 0x0003; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        SBI( URT_SFR_PR1xx, URT_SFR_BIT_STPR1x );
        SBI( URT_SFR_PR0xx, URT_SFR_BIT_STPR0x );
        /* Set INTSRx low priority */
        SBI( URT_SFR_PR1xx, URT_SFR_BIT_SRPR1x );
        SBI( URT_SFR_PR0xx, URT_SFR_BIT_SRPR0x );
        /* Set INTSREx low priority */
        SBI( URT_SFR_PR1xx, URT_SFR_BIT_SREPR1x );
        SBI( URT_SFR_PR0xx, URT_SFR_BIT_SREPR0x );

        URT_SMRx0 = 0x0022U;
        URT_SCRx0 = 0x8097U;
        URT_NFENx |= 0x10U;
        URT_SIRx1 = 0x0007U;    /* clear error flag */
        URT_SMRx1 = 0x0122U;
        URT_SCRx1 = 0x4497U;
#else
        URT_SAUxEN = 1U;    /* supply SAUx clock */

        URT_STx |= 0x0003; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        URT_STPR1x = 1U;
        URT_STPR0x = 1U;
        /* Set INTSRx low priority */
        URT_SRPR1x = 1U;
        URT_SRPR0x = 1U;
        /* Set INTSREx low priority */
        URT_SREPR1x = 1U;
        URT_SREPR0x = 1U;

        URT_SMRx0 = 0x0022U;
        URT_SCRx0 = 0x8097U;
        URT_NFENx |= 0x10U;
        URT_SIRx1 = 0x0007U;    /* clear error flag */
        URT_SMRx1 = 0x0122U;
        URT_SCRx1 = 0x4497U;
#endif
    }
#if ( UART1_CHANNEL == 1 )
    else if ( _urt_channel == 1 ) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
        SBI2( URT1_SFR2_PERx, URT1_SFR2_BIT_SAUxEN ); /* supply SAUx clock */

        URT1_STx |= 0x000C; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        SBI( URT1_SFR_PR1xx, URT1_SFR_BIT_STPR1x );
        SBI( URT1_SFR_PR0xx, URT1_SFR_BIT_STPR0x );
        /* Set INTSRx low priority */
        SBI( URT1_SFR_PR1xx, URT1_SFR_BIT_SRPR1x );
        SBI( URT1_SFR_PR0xx, URT1_SFR_BIT_SRPR0x );
        /* Set INTSREx low priority */
        SBI( URT1_SFR_PR1xx, URT1_SFR_BIT_SREPR1x );
        SBI( URT1_SFR_PR0xx, URT1_SFR_BIT_SREPR0x );

        URT1_SMRx0 = 0x8022U;
        URT1_SCRx0 = 0x8097U;
        URT1_NFENx |= 0x10U;
        URT1_SIRx1 = 0x0007U;    /* clear error flag */
        URT1_SMRx1 = 0x8122U;
        URT1_SCRx1 = 0x4497U;
#else
        URT1_SAUxEN = 1U;    /* supply SAUx clock */

        URT1_STx |= 0x000C; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        URT1_STPR1x = 1U;
        URT1_STPR0x = 1U;
        /* Set INTSRx low priority */
        URT1_SRPR1x = 1U;
        URT1_SRPR0x = 1U;
        /* Set INTSREx low priority */
        URT1_SREPR1x = 1U;
        URT1_SREPR0x = 1U;

        URT1_SMRx0 = 0x8022U;
        URT1_SCRx0 = 0x8097U;
        URT1_NFENx |= 0x10U;
        URT1_SIRx1 = 0x0007U;    /* clear error flag */
        URT1_SMRx1 = 0x8122U;
        URT1_SCRx1 = 0x4497U;
#endif
    }
#endif
#if ( UART2_CHANNEL == 2 )
    else{
#ifdef WORKAROUND_READ_MODIFY_WRITE
        SBI2( URT2_SFR2_PERx, URT2_SFR2_BIT_SAUxEN ); /* supply SAUx clock */

        URT2_STx |= 0x0003; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        SBI( URT2_SFR_PR1xx, URT2_SFR_BIT_STPR1x );
        SBI( URT2_SFR_PR0xx, URT2_SFR_BIT_STPR0x );
        /* Set INTSRx low priority */
        SBI( URT2_SFR_PR1xx, URT2_SFR_BIT_SRPR1x );
        SBI( URT2_SFR_PR0xx, URT2_SFR_BIT_SRPR0x );
        /* Set INTSREx low priority */
        SBI( URT2_SFR_PR1xx, URT2_SFR_BIT_SREPR1x );
        SBI( URT2_SFR_PR0xx, URT2_SFR_BIT_SREPR0x );

        URT2_SMRx0 = 0x0022U;
        URT2_SCRx0 = 0x8097U;
        URT2_NFENx |= 0x10U;
        URT2_SIRx1 = 0x0007U;    /* clear error flag */
        URT2_SMRx1 = 0x0122U;
        URT2_SCRx1 = 0x4497U;
#else
        URT2_SAUxEN = 1U;    /* supply SAUx clock */

        URT2_STx |= 0x0003; /* disable UARTx receive and transmit */
        /* Set INTSTx low priority */
        URT2_STPR1x = 1U;
        URT2_STPR0x = 1U;
        /* Set INTSRx low priority */
        URT2_SRPR1x = 1U;
        URT2_SRPR0x = 1U;
        /* Set INTSREx low priority */
        URT2_SREPR1x = 1U;
        URT2_SREPR0x = 1U;

        URT2_SMRx0 = 0x0022U;
        URT2_SCRx0 = 0x8097U;
        URT2_NFENx |= 0x10U;
        URT2_SIRx1 = 0x0007U;    /* clear error flag */
        URT2_SMRx1 = 0x0122U;
        URT2_SCRx1 = 0x4497U;
#endif
    }
#endif
}

void HardwareSerial::uart_init2(void)
{
    if ( _urt_channel == 0 ){
#ifdef WORKAROUND_READ_MODIFY_WRITE
        CBI(URT_SFR_IFxx, URT_SFR_BIT_STIFx);    /* clear INTSTx interrupt flag */
        CBI(URT_SFR_MKxx, URT_SFR_BIT_STMKx);    /* enable INTSTx interrupt */
        CBI(URT_SFR_IFxx, URT_SFR_BIT_SRIFx);    /* clear INTSRx interrupt flag */
        CBI(URT_SFR_MKxx, URT_SFR_BIT_SRMKx);    /* enable INTSRx interrupt */
        CBI(URT_SFR_IFxx, URT_SFR_BIT_SREIFx);   /* clear INTSREx interrupt flag */
        CBI(URT_SFR_MKxx, URT_SFR_BIT_SREMKx);   /* enable INTSREx interrupt */

        URT_SOEx |= 0x0001U;   /* enable UARTx output */
        URT_SSx |= 0x0003;     /* enable UARTx receive and transmit */
#else
        URT_STIFx = 0U;    /* clear INTSTx interrupt flag */
        URT_STMKx = 0U;    /* enable INTSTx interrupt */
        URT_SRIFx = 0U;    /* clear INTSRx interrupt flag */
        URT_SRMKx = 0U;    /* enable INTSRx interrupt */
        URT_SREIFx = 0U;   /* clear INTSREx interrupt flag */
        URT_SREMKx = 0U;   /* enable INTSREx interrupt */

        URT_SOEx |= 0x0001U;   /* enable UARTx output */
        URT_SSx |= 0x0003;     /* enable UARTx receive and transmit */
#endif
        /* Set RxD pin */
        URT_PMx |= URT_RXD_Px;
        /* Set TxD pin */
        URT_Px |= URT_TXD_Px;
        URT_PMx &= ~(URT_TXD_Px);
    }
#if ( UART1_CHANNEL == 1 )
    else if ( _urt_channel == 1 ) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
        CBI(URT1_SFR_IFxx, URT1_SFR_BIT_STIFx);    /* clear INTSTx interrupt flag */
        CBI(URT1_SFR_MKxx, URT1_SFR_BIT_STMKx);    /* enable INTSTx interrupt */
        CBI(URT1_SFR_IFxx, URT1_SFR_BIT_SRIFx);    /* clear INTSRx interrupt flag */
        CBI(URT1_SFR_MKxx, URT1_SFR_BIT_SRMKx);    /* enable INTSRx interrupt */
        CBI(URT1_SFR_IFxx, URT1_SFR_BIT_SREIFx);   /* clear INTSREx interrupt flag */
        CBI(URT1_SFR_MKxx, URT1_SFR_BIT_SREMKx);   /* enable INTSREx interrupt */

        URT1_SOEx |= 0x0004U;   /* enable UARTx output */
        URT1_SSx |= 0x000c;     /* enable UARTx receive and transmit */
#else
        URT1_STIFx = 0U;    /* clear INTSTx interrupt flag */
        URT1_STMKx = 0U;    /* enable INTSTx interrupt */
        URT1_SRIFx = 0U;    /* clear INTSRx interrupt flag */
        URT1_SRMKx = 0U;    /* enable INTSRx interrupt */
        URT1_SREIFx = 0U;   /* clear INTSREx interrupt flag */
        URT1_SREMKx = 0U;   /* enable INTSREx interrupt */

        URT1_SOEx |= 0x0004U;   /* enable UARTx output */
        URT1_SSx |= 0x000c;     /* enable UARTx receive and transmit */
#endif
        /* Set RxD pin */
        URT1_PMx |= URT1_RXD_Px;
        /* Set TxD pin */
        URT1_Px |= URT1_TXD_Px;
        URT1_PMx &= ~(URT1_TXD_Px);
    }
#endif
#if ( UART2_CHANNEL == 2 )
    else{
#ifdef WORKAROUND_READ_MODIFY_WRITE
    	CBI(URT2_SFR_IFxx, URT2_SFR_BIT_STIFx);    /* clear INTSTx interrupt flag */
        CBI(URT2_SFR_MKxx, URT2_SFR_BIT_STMKx);    /* enable INTSTx interrupt */
        CBI(URT2_SFR_IFxx, URT2_SFR_BIT_SRIFx);    /* clear INTSRx interrupt flag */
        CBI(URT2_SFR_MKxx, URT2_SFR_BIT_SRMKx);    /* enable INTSRx interrupt */
        CBI(URT2_SFR_IFxx, URT2_SFR_BIT_SREIFx);   /* clear INTSREx interrupt flag */
        CBI(URT2_SFR_MKxx, URT2_SFR_BIT_SREMKx);   /* enable INTSREx interrupt */

        URT2_SOEx |= 0x0001U;   /* enable UARTx output */
        URT2_SSx |= 0x0003;     /* enable UARTx receive and transmit */
#else
        URT2_STIFx = 0U;    /* clear INTSTx interrupt flag */
        URT2_STMKx = 0U;    /* enable INTSTx interrupt */
        URT2_SRIFx = 0U;    /* clear INTSRx interrupt flag */
        URT2_SRMKx = 0U;    /* enable INTSRx interrupt */
        URT2_SREIFx = 0U;   /* clear INTSREx interrupt flag */
        URT2_SREMKx = 0U;   /* enable INTSREx interrupt */

        URT2_SOEx |= 0x0001U;   /* enable UARTx output */
        URT2_SSx |= 0x0003;     /* enable UARTx receive and transmit */
#endif
        /* Set RxD pin */
        URT2_PMx |= URT2_RXD_Px;
        /* Set TxD pin */
        URT2_Px |= URT2_TXD_Px;
        URT2_PMx &= ~(URT2_TXD_Px);
    }
#endif
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if ( UART_CHANNEL == 0 ) || ( UART_CHANNEL == 2 ) || ( UART1_CHANNEL == 1 ) || ( UART2_CHANNEL == 2 )
    HardwareSerial Serial(&rx_buffer, &tx_buffer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#if ( UART1_CHANNEL == 1 )
    HardwareSerial Serial1(&rx_buffer1, &tx_buffer1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
#endif
#if ( UART2_CHANNEL == 2 )
    HardwareSerial Serial2(&rx_buffer2, &tx_buffer2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0);
#endif
#else
#if defined(UBRRH) && defined(UBRRL)
  HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR, RXEN, TXEN, RXCIE, UDRIE, U2X);
#elif defined(UBRR0H) && defined(UBRR0L)
  HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0, UDRIE0, U2X0);
#elif defined(USBCON)
  // do nothing - Serial object and buffers are initialized in CDC code
#else
  #error no serial port defined  (port 0)
#endif

#if defined(UBRR1H)
  HardwareSerial Serial1(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1, RXEN1, TXEN1, RXCIE1, UDRIE1, U2X1);
#endif
#if defined(UBRR2H)
  HardwareSerial Serial2(&rx_buffer2, &tx_buffer2, &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2, RXEN2, TXEN2, RXCIE2, UDRIE2, U2X2);
#endif
#if defined(UBRR3H)
  HardwareSerial Serial3(&rx_buffer3, &tx_buffer3, &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3, RXEN3, TXEN3, RXCIE3, UDRIE3, U2X3);
#endif
#endif

#endif // whole file

