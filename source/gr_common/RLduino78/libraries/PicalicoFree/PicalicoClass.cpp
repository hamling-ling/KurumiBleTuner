/*
 * PicalicoClass.cpp
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


#include "RLduino78_mcu_depend.h"
#include "RLduino78.h"
#include "PicalicoClass.h"

uint8_t PicalicoCount = 0;
static picalico_t picalicos[MAX_PICALICO];
static volatile int8_t picaTicksCount[MAX_PICALICO];
int picaPatternTable[MAX_PICALICO][NUM_PICA_PATTERN];
int picaInterval[MAX_PICALICO];
int picaIndex[MAX_PICALICO];
int picaLedOn[MAX_PICALICO];
int picaLedOff[MAX_PICALICO];


void ledout(uint8_t num, int color) {
    switch (color)
    {
        case 9 :        // 消灯
            digitalWrite( picalicos[num].Red.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Green.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Blue.nbr, picaLedOff[num] );
            break;

        case 0 :        // 赤
            digitalWrite( picalicos[num].Red.nbr, picaLedOn[num] );
            digitalWrite( picalicos[num].Green.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Blue.nbr, picaLedOff[num] );
            break;

        case 1 :        // 緑
            digitalWrite( picalicos[num].Red.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Green.nbr, picaLedOn[num] );
            digitalWrite( picalicos[num].Blue.nbr, picaLedOff[num] );
            break;

        case 2 :        // 青
            digitalWrite( picalicos[num].Red.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Green.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Blue.nbr, picaLedOn[num] );
            break;

        default :
            digitalWrite( picalicos[num].Red.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Green.nbr, picaLedOff[num] );
            digitalWrite( picalicos[num].Blue.nbr, picaLedOff[num] );
            break;
    }
}

void changeColor() {
	for (uint8_t i = 0; i < PicalicoCount; i++){
		picaTicksCount[i]++;
		if(picaInterval[i] < picaTicksCount[i]){
			ledout(i, picaPatternTable[i][picaIndex[i]]);
			picaTicksCount[i] = 0;
			picaIndex[i]++;
			if(picaIndex[i] == NUM_PICA_PATTERN){
				picaIndex[i] = 0;
			}
		}
	}
}

Picalico::Picalico(int led)
{
    if( PicalicoCount < MAX_PICALICO){
        this->picalicoIndex = PicalicoCount; // assign a picalico index to this instance
    	PicalicoCount++;
        picaTicksCount[this->picalicoIndex] = 0;
        picaIndex[this->picalicoIndex] = 0;
        picaInterval[this->picalicoIndex] = DEFAULT_PICA_INTERVAL;
        setActiveLed(this->picalicoIndex, led);
    }
    else
        this->picalicoIndex = INVALID_SERVO ;  // too many picalicos
}

void Picalico::setActiveLed(uint8_t num, int led){
    if (led == HIGH){
        picaLedOn[num] = HIGH;
        picaLedOff[num] = LOW;
    } else {
    	picaLedOn[num] = LOW;
    	picaLedOff[num] = HIGH;
    }
}

uint8_t Picalico::attach(int red, int green, int blue)
{
    return this->attach(red, green, blue, DEFAULT_PICA_INTERVAL);
}

uint8_t Picalico::attach(int red, int green, int blue, int interval)
{
    if(this->picalicoIndex < MAX_PICALICO ){
        pinMode( red, OUTPUT); // set picalico red pin to output
        pinMode( green, OUTPUT); // set picalico green pin to output
        pinMode( blue, OUTPUT); // set picalico blue pin to output
    	digitalWrite( red, picaLedOff[this->picalicoIndex]);
    	digitalWrite( green, picaLedOff[this->picalicoIndex]);
    	digitalWrite( blue, picaLedOff[this->picalicoIndex]);

        picalicos[this->picalicoIndex].Red.nbr = red;
        picalicos[this->picalicoIndex].Green.nbr = green;
        picalicos[this->picalicoIndex].Blue.nbr = blue;
        picaInterval[this->picalicoIndex] = interval;
        if(this->picalicoIndex == 0){
            attachMicroIntervalTimerHandler(changeColor, 1000);
        }
        picalicos[this->picalicoIndex].Red.isActive = true;
        picalicos[this->picalicoIndex].Green.isActive = true;
        picalicos[this->picalicoIndex].Blue.isActive = true;
    }
    return this->picalicoIndex;
}

void Picalico::detach()
{
	int check = 0;
    picalicos[this->picalicoIndex].Red.isActive = false;
    picalicos[this->picalicoIndex].Green.isActive = false;
    picalicos[this->picalicoIndex].Blue.isActive = false;
    pinMode( picalicos[this->picalicoIndex].Red.nbr, INPUT);
    pinMode( picalicos[this->picalicoIndex].Green.nbr, INPUT);
    pinMode( picalicos[this->picalicoIndex].Blue.nbr, INPUT);
	digitalWrite( picalicos[this->picalicoIndex].Red.nbr, picaLedOff[this->picalicoIndex]);
	digitalWrite( picalicos[this->picalicoIndex].Green.nbr, picaLedOff[this->picalicoIndex]);
	digitalWrite( picalicos[this->picalicoIndex].Blue.nbr, picaLedOff[this->picalicoIndex]);

	for (uint8_t i = 0; i < PicalicoCount; i++){
		if(picalicos[i].Red.isActive == true)
			check++;
	}
	if(check == 0) //in case all picalicos are off
    	detachIntervalTimerHandler();
}

bool Picalico::write(int value)
{
    return PCFsendNum(value, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::write(char value)
{
    return PCFsendChar(value, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::writeTime(int hour, int min)
{
    return PCFsendTime(hour, min, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::writePercent(int value)
{
    return PCFsendPercent(value, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::writeTemperature(int value)
{
    return PCFsendTemperature(value, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::writeMessage(int num)
{
    return PCFsendFixedPhrases(num, picaPatternTable[this->picalicoIndex]);
}

bool Picalico::attached()
{
    return picalicos[this->picalicoIndex].Red.isActive;
}
