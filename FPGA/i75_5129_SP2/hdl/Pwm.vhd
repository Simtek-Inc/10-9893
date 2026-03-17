library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Pwm is
Port (
  Rst : in std_logic;
  Clk : in std_logic;
  PwmClk : in std_logic;
  PwmDimValue : in std_logic_vector(7 downto 0);
  PwmCtrl : out std_logic
  );

end Pwm;

architecture Behavioral of Pwm is

constant PWM_OFF : std_logic := '0';
constant PWM_ON : std_logic := '1';

--signal PwmCount : unsigned(16 downto 0);
signal PwmCount : unsigned(8 downto 0);
signal iPwmCtrl : std_logic;

begin

PwmCountProcess:
process(PwmClk,PwmCount)
begin
  if(Rst = '1') then
    PwmCount <= (others => '0');
  elsif(rising_edge(PwmClk)) then
    if(PwmCount = 300)then
      PwmCount <= (others => '0');
    else
      PwmCount <= PwmCount + 1;
    end if;
  end if;
end process PwmCountProcess;

PwmControlProcess:
process(Clk,PwmCount,PwmDimValue)
begin
  if(Rst = '1') then
    iPwmCtrl <= '0';
  elsif(rising_edge(Clk))  then
    if(PwmCount >= unsigned(PwmDimValue)) then
        iPwmCtrl <= PWM_OFF;
      else
        iPwmCtrl <= PWM_ON;
      end if;
  end if;
end process PwmControlProcess;

PwmCtrl <= iPwmCtrl;

end Behavioral;