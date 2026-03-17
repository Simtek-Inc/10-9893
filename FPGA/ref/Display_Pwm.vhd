library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Display_Pwm is
Port (
  Rst : in std_logic;
  Clk : in std_logic;
  Display_PwmClk : in std_logic;
  Display_PwmDimValue : in std_logic_vector(7 downto 0);
  Display_PwmCtrl : out std_logic
  );

end Display_Pwm;

architecture Behavioral of Display_Pwm is

constant Display_Pwm_OFF : std_logic := '0';
constant Display_Pwm_ON : std_logic := '1';

--signal Display_PwmCount : unsigned(16 downto 0);
signal Display_PwmCount : unsigned(8 downto 0);
signal iDisplay_PwmCtrl : std_logic;

begin

Display_PwmCountProcess:
process(Display_PwmClk,Display_PwmCount)
begin
  if(Rst = '1') then
    Display_PwmCount <= (others => '0');
  elsif(rising_edge(Display_PwmClk)) then
    if(Display_PwmCount = 300)then
      Display_PwmCount <= (others => '0');
    else
      Display_PwmCount <= Display_PwmCount + 1;
    end if;
  end if;
end process Display_PwmCountProcess;

Display_PwmControlProcess:
process(Clk,Display_PwmCount,Display_PwmDimValue)
begin
  if(Rst = '1') then
    iDisplay_PwmCtrl <= '0';
  elsif(rising_edge(Clk))  then
    if(Display_PwmCount >= unsigned(Display_PwmDimValue)) then
        iDisplay_PwmCtrl <= Display_Pwm_OFF;
      else
        iDisplay_PwmCtrl <= Display_Pwm_ON;
      end if;
  end if;
end process Display_PwmControlProcess;

Display_PwmCtrl <= iDisplay_PwmCtrl;

end Behavioral;