library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity top_level_74_2131 is
    Port ( master_clock : in std_logic;
           master_clock_out : out std_logic;
           hb : out std_logic;
           logic_reset : in std_logic;
           read : in std_logic;
           write : in std_logic;
           ale : in std_logic;
           add_data : inout std_logic_vector(7 downto 0);
           switch_data1 : in std_logic_vector(7 downto 0);
			  switch_data2 : in std_logic_vector(7 downto 0);
			  switch_data3 : in std_logic_vector(7 downto 0);
			  switch_data4 : in std_logic_vector(7 downto 0);
			  solenoid_1 : out std_logic;
			  solenoid_2 : out std_logic;
			  Ain1 : in std_logic;
			  Bin1 : in std_logic;			  
			  Ain2 : in std_logic;
			  Bin2 : in std_logic;
			  Ain3 : in std_logic;
			  Bin3 : in std_logic;			  
			  Ain4 : in std_logic;
			  Bin4 : in std_logic;
			  i_odim1 : out std_logic;
			  i_odim2 : out std_logic;
			  i_odim3 : out std_logic;
			  i_odim4 : out std_logic
			  );
end top_level_74_2131;

architecture Behavioral of top_level_74_2131 is
-----------------------------------------------
COMPONENT address_latch
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		ale : IN std_logic;
		add_data : IN std_logic_vector(7 downto 0);          
		address : OUT std_logic_vector(7 downto 0)
		);
	END COMPONENT;
-----------------------------------------------
COMPONENT talk_to_micro
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;          
		master_clock_out : OUT std_logic;
		hb : OUT std_logic
		);
	END COMPONENT;
----------------------------------------------
COMPONENT get_data
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		read : IN std_logic;
		write : IN std_logic;
		address : IN std_logic_vector(7 downto 0);
		data : INOUT std_logic_vector(7 downto 0);
		ann1dimbyte : out std_logic_vector(7 downto 0);
		ann2dimbyte : out std_logic_vector(7 downto 0);
		ann3dimbyte : out std_logic_vector(7 downto 0);
		ann4dimbyte : out std_logic_vector(7 downto 0);
		switch_data1 : IN std_logic_vector(7 downto 0);
      switch_data2 : IN std_logic_vector(7 downto 0);
		switch_data3 : IN std_logic_vector(7 downto 0);
		switch_data4 : IN std_logic_vector(7 downto 0);
		solenoid_1 : out std_logic;
		solenoid_2 : out std_logic;
	   datacounter1 : in std_logic_vector(11 downto 0);
      datacounter2 : in std_logic_vector(11 downto 0);
      datacounter3 : in std_logic_vector(11 downto 0);
      datacounter4 : in std_logic_vector(11 downto 0)--;
		);
	END COMPONENT;
-----------------------------------------------
COMPONENT state_machine
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		outA : IN std_logic;
		outB : IN std_logic;          
		dir : OUT std_logic;
		tclk : OUT std_logic
		);
	END COMPONENT;
-------------------------------------------------
COMPONENT filter
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		Ain : IN std_logic;
		Bin : IN std_logic;          
		outA : OUT std_logic;
		outB : OUT std_logic
		);
	END COMPONENT;
-----------------------------------------------	
COMPONENT ucounter
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		dir : IN std_logic;
		tclk : IN std_logic;          
		Dout : OUT std_logic_vector(11 downto 0)
		);
	END COMPONENT;
------------------------------------------------
COMPONENT backlight_pwm
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		pwm_clock : IN std_logic;
		pwm_count : IN std_logic_vector(7 downto 0);          
		dim : OUT std_logic
		);
	END COMPONENT;
------------------------------------------------
	COMPONENT clock_divide
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;          
		pwm_clock : OUT std_logic
		);
	END COMPONENT;
-----------------------------------------------
signal address : std_logic_vector(7 downto 0);
signal datacounter1 : std_logic_vector(11 downto 0);
signal datacounter2 : std_logic_vector(11 downto 0);
signal datacounter3 : std_logic_vector(11 downto 0);
signal datacounter4 : std_logic_vector(11 downto 0);
signal outA1,outB1,dir1,tclk1 : std_logic;
signal outA2,outB2,dir2,tclk2 : std_logic;
signal outA3,outB3,dir3,tclk3 : std_logic;
signal outA4,outB4,dir4,tclk4 : std_logic;
signal ann1dimbyte : std_logic_vector(7 downto 0);
signal ann2dimbyte : std_logic_vector(7 downto 0);
signal ann3dimbyte : std_logic_vector(7 downto 0);
signal ann4dimbyte : std_logic_vector(7 downto 0);
signal pwm_clock1 : std_logic;
signal pwm_clock2 : std_logic;
signal pwm_clock3 : std_logic;
signal pwm_clock4 : std_logic;

begin

Inst_address_latch: address_latch PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		ale => ale,
		add_data => add_data,
		address => address
	);

Inst_talk_to_micro: talk_to_micro PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		master_clock_out => master_clock_out,
		hb => hb
	);

Inst_get_data: get_data PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		read => read,
		write => write,
		address => address,
		solenoid_1 => solenoid_1,
		solenoid_2 => solenoid_2,
		switch_data1 => switch_data1,
      switch_data2 => switch_data2,
      switch_data3 => switch_data3,
      switch_data4 => switch_data4,
		data => add_data,
      datacounter1 => datacounter1,
	   datacounter2 => datacounter2,
      datacounter3 => datacounter3,
      datacounter4 => datacounter4,
		ann1dimbyte => ann1dimbyte,
		ann2dimbyte => ann2dimbyte,
		ann3dimbyte => ann3dimbyte,
		ann4dimbyte => ann4dimbyte
	);
---------------------------------------------------
Inst_state_machine_1: state_machine PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		outA => outA1,
		outB => outB1,
		dir => dir1,
		tclk => tclk1
	);
------------------------------------------------------
Inst_filter_1: filter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		Ain => Ain1,
		Bin => Bin1,
		outA => outA1,
		outB => outB1
	);
---------------------------------------------------------------	
Inst_ucounter_1: ucounter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		dir => dir1,
		tclk => tclk1,
		Dout => datacounter1
	);
-----------------------------------------------------------------	
Inst_state_machine_2: state_machine PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		outA => outA2,
		outB => outB2,
		dir => dir2,
		tclk => tclk2
	);
---------------------------------------------------------
Inst_filter_2: filter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		Ain => Ain2,
		Bin => Bin2,
		outA => outA2,
		outB => outB2
	);
----------------------------------------------------------	
Inst_ucounter_2: ucounter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		dir => dir2,
		tclk => tclk2,
		Dout => datacounter2
	);	
-----------------------------------------------------------------	
Inst_state_machine_3: state_machine PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		outA => outA3,
		outB => outB3,
		dir => dir3,
		tclk => tclk3
	);
-------------------------------------------------------
Inst_filter_3: filter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		Ain => Ain3,
		Bin => Bin3,
		outA => outA3,
		outB => outB3
	);
--------------------------------------------------------------	
Inst_ucounter_3: ucounter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		dir => dir3,
		tclk => tclk3,
		Dout => datacounter3
	);	
----------------------------------------------------	
Inst_state_machine_4: state_machine PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		outA => outA4,
		outB => outB4,
		dir => dir4,
		tclk => tclk4
	);
-------------------------------------------------------------
Inst_filter_4: filter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		Ain => Ain4,
		Bin => Bin4,
		outA => outA4,
		outB => outB4
	);
-------------------------------------------------------------------	
Inst_ucounter_4: ucounter PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		dir => dir4,
		tclk => tclk4,
		Dout => datacounter4
	);	
------------------------------------------------------------------
	Inst_autopilot_2_pwm: backlight_pwm PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock1,
		pwm_count => ann1dimbyte,
		dim => i_odim1
	);	
------------------------------------------------------------------
	Inst_autopilot_1_pwm: backlight_pwm PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock2,
		pwm_count => ann2dimbyte,
		dim => i_odim2
	);	
------------------------------------------------------------------
	Inst_copilot_id_pwm: backlight_pwm PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock3,
		pwm_count => ann3dimbyte,
		dim => i_odim3
	);
------------------------------------------------------------------
	Inst_pilot_id_pwm: backlight_pwm PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock4,
		pwm_count => ann4dimbyte,
		dim => i_odim4
	);
--------------------------------------------------------------------
	Inst_clock_divide1: clock_divide PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock1
	);
-------------------------------------------------------------
	Inst_clock_divide2: clock_divide PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock2
	);
--------------------------------------------------------------
	Inst_clock_divide3: clock_divide PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock3
	);
--------------------------------------------------------------
	Inst_clock_divide4: clock_divide PORT MAP(
		master_clock => master_clock,
		logic_reset => logic_reset,
		pwm_clock => pwm_clock4
	);
-------------------------------------------
end Behavioral;
