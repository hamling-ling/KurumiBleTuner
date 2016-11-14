/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework pin define file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2012-12-18 17:02:26 +0900#$
 *
 ***************************************************************************
 * Copyright (C) 2012 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/**
 * @file  pins_arduino.h
 * @brief RLduino78フレームワーク ピン定義ファイル
 */
#ifndef PINS_ARDUINO_H
#define PINS_ARDUINO_H
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <RLduino78.h>


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#if defined(REL_GR_KURUMI)
#define NUM_DIGITAL_PINS            25
#define NUM_ANALOG_INPUTS           8
#define NUM_SWPWM_PINS              4
#else
#define NUM_DIGITAL_PINS            20
#define NUM_ANALOG_INPUTS           6
#define NUM_SWPWM_PINS              0
#endif


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 8;
static const uint8_t SCL = 7;

static const uint8_t A0 = 14;
static const uint8_t A1 = 15;
static const uint8_t A2 = 16;
static const uint8_t A3 = 17;
static const uint8_t A4 = 18;
static const uint8_t A5 = 19;
static const uint8_t A6 = 20;
static const uint8_t A7 = 21;


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
#endif /* PINS_ARDUINO_H */
