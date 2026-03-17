library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ucounter is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           dir : in std_logic;
           tclk : in std_logic;
           Dout : out std_logic_vector(11 downto 0));
end ucounter;

architecture Behavioral of ucounter is


signal count : STD_LOGIC_VECTOR(11 downto 0);

begin
  twelve_bit:
  process(master_clock,logic_reset,dir,tclk,count)
  begin
    if(logic_reset = '1')then
		count <= "000000000000";				--reset counter to 0
	elsif(master_clock'event and master_clock = '1')then
--------------------------------
      if(tclk = '1')then
		 if(dir='1')then			--0 indicates a clockwise change if(dir='0')then
		  count <= count + 1;	--add 1 to current count value
------------------------------------
		  elsif
		(tclk = '1')then
		 if(dir='0')then			-- 1 indicates a counter clockwise change	if(dir='1')then							
        count <= count - 1;	--subtract 1 from current count value
		end if;
	    end if;
---------------------------------------
		end if;
     end if;
end process twelve_bit;

Dout <= count;

end Behavioral;
