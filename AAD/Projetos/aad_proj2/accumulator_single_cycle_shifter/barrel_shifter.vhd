library IEEE;
use IEEE.std_logic_1164.all;

entity barrel_shifter is
  generic
  (
    DATA_BITS : positive := 8
  );
  port
  (
    data_in  : in  std_logic_vector(DATA_BITS-1 downto 0);
    shift    : in  std_logic_vector(4 downto 0);  -- 5 bits para shift até 31
    data_out : out std_logic_vector(DATA_BITS-1 downto 0)
  );
end barrel_shifter;

architecture structural of barrel_shifter is
  signal shift1_out  : std_logic_vector(DATA_BITS-1 downto 0);
  signal shift2_out  : std_logic_vector(DATA_BITS-1 downto 0);
  signal shift4_out  : std_logic_vector(DATA_BITS-1 downto 0);
  signal shift8_out  : std_logic_vector(DATA_BITS-1 downto 0);
  signal shift16_out : std_logic_vector(DATA_BITS-1 downto 0);
begin
  -- shift de 1 bit
  shift1: process(data_in, shift(0))
  begin
    if shift(0) = '1' then
      shift1_out <= data_in(DATA_BITS-2 downto 0) & '0';
    else
      shift1_out <= data_in;
    end if;
  end process;

  -- shift de 2 bits
  shift2: process(shift1_out, shift(1))
  begin
    if shift(1) = '1' then
      shift2_out <= shift1_out(DATA_BITS-3 downto 0) & "00";
    else
      shift2_out <= shift1_out;
    end if;
  end process;

  -- shift de 4 bits
  shift4: process(shift2_out, shift(2))
  begin
    if shift(2) = '1' then
      shift4_out <= shift2_out(DATA_BITS-5 downto 0) & "0000";
    else
      shift4_out <= shift2_out;
    end if;
  end process;

  -- shift de 8 bits
  shift8: process(shift4_out, shift(3))
  begin
    if shift(3) = '1' then
      shift8_out <= shift4_out(DATA_BITS-9 downto 0) & "00000000";
    else
      shift8_out <= shift4_out;
    end if;
  end process;

  -- shift de 16 bits
  shift16: process(shift8_out, shift(4))
  begin
    if shift(4) = '1' then
      shift16_out <= shift8_out(DATA_BITS-17 downto 0) & "0000000000000000";
    else
      shift16_out <= shift8_out;
    end if;
  end process;

  data_out <= shift16_out;
end structural;
