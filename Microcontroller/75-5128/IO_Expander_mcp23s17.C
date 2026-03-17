//;************************************************
//DESCRIPTION: Device Driver for IO Expander MCP23S17
//
//
//;************************************************
#include<aduc841.h>
#include<absacc.h>

#define IO_BANK0 1
//;************************************************
// BANK = 1;
// Registers associated with the GPIO PORTS 
//#ifdef IO_BANK1
//
//#define IO_DIR_A     0x00
//#define I_POL_A      0x01
//#define GP_INT_EN_A  0x02
//#define DEF_VAL_A    0x03
//#define INT_CON_A    0x04
//#define IO_CON_A     0x05
//#define GP_PU_A      0x06
//#define INT_F_A      0x07
//#define INT_CAP_A    0x08
//#define GP_IO_A      0x09
//#define O_LAT_A      0x0A
//
//#define IO_DIR_B     0x10
//#define I_POL_B      0x11
//#define GP_INT_EN_B  0x12
//#define DEF_VAL_B    0x13
//#define INT_CON_B    0x14
//#define IO_CON_B     0x15
//#define GP_PU_B      0x16
//#define INT_F_B      0x17
//#define INT_CAP_B    0x18
//#define GP_IO_B      0x19
//#define O_LAT_B      0x1A
//#endif


//;************************************************
// BANK = 0;
// Registers associated with the GPIO PORTS 
#ifdef IO_BANK0

#define IO_DIR_A     0x00
#define IO_DIR_B     0x01

#define I_POL_A      0x02
#define I_POL_B      0x03

#define GP_INT_EN_A  0x04
#define GP_INT_EN_B  0x05

#define DEF_VAL_A    0x06
#define DEF_VAL_B    0x07

#define INT_CON_A    0x08
#define INT_CON_B    0x09

#define IO_CON_A     0x0A
#define IO_CON_B     0x0B

#define GP_PU_A      0x0C
#define GP_PU_B      0x0D

#define INT_F_A      0x0E
#define INT_F_B      0x0F

#define INT_CAP_A    0x10
#define INT_CAP_B    0x11

#define GP_IO_A      0x12
#define GP_IO_B      0x13

#define O_LAT_A      0x14
#define O_LAT_B      0x15

#endif



#define _IO_CS0(x) (PBYTE[0x0023] = x)

#define _IO_CS1(x) (PBYTE[0x0024] = x)

#define _IO_CLOCK(x) (PBYTE[0x0025] = x)

#define _MOSI(x) (PBYTE[0x0026] = x)


#define _IO_RESET(x) (PBYTE[0x0027] = x)


#define _GET_MISO() (PBYTE[0x0028])

#define CONTROL_WRITE 0x40
#define CONTROL_READ 0x41


extern char status[];
extern bit send_status_bit;

//;************************************************
//;************************************************
void initialize_mcp23s17(void);
void write_mcp23s17(unsigned char dat);
unsigned char read_mcp23s17(void);
char* read_porta_portb(void);

//;************************************************
/* initialize mcp23s17 io expander */
void initialize_mcp23s17(void)
{  

   _IO_RESET(0);
   _IO_RESET(0);
   _IO_CS0(1); // disable mcp23S17 device 0
   _IO_CLOCK(0);  // 
   _MOSI(0);
   _IO_RESET(1);
   _IO_CS0(0); // enable chip select 0
   write_mcp23s17(CONTROL_WRITE); // write command
   write_mcp23s17(GP_PU_A); // address GPIO A PULL UP REGISTER
   write_mcp23s17(0xFF); // enable gpio a pull up resistors
   write_mcp23s17(0xFF); // enable gpio b pull up resistors
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 

} // end initialize MCP23S17 IO expander function 



//;************************************************
/* read port a and port b mcp23s17 io expander */
char* read_porta_portb(void)
{
   unsigned char port[2];
   _IO_CS0(0); // enable chip select 0
   write_mcp23s17(CONTROL_READ); // write command
   write_mcp23s17(GP_IO_A); // address GPIO A input REGISTER
   port[0] = read_mcp23s17(); // get  gpio a input data
   port[1] = read_mcp23s17(); // get  gpio b input data
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 
   _IO_CS0(1); // disable chip select 0; 
   return (&port[0]);
}


//;************************************************
/* write to mcp23s17 io expander */
void write_mcp23s17(unsigned char dat)
{
unsigned char i;

   for (i = 0x80; i > 0; i >>= 1) {
      _MOSI((dat & i) ? 1 : 0);
      _IO_CLOCK(0);
      _IO_CLOCK(0);
      _IO_CLOCK(1);
      _IO_CLOCK(1);
      _IO_CLOCK(0);
      }   
}


//;************************************************
/* read from mcp23s17 io expander */
unsigned char read_mcp23s17(void)
{
unsigned char i,input_data;
   input_data = 0;
   for (i = 0; i < 8; i++ ) {
      input_data <<= 1;  
      _IO_CLOCK(0);
      _IO_CLOCK(1);
      input_data |= _GET_MISO();  
      _IO_CLOCK(1);
      _IO_CLOCK(0);
      }
return (input_data);         
}
//;************************************************