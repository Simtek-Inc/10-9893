//========================================================================================//
//========================================================================================//
//========================================================================================//
//     Copyright (c) 2016     Simtek, Incorporated      All rights reserved.              //
//                                                                                        //
//     This is unpublished proprietary source code of Simtek, Inc                         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
//========================================================================================//
//========================================================================================//
// 75-0640  for 10-8139-01                                                                //
//========================================================================================//
// Author      : C. Mason                                                                 //
// Date        : April  8, 2016                                                           //
// Revision    : -                                                                        //
// Notes       : Shipped with instrument.                                                 //
//========================================================================================//
//                                                                                        //
//========================================================================================//
//          include file declarations                                                     //
//========================================================================================//
#include "reg80390.h"
#include "types.h"
#include "intrins.h"
#include "CIRCUIT.H"
#include "printd.h"

sbit A_0                = 0xE0;
sbit A_1                = 0xE1;
sbit A_2                = 0xE2;
sbit A_3                = 0xE3;
sbit A_4                = 0xE4;
sbit A_5                = 0xE5;
sbit A_6                = 0xE6;
sbit A_7                = 0xE7;

//U16_T MAX144ReadADC(void);
void x100nSDelay(void);
void x001uSDelay(void);
void ext1_5thHardwareDelay(void);
void extHardwareDelay(void);
byte nibbleswap(byte dat);

// ---------------------------------------------------------------------------------------
// Function Name : x001uSDelay(void)
// Purpose       : create a 1uS delay
// Params        : void
// Returns       : void
// Note          : 
// ---------------------------------------------------------------------------------------
//
void x001uSDelay(void)
{
      x100nSDelay();                                                                      // 100nS 100nS
      x100nSDelay();                                                                      // 100nS 200nS
      x100nSDelay();                                                                      // 100nS 300nS
      x100nSDelay();                                                                      // 100nS 400nS
      x100nSDelay();                                                                      // 100nS 500nS
      x100nSDelay();                                                                      // 100nS 600nS
      x100nSDelay();                                                                      // 100nS 700nS
      x100nSDelay();                                                                      // 100nS 800nS
      x100nSDelay();                                                                      // 100nS 900nS
      _nop_();                                                                            //  20nS 920nS
}

// ---------------------------------------------------------------------------------------
// Function Name : x100nSDelay(void)
// Purpose       : create a 100nS delay
// Params        : void
// Returns       : void
// Note          : the call and return both take 40nS each
// ---------------------------------------------------------------------------------------
//
void x100nSDelay(void)
{
      _nop_();                                                                            // 20nS 020nS
}

// ---------------------------------------------------------------------------------------
// Function Name : ext1_5thHardwareDelay(void)
// Purpose       : create a delay
// Params        : void
// Returns       : void
// Note          : 
// ---------------------------------------------------------------------------------------
//
void ext1_5thHardwareDelay(void)                                                          // give the hardware some time to react
{
      _nop_();                                                                            // 20nS 020nS
      _nop_();                                                                            // 20nS 040nS
      _nop_();                                                                            // 20nS 060nS
      _nop_();                                                                            // 20nS 080nS
      _nop_();                                                                            // 20nS 100nS
      _nop_();                                                                            // 20nS 120nS
      _nop_();                                                                            // 20nS 140nS
      _nop_();                                                                            // 20nS 160nS
      _nop_();                                                                            // 20nS 180nS
      _nop_();                                                                            // 20nS 200nS
      _nop_();                                                                            // 20nS 220nS
      _nop_();                                                                            // 20nS 240nS
      _nop_();                                                                            // 20nS 260nS
      _nop_();                                                                            // 20nS 280nS
      _nop_();                                                                            // 20nS 300nS
      _nop_();                                                                            // 20nS 320nS
      _nop_();                                                                            // 20nS 340nS
      _nop_();                                                                            // 20nS 360nS
      _nop_();                                                                            // 20nS 380nS
      _nop_();                                                                            // 20nS 400nS
}

// ---------------------------------------------------------------------------------------
// Function Name : nibbleswap      
// Purpose       : swap the high and low nibbles of the given byte
// Params        : byte to swapped
// Returns       : swapped byte     
// Note          : 
// ---------------------------------------------------------------------------------------
byte nibbleswap(byte dat)
{
      AssyVarPass1 = dat;
#pragma asm
      push  Acc
      mov   Acc,tmpvar1
      swap  a
      mov   tmpvar1,Acc
      pop   Acc
#pragma endasm
      return AssyVarPass1;
}

// ---------------------------------------------------------------------------------------
// Function Name : extHardwareDelay(void)
// Purpose       : create a delay
// Params        : void
// Returns       : void
// Note          : 
// ---------------------------------------------------------------------------------------
void extHardwareDelay(void)                                                               // give the hardware some time to react
{
      ext1_5thHardwareDelay();                                                            // 01
      ext1_5thHardwareDelay();                                                            // 02
      ext1_5thHardwareDelay();                                                            // 03
      ext1_5thHardwareDelay();                                                            // 04
      ext1_5thHardwareDelay();                                                            // 05
      ext1_5thHardwareDelay();                                                            // 06
      ext1_5thHardwareDelay();                                                            // 07
      ext1_5thHardwareDelay();                                                            // 08
}

// ----------------------------------------------------------------------------
// Function Name: U16_T MAX144ReadADC(void)
// Purpose: read the adc value
// Params: void
// Returns: U16_T value
// Note:
// ----------------------------------------------------------------------------
//
//U16_T MAX144ReadADC(void)                                   // read the ADC
//{
//      U16_T reading;                                        // define a variable to hold the ADC data
//      
//      ADCClk = 1;                                           // initailize ADC clk is high to initiate internal clock conversion
//      ADCDin = 1;                                           // initialize ADC data pin is setup for reading
//      ADCCs  = 0;                                           // initialize ADC Cs for ADC wake up
//      x100nSDelay();                                        // 100nS give the display time to see the change
//      ADCCs  = 1;                                           // start the ADC to wake up signal
//                                                            // required delay = 60nS minimum
//      x100nSDelay();                                        // 100nS give the display time to see the change
//      ADCCs  = 0;                                           // finish the ADC wake up signal
//                                                            // required delay = 2.5uS minimum
//      x001uSDelay();                                        // 1uS give the display time to see the change
//      x001uSDelay();                                        // 1uS give the display time to see the change
//      x001uSDelay();                                        // 1uS give the display time to see the change
//      ADCClk = 0;                                           // signal the ADC to start a sample conversion
//#pragma asm
//      push   0x07                                           // save the contents of R7
//      push   ACC                                            // save the contents of ACC
//      mov    R7,#8                                          // load counter with the number of bits to retrieve
//mj1:	jnb	ADCDin,mj1							// need at least 100nS before this check
//mj2:  setb  ADCClk                                          // activate the ADC clock
//#pragma endasm          
//      x100nSDelay();                                        // 100nS give the display time to see the change
//#pragma asm
//      mov    c,ADCDin                                       // retrieve a bit of data from the ADC
//      rlc    a                                              // move the bit into the lsb of the ACC
//#pragma endasm          
//      x100nSDelay();                                        // 100nS give the display time to see the change
//#pragma asm
//      clr    ADCClk                                         // deactivate the ADC clock
//      djnz   r7,mj2                                         // loop back to get the next bit until eight bits have been retrieved
//      anl    a,#0x1F                                        // mask off all but the channel ID and the 4 msb
//      mov    dptr,#reading?440                              // load a pointer to the location to store this data
//      movx   @dptr,a                                        // save the upper byte of the word for later use
//      mov    R7,#8                                          // load counter with the number of bits to retrieve
//mj3:  setb   ADCClk                                         // activate the ADC clock
//#pragma endasm          
//      x100nSDelay();                                        // 100nS give the display time to see the change
//#pragma asm
//      mov    c,ADCDin                                       // retrieve a bit of data from the ADC
//      rlc    a                                              // move the bit into the lsb of the ACC
//#pragma endasm          
//      x100nSDelay();                                        // 100nS give the display time to see the change
//#pragma asm
//      clr    ADCClk                                         // deactivate the ADC clock
//      djnz   r7,mj3                                         // loop back to get the next bit until eight bits have been retrieved
//      clr    ADCCs                                          // deactivate the ADC chip select
//      inc    dptr                                           // move the pointer to the low byte storage location
//      movx   @dptr,a                                        // save the lower byte of the word for later use
//      pop    ACC                                            // retireve the old contents of ACC
//      pop    0x07                                           // retireve the old contents of R7
//#pragma endasm    
//      return reading;                                       // pass the ADC reading back to the calling routine
//}