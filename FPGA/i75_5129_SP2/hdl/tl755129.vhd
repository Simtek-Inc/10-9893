--//	SIMTEK, INC
--//	PART #:75-5129
--//	DESCRIPTION: ,
--//	SIMTEK PART # 10-8665 
--//	by: david bradshaw
--//	April 3, 2019

--//;=========================================================================
--//;This program used with 51-6160 (51-6108) circuit board assembly 
--//;using ribbon cables.
--//;=========================================================================

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity tl755129 is
  Port (
    MClk : in std_logic;
    UcClk : out std_logic;
    Rst : in std_logic;
    AddrData : inout std_logic_vector(7 downto 0);
    Rd : in std_logic;
    Wr : in std_logic;
    Ale : in std_logic;
    Max_DE : out std_logic; -- max 422 drive enable
    Max_422_232_Sel : out std_logic;
    PanelDrive : out std_logic;
    OE_6211 : out std_logic;
    Clk_6211 : out std_logic;
    Load_6211 : out std_logic;
    Din_6211 : out std_logic;
    OE_6212 : out std_logic;
    Clk_6212 : out std_logic;
    Load_6212 : out std_logic;
    Din_6212 : out std_logic;
    Encoder_VDC : out std_logic;
    A1 : in std_logic;
	B1 : in std_logic;
    Encoder_PB_in : in std_logic;
    Encoder_PB_rtn : out std_logic;
    Encoder_VDC_rtn : out std_logic;
    -- input output expander drive signals using MCP23S17
    IO_CS0 : out std_logic;
    IO_CS1 : out std_logic;
    IO_Clk : out std_logic;
    IO_MOSI : out std_logic;
    IO_MISO : in std_logic;
    IO_Reset : out std_logic
    );
end tl755129;

architecture Behavioral of tl755129 is

--Clock divider--------------------------------------
COMPONENT ClockDivider 
PORT (
    MClk : in std_logic;
    PwmClk : out std_logic;
    UcClk : out std_logic
    );
END COMPONENT;

--Address latch------------------------------------------
COMPONENT address_latch
PORT(
	MClk : IN std_logic;
	Rst : IN std_logic;
	Ale : IN std_logic;
	AddrData : IN std_logic_vector(7 downto 0);          
	Address : OUT std_logic_vector(7 downto 0)
	);
END COMPONENT;

--get data from microcontroller--------------------------
COMPONENT RamMem
PORT(
    MClk : in std_logic;
    Rst : in std_logic;
    Rd : in std_logic;
    Wr : in std_logic;
    AddrData : inout std_logic_vector(7 downto 0);
    Address : in std_logic_vector(7 downto 0);
    PanelDimValue : out std_logic_vector(7 downto 0);
    D6211DimValue : out std_logic_vector(7 downto 0);
    Clk_6211 : out std_logic;
    Load_6211 : out std_logic;
    Din_6211 : out std_logic;
    Clk_6212 : out std_logic;
    Load_6212 : out std_logic;
    Din_6212 : out std_logic ;
    datacounter1 : in std_logic_vector(7 downto 0);
    Encoder_PB_in : in std_logic;
    -- input output expander drive signals using MCP23S17
    IO_CS0 : out std_logic;
    IO_CS1 : out std_logic;
    IO_Clk : out std_logic;
    IO_MOSI : out std_logic;
    IO_MISO : in std_logic;
    IO_Reset : out std_logic
	);
END COMPONENT;

COMPONENT Pwm
PORT (
  Rst : in std_logic;
  Clk : in std_logic;
  PwmClk : in std_logic;
  PwmDimValue : in std_logic_vector(7 downto 0);
  PwmCtrl : out std_logic
  );
END COMPONENT;

COMPONENT Display_Pwm
PORT (
  Rst : in std_logic;
  Clk : in std_logic;
  PwmClk : in std_logic;
  PwmDimValue : in std_logic_vector(7 downto 0);
  PwmCtrl : out std_logic
  );
END COMPONENT;


COMPONENT state_machine
Port ( 
    master_clock : in std_logic;
    logic_reset : in std_logic;
    A : in std_logic;
    B : in std_logic;
    dir : out std_logic;
    tclk : out std_logic
    );
END COMPONENT;

COMPONENT filter
Port (
    master_clock : in std_logic;
    logic_reset : in std_logic;
    A : in std_logic;
    B : in std_logic;
    outA : out std_logic;
    outB : out std_logic
	);
END COMPONENT;

COMPONENT ucounter
Port (     
    master_clock : in std_logic;
    logic_reset : in std_logic;
    dir : in std_logic;
    ce : in std_logic;
    Dout : out std_logic_vector(7 downto 0)
    );
END COMPONENT;

signal Address :std_logic_vector(7 downto 0);

signal iPwmClk : std_logic;
signal iPWM_CTRL : std_logic;

signal iPanelDimValue :std_logic_vector(7 downto 0);
signal iD6211DimValue :std_logic_vector(7 downto 0);

signal outA1,outB1,dir1,tclk1 : std_logic;
signal datacounter1 : std_logic_vector(7 downto 0);	

begin

--set com to RS232 to microcontroller--------------------
Max_422_232_Sel <= '0';     
Max_DE <= '0';
OE_6212 <= OE_6211;
Encoder_VDC <= '1';
Encoder_VDC_rtn <= '0';
Encoder_PB_rtn <= '0';

--get data from microcontroller--------------------------
Inst_RamMem: RamMem PORT MAP(
  MClk  => MClk,
  Rst   => Rst,
  Rd    => Rd,
  Wr    => Wr,
  AddrData => AddrData,
  Address => Address, 
  PanelDimValue => iPanelDimValue,
  D6211DimValue => iD6211DimValue,
  Clk_6211 => Clk_6211,
  Load_6211 => Load_6211,
  Din_6211 => Din_6211,
  Clk_6212 => Clk_6212,
  Load_6212 => Load_6212,
  Din_6212 => Din_6212,
  datacounter1 => datacounter1,
  Encoder_PB_in => Encoder_PB_in,
    -- input output expander drive signals using MCP23S17
  IO_CS0 => IO_CS0,
  IO_CS1 => IO_CS1,
  IO_Clk => IO_Clk,
  IO_MOSI => IO_MOSI,
  IO_MISO => IO_MISO,
  IO_Reset => IO_Reset
  );
-------------------------------------------------------
Inst_Pwm_1 : Pwm PORT MAP(
  Rst => RST,
  Clk => MClk,
  PwmClk => iPwmClk,
  PwmDimValue => iPanelDimValue,
  PwmCtrl => PanelDrive
  );
----------------------------------------------------------
Inst_ClockDivider_1 : ClockDivider PORT MAP(
    MClk => MClk,
    PwmClk => iPwmClk,
    UcClk => UcClk
    );
-------------------------------------------------------
Inst_Display_Pwm_1 : Display_Pwm PORT MAP(
  Rst => RST,
  Clk => MClk,
  PwmClk => iPwmClk,
  PwmDimValue => iD6211DimValue,
  PwmCtrl => OE_6211
  );
-------------------------------------------------------
--Address latch for data input from microcontroller------
Inst_address_latch: address_latch PORT MAP(
  MClk => MClk,
  Rst => Rst,
  Ale => Ale,
  AddrData => AddrData,
  Address => Address
);
----------------------------------------------------------
	Inst_state_machine_1: state_machine PORT MAP(
		master_clock => MClk,
		logic_reset => RST,
		A => outA1,
		B => outB1,
		dir => dir1,
		tclk => tclk1
	);

	Inst_ucounter_1: ucounter PORT MAP(
		master_clock => MClk,
		logic_reset => RST,
		dir => dir1,
		ce => tclk1,
		Dout => datacounter1 
	);

	Inst_filter_1: filter PORT MAP(
		master_clock => MClk,
		logic_reset => RST,
		A => A1,
		B => B1,
		outA => outA1,
		outB => outB1
	);
----------------------------------------------------------
end Behavioral;