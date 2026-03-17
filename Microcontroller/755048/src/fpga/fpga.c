//========================================================================================//
//========================================================================================//
//========================================================================================//
//     Copyright (c) 2017     Simtek, Incorporated      All rights reserved.              //
//                                                                                        //
//     This is unpublished proprietary source code of Simtek, Inc                         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
//========================================================================================//
//========================================================================================//
// 75-1503  for 10-8324-01                                                                //
//========================================================================================//
// Author      : C. Mason                                                                 //
// Date        : February 8, 2017                                                         //
// Revision    : -                                                                        //
// Notes       : Shipped with instrument.                                                 //
//========================================================================================//
//                                                                                        //
//========================================================================================//
//          include file declarations                                                     //
//========================================================================================//
#include "CIRCUIT.H"                                                                      //

void writeFpgaAddress(byte adrs);                                                         //

       word idata fpga16  _at_ 0x20;                                                      //
       byte bdata ByteHi  _at_ 0x20;                                                      //
       byte bdata ByteLo  _at_ 0x21;                                                      //
    sbit          bit_00  = ByteLo^0;                                                     //
    sbit          bit_01  = ByteLo^1;                                                     //
    sbit          bit_02  = ByteLo^2;                                                     //
    sbit          bit_03  = ByteLo^3;                                                     //
    sbit          bit_04  = ByteLo^4;                                                     //
    sbit          bit_05  = ByteLo^5;                                                     //
    sbit          bit_06  = ByteLo^6;                                                     //
    sbit          bit_07  = ByteLo^7;                                                     //
    sbit          bit_08  = ByteHi^0;                                                     //
    sbit          bit_09  = ByteHi^1;                                                     //
    sbit          bit_0A  = ByteHi^2;                                                     //
    sbit          bit_0B  = ByteHi^3;                                                     //
    sbit          bit_0C  = ByteHi^4;                                                     //
    sbit          bit_0D  = ByteHi^5;                                                     //
    sbit          bit_0E  = ByteHi^6;                                                     //
    sbit          bit_0F  = ByteHi^7;                                                     //


//----------------------------------------------------------------------------------------
// Function Name : writeFpgaAddress                                                       //
// Purpose       : write the address of the register to access within the FPGA            //
// Params        : the address of the register                                            //
// Returns       : void                                                                   //
// Note          :                                                                        //
//----------------------------------------------------------------------------------------
void writeFpgaAddress(byte adrs)                                                          //
{
      byte i;

      fpgaserialale     = 1;                                                              // make sure the fpga ale is deactived before starting
      fpgaserialclock   = 1;                                                              // make sure the fpga serial register clock is deactived before starting
      ByteHi            = adrs;                                                           // zero out the read byte to start
      fpgaserialale     = 0;                                                              // activate the ale to move the address into the address latch from the sipo function
      for (i = 0; i < 8; i++)                                                             // loop until all bits are read
        {
        fpgadatain      = bit_08;                                                         // write the bit of the data to the fpga data input pin
        fpgadatain      = bit_08;                                                         // burn a ccyle
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // burn a ccyle
        ByteHi          = ByteHi >> 1;                                                    // shift the next bit into position for the next bit write
        fpgaserialclock = 1;                                                              // set the clock to activate
        fpgaserialclock = 1;                                                              // set the clock to activate
        }
      fpgaserialclock   = 1;                                                              // set the clock to high to burn a cycle
      fpgaserialale     = 1;                                                              // now that the address is latched deactivate the ale
      fpgadatain        = 1;                                                              // write the bit of the data to the fpga data input pin
}

//----------------------------------------------------------------------------------------
// Function Name : writeFpgaData                                                          //
// Purpose       : write the address of the register to access within the FPGA            //
// Params        : the address of the register, and the bits of resolution                //
// Returns       : void                                                                   //
// Note          :                                                                        //
//----------------------------------------------------------------------------------------
void writeFpgaData(byte adrs, word dat, byte numofbits)                                   //
{
      byte i;

      writeFpgaAddress(adrs);                                                             // output the fpga register address
      fpgaserialload    = 1;                                                              // make sure the fpga load is deactived before starting
      fpgaserialclock   = 1;                                                              // make sure the fpga serial register clock is deactived before starting
      fpga16            = dat;                                                            // load the data word to write to the register 
      fpgaserialload    = 0;                                                              // activate the load ale to move the address into the address latch from the sipo function
      for (i = 0; i < numofbits; i++)                                                     // loop until all bits are read
        {
        fpgadatain      = bit_00;                                                         // write the bit of the data to the fpga data input pin
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // burn a ccyle
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // burn a ccyle
        fpga16          = fpga16 >> 1;                                                    // shift the next bit into position for the next bit write
        fpgaserialclock = 1;                                                              // activate the fpga serial register clock
        fpgaserialclock = 1;                                                              // activate the fpga serial register clock
        }
      fpgaserialclock   = 1;                                                              // set the clock to high again to burn a cycle
      fpgaserialload    = 1;                                                              // now that the data is clocked in activate the load
      fpgadatain        = 1;                                                              // write the bit of the data to the fpga data input pin
}

//----------------------------------------------------------------------------------------
// Function Name : writeFpgaDataL                                                         //
// Purpose       : write the address of the register to access within the FPGA            //
// Params        : the address of the register, and the bits of resolution                //
// Returns       : void                                                                   //
// Note          :                                                                        //
//----------------------------------------------------------------------------------------
void writeFpgaDataL(byte adrs, byte *pdat, byte numofbits)                                //
{
      byte i;                                                                             //
      
      writeFpgaAddress(adrs);                                                             // output the fpga register address
      fpgaserialload    = 1;                                                              // make sure the fpga load is deactived before starting
      fpgaserialclock   = 1;                                                              // make sure the fpga serial register clock is deactived before starting
      fpgaserialload    = 0;                                                              // activate the load ale to move the address into the address latch from the sipo function
      for (i = 0; i < numofbits; i++)                                                     // loop until all bits are read
        {
        if((i % 8) == 0)                                                                  //
          {
          ByteLo        = *pdat;                                                          //
          pdat++;                                                                         //
          }
        fpgadatain      = bit_00;                                                         // write the bit of the data to the fpga data input pin
        fpgadatain      = bit_00;                                                         // burn a ccyle
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // burn a ccyle
        fpgaserialclock = 0;                                                              // deactivate the fpga serial register clock
        fpgaserialclock = 0;                                                              // burn a ccyle
        ByteLo          = ByteLo >> 1;                                                    // shift the next bit into position for the next bit write
        fpgaserialclock = 1;                                                              // activate the fpga serial register clock
        fpgaserialclock = 1;                                                              // activate the fpga serial register clock
        }
      fpgaserialclock   = 1;                                                              // set the clock to high again to burn a cycle
      fpgaserialload    = 1;                                                              // now that the data is clocked in activate the load
      fpgadatain        = 1;                                                              // write the bit of the data to the fpga data input pin
}

//----------------------------------------------------------------------------------------
// Function Name : readFpgaData                                                           //
// Purpose       : read the data from the register address given                          //
// Params        : the address of the register, and the bits of resolution                //
// Returns       : void                                                                   //
// Note          : this routine is setup for a max of 16 bits                             //
//----------------------------------------------------------------------------------------
word readFpgaData(byte adrs, numofbits)                                                   //
{
      byte i;

      writeFpgaAddress(adrs);                                                             // output the fpga register address
      fpgaserialclock   = 1;                                                              // set the clock and, the load line to make sure
      fpgaserialload    = 1;                                                              //  the index starts at zero , and the data is latched
      fpgadataout       = 1;                                                              // setup pin for reading
      fpgaserialload    = 0;                                                              // activate the load to move the parallel data into the serial hold register
      fpga16            = 0;                                                              // clear the data word before starting read 
      for (i = 0; i < numofbits; i++)                                                     // loop until all bits are read
        {
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpgaserialclock = 1;                                                              // the data is written on the rising edge
        fpga16          = fpga16 >> 1;                                                    // shift the next bit into position for the next bit write
        bit_0F          = fpgadataout;                                                    // write the bit of the data to the fpga data input pin
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        fpgaserialclock = 0;                                                              // toggle the fpga serial clock,
        }
      fpgaserialclock   = 1;                                                              // burn a cycle to make sure that the data pin has settled
      fpgaserialclock   = 1;                                                              // burn a cycle to make sure that the data pin has settled
      fpgaserialclock   = 1;                                                              // burn a cycle to make sure that the data pin has settled
      fpgaserialclock   = 1;                                                              // burn a cycle to make sure that the data pin has settled
      fpgaserialload    = 1;                                                              // make sure the fpga ale is deactived before starting
      i = 16 - numofbits;                                                                 // find number of bits to adjust data passed back
      if(i != 0)  {                                                                       // if the data is a full 16 no adjustment is necessary
        while(i != 0)                                                                     // keep shifting until data placement is right for numofbits
          {  fpga16     = fpga16 >> 1; i--;  }}                                           // shift the data over one bit, adjust count for one shift done
      return fpga16;                                                                      // return the data
}