
-- VHDL Instantiation Created from source file address_latch.vhd -- 09:09:44 09/10/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT address_latch
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		ale : IN std_logic;
		add_data : IN std_logic_vector(7 downto 0);          
		address : OUT std_logic_vector(7 downto 0)
		);
	END COMPONENT;

	Inst_address_latch: address_latch PORT MAP(
		master_clock => ,
		logic_reset => ,
		ale => ,
		add_data => ,
		address => 
	);


