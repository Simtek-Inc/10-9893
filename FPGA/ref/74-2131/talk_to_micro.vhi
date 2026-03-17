
-- VHDL Instantiation Created from source file talk_to_micro.vhd -- 09:24:43 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT talk_to_micro
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;          
		master_clock_out : OUT std_logic;
		hb : OUT std_logic
		);
	END COMPONENT;

	Inst_talk_to_micro: talk_to_micro PORT MAP(
		master_clock => ,
		logic_reset => ,
		master_clock_out => ,
		hb => 
	);


