
-- VHDL Instantiation Created from source file clock_divide.vhd -- 08:41:58 09/11/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT clock_divide
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;          
		pwm_clock : OUT std_logic
		);
	END COMPONENT;

	Inst_clock_divide: clock_divide PORT MAP(
		master_clock => ,
		logic_reset => ,
		pwm_clock => 
	);


