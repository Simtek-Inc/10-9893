//;************************************************
//DESCRIPTION:
//SIMTEK PART # 10-8665-01
//FOR POE C-130
//75-5128 
//DAVID B.
//4/3/2019
//;************************************************
#include<aduc841.h>
#include<absacc.h>
#include "IO_expander_mcp23s17.h"
//#include<intrins.h>
//;************************************************
#define RESET_COMMAND 0xF0				//
#define FIRMWARE_COMMAND 0xFE			//
#define STATUS_COMMAND 0xF1				//
#define INDICATORS_COMMAND 0xF3			//
#define DIMMING_COMMAND 0xF4			//
#define DISPLAY_COMMAND 0xF5			//
//;************************************************
sbit fpga_reset	= P3^5;					// u2 pin 23 IC_OUT_LCRST 
//;************************************************
sbit ADC_clk = P2^2;
sbit ADC_data = P2^3;
sbit ADC_cs = P2^4;
//;************************************************
bit	reset_bit = 0;						// reset flag
bit send_programs_bit = 0;				// program response flag
bit send_status_bit = 0;				// status response flag
bit	update_dimming_bit = 0;				// dimming update flag
//;************************************************
unsigned char RS_micro_firmware[] =  {0x75,0x51,0x28,0x2D};
unsigned char RS_fpga_firmware[4];		// array of 4 sequential storage locations 0 - 3
unsigned char panel_dim_byte;
unsigned char display_dim_byte;
unsigned char display_data[18];			// array of 18 sequential storage locations 0 - 17
//;************************************************
unsigned char bdata Abyte;				// create a bit addressable byte
sbit Abyte0 = Abyte^0;				   		
sbit Abyte1 = Abyte^1;
sbit Abyte2 = Abyte^2;
sbit Abyte3 = Abyte^3;
sbit Abyte4 = Abyte^4;
sbit Abyte5 = Abyte^5;
sbit Abyte6 = Abyte^6;
sbit Abyte7 = Abyte^7;
//;************************************************
unsigned char bdata Bbyte;				// create a bit addressable byte
sbit Bbyte0 = Bbyte^0;				   		
sbit Bbyte1 = Bbyte^1;
sbit Bbyte2 = Bbyte^2;
sbit Bbyte3 = Bbyte^3;
sbit Bbyte4 = Bbyte^4;
sbit Bbyte5 = Bbyte^5;
sbit Bbyte6 = Bbyte^6;
sbit Bbyte7 = Bbyte^7;
//;************************************************
unsigned char Status_byte_1;		// rocker
unsigned char Status_byte_2;		// pot data
unsigned char Status_byte_3;		// toggle + encoder press
unsigned char Status_byte_4;		// rotary switch
unsigned char Status_byte_5;		// encoder data
//;************************************************
unsigned char *sw_ptr; // pointer to switch data porta and portb
//;************************************************
void setup_serial(void);
void reset_everything(void);
void transmit_data(void);
void receive_data(void);
void send_programs(void);
void update_dimming(void);
void clk_in_display_data(void);
void latch_display_data(void);
void send_status(void);
void clk_adc(void);
void adc_read(void);
void clk_in_6211_data(void);
void latch_6211_data(void);
void update_6211_data(void);
void clk_in_6212_data(void);
void latch_6212_data(void);
void update_6212_data(void);

void check_input(void);
void initialize_mcp23s17(void);
//;************************************************
void main(void)
{	setup_serial();
	//ewait = 0x00;	
	fpga_reset = 1;
	fpga_reset = 0;
	send_programs_bit = 0;
	reset_bit = 1;
	reset_everything();
	P1 	= 0x00;							// configure as inputs
	initialize_mcp23s17();
//;************************************************
	while(1)						
	{
	reset_everything();
	send_programs();
	update_dimming();
	adc_read();
	update_6211_data();
	update_6212_data();
	sw_ptr = read_porta_portb();
    check_input();
	send_status();								
	}
}
//;************************************************
void setup_serial(void)
{ 	
	SCON	= 0x40;						// MODE1 UART
	T3CON	= 0x87;						// 9600K BPS	
	T3FD	= 0x01;						// DEFAULT IS 9600K BAUD
	REN 	= 1;						// RECEIVE ENABLED
	ES 		= 1;						// ENABLE SERIAL
	EA		= 1;						// ALL INTERRUPTS OK
	RI 		= 0;						// CLR RECEIVE FLAG
	TI		= 0;						// CLR TRANSMIT FLAG
}
//;************************************************
void reset_everything(void)
{ 
unsigned char i;	
if(reset_bit)							// check to see if reset flag is set
	{								   	
	PBYTE[0X0010] = 0x00;				// reset display dimming			 
	PBYTE[0X0011] = 0x00;				// reset panel dimming

	RS_fpga_firmware[0] = PBYTE[2];
	RS_fpga_firmware[1] = PBYTE[3];
	RS_fpga_firmware[2] = PBYTE[4];
	RS_fpga_firmware[3] = PBYTE[5];
	Status_byte_1 = 0x00;
	Status_byte_2 = 0x00;
	Status_byte_3 = 0x00;
	Status_byte_4 = 0x00;
	Status_byte_5 = 0x00;
	for(i = 0; i < 18; i++)
		{
		display_data[i] = 0x00;			// array of 18 sequential storage locations 0 - 17
		reset_bit = 0;					// reset panel dimming
		}
	}
}
//;************************************************
void transmit_data(void)
{
	while(!TI);		
	TI = 0;
}
//;************************************************
void receive_data(void)
{
	while(!RI);		
	RI = 0;
}
//;************************************************
void send_programs(void)
{
unsigned char i;  	
if(send_programs_bit)					// check to see if send program flag is set
	{
   		SBUF = FIRMWARE_COMMAND;	
		transmit_data();
//;micro program***********************************			
	for(i = 0; i < 4; i++)
		{	
		SBUF = RS_micro_firmware[i];	
		transmit_data();				    
		}
//;fpga program************************************			
	for(i = 0; i < 4; i++)
		{			
		SBUF = RS_fpga_firmware[i];
		transmit_data();	
		}
//;************************************************				
	send_programs_bit = 0;				// clear the flag
	}
}
//;************************************************
void update_dimming(void)
{ 	
if(update_dimming_bit)					// check to see if update dimming flag is set
	{
	PBYTE[0X0010] = display_dim_byte;	
	PBYTE[0X0011] = panel_dim_byte;
	update_dimming_bit = 0;				// clear the flag
	}
}
//;************************************************
void send_status(void)
{
if(send_status_bit)
	{
	SBUF = STATUS_COMMAND;	
	transmit_data();
	
	SBUF = Status_byte_1;	
	transmit_data();
	
	SBUF = Status_byte_2;				
	transmit_data();
	
	SBUF = Status_byte_3;	
	transmit_data();
	
	SBUF = Status_byte_4;	
	transmit_data();

	Status_byte_5 = PBYTE[0X0060];
	SBUF = Status_byte_5;				//encoder data	
	transmit_data();	

	send_status_bit = 0;
	}
}
//;************************************************
void clk_adc(void)
{
	ADC_clk = 1;
  	ADC_clk = 0;
}
//;************************************************
void adc_read(void)
{	ADC_clk = 1;			// adc clock high
    ADC_cs = 1;				// adc cs high
    ADC_cs = 0;				// adc chip select lo
while(ADC_data == 0);		// wait for end of conversion
		clk_adc();			//
		clk_adc();			//
		clk_adc();			//
		clk_adc();			//channel id
//channel id
	Abyte0 = ADC_data;		// bit 4 is channels data		
//channel id				
	if(!Abyte0)				// channel 0
		{
		Abyte = 0x00;
		clk_adc();
	 	Abyte7 = ADC_data;		
		clk_adc();
	 	Abyte6 = ADC_data;
		clk_adc();
	 	Abyte5 = ADC_data;		
		clk_adc();
	 	Abyte4 = ADC_data;
		clk_adc();
		Abyte3 = ADC_data;		
		clk_adc(); 
	 	Abyte2 = ADC_data;
		clk_adc(); 
		Abyte1 = ADC_data;
		clk_adc(); 
		Abyte0 = ADC_data;		
		clk_adc();			//d3		
		clk_adc();			//d2		
		clk_adc();			//d1
		clk_adc();			//d0
		Status_byte_2 = ~Abyte;
		}
}
//;************************************************
void latch_6211_data(void)
{	//when X"41" => Load_6211
	PBYTE[0X0041] = 1;
	PBYTE[0X0041] = 0;	
}
//;************************************************
void clk_in_6211_data(void)
{	//when X"40" => Clk_6211
	PBYTE[0X0040] = 1;
	PBYTE[0X0040] = 0;
}
//;************************************************
void update_6211_data(void)
{		//when X"42" => Din_6211
//;************************************************
		PBYTE[0X0042] = 0;                   		// S48 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                			// S47 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                  		// S46 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;              			// S45 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                   		// S44 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                   		// S43 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                   		// S42 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                   		// S41 nc
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte1;
		PBYTE[0X0042] = Abyte0;                   	// S40 ds5_b display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte0;		
		PBYTE[0X0042] = Abyte0;                   	// S39 ds5_a display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte6;		
		PBYTE[0X0042] = Abyte0;                   	// S38 ds5_g display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte2;		
		PBYTE[0X0042] = Abyte0;                   	// S37 ds5_c display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte4;		
		PBYTE[0X0042] = Abyte0;                   	// S36 ds5_e display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte3;		
		PBYTE[0X0042] = Abyte0;                   	// S35 ds5_d display_data[17]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[17];
		Abyte0 = Bbyte5;		
		PBYTE[0X0042] = Abyte0;                   	// S34 ds5_f display_data[17]
		clk_in_6211_data();
//;************************************************		
		PBYTE[0X0042] = 0;                   		// S33 nc
		clk_in_6211_data();
//;************************************************		
		PBYTE[0X0042] = 0;                   		// S32 nc
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte5;		
		PBYTE[0X0042] = Abyte0;                    	// S31 ds4_f display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte3;		
		PBYTE[0X0042] = Abyte0;                   	// S30 ds4_d display_data[16]  
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte4;		
		PBYTE[0X0042] = Abyte0;                		// S29 ds4_e display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte2;		
		PBYTE[0X0042] = Abyte0;               		// S28 ds4_c display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte6;		
		PBYTE[0X0042] = Abyte0;               		// S27 ds4_g display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte0;		
		PBYTE[0X0042] = Abyte0;               		// S26 ds4_a display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[16];
		Abyte0 = Bbyte1;		
		PBYTE[0X0042] = Abyte0;               		// S25 ds4_b display_data[16]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte1;		
		PBYTE[0X0042] = Abyte0;              		// S24 ds3_b display_data[15]  
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte0;		
		PBYTE[0X0042] = Abyte0;                   	// S23 ds3_a display_data[15]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte6;		
		PBYTE[0X0042] = Abyte0;	           			// S22 ds3_g display_data[15] 
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte2;		
		PBYTE[0X0042] = Abyte0;               		// S21 ds3_c display_data[15]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte4;		
		PBYTE[0X0042] = Abyte0;                   	// S20 ds3_e display_data[15]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte3;		
		PBYTE[0X0042] = Abyte0;                   	// S19 ds3_d display_data[15]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[15];
		Abyte0 = Bbyte5;		
		PBYTE[0X0042] = Abyte0;                		// S18 ds3_f display_data[15]
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                  		// S17 nc
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;                    		// S16 nc
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte5;
		PBYTE[0X0042] = Abyte0;                   	// S15 ds2_f display_data[14] 
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte3;		
		PBYTE[0X0042] = Abyte0;                   	// S14 ds2_d display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte4;		
		PBYTE[0X0042] = Abyte0;                   	// S13 ds2_e display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte2;		
		PBYTE[0X0042] = Abyte0;                   	// S12 ds2_c display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte6;		
		PBYTE[0X0042] = Abyte0;                   	// S11 ds2_g display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte0;		
		PBYTE[0X0042] = Abyte0;                   	// S10 ds2_a display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[14];
		Abyte0 = Bbyte1;		
		PBYTE[0X0042] = Abyte0;                   	// S9 ds2_b display_data[14]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte1;		
		PBYTE[0X0042] = Abyte0;                   	// S8 ds1_b display_data[13]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte0;		
		PBYTE[0X0042] = Abyte0;               		// S7 ds1_a display_data[13]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte6;		
		PBYTE[0X0042] = Abyte0;               		// S6 ds1_g display_data[13]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte2;		
		PBYTE[0X0042] = Abyte0;               		// S5 ds1_c display_data[13]
 		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte4;		
		PBYTE[0X0042] = Abyte0;               		// S4 ds1_e display_data[13]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte3;		
		PBYTE[0X0042] = Abyte0;               		// S3 ds1_d display_data[13]
		clk_in_6211_data();
//;************************************************
		Bbyte = display_data[13];
		Abyte0 = Bbyte5;		
		PBYTE[0X0042] = Abyte0;               		// S2 ds1_f display_data[13]
		clk_in_6211_data();
//;************************************************
		PBYTE[0X0042] = 0;               			// S1 nc
		clk_in_6211_data();
//;************************************************
		latch_6211_data();
//;************************************************
}
//;************************************************
void latch_6212_data(void)
{	//when X"44" => Load_6212 <= AddrData(0)
	PBYTE[0X0044] = 1;
	PBYTE[0X0044] = 0;	
}
//;************************************************
void clk_in_6212_data(void)
{	//when X"43" => Clk_6212
	PBYTE[0X0043] = 1;
	PBYTE[0X0043] = 0;
}
//;************************************************
void update_6212_data(void)
{		//when X"45" => Din_6212 <= AddrData(0);
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S112 DS13_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S111 DS13_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S110 DS13_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S109 DS13_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S108 DS13_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S107 DS13_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S106 DS13_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S105 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[12];
		Abyte0 = Bbyte7;
		PBYTE[0X0045] = Abyte0;                   	// S104 dp1
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S103 nc
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S102 nc
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S101 nc
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S100 nc
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S99 nc
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S98 nc
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S97 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S96 DS12_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S95 DS12_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S94 DS12_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S93 DS12_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S92 DS12_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S91 DS12_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[11];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S90 DS12_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S89 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S88 DS11_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S87 DS11_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S86 DS11_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S85 DS11_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S84 DS11_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S83 DS11_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[10];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S82 DS11_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S81 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S80 DS10_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S79 DS10_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S78 DS10_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S77 DS10_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S76 DS10_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S75 DS10_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[9];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S74 DS10_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S73 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S72 DS9_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S71 DS9_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S70 DS9_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S69 DS9_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S68 DS9_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S67 DS9_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[8];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S66 DS9_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S65 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S64 DS8_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S63 DS8_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S62 DS8_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S61 DS8_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S60 DS8_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S59 DS8_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[7];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S58 DS8_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S57 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S56 DS7_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S55 DS7_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S54 DS7_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S53 DS7_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S52 DS7_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S51 DS7_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[6];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S50 DS7_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S49 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S48 DS6_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S47 DS6_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S46 DS6_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S45 DS6_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S44 DS6_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S43 DS6_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[5];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S42 DS6_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S41 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S40 DS5_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S39 DS5_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S38 DS5_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S37 DS5_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S36 DS5_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S35 D5_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[4];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S34 DS5_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S33 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S32 DS4_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S31 DS4_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S30 DS4_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S29 DS4_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S28 DS4_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S27 DS4_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[3];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S26 DS4_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S25 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S24 DS3_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S23 DS3_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S22 DS3_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S21 DS3_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S20 DS3_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S19 DS3_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[2];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S18 DS3_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S17 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                   	// S16 DS2_e
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                 	// S15 DS2_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                   	// S14 DS2_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;               		// S13 DS2_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                    	// S12 DS2_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                    	// S11 DS2_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[1];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                    	// S10 DS2_c
		clk_in_6212_data();
//;************************************************
		PBYTE[0X0045] = 0;                   		// S9 nc
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte2;
		PBYTE[0X0045] = Abyte0;                   	// S8 DS1_c
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte6;
		PBYTE[0X0045] = Abyte0;                  	// S7 DS1_g
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte0;
		PBYTE[0X0045] = Abyte0;                   	// S6 DS1_a
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte1;
		PBYTE[0X0045] = Abyte0;                  	// S5 DS1_b
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte5;
		PBYTE[0X0045] = Abyte0;                  	// S4 DS1_f
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte3;
		PBYTE[0X0045] = Abyte0;                  	// S3 DS1_d
		clk_in_6212_data();
//;************************************************
		Bbyte = display_data[0];
		Abyte0 = Bbyte4;
		PBYTE[0X0045] = Abyte0;                  	// S2 DS1_e
		clk_in_6212_data();
//;************************************************		
		PBYTE[0X0045] = 0;                   		// S1 nc
		clk_in_6212_data();
//;************************************************
		latch_6212_data();
//;************************************************
}
//;************************************************
// check input data from port expander
void check_input(void)
{  

char porta;
char portb;
//char tmp;
   	porta = *sw_ptr;
   	portb = *(++sw_ptr);

 	Status_byte_1 = portb;
	Status_byte_1 = ~Status_byte_1;
	Status_byte_1 = (Status_byte_1 & 0x03);	// manual valve control 
											// 0x01 = open
											// 0x02 = closed
	Abyte = 0x00;
	Bbyte = 0x00;
	
	Bbyte = portb;							// emer toggle + encoder pressed
	Abyte0 = Bbyte7;						// 0x01 = emer depress norm		
	Abyte1 = Bbyte6;						// 0x02 = emer depress dump
    Bbyte = PBYTE[0X0061];
   	Abyte4 = Bbyte0; 						// 0x10 = encoder pressed
											// 0x00 = encoder not pressed
	Status_byte_3 = Abyte;
	Status_byte_3 = ~Status_byte_3;
	Status_byte_3 = (Status_byte_3 & 0x13);

   	Status_byte_4 = porta;					// rotary switch
   	Status_byte_4 = ~Status_byte_4;			// 0x01 = const alt
   	Status_byte_4 = (Status_byte_4 & 0x1F);	// 0x02 = man
											// 0x04 = auto
											// 0x08 = no press
											// 0x10 = aux vent
//	Bbyte = portb;
	//Bbyte = ~Bbyte;
//	Abyte1 = Bbyte7;
//	Abyte0 = Bbyte6;
//    Bbyte = PBYTE[0X0061];
//   	Abyte4 = Bbyte0;

//	Status_byte_3 = ~Abyte;

}
//;************************************************
void SER_INT(void) interrupt 4
{
unsigned char hold_byte;
unsigned char i;
if(!TI)
TI = 0;
//;************************************************
	if (RI)
	{hold_byte = SBUF;						
	 RI = 0;
//;************************************************									
		switch(hold_byte)						// 
			 {
//;************************************************				 										
			case RESET_COMMAND:	 		// 0xF0
				reset_bit = 1;
			break;
//;************************************************									
			case FIRMWARE_COMMAND:		// 0xFE	
				send_programs_bit = 1;
			break;
//;************************************************
			case STATUS_COMMAND:		// 0xF1
				send_status_bit = 1;
			break;
//;************************************************
			case DIMMING_COMMAND:		// 0xF4		 
				receive_data();
				panel_dim_byte = SBUF;
				receive_data();
				display_dim_byte = SBUF;
				update_dimming_bit = 1;
			break;
//;************************************************
			case DISPLAY_COMMAND:		// 0xF5
				for(i = 0; i < 18; i++)
 			    	{
 			    	receive_data();					
		 			display_data[i] = SBUF;
					}
			break;
//;************************************************							
			}							// end case
	}									//end if RI
}
//;************************************************	