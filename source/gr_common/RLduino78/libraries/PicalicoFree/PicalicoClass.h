/*
 * PicalicoClass.h
 *
 *  Created on: Nov 12, 2014
 *      Author: Renesas Electronics
 *
 ***************************************************************************
 * Copyright (C) 2014 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/* Modified by Yuuki Okamiya : Dec 17, 2014*/

/* note : this library uses attachMicroIntervalTimerHandler(),
 *        that influence to use tone().
 */


#ifndef PICALICOCLASS_H_
#define PICALICOCLASS_H_

#include "PicalicoFree.h"

#define MAX_PICALICO 4
#define NUM_PICA_PATTERN 24
#define DEFAULT_PICA_INTERVAL 100
#define INVALID_SERVO 255

typedef struct{
  uint8_t nbr        :6 ;             // a pin number from 0 to 63
  uint8_t isActive   :1 ;             // true if this channel is enabled, pin not pulsed if false
} PicalicoPin_t;

typedef struct {
	PicalicoPin_t Red;
	PicalicoPin_t Green;
	PicalicoPin_t Blue;
    unsigned int ticks;
} picalico_t;


class Picalico
{
public:
    Picalico(int led = LOW);
    uint8_t attach(int red, int green, int blue);
    uint8_t attach(int red, int green, int blue, int interval);
    void detach();
    void setActiveLed(uint8_t num, int led);
    bool write(int value); // -100 to 100
    bool write(char value); // ASCII 0x21 to 0x79
    bool writeTime(int hour, int min); // 00:00 to 23:59
    bool writePercent(int value); // 0% to 100%
    bool writeTemperature(int value); // -50C to 100C
    bool writeMessage(int num); //No. 0 to 100
    bool attached();
private:
    uint8_t picalicoIndex;
};


#endif /* PICALICOCLASS_H_ */
