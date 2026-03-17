library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity RamMem is
  Port ( 
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
    Din_6212 : out std_logic;
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
end RamMem;

architecture Behavioral of RamMem is

signal testbyte : std_logic_vector(7 downto 0);
signal rev1,rev2,rev3,rev4 : std_logic_vector(7 downto 0);

signal iPanelDimValue : std_logic_vector(7 downto 0);
signal iD6211DimValue : std_logic_vector(7 downto 0);

begin
--/program REV/-----------------------
--/75-5129 rev -/---------------------
rev1 <= X"75";
rev2 <= X"51";
rev3 <= X"29";
rev4 <= X"2D";
--------------------------------------

Write_Data:
--process(master_clock,reset,wwrite,address,data)
process(MClk,Rst,Wr,Address,AddrData)
begin
  if (Rst = '1') then
    testbyte <= X"77";
    iPanelDimValue <= X"00";
    iD6211DimValue <= X"00";
    IO_CS0  <= '1';
    IO_CS1  <= '1';
    IO_Clk  <= '0';
    IO_MOSI  <= '0';
    IO_Reset  <= '1';
  elsif (rising_edge(MClk)) then
    if (Wr = '0') then
      case (Address) is 
        when X"01" => testbyte <= AddrData;
        when X"10" => iD6211DimValue <= AddrData;
        when X"11" => iPanelDimValue <= AddrData;
-------------------------------------------------        
        when X"40" => Clk_6211 <= AddrData(0);
        when X"41" => Load_6211 <= AddrData(0);
        when X"42" => Din_6211 <= AddrData(0);
        when X"43" => Clk_6212 <= AddrData(0);
        when X"44" => Load_6212 <= AddrData(0);
        when X"45" => Din_6212 <= AddrData(0);
-------------------------------------------------
        when X"23" => IO_CS0  <= AddrData(0);
        when X"24" => IO_CS1  <= AddrData(0);
        when X"25" => IO_Clk  <= AddrData(0);
        when X"26" => IO_MOSI  <= AddrData(0);
        when X"27" => IO_Reset  <= AddrData(0);
        when others => null;
      end case;
    end if;
  end if;
end process Write_Data;


DataAssignmentProcess:
process(Rst,MClk,iPanelDimValue,iD6211DimValue)
begin
  if(rising_edge(MClk))then
    if(Rst = '1') then
      PanelDimValue <= (others => '0');
      D6211DimValue <= (others => '0');
  	else
      PanelDimValue <= iPanelDimValue;
      D6211DimValue <= iD6211DimValue;
    end if;
  end if;
end process DataAssignmentProcess;

Read_Data:
process(MClk,Rd,Address,rev1,rev2,rev3,rev4,testbyte,iPanelDimValue,iD6211DimValue,datacounter1,Encoder_PB_in,IO_MISO)
begin
  if (MClk'event and MClk = '1') then
    if (Rd = '0') then
      case (Address) is 
        when X"01" => AddrData <= testbyte;
        when X"02" => AddrData <= rev1;--program number 
        when X"03" => AddrData <= rev2;-- 
        when X"04" => AddrData <= rev3;-- 
        when X"05" => AddrData <= rev4;-- 
        when X"22" => AddrData <= iPanelDimValue;
        when X"23" => AddrData <= iD6211DimValue;
        when X"60" => AddrData(7 downto 0) <= datacounter1;
        when x"61" => AddrData(0) <= Encoder_PB_in;
        when X"28" =>  AddrData  <= "0000000" & IO_MISO; 
        when others => AddrData <= X"00";
        end case;
    else
      AddrData <= (others => 'Z');
    end if;
  end if;
end process Read_Data;

end Behavioral;
