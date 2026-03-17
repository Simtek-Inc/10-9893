library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity get_data is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           read : in std_logic;
           write : in std_logic;
           address : in std_logic_vector(7 downto 0);
			  data : inout std_logic_vector(7 downto 0);
           flag_ctrl : out std_logic;
			  solenoid_1 : out std_logic;
			  solenoid_2 : out std_logic;
           ann1dimbyte : out std_logic_vector(7 downto 0);
			  ann2dimbyte : out std_logic_vector(7 downto 0);
			  ann3dimbyte : out std_logic_vector(7 downto 0);
			  ann4dimbyte : out std_logic_vector(7 downto 0);
			  switch_data1 : in std_logic_vector(7 downto 0);
			  switch_data2 : in std_logic_vector(7 downto 0);
			  switch_data3 : in std_logic_vector(7 downto 0);
			  switch_data4 : in std_logic_vector(7 downto 0);
			  datacounter1 : in std_logic_vector(11 downto 0);
			  datacounter2 : in std_logic_vector(11 downto 0);
           datacounter3 : in std_logic_vector(11 downto 0);
           datacounter4 : in std_logic_vector(11 downto 0)
           );
end get_data;

architecture Behavioral of get_data is

begin

dread:
process(master_clock,logic_reset,read,address)
  begin
   if(logic_reset = '1')then
	  data <= (others => 'Z');
	elsif(master_clock'event and master_clock = '1')then
	  if(read = '0')then
	    case(address)is
		   when"00000001" => data <= switch_data1;--1
			when"00000010" => data <= switch_data2;--2
			when"00000011" => data <= switch_data3;--3
			when"00000100" => data <= switch_data4;--4
			when"00000101" => data(5 downto 0) <= datacounter1(5 downto 0);--5
			when"00000110" => data(5 downto 0) <= datacounter1(11 downto 6);--6
			when"00000111" => data(5 downto 0) <= datacounter2(5 downto 0);--7
			when"00001000" => data(5 downto 0) <= datacounter2(11 downto 6);--8
			when"00001001" => data(5 downto 0) <= datacounter3(5 downto 0);--9
			when"00001010" => data(5 downto 0) <= datacounter3(11 downto 6);--A
			when"00001011" => data(5 downto 0) <= datacounter4(5 downto 0);--B
			when"00001100" => data(5 downto 0) <= datacounter4(11 downto 6);--C
			when others => data <= "00000000";
		 end case;
		   else
			  data <=(others => 'Z');
		end if;
	 end if;
end process dread;

dwrite:
process(master_clock,logic_reset,write,address,data)
  begin
    if(logic_reset = '1')then
	   flag_ctrl <= '1';
	 elsif(master_clock'event and master_clock = '1')then
	   if(write = '0')then
		  case(address)is
		    when "10000001" => 	solenoid_1 <= data(0);--pitch solenoid
			 						  	solenoid_2 <= data(1);--lateral solenoid
		    when "10000010" => 	ann1dimbyte <= data;--autopilot 2
			 when "10000011" => 	ann2dimbyte <= data;--autopilot 1
			 when "10000100" => 	ann3dimbyte <= data;--copilot id
			 when "10000101" => 	ann4dimbyte <= data;--pilot id
			when others => null;
		  end case;
		end if;
	 end if;
end process dwrite;

end Behavioral;
