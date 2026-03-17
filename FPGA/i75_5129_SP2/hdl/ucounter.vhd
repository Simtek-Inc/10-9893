library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ucounter is
Port (     master_clock : in std_logic;
           logic_reset : in std_logic;
           dir : in std_logic;
           ce : in std_logic;
           Dout : out std_logic_vector(7 downto 0));
end ucounter;

architecture Behavioral of ucounter is
signal count : unsigned(7 downto 0);

begin
  seven_bit:
  process(master_clock,logic_reset,dir,ce)
  begin
    if(logic_reset = '1')then
	 count <= "00000000";--reset counter to 0
	  elsif(master_clock'event and master_clock = '1')then
	   if (ce = '1')then
		 if(dir='0')then
		  count <= count + 1;
		     else
		  count <= count - 1;
		 end if;
	   end if;
	 end if;
end process seven_bit;

Dout <= std_logic_vector(count);

end Behavioral;
