/*
 * Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com
 * MAX3421E USB host controller support
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Modified 28 February 2013 by tomofumi.kami.pc@renesas.com
 */

/* MAX3421E USB host controller support */

#include "Max3421e.h"
#include "RLduino78_mcu_depend.h"

static byte vbusState;

/* Functions    */

void MAX3421E::setRST(uint8_t val)
{
	if (val == LOW)
		digitalWrite( RST_PIN, LOW );
	else
		digitalWrite( RST_PIN, HIGH );
}

uint8_t MAX3421E::readINT(void)
{
	return digitalRead( INT_PIN );
}

uint8_t MAX3421E::readGPX(void)
{
	// return digitalRead( GPX_PIN );
	return LOW;
}


/* Constructor */
MAX3421E::MAX3421E( void )
{
    // initialize SPI pins
    pinMode( SCK_PIN, OUTPUT );
    pinMode( MISO_PIN, INPUT );
    pinMode( MOSI_PIN, OUTPUT );
    pinMode( SS_PIN, OUTPUT );
    digitalWrite( SCK_PIN, HIGH );
    digitalWrite( MOSI_PIN, HIGH );
    digitalWrite( SS_PIN, HIGH );

    // initialize pins
    pinMode( INT_PIN, INPUT );
    pinMode( GPX_PIN, INPUT );
    pinMode( RST_PIN, OUTPUT );
    setRST( HIGH );

    if (SPI_SAUxEN == 0) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
        SBI2(SFR2_PER0, SFR2_BIT_SAUxEN); /* supply SAUx clock */
#else
        SPI_SAUxEN = 1U;          /* supply SAUx clock */
#endif
        NOP();
        NOP();
        NOP();
        NOP();
        SPI_SPSx = 0x0001U;
    }

#ifdef WORKAROUND_READ_MODIFY_WRITE
    SPI_STx      |= SPI_CHx;      /* disable CSIxx */
    CBI(SFR_IFxx,  SFR_BIT_CSIIFxx);  /* clear INTCSIxx interrupt flag */
    SBI(SFR_MKxx,  SFR_BIT_CSIMKxx);  /* disable INTCSIxx interrupt */
    CBI(SFR_PR1xx, SFR_BIT_CSIPR1xx); /* set INTCSIxx high priority */
    CBI(SFR_PR0xx, SFR_BIT_CSIPR0xx);
#else
    SPI_STx      |= SPI_CHx;      /* disable CSIxx */
    SPI_CSIIFxx   = 0U;           /* clear INTCSIxx interrupt flag */
    SPI_CSIMKxx   = 1U;           /* disable INTCSIxx interrupt */
    SPI_CSIPR1xx  = 0U;           /* set INTCSIxx high priority */
    SPI_CSIPR0xx  = 0U;
#endif
    SPI_SIRxx     = 0x0007U;      /* clear error flag */
    SPI_SMRxx     = 0x0020U;
    SPI_SCRxx     = 0xF007U;
    SPI_SDRxx     = 0x0200U;

    SPI_SOx      |= SPI_CHx << 8; /* CSIxx clock initial level */
    SPI_SOx      &= ~SPI_CHx;     /* CSIxx SO initial level */
    SPI_SOEx     |= SPI_CHx;      /* enable CSIxx output */
    SPI_SSx      |= SPI_CHx;      /* enable CSIxx */
}

byte MAX3421E::getVbusState( void )
{ 
    return( vbusState );
}
/* initialization */
//void MAX3421E::init()
//{
//    /* setup pins */
//    pinMode( INT_PIN, INPUT);
//    pinMode( GPX_PIN, INPUT );
//    pinMode( SS_PIN, OUTPUT );
//    //pinMode( BPNT_0, OUTPUT );
//    //pinMode( BPNT_1, OUTPUT );
//    //digitalWrite( BPNT_0, LOW );
//    //digitalWrite( BPNT_1, LOW );
//    Deselect_MAX3421E;
//    pinMode( RST_PIN, OUTPUT );
//    digitalWrite( RST_PIN, HIGH );  //release MAX3421E from reset
//}
//byte MAX3421E::getVbusState( void )
//{
//    return( vbusState );
//}
//void MAX3421E::toggle( byte pin )
//{
//    digitalWrite( pin, HIGH );
//    digitalWrite( pin, LOW );
//}
/* Single host register write   */
void MAX3421E::regWr( byte reg, byte val)
{
    digitalWrite( SS_PIN, LOW );
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SPI_SIOxx = ( reg | 0x02 );
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
    SPI_SIOxx = val;
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
#else
    SPI_SIOxx = ( reg | 0x02 );
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
    SPI_SIOxx = val;
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
#endif
    digitalWrite( SS_PIN, HIGH );
    return;
}
/* multiple-byte write */
/* returns a pointer to a memory position after last written */
char * MAX3421E::bytesWr( byte reg, byte nbytes, char * data )
{
    digitalWrite( SS_PIN, LOW );
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SPI_SIOxx = ( reg | 0x02 );
    while( nbytes-- ) {
        while(!SPI_CSIIFxx);         //check if previous byte was sent
        CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
        SPI_SIOxx = ( *data ); // send next data byte
        data++;                  // advance data pointer
    }
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
#else
    SPI_SIOxx = ( reg | 0x02 );
    while( nbytes-- ) {
        while(!SPI_CSIIFxx);         //check if previous byte was sent
        SPI_CSIIFxx = 0;
        SPI_SIOxx = ( *data ); // send next data byte
        data++;                  // advance data pointer
    }
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
#endif
    digitalWrite( SS_PIN, HIGH );
    return( data );
}
/* GPIO write. GPIO byte is split between 2 registers, so two writes are needed to write one byte */
/* GPOUT bits are in the low nibble. 0-3 in IOPINS1, 4-7 in IOPINS2 */
/* upper 4 bits of IOPINS1, IOPINS2 are read-only, so no masking is necessary */
void MAX3421E::gpioWr( byte val )
{
    regWr( rIOPINS1, val );
    val = val >>4;
    regWr( rIOPINS2, val );
    
    return;     
}
/* Single host register read        */
byte MAX3421E::regRd( byte reg )    
{
    digitalWrite( SS_PIN, LOW );
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SPI_SIOxx = reg;
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
    SPI_SIOxx = 0; //send empty byte
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
#else
    SPI_SIOxx = reg;
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
    SPI_SIOxx = 0; //send empty byte
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
#endif
    digitalWrite( SS_PIN, HIGH );
    return(SPI_SIOxx);
}
/* multiple-bytes register read                             */
/* returns a pointer to a memory position after last read   */
char * MAX3421E::bytesRd ( byte reg, byte nbytes, char  * data )
{
    digitalWrite( SS_PIN, LOW );
#ifdef WORKAROUND_READ_MODIFY_WRITE
    SPI_SIOxx = reg;
    while(!SPI_CSIIFxx);
    CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
    while( nbytes ) {
        SPI_SIOxx = 0; //send empty byte
        nbytes--;
        while(!SPI_CSIIFxx);
        CBI(SFR_IFxx, SFR_BIT_CSIIFxx);
        *data = SPI_SIOxx;
        data++;
    }
#else
    SPI_SIOxx = reg;
    while(!SPI_CSIIFxx);
    SPI_CSIIFxx = 0;
    while( nbytes ) {
        SPI_SIOxx = 0; //send empty byte
        nbytes--;
        while(!SPI_CSIIFxx);
        SPI_CSIIFxx = 0;
        *data = SPI_SIOxx;
        data++;
    }
#endif
    digitalWrite( SS_PIN, HIGH );
    return( data );
}
/* GPIO read. See gpioWr for explanation */
/* GPIN pins are in high nibbles of IOPINS1, IOPINS2    */
byte MAX3421E::gpioRd( void )
{
    byte tmpbyte = 0;
    tmpbyte = regRd( rIOPINS2 );            //pins 4-7
    tmpbyte &= 0xf0;                        //clean lower nibble
    tmpbyte |= ( regRd( rIOPINS1 ) >>4 ) ;  //shift low bits and OR with upper from previous operation. Upper nibble zeroes during shift, at least with this compiler
    return( tmpbyte );
}
/* reset MAX3421E using chip reset bit. SPI configuration is not affected   */
boolean MAX3421E::reset()
{
    unsigned short tmp = 0;
    regWr( rUSBCTL, bmCHIPRES );                        //Chip reset. This stops the oscillator
    regWr( rUSBCTL, 0x00 );                             //Remove the reset
    while(!(regRd( rUSBIRQ ) & bmOSCOKIRQ )) {          //wait until the PLL is stable
        tmp++;                                          //timeout after 256 attempts
        if( tmp == 0 ) {
            return( false );
        }
    }
    return( true );
}
/* turn USB power on/off                                                */
/* does nothing, returns TRUE. Left for compatibility with old sketches               */
/* will be deleted eventually                                           */
///* ON pin of VBUS switch (MAX4793 or similar) is connected to GPOUT7    */
///* OVERLOAD pin of Vbus switch is connected to GPIN7                    */
///* OVERLOAD state low. NO OVERLOAD or VBUS OFF state high.              */
boolean MAX3421E::vbusPwr ( boolean action )
{
//  byte tmp;
//    tmp = regRd( rIOPINS2 );                //copy of IOPINS2
//    if( action ) {                          //turn on by setting GPOUT7
//        tmp |= bmGPOUT7;
//    }
//    else {                                  //turn off by clearing GPOUT7
//        tmp &= ~bmGPOUT7;
//    }
//    regWr( rIOPINS2, tmp );                 //send GPOUT7
//    if( action ) {
//        delay( 60 );
//    }
//    if (( regRd( rIOPINS2 ) & bmGPIN7 ) == 0 ) {     // check if overload is present. MAX4793 /FLAG ( pin 4 ) goes low if overload
//        return( false );
//    }                      
    return( true );                                             // power on/off successful                       
}
/* probe bus to determine device presense and speed and switch host to this speed */
void MAX3421E::busprobe( void )
{
    byte bus_sample;
    bus_sample = regRd( rHRSL );            //Get J,K status
    bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte
    switch( bus_sample ) {                          //start full-speed or low-speed host 
        case( bmJSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            else {
                regWr( rMODE, MODE_LS_HOST);        //start low-speed host
                vbusState = LSHOST;
            }
            break;
        case( bmKSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_LS_HOST );       //start low-speed host
                vbusState = LSHOST;
            }
            else {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            break;
        case( bmSE1 ):              //illegal state
            vbusState = MAXSE1;
            break;
        case( bmSE0 ):              //disconnected state
		regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ);
            vbusState = MAXSE0;
            break;
        }//end switch( bus_sample )
}
/* MAX3421E initialization after power-on   */
void MAX3421E::powerOn()
{
    /* Configure full-duplex SPI, interrupt pulse   */
    regWr( rPINCTL,( bmFDUPSPI + bmINTLEVEL + bmGPXB ));    //Full-duplex SPI, level interrupt, GPX
    if( reset() == false ) {                                //stop/start the oscillator
        Serial.println("Error: OSCOKIRQ failed to assert");
    }

    /* configure host operation */
    regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, Host, Separate GPIN IRQ on GPX
    regWr( rHIEN, bmCONDETIE|bmFRAMEIE );                                             //connection detection
    /* check if device is connected */
    regWr( rHCTL,bmSAMPLEBUS );                                             // sample USB bus
    while(!(regRd( rHCTL ) & bmSAMPLEBUS ));                                //wait for sample operation to finish
    busprobe();                                                             //check if anything is connected
    regWr( rHIRQ, bmCONDETIRQ );                                            //clear connection detect interrupt                 
    regWr( rCPUCTL, 0x01 );                                                 //enable interrupt pin
}
/* MAX3421 state change task and interrupt handler */
byte MAX3421E::Task( void )
{
    byte rcode = 0;
    byte pinvalue;
    //Serial.print("Vbus state: ");
    //Serial.println( vbusState, HEX );
    pinvalue = readINT();
    if( pinvalue  == LOW ) {
        rcode = IntHandler();
    }
    pinvalue = readGPX();
    if( pinvalue == LOW ) {
        GpxHandler();
    }
//    usbSM();                                //USB state machine                            
    return( rcode );   
}   
byte MAX3421E::IntHandler()
{
    byte HIRQ;
    byte HIRQ_sendback = 0x00;
    HIRQ = regRd( rHIRQ );                  //determine interrupt source
    //if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
    //    HIRQ_sendback |= bmFRAMEIRQ;
    //}//end FRAMEIRQ handling
    if( HIRQ & bmCONDETIRQ ) {
        busprobe();
        HIRQ_sendback |= bmCONDETIRQ;
    }
    /* End HIRQ interrupts handling, clear serviced IRQs    */
    regWr( rHIRQ, HIRQ_sendback );
    return( HIRQ_sendback );
}
byte MAX3421E::GpxHandler()
{
    byte GPINIRQ = regRd( rGPINIRQ );          //read GPIN IRQ register
//    if( GPINIRQ & bmGPINIRQ7 ) {            //vbus overload
//        vbusPwr( OFF );                     //attempt powercycle
//        delay( 1000 );
//        vbusPwr( ON );
//        regWr( rGPINIRQ, bmGPINIRQ7 );
//    }       
    return( GPINIRQ );
}

//void MAX3421E::usbSM( void )                //USB state machine
//{
//    
//
//}
