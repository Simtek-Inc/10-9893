library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity backlight_pwm is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           pwm_clock : in std_logic;
           pwm_count : in std_logic_vector(7 downto 0);
           dim : out std_logic);
end backlight_pwm;

architecture Behavioral of backlight_pwm is

signal comparator : unsigned(8 downto 0);

begin

compare_counter:
process(pwm_clock,logic_reset)
  begin
    if(logic_reset = '1')then
	   comparator <= (others => '0');
	 elsif(pwm_clock'event and pwm_clock = '1')then
	   if(comparator = 256)then
		  comparator <= (others =>'0');
		else
		  comparator <= comparator + '1';
		end if;
	 end if;
end process compare_counter;

signal_generation:
process(master_clock,logic_reset,comparator,pwm_count)
  begin
    if(logic_reset = '1')then
	   dim <= 'Z';
	 elsif(master_clock'event and master_clock = '1')then
	   if(comparator > unsigned(pwm_count)or pwm_count = "00000000")then
		  dim <= 'Z';
		else
		  dim <= '0';
		end if;
	 end if;
end process signal_generation; 

end Behavioral;
