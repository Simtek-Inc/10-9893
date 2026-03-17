library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity address_latch is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           ale : in std_logic;
           add_data : in std_logic_vector(7 downto 0);
           address : out std_logic_vector(7 downto 0));
end address_latch;

architecture Behavioral of address_latch is

begin

latch:
process(master_clock,logic_reset,ale,add_data)
  begin
    if(logic_reset = '1')then
	   address <= (others => 'Z');
	 elsif(master_clock'event and master_clock = '1')then
	   if(ale = '1')then
		  address <= add_data;
		end if;
	 end if;
end process latch;

end Behavioral;
