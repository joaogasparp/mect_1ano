library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity accumulator is
  generic
  (
    ADDR_BITS      : integer range 2 to 8 := 4;
    DATA_BITS_LOG2 : integer range 2 to 5 := 3
  );
  port
  (
    clock       : in  std_logic;
    -- write port
    write_addr  : in  std_logic_vector(ADDR_BITS-1 downto 0);
    write_inc   : in  std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
    write_shift : in  std_logic_vector(DATA_BITS_LOG2-1 downto 0);
    -- read port
    read_addr   : in  std_logic_vector(ADDR_BITS-1 downto 0);
    read_data   : out std_logic_vector(2**DATA_BITS_LOG2-1 downto 0)
  );
end accumulator;

architecture structural of accumulator is
  signal s_current_value   : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_shifted_inc     : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_sum            : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_carry_out      : std_logic;
  signal s_extended_shift : std_logic_vector(4 downto 0);  -- para o barrel shifter
  signal write_enable     : std_logic;
begin
  write_enable <= '1';

  s_extended_shift <= "00" & write_shift;

  shifter : entity work.barrel_shifter
    generic map
    (
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map
    (
      data_in  => write_inc,
      shift    => s_extended_shift,
      data_out => s_shifted_inc
    );

  ram : entity work.dual_port_ram(asynchronous_new)
    generic map
    (
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map
    (
      clock      => clock,
      write_addr => write_addr,
      write_data => s_sum,
      write_en   => write_enable,
      read_addr  => read_addr,
      read_data  => read_data
    );

  aux_ram : entity work.dual_port_ram(asynchronous_new)
    generic map
    (
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map
    (
      clock      => clock,
      write_addr => write_addr,
      write_data => s_sum,
      write_en   => write_enable,
      read_addr  => write_addr,
      read_data  => s_current_value
    );

  adder : entity work.adder_n
    generic map
    (
      N => 2**DATA_BITS_LOG2
    )
    port map
    (
      a     => s_current_value,
      b     => s_shifted_inc,
      c_in  => '0',
      s     => s_sum,
      c_out => s_carry_out
    );

end structural;
