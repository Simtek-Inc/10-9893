
-- VHDL Instantiation Created from source file ucounter.vhd -- 11:59:32 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT ucounter
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		dir : IN std_logic;
		tclk : IN std_logic;          
		Dout : OUT std_logic_vector(11 downto 0)
		);
	END COMPONENT;

	Inst_ucounter: ucounter PORT MAP(
		master_clock => ,
		logic_reset => ,
		dir => ,
		tclk => ,
		Dout => 
	);


