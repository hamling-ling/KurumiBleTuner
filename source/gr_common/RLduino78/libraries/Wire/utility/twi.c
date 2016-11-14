/*
  twi.c - TWI/I2C library for Wiring & Arduino
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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 1 March 2013 by masahiko.nagata.cj@renesas.com
*/

#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include "iodefine_ext.h"
#include "iodefine.h"
#include "utiltwi.h"
#include "RLduino78.h"
#include "RLduino78_mcu_depend.h"
#include "pins_arduino.h"
#include "twi.h"

#ifdef WORKAROUND_READ_MODIFY_WRITE
#define WIR_SFR2_IICCTLx0    0xf0230    // IICCTL00
#define WIR_SFR2_BIT_SPTx    0          // IICCTL00.0:SPT0
#define WIR_SFR2_BIT_STTx    1          // IICCTL00.1:STT0
#define WIR_SFR2_BIT_ACKEx   2          // IICCTL00.2:ACKE0
#define WIR_SFR2_BIT_WTIMx   3          // IICCTL00.3:WTIM0
#define WIR_SFR2_BIT_SPIEx   4          // IICCTL00.4:SPIE0
#define WIR_SFR2_BIT_WRELx   5          // IICCTL00.5:WREL0
#define WIR_SFR2_BIT_IICEx   7          // IICCTL00.7:IICE0

#define WIR_SFR_IFx          0xfffe2    // IF1L
#define WIR_SFR_BIT_IICAIFx  3          // IF1L.3:IICAIF0

#define WIR_SFR_MKx          0xfffe6    // MK1L
#define WIR_SFR_BIT_IICAMKx  3          // MK1L.3:IICAMK0

#define WIR_SFR2_PERx        0xf00f0    // PER0
#define WIR_SFR2_BIT_IICAxEN 4          // PER0.4:IICA0EN

#define WIR_SFR_PR1xx        0xfffee    // PR11L
#define WIR_SFR_BIT_IICAPR1x 3          // PR11L.3:IICAPR10
#define WIR_SFR_PR0xx        0xfffea    // PR01L
#define WIR_SFR_BIT_IICAPR0x 3          // PR01L.3:IICAPR00

#define WIR_SFR2_IICCTLx1    0xf0231    // IICCTL01
#define WIR_SFR2_BIT_PRSx    0          // IICCTL01.0:PRS0		//	  IICCTL01.iicctl01 |= 0x01;
#define WIR_SFR2_BIT_SMCx    3          // IICCTL01.3:SMC0

#define WIR_SFR_IICFx        0xfff52    // IICF0
#define WIR_SFR_BIT_IICRSVx  0          // IICF0.0:IICRSV0
#define WIR_SFR_BIT_STCENx   1          // IICF0.1:STCEN0

#define WIR_SFR_P6x          0xfff06    // P6
#define WIR_SFR_BIT_P60      0          // P6.0:P60
#define WIR_SFR_BIT_P61      1          // P6.1:P61

#define WIR_SFR_PM6x         0xfff26    // PM6
#define WIR_SFR_BIT_PM60     0          // PM6.0:PM60
#define WIR_SFR_BIT_PM61     1          // PM6.1:PM61

#define WIR_SFR_PM0x         0xfff20    // PM0
#define WIR_SFR_BIT_PM00     0          // PM0.0:PM00
#define WIR_SFR_BIT_PM01     1          // PM0.1:PM01
#endif

static volatile uint8_t twi_state;
static volatile uint8_t twi_slarw;
static volatile uint8_t twi_sendStop;           // should the transaction end with a stop
static volatile uint8_t twi_inRepStart;         // in the middle of a repeated start

static void (*twi_onSlaveTransmit)(void);
static void (*twi_onSlaveReceive)(uint8_t*, int);

static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndex;
static volatile uint8_t twi_masterBufferLength;

static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static uint8_t twi_rxBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twi_error;

static uint8_t i2cStop = 0;
static uint8_t slaveAddress = 0;

static uint8_t i2cSlaveReciver = 0;

void I2C_Start(void);

/* 
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
void twi_init(void)
{
  // initialize state
  twi_state = TWI_READY;
  twi_sendStop = true;      // default value
  twi_inRepStart = false;
  
  // activate internal pullups for twi.
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

  // initialize I2C H/W
  I2C_Start();
}

/* 
 * Function twi_slaveInit
 * Desc     sets slave address and enables interrupt
 * Input    none
 * Output   none
 */
void twi_setAddress(uint8_t address)
{
  // set twi slave address (skip over TWGCE bit)
  slaveAddress = address << 1;
}

/* 
 * Function twi_readFrom
 * Desc     attempts to become twi bus master and read a
 *          series of bytes from a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes to read into array
 *          sendStop: Boolean indicating whether to send a stop at the end
 * Output   number of bytes read
 */
uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop)
{
  uint8_t i;

  if (true != twi_inRepStart) {
	  I2C_Start();
  }
  i2cStop = 0;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return 0;
  }

  // wait until twi is ready, become master receiver
  while(TWI_READY != twi_state){
    continue;
  }
  twi_state = TWI_MRX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length-1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.

  // build sla+w, slave device address + w bit
  twi_slarw = TW_READ;
  twi_slarw |= address << 1;

#ifdef WORKAROUND_READ_MODIFY_WRITE
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
  CBI( WIR_SFR_IFx, WIR_SFR_BIT_IICAIFx );
  CBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx );
#else
  IICE0 = 1;
  ACKE0 = 1;
  IICAIF0 = 0;
  IICAMK0 = 0;
#endif
  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;         // remember, we're dealing with an ASYNC ISR
  }
  else
    // send start condition
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_STTx );
#else
    STT0 = 1;
#endif

  IICA0.iica0 = twi_slarw;

  // wait for read operation to complete
  while(TWI_MRX == twi_state){
    continue;
  }

  if (twi_masterBufferIndex < length)
    length = twi_masterBufferIndex +1;

  // copy twi buffer to data
  for(i = 0; i < length; ++i){
    data[i] = twi_masterBuffer[i];
  }
    
  return length;
}

/* 
 * Function twi_writeTo
 * Desc     attempts to become twi bus master and write a
 *          series of bytes to a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 *          sendStop: boolean indicating whether or not to send a stop at the end
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 */
uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait, uint8_t sendStop)
{
  uint8_t i;

  if (true != twi_inRepStart) {
    I2C_Start();
  }

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return 1;
  }

  // wait until twi is ready, become master transmitter
  while(TWI_READY != twi_state){
    continue;
  }
  twi_state = TWI_MTX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterBufferIndex = 0;
  twi_masterBufferLength = length;
  
  // copy data to twi buffer
  for(i = 0; i < length; ++i){
    twi_masterBuffer[i] = data[i];
  }
  
  // build sla+w, slave device address + w bit
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;
  
  // if we're in a repeated start, then we've already sent the START
  // in the ISR. Don't do it again.
  //
#ifdef WORKAROUND_READ_MODIFY_WRITE
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
  CBI( WIR_SFR_IFx, WIR_SFR_BIT_IICAIFx );
  CBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx );
#else
  IICE0 = 1;
  ACKE0 = 1;
  IICAIF0 = 0;
  IICAMK0 = 0;
#endif
  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;         // remember, we're dealing with an ASYNC ISR
  }
  else{
  // send start condition
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_STTx );
#else
    STT0 = 1;
#endif
  }
  IICA0.iica0 = twi_slarw;

  // wait for write operation to complete
  while(wait && (TWI_MTX == twi_state)){
    continue;
  }
  
  if (twi_error == 0xFF)
    return 0;   // success
  else if (twi_error == TW_MT_SLA_NACK)
    return 2;   // error: address send, nack received
  else if (twi_error == TW_MT_DATA_NACK)
    return 3;   // error: data send, nack received
  else
    return 4;   // other twi error
}

/* 
 * Function twi_transmit
 * Desc     fills slave tx buffer with data
 *          must be called in slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
uint8_t twi_transmit(const uint8_t* data, uint8_t length)
{
  uint8_t i;

  // ensure data will fit into buffer
  if(TWI_BUFFER_LENGTH < length){
    return 1;
  }
  
  // ensure we are currently a slave transmitter
  if(TWI_STX != twi_state){
    return 2;
  }
  
  // set length and copy data into tx buffer
  twi_txBufferLength = length;
  for(i = 0; i < length; ++i){
    twi_txBuffer[i] = data[i];
  }
  
  return 0;
}

/* 
 * Function twi_attachSlaveRxEvent
 * Desc     sets function called before a slave read operation
 * Input    function: callback function to use
 * Output   none
 */
void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) )
{
  twi_onSlaveReceive = function;
}

/* 
 * Function twi_attachSlaveTxEvent
 * Desc     sets function called before a slave write operation
 * Input    function: callback function to use
 * Output   none
 */
void twi_attachSlaveTxEvent( void (*function)(void) )
{
  twi_onSlaveTransmit = function;
}

/* 
 * Function twi_reply
 * Desc     sends byte or readys receive line
 * Input    ack: byte indicating to ack or to nack
 * Output   none
 */
/*void twi_reply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
  if(ack){
    IICE0 = 1;
    ACKE0 = 1;
    IICAIF0 = 0;
    IICAMK0 = 0;
  }else{
    IICE0 = 1;
    ACKE0 = 0;
    IICAIF0 = 0;
    IICAMK0 = 0;
  }
}*/

/* 
 * Function twi_stop
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void twi_stop(void)
{
  // send stop condition
#ifdef WORKAROUND_READ_MODIFY_WRITE
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
  CBI( WIR_SFR_IFx, WIR_SFR_BIT_IICAIFx );
  SBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_SPTx );
#else
  IICE0 = 1;
  ACKE0 = 1;
  IICAIF0 = 0;
  IICAMK0 = 1;
  SPT0 = 1;
#endif

  // wait for stop condition to be exectued on bus
  // TWINT is not set after a stop condition!
  while( !SPD0 ){
    continue;
  }

  // update twi state
  twi_state = TWI_READY;
}

/* 
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twi_releaseBus(void)
{
  // release bus
#ifdef WORKAROUND_READ_MODIFY_WRITE
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
  CBI( WIR_SFR_IFx, WIR_SFR_BIT_IICAIFx );
  CBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx );
#else
  IICE0 = 1;
  ACKE0 = 1;
  IICAIF0 = 0;
  IICAMK0 = 0;
#endif

  // update twi state
  twi_state = TWI_READY;
}

INTERRUPT void iica0_interrupt(void)
{
  uint8_t tw_status;

  if( MSTS0 == 1 ){
    if( TRC0 == 1 ){    // Master Transmitter
      if( ALD0 == 1 ){
        tw_status = TW_MT_ARB_LOST;
      }
      else if( ACKD0 == 1 ){
        if( STD0 == 1 ){
          tw_status = TW_MT_SLA_ACK;
        }
        else{
          tw_status = TW_MT_DATA_ACK;
        }
      }
      else{
        if( STD0 == 1 ){
          tw_status = TW_MT_SLA_NACK;
        }
        else{
          tw_status = TW_MT_DATA_NACK;
        }
      }
    }
    else{               // Master Receiver
      if( ACKD0 == 1 ){
        if( STD0 == 1 ){
          tw_status = TW_MR_SLA_ACK;
        }
        else{
          tw_status = TW_MR_DATA_ACK;
        }
      }
      else{
        if( STD0 == 1 ){
          tw_status = TW_MR_SLA_NACK;
        }
        else{
          tw_status = TW_MR_DATA_ACK;
        }
      }
    }
  }
  else{
    if( COI0 == 1 ){
      if( TRC0 == 1 ){    // Slave Transmitter
        if( ALD0 == 1 ){
          tw_status = TW_ST_ARB_LOST_SLA_ACK;
        }
        else if( ACKD0 == 1 ){
          if( STD0 == 1 ){
            tw_status = TW_ST_SLA_ACK;
          }
          else{
            tw_status = TW_ST_DATA_ACK;
          }
        }
        else{
          tw_status = TW_ST_DATA_NACK;
        }
      }
      else{               // Slave Receiver
        if( ALD0 == 1 ){
          tw_status = TW_SR_ARB_LOST_SLA_ACK;
        }
        else if( ACKD0 == 1 ){
          if( STD0 == 1 ){
            tw_status = TW_SR_SLA_ACK;
          }
          else{
            tw_status = TW_SR_DATA_ACK;
          }
        }
        else{
          tw_status = TW_SR_DATA_NACK;
        }
      }
    }
    else{
      if( SPD0 == 1 ){
    	if( i2cSlaveReciver == 1 ){
          tw_status = TW_SR_STOP;
    	}
    	else{
          tw_status = TW_ST_LAST_DATA;
    	}
      }
      else{
      	tw_status = TW_NO_INFO;
      }
    }
  }

  switch(tw_status){
/* RL78ではTW_STARTとTW_REP_START時にHW的に割り込みが発生しないため処理を削除
  // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
      // copy device address and r/w bit to output register and ack
      IICA0.iica0 = twi_slarw;
      twi_reply(1);
      break;
*/
    // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
    case TW_MT_DATA_ACK: // slave receiver acked data
    {volatile int w; for (w = 0; w < 5; w++);} //wait for i2c specification
    // if there is data to send, send it, otherwise stop
      if(twi_masterBufferIndex < twi_masterBufferLength){
        // copy data to output register and ack
      	IICA0.iica0 = twi_masterBuffer[twi_masterBufferIndex++];
      }else{
        if (twi_sendStop)
          twi_stop();
        else {
          twi_inRepStart = true;    // we're gonna send the START
          // don't enable the interrupt. We'll generate the start, but we
          // avoid handling the interrupt until we're in the next transaction,
          // at the point where we would normally issue the start.
#ifdef WORKAROUND_READ_MODIFY_WRITE
          SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_STTx );
#else
          STT0 = 1;
#endif
          twi_state = TWI_READY;
        }
      }
      break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      twi_error = TW_MT_SLA_NACK;
      twi_stop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      twi_error = TW_MT_DATA_NACK;
      twi_stop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      twi_error = TW_MT_ARB_LOST;
      twi_releaseBus();
      break;

    // Master Receiver
    case TW_MR_DATA_ACK:
      if( i2cStop == 0 ){
        // put final byte into buffer
        twi_masterBuffer[twi_masterBufferIndex] = IICA0.iica0;
        if(twi_masterBufferIndex < twi_masterBufferLength){
#ifdef WORKAROUND_READ_MODIFY_WRITE
          SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
    	  WREL0 = 1;
#endif
    	  twi_masterBufferIndex++;
        }else{
#ifdef WORKAROUND_READ_MODIFY_WRITE
          CBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
          SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WTIMx );
          SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
    	  ACKE0 = 0;
    	  WTIM0 = 1;
    	  WREL0 = 1;
#endif
    	  i2cStop = 1;
        }
      }
      else{
        i2cStop = 0;
        if (twi_sendStop)
          twi_stop();
        else {
          twi_inRepStart = true;    // we're gonna send the START
          // don't enable the interrupt. We'll generate the start, but we
          // avoid handling the interrupt until we're in the next transaction,
          // at the point where we would normally issue the start.
#ifdef WORKAROUND_READ_MODIFY_WRITE
          SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_STTx );
#else
          STT0 = 1;
#endif
          twi_state = TWI_READY;
        }
      }
      break;
    case TW_MR_SLA_ACK:  // address sent, ack received
      // ack if more bytes are expected, otherwise nack
#ifdef WORKAROUND_READ_MODIFY_WRITE
        SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
        CBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WTIMx );
        SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
    	ACKE0 = 1;
    	WTIM0 = 0;
    	WREL0 = 1;
#endif
      break;
    case TW_MR_DATA_NACK: // data received, nack sent
    case TW_MR_SLA_NACK: // address sent, nack received
      twi_stop();
      break;

    // Slave Receiver
    case TW_SR_SLA_ACK:   // addressed, returned ack
    case TW_SR_GCALL_ACK: // addressed generally, returned ack
    case TW_SR_ARB_LOST_SLA_ACK:   // lost arbitration, returned ack
    case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
      // enter slave receiver mode
      twi_state = TWI_SRX;
      // indicate that rx buffer can be overwritten and ack
      twi_rxBufferIndex = 0;
#ifdef WORKAROUND_READ_MODIFY_WRITE
      SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
      WREL0 = 1U;
#endif
      i2cSlaveReciver = 1;
      break;
    case TW_SR_DATA_ACK:       // data received, returned ack
    case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
      // if there is still room in the rx buffer
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        // put byte in buffer and ack
        twi_rxBuffer[twi_rxBufferIndex++] = IICA0.iica0;
      }
#ifdef WORKAROUND_READ_MODIFY_WRITE
      SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
      WREL0 = 1U;
#endif
      break;
    case TW_SR_STOP: // stop or repeated start condition received
      // put a null char after data if there's room
      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){
        twi_rxBuffer[twi_rxBufferIndex] = '\0';
      }
      // callback to user defined callback
      twi_onSlaveReceive(twi_rxBuffer, twi_rxBufferIndex);
      // since we submit rx buffer to "wire" library, we can reset it
      twi_rxBufferIndex = 0;
      break;
    case TW_SR_DATA_NACK:       // data received, returned nack
    case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
      break;
    
    // Slave Transmitter
    case TW_ST_SLA_ACK:          // addressed, returned ack
    case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
      // enter slave transmitter mode
      twi_state = TWI_STX;
      // ready the tx buffer index for iteration
      twi_txBufferIndex = 0;
      // set tx buffer length to be zero, to verify if user changes it
      twi_txBufferLength = 0;
      // request for txBuffer to be filled and length to be set
      // note: user must call twi_transmit(bytes, length) to do this
      twi_onSlaveTransmit();
      // if they didn't change buffer & length, initialize it
      if(0 == twi_txBufferLength){
        twi_txBufferLength = 1;
        twi_txBuffer[0] = 0x00;
      }
      i2cSlaveReciver = 0;
      // transmit first byte from buffer, fall
    case TW_ST_DATA_ACK: // byte sent, ack returned
      // copy data to output register
      IICA0.iica0 = twi_txBuffer[twi_txBufferIndex++];
      break;
    case TW_ST_DATA_NACK: // received nack, we are done
#ifdef WORKAROUND_READ_MODIFY_WRITE
      SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WRELx );
#else
      WREL0 = 1U;
#endif
      break;
    case TW_ST_LAST_DATA: // received ack, but we are done already!
      // leave slave receiver state
      twi_state = TWI_READY;
      break;

    // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      twi_error = TW_BUS_ERROR;
      twi_stop();
      break;
  }
}

void I2C_Start(void)
{
#if ( UART1_CHANNEL == 1 )
  /* RxD1,TxD Input pin */
  PM0.pm0 |= 0x03U;
#endif
#ifdef WORKAROUND_READ_MODIFY_WRITE
  PM6.pm6 |= 0x03U;
  SBI2( WIR_SFR2_PERx, WIR_SFR2_BIT_IICAxEN ); /* supply IICA0 clock */
  CBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx ); /* disable IICA0 operation */
  SBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx ); /* disable INTIICA0 interrupt */
  CBI( WIR_SFR_IFx, WIR_SFR_BIT_IICAIFx ); /* clear INTIICA0 interrupt flag */
  /* Set INTIICA0 low priority */
  SBI( WIR_SFR_PR1xx, WIR_SFR_BIT_IICAPR1x );
  SBI( WIR_SFR_PR0xx, WIR_SFR_BIT_IICAPR0x );
  /* Set SCLA0, SDAA0 pin */
  CBI2( WIR_SFR2_IICCTLx1, WIR_SFR2_BIT_SMCx );
  IICWL0.iicwl0 = (configCPU_CLOCK_HZ/2*0.47)/TWI_FREQ;
  IICWH0.iicwh0 = (configCPU_CLOCK_HZ/2*0.53)/TWI_FREQ;
  SVA0.sva0 = slaveAddress;
  IICCTL01.iicctl01 |= 0x01;
  SBI( WIR_SFR_IICFx, WIR_SFR_BIT_STCENx );
  SBI( WIR_SFR_IICFx, WIR_SFR_BIT_IICRSVx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_SPIEx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_WTIMx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_ACKEx );
  CBI( WIR_SFR_MKx, WIR_SFR_BIT_IICAMKx );
  SBI2( WIR_SFR2_IICCTLx0, WIR_SFR2_BIT_IICEx );
#else
  /* Set SCLA0, SDAA0 pin */
  PM6.pm6 |= 0x03U;
  IICA0EN = 1U; /* supply IICA0 clock */
  IICE0 = 0U; /* disable IICA0 operation */
  IICAMK0 = 1U; /* disable INTIICA0 interrupt */
  IICAIF0 = 0U; /* clear INTIICA0 interrupt flag */
  /* Set INTIICA0 low priority */
  IICAPR10 = 1U;
  IICAPR00 = 1U;
  SMC0 = 0U;
  IICWL0.iicwl0 = (configCPU_CLOCK_HZ/2*0.47)/TWI_FREQ;
  IICWH0.iicwh0 = (configCPU_CLOCK_HZ/2*0.53)/TWI_FREQ;
  SVA0.sva0 = slaveAddress;
  IICCTL01.iicctl01 |= 0x01;
  STCEN0 = 1U;
  IICRSV0 = 1U;
  SPIE0 = 1U;
  WTIM0 = 1U;
  ACKE0 = 1U;
  IICAMK0 = 0U;
  IICE0 = 1U;
#endif
  /* Set SCLA0, SDAA0 pin */
  P6.p6 &= 0xFCU;
  PM6.pm6 &= 0xFCU;

}

