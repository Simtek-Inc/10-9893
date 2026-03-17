library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity clock_divide is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           pwm_clock : out std_logic);
end clock_divide;

architecture Behavioral of clock_divide is

signal pwm_count : unsigned(11 downto 0);

begin

pwm_counting:
process(master_clock,logic_reset)
  begin
    if(logic_reset = '1')then
	   pwm_count <= (others =>'0');
	 elsif(master_clock'event and master_clock = '1')then
	   if(pwm_count = 2048)then
		  pwm_count <= (others => '0');
		else
		  pwm_count <= pwm_count + '1';
		end if;
	 end if;
end process pwm_counting;


pwm_generate:
process(master_clock,logic_reset,pwm_count)
  begin
    if(logic_reset = '1')then
	   pwm_clock <= '0';
	 elsif(master_clock'event and master_clock = '1')then
	   if(pwm_count < 1024)then
		  pwm_clock <= '0';
		else
		  pwm_clock <= '1';
		end if;
	 end if;
end process pwm_generate;

end Behavioral;
