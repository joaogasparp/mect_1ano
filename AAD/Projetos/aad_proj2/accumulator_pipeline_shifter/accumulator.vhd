library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity accumulator is
  generic(
    ADDR_BITS      : integer range 2 to 8 := 4;
    DATA_BITS_LOG2 : integer range 2 to 5 := 3
  );
  port(
    clock       : in  std_logic;

    -- Write port
    write_addr  : in  std_logic_vector(ADDR_BITS-1 downto 0);
    write_inc   : in  std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
    write_shift : in  std_logic_vector(DATA_BITS_LOG2-1 downto 0);

    -- Read port
    read_addr   : in  std_logic_vector(ADDR_BITS-1 downto 0);
    read_data   : out std_logic_vector(2**DATA_BITS_LOG2-1 downto 0)
  );
end accumulator;

architecture pipelined of accumulator is

  -----------------------------------------------------------------------------
  -- Signals for Pipeline Registers (Stage 1 -> Stage 2)
  -----------------------------------------------------------------------------
  signal p1_write_addr_reg   : std_logic_vector(ADDR_BITS-1 downto 0) 
                               := (others => '0');
  signal p1_old_value_reg    : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0) 
                               := (others => '0');
  signal p1_shifted_inc_reg  : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0)
                               := (others => '0');

  -----------------------------------------------------------------------------
  -- Other Signals
  -----------------------------------------------------------------------------
  signal s_current_value     : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_shifted_inc       : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_sum               : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_carry_out         : std_logic;
  signal write_enable        : std_logic := '1';

  -- Extended shift bits (if your barrel shifter expects 5 bits for 8-bit data)
  -- In your code, "00" & write_shift was used; you can adapt as needed.
  signal s_extended_shift    : std_logic_vector(4 downto 0);

  -- Outputs from the main RAM and aux RAM
  signal s_ram_read_data     : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_aux_read_data     : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);

begin

  -----------------------------------------------------------------------------
  -- Create Extended Shift for the barrel shifter input
  -----------------------------------------------------------------------------
  s_extended_shift <= "00" & write_shift; 
  -- Adjust if your barrel_shifter entity expects a different # of shift bits

  -----------------------------------------------------------------------------
  -- BARREL SHIFTER (Stage 1 operation)
  -----------------------------------------------------------------------------
  shifter : entity work.barrel_shifter
    generic map(
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map(
      data_in  => write_inc,
      shift    => s_extended_shift,
      data_out => s_shifted_inc
    );

  -----------------------------------------------------------------------------
  -- DUAL_PORT_RAM #1 (MAIN RAM)
  -----------------------------------------------------------------------------
  ram : entity work.dual_port_ram(asynchronous_new)
    generic map(
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map(
      clock      => clock,
      write_addr => p1_write_addr_reg,  -- from pipeline reg (stage 2)
      write_data => s_sum,
      write_en   => write_enable,
      read_addr  => read_addr,         -- from external read port
      read_data  => s_ram_read_data
    );

  -----------------------------------------------------------------------------
  -- DUAL_PORT_RAM #2 (AUX RAM)
  -----------------------------------------------------------------------------
  aux_ram : entity work.dual_port_ram(asynchronous_new)
    generic map(
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map(
      clock      => clock,
      write_addr => p1_write_addr_reg,  -- from pipeline reg (stage 2)
      write_data => s_sum,
      write_en   => write_enable,
      read_addr  => write_addr,         -- stage 1 read uses *current* write_addr
      read_data  => s_aux_read_data
    );

  -----------------------------------------------------------------------------
  -- PIPELINE REGISTER: Stage 1 -> Stage 2
  --   Capture:
  --     - Old value from aux_ram
  --     - Shifted increment
  --     - Current write_addr
  -----------------------------------------------------------------------------
  process(clock)
  begin
    if rising_edge(clock) then
      -- Latch the "old value" read in Stage 1
      p1_old_value_reg   <= s_aux_read_data;

      -- Latch the shifted increment
      p1_shifted_inc_reg <= s_shifted_inc;

      -- Latch the address (so we know where to write in Stage 2)
      p1_write_addr_reg  <= write_addr;
    end if;
  end process;

  -----------------------------------------------------------------------------
  -- STAGE 2: ADDER
  --   Add the latched old value + latched shifted increment.
  --   Then we write 's_sum' to both RAMs at p1_write_addr_reg.
  -----------------------------------------------------------------------------
  adder : entity work.adder_n
    generic map(
      N => 2**DATA_BITS_LOG2
    )
    port map(
      a     => p1_old_value_reg,
      b     => p1_shifted_inc_reg,
      c_in  => '0',
      s     => s_sum,
      c_out => s_carry_out
    );

  -----------------------------------------------------------------------------
  -- READ-DATA OUTPUT (with simple bypass if needed)
  --   If read_addr = p1_write_addr_reg (the address being written this cycle),
  --   then output s_sum instead of s_ram_read_data.
  -----------------------------------------------------------------------------
  process(all)
  begin
    if (read_addr = p1_write_addr_reg) then
      -- Bypass new data if reading the same address being written in Stage 2
      read_data <= s_sum;
    else
      -- Normal read
      read_data <= s_ram_read_data;
    end if;
  end process;

end pipelined;
