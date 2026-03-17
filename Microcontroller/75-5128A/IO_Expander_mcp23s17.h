//;************************************************
//DESCRIPTION: Device Driver for IO Expander MCP23S17
//
//
//;************************************************
//#include<aduc841.h>
//;************************************************



//#define SET_IO_CS0 (PBYTE[0x23] = 1)
//#define CLEAR_IO_CS0 (PBYTE[0x23] = 0)
//
//#define SET_IO_CS1 (PBYTE[0x24] = 1)
//#define CLEAR_IO_CS1 (PBYTE[0x24] = 0)
//
//#define SET_IO_CLOCK (PBYTE[0x25] = 1)
//#define CLEAR_IO_CLOCK (PBYTE[0x25] = 0)
//
//#define SET_IO_SI (PBYTE[0x26] = 1)
//#define CLEAR_IO_SI (PBYTE[0x26] = 0)
//
//
//#define SET_IO_RESET (PBYTE[0x27] = 1)
//#define CLEAR_IO_RESET (PBYTE[0x27] = 0)
//
//
//#define GET_IO_DATA (PBYTE[0x28])
//
//#define CONTROL_WRITE 0x40
//#define CONTROL_READ 0x41



//;************************************************
//;************************************************
void initialize_mcp23s17(void);
void write_mcp23s17(unsigned char dat);
unsigned char read_mcp23s17(void);
char* read_porta_portb(void);

//;************************************************






//;************************************************