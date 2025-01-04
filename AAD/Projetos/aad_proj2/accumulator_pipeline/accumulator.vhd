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
    -- "Write" port: we accumulate at this address
    write_addr  : in  std_logic_vector(ADDR_BITS-1 downto 0);
    write_inc   : in  std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
    -- "Read" port: we want to observe memory contents at this address
    read_addr   : in  std_logic_vector(ADDR_BITS-1 downto 0);
    read_data   : out std_logic_vector(2**DATA_BITS_LOG2-1 downto 0)
  );
end accumulator;

-------------------------------------------------------------------------------
-- Pipelined Architecture
-------------------------------------------------------------------------------
architecture pipelined of accumulator is

  -----------------------------------------------------------------------------
  -- Pipeline Registers
  --   These hold the values from Stage 1 (cycle N) and feed them to Stage 2
  --   (cycle N+1).
  -----------------------------------------------------------------------------
  signal p1_write_addr_reg : std_logic_vector(ADDR_BITS-1 downto 0) := (others => '0');
  signal p1_old_value_reg  : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0) := (others => '0');
  signal p1_write_inc_reg  : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0) := (others => '0');

  -----------------------------------------------------------------------------
  -- Other Signals
  -----------------------------------------------------------------------------
  signal s_sum           : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_carry_out     : std_logic;
  signal write_enable    : std_logic := '1';

  -- Outputs from RAMs
  signal s_ram_read_data : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);
  signal s_aux_read_data : std_logic_vector(2**DATA_BITS_LOG2-1 downto 0);

begin

  -----------------------------------------------------------------------------
  -- RAM #1
  -----------------------------------------------------------------------------
  ram : entity work.dual_port_ram(asynchronous_new)
    generic map(
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map(
      clock      => clock,
      -- Write side (Stage 2)
      write_addr => p1_write_addr_reg,
      write_data => s_sum,
      write_en   => write_enable,
      -- Read side (for external read port)
      read_addr  => read_addr,
      read_data  => s_ram_read_data
    );


  -----------------------------------------------------------------------------
  -- RAM #2 (aux_ram)
  -----------------------------------------------------------------------------
  aux_ram : entity work.dual_port_ram(asynchronous_new)
    generic map(
      ADDR_BITS => ADDR_BITS,
      DATA_BITS => 2**DATA_BITS_LOG2
    )
    port map(
      clock      => clock,
      -- Write side (Stage 2)
      write_addr => p1_write_addr_reg,
      write_data => s_sum,
      write_en   => write_enable,
      -- Read side (Stage 1) uses the *current* cycle's write_addr
      read_addr  => write_addr,
      read_data  => s_aux_read_data
    );


  -----------------------------------------------------------------------------
  -- Stage 1 Pipeline Register:
  --   Capture old value + address + increment at the rising edge.
  -----------------------------------------------------------------------------
  process(clock)
  begin
    if rising_edge(clock) then
      p1_write_addr_reg <= write_addr;      -- The address we read *this cycle*
      p1_old_value_reg  <= s_aux_read_data; -- The "old value" read from aux_ram
      p1_write_inc_reg  <= write_inc;       -- The increment we want to add
    end if;
  end process;


  -----------------------------------------------------------------------------
  -- Stage 2: Adder
  --   Add the latched old value + latched increment, then write result to RAMs.
  -----------------------------------------------------------------------------
  adder : entity work.adder_n
    generic map(
      N => 2**DATA_BITS_LOG2
    )
    port map(
      a     => p1_old_value_reg,
      b     => p1_write_inc_reg,
      c_in  => '0',
      s     => s_sum,
      c_out => s_carry_out
    );


  -----------------------------------------------------------------------------
  -- Hazard/BYPASS LOGIC for read_data:
  --   If read_addr == the address being written *this cycle* (Stage 2),
  --   output the newly computed sum instead of the old data from ram.
  -----------------------------------------------------------------------------
  process(all)
  begin
    if (read_addr = p1_write_addr_reg) then
      -- Bypass the new data
      read_data <= s_sum;
    else
      -- Normal read
      read_data <= s_ram_read_data;
    end if;
  end process;

end pipelined;
