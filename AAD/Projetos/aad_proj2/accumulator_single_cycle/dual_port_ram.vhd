library IEEE;
use     IEEE.std_logic_1164.all;
use     IEEE.numeric_std.all;

entity dual_port_ram is
  generic
  (
    ADDR_BITS : integer range 2 to 16;
    DATA_BITS : integer range 1 to 32
  );
  port
  (
    clock        : in std_logic;
    -- write port
    write_addr : in  std_logic_vector(ADDR_BITS-1 downto 0);
    write_data : in  std_logic_vector(DATA_BITS-1 downto 0);
    write_en   : in  std_logic;
    -- read port
    read_addr  : in  std_logic_vector(ADDR_BITS-1 downto 0);
    read_data  : out std_logic_vector(DATA_BITS-1 downto 0)
  );
end dual_port_ram;

architecture synchronous_old of dual_port_ram is
  type ram_t is array(0 to 2**ADDR_BITS-1) of std_logic_vector(DATA_BITS-1 downto 0);
  signal ram : ram_t := (others => (others => '0'));
begin
  -- synchronous
  process(clock) is
  begin
    if rising_edge(clock) then
      if write_en = '1' then
        ram(to_integer(unsigned(write_addr))) <= write_data;
      end if;
      -- always read read old data
      read_data <= transport ram(to_integer(unsigned(read_addr))) after 200 ps;
    end if;
  end process;
end synchronous_old;

architecture synchronous_new of dual_port_ram is
  type ram_t is array(0 to 2**ADDR_BITS-1) of std_logic_vector(DATA_BITS-1 downto 0);
  signal ram : ram_t := (others => (others => '0'));
begin
  -- synchronous
  process(clock) is
  begin
    if rising_edge(clock) then
      if write_en = '1' then
        ram(to_integer(unsigned(write_addr))) <= write_data;
      end if;
      if read_addr = write_addr and write_en = '1' then
        read_data <= transport write_data after 50 ps;
      else
        read_data <= transport ram(to_integer(unsigned(read_addr))) after 200 ps;
      end if;
    end if;
  end process;
end synchronous_new;

architecture asynchronous_old of dual_port_ram is
  type ram_t is array(0 to 2**ADDR_BITS-1) of std_logic_vector(DATA_BITS-1 downto 0);
  signal ram : ram_t := (others => (others => '0'));
begin
  -- synchronous
  process(clock) is
  begin
    if rising_edge(clock) and write_en = '1' then
        ram(to_integer(unsigned(write_addr))) <= write_data;
    end if;
  end process;
  -- asynchronous
  process(read_addr) is
  begin
    read_data <= transport ram(to_integer(unsigned(read_addr))) after 200 ps;
  end process;
end asynchronous_old;

-- trocar esta parte do new
architecture asynchronous_new of dual_port_ram is
  type ram_t is array(0 to 2**ADDR_BITS-1) of std_logic_vector(DATA_BITS-1 downto 0);
  signal ram : ram_t := (others => (others => '0'));
begin
  -- write sincrono
  process(clock) is
  begin
    if rising_edge(clock) then
      if write_en = '1' then
        ram(to_integer(unsigned(write_addr))) <= write_data;
      end if;
    end if;
  end process;

  -- read assincrono
  read_data <= ram(to_integer(unsigned(read_addr)));

end asynchronous_new;
