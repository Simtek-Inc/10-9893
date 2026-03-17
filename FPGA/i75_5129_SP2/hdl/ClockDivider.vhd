library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ClockDivider is
  Port (
    MClk : in std_logic;
    PwmClk : out std_logic;
    UcClk : out std_logic
    );
end ClockDivider;


architecture Behavioral of ClockDivider is

--signal MasterCount : unsigned (26 downto 0);
--signal MasterCount : unsigned (15 downto 0);
signal div20Count : unsigned (9 downto 0);
signal iPwmClk : std_logic;

begin

--ClkDivProcess: process(Clk,div20Count,MasterCount)
ClkDivProcess: process(MClk,div20Count)
begin
  if (rising_edge(MClk)) then
    if (div20Count = 1333) then
      div20Count <= (others => '0');
    else
      div20Count <= div20Count + 1;
    end if;
  end if;
end process ClkDivProcess;
UcClk <= div20Count(0);

PwmClkGeneration: process(MClk,div20Count)
begin
  if (rising_edge(MClk)) then
    if (div20Count < 666) then
      iPwmClk <= '0';
    else
      iPwmClk <= '1';
    end if;
  end if;
end process PwmClkGeneration;

PwmClk <= IPwmClk;


--HeartBeatGeneration : process(Clk,MasterCount)
--begin
--  if (rising_edge(Clk)) then
--    if ((MasterCount < 10000000) or 
--      ((MasterCount > 20000000) and (MasterCount < 30000000))) then
--      HeartBeat <= '0';
--    else
--      HeartBeat <= '1';
--    end if;
--  end if;
--end process HeartBeatGeneration;
end Behavioral;

