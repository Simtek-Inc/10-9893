
-- VHDL Instantiation Created from source file backlight_pwm.vhd -- 07:43:04 09/11/2008
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT backlight_pwm
	PORT(
		master_clock : IN std_logic;
		logic_reset : IN std_logic;
		pwm_clock : IN std_logic;
		pwm_count : IN std_logic_vector(7 downto 0);          
		dim : OUT std_logic
		);
	END COMPONENT;

	Inst_backlight_pwm: backlight_pwm PORT MAP(
		master_clock => ,
		logic_reset => ,
		pwm_clock => ,
		pwm_count => ,
		dim => 
	);


