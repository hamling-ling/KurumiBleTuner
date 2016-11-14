/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.
 
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

  Modified 1 March 2013 by masahiko.nagata.cj@renesas.com
 */

/* 
 
 A servo is activated by creating an instance of the Servo class passing the desired pin to the attach() method.
 The servos are pulsed in the background using the value most recently written using the write() method
 
 Note that analogWrite of PWM on pins associated with the timer are disabled when the first servo is attached.
 Timers are seized as needed in groups of 12 servos - 24 servos use two timers, 48 servos will use four.
 
 The methods are:
 
 Servo - Class for manipulating servo motors connected to Arduino pins.
 
 attach(pin )  - Attaches a servo motor to an i/o pin.
 attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
 default min is 544, max is 2400  
 
 write()     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
 writeMicroseconds() - Sets the servo pulse width in microseconds 
 read()      - Gets the last written servo pulse width as an angle between 0 and 180. 
 readMicroseconds()   - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
 attached()  - Returns true if there is a servo attached. 
 detach()    - Stops an attached servos from pulsing its i/o pin. 
 
*/

#include "RLduino78_mcu_depend.h"
#include "Servo.h"


#if ( SERVO_CHANNEL == 1 )
#define	SRV_TAUxEN	TAU0EN
#define	SRV_TPSx	TPS0.tps0
#define	SRV_TDRxx	TDR01.tdr01
#define	SRV_TMPR1xx	TMPR101
#define	SRV_TMPR0xx	TMPR001
#define	SRV_TMMKxx	TMMK01
#define	SRV_TMIFxx	TMIF01
#define	SRV_TMRxx	TMR01.tmr01
#define	SRV_TSx		TS0.ts0
#define	SRV_TTx     TT0.tt0
#define	SRV_CHx		0x0002

#elif ( SERVO_CHANNEL == 4 )
#define	SRV_TAUxEN	TAU0EN
#define	SRV_TPSx	TPS0.tps0
#define SRV_TDRxx	TDR04.tdr04
#define SRV_TCRxx	TCR04.tcr04
#define	SRV_TMPR1xx	TMPR104
#define	SRV_TMPR0xx	TMPR004
#define	SRV_TMMKxx	TMMK04
#define	SRV_TMIFxx	TMIF04
#define	SRV_TMRxx	TMR04.tmr04
#define	SRV_TSx		TS0.ts0
#define	SRV_TTx     TT0.tt0
#define	SRV_CHx		0x0010

#elif ( SERVO_CHANNEL == 5 )
#define	SRV_TAUxEN	TAU0EN
#define	SRV_TPSx	TPS0.tps0
#define SRV_TDRxx	TDR05.tdr05
#define	SRV_TMPR1xx	TMPR105
#define	SRV_TMPR0xx	TMPR005
#define	SRV_TMMKxx	TMMK05
#define	SRV_TMIFxx	TMIF05
#define	SRV_TMRxx	TMR05.tmr05
#define	SRV_TSx		TS0.ts0
#define	SRV_TTx     TT0.tt0
#define	SRV_CHx		0x0020

#endif


#ifdef WORKAROUND_READ_MODIFY_WRITE

#if ( SERVO_CHANNEL == 1 )
#define SRV_SFR2_PERx        0xf00f0    // PER0
#define SRV_SFR2_BIT_TAUxEN  0          // PER0.0:TAU0EN
#define SRV_SFR_PR1xx        0xfffee    // PR11L
#define SRV_SFR_BIT_TMPR1xx  5          // PR11L.5:TMPR101
#define SRV_SFR_PR0xx        0xfffea    // PR01L
#define SRV_SFR_BIT_TMPR0xx  5          // PR01L.5:TMPR001
#define SRV_SFR_MKxx         0xfffe6    // MK1L
#define SRV_SFR_BIT_TMMKxx   5          // MK1L.5:TMMK01
#define SRV_SFR_IFxx         0xfffe2	// IF1L
#define SRV_SFR_BIT_TMIFxx   5          // IF1L.5:TMIF01
#define	SRV_SFR2_TSx         0xf01b2    // TS0
#define	SRV_SFR2_TTx         0xf01b4    // TT0
#define	SRV_SFR2_BIT_CHx     1          // 0x0002

#elif ( SERVO_CHANNEL == 4 )
#define SRV_SFR2_PERx        0xf00f0    // PER0
#define SRV_SFR2_BIT_TAUxEN  0          // PER0.0:TAU0EN
#define SRV_SFR_PR1xx        0xfffef    // PR11H
#define SRV_SFR_BIT_TMPR1xx  7          // PR11H.7:TMPR104
#define SRV_SFR_PR0xx        0xfffeb    // PR01H
#define SRV_SFR_BIT_TMPR0xx  7          // PR01H.7:TMPR004
#define SRV_SFR_MKxx         0xfffe7    // MK1H
#define SRV_SFR_BIT_TMMKxx   7          // MK1H.7:TMMK04
#define SRV_SFR_IFxx         0xfffe3	// IF1H
#define SRV_SFR_BIT_TMIFxx   7          // IF1H.7:TMIF04
#define	SRV_SFR2_TSx         0xf01b2    // TS0
#define	SRV_SFR2_TTx         0xf01b4    // TT0
#define	SRV_SFR2_BIT_CHx     4          // 0x0010

#elif ( SERVO_CHANNEL == 5 )
#define SRV_SFR2_PERx        0xf00f0    // PER0
#define SRV_SFR2_BIT_TAUxEN  0          // TAU0EN
#define SRV_SFR_PR1xx        0xfffdc    // PR12L
#define SRV_SFR_BIT_TMPR1xx  0          // PR12L.0:TMPR105
#define SRV_SFR_PR0xx        0xfffd8    // PR02L
#define SRV_SFR_BIT_TMPR0xx  0          // PR02L.0:TMPR005
#define SRV_SFR_MKxx         0xfffd4    // MK2L
#define SRV_SFR_BIT_TMMKxx   0          // MK2L.0:TMMK05
#define SRV_SFR_IFxx         0xfffd0    // IF2L
#define SRV_SFR_BIT_TMIFxx   0          // IF2L.0:TMIF05
#define	SRV_SFR2_TSx         0xf01b2    // TS0
#define	SRV_SFR2_TTx         0xf01b4    // TT0
#define	SRV_SFR2_BIT_CHx     5          // 0x0020
#endif

#endif

#define usToTicks(_us)    (( clockCyclesPerMicrosecond()* _us) / 32)     // converts microseconds to tick (assumes prescale of 8)  // 12 Aug 2009
#define ticksToUs(_ticks) (( (unsigned long)_ticks * 32)/ clockCyclesPerMicrosecond() ) // converts from ticks back to microseconds


#define TRIM_DURATION       2                               // compensation ticks to trim adjust for digitalWrite delays // 12 August 2009

//#define NBR_TIMERS        (MAX_SERVOS / SERVOS_PER_TIMER)

static servo_t servos[MAX_SERVOS];                          // static array of servo structures
static volatile int8_t Channel[_Nbr_16timers ];             // counter for the servo being pulsed for each timer (or -1 if refresh interval)

uint8_t ServoCount = 0;                              // the total number of attached servos
uint16_t totalTicks;
uint16_t g_servo_refresh_interval = 20000;

// convenience macros
#define SERVO_INDEX_TO_TIMER(_servo_nbr) ((timer16_Sequence_t)(_servo_nbr / SERVOS_PER_TIMER)) // returns the timer controlling this servo
#define SERVO_INDEX_TO_CHANNEL(_servo_nbr) (_servo_nbr % SERVOS_PER_TIMER)       // returns the index of the servo on this timer
#define SERVO_INDEX(_timer,_channel)  ((_timer*SERVOS_PER_TIMER) + _channel)     // macro to access servo index by timer and channel
#define SERVO(_timer,_channel)  (servos[SERVO_INDEX(_timer,_channel)])            // macro to access servo class by timer and channel

#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in uS for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in uS for this servo

/************ static functions common to all instances ***********************/

static void handle_interrupts(timer16_Sequence_t timer)
{
    if( Channel[timer] == 0 ){
        if( SERVO_INDEX(timer,Channel[timer]) < ServoCount && SERVO(timer,Channel[timer]).Pin.isActive == true ){
            digitalWrite( SERVO(timer,Channel[timer]).Pin.nbr,HIGH); // pulse this channel0 high if activated
        }
    }
    else if( Channel[timer] >= 1 ){
        if( SERVO_INDEX(timer,Channel[timer]) < ServoCount && SERVO(timer,Channel[timer]).Pin.isActive == true ){
            digitalWrite( SERVO(timer,Channel[timer]).Pin.nbr,HIGH); // pulse this channel high if activated
        }
        if( SERVO_INDEX(timer,Channel[timer]-1) < ServoCount && SERVO(timer,Channel[timer]-1).Pin.isActive == true ){
            digitalWrite( SERVO(timer,Channel[timer]-1).Pin.nbr,LOW); // pulse this channel low if activated
        }
    }
    
    Channel[timer]++;    // increment to the next channel
    if( SERVO_INDEX(timer,Channel[timer]) < ServoCount+1 && Channel[timer] < SERVOS_PER_TIMER){
        SRV_TDRxx = SERVO(timer,Channel[timer]).ticks;
        totalTicks += SERVO(timer,Channel[timer]).ticks;
    }  
    else{ 
        // finished all channels so wait for the refresh period to expire before starting over 
        if( totalTicks + 4 < usToTicks(g_servo_refresh_interval)){  // allow a few ticks to ensure the next OCR1A not missed
           SRV_TDRxx = (unsigned int)usToTicks(g_servo_refresh_interval) - totalTicks;
        }
        else{
            SRV_TDRxx = 4;  // at least REFRESH_INTERVAL has elapsed
        }
        Channel[timer] = -1; // this will get incremented at the end of the refresh period to start again at the first channel
        totalTicks = 0;
    }
}

extern "C"
INTERRUPT void servo_interrupt(void)
{
    handle_interrupts(_timer1);
}

static void initISR(timer16_Sequence_t timer)
{  
    if( timer == _timer1){
        if( SRV_TAUxEN == 0 ){
#ifdef WORKAROUND_READ_MODIFY_WRITE
            SBI2( SRV_SFR2_PERx, SRV_SFR2_BIT_TAUxEN );        /* supplies input clock */
#else
            SRV_TAUxEN = 1U;          /* supplies input clock */
#endif
            SRV_TPSx   = TIMER_CLOCK;
        }

#ifdef WORKAROUND_READ_MODIFY_WRITE
        /* Set INTTM04 low priority */
        SBI( SRV_SFR_PR1xx, SRV_SFR_BIT_TMPR1xx );
        SBI( SRV_SFR_PR0xx, SRV_SFR_BIT_TMPR0xx );
        /* Mask channel 04 interrupt */
        CBI( SRV_SFR_MKxx, SRV_SFR_BIT_TMMKxx );    /* enable INTTM04 interrupt */
        CBI( SRV_SFR_IFxx, SRV_SFR_BIT_TMIFxx );    /* clear INTTM04 interrupt flag */
#else
        /* Set INTTM04 low priority */
        SRV_TMPR1xx = 1U;
        SRV_TMPR0xx = 1U;
        /* Mask channel 04 interrupt */
        SRV_TMMKxx  = 0U;    /* enable INTTM04 interrupt */
        SRV_TMIFxx  = 0U;    /* clear INTTM04 interrupt flag */
#endif
        /* Channel 0 used as interval timer */
        SRV_TMRxx   = 0x8000U;
        SRV_TDRxx   = (unsigned int)usToTicks(g_servo_refresh_interval);
        SRV_TSx    |= SRV_CHx;     /* operation is enabled (start trigger is generated) */

        delay(1);
        
        Channel[timer] = -1;
        handle_interrupts(_timer1);   /* TDR0x setting */

    }
}

static void finISR(timer16_Sequence_t timer)
{
    if( timer == _timer1){
#ifdef WORKAROUND_READ_MODIFY_WRITE
        //disable use of the given timer
        SBI( SRV_SFR_MKxx, SRV_SFR_BIT_TMMKxx );    /* disable INTTM01 interrupt */
        SRV_TTx   |= SRV_CHx;     /* operation is stopped (stop trigger is generated) */
#else
        //disable use of the given timer
        SRV_TMMKxx = 1U;        /* disable INTTM01 interrupt */
        SRV_TTx   |= SRV_CHx;     /* operation is stopped (stop trigger is generated) */
#endif
    }
}

static boolean isTimerActive(timer16_Sequence_t timer)
{
  // returns true if any servo is active on this timer
    for(uint8_t channel=0; channel < SERVOS_PER_TIMER; channel++){
        if(SERVO(timer,channel).Pin.isActive == true)
            return true;
    }
    return false;
}

/****************** end of static functions ******************************/

Servo::Servo()
{
    if( ServoCount < MAX_SERVOS){
        this->servoIndex = ServoCount++;                    // assign a servo index to this instance
        servos[this->servoIndex].ticks = usToTicks(DEFAULT_PULSE_WIDTH);   // store default values  - 12 Aug 2009
    }
    else
        this->servoIndex = INVALID_SERVO ;  // too many servos
}

uint8_t Servo::attach(int pin)
{
    return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min, int max)
{
    if(this->servoIndex < MAX_SERVOS ){
        pinMode( pin, OUTPUT);                                  // set servo pin to output
        servos[this->servoIndex].Pin.nbr = pin;
        // todo min/max check: abs(min - MIN_PULSE_WIDTH) /4 < 128 
        this->min = (MIN_PULSE_WIDTH - min)/4;         // resolution of min/max is 4 uS
        this->max = (MAX_PULSE_WIDTH - max)/4;
        // initialize the timer if it has not already been initialized 
        timer16_Sequence_t timer = SERVO_INDEX_TO_TIMER(servoIndex);
        if(isTimerActive(timer) == false)
            initISR(timer);
        servos[this->servoIndex].Pin.isActive = true;  // this must be set after the check for isTimerActive
    } 
    return this->servoIndex;
}

void Servo::detach()
{
    servos[this->servoIndex].Pin.isActive = false;
    timer16_Sequence_t timer = SERVO_INDEX_TO_TIMER(servoIndex);
    if(isTimerActive(timer) == false) {
        finISR(timer);
    }
}

void Servo::write(int value)
{  
    if(value < MIN_PULSE_WIDTH)  
    {  // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
        if(value < 0) value = 0;
        if(value > 180) value = 180;
        value = map(value, 0, 180, SERVO_MIN(), SERVO_MAX());
    }
    this->writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value)
{
    // calculate and store the values for the given channel
    byte channel = this->servoIndex;
    if( (channel < MAX_SERVOS) )            // ensure channel is valid
    {
    	if( value < 0 )          // ensure pulse width is valid
            value = 0;
        else if( value >= g_servo_refresh_interval )
            value = g_servo_refresh_interval - 1;
        
        value = value - TRIM_DURATION;
        value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead - 12 Aug 2009

#if defined(REL_GR_KURUMI)
        servos[channel].ticks = value;
#else
        noInterrupts();
        servos[channel].ticks = value;
        interrupts();
#endif
    } 
}

int Servo::read() // return the value as degrees
{
    return map( this->readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}

int Servo::readMicroseconds()
{
    unsigned int pulsewidth;
    if( this->servoIndex != INVALID_SERVO )
        pulsewidth = ticksToUs(servos[this->servoIndex].ticks)  + TRIM_DURATION;   // 12 aug 2009
    else
        pulsewidth  = 0;
    return pulsewidth;
}

bool Servo::attached()
{
    return servos[this->servoIndex].Pin.isActive;
}

boolean setServoRefreshInterval(uint16_t time){
	if( MIN_REFRESH_INTERVAL <= time && time <= MAX_REFRESH_INTERVAL){
		g_servo_refresh_interval = time;
		return true;
	}
	return false;
}
