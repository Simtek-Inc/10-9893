library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity talk_to_micro is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           master_clock_out : out std_logic;
           hb : out std_logic);
end talk_to_micro;

architecture Behavioral of talk_to_micro is

signal master_count : unsigned(22 downto 0);

begin

set_heart_beat:
process(master_count)
  begin
    if(master_count > 8000000)then
	   hb <= '0';
	 else
	   hb <= '1';
	 end if;
end process set_heart_beat;

set_count:
process(master_clock)
  begin
    if(master_clock'event and master_clock = '1')then
	   master_count <= master_count + 1;
	 end if;
end process set_count;

set_up_clock_for_download:
process(logic_reset,master_count)
  begin
    if(logic_reset = '1')then
	   master_clock_out <= master_count(1);
	 else
	   master_clock_out <= master_count(0);
	 end if;
end process set_up_clock_for_download;


end Behavioral;
