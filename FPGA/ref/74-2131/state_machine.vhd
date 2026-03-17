library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity state_machine is
    Port ( master_clock : in std_logic;
           logic_reset : in std_logic;
           outA : in std_logic;--from filter
           outB : in std_logic;--from filter
           dir : out std_logic;--to counter
           tclk : out std_logic--to counter
			  );
end state_machine;

architecture Behavioral of state_machine is

signal cs,ns : std_logic_vector(1 downto 0); -- cs = current state / ns = next state
signal change : std_logic;

begin
	syn_process:
	process(master_clock,logic_reset,ns,outA,outB)--sensitvity list syn process
	 begin
	  if(master_clock'event and master_clock = '1')then -- clock rising edge
	   if(logic_reset = '1')then	--
		   if(outA = '0' and outB = '0')then
			 cs <= "00";								-- encoder is in output state 0
			elsif(outA = '1' and outB = '0')then
			 cs <= "10";								-- encoder is in output state 1
 		   elsif(outA = '1' and outB = '1')then
			 cs <= "11";								-- encoder is in output state 2
			elsif(outA = '0' and outB = '1')then
				cs <= "01";								-- encoder is in output state 3
			end if;
		 else
		 cs <= ns;
		end if;
    end if;
   end process syn_process;

  combination_process:
  process(outA,outB,cs)-- sensitivity list for combination process
   begin
	 case(cs) is -- process will determine based on current state of encoder what the next 
	 				 -- state is and what direction the encoder is being moved
	 when	"00" =>								--state 1
	  if(outA = '1' and outB = '0')then
 	  	ns <= "10";
	  	dir <= '0';-- 
		change <= '1';
	  elsif(outA = '0' and outB = '1')then
	   ns <= "01";
		dir <= '1';								-- 1 direction is for counterclockwise change
		change <= '1';
	  elsif(outA = '0' and outB = '0')then
	   ns <= cs;
		dir <= '0';
		change <= '0';
	  end if;
---------------------------------- 
   when "10" =>								--state 2
	 if(outA = '1' and outB = '1')then
	  	ns <= "11";
	  	dir <= '0'; 							-- 0 direction is for clockwise change
	  	change <= '1';
	 elsif(outA = '0' and outB = '0')then
	  	ns <= "00";
	  	dir <= '1';
	  	change <= '1';
	 elsif(outA = '1' and outB = '0')then
	  	ns <= cs;
	  	dir <= '0';
	  	change <= '0';
	 end if;
----------------------------------
   when "11" =>								--state 3
	 if(outA = '0' and outB = '1')then
		ns <= "01";
	  	dir <= '0'; 							-- 0 direction is for clockwise change
		change <= '1';
	 elsif(outA = '1' and outB = '0')then
	  	ns <= "10";
	  	dir <= '1';
		change <= '1';
	 elsif(outA = '1' and outB = '1')then
	  	ns <= cs;
	  	dir <= '0';
	  	change <= '0';
	 end if;
----------------------------------
   when "01" =>								--state 4
	 if(outA = '0' and outB = '0')then
	  	ns <= "00";
	  	dir <= '0'; 							-- 0 direction is for clockwise change
		change <= '1';
	 elsif(outA = '1' and outB = '1')then
	  	ns <= "11";
	  	dir <= '1';
		change <= '1';
	 elsif(outA = '0' and outB = '1')then
	  	ns <= cs;
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
