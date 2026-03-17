library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity filter is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           Ain : in std_logic;
           Bin : in std_logic;
           outA : out std_logic;
           outB : out std_logic
			  );
end filter;

architecture Behavioral of filter is

signal qav,qbv : std_logic_vector(3 downto 0);
signal ja,ka,jb,kb,qabar,qbbar : std_logic;
signal qaint,qbint : std_logic;

begin

a_debounce:
process(master_clock,Ain)
begin
  if (master_clock'event and master_clock = '1') then
  qav(0) <= Ain;
  qav(1) <= qav(0);
  qav(2) <= qav(1);
  qav(3) <= qav(2);
  end if;
end process a_debounce;

b_debounce:
process(master_clock,Bin)
begin
  if (master_clock'event and master_clock = '1') then
  qbv(0) <= Bin;
  qbv(1) <= qbv(0);
  qbv(2) <= qbv(1);
  qbv(3) <= qbv(2);
  end if;
end process b_debounce;

a_output_generation:
process(qav)
begin
  ja <= qav(1) and qav(2) and qav(3);
  ka <= (not qav(1)) and (not qav(2)) and (not qav(3));
end process a_output_generation;

b_output_generation:
process(qbv)
begin
  jb <= qbv(1) and qbv(2) and qbv(3);
  kb <= (not qbv(1)) and (not qbv(2)) and (not qbv(3));
end process b_output_generation;
  
a_jk_flipflop:
process(master_clock,ja,ka,qaint)
begin
--  if (reset = '1') then
--  qaint <= '0';
  if (master_clock'event and master_clock = '1') then
    if (ja = '1' and ka = '1') then
	 qaint <= not qaint;
    elsif (ja = '1' and ka = '0') then
	 qaint <= '1';
    elsif (ja = '0' and ka = '1') then
	 qaint <= '0';
    end if;
  end if;
  qabar <= not qaint;
end process a_jk_flipflop;

b_jk_flipflop:
process(master_clock,jb,kb,qbint)
begin
--  if (reset = '1') then
--  qbint <= '0';
  if (master_clock'event and master_clock = '1') then
    if (jb = '1' and kb = '1') then
	 qbint <= not qbint;
    elsif (jb = '1' and kb = '0') then
	 qbint <= '1';
    elsif (jb = '0' and kb = '1') then
	 qbint <= '0';
    end if;
  end if;
  qbbar <= not qbint;
end process b_jk_flipflop;

outA <= qaint;
outB <= qbint;

  
end Behavioral;
