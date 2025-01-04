--
-- AAD 2024/2025, accumulator test bench
--

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity accumulator_tb is
end accumulator_tb;

architecture stimulus of accumulator_tb is
  constant ADDR_BITS      : positive := 4;
  constant DATA_BITS_LOG2 : positive := 3;

  signal clock        : std_logic;
  signal s_write_addr : std_logic_vector(ADDR_BITS-1 downto 0);
  signal s_write_inc  : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_read_addr  : std_logic_vector(ADDR_BITS-1 downto 0);
  signal s_read_data  : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);

begin
  uut : entity work.accumulator(pipelined)
    generic map (
      ADDR_BITS      => ADDR_BITS,
      DATA_BITS_LOG2 => DATA_BITS_LOG2
    )
    port map (
      clock      => clock,
      write_addr => s_write_addr,
      write_inc  => s_write_inc,
      read_addr  => s_read_addr,
      read_data  => s_read_data
    );

  clock_stim : process is
  begin
    for cycle in 0 to 10+2+2**ADDR_BITS loop
      clock <= '0';
      wait for 500 ps;
      clock <= '1';
      wait for 500 ps;
    end loop;
    wait;
  end process;

  write_stim : process is
  begin
    s_write_addr <= std_logic_vector(to_unsigned(3, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(7, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(15, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(3, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(3, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(1, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(3, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(7, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(5, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(64, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(4, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(1, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(4, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(16, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(3, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(1, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(10, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(100, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= std_logic_vector(to_unsigned(2, ADDR_BITS));
    s_write_inc  <= std_logic_vector(to_unsigned(17, 2**DATA_BITS_LOG2));
    wait for 1000 ps;

    s_write_addr <= (others => '0');
    s_write_inc  <= (others => '0');
    wait;
  end process;

  read_stim : process is
  begin
    s_read_addr <= std_logic_vector(to_unsigned(3, ADDR_BITS));
    wait for 12*1000 ps;

    for i in 0 to 2**ADDR_BITS-1 loop
      s_read_addr <= std_logic_vector(to_unsigned(i, ADDR_BITS));
      wait for 1000 ps;
    end loop;
    wait;
  end process;

end stimulus;
