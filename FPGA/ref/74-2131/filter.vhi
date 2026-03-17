
-- VHDL Instantiation Created from source file filter.vhd -- 11:51:35 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

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

	Inst_filter: filter PORT MAP(
		master_clock => ,
		logic_reset => ,
		Ain => ,
		Bin => ,
		outA => ,
		outB => 
	);


