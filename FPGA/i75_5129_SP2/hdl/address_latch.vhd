library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity address_latch is
  Port ( 
    MClk : in std_logic;
    Rst : in std_logic;
    Ale : in std_logic;
    AddrData : in std_logic_vector(7 downto 0);
    Address : out std_logic_vector(7 downto 0)
    );
end address_latch;

architecture Behavioral of address_latch is

begin

latch:
process(MClk,Rst,Ale,AddrData)
begin
  if (Rst = '1') then
    Address <= (others => '0');
  elsif (MClk'event and MClk = '1') then
    if (Ale = '1') then
      Address <= AddrData;
    end if;
  end if;
end process latch;

end Behavioral;
