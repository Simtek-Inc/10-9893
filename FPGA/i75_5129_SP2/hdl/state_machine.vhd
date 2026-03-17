library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity state_machine is
Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           A : in std_logic;
           B : in std_logic;
           dir : out std_logic;
           tclk : out std_logic);
end state_machine;

architecture Behavioral of state_machine is

signal cs,ns : std_logic_vector(1 downto 0); -- cs = current state / ns = next state
signal change : std_logic;
begin
	syn_process:
	process(master_clock,logic_reset,ns,A,B)--sensitvity list syn process
	 begin
	  if(master_clock'event and master_clock = '1')then -- clock rising edge
	   if(logic_reset = '1')then	--
		   if(A = '0' and B = '0')then
			 cs <= "00";-- encoder is in output state 1
			elsif(A = '1' and B = '0')then
			 cs <= "10";-- encoder is in output state 2
 		   elsif(A = '1' and B = '1')then
			 cs <= "11";-- encoder is in output state 3
			   else
				cs <= "01";-- encoder is in output state 4
			end if;
		 else
		 cs <= ns;
		end if;
    end if;
   end process syn_process;

  combination_process:
  process(A,B,cs)-- sensitivity list for combination process
   begin
	 case(cs) is -- process will determine based on current state of encoder what the next 
	 				 -- state is and what direction the encoder is being moved
	 when	"00" =>--state 1
	  if(A = '1' and B = '0')then
 	  	ns <= "10";
	  	dir <= '0';-- 
	  	change <= '1';
	  elsif(A = '0' and B = '1')then
	   ns <= "01";
		dir <= '1';-- 1 direction is for counterclockwise change
		change <= '1';
	  elsif(A = '0' and B = '0')then
	   ns <= "00";
		dir <= '0';
		change <= '0';
	  end if;
---------------------------------- 
   when "10" =>--state 2
	 if(A = '1' and B = '1')then
	  ns <= "11";
	  dir <= '0'; -- 0 direction is for clockwise change
	  change <= '1';
	 elsif(A = '0' and B = '0')then
	  ns <= "00";
	  dir <= '1';
	  change <= '1';
	 elsif(A = '1' and B = '0')then
	  ns <= "10";
	  dir <= '0';
	  change <= '0';
	 end if;
----------------------------------
   when "11" =>--state 3
	 if(A = '0' and B = '1')then
	  ns <= "01";
	  dir <= '0'; -- 0 direction is for clockwise change
	  change <= '1';
	 elsif(A = '1' and B = '0')then
	  ns <= "10";
	  dir <= '1';
	  change <= '1';
	 elsif(A = '1' and B = '1')then
	  ns <= "11";
	  dir <= '0';
	  change <= '0';
	 end if;
----------------------------------
   when "01" =>--state 4
	 if(A = '0' and B = '0')then
	  ns <= "00";
	  dir <= '0'; -- 0 direction is for clockwise change
	  change <= '1';
	 elsif(A = '1' and B = '1')then
	  ns <= "11";
	  dir <= '1';
	  change <= '1';
	 elsif(A = '0' and B = '1')then
	  ns <= "01";
	  dir <= '0';
	  change <= '0';
	 end if;
----------------------------------
	when others => ns <= cs;
					dir <= '0';
					change <= '0';
	end case;
end process combination_process;

tclk <= change;

end Behavioral;
