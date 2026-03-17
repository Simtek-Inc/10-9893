
-- VHDL Instantiation Created from source file state_machine.vhd -- 11:26:53 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

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

	Inst_state_machine: state_machine PORT MAP(
		master_clock => ,
		logic_reset => ,
		outA => ,
		outB => ,
		dir => ,
		tclk => 
	);


