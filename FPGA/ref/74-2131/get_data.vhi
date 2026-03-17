
-- VHDL Instantiation Created from source file get_data.vhd -- 09:47:15 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT get_data
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		read : IN std_logic;
		write : IN std_logic;
		address : IN std_logic_vector(7 downto 0);
		switch_data : IN std_logic_vector(7 downto 0);    
		data : INOUT std_logic_vector(7 downto 0);      
		flag_ctrl : OUT std_logic
		);
	END COMPONENT;

	Inst_get_data: get_data PORT MAP(
		master_clock => ,
		logic_reset => ,
		read => ,
		write => ,
		address => ,
		flag_ctrl => ,
		switch_data => ,
		data => 
	);


